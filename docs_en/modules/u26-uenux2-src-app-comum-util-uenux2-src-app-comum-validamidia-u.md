# u26 — power monitor thread, fatal errors, MI→MV report sync, result-stick validation, keyboard test, vote entry states

Unit u26 covers **110 wasm functions**. The tools grouped them under six directories of the VOTA application
(`uenux2`, the TSE code of the urna's election-day program):

| directory | what lives there |
|---|---|
| `app/comum/util/` | `CMonitoraAlimentacao`: logs changes of the power source and battery charge (plus `FormataTamanho`, path inferred) |
| `app/comum/validamidia/` | `CValidaMidia` / `CFabricaConteudoMidiaStart`: what the result USB stick (MR) may contain before results are copied to it |
| `app/vota/comum/` | `CSincronizaVota` (sign a report and replicate it from the internal to the external flash; global "urna desligando" flag), `votadefs.cpp` (signature checks, number of extra BU copies), `CThreadVota` (path inferred: the fatal-error handler of the three VOTA threads) |
| `app/vota/eleitor/iniciovotacao/` | `CVerificaHorarioZeresima` (wait for the zerésima time), the keyboard test (`testeteclado/`: `CPreZeresima`, `CTesteTeclado`, `CEsperaRetestar`) |
| `app/vota/eleitor/voto*/` | the vote-entry sub-states `CPedeMajoritario`, `CPedeProporcional`, `CCompletaProporcional`, `CPedeNulo` |
| `app/vota/log/`, `app/vota/monitor/` | `CLogVota::GetInst` and three log messages; `CThreadMonitor`, the hardware-watch thread |

The unit also received **helpers of files owned by other units**, because their callers are here. These are
`api::CDateTime` comparisons, report-builder pieces (`CPaperFormBuilder::Show/AddCut`, `CSubReport`, `CLp::Imprime`), the
"estado da urna" report (`CImprimirExtratoCarga::Imprime`, `CRelatorioTesteImpressora` ctor), the "Mais informações"
states (`CImpressaoEstadoUrna`, `CImpressaoListaEleitores`, `CInicioVotacao`, `CConfirmaImpressaoZeresima`), three
operator-terminal log lines, `CApplicationContextStack::Top`, `CApplication::ShowExceptionMsg`, `api::CTimer::Start`
and four libc++ instances. §15 maps all 110.

**Runtime.** The 20 µs sampling profiler saw 13 functions in the recorded votes: `CLogVota::GetInst` (184),
`CDateTime()`/`Compare` (479, 759), `CPath::GetPathMR` (949) and the proportional/majoritarian vote-entry states (5923,
5930, 5931, 11683, 11684, 11711, 11712, 11756, 11757). Sampling misses tiny functions, so a few more certainly ran:
`CPedeMajoritario::GetTelaCargoAtual` (5920, a 16-byte thunk called directly by the observed 11683/11684) and the
`CCompletaProporcional` constructor (5932, called by the lazy `CPedeNominal::GetInst` inlined in 11711 the first time a
party number is typed). `CPedeNulo::GetProximoEstado` (11744) runs for a proportional null vote, and `CTime::Compare`
(5448) whenever 759 compares two equal dates. The rest is not reached in the simulator: the monitor thread is never
started, the page never enters the start-of-day flow (zerésima, keyboard test), and the end-of-day flow (BU, result
stick) is never reached.

**Reconstructed sources** (`// wasm func N` on every function), under `src/uenux2/src/`:

* own files: `app/comum/util/cmonitoraalimentacao.{h,cpp}`, `app/comum/util/formatatamanho.cpp` (path inferred),
  `app/comum/validamidia/cvalidamidia.{h,cpp}`, `app/comum/validamidia/cfabricaconteudomidiastart.{h,cpp}`,
  `app/vota/comum/csincronizavota.{h,cpp}`, `app/vota/comum/votadefs.{h,cpp}`, `app/vota/comum/cthreadvota.{h,cpp}` (path
  inferred), `app/vota/log/clogvota.{h,cpp}`, `app/vota/monitor/cthreadmonitor.{h,cpp}`,
  `app/vota/eleitor/iniciovotacao/cverificahorariozeresima.{h,cpp}`,
  `app/vota/eleitor/iniciovotacao/testeteclado/{ctesteteclado,cprezeresima}.{h,cpp}`,
  `app/vota/eleitor/votamajoritario/cpedemajoritario.{h,cpp}`,
  `app/vota/eleitor/votaproporcional/{cpedeproporcional,ccompletaproporcional,cpedenulo}.{h,cpp}`
* fragments of other units' files: `app/vota/eleitor/comum/ctelasvota.u26.cpp`,
  `app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp`, `app/comum/u26-foreign-fragments.cpp`,
  `app/vota/u26-foreign-fragments.cpp`, `api/u26-foreign-fragments.cpp`

---

## 1. Glossary

| term | meaning |
|---|---|
| MI / MV / MR | *memória interna* (internal flash `/dsk/fi`), *memória de votação* (memory card `/dsk/fe`), *mídia de resultado* (result USB stick `/dsk/mr/`) |
| trab1 / trab2 | work directory of the 1st / 2nd round (`<flash>/dinamico/trab<N>/`) |
| SAVD, `.vsu` | the urna's signing/verification service; a `.vsu` is the signature package of a file in `dinamico/` |
| zerésima | report printed before the vote opens, proving that the urna holds zero votes |
| TE / MT | *terminal do eleitor* (the voter's screen and keypad) / *microterminal* (the mesário's 2-line LCD) |
| teste do teclado | the start-of-day test in which the mesário presses the 13 keys of the voter keypad in a random order |
| legenda | a party number (2 digits); a *voto de legenda* is a vote for a party only |
| inapto | a registered candidate who cannot receive votes; the vote is null |
| vias | printed copies of a report (BU, estado da urna, lista de eleitores ...) |
| chave | the urna's power key; "URNA DESLIGADA" = key turned off |
| contexto da aplicação | `api::CApplicationContext` (unit u15), the text shown when an operation fails |

## 2. Classes (RTTI) and layouts

```
api::CThread (u18)
└─ vota::CThreadVota            typeinfo @1532484, vtable @1532460   (path inferred vota/comum/cthreadvota.cpp)
   ├─ CThreadEleitor, CThreadOperador   (units u07 / u10)
   └─ vota::CThreadMonitor      typeinfo @1600968, vtable @1600768, 48 bytes

comum::CAppState ─ vota::CEstadoComDesligamentoAutomatico (u06)
   ├─ vota::CVerificaHorarioZeresima          @1545864   52 B (+28 tela, +36 CDateTime horário, +48 tick 2 s)
   ├─ testeteclado::CBase ─ CPreZeresima      @1546044   (+28 tela, +36 CDateTime agora)
   │                     ─ CRetomada          (u07/u20)
   ├─ testeteclado::CTesteTeclado             @1547140   68 B
   ├─ testeteclado::CEsperaRetestar / CTesteFalhou / CEnviarManutencao / CErroTesteTecladoFim   (u02)
comum::CAppState ─ vota::CVotacaoStateAudio (u08)
   ├─ vota::CPedeMajoritario                  @1550288   28 B
   ├─ vota::CPedeProporcional                 @1549440   28 B
   └─ vota::CCompletaProporcional             @1548104   32 B (+28 ETelaVotacao)
        ├─ vota::CPedeNulo                    @1548532   (tela 5)
        └─ vota::CPedeNominal                 @1549088   (tela 8, other unit)
comum::impl::IValidaMidia ─ comum::impl::CValidaMidia  @1577032, 4 B (vptr only)
vota::testeteclado::impl::IGeradorTeclas ─ impl::CGeradorTeclasAleatorio  @1547296
comum::IEventosLog ─ vota::CLogVota            @1532792, 8 B (+4 ELogAplicativos = 1)
comum::CImprimirExtratoCarga (abstract) ─ comum::CRelatorioTesteImpressora  @1576660  (26 slots)
```

Slot names of `CThreadVota` (from the bodies; the u07 names are kept for 3/4 with corrected parameter types):
`[0] ~CThreadVota (2126) [1] deleting [2] Run = 0 [3] TrataExcecao(const ecourna::api::exception::CError&) (7710)
[4] TrataExcecaoDesconhecida(const std::exception&) (7709) [5] FinalizaExecucao = 0`. 7710 `dynamic_cast`s its argument
from `CError` to `api::CUeDesligandoError`, and 7709 only calls `what()`. Slots 3 and 4 are already pure in
`api::CThread`'s own vtable (@1599960: slots 2, 3, 4 = `__cxa_pure_virtual`), so CThreadVota *overrides* them; only
slot 5 is new. (Unit u18's `cthread.h` gives the base declarations the wrong parameter lists: both bodies take a second
argument.) **No code dispatches to slots 3/4:** `CThread::ThreadProc` (10255) only calls `Run()` and sets the state to
3, and neither `CThreadEleitor::Run` (7061) nor `CThreadOperador::Run` (10204) has a try/catch, so both handlers are
unreachable in this build.

`CVotacaoStateAudio` hook slots 9/10 are now **attested**. `CCompletaProporcional` passes its
`__PRETTY_FUNCTION__` to `GetTelaCargoAtual(const std::string&)`, and the strings are
`"virtual void vota::CCompletaProporcional::ProcessInputAudio()"` (slot 9) and `"…::StartStateAudio()"` (slot 10).

`CMonitoraAlimentacao` (8 bytes, @1911568): `+0 ELogAplicativos (1)`, `+4 tipo de alimentação`,
`+5/+6 status das baterias interna/externa` (all `0xFF` = nothing logged yet), `+7 contador de eventos`.

## 3. The monitor thread (`CThreadMonitor`, `CMonitoraAlimentacao`)

**Creation.** `CThreadMonitor::GetInst` (1898) is a lazy singleton @1911596. The tools first named it after the inlined
`CMonitoraAlimentacao::CreateInst` record (they now show `vota::CThreadMonitor::GetInst`). Its constructor (cthreadmonitor.cpp:52) reads the headphone state from
`api::IPower`, sets the next periodic log to "now" and calls `CMonitoraAlimentacao::CreateInst(1)`, which throws
`CUeComumUtilError 9156 "Instância já criada"` if called twice. Callers: the urna execution policy
`vota::CExecucaoVota` and `CThreadOperador::FinalizaExecucao`.

**Loop** (`Run`, 10226; `do … while (!m_bParar)`, 500 ms sleep):

1. `CMonitoraAlimentacao::VerificaAlimentacao()` (cmonitoraalimentacao.cpp:52). When the power source changes
   (status bits 1..2: 0 rede, 1 bateria interna, 2 bateria externa, 3 → throws 8850), it logs `CLogComum::LogaTipoBateria()`
   ("Urna operando na rede elétrica/bateria interna/bateria externa") and resets the battery states. When the charge
   state of the battery in use changes (bits 3..4 internal, 5..6 external), it logs `LogaNivelBateria(1|2, estado)`
   ("Carga da [ALIMENTAÇÃO BATERIA …]: [PLENA|PARCIAL|CRÍTICA|AUSENTE]"). After **20 source changes** it logs
   "Suspenso monitoramento de alimentação devido repetição de eventos. [ 20 ] eventos" once and stops watching.
2. `MonitorFoneOuvido` (:127): "Fone de ouvido conectado/desconectado" on every change (status word +8, bit 1).
3. `VotacaoSuspensa` (:93): status bit 14 = power key off. Then `SaiPorVotacaoSuspensa` logs
   "Mudança do estado da chave: URNA DESLIGADA", sets the shutdown flag (func 3336), stops the voter and operator
   threads (`m_bParar`) and starts an `std::async` beep (lambda :103, func 10224, unit u19).
4. `VerificaErroCriticoFlash` (:153/:156): outside the *treinamento* phase, a missing external flash throws
   `CUeVotaError 9390 "Erro na Mídia Externa - Mídia não está presente"`.
5. `VerificaErroCriticoKbdTE` (:165/:167): status byte +19 bit 0 → `CUeVotaError 9391 "Erro no teclado do eleitor -
   dispositivo desconectado"`. The keypad connection is reported through the power/controller status block.
6. Every **1800 s** (`ISystemDateTime` slot 0), three logs run:
   * `LogaEspaco` (:225) logs "Espaço livre/utilizado na MI/MV [x.y MB]" (`std::filesystem::space` of
     `CPath::GetPathDinamico(0|1)`, the *throwing* overload: `__space` (4677) has its `error_code*` removed because it is
     always null, so a `statfs` failure throws `filesystem_error("space")`). It then calls
     `IInterfaceInit::LogDiskInfo` (command 19).
   * `LogaMemoria` logs "Quantidade de memória total/livre/usada [..]" from `/proc/meminfo` (func 5463).
   * `LogaStatusRedeAcBateria` (:258/:260) runs only on urna models ≥ 2020 and logs
     "Urna ligada conectada na|desconectada da rede CA em [v.vvV] e na Bateria Interna|Externa com [v.vvV/xA]"
     (IPower slots 1..4, values in hundredths; the current is a `double` printed with a plain `{}`, e.g. "0.35A").
     "Interna" is chosen when slot 3 (external battery voltage) is ≤ 0, and the voltage printed is then slot 1.

`FinalizaExecucao` (10225) sets `m_bParar` on the operator and voter threads. The `:117 MostraMensagemDesligamento`
source_location record exists in the data, but no code references it. Its caller was probably removed together
with the code that followed the `std::async` (see §12).

## 4. Fatal errors of a VOTA thread (`CThreadVota::TrataExcecao*`, 7710/7709)

This path runs on the urna only; in this build nothing calls it (see §2).

1. A `CUeDesligandoError` (the urna is being switched off) is ignored (7710 only). If the shutdown flag @1832936 is
   already set, the handler returns.
2. `FinalizaExecucao()` (slot 5).
3. On the mesário's microterminal (func 4633) the LED goes off and the screen shows "URNA ELETRÔNICA INOPERANTE" /
   "Siga as instruções na tela do eleitor".
4. `CSincronizaVota::MarcaUrnaDesligando()` (3336).
5. The voter screen shows the error page built from the top of the application-context stack (func 1695):
   * type: 2 for the generic fallback context, 1 otherwise (func 5569);
   * detalhe: the context's detail line;
   * title: `std::format("{} ({})", título, erro.GetCodigo())` for a `CError`, or the plain título for a `std::exception`;
   * message: `mensagem + "\n\n" + ações joined by "\n"`. For the generic context the exception text comes first. For a
     `std::exception` the regex `.*?\).*?:\d+:-?\d+ - ((.|\n)*)` strips the "função arquivo:linha:coluna - " prefix
     of TSE `what()` strings.

   `api::CApplication::ShowExceptionMsg(tipo, detalhe, título, mensagem, what)` (5568) displays the page.
6. `EnterLoopDoingNothing()` (5566) parks the thread forever.

If the stack is empty, `Top()` returns the generic context with **detalhe "Erro inesperado" and título "Pilha de
contexto vazia"**. That is the argument order in the binary; unit u15's comment has them swapped.

## 5. `CSincronizaVota` and `votadefs.cpp`: keeping the MV identical to the MI

* **Shutdown flag** `ms_desligando` (byte @1832936): set by 3336, tested inline by every writer.
  `VerificaUrnaDesligando()` throws `api::CUeDesligandoError(4201, "Urna desligando")` (csincronizavota.cpp:47). Func 1685 is
  only the out-of-line exception construction.
* **`SincronizaRelatorios(arquivo)`** (1836, :392). It runs after a report file was written into `<MI>/dinamico/trab<N>/`.
  1. It checks the shutdown flag, then calls `fsync` (`CSynchronizer`, empty in this build).
  2. It builds a `vota::CAssinadorVota` bound to package 122/123 (`vota.vsu` of the MI, turno 1/2).
  3. It maps the name to its SAVD file id: only `bu.dat` (86), `buj.dat` (84), `ze.dat` (88), `rze.dat` (89),
     `bim.dat` (106) and `behb.dat` (108) are accepted. Any other name throws `CUeVotaError 9302 "O arquivo<x> não é um
     relatório válido para sincronização"`. The literal @124100 is `"O arquivo"` with no trailing space, so the
     name is glued to the word (sic).
  4. `assinador.Assina(id)` (func 1277: SAVD sign request 0x42 for application 1) and fsync.
  5. It copies `<MI trab>/arquivo` → `<MV trab>/arquivo` (`CSystem::CopyFile`).
  6. It copies the matching signature package MI → MV (bu.vsu 126→128 / 127→129, buj.vsu 130→132, ze.vsu 138→140,
     rze.vsu 142→144, bim.vsu 191→193, behb.vsu 195→197; odd ids for turno 2). The inlined helper would copy **all six**
     packages for an unknown name, but that branch is dead because step 3 already rejected unknown names.
  7. fsync.
* **`VerificaAssinaturaMI/MV(pacote)`** (3290/3288): `IInterfaceSavd::ValidarUE(<trab of MI|MV>/<pacote>)`. It is used after
  every vote for `vota.vsu`, `rdv.vsu` and `uenux.vsu` (unit u07). In the web build `CWasmSavd` accepts everything.
* **`GetQuantidadeMaximaBUsAdicionais()`** (4582, :135) returns how many more BU copies may be printed. A voter-training
  urna (fase '3' + `treinamentoEleitor`) always gets 1. Otherwise the result is
  `obrigatórias − já impressas + adicionais` (`CInformacaoEleicao` funcs 3847/5914: 1 each in demo mode, else PU
  parameters +12/+16; `EstadoGeralVota.qtdBU`), with the assert `numImpresso >= numObrigatorias` (3449).

## 6. Validation of the result stick (`CValidaMidia`, `CFabricaConteudoMidiaStart`)

`vota::CCopiaResultadoParaMR` (12134, unit u09) uses two slots before copying BU/RDV/log files to `/dsk/mr/`:
slot 3 `ValidaMidiaResultado(aplicativo, turno)` if the stick is not empty, and, outside training, slot 6
`MidiaContemResultados()` ("Mídia de resultado já contém arquivos de resultados" 9369).

**`ValidaConteudo(dir, esperados)`** (5572) is the core:

1. It deletes desktop-OS metadata (5571). A missing name is not an error (libc++ `remove_all_impl` clears ENOENT), but
   the *throwing* `remove_all(path)` is used (ErrorHandler with `ec = nullptr`, no try/catch in 5571/5572/11172), so any
   other failure (read-only stick, EACCES, EIO...) throws `std::filesystem::filesystem_error("remove_all")` to
   `CCopiaResultadoParaMR`. For macOS: `.DS_Store .AppleDouble .LSOverride
   .DocumentRevisions-V100 .fseventsd .Spotlight-V100 .TemporaryItems .Trashes .VolumeIcon.icns
   .com.apple.timemachine.donotpresent`. For Windows: `Thumbs.db Thumbs.db:encryptable ehthumbs.db ehthumbs_vista.db
   "System Volume Information" $RECYCLE.BIN`. Each is removed with `std::filesystem::remove_all(dir + name)`.
2. If the directory is missing or empty, the result is valid (1) when nothing is expected, else invalid (6).
3. A sub-directory makes the stick invalid (7). Regular files are collected.
4. The result is invalid if more names are expected than there are files (8), if any file matches none of the **14
   permitted names** (9), or if an expected name is absent (10). Otherwise it is valid (2).

Permitted names (func 5572, in this order): `infomidia.dat`, `infomidia.vsc`, `turno2.jez`, `turno2.vsc`,
`#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.jez`, `#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.vsc`,
`#regex#[ost][0-9]{5}[a-z]{2}[0-9]{13}-[a-z]+\.[a-z]+` (result files), `#regex#[ost][0-9]{5}[0-9]{4}[0-9]{4}[0-9]{8}-[a-z]+\.[a-z]+`,
`#regex#[0-9]{8}\.[ste|STE]`, `ste.config`, `#regex#[0-9]{2}.pub`, `#regex#[0-9]{2}.id`, `#regex#[0-9]{2}ue`,
`#regex#[0-9]{3}ue[0-9]{2}\.vpe`.
A name is either `#regex#<pattern>`, tested with **unanchored** `boost::regex_search` (func 2199), or a literal of
the same length where `#` matches a digit, `@` a letter and `?` a letter/digit/`-`/`_`, in either string (5464).

`ConteudoIncializacao(aplicativo, turno)` (cfabricaconteudomidiastart.cpp:62) lists what an initialised stick must hold.
Applications 11–14 and 18 need `infomidia.dat/.vsc`. Applications 15/16/17 need those plus the SA package
`t|o|s#####xx-pkgsa.jez/.vsc` (treinamento/oficial/simulado). Application 20 needs `##.pub ##.id ##ue ###ue##.vpe`.
Applications 7–10 need nothing. The others (1–6 and 19) throw 9200 "Não há mídia de resultado de inialização para a
aplicação '{}'." (sic) when the turno is '1'/'2'; with any other turno their list is empty and nothing is thrown.
Applications 7–10 and 15–17 require turno '1'/'2' (else result 5), and values outside 1..20 give result 4. The web
build's `CWasmInit` reports **10** for VOTA, so for the simulator the list is empty and only the permitted-names check
applies. When the result is ≥ 4, the *expected* names (not the offending files) are logged at LOG_INFO (5574).

Slot 8 (`MidiaContemSomenteResultados`, 11167) is a stricter variant: no metadata cleanup, no extra files, no
sub-directory. Slot 9 only runs the cleanup.

## 7. Start of the day: zerésima time, keyboard test, "Mais informações"

* **`CVerificaHorarioZeresima`** (11871/11869/11870/11868; constructor in 5947). Before `CConfiguracaoEleicao +544` (date/time
  of the zerésima) it logs "Aguardando data e hora para emissão da zerésima", shows the "ATENÇÃO / Esta urna eletrônica
  só funcionará a partir de …" screen and re-checks every 2 s. Once the time has passed, with the assert
  `EstadoVota == EAVAGUARDAHORAZERESIMA` (:64/:80), it moves to `CInicioZeresima`. Keys: BRANCO opens
  `CMaisInformacoes`. CONFIRMA prints the **estado da urna** report (§9) while `numRelatorioEstado` (PU) is greater
  than the copies already printed (`EstadoGeralVota +73`), then rebuilds the screen.
* **Keyboard test.**
  * `CPreZeresima::CriaTela` (11862) makes the test optional ("Quer testar o teclado?" Testar / Não testar, func 6581)
    while today is before the eve of the zerésima date (`i64.load offset=544` of `CConfiguracaoEleicao`, the date part
    of the same data/hora da zerésima that `CVerificaHorarioZeresima` waits for, minus one day with func 5477), and
    mandatory ("Por favor, teste o teclado", 6580) from that eve on. Since the zerésima is printed on election day
    this is in practice the eve of the election. "Não testar" (slot 12, 11861) works only in the optional period.
  * After the test, `GetEstadoPassouNoTeste` (5945, :56) goes to `CGeraDadosDinamicos` (estado '1'/'2') or
    `CVerificaHorarioZeresima` ('3'). Any other state logs "Erro estado do aplicativo não esperado" and throws
    9377 "Estado nao esperado <n>" (with the numeric char code, e.g. "52").
  * `CTesteTeclado::StartState` (11805) logs "Início do teste de Teclado do TE" and draws the 13 keys as `CTextBox`es.
    The layout depends on the urna model (≤ 2019 or ≥ 2020, `GetKeyboardLayout`, ctelasvota.cpp:1301). The labels are
    `"1".."9","0","BRANCO","CORRIGE","CONFIRMA"` @1833360. It asks `impl::IGeradorTeclas` (default registration
    :121: `CGeradorTeclasAleatorio`) for a random order and highlights the first key. Errors: 9359 "Sem teclas para
    testar", 9360/9361/9362 "Tecla não encontrada: <x>".
  * `ProcessInput` (11804): the right key goes back to normal and the next key is highlighted. After the last key it
    logs "Fim do teste de Teclado do TE - Sucesso" and goes to the saved next state. A **wrong key** stores the expected
    and pressed keys in `CInformacaoEleitor` +9/+10 and goes to `CTesteFalhou` ("Teste Falhou", "Falha: Esperada X,
    pressionada Y", Repetir teste / Prosseguir). It **also logs the success message** (see §13).
  * `CEsperaRetestar` (11823/11826) counts down to `g_horaRetestar` @1835000 ("Por favor, espere {}s …"), then goes back
    to `CTesteTeclado`.
* **`CConfirmaImpressaoZeresima::ProcessInput`** (11927): BRANCO opens "Mais informações". CONFIRMA, after the zerésima
  deadline (+20), goes to `CImpressaoZeresimaTardia`; otherwise it sets `EstadoVota = '4'`, saves the state and goes to
  `CGeraZeresima`.
* **`CInicioVotacao`** (11988/11987) waits for the opening time (+28), polling with its tick (+40), then goes to
  `CAguardaMensagem`.
  `IniciaVotacao` (5972) records `dhIniAquisicao` and saves the state when `CRdvVota.Comparecimento() == 0`, shows
  `CTelasVota +132` (first opening) or `+140` (resumed), and posts message 8 to the operator thread.
* **"Mais informações" items**: `CImpressaoEstadoUrna` (11911) prints the estado da urna report again, with no copy
  limit (see §13). `CImpressaoListaEleitores` (11908, condensed) prints the voter list with the legend "AUD: Áudio / BIO:
  Biometria / IMP: Impedido / TTE: Transferência Temporária" and counts copies at `EstadoGeralVota +74`.
  `CGeradorRelVersaoPacoteDados::Imprime` (5592/5596) prints the package versions report.

## 8. Vote entry (observed executing)

```
CPedeProporcional (screen "party", 2 digits)
   BRANCO ─────────────► CConfereVotoEmCargo<CProporcionalBranco, 4>
   2 digits complete ──► party exists && LegendaValida(cargo, party) (func 5922) ─► CPedeNominal (tela 8)
                                                                              else ─► CPedeNulo (tela 5)
CCompletaProporcional (CPedeNominal / CPedeNulo): the remaining digits
   CORRIGE ────────────► CPedeProporcional
   complete ───────────► g_votoDigitado += digits; GetProximoEstado(g_votoDigitado)   (slot 16)
      CPedeNulo:  full-length number of an "inapto" candidate ─► <CCandidatoInapto, 14>, else <CProporcionalNulo, 7>
CPedeMajoritario (full number)
   BRANCO ─────────────► <CMajoritarioBranco, 4>
   complete ───────────► !ExisteCandidatoValidoResposta ─► <CMajoritarioNulo, 7>
                         VotoRepetido (same nominal vote for an earlier vaga of this cargo) ─► <CMajoritarioRepetido, 16>
                         else ─► <CMajoritarioValido, 2>
```

* `ExisteCandidatoValidoResposta` (:125). For a consulta (referendum) it looks the answer up in `CRespostas`.
  Otherwise the candidate must exist and not be flagged at `CCandidatura +52` (inapto), and its party must exist:
  a missing party logs "Erro partido não encontrado" and throws 9382. In practice an inapto majoritarian candidate leads
  to the **null-vote** screen (no "inapto" screen for majoritarian offices). Both paths store an RDV "nulo".
* `VotoRepetido` runs only when `CCargo.qtdEscolhas > 1`, `CCargo +14 == 0` and `g_numeroEscolha > 1`. It scans the
  last `g_numeroEscolha − 1` entries of `g_votosEleitor` with `.at()`.
* `EInputResult 9` ("Confirma" in the enum) is what the input field returns once it is full. `CCompletaProporcional`'s
  echo (5930) refuses a digit only when the text did not change. The base version (3139) also refuses a no-op CORRIGE.
* Audio templates: majoritarian and proportional use "Você está votando para {cargo-atual}. {quantidade-digitos}. Voto
  {progresso}." (a consulta drops the prefix). `CPedeNulo` uses "… no candidato {voto}. Número errado. Aperte confirma
  para prosseguir, ou corrige para reiniciar este voto."

## 9. Printed reports touched here

* **Estado da urna** (`CImprimirExtratoCarga::Imprime`, 5591, a template method over 24 virtual hooks of
  `CRelatorioTesteImpressora`). The report contains, in order:
  * paper cut, "IMPRESSO EM MODO DEMONSTRAÇÃO" (demo mode), the carga title with `<uf>`, "Eleições Comunitárias",
    election name, pleito name and "(DD/MM/YYYY)", the eleição names;
  * the `====` separator, "ESTADO DA URNA", the phase separator, "UE DE SEÇÃO|CONTINGÊNCIA", "UF …",
    município/zona (labels `<MCSN>`, `<ZCSN>` translated by `CTradutorFrase`), local lines;
  * "Código identificação UE {:08}" / "MC {:.8}", "Código de identificação carga" + code, "Data da carga" / "Hora da
    carga", "EMISSÃO DO RELATÓRIO", "RESUMO DA CORRESPONDÊNCIA";
  * the daylight-saving forecast ("Previsão de horário de verão:", "Início: …, fim: ….", "-> Urna em horário de verão
    <-" / "Não existe previsão de horário de verão."), skipped in treinamento;
  * QR code(s), **"Ver: 10.23.0.1 - DESENVOLVIMENTO"** (literal @326597), "URNA OPERANDO EM PERFEITAS / CONDIÇÕES DE
    FUNCIONAMENTO", "ASSINATURAS", 8 blank lines and a cut.

  The job goes through `CPaperFormBuilder::Show(título, via)` (3671) → `CLp::Imprime` (3875) → the
  `IImpressoraRelatorios` singleton, which is `CWasmNullPrinter` in the web build (nothing is printed).
* `CRelVotaUtil::CortaPapel` (2882): 2 blank lines and a cut as an untitled job (between zerésima copies).
* `CGeradorRelVersaoPacoteDados` footer (5596): "Código de identificação da carga", code, "Ver: 10.23.0.1" (the
  `GetVersionNumber` of the same literal), cut.

## 10. BOLETIM DE URNA (BU): what this unit contributes

1. **After generation** (unit u08/u09 `CGeraBU`, 12110, writes `<MI>/dinamico/trab<N>/bu.dat`), `CGeraBU` calls
   `CSincronizaVota::SincronizaRelatorios("bu.dat")` (§5). It stops with `CUeDesligandoError 4201` if the urna is
   shutting down. Otherwise it asks SAVD to sign `bu.dat` (file id 86, sign command 0x42, application 1), fsyncs, copies
   `bu.dat` to `<MV>/dinamico/trab<N>/bu.dat` and copies the package `bu.vsu` (ids 126→128 for turno 1, 127→129 for turno 2).
   The same happens for `buj.dat`, `bim.dat`, `behb.dat` (CGeraRelatorios) and `ze.dat`, `rze.dat` (zerésima).
   The MV copy of the BU is therefore a byte copy of the MI one, and the MV `.vsu` is the MI package (the MV file is not
   signed separately).
2. **Number of copies**: `GetQuantidadeMaximaBUsAdicionais()` (4582) bounds the extra copies ("vias adicionais") that
   `CEmitirMaisBU` offers: `obrigatórias + adicionais − impressas`, or 1 on a voter-training urna. The printed-copy
   counter is `EstadoGeralVota.qtdBU`.
3. **Result stick**: before `bu.dat`, `rdv.dat`, `jufa.dat`, `imgbu.dat`, `imgze.dat`, `hash.dat`, `log.jez`, `vota.vsc`, `mr.ver`
   (and the `wsq*.jez` of biometric urnas) are copied to `/dsk/mr/`, a non-empty stick must pass
   `ValidaMidiaResultado(aplicativo, turno)` (§6). Outside training it must not already hold result files
   (`MidiaContemResultados()`, regex `[ost]#####xx#############-<tipo>.<ext>`). The file-name pattern of the section's
   results is `<fase o|s|t><pleito:05><uf><município:05><zona:04><seção:04>-<tipo>.<ext>` (slot 5, func 11170).
4. **Failure**: an exception while writing the BU (context "Gerando boletim de urna na MI", unit u09) would end in the
   fatal-error screen of §4 on a real urna. In the web build none of this is reached.

## 11. Web build specifics

* Not started or not reached: `CThreadMonitor`, the start-of-day chain (`CVerificaHorarioZeresima`, keyboard test,
  zerésima, `CInicioVotacao`), the end-of-day chain (BU sync, `CValidaMidia`) and `CThreadVota`'s handlers.
* The platform singletons used by this code are mocks. `IPower` = `api::teste::CPowerMock` (mains, full battery).
  `IUrna` = `CUrnaMock` (model 2020). `IImpressoraRelatorios` = `CWasmNullPrinter`. `IInterfaceSavd` = `(anon)::CWasmSavd`
  (signs/validates nothing). `IInterfaceInit` = `CWasmInit` (aplicativo 10).
* `/proc/meminfo`, `/dsk/mr/`, `/dev/urna` do not exist in MEMFS: memory logs would be 0, the stick "absent", and
  `LogInfo`/`DebugUenux` silent (no `DEBUG_UENUX` environment variable).

## 12. WebAssembly / Emscripten observations

* **Threads that cannot exist.** Without pthreads the compiler proved that `std::thread`/`std::async` always throw.
  `SaiPorVotacaoSuspensa`, `CApplication::ShowExceptionMsg` (5568) and `api::CTimer::Start` (10850) are reduced to
  "allocate the shared state + `std::__thread_struct` (2541) + throw `std::system_error("thread constructor failed")`",
  and **everything after the call was deleted** (the error-screen drawing of 5568, the rest of `SaiPorVotacaoSuspensa`).
  This is why the `:117` record has no reference left.
* `std::this_thread::sleep_for(500ms)` in `CThreadMonitor::Run` became `if (byte@1584624 == 1) emscripten_sleep(500)`,
  which aborts in this build (no Asyncify).
* **merge-similar-functions**: `CLogVota` 3902 is the body of every 38-byte log literal. Its five 8-byte chunks became
  pointer parameters: 5880 "Imprimindo relatório de estado da urna", 5881 "Erro estado do aplicativo não esperado",
  5885 "Mídia de resultado não estava presente". `GetTelaCargoAtual` thunks 5920/5923/5931 pass the srcloc and the error
  code to shared bodies 6050/3921. The generic-context constructor 5558 and the lazy singletons 6051/764 are the same
  kind of merge.
* **Dead-argument elimination / constant propagation**: `CPaperFormBuilder::Show` keeps a `bool` in its lambda's RTTI name
  but the wasm body has only two parameters, because every caller passes `true`.
* **wasm32 `size_t`**: `FormataTamanho(size_t)` truncates the 64-bit statvfs/memory counts modulo 4 GiB.
* **libc++ ABI v2 details**: `std::filesystem::space` inlined via `__space` (4677); `std::map` initializer-list
  constructor (6579) for the keypad layout; `__tree::destroy` (4148); unanchored `boost::regex_search` wrapper (2199)
  that swallows exceptions.

## 13. Suspicious or weird code

1. **Failed keyboard test logged as success** (11804 → 4511): a wrong key writes "Fim do teste de Teclado do TE -
   Sucesso" to `logd.dat`. No failure record exists anywhere in the binary.
2. **Weak permitted-file check on the result stick** (5572/5465): `#regex#` patterns are searched, not anchored. For
   example `[0-9]{2}ue` accepts any name that *contains* two digits followed by "ue", and `[0-9]{8}\.[ste|STE]` is a
   character class (it accepts `12345678.|` or `12345678.e…`).
3. **Possible heap over-read in `LeValorMemInfo`** (5463): a 4096-byte buffer, `read(…, 4096)`, then `strlen(buffer)`.
   If `/proc/meminfo` is 4096 bytes or longer there is no NUL, and `strlen` runs past the allocation.
4. **Truncated sizes in the log** (1947): `size_t` parameter on wasm32 (§12).
5. **Swapped fallback context** (1695): "Pilha de contexto vazia" becomes the *title* (context +12, printed as
   "<título> (<código>)") and "Erro inesperado" the detail (+0). `InitApplication` uses "Erro inesperado" as the title
   (detail "Não é possível continuar a execução"). The binary fact is certain; that it is an unintended swap is an
   inference.
6. **Exception handlers unreachable** (7709/7710): no try/catch dispatches to `CThreadVota` slots 3/4 in this binary.
   On the urna an exception escaping `Run()` would terminate instead of showing the error page, unless the native
   build differs.
7. **Misleading diagnostics** (11172): when the stick is invalid, the log lists the *expected* names, not the files that
   caused the rejection.
8. **Unbounded estado-da-urna copies from "Mais informações"** (11911): no check against `numRelatorioEstado`, unlike
   11869. The menu itself (`CMaisInformacoes::ProcessInput`, 11896) does not gate option 1 either: it maps the typed
   number 1..5 straight to the item states.
9. **Monitoring suppressed after 20 events** (`VerificaAlimentacao`): repeated source changes silence all later power
   logs (by design, anti-flood).
10. **Hard-coded development version on paper** (5591): the estado-da-urna report prints the literal @326597 in full,
    "Ver: 10.23.0.1 - DESENVOLVIMENTO". The package-versions footer (5596) and the voter list (11908) pass the same
    literal through `api::CStringUtils::GetVersionNumber` (regex `^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+`) and print only
    "Ver: 10.23.0.1".

## 14. Open questions

* The real file of `CThreadVota` (`vota/comum/` here, `vota/` in u07's include) and of `FormataTamanho`.
* The names of IPower slots 1..4 (voltages/current) and of the status bits beyond those printed.
* `CCargo +14` (skips the repeated-vote check) and `CCandidatura +52` (inapto flag) need a data-model confirmation.
* `ETipoUrnaOperacao '2'` in the `CRelatorioTesteImpressora` constructor (use of pleito 2).
* The meaning of `CSubReport`'s flag and of `CLp::Imprime`'s IImpressoraRelatorios slot 9 / CSubReport slots 5, 6.
* Whether the native build keeps a try/catch around `Run()` that calls slots 3/4.

## 15. Complete mapping table (110 functions)

* **run**: ✓ = observed executing in the recorded votes (`analysis/runtime/*.functions.tsv`).
* **reconstructed in**: path under `src/uenux2/src/`. "library" = libc++ code, not reconstructed.

| idx | size | run | tools name | reconstructed symbol | original file | reconstructed in | kind / conf. |
|---|---|---|---|---|---|---|---|
| 184 | 2232 | ✓ | `vota::CLogVota::GetInst` | `vota::CLogVota::GetInst` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE (srcloc :37) / high |
| 479 | 24 | ✓ | `api_f479` | `api::CDateTime::CDateTime() [current local time]` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | TSE helper / medium |
| 759 | 36 | ✓ | `vota_f759` | `api::CDateTime::Compare` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | TSE helper / medium |
| 940 | 77 |  | `api_f940` | `api::IField::SetEstado` | uenux2/src/api/gui/ifield.h (path inferred) | api/u26-foreign-fragments.cpp | TSE GUI helper / medium |
| 949 | 15 | ✓ | `comum_f949` | `comum::CPath::GetPathMR` | uenux2/src/app/comum/cpath.cpp | app/comum/cpath.cpp (unit u22); note in app/comum/u26-foreign-fragments.cpp | TSE / high |
| 1156 | 169 |  | `comum_f1156` | `comum::CRelUtil::IncluiSeparador` | uenux2/src/app/comum/relatorios/crelutil.cpp (path inferred) | app/comum/u26-foreign-fragments.cpp | TSE report helper / low |
| 1264 | 371 |  | `comum_f1264` | `api::CPaperFormBuilder::AddCut` | uenux2/src/api/gui/cpaperformbuilder.h (path inferred) | api/u26-foreign-fragments.cpp | TSE report helper / medium |
| 1280 | 123 |  | `api_f1280` | `vota::CMaisInformacoes::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (lazy singleton) / medium |
| 1398 | 14 |  | `api_f1398` | `comum::IEventosLog::LogaAviso` | uenux2/src/app/comum/log/ieventoslog.h | note in app/comum/u26-foreign-fragments.cpp | TSE inline (severity 2 copy) / medium |
| 1685 | 136 |  | `vota::CSincronizaVota::VerificaUrnaDesligando` | `vota::CSincronizaVota::VerificaUrnaDesligando [throw-expression: api::CUeDesligandoError ctor]` | uenux2/src/app/vota/comum/csincronizavota.cpp | app/vota/comum/csincronizavota.cpp | TSE (srcloc :47) / high |
| 1695 | 354 |  | `vota_f1695` | `api::CApplicationContextStack::Top` | uenux2/src/api/gui/capplicationcontextstack.cpp (path inferred) | api/u26-foreign-fragments.cpp | TSE / medium |
| 1836 | 5000 |  | `vota::CSincronizaVota::SincronizaRelatorios` | `vota::CSincronizaVota::SincronizaRelatorios` | uenux2/src/app/vota/comum/csincronizavota.cpp | app/vota/comum/csincronizavota.cpp | TSE (srcloc :392) / high |
| 1898 | 450 |  | `vota::CThreadMonitor::GetInst` (formerly shown by the tools as `comum::util::CMonitoraAlimentacao::CreateInst`) | `vota::CThreadMonitor::GetInst` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (ctor inlined; CMonitoraAlimentacao::CreateInst in app/comum/util/cmonitoraalimentacao.cpp) | TSE (srclocs cthreadmonitor.cpp:52, cmonitoraalimentacao.cpp:29 inlined) / medium |
| 1947 | 1448 |  | `vota_f1947` | `comum::util::FormataTamanho` | uenux2/src/app/comum/util/formatatamanho.cpp (path and name inferred) | app/comum/util/formatatamanho.cpp | TSE helper / low |
| 2126 | 64 |  | `vota::CThreadMonitor::vf0` | `vota::CThreadVota::~CThreadVota` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | app/vota/comum/cthreadvota.cpp | TSE (dtor, slot 0 of CThreadVota and CThreadMonitor) / high |
| 2541 | 39 |  | `vota_f2541` | `std::__thread_struct::__thread_struct` | library: libcxx/src/thread.cpp | comment in api/u26-foreign-fragments.cpp | library (std::thread / std::async support) / medium |
| 2758 | 507 |  | `comum_f2758` | `comum::impl::(anonymous)::TodosCorrespondem` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 2784 | 37 |  | `comum_f2784` | `comum::impl::(anonymous)::ConteudoValido` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 2868 | 171 |  | `vota::testeteclado::CTesteTeclado::vf0` | `vota::testeteclado::CTesteTeclado::~CTesteTeclado` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (dtor) / high |
| 2882 | 465 |  | `vota::CRelVotaUtil::CortaPapel` | `vota::CRelVotaUtil::CortaPapel` | uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE / high |
| 3054 | 290 |  | `api_f3054` | `vota::testeteclado::(anonymous)::AdicionaCabecalhoTesteTeclado` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE GUI / low |
| 3268 | 538 |  | `vota_f3268` | `vota::CLogVota::LogaQuantidade` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / low |
| 3288 | 356 |  | `vota::VerificaAssinaturaMV` | `vota::VerificaAssinaturaMV` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :110) / high |
| 3290 | 356 |  | `vota::VerificaAssinaturaMI` | `vota::VerificaAssinaturaMI` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :103) / high |
| 3336 | 12 |  | `vota_f3336` | `vota::CSincronizaVota::MarcaUrnaDesligando` | uenux2/src/app/vota/comum/csincronizavota.h | app/vota/comum/csincronizavota.h (inline) | TSE / medium |
| 3660 | 116 |  | `vota_f3660` | `api::CSubReport::CSubReport` | uenux2/src/api/gui/reports/csubreport.cpp (path inferred) | api/u26-foreign-fragments.cpp | TSE report helper / medium |
| 3671 | 483 |  | `comum_f3671` | `api::CPaperFormBuilder::Show` | uenux2/src/api/gui/cpaperformbuilder.cpp (path inferred) | api/u26-foreign-fragments.cpp | TSE (lambda RTTI gives the signature) / high |
| 3855 | 143 |  | `vota_f3855` | `vota::testeteclado::CTesteTeclado::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (lazy singleton) / high |
| 3865 | 22 |  | `vota_f3865` | `vota::CInicioZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciozeresima.cpp (path inferred) | note in app/vota/u26-foreign-fragments.cpp | TSE (merged singleton thunk) / high |
| 3875 | 82 |  | `vota_f3875` | `api::CLp::Imprime` | uenux2/mock/app/simulador/wasm/cwasmclp.cpp (or api/print/clp.cpp; path inferred) | api/u26-foreign-fragments.cpp | TSE / low |
| 3902 | 145 |  | `vota_f3902` | `vota::CLogVota::Loga38 [merged body of 38-byte log literals]` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | wasm-opt merged body / medium |
| 4148 | 53 |  | `api_f4148` | `std::__tree<std::string,...>::destroy (keypad layout map)` | library: libcxx/include/__tree | comment in app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | library instantiation / medium |
| 4511 | 150 |  | `vota_f4511` | `vota::CLogVota::LogaFimTesteTecladoSucesso` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / medium |
| 4582 | 157 |  | `vota::GetQuantidadeMaximaBUsAdicionais` | `vota::GetQuantidadeMaximaBUsAdicionais` | uenux2/src/app/vota/comum/votadefs.cpp | app/vota/comum/votadefs.cpp | TSE (srcloc :135) / high |
| 4633 | 675 |  | `vota_f4633` | `vota::(anonymous)::MostraUrnaInoperanteNoMicroterminal` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | app/vota/comum/cthreadvota.cpp | TSE / low |
| 4677 | 494 |  | `vota_f4677` | `std::filesystem::__space` | library: libcxx/src/filesystem/operations.cpp | comment in app/vota/monitor/cthreadmonitor.cpp | library / high |
| 5448 | 23 |  | `vota_f5448` | `api::CTime::Compare` | uenux2/src/api/util/ctime.cpp | api/u26-foreign-fragments.cpp | TSE helper / medium |
| 5453 | 132 |  | `vota_f5453` | `vota::(anonymous)::EspacoUtilizado` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / low |
| 5454 | 122 |  | `vota_f5454` | `vota::(anonymous)::EspacoLivre` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / low |
| 5463 | 1093 |  | `vota_f5463` | `vota::(anonymous)::LeValorMemInfo` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE / low |
| 5464 | 110 |  | `comum_f5464` | `comum::impl::(anonymous)::CaractereCoringa` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 5465 | 258 |  | `comum_f5465` | `comum::impl::(anonymous)::BuscaRegex` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 5471 | 242 |  | `vota_f5471` | `api::CDateTime::DiferencaSegundos` | uenux2/src/api/util/cdatetime.cpp | api/u26-foreign-fragments.cpp | TSE helper / low |
| 5568 | 100 |  | `vota_f5568` | `api::CApplication::ShowExceptionMsg` | uenux2/src/api/gui/capplication.cpp | api/u26-foreign-fragments.cpp | TSE (body cut after std::async) / medium |
| 5569 | 217 |  | `vota_f5569` | `vota::(anonymous)::TipoContexto` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | app/vota/comum/cthreadvota.cpp | TSE / low |
| 5571 | 2179 |  | `comum_f5571` | `comum::impl::(anonymous)::RemoveArquivosSO` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 5572 | 2128 |  | `comum_f5572` | `comum::impl::(anonymous)::ValidaConteudo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE / medium |
| 5574 | 16 |  | `comum_f5574` | `comum::impl::(anonymous)::LogInfo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (variadic wrapper of vota_f2921) / medium |
| 5586 | 205 |  | `comum_f5586` | `comum::CRelatorioTesteImpressora::CRelatorioTesteImpressora` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | app/comum/u26-foreign-fragments.cpp | TSE (ctor) / high |
| 5591 | 8678 |  | `vota_f5591` | `comum::CImprimirExtratoCarga::Imprime` | uenux2/src/app/comum/relatorios/cimprimirextratocarga.cpp (path inferred) | app/comum/u26-foreign-fragments.cpp | TSE (template method) / medium |
| 5592 | 53 |  | `comum::CGeradorRelVersaoPacoteDados::vf2` | `comum::CGeradorRelVersaoPacoteDados::Imprime` | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (path inferred) | app/comum/u26-foreign-fragments.cpp | TSE (slot 2) / medium |
| 5596 | 826 |  | `comum::CGeradorRelVersaoPacoteDados::vf5` | `comum::CGeradorRelVersaoPacoteDados::MontaRodape` | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (path inferred) | app/comum/u26-foreign-fragments.cpp | TSE (slot 5) / low |
| 5880 | 29 |  | `vota_f5880` | `vota::CLogVota::LogaImprimindoRelatorioEstadoUrna` | uenux2/src/app/vota/log/clogvota.cpp | app/vota/log/clogvota.cpp | TSE / medium |
| 5920 | 16 |  | `vota::CPedeMajoritario::GetTelaCargoAtual` | `vota::CPedeMajoritario::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.u07.cpp (body 6050); note in cpedemajoritario.cpp | TSE (srcloc :44; thunk) / high |
| 5923 | 16 | ✓ | `vota::CPedeProporcional::GetTelaCargoAtual` | `vota::CPedeProporcional::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp (note; body 6050 in cpedemajoritario.u07.cpp) | TSE (srcloc :42; thunk) / high |
| 5930 | 522 | ✓ | `vota::CCompletaProporcional::EmiteEcoComInputField` | `vota::CCompletaProporcional::EmiteEcoComInputField` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (srcloc :60) / high |
| 5931 | 20 | ✓ | `vota::CCompletaProporcional::GetTelaCargoAtual` | `vota::CCompletaProporcional::GetTelaCargoAtual` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp (note; body 3921 in cconfirmavotoemcargo.cpp, u06) | TSE (srcloc :83; thunk) / high |
| 5932 | 28 |  | `vota_f5932` | `vota::CCompletaProporcional::CCompletaProporcional` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.h | app/vota/eleitor/votaproporcional/ccompletaproporcional.h (inline ctor) | TSE (ctor) / medium |
| 5945 | 207 |  | `vota::testeteclado::CPreZeresima::GetEstadoPassouNoTeste` | `vota::testeteclado::CPreZeresima::GetEstadoPassouNoTeste` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (srcloc :56) / high |
| 5972 | 108 |  | `vota_f5972` | `vota::(anonymous)::IniciaVotacao` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE / low |
| 6579 | 572 |  | `api_f6579` | `std::map<std::string, SLayoutTecla>::map(initializer_list)` | library: libcxx/include/map | comment in app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | library instantiation / medium |
| 6580 | 201 |  | `vota_f6580` | `vota::CTelasVota::CriaTelaTesteTeclado` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE / medium |
| 6581 | 211 |  | `vota_f6581` | `vota::CTelasVota::CriaTelaTesteTecladoOpcional` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE / medium |
| 7709 | 2883 |  | `vota::CThreadMonitor::vf4` | `vota::CThreadVota::TrataExcecaoDesconhecida` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | app/vota/comum/cthreadvota.cpp | TSE (vtable slot 4) / medium |
| 7710 | 2317 |  | `vota::CThreadMonitor::vf3` | `vota::CThreadVota::TrataExcecao` | uenux2/src/app/vota/comum/cthreadvota.cpp (path inferred) | app/vota/comum/cthreadvota.cpp | TSE (vtable slot 3) / medium |
| 10225 | 18 |  | `vota::CThreadMonitor::vf5` | `vota::CThreadMonitor::FinalizaExecucao` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE (vtable slot 5) / high |
| 10226 | 7171 |  | `vota::CThreadMonitor::vf2` | `vota::CThreadMonitor::Run` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp | TSE (vtable slot 2; many srclocs) / high |
| 10227 | 13 |  | `vota::CThreadMonitor::vf1` | `vota::CThreadMonitor::~CThreadMonitor [deleting]` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (comment) | compiler-generated (slot 1) / high |
| 10229 | 38 |  | `vota_f10229` | `vota::CThreadMonitor::s_instancia at-exit reset` | uenux2/src/app/vota/monitor/cthreadmonitor.cpp | app/vota/monitor/cthreadmonitor.cpp (comment) | compiler-generated / medium |
| 10416 | 176 |  | `vota::CPerguntaEleitorVotando::vf2` | `vota::CPerguntaEleitorVotando::StartState` | uenux2/src/app/vota/operador/... (file unknown) | app/vota/u26-foreign-fragments.cpp | TSE (vtable slot 2) / medium |
| 10642 | 164 |  | `vota::CEleitorJaVotou::vf2` | `vota::CEleitorJaVotou::StartState` | uenux2/src/app/vota/operador/... (file unknown) | app/vota/u26-foreign-fragments.cpp | TSE (vtable slot 2) / medium |
| 10774 | 409 |  | `vota::CControladorRegistraMesariosVota::vf28` | `vota::CControladorRegistraMesariosVota::LogaLimiteMesariosAtingido` | uenux2/src/app/vota/operador/... (file unknown) | app/vota/u26-foreign-fragments.cpp | TSE (vtable slot 28) / low |
| 10850 | 80 |  | `api::CTimer::vf2` | `api::CTimer::Start` | uenux2/src/api/util/ctimer.cpp (path inferred) | api/u26-foreign-fragments.cpp | TSE (vtable slot 2) / low |
| 11166 | 138 |  | `comum::impl::CValidaMidia::vf9` | `comum::impl::CValidaMidia::RemoveArquivosSistemaOperacional` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 9) / medium |
| 11167 | 1260 |  | `comum::impl::CValidaMidia::vf8` | `comum::impl::CValidaMidia::MidiaContemSomenteResultados` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 8) / low |
| 11168 | 414 |  | `comum::impl::CValidaMidia::vf7` | `comum::impl::CValidaMidia::MidiaContemArquivo` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 7) / low |
| 11169 | 388 |  | `comum::impl::CValidaMidia::vf6` | `comum::impl::CValidaMidia::MidiaContemResultados` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 6; name as in u09) / medium |
| 11170 | 887 |  | `comum::impl::CValidaMidia::vf5` | `comum::impl::CValidaMidia::MidiaContemResultadosSecao` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 5) / low |
| 11171 | 259 |  | `comum::impl::CValidaMidia::vf4` | `comum::impl::CValidaMidia::MidiaSemArquivosIndevidos` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 4) / low |
| 11172 | 2432 |  | `comum::impl::CValidaMidia::vf3` | `comum::impl::CValidaMidia::ValidaMidiaResultado` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp (+ cfabricaconteudomidiastart.cpp inlined, srcloc :62) | TSE (slot 3; name as in u09) / medium |
| 11173 | 47 |  | `comum::impl::CValidaMidia::vf2` | `comum::impl::CValidaMidia::MidiaResultadoValida` | uenux2/src/app/comum/validamidia/cvalidamidia.cpp | app/comum/validamidia/cvalidamidia.cpp | TSE (slot 2) / low |
| 11682 | 191 |  | `vota::CPedeMajoritario::vf15` | `vota::CPedeMajoritario::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (slot 15) / high |
| 11683 | 1210 | ✓ | `vota::CPedeMajoritario::ProcessInputAudio` | `vota::CPedeMajoritario::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (srcloc :64, :125) / high |
| 11684 | 103 | ✓ | `vota::CPedeMajoritario::vf10` | `vota::CPedeMajoritario::StartStateAudio` | uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | app/vota/eleitor/votamajoritario/cpedemajoritario.cpp | TSE (slot 10) / high |
| 11710 | 51 |  | `vota::CPedeProporcional::vf15` | `vota::CPedeProporcional::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (slot 15) / high |
| 11711 | 811 | ✓ | `vota::CPedeProporcional::ProcessInputAudio` | `vota::CPedeProporcional::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (srcloc :84) / high |
| 11712 | 103 | ✓ | `vota::CPedeProporcional::vf10` | `vota::CPedeProporcional::StartStateAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | app/vota/eleitor/votaproporcional/cpedeproporcional.cpp | TSE (slot 10) / high |
| 11744 | 433 |  | `vota::CPedeNulo::GetProximoEstado` | `vota::CPedeNulo::GetProximoEstado` | uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp | app/vota/eleitor/votaproporcional/cpedenulo.cpp | TSE (srcloc :44) / high |
| 11745 | 52 |  | `vota::CPedeNulo::vf15` | `vota::CPedeNulo::GetMensagemAudio` | uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp | app/vota/eleitor/votaproporcional/cpedenulo.cpp | TSE (slot 15) / high |
| 11756 | 786 | ✓ | `vota::CCompletaProporcional::vf9` | `vota::CCompletaProporcional::ProcessInputAudio` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (slot 9; __PRETTY_FUNCTION__ literal) / high |
| 11757 | 267 | ✓ | `vota::CCompletaProporcional::vf10` | `vota::CCompletaProporcional::StartStateAudio` | uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp | TSE (slot 10; __PRETTY_FUNCTION__ literal) / high |
| 11804 | 1437 |  | `vota::testeteclado::CTesteTeclado::ProcessInput` | `vota::testeteclado::CTesteTeclado::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | TSE (srcloc :50/:60/:81) / high |
| 11805 | 10638 |  | `vota::testeteclado::CTesteTeclado::StartState` | `vota::testeteclado::CTesteTeclado::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp (+ ctelasvota.u26.cpp GetKeyboardLayout) | TSE (srcloc :38/:42/:121) / high |
| 11806 | 13 |  | `vota::testeteclado::CTesteTeclado::vf1` | `vota::testeteclado::CTesteTeclado::~CTesteTeclado [deleting]` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp (comment) | compiler-generated (slot 1) / high |
| 11808 | 38 |  | `vota_f11808` | `vota::testeteclado::CTesteTeclado s_instancia at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp | app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h (comment) | compiler-generated / medium |
| 11823 | 67 |  | `vota::testeteclado::CEsperaRetestar::vf9` | `vota::testeteclado::CEsperaRetestar::ProcessTickNaoDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred) | app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp | TSE (slot 9) / medium |
| 11826 | 483 |  | `vota_f11826` | `vota::testeteclado::TextoEsperaRetestar` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (path inferred) | app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.u26.cpp | TSE (text source, table slot 1862) / low |
| 11842 | 5 |  | `vota::CRegerarZeresima::vf9` | `vota::CRegerarZeresima::AposImpressao` | uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (slot 9) / low |
| 11861 | 70 |  | `vota::testeteclado::CPreZeresima::vf12` | `vota::testeteclado::CPreZeresima::GetEstadoSemTeste` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (slot 12) / medium |
| 11862 | 107 |  | `vota::testeteclado::CPreZeresima::vf10` | `vota::testeteclado::CPreZeresima::CriaTela` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp | TSE (slot 10) / medium |
| 11868 | 13 |  | `vota::CVerificaHorarioZeresima::vf5` | `vota::CVerificaHorarioZeresima::FinishState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (slot 5) / high |
| 11869 | 861 |  | `vota::CVerificaHorarioZeresima::vf7` | `vota::CVerificaHorarioZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (slot 7) / high |
| 11870 | 124 |  | `vota::CVerificaHorarioZeresima::ProcessTickNaoDesligamento` | `vota::CVerificaHorarioZeresima::ProcessTickNaoDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (srcloc :80) / high |
| 11871 | 312 |  | `vota::CVerificaHorarioZeresima::StartState` | `vota::CVerificaHorarioZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp | TSE (srcloc :64) / high |
| 11908 | 4670 |  | `vota::CImpressaoListaEleitores::vf2` | `vota::CImpressaoListaEleitores::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaolistaeleitores.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp (condensed) | TSE (slot 2) / medium |
| 11911 | 565 |  | `vota::CImpressaoEstadoUrna::vf2` | `vota::CImpressaoEstadoUrna::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaoestadourna.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2) / high |
| 11927 | 337 |  | `vota::CConfirmaImpressaoZeresima::vf7` | `vota::CConfirmaImpressaoZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaimpressaozeresima.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (slot 7) / high |
| 11987 | 70 |  | `vota::CInicioVotacao::vf8` | `vota::CInicioVotacao::ProcessTick` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (slot 8) / medium |
| 11988 | 117 |  | `vota::CInicioVotacao::vf2` | `vota::CInicioVotacao::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (path inferred) | app/vota/u26-foreign-fragments.cpp | TSE (slot 2) / medium |
| 13012 | 475 |  | `vota_f13012` | `vota::(anonymous)::TextoSegundosParaDesligamento` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | app/vota/eleitor/comum/ctelasvota.u26.cpp | TSE (text source, table slot 1102) / low |

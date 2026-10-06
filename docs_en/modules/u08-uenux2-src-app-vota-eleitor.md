# u08 — `uenux2/src/app/vota/eleitor`: voter audio and the BU / encerramento states

Unit u08 covers 107 wasm functions attributed to five original files of the VOTA application
(the urna's election-day program):

| original file | what it is |
|---|---|
| `uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp` | `vota::CVotacaoStateAudio`, the base class of every per-cargo voting screen. It speaks messages to blind or low-vision voters, repeats them and echoes keys |
| `uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp` | `vota::CGeraBU`: renders the **Boletim de Urna** (BU) report at the close of voting |
| `uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp` | `vota::CGeraRelatorios`: renders the other end-of-day reports (BUJ, BIM, BEHB) |
| `uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp` | `vota::CCopiaResultadoParaMR`: copies the result files to the result USB stick (MR) and verifies the copy |
| `uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp` | `vota::CEmitirMaisBU`: asks the poll worker how many extra BU copies to print |

The unit also received helper functions from other files: `comum/relatorios` report helpers,
`CLogVota` log methods, `CInformacaoEleitor::GetInst`, at-exit destructors, and **50 libc++
`<regex>` template instances** used by the audio code (and shared with four other modules, §8). Every one of them is listed in
[§11](#11-complete-mapping-table-107-functions).

Reconstructed sources (legible C++, one comment `// wasm func N` per function):

* `src/uenux2/src/app/vota/eleitor/cvotacaostateaudio.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.{h,cpp}`
* `src/uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.{h,cpp}`

## 1. Glossary

| term | meaning |
|---|---|
| eleitor | voter. "Eleitor" code runs on the voter terminal (the urna itself) |
| mesário / operador | poll worker, who works at the *terminal do mesário* |
| cargo, escolha, vaga | office being voted for, one of the choices for it (e.g. two Senate seats), seat |
| BU (Boletim de Urna) | per-section result: votes per candidate, party, blank and null, plus turnout. Printed, signed, written to the result media and published as QR codes |
| BUJ, BIM, BEHB | Boletim de Justificativa (voters who justified absence), Boletim de Identificação de Mesários (poll workers present), relatório de Eleitores com Habilitação Biográfica (voters admitted without fingerprint match) |
| vias | printed copies of a report |
| zerésima | report printed before voting that shows all counters at zero |
| encerramento | closing of the voting |
| MI / MR / MV | memória interna (internal flash) / mídia de resultado (result USB stick, `/dsk/mr/`) / memória de votação (flash card) |
| RDV | Registro Digital do Voto, the shuffled record of every vote |
| CV (código verificador) | check code printed on the BU, computed by `comum::CCalculaCV` with the key `cv.ber.pri` |
| carga, correspondência | the software/data load of the urna, and the record of that load (`DadoCorrespondencia`: carga + section + signature) |
| fase | `o`ficial / `s`imulado / `t`reinamento; `EUrnaFase` '1' simulado, '2' oficial, '3' treinamento |
| tecla indevida | a key that is not allowed on the current screen |

## 2. Classes and hierarchy (RTTI)

```
api::CState
 └─ comum::CAppState                         (+4 m_proximoEstado, +8/+9/+10 flags: messages|keyboard|ticks)
     ├─ vota::CVotacaoStateAudio             typeinfo @1535192, vtable @1534936 (16 slots)   abstract
     │    ├─ CInstrucaoVotacaoAcessibilidade, CConfirmaVotoSemCandidato, CPedeMajoritario,
     │    │  CPedeProporcional
     │    ├─ CCompletaProporcional ─ CPedeNominal, CPedeNulo
     │    ├─ CConfirmaVotoEmCargo
     │    │    ├─ CConfirmaProporcional ─ CCandidatoInapto, CConfirmaVotoLegenda, CCandidatoInexistente,
     │    │    │                          CConfirmaVotoNominal, CProporcionalBranco, CProporcionalNulo
     │    │    └─ CConfirmaMajoritario  ─ CMajoritarioBranco, CMajoritarioNulo, CMajoritarioRepetido,
     │    │                               CMajoritarioValido                     (other units)
     │    └─ IConfereVotoEmCargo ─ CConfereVotoEmCargo<TConfirma, ETelaVotacao>  (unit u06)
     ├─ vota::CGeraBU                        typeinfo @1540424, vtable @1540372, 28 bytes
     ├─ vota::CGeraRelatorios                typeinfo @1540576, vtable @1540524, 20 bytes
     ├─ vota::CCopiaResultadoParaMR          typeinfo @1540212, vtable @1540080, 20 bytes
     └─ vota::CEmitirMaisBU                  typeinfo @1541264, vtable @1541180, 12 bytes
```

The `api::CState` slot order is recovered from classes that have `source_location` records: [0] complete
dtor, [1] deleting dtor, [2] `StartState`, [3] `NeedChangeState` (`CAppState` = `GetNextState() != this`,
func 7480), [4] `GetNextState` (returns `+4`), [5] `FinishState`, [6] `ProcessMessage`, [7]
`ProcessInput`, [8] `ProcessTick(uebyte)`. `CVotacaoStateAudio` adds [9] `ProcessInputAudio`=0,
[10] `StartStateAudio`=0, [11] `FinishStateAudio` (no-op), [12] `ProcessTickAudio`=0, [13]
`EmiteEcoComInputField`, [14] `FormataMensagem`, [15] `GetMensagemAudio`=0. The names of 10, 11, 12, 15
come from the subclass sources of unit u06 (srcloc `CInstrucaoVotacaoAcessibilidade::StartStateAudio`,
the lambda type `IConfereVotoEmCargo::ProcessTickAudio(unsigned char)::$_0`). The name of 14 is inferred.

All four encerramento states are lazy singletons. Their `GetInst` functions (6129, 6084, 6174, 3879)
belong to other units, and their constructors are inlined there. `CGeraBU` holds two
`shared_ptr<CPreShowFormVota>` screens: "Votação encerrada" (`telaVotacaoEncerrada`) and "Preparando
dados para encerramento" (`telaPreparandoDadosEncerramento`). `CGeraRelatorios` holds the second one.
`CCopiaResultadoParaMR` holds "Gravando o resultado na mídia" (`telaCopiaResultadoParaMR`).

## 3. `CVotacaoStateAudio`: the voter-audio state machine

Layout (28 bytes): `+11 bool m_audioHabilitado`, `+12 uebyte m_tickRepeticao` (2000 ms tick of
`CThreadEleitor`), `+13 uebyte m_tickInicio` (1500 ms), `+16 unique_ptr<?>` (only destroyed),
`+20 shared_ptr<api::IEsperaAudio> m_esperaFimAudio`. The constructor is func 1785, which the tools
placed in `celeitorvotando.cpp`.

```
StartState (7028)   m_audioHabilitado = CInformacaoEleitor::m_modoAudio (+4) != 2 (SemAudio)
                    StartStateAudio()                         [subclass: draws the screen]
                    audio ? StartTick(m_tickInicio 1.5 s)
ProcessTick (7010)  tick is m_tickInicio or m_tickRepeticao?
                      no  -> ProcessTickAudio(tick)           [subclass]
                      yes -> ISound::GetStatus()==1 (playing)?  -> return (try again next tick)
                             cancel wait, stop both ticks
                             audio ? PlayInterruptibleMessage(FormataMensagem(GetMensagemAudio()))
                             IniciarEsperaFimAudio(): m_esperaFimAudio = ISound::EsperaFimAudio(
                                 []{return false;}, [this](bool fim){ fim && audio -> StartTick(m_tickRepeticao) })
ProcessInput (7029) audio ? {cancel wait; stop ticks; ISound::Stop(); stop ticks}
                    ProcessInputAudio()                        [subclass: reads the key, echoes it]
                    GetNextState()==this && audio -> StartTick(m_tickRepeticao)
FinishState (7022)  cancel wait; stop ticks; FinishStateAudio()
```

So a state speaks its message 1.5 s after it is entered. After each utterance ends it waits 2 s
and repeats the message, indefinitely, until a key is pressed (the key stops the audio and restarts the
2 s timer) or the state is left. The repetition can be heard with
`node tools/run/headless.mjs --scenario municipal-t1 --audio --save-audio DIR --keys "  C  91001  C    12  C    "`.
With the audio on, the first screen is `CInstrucaoVotacaoAcessibilidade`, which only leaves on CONFIRMA,
hence the first `C` (without it, as in the earlier version of this command, the digits `91001` are pressed on
the instruction screen and ignored, and `12` ends up in the Vereador field). The run (re-checked 2026-09-23)
goes CPedeProporcional → CConfirmaVotoNominal → CPedeMajoritario → CMajoritarioNulo → CAguardaMensagem
(done) and produced 20 WAVs, with identical consecutive files for the repeated messages (for example
`speech-012/013/014`, 153 484 bytes each: the CPedeMajoritario prompt for Prefeito, repeated during the 6 s
pause). No abort and no exception occurred.

**Key echo** (`PlayKey`, func 1455, srcloc 136/226):

* key 0 (refused): `IBeep::BeepErro()` (slot 6 = 554 Hz for 200 ms in `CWasmBeep`) and log
  `"Tecla indevida pressionada"`. This happens even when audio is off, which is why the web log
  `logd.dat` contains this line.
* with audio: digits, `B` (BRANCO) and `D` (CORRIGE) speak `api::KeyName(key)` and can be interrupted.
  `C` (CONFIRMA) is queued and waited for (`PlayMessage`). A refused key plays the file
  `:/resource/sounds/tecE.wav` (`PlayFile`).

**Which key is refused** (`EmiteEcoComInputField`, func 3139, srcloc 188): the form must have exactly one
input field (otherwise `CBaseError<EUeVotaError>` 9318 "Nao havia campos de input no formulario sem nada
digitado"). The field is read (`CInteractiveForm::Read`, inlined). If the result is CORRIGE (5) or a
typed key (13) **and the text did not change**, the key is refused. That covers a digit when the field is
already full, and CORRIGE on an empty field. The function returns `{result, text}`.
`EmiteEcoCorrigeConfirma` (srcloc 209, inlined into func 11754) is used on the CONFIRMA/CORRIGE screens.
There, any key other than `C`/`D` is refused.

**Message templates** (`GetMensagemAudio`, slot 15, in the subclasses; the strings are Latin-1 because
RHVoice reads Latin-1):

| state (slot 15 func) | template |
|---|---|
| CPedeProporcional (11710), CPedeMajoritario (11682) | `Você está votando para {cargo-atual}. {quantidade-digitos}. Voto {progresso}.` / `{cargo-atual} {quantidade-digitos}. Voto {progresso}.` |
| CPedeNominal (11725) | `… Digite os demais dígitos do seu candidato, ou aperte confirma para prosseguir, ou corrige para reiniciar este voto.` |
| CConfirmaVotoNominal (11728) | `Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Aperte confirma ou corrige.` |
| CMajoritarioValido (11687) | `… {voto}: {candidato-com-suplentes}. …` and, for a referendum, `Você está respondendo à pergunta: {cargo-atual}. Você escolheu a opção {voto}: {escolha-referendo}. …` |
| CConfirmaVotoLegenda (11735), CCandidatoInexistente (11731) | `… no número {voto}. [Candidato inexistente.] Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige …` |
| CPedeNulo (11745), CMajoritarioNulo/CProporcionalNulo (11693/11701) | `… no candidato {voto}. Número errado. [Se apertar confirma, este voto será nulo.] …` |
| CCandidatoInapto (11748), CMajoritarioRepetido (11690) | `… Candidat{o/a} não concorre …` / `… já foi escolhid{o/a} em voto anterior …`, `Você já votou na opção "{escolha-referendo}". …` |
| CMajoritarioBranco/CProporcionalBranco (11696/11704) | `Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige.` |
| CConfirmaVotoSemCandidato (7425) | `Você está votando para {cargo-atual}, mas não há candidato para o cargo` |

**Tag substitution** (`FormataMensagem`, func 6999): a `std::sregex_token_iterator` with pattern
`\{.+?\}` and sub-matches `{-1, 0}` walks the text between tags and the tags themselves:

| tag | value |
|---|---|
| `{cargo-atual}` | cargo name (for a referendum, the consulta's audio name if it has one), plus `" {n}ª vaga"` (or `"escolha"` for a consulta) when the cargo has ≥ 2 choices. `n` = `g_numeroEscolha` @1536340 |
| `{voto}` | the typed number `g_votoDigitado` (@1833288), digits separated for the TTS: `regex_replace(s, "\B\d", ", $&")`, so "123" becomes "1, 2, 3" |
| `{legenda}` | the first 2 typed digits, separated in the same way |
| `{nome-partido}` | `FormatPartyName(first 2 digits)`: `atoi`, lookup in `CPartidos`, throws 9321 "Nao foi encontrado o partido" after logging "Erro partido não encontrado" |
| `{escolha-referendo}` | the current `CRespostas` answer (audio name at +16, or else the description at +4) |
| `{o/a}` | `"a"` when the current candidacy has sex code 2, otherwise `"o"` |
| `{candidato}` | the candidate's optional audio name (+32/+44), or else the ballot name (+20) |
| `{candidato-com-suplentes}` | the same, plus for each substitute `", " + CCargoDSNomeSexoCandidato{i, true}() + " " + name` |
| `{quantidade-digitos}` | `"{} dígitos"` / `"{} dígito"` |
| `{progresso}` | `"{n} de {total}"`, where n = votes confirmed + 1 and total = the sum of the choices of all cargos |

## 4. The encerramento chain (fim da votação)

`EEstadoVota` = ASN.1 `EstadoGeralVota.estadoVota` + 49 and `EEstadoEncerramento` = `estadoEncerramento` + 49
(`src/asn1/ModuloEstadoGeralVota.asn`). The state is persisted in `vota.bin` by `comum::SalvaEstado()`
(func 491, "Gravando o estado da urna").

```
EAVGERARBU (59)          CGeraBU            BU report -> trab/bu.dat            -> 60
EAVGERARRELATORIOS (60)  CGeraRelatorios    buj.dat, [bim.dat], [behb.dat]      -> 61, next CInicioBU
EAVIMPRIMIRBU (61)       CInicioBU / CQuerImprimirBU / CImprimindoBU  (other units: print the BU vias)
EAVGRAVARRESULTADOS (62) CGravaResultado    (other unit: ModuloBoletimUrna BU, RDV, hashes, signatures)
EAVCOPIARESULTADOSMR(63) CCopiaResultadoParaMR  copy to /dsk/mr/, verify       -> 64 + encerramento 50
EAVENCERRADA (64)        CImprimirBUOutrasObrigatorias (EAEIMPRIMIROBRIGATORIABU 50) -> CRetirarMR (EAERETIRAMR 51)
                         -> CVerificaQtdBUsAdicionais (EAEFIMDOSTRABALHOS 52) -> CEmitirMaisBU -> CMostraQRCodeBU …
```

Each report step logs `"Gerando relatório [<X>] [INÍCIO]"` and `"… [TÉRMINO]"` (func 1047,
`IEventosLog::ConverteERelatoriosUE`: 3 = `BU`, 4 = `BUJ`, 9 = `BIM`, 12 = `ELEITORES HABILITADOS
BIOGRAFICAMENTE`). Each report is written inside an `api::CApplicationContextGuard(2, "", "Gerando … na
MI", "Ocorreu um erro durante a geração … na MI.")`, which supplies the error screen text if an exception
escapes. The guard covers only the writing: the BU generator (with `CCalculaCV` and `LeChave`) and the BUJ
generator are constructed *before* it, so an exception raised while building them (8090/8091, 9051–9053, a
missing key file) escapes without that context. After the guard is destroyed `CGeraBU` saves the state and
mirrors `bu.dat`; `CGeraRelatorios` mirrors each file while the guard is still alive and saves the state once
at the end.

### 4.1 `CGeraRelatorios::StartState` (func 12105)

1. `Assert(GetEstadoVota() == EAVGERARRELATORIOS)` (srcloc 51, EUeAssertError 3455), then the "Preparando dados…" screen.
2. **BUJ** (always). `comum::CGeradorBUJ` builds four parts, then `GeraBUJ(path)` (cgeradorbuj.cpp:189)
   writes the file:
   * the header: "Boletim de Justificativa Eleitoral" + `IncluiCabecalhoEleicoesMZS` + opening/closing
     dates + summary of the correspondência;
   * the count: `"{:05} Justificativas"`;
   * one detail block per entry of the justificativa `CDataMap` (`Next()` past the end throws
     "Operação inválida {}", EUeIoError 5977);
   * the trailer: load code, `"Ver: 10.23.0.1"`, "ASSINATURAS:", "PRESIDENTE:", "MESÁRIOS:", "FISCAIS:", cut.

   The file is `buj.dat`.
3. **BIM** (when func 1950 returns true: not demo mode, and the flag at `CConfiguracaoEleicao+488` is
   set). `CImprimirIdentificacaoMesariosFinal(final)` is inlined. It prints "Mesários registrados na
   abertura/no fechamento" from the SQLite table `comparecimento_mesario` (DAO func 815), with "Nome:" and
   "Assinatura:" lines. The file is `bim.dat`.
4. **BEHB** (when not in demo mode and `CLocal::UrnaBiometrica()`). "Eleitores com habilitação
   biográfica": for every section (main section, "Seção agregada: {:04}", "Transferência temporária"),
   the voters who were admitted biographically, collected in a `map<section, vector<{número, nome}>>`,
   each vector sorted with `std::sort`. The file is `behb.dat`.
5. `SetEstadoVota(EAVIMPRIMIRBU)`, `SalvaEstado()`, next state `CInicioBU` (func 5983).

Each file is written to `CPath::GetPathTrab(INTERNA)/<name>` and mirrored by
`CSincronizaVota::SincronizaRelatorios(name)`.

### 4.2 `CCopiaResultadoParaMR::StartState` (func 12134)

1. `CApplicationContextGuard(10, "Copiando resultado para MR", "Erro de gravação", "Ocorreu um erro durante
   a cópia do resultado para a MR.")`, then `Assert(EAVCOPIARESULTADOSMR)` (srcloc 59, 3451), then the screen.
2. In demo mode (`IInterfaceInit::GetDemoMode()`), nothing is copied.
3. Otherwise:
   * `EnviarMensagemThrowVoid(10, "habilitando MR")` followed by a 500 ms sleep (func 2863);
   * the MR must be present (9367, srcloc 94);
   * it is remounted;
   * if `/dsk/mr/` is not empty, `IValidaMidia::ValidaMidiaResultado(device, turno)` must accept it
     (9368, srcloc 112, plus syslog "mídia inválida para gravação pelo motivo [%d]");
   * outside training, the MR must not already contain results (9369, srcloc 126).
4. Copy with `comum::CCopiadorMR` (9 files) and `CCopiadorWSQMR` (3 files, biometric urnas only), with an
   `fsync` (`CSynchronizer`) after each. The names come from
   `DeterminaNomeArquivoSemLetra(mun, zona, secao, fase, tipo)` =
   `format("{:c}{:05}{}{:05}{:04}{:04}-", fase, pleito, <CLocal string>, mun, zona, secao) + CArquivosResultado[tipo]`:

   | tipo | suffix | | tipo | suffix |
   |---|---|---|---|---|
   | 4 | `bu.dat` | | 12 | `hash.dat` |
   | 6 | `rdv.dat` | | 13 | `log.jez` |
   | 8 | `jufa.dat` | | 1 | `vota.vsc` |
   | 9 | `imgbu.dat` | | 19 | `mr.ver` |
   | 11 | `imgze.dat` | | 16/17/18 | `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` |

5. Verification: **only the BU file** (tipo 4) is compared between `GetPathResult(0)` and `/dsk/mr/`,
   with an inlined `api::CSystem::AreFilesEqual` (same size, then `fread` in 32 KiB chunks into two static
   buffers and `memcmp`). A mismatch logs "Erro na cópia dos resultados para MR" and throws 9370
   "Nao copiou" (srcloc 174). On success the MR is unmounted and "Mídia de resultado gravada" is logged.
6. `EstadoVota = EAVENCERRADA (64)`, `EstadoEncerramento = EAEIMPRIMIROBRIGATORIABU (50)`, `SalvaEstado()`,
   next state `CImprimirBUOutrasObrigatorias`.

### 4.3 `CEmitirMaisBU` (funcs 12082 / 12081)

`StartState` asserts `EAVENCERRADA` (64, srcloc :46) and `EAEFIMDOSTRABALHOS` (52, srcloc :47), logs "Mesário indagado sobre
quantidade de vias adicionais" and shows `telaEmitirMaisBU`. In `ProcessInput`:

* CORRIGE: log "Mesário não solicitou emissão de vias adicionais", then go to `CMostraQRCodeBU`.
* CONFIRMA:
  * An empty field logs the same message and keeps asking.
  * Otherwise n = `CStringUtils::ToByte(text)` and the log gets `"Solicitada a emissão de [n] via(s)
    adicional(is)"`.
  * If n > `GetQuantidadeMaximaBUsAdicionais()`, it logs "Quantidade de vias adicionais excede o máximo
    permitido", shows the limit screen, sleeps 3 s and asks again.
  * Otherwise it prints n copies: for each, `ImprimeBU(qtdBU+1, false)`, `++qtdBU`, `SalvaEstado()`. It
    stops early if the urna is shutting down (flag @1832936).
* Next state: `CMostraQRCodeBU`.

## 5. BOLETIM DE URNA: what `CGeraBU` does, step by step

`CGeraBU::StartState` (func 12110) is the only place in the binary where the construction of the printed
BU (`comum::CGeradorBU`, `cgeradorbu.cpp`) and its generation loop (`CGeradorRelBase::GeraRelatorio`,
`cgeradorrelbase.h:61`) exist, because both are inlined. Sequence:

1. Show "Votação encerrada". `Assert(appInfo.GetVota().GetEstadoVota() == EAVGERARBU)` (cgerabu.cpp:54,
   EUeAssertError 3454). Wait 1 s (see §9.1). Show "Preparando dados para encerramento". Log
   `Gerando relatório [BU] [INÍCIO]`.
2. `CCargos::Inicio()`. `separadorFase = CRelUtil::GetSeparadorFase(EstadoGeral.fase)`:
   `==============TREINAMENTO=============`, `===============SIMULADO===============`, 38 `=` for
   oficial, or `============DEMONSTRAÇÃO==============` in demo mode.
3. **Emission time.** If `EstadoGeralVota.dhEmissao` (+80, optional) is unset, it is set to
   `CDateTime::Now()`. It is read back through `GetDtHrEmissaoBU()`, which throws 8093 "A data/hora da
   emissão do BU não foi registrada".
4. **Load history.** `GetGap()` returns the `codigoCarga` of every `EstadoGeralGap.correspondencias`
   entry (gap.bin) as a `vector<string>` (func 3788).
5. **Build the 13 parts of the report** (`CGeradorBU` ctor → `CGeradorBUBase` → `CGeradorRelBase`, each
   part checked non-null, error codes 9055–9067):
   * **header** (`CriaHeader`):
     * `CRelUtil::IncluiCabecalhoEleicoesMZS(title "Boletim de Urna", município, zona, seção, nome do
       município)`;
     * `Eleitores aptos {:04}` (aptos + aptos of the other category, counted from `CEleitores`);
     * `Comparecimento {:04}` = `CRdvVota::Comparecimento()` (the maximum over the eleições);
     * on biometric urnas: `Habilitação biométrica / biográfica / sem biometria` counts;
     * `Eleitores faltosos` = aptos − comparecimento;
     * `Código identificação UE {:08}` (= `carga.numeroInternoUrna`);
     * if voting took place, the opening/closing date and time (`dhIniAquisicao`/`dhFimAquisicao`, 8090/8091
       when missing);
     * "RESUMO DA CORRESPONDÊNCIA".
   * **trailer**:
     * `separadorFase`;
     * "Código de identificação da carga" + `CRelUtil::GetIDCargaFormatado(codigoCarga)` (func 2786: seven
       groups of 3 and the rest, joined by `.`; "ID de carga inválido" when shorter than 21 characters);
     * `Ver: 10.23.0.1` (from "10.23.0.1 - DESENVOLVIMENTO");
     * a configuration text whose `<uf>` is replaced by the UF;
     * 20 blank lines and a paper cut.
   * **qrcode** (`CriaQRCode`, cgeradorbu.cpp:124):
     * demo mode: "DEMONSTRAÇÃO PRÉ-ELEIÇÃO" / "NÃO HÁ QR CODE";
     * when the configuration bit `CConfiguracaoEleicao+486 & 1` is set:
       `CGeradorBUQRCode(zona, seção, numeroInternoUrna, codigoCarga, histórico; comparecimento,
       dhEmissao)` produces up to 1100 bytes per QR (func 5604, other unit). The payloads start with
       `QRBU:{i}:{n} VRQR:… ` and hold `KEY:value` fields (PROC, PLEI, TURN, FASE, UNFE, ZONA/SECA,
       IDUE, IDCA/HASH, CARG, TIPO, NOMI, LEGP/LEGC, BRAN, NULO, TOTC, TOTP, … ` HASH:{}`, ` ASSI:{}`).
       They are printed under `============= BU DIGITAL =============`, each preceded by
       `-------------- {i:02} / {n:02} ---------------`. The printout continues with
       `======== CERTIFICADO DIGITAL =========` (the urna certificate in hex, split into
       ⌈2·bytes/1082⌉ QR codes with the fields `QRCE:i:n IDUE:… MDUE:… CERT:…`), then
       `ASSINATURA BU DIGITAL: <signature>` if a signature was returned;
     * otherwise no QR part.
   * **per-cargo parts**: `headerProporcional` (separator + cargo name padded with `-` to 38 columns),
     `headerProporcionalPartido`, `partidoApenasVotoLegenda` ("Não há votos nominais"),
     `trailerProporcionalPartido`, `cargoSemCandidato` ("Não há candidatos concorrendo"),
     `trailerCargoSemCandidato`, `trailerProporcional`, `headerMajoritario`, `detalheCandidato`,
     `trailerMajoritario`. They are made of `CDataText` fields (ids 1625, 1626, 1632, 1633, 1635/1636,
     2920–2923) that the generator's data sources fill with the RDV counts at print time.
6. **Código verificador.** `comum::CCalculaCV` (56 bytes) is created and published as a poly-singleton
   (`CPolySingletonList::push<CCalculaCV>`, error 6756 if it already exists):
   * *identificação* = `format("{:05}{:04}{:04}{}{}{}{}", município, zona, seção,
     format("{:05}{:05}{}", cfg[+0], pleito, dataPleito "YYYYMMDD"), numeroInternoUrna, pleito, fase-char)`.
     The nested string is the **4th** argument (packed arg types `6,6,6,13,6,6,2`: three unsigned, a
     string_view, two unsigned, a char). `pleito` is `CConfiguracaoEleicao+28`, `dataPleito` is `+44`. It must be at
     least 13 characters from `0123456789sodtSODT` (9051 "Identificação inválida [..]").
   * first CV = `"0000000000"`, which must be decimal (9052).
   * tipo = `DefineTipo(demo)`: `'A'` in demo mode, `'F'` otherwise (cgeradorbu.cpp:161).
   * key = `comum::util::LeChave("/dsk/fi/estatico/chave/cv.ber.pri")` (util.cpp:37). This reads an
     ASN.1 `EntidadeChave` envelope and `CKeyLoader::DecipherKeyIfNeeded` deciphers it. The key must be at
     least 16 bytes (9053 "Chave privada com tamanho pequeno…") and the first 16 bytes are kept.
   * state `buffer = tipo + identificação + cv` (ecourna_f5615). The BU data sources then append every
     printed value with `CCalculaCV::IncluiString` (ccalculacv.cpp:109, other unit), and the CV printed on
     the BU is a MAC of that buffer under the key (unit u25 / relatórios).
7. **Write the report.** `CApplicationContextGuard(2, "", "Gerando boletim de urna na MI", "Ocorreu um erro
   durante a geração do boletim de urna na MI.")`. Then `GeraRelatorio(GetPathTrab(INTERNA)/"bu.dat")`
   runs:
   * `IPaperRelatorios::Abre(path)`, header, `CGeradorBU::ImprimePreTexto()`;
   * the cargo loop. When the list contains more than one eleição, a `======` / eleição name / `======`
     banner precedes the cargos of each eleição (the first comparison is against the eleição of the
     *last* cargo). Each cargo then goes to `ImprimeProporcionalPartido` (proportional eleição cargo),
     `ImprimeMajoritario` (majoritarian cargo) or `ImprimeConsulta` (referendum);
   * the QR part (if any), the trailer, `Fecha()`, `CSynchronizer`.
8. `SalvaEstado()`, `CSincronizaVota::SincronizaRelatorios("bu.dat")`, `~CGeradorBU`, log `[BU]
   [TÉRMINO]`, `SetEstadoVota(EAVGERARRELATORIOS)`, `SalvaEstado()`, next state `CGeraRelatorios`.

What happens later, in other units: `CImprimindoBU::ImprimeBU(via, …)` prints the vias,
`CGravaResultado` (EAVGRAVARRESULTADOS) builds and signs the ASN.1 `ModuloBoletimUrna` BU and the other
result files, `CCopiaResultadoParaMR` (§4.2) copies `…-bu.dat`, `…-imgbu.dat`, `…-rdv.dat`, … to the MR,
and `CMostraQRCodeBU` / `CMostraQRCodeCertificado` show the QR codes on screen. For the screen,
`CMostraQRCodeCertificado` uses func 12039 (`MontaQRCodesCertificado(estado, false)`, one QR, then
`MontaConteudoQRCode`).

In the simulator the BU cannot be generated. The emscripten_sleep of step 1 would abort first (§9.1), and
the key `/dsk/fi/estatico/chave/cv.ber.pri` does not exist in any scenario of `upstream/fs/`.

## 6. Data read and written

| data | access |
|---|---|
| `vota.bin` (`EstadoGeralVota`: estadoVota, estadoEncerramento, qtdBU +8, dhIni/FimAquisicao, dhEmissao +80) | read/write via `CAppInfo::GetVota()` + `SalvaEstado()` |
| `EstadoGeralUrna` (fase +48, município/zona/seção +20..+26, correspondência +60 with numeroInternoUrna and codigoCarga +28), `gap.bin` (correspondências) | read |
| `/dsk/fi/estatico/chave/cv.ber.pri` (EntidadeChave) | read (CV key) |
| `uenux.db` table `comparecimento_mesario` | read (BIM) |
| `CPath::GetPathTrab(INTERNA)/{bu,buj,bim,behb}.dat` | written (text reports) |
| `CPath::GetPathResult(0)/<prefix>-<suffix>` → `/dsk/mr/<same>` | copied and verified (BU only) |
| `dinamico/log/logd.dat` | log lines (Latin-1) |
| certificate from `CEstadoGeral::RecuperarCertificado()` | read (QR "CERTIFICADO DIGITAL") |

## 7. Web-build specifics

* The voter audio uses `simulador::CWasmWebSound` and JS Web Audio. `PlayInterruptibleMessage` passes
  mode 0 to `js_wasm_web_sound_play_wav`, which stops and clears the queue, then plays. `PlayMessage`
  passes mode 1, which enqueues. `ISound::Wait` (func 9523) **does not block** in the web build, so
  `PlayMessage` returns at once, whereas on the urna it holds the voter thread until the message ends.
  `ISound::PlayFile` (func 9580) ignores the file, so `tecE.wav` is never played; the beep from
  `CWasmBeep` still sounds.
* The page sets `CInformacaoEleitor::m_modoAudio` through `votaInit` (`audioEleitorHabilitado`) and
  `votaSetAudioEnabled`. With audio off (the recorded sessions), only `StartState`, `FinishState`,
  `ProcessInput`, `ProcessTick`, `PlayKey`, `EmiteEcoComInputField` and `EmiteEcoCorrigeConfirma` run.
  These are the 7 observed functions.
* The encerramento classes are compiled in unchanged, but no recorded session reaches them.

## 8. wasm / Emscripten observations

* **LTO inlining** makes `CGeraBU::StartState` (17 KB) and `CGeraRelatorios::StartState` (18.8 KB) the
  only copies of large parts of `comum/relatorios` (CGeradorBU, CGeradorBUJ, CImprimirIdentificacaoMesariosFinal,
  CCalculaCV ctor, LeChave, CPolySingletonList::push). `CopiaResultado` and `EmiteEcoCorrigeConfirma`
  exist only inlined.
* **Misnamed by the tools** when this chapter was written (the database now shows the corrected names):
  * func 11754 ("CVotacaoStateAudio::EmiteEcoCorrigeConfirma") is `CConfirmaVotoEmCargo::ProcessInputAudio`;
  * func 12134 ("CopiaResultado") is `StartState`;
  * func 1950 ("EhModoDemonstracao") is a caller that inlines it: it returns `!EhModoDemonstracao() &&
    CConfiguracaoEleicao+488` (the BIM condition). Func 2286, which carries the same srcloc
    (cinformacaoeleicao.cpp:40), really is `CInformacaoEleicao::EhModoDemonstracao()` (it only returns
    `IInterfaceInit::GetDemoMode()`; the unused `this` was removed);
  * func 5604 ("CCargos::GetCurrentEleicaoVersaoPacote") is the BU QR generator;
  * func 7787 (formerly shown by the tools as "CHKDFSeed::GetSeed") is the start-up function,
    `vota::CInformacaoEleitor::Inicializar` (name inferred, u02).
* **ICF:** `~CVotacaoStateAudio` (1035) is the destructor of every subclass that adds no member. The
  lambda `IniciarEsperaFimAudio()::$_0` folds into `icf_ret_0_vf6` (return false). The eleven at-exit
  thunks of this unit (7479, 11689…11737) share body 906, which four thunks of other units (11686, 11727,
  11747, 11750) also call.
* **merge-similar-functions:** the CLogVota message helpers (4556, 5885) pass pointers into the middle of
  the same literal to a shared body (`api_f6115`, `vota_f3902`).
* **libc++ `<regex>`**: the tools gave this unit 50 `std::basic_regex<char>` template instances (25.5 KB of
  code) because `FormataMensagem` calls them. They are not used only by `FormataMensagem`: the same instances
  are also called by `api::CInputMenuField` (func 5492, pattern `%[ST]`), `CThreadMonitor` (7709),
  ecourna `CSerialMidia` (9258, `([A-Fa-f0-9]{8})`) and `CRelatorioTesteImpressora` (5782). Boost.Regex is
  linked as well (TSE helper func 2199). The regex constants are ABI-v2 (`ECMAScript` = 512).
* **String encodings:** report and audio strings are Latin-1 in the data segment (`"Gerando relat\xF3rio"`),
  while other messages are UTF-8.
* Error codes are fixed per throw site (`CBaseError<EUeVotaError>` 9318, 9319, 9321, 9367–9371).

## 9. Suspicious / notable code

1. **Abort paths through `emscripten_sleep`.** Funcs 12110 (1000 ms), 12081 (3000 ms) and 2863 (500 ms, on
   the `CopiaResultado` path) call `emscripten_sleep` when the byte @1584624 is 1. That byte is
   initialised to 1 and nothing stores to it: a run of headless.mjs printed 1 after `votaInit` and after a
   vote. The glue implements `_emscripten_sleep` as `abort()` (no Asyncify), so reaching any of these
   states would kill the simulator. The web flow does not reach them.
2. **The BU cannot be produced in the simulator.** `cv.ber.pri` is missing (§5; no `chave/` directory in any
   scenario of `upstream/fs/`), so `LeChave` would throw before any report is written, even if the sleep were
   removed, and outside the "Gerando boletim de urna na MI" error context.
3. **Only the BU file is verified after the MR copy** (§4.2). A truncated `rdv.dat`, `log.jez` or `.vsc` on
   the stick is not detected here, unless `CCopiadorMR::Copia` throws on its own.
4. **Audio behaves differently on the web** (§7). `PlayMessage` does not wait and `PlayFile` is silent.
   Blind voters in the simulator hear a different timing from the real urna. Informational.
5. **`FormatPartyName` throws inside `ProcessTick`** if the typed prefix is not a party (9321). The
   templates that use `{nome-partido}` (legenda and inexistent-candidate confirmations) are only reached
   with a valid party, so this is defensive. Checked with headless.mjs `--audio` in municipal-t1: `91999`
   (party 91 exists, no candidate) reaches `CCandidatoInexistente` and its message is spoken and repeated;
   `98999` (no party 98) goes to `CPedeNulo` / `CProporcionalNulo`, whose templates have no `{nome-partido}`.
   Neither run threw.
6. **Variable name in an assert:** the assert in cgerarelatorios.cpp:51 reads `poInfo.GetVota()`, while the
   other files of the chain read `appInfo.GetVota()`. The macro stringifies its argument, so the local
   variable in `CGeraRelatorios::StartState` is really called `poInfo` (the reconstruction keeps that name).
   Cosmetic.

## 10. Open questions

* The type held by `CVotacaoStateAudio+16` (a `unique_ptr` that is only destroyed).
* The meaning of the `CDataText` ids (1625…2961) used in the BU parts. They are resolved in `comum/relatorios` (unit u25).
* The exact names of the `ISound` / `ITextToSpeech` / `IEsperaAudio` / `IValidaMidia` slots. They are inferred from behaviour.
* The `CLocal` string inserted by `DeterminaNomeArquivoSemLetra` (the `{}` field) and `CConfiguracaoEleicao[+0]` used in the CV identificação.

## 11. Complete mapping table (107 functions)

Columns:

* **run**: observed executing in the recorded votes.
* **original file**: where the function lives in the TSE tree. Paths not attested by a srcloc are inferred.
* **reconstructed in**: the file under `src/uenux2/src/app/vota/eleitor/` that holds its reconstruction or description.
* **kind**: TSE code, compiler-generated, or library.

| idx | size | run | tools name | reconstructed symbol | original file | reconstructed in | kind / conf. |
|---|---|---|---|---|---|---|---|
| 509 | 102 |  | `vota_f509` | `vota::CInformacaoEleitor::GetInst` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | cvotacaostateaudio.cpp | TSE / medium |
| 625 | 376 |  | `comum_f625` | `std::vector<std::pair<std::string,std::string>>::push_back(value_type&&)` | library: libcxx/include/vector | fimvotacao/cgerabu.cpp | library / high |
| 906 | 32 |  | `vota_f906` | `vota::(anonymous namespace)::ResetaSingletonAudio` | compiler-generated (statics of celeitorvotando.cpp) | cvotacaostateaudio.cpp | compiler / low |
| 950 | 23 |  | `vota_f950` | `std::__throw_regex_error(regex_constants::error_type)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 965 | 458 |  | `ecourna_f965` | `std::__bracket_expression<char, regex_traits<char>>::__add_char` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 1035 | 180 |  | `vota::CVotacaoStateAudio::vf0` | `vota::CVotacaoStateAudio::~CVotacaoStateAudio` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 1202 | 466 |  | `vota_f1202` | `std::regex_token_iterator<const char*>::regex_token_iterator(const regex_token_iterator&)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 1204 | 306 |  | `vota::CVotacaoStateAudio::PlayMessage` | `vota::CVotacaoStateAudio::PlayMessage` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 1276 | 32 |  | `vota_f1276` | `comum::CCopiadorMR::CCopiadorMR` | uenux2/src/app/comum/gravadores/ccopiadormr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / medium |
| 1321 | 226 |  | `vota_f1321` | `std::vector<std::pair<char,char>>::push_back` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 1455 | 496 | yes | `vota::CVotacaoStateAudio::PlayKey` | `vota::CVotacaoStateAudio::PlayKey` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 1542 | 515 |  | `comum_f1542` | `comum::IncluiDataHora` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / low |
| 1597 | 7 |  | `ecourna_f1597` | `std::__throw_regex_error<regex_constants::error_brack>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 1599 | 7 |  | `ecourna_f1599` | `std::__throw_regex_error<regex_constants::error_escape>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 2062 | 1365 |  | `vota_f2062` | `std::basic_regex<char>::__parse<const char*>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 2248 | 554 |  | `comum_f2248` | `comum::IncluiLinhaQuantidade` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 2252 | 2402 |  | `comum_f2252` | `comum::GetIDCargaFormatado(const CDadoCorrespondencia&)` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / low |
| 2391 | 7 |  | `ecourna_f2391` | `std::__throw_regex_error<regex_constants::error_collate>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 2393 | 249 |  | `vota_f2393` | `std::basic_regex<char>::__push_back_ref` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 2416 | 123 |  | `vota_f2416` | `std::basic_regex<char>::basic_regex(const char*, flag_type)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 2775 | 543 |  | `comum_f2775` | `api::CPaperFormBuilder::AddQRCode` | uenux2/src/api/gui/cpaperformbuilder.h | fimvotacao/cgerabu.cpp | TSE / low |
| 2785 | 105 |  | `comum_f2785` | `comum::IncluiSeparadorFase` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerarelatorios.cpp | TSE / low |
| 2789 | 738 |  | `comum_f2789` | `std::__sort4<_ClassicAlgPolicy, __less<>, SEleitorBEHB*>` | library: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | library / medium |
| 3081 | 124 |  | `vota_f3081` | `std::basic_regex<char>::__parse_QUOTED_CHAR` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 3086 | 7 |  | `vota_f3086` | `std::__throw_regex_error<regex_constants::error_badbrace>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 3087 | 7 |  | `vota_f3087` | `std::__throw_regex_error<regex_constants::error_brace>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 3102 | 993 |  | `vota_f3102` | `std::basic_regex<char>::__parse_ERE_expression` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 3107 | 7 |  | `vota_f3107` | `std::__throw_regex_error<regex_constants::__re_err_empty>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 3108 | 309 |  | `vota_f3108` | `std::basic_regex<char>::__parse_extended_reg_exp` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 3135 | 7 |  | `vota_f3135` | `std::__throw_regex_error<regex_constants::__re_err_parse>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 3137 | 283 |  | `vota_f3137` | `std::regex_token_iterator<const char*>::operator==` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 3139 | 525 | yes | `vota::CVotacaoStateAudio::EmiteEcoComInputField` | `vota::CVotacaoStateAudio::EmiteEcoComInputField` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 3143 | 224 |  | `vota::CVotacaoStateAudio::PlayInterruptibleMessage` | `vota::CVotacaoStateAudio::PlayInterruptibleMessage` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 3329 | 153 |  | `ecourna_f3329` | `std::__get_classname(const char*, bool)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 3646 | 666 |  | `comum_f3646` | `comum::MontaConteudoQRCode` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 3688 | 445 |  | `comum_f3688` | `comum::IncluiDatasAberturaFechamento` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 3695 | 135 |  | `comum_f3695` | `std::__tree<...map<SChaveSecao, vector<SEleitorBEHB>>...>::destroy` | library: libcxx/include/__tree | fimvotacao/cgerarelatorios.cpp | library / medium |
| 3827 | 29 |  | `vota_f3827` | `comum::CCopiadorWSQMR::CCopiadorWSQMR` | uenux2/src/app/comum/gravadores/ccopiadormr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / medium |
| 3872 | 180 |  | `comum_f3872` | `comum::CriaDetalheCandidato` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 4203 | 846 |  | `vota_f4203` | `std::basic_regex<char>::__parse_awk_escape` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 4237 | 2688 |  | `vota_f4237` | `std::basic_regex<char>::__parse_bracket_expression` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 4242 | 2302 |  | `vota_f4242` | `std::basic_regex<char>::__parse_ERE_dupl_symbol` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 4257 | 326 |  | `vota_f4257` | `std::basic_regex<char>::__parse_basic_reg_exp` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 4307 | 478 |  | `vota_f4307` | `std::regex_token_iterator<const char*>::regex_token_iterator(first, last, re, initializer_list<int>)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 4557 | 162 |  | `vota_f4557` | `vota::CLogVota::LogaMesarioNaoSolicitouVias` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / medium |
| 4563 | 174 |  | `vota_f4563` | `vota::CLogVota::LogaMesarioIndagadoQtdVias` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / medium |
| 4671 | 62 |  | `vota_f4671` | `std::regex_error::regex_error(regex_constants::error_type)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 5185 | 886 |  | `ecourna_f5185` | `std::regex_traits<char>::__lookup_collatename` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 5186 | 873 |  | `ecourna_f5186` | `std::basic_regex<char>::__parse_awk_escape (string* variant)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 5187 | 1151 |  | `ecourna_f5187` | `std::basic_regex<char>::__parse_character_escape (variant)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 5581 | 427 |  | `comum_f5581` | `comum::IncluiCabecalhoQRCode` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 5584 | 15 |  | `comum_f5584` | `comum::IncluiTextoComUF` | uenux2/src/app/comum/relatorios/crelutil.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 5599 | 1759 |  | `comum_f5599` | `std::__insertion_sort_incomplete<..., SEleitorBEHB*>` | library: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | library / medium |
| 5600 | 6076 |  | `comum_f5600` | `std::__introsort<..., SEleitorBEHB*>` | library: libcxx/include/__algorithm/sort.h | fimvotacao/cgerarelatorios.cpp | library / medium |
| 5602 | 320 |  | `comum_f5602` | `std::vector<SEleitorBEHB>::push_back(value_type&&)` | library: libcxx/include/vector | fimvotacao/cgerarelatorios.cpp | library / medium |
| 5613 | 503 |  | `comum_f5613` | `comum::AddNomeCargo` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 5614 | 107 |  | `comum_f5614` | `comum::CCalculaCV::~CCalculaCV` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | fimvotacao/cgerabu.cpp | TSE / medium |
| 5634 | 2021 |  | `comum_f5634` | `comum::MontaQRCodesCertificado` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 5883 | 179 |  | `vota_f5883` | `vota::CLogVota::LogaMRInvalida` | uenux2/src/app/vota/log/clogvota.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / medium |
| 5980 | 225 |  | `comum_f5980` | `std::vector<std::vector<std::pair<std::string,std::string>>>::~vector` | library: libcxx/include/vector | fimvotacao/cgerabu.cpp | library / high |
| 6751 | 64 |  | `vota_f6751` | `std::basic_regex<char>::__test_back_ref(char)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6762 | 439 |  | `ecourna_f6762` | `std::__bracket_expression<char, regex_traits<char>>::__add_neg_char ('_' constant-propagated)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6763 | 171 |  | `vota_f6763` | `std::__bracket_expression<char, regex_traits<char>>::__add_digraph` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6764 | 1015 |  | `vota_f6764` | `std::__bracket_expression<char, regex_traits<char>>::__add_range` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6765 | 332 |  | `vota_f6765` | `std::basic_regex<char>::__parse_class_escape` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6770 | 7 |  | `vota_f6770` | `std::__throw_regex_error<regex_constants::error_ctype>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 6788 | 1114 |  | `vota_f6788` | `std::basic_regex<char>::__parse_character_escape` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6826 | 1290 |  | `vota_f6826` | `std::basic_regex<char>::__parse_atom<const char*>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6833 | 741 |  | `vota_f6833` | `std::basic_regex<char>::__parse_assertion<const char*>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6862 | 1244 |  | `vota_f6862` | `std::basic_regex<char>::__parse_RE_dupl_symbol` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6865 | 637 |  | `vota_f6865` | `std::basic_regex<char>::__parse_simple_RE / __parse_RE_expression` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6869 | 83 |  | `vota_f6869` | `std::basic_regex<char>::__parse_term<const char*>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6871 | 391 |  | `vota_f6871` | `std::basic_regex<char>::__parse_ecma_exp<const char*>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6879 | 7 |  | `vota_f6879` | `std::__throw_regex_error<regex_constants::__re_err_grammar>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / high |
| 6891 | 94 |  | `vota_f6891` | `std::basic_regex<char>::__parse_alternative<const char*> (__parse_term inlined)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6939 | 590 |  | `vota_f6939` | `std::regex_iterator<const char*>::operator++` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6942 | 504 |  | `vota_f6942` | `std::regex_iterator<const char*>::operator==` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6954 | 239 |  | `vota::CVotacaoStateAudio::FormatPartyName` | `vota::CVotacaoStateAudio::FormatPartyName` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 6958 | 863 |  | `vota_f6958` | `std::regex_replace<back_insert_iterator<string>, const char*, regex_traits<char>, char>` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6970 | 404 |  | `vota_f6970` | `std::regex_token_iterator<const char*>::operator++` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 6999 | 4946 |  | `vota::CVotacaoStateAudio::vf14` | `vota::CVotacaoStateAudio::FormataMensagem` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / medium |
| 7010 | 683 | yes | `vota::CVotacaoStateAudio::ProcessTick` | `vota::CVotacaoStateAudio::ProcessTick` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 7022 | 132 | yes | `vota::CVotacaoStateAudio::vf5` | `vota::CVotacaoStateAudio::FinishState` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 7028 | 55 | yes | `vota::CVotacaoStateAudio::vf2` | `vota::CVotacaoStateAudio::StartState` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 7029 | 282 | yes | `vota::CVotacaoStateAudio::ProcessInput` | `vota::CVotacaoStateAudio::ProcessInput` | uenux2/src/app/vota/eleitor/cvotacaostateaudio.cpp | cvotacaostateaudio.cpp | TSE / high |
| 7479 | 12 |  | `vota_f7479` | `__dtor_static_unique_ptr@1832992` | compiler-generated (statics of celeitorvotando.cpp) | cvotacaostateaudio.cpp | compiler / low |
| 9511 | 332 |  | `ecourna_f9511` | `std::basic_regex<char>::__parse_class_escape (instance used by libcxx_f3523)` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 9512 | 211 |  | `ecourna_f9512` | `std::basic_regex<char>::__parse_collating_symbol` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 9513 | 490 |  | `ecourna_f9513` | `std::basic_regex<char>::__parse_character_class` | library: libcxx/include/regex | cvotacaostateaudio.cpp | library / medium |
| 11689 | 12 |  | `vota_f11689` | `__dtor_static_unique_ptr@1838312` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11692 | 12 |  | `vota_f11692` | `__dtor_static_unique_ptr@1838284` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11695 | 12 |  | `vota_f11695` | `__dtor_static_unique_ptr@1838256` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11698 | 12 |  | `vota_f11698` | `__dtor_static_unique_ptr@1838228` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11703 | 12 |  | `vota_f11703` | `__dtor_static_unique_ptr@1838200` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11706 | 12 |  | `vota_f11706` | `__dtor_static_unique_ptr@1838172` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11714 | 12 |  | `vota_f11714` | `__dtor_static_unique_ptr@1838108` | compiler-generated (statics of celeitorvotando.cpp) | cvotacaostateaudio.cpp | compiler / low |
| 11730 | 12 |  | `vota_f11730` | `__dtor_static_unique_ptr@1837944` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11734 | 12 |  | `vota_f11734` | `__dtor_static_unique_ptr@1837916` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11737 | 12 |  | `vota_f11737` | `__dtor_static_unique_ptr@1837888` | compiler-generated (statics of cconferevotoemcargo.h) | cvotacaostateaudio.cpp | compiler / low |
| 11754 | 653 | yes | `vota::CVotacaoStateAudio::EmiteEcoCorrigeConfirma` | `vota::CConfirmaVotoEmCargo::ProcessInputAudio` | uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.cpp | cvotacaostateaudio.cpp | TSE / high |
| 12039 | 422 |  | `comum_f12039` | `comum::ConteudoQRCodeCertificado` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | fimvotacao/cgerabu.cpp | TSE / low |
| 12081 | 1543 |  | `vota::CEmitirMaisBU::ProcessInput` | `vota::CEmitirMaisBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / high |
| 12082 | 250 |  | `vota::CEmitirMaisBU::StartState` | `vota::CEmitirMaisBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cemitirmaisbu.cpp | fimvotacao/cemitirmaisbu.cpp | TSE / high |
| 12105 | 18808 |  | `vota::CGeraRelatorios::StartState` | `vota::CGeraRelatorios::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp | fimvotacao/cgerarelatorios.cpp | TSE / high |
| 12110 | 17349 |  | `vota::CGeraBU::vf2` | `vota::CGeraBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp | fimvotacao/cgerabu.cpp | TSE / high |
| 12134 | 5966 |  | `vota::CCopiaResultadoParaMR::CopiaResultado` | `vota::CCopiaResultadoParaMR::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/ccopiaresultadoparamr.cpp | fimvotacao/ccopiaresultadoparamr.cpp | TSE / high |
| 13424 | 90 |  | `vota_f13424` | `vota::(anonymous namespace)::TextoStatusAudio` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | cvotacaostateaudio.cpp | TSE / low |

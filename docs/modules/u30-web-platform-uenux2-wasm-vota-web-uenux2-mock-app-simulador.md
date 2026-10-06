# u30: plataforma web - `uenux2/wasm/vota_web` + `uenux2/mock/app/simulador`

A unidade u30 tem 109 funções wasm (36 delas executaram nos votos gravados). Elas são o **lado do navegador do
simulador**: código que não existe na urna eletrônica e substitui a inicialização do seu
processo, o seu hardware e o seu estado persistente:

* a **API C que o JavaScript aciona** (`main`, `votaTick`, `votaPressKey`, `votaGetStateJson`,
  `votaSetAudioEnabled`) e os helpers de `votaInit` que ficam nesta unidade (leitores do JSON de opções, o
  gravador da "assinatura simulada", o relator de erros);
* dois **mocks de hardware**: `simulador::CWasmBeep` (o buzzer da urna como tons de Web Audio) e
  `simulador::(anonymous)::CEsperaAudioWasm` (um handle cancelável sobre uma "espera pelo fim da
  fala" assíncrona);
* a maior parte de **`comum::teste::CAppInfoBuilder`**, uma *fixture de teste* da base de código da urna que o build web executa
  em produção para fabricar os arquivos de estado da urna (`eg.bin`, `gap.bin`, `sa.bin`, `vota.bin`) na primeira inicialização;
* 62 funções que as ferramentas colocaram aqui só por vizinhança na tabela ou porque `votaInit` as chama.
  Elas são identificadas abaixo: libc++ (13), SQLite (5), OpenSSL (14), comparadores de ordenação das listas de cargos (3),
  destrutores `atexit` de singletons gerados pelo compilador (22) e pequenas funções de classes de `comum` (5).

As 47 funções do TSE da unidade propriamente dita: 15 em `vota_web_wasm.cpp`, 11 nos mocks do simulador, 21 no
builder (`cappinfobuilder.cpp`).

Outras unidades cobrem o restante dos mesmos arquivos: u28 (`CWasmSavd`, `CPoliticaExecucaoEleitorWeb`, `SalvaGeral`,
`SalvaApps`), u29 (`CVotaWebEngine`, `votaInit`, o JSON de estado), u31 (`CWasmScreen`, `CWasmWebSound`,
`CWasmInputKbd`, ...), u19 (`CSimuladorWasm::Executa` e o registro de poly-singletons). O protocolo da página está
documentado em [`docs/03-js-wasm-interface.md`](../03-js-wasm-interface.md); este capítulo não o repete.

Arquivos-fonte reconstruídos:

```
src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp        main, votaTick (CVotaWebEngine::Tick), votaPressKey,
                                                     votaGetStateJson, votaSetAudioEnabled (SetAudioEnabled :592),
                                                     JsonBool/JsonInt/JsonString, WriteFileIfMissing,
                                                     WriteSimulatedSignatures, ReportError, voter-queue thunks
src/uenux2/mock/app/simulador/wasm/cwasmbeep.{h,cpp}  simulador::CWasmBeep (whole class)             (path inferred)
src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp   CEsperaAudioWasm + the waits map        (path inferred)
src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp     CAppInfoBuilder ctor, setters, Salva, CriaEstado/Copia,
                                                     fixture defaults, AssinaturaFicticia, EscreveArquivo
src/uenux2/src/app/comum/u30-foreign-fragments.cpp    CAppInfo::CarregaGeral, CEstadoGeralVota::SetEstadoVota,
                                                     IServicoEstado<..>::Salva x2, CServicoEstadoGeralSA ctor,
                                                     the 3 cargo comparators, comum singleton atexit dtors
src/uenux2/src/app/vota/u30-foreign-fragments.cpp     vota singleton atexit dtors (table)
```

## 1. Glossário

| termo | significado |
|---|---|
| urna / UE | a *urna eletrônica*; modelos UE2013..UE2022 |
| VOTA | o aplicativo do dia da eleição da urna (terminal do eleitor + terminal do mesário) |
| eleitor / mesário | quem vota / quem opera a seção. O mesário "libera" (*habilita*) a urna para cada eleitor |
| carga | o carregamento de uma urna com os dados da eleição antes do dia da eleição, a partir de uma *flash de carga* (FC) |
| MI / MV (flash interna / externa) | a flash interna (`/dsk/fi/`) e o cartão de votação removível (`/dsk/fe/`); todo arquivo de estado existe nos dois |
| `dinamico/`, `trab1/`, `trab2/` | a área dinâmica (gravável) e os diretórios de trabalho do 1º e do 2º *turno* |
| `eg.bin`, `gap.bin`, `sa.bin`, `vota.bin` | estado persistente: *estado geral*, GAP (lançador de aplicativos), SA (sistema de apuração manual), VOTA; `ModuloEstadoGeral*` codificado em BER |
| `.vsu` | pacote de assinatura de um arquivo, produzido na urna pelo SAVD (serviço de assinatura) com o HSM |
| fase | `'1'` oficial, `'2'` simulado, `'3'` treinamento; seleciona o prefixo dos arquivos de dados `o`/`s`/`t` |
| zerésima | o relatório impresso antes do início da votação, que comprova que os contadores de votos estão zerados |
| BU / RDV | *Boletim de Urna* (resultado por seção, impresso e gravado) / *Registro Digital do Voto* (registro embaralhado de todos os votos) |
| encerramento | o encerramento da seção pelo mesário: geração do BU, do RDV e de outros arquivos de resultado |

## 2. Classes e hierarquia (RTTI)

```
api::IBeep                                   (class, typeinfo @1531756; destructor declared LAST: slots 7/8)
  └─ simulador::CWasmBeep                    (si, typeinfo @1530348, vtable @1530312, 4 bytes)

api::IEsperaAudio                            (class, typeinfo @1528612)       "wait for the end of the audio"
  ├─ api::CEsperaAudio                       (si, vtable @1528584, 8 bytes: +4 bool cancelled)   (u31/api)
  └─ simulador::(anonymous)::CEsperaAudioWasm(si, typeinfo @1528684, vtable @1528668, 12 bytes:
                                                +4 int id, +8 bool cancelled), created with make_shared
                                                (control block vtable @1528628)

comum::teste::CAppInfoBuilder                (no RTTI, 500 bytes)             test fixture, mock/app/comum
    +0   md::CEstadoGeral      (180)   -> dinamico/eg.bin
    +180 md::CEstadoGeralGap[2] (44)   -> trab1|2/gap.bin
    +268 md::CEstadoGeralSA[2]  (16)   -> trab1|2/sa.bin
    +300 md::CEstadoGeralVota[2](100)  -> trab1|2/vota.bin

(anonymous)::CVotaWebEngine                  (no RTTI, 40 bytes, unit u29): the state kept between API calls
```

Os nomes dos slots de `IBeep` são inferidos a partir dos chamadores (o destrutor ocupa os dois últimos slots, 7 e 8):

| slot | CWasmBeep | tons (Hz x 1/100 s) | chamadores |
|---:|---|---|---|
| 0 | `SetVolume(int)` 8541 | `js_wasm_beep_set_volume(v)` | nenhum encontrado |
| 1 | `Beep(freq, dur)` 8538 | `js_wasm_beep_queue(freq, dur*10)` | as melodias abaixo (virtualmente), `CExibeAlertaDesligamento` (2000, 100), `EnterLoopBeeping` (800, n) |
| 2 | `BeepConfirma(int n)` 8533 | n x {2300x10, 2200x10} | `CEleitorVotando::NeedChangeState` (1: cargo confirmado), `CFimVotoEleitor::StartState` (4: o bipe do "FIM") |
| 3 | `BeepAlerta()` 8523 | 392x50, 370x50, 349x50, depois 3 x {329, 334, 329, 324}x10, 329x10 ("trombone triste") | `CVerificaEleicaoPassou::StartState` (data da eleição inválida) |
| 4 | `BeepFinalizacao()` 8514 | 17 notas: 1244 830 1046 1244 / 1661 1568 1661 pausa / 1864 1661 1864 pausa / 2489 2349 2489 2349 2489x40 | `CThreadMonitor::SaiPorVotacaoSuspensa` (assíncrono), `CInformacaoZeresimaTardia::ProcessInput` |
| 5 | `BeepSuspensao()` 8505 | 2489 2349 1864 1568 1244 1174 (x11), 1244x69 | `CEleitorVotando::DescartaVotos` |
| 6 | `BeepErro()` 8494 | 554x20 (C#5, 200 ms) | teclas recusadas: `CVotacaoStateAudio::PlayKey`, `CInputMenuField::Read`, `CMenuValidation` |

As tabelas de tons são as constantes i64 armazenadas no `std::vector<{int,int}>` de cada método; `BeepAlerta`
monta seu vetor com `push_back` (func 760, dobrada com código do RHVoice) com o laço de vibrato desenrolado.

## 3. Fluxo de controle

### 3.1 `main` (func 10307)

`main` roda uma vez, dentro do `callMain` do glue, antes de o adaptador da página existir:

1. `js_use_external_keyboard()`; largura/altura de `js_ler_dimensao_tela(0, 1280)` / `(1, 800)` - o JS lê
   `?screenWidth=`, `?screen=WxH` (também `resolution`, `resolucao`) ou `Module.votaScreenWidth/Height`. Os valores
   são passados para `CSimuladorWasm` **estreitados para `short`** (`i32.extend16_s`).
2. `CSimuladorWasm simulador(10, "VOTA Web", w, h)` (func 8311, u19).
3. `CApplication::InitApplication("VOTA", "Software de Votação", "10.23.0.1 - DESENVOLVIMENTO", true)` - essa
   string de versão é a que o BU e os logs carregam (`urna.versaoVotacao`).
4. Registra os serviços do processo em `api::CPolySingletonList`, cada um por meio do wrapper de registrar-ou-substituir
   (o `replace` da u19: `exists` -> `erase` -> `push`; 10090/9507/9437/8728 começam com esse teste; um `push` puro
   lançaria 6756 em caso de duplicata), nesta ordem: `IInterfaceSavd` = `CWasmSavd`
   (responde OK a toda solicitação de assinatura/validação), `IRng` = `CPrng`, `ISymmetricCipherFactory`,
   `IFingerPrepare` = `CFingerPrepareSimulador`, `IAjusteDataHora::CreateInst()`, as quatro `IGenericFactory`
   (threads = `CWasmThread`, locks = wrappers POSIX sobre pthreads stubados), `IPoliticaExecucaoEleitor` =
   `CPoliticaExecucaoEleitorWeb`, `ISincronismoVotoEleitor` = `CSincronismoVotoEleitorWeb` (não grava o
   RDV), `IExecucaoVota` = `CExecucaoVotaCooperativa(CAguardaMensagem::GetInst())`.
5. `simulador.Executa(lambda)` (func 8302): registra os mocks de hardware (tela, teclado, `CWasmBeep`, impressora
   nula...), depois a lambda (func 10384) substitui `ISound` por `CWasmWebSound` e emite `vota:ready`.
6. Retorna 0; `noExitRuntime` mantém o módulo vivo.

Registrar as políticas web *antes que alguém as peça* é o que impede que os padrões da urna
(`CExecucaoVota` com threads reais, `CPoliticaExecucaoEleitor` com `emscripten_sleep`, `CSincronismoVotoEleitor`
gravando `rdv.dat`) sejam criados: os `GetInst()` deles só criam um padrão quando nada está registrado.

### 3.2 `votaTick` (func 10619 = `CVotaWebEngine::Tick`)

```
votaTick():
  if !initialized or done: return 0
  before = CurrentStateName()                          (RTTI name of the voter thread's state, demangled)
  IExecucaoVota::GetInst().Processa()                  (slot 7: one step of CThreadEleitor)
  after  = CurrentStateName()
  if after contains "CSincronismoEleitor":             ("Gravando..." screen)
      now = emscripten_get_now()
      first tick:  recStarted=1, step=0, deadline=now+120
      later:       if !recFinished and now >= deadline:
                      step <= 3 : CTelasVota::AvancaBarraProgresso(); deadline=now+160; step++
                      else      : post message 5 to the voter queue; recFinished=1
  else: reset the recording fields
  if after contains "CAguardaMensagem" and before != after: done = 1
  json = BuildStateJson()
  if before != after or json != lastJson: lastJson = json; emit vota:state
  if done and (same condition): emit vota:done (same JSON)
  return 1
  catch std::exception -> ReportError(what()), return 0;  catch ... -> ReportError("erro desconhecido em votaTick")
```

Linha do tempo da animação de gravação: passos da barra em +120, +280, +440, +600 ms, mensagem 5 em +760 ms. Na urna,
a ordem é a inversa: a thread do *operador* envia a mensagem 5 primeiro (`CSincronismoOperador::StartState`,
func 10210), e a thread do eleitor então chama `SincronizaVoto` a partir de `CSincronismoEleitor::ProcessMessage(5)`
(func 7181), que grava o RDV e o `vota.bin` nas duas flashes e avança a barra (u07 §4). Aqui a mensagem 5
vem de `votaTick`, a mesma func 7181 chama `CSincronismoVotoEleitorWeb::SincronizaVoto` (`return true`),
nada é gravado, e a barra é só um timer.

`done` é ligado em **qualquer** transição de volta para `CAguardaMensagem` (um voto concluído ou um descartado); depois
disso, `votaTick` retorna 0 para sempre, então cada carregamento da página atende exatamente um eleitor.

### 3.3 As outras exportações

* `votaPressKey(const char*)` (10703): cria o engine se necessário, depois `js_push_key(key)`: a tecla vai para
  `Module.uenuxKeys` e volta por meio de `CWasmInputKbd` em um tick posterior. Nenhuma verificação de `initialized`.
* `votaGetStateJson()` (10171): `lastJson = BuildStateJson(); return lastJson.c_str()`. Sobrescreve o cache
  com o qual `votaTick` compara (ver §9).
* `votaSetAudioEnabled(int)` (10240): `CVotaWebEngine::SetAudioEnabled` (srcloc `vota_web_wasm.cpp:592`):
  `audioEnabled = a != 0; ISound::instance().Mute(!a)` (slot 6 de ISound -> `js_wasm_web_sound_mute`).
* `ReportError(text)` (10857): `console.error(text)` e `vota:error` com `{"message":"<escaped>"}`.

### 3.4 Os leitores do JSON de opções (14477, 11571, 11546)

`votaInit` recebe `{"fase":"te","pe":2400,"turno":1,"uf":"ac","municipio":1,"zona":1,"secao":1,
"audioEleitorHabilitado":false,"reproduzirAudio":false}`. Os três leitores compartilham um algoritmo: encontram a primeira
ocorrência de `"<key>"` **em qualquer lugar do texto**, depois o próximo `:`, pulam `" \t\r\n"` e leem:

| leitor | valor | padrão | peculiaridades |
|---|---|---|---|
| `JsonBool(json, key, def)` 14477 | `compare(pos,4,"true")` / `compare(pos,5,"false")` | o argumento | qualquer outro token mantém o padrão |
| `JsonInt(json, key)` 11571 | `strtol(p, &end, 10)` | 1 (constante: todo chamador passa 1) | sem verificação de intervalo; negativos aceitos; `long` tem 32 bits, então satura em 2^31-1 |
| `JsonString(json, key, def)` 11546 | caracteres até a próxima `"` não escapada | o argumento | `\x` mantém `x` literalmente: escapes de JSON (`\n`, `\u00e9`) não são decodificados; sem terminação -> padrão |

Em seguida, `votaInit` trunca `zona` e `secao` para 16 bits e mapeia `fase` = `"oficial"`/`"o"` -> `'1'`,
`"simulado"`/`"s"` -> `'2'`, qualquer outro valor -> `'3'`, `turno == 2 ? '2' : '1'`.

### 3.5 Fabricando o estado da urna: `CAppInfoBuilder`

No primeiro `votaInit` (quando `<MI>/dinamico/eg.bin` não existe; teste = func 11661 + `exists`), `votaInit`
monta a fixture e a salva:

```
CAppInfoBuilder b(pe)                                   10268
 .SetFase(f).SetTurno(t).SetTipoUrna('1').SetLocal(m, z, s)   10197 10188 10181 10176
b.geral.uf = ToUpper(uf); b.vota[0..1] = {estado '1', treinamentoEleitor = true}      (inlined)
b.Salva(false, {MI, MV})                                10256
    SalvaGeral(false, {MI,MV})                          10243 (u28): eg.bin on MI and MV
    SalvaAppsPrimeiroTurno(false, {MI,MV}, TodosApps()) 10242 -> 6004 -> SalvaApps(.., '1', {Gap,SA,Vota})
    SalvaAppsSegundoTurno (false, {MI,MV}, TodosApps()) 10239 -> 6004 -> SalvaApps(.., '2', ...)
WriteSimulatedSignatures()                              11733
```

Todo objeto de estado é **reconstruído por meio do seu construtor membro a membro de md** antes de ser codificado
(`CriaEstado` 10205/10123/10101/10067, helpers folha `Copia` 9863/10167/10155, mais os `Copia` 5324/9862 da u29 e
o provável `Copia(CAjusteDataHora)` 10162): isso executa de novo as validações de md que os
padrões aplicados no lugar pularam (município < 100000, zona/seção < 10000, modelo 2013..2022, fase '1'..'3') e o
recálculo de `data2T` do gap. Cada arquivo é gravado por `IServicoEstado<T, Conversor>::Salva` (corpo mesclado 2894:
`CFileASN::WriteToFile(GetPathArquivo(), Conversor().Converte(estado))`; instanciações 10089 SA, 10112 GAP nesta
unidade).

Valores da fixture (construtor 10268 e os construtores padrão 10261, 9952, 10278):

| campo | valor | observações |
|---|---|---|
| `estadoUrna` | `'3'` testada | a carga está completa e testada |
| `idPE` | `pe` vindo das opções (15000 antes de o corpo executar) | junto com a letra da fase, dá nome aos arquivos da eleição, p. ex. `t02400-cp.dat` |
| `dadoLocal` | UF `"EE"` (depois `ToUpper(uf)`), município/zona/seção 1/1/1 (depois as opções), tipo local 1 | |
| `dadoCarga` | turno `'1'`, tipo de urna T1/T2 `'1'` (seção), modelo **2015**, fase `'2'` (depois as opções) | |
| `ajusteDataHora` | {0, 0} | |
| `correspondencia` (a carga) | urna **87654321**, serial da FC `"12345678"`, horário da carga **31/12/2020 23:59:58**, código de carga **`123456789012345678901234`**, localidade 1/1/1, assinatura = 139 bytes falsos, gerador `{"nome_maquina", "12345678", "99999999"}` | o `urna.correspondenciaResultado.carga` do BU |
| `versao` | `"7.2.1.3 - TESTE EG ASN1"` | |
| `hashVersoesPacotes` | 64 bytes `0x0A` | |
| GAP (x2) | sem correspondências, app/appAnterior 14 (nenhum), flags false, data do 2º turno 31/12/2080 | o `historicoCodigosCarga` do BU vem daqui: vazio |
| SA (x2) | estado `'1'` inicial, não bloqueado, município/zona/**seção 1/1/2** | a seção difere da do estado geral |
| VOTA (x2) | estado `'1'` inicial, encerramento `'1'`, todos os contadores 0, sem datas; `votaInit` define `treinamentoEleitor` | depois `votaInit` define `EAVVOTAR` ('8', func 11266) e salva |

A assinatura falsa (`AssinaturaFicticia`, 9963) é o texto hexadecimal `123456789abcdefABCDEF` repetido até 278
dígitos e decodificado: 139 bytes, o tamanho DER máximo de uma assinatura ECDSA P-521.

Depois, nos dois caminhos (fixture recém-gravada ou `eg.bin` já presente), `votaInit` carrega `eg.bin` no
cache de `CAppInfo` com `CAppInfo::CarregaGeral` (11572: `m_geral = CServicoEstadoGeral(MI).Carrega()`, também zera
CAppInfo+528).

### 3.6 Assinaturas simuladas (`WriteSimulatedSignatures`, 11733, e `WriteFileIfMissing`, 11818)

Chamada quatro vezes por `votaInit`. Para cada flash (`fi`, `fe`): `dinamico/eg.vsu` e, para `trab1` e `trab2`:
`eg.vsu gap.vsu sa.vsu vota.vsu rdv.vsu uenux.vsu`. Conteúdo: os 38 bytes
`assinatura simulada para vota_web_wasm`. `WriteFileIfMissing` cria os diretórios pais e só grava
quando o caminho não existe; `votaInit` também a usa para `serialv.dat` = `"ABCDDCBA"` nas duas raízes.
Nada verifica esses arquivos: `CWasmSavd` responde "OK" a todo `ValidarUE` (u28).

### 3.7 Esperas de áudio assíncronas (`CEsperaAudioWasm`)

`CWasmWebSound::vf9` (u31) armazena `{cancelado, terminado}` (duas `std::function`) em um
`std::map<int, SEspera>` estático (@1832664) sob um id novo e chama `js_wasm_web_sound_wait_async(id)`; o JavaScript faz polling
a cada 20 ms por meio das exportações `uenux_wasm_web_sound_wait_cancel_requested` (9614) e
`uenux_wasm_web_sound_wait_finished` (9604). O handle retornado é um `CEsperaAudioWasm`:

* `Cancela()` (2680, slot 2): só uma vez; apaga a entrada do map **sem chamar `terminado`** e interrompe o polling
  do JS (`js_wasm_web_sound_cancel_wait(id)`, enviado mesmo que a entrada já não exista).
* `Cancelada()` (9445, slot 3): a flag.
* os destrutores (9459, 9448) chamam `Cancela()`: descartar o handle cancela a espera.
  `CVotacaoStateAudio` guarda o handle e cancela a espera anterior antes de iniciar uma nova (func 7010).

## 4. Dados lidos e gravados

| o quê | como | onde |
|---|---|---|
| `/dsk/fi|fe/dinamico/eg.bin` | `ModuloEstadoGeralUrna.EstadoGeralUrna` via `CConversorEstadoGeral` | gravado uma vez pelo builder, lido por `CarregaGeral` |
| `/dsk/fi|fe/dinamico/trab1|2/gap.bin, sa.bin, vota.bin` | `ModuloEstadoGeralGap/SA/Vota` | gravados por `SalvaApps` (u28) com o `CriaEstado` desta unidade |
| `.../*.vsu` | texto puro | `WriteSimulatedSignatures` |
| `/dsk/fi|fe/serialv.dat` | `"ABCDDCBA"` | `WriteFileIfMissing` (a partir de `votaInit`) |
| importações | `js_use_external_keyboard`, `js_ler_dimensao_tela`, `js_wasm_beep_init/queue/set_volume`, `js_wasm_web_sound_cancel_wait`, `emscripten_get_now` (+ `js_push_key`, `js_console_error`, `js_emit_event` por meio de helpers da u29) | |
| exportações | `Kb` main, `Hb` votaTick, `Gb` votaPressKey, `Jb` votaGetStateJson, `Ib` votaSetAudioEnabled | |
| eventos DOM | `vota:state`, `vota:done` (votaTick), `vota:error` (ReportError) | via `js_emit_event` |

Nenhuma rede, nenhum SQL e nenhuma criptografia nesta unidade (as funções de OpenSSL/SQLite listadas na §7 são código de biblioteca
que as ferramentas atribuíram erroneamente).

## 5. Particularidades do build web

* A API C, `main`, o uso da fixture e os substitutos de assinatura só existem no simulador.
* A thread do operador (mesário) nunca executa. `votaTick` desempenha o único papel dela necessário para um eleitor: liberar o
  eleitor da tela "Gravando" (mensagem 5). A mensagem 1 ("Eleitor foi habilitado") é postada por `votaInit`.
* O estado da urna não é produzido por uma carga e uma zerésima: ele é fabricado (§3.5) e `votaInit` salta para
  "votação aberta" (`EAVVOTAR`) diretamente.
* A fixture só é gravada se `eg.bin` estiver ausente (ver §9: um init que falhou faz os inits seguintes a reutilizarem).
* O buzzer é Web Audio; `CWasmBeep` preserva as durações da urna (unidades de 1/100 s).

## 6. BOLETIM DE URNA (BU)

Esta unidade nunca gera um BU: o encerramento é iniciado pela mensagem 7 da thread do operador
(`CFimAquisicaoVotos` -> `CAguardaMensagem`), que não executa no build web, e `votaTick` para no
primeiro retorno a `CAguardaMensagem` (ver [`docs/10-boletim-de-urna.md`](../10-boletim-de-urna.md) §5.1). Ela
define, sim, dados e código que o BU usa quando o código do BU é acionado diretamente (harness em `docs/bu/`):

1. **Identidade do cabeçalho vinda da fixture.** `urna.correspondenciaResultado.carga` = urna 87654321, FC `12345678`,
   `nome_maquina`/`12345678`/`99999999`, `20201231T235958`, código de carga `123456789012345678901234`
   (func 10261/9952); `fase` treinamento (`'3'` a partir do `"te"` da página), `tipoUrna` seção (`'1'`), e o
   município/zona/seção das opções. `historicoCodigosCarga` está vazio porque o `gap.bin` da fixture não tem
   correspondências (func 10278).
2. **Versão do software.** `urna.versaoVotacao` = `"10.23.0.1 - DESENVOLVIMENTO"`, definida por `main` por meio de
   `CApplication::InitApplication`. (O próprio `versao` de `eg.bin` é o `"7.2.1.3 - TESTE EG ASN1"` da fixture.)
3. **Ordem dos cargos.** O texto do BU, os QR codes e os arquivos de resultado listam as eleições pelo
   `ordemImpressao` da eleição (byte +5 do seu registro `CSituacoesEleicoes`) e depois os cargos por `CCargo::ordemImpressao`
   (+16): comparador 11558 usado por `CCargos::OrdenaPorOrdemImpressao` (func 3782, chamada por
   `CGeraBU::StartState` 12110 e `CGravaResultado` 12098). Dentro do arquivo do BU, `CGravadorBU` (slot 7) usa
   `CConfiguracaoEleicao::GetCargos(eleição, true)` (func 3775) ordenado pelo comparador 11547 (`CCargo +16`). Já o
   eleitor é consultado na ordem de *aquisição* (comparador 11559: eleição +4, depois `CCargo +15`).
4. **Assinaturas.** Os pacotes de assinatura do próprio BU (`bu.vsu`, ids 126-129) não estão entre os substitutos; a
   cadeia de hashes e sua assinatura precisam de chaves que o build web não tem (`docs/10` §4).
5. **Votos.** Nenhum voto é persistido: `CSincronismoVotoEleitorWeb` retorna true sem gravar `rdv.dat` nem
   `vota.bin`, e a barra "Gravando" é o timer da §3.2. Um BU gerado depois de uma sessão web contaria zero
   votos a partir do RDV.

## 7. Observações sobre wasm / Emscripten

* **Thunks especializados por ponto de chamada.** As funcs 11026/11100/10376 (`api_f3903(queue, 1|9|5)`), 11661
  (`GetPathDinamico(a, 0)`) e 10242/10239 (`6004(.., '1'|'2')`) são funções de 9-15 bytes que chamam um corpo compartilhado
  com uma constante. Esse é o formato deixado pela especialização de funções do LLVM sobre um argumento constante seguida do
  `merge-similar-functions` do wasm-opt; no código-fonte, muito provavelmente são chamadas simples com uma constante.
* **Uma importação na tabela de funções.** `votaTick` chama `emscripten_get_now` por meio de `invoke_d(107)`: o slot
  107 da tabela contém a própria importação, porque a chamada está dentro de um `try`.
* **`i32.extend16_s` em main**: o tamanho da tela vindo da URL é estreitado para `short` (§9).
* **Construtores que retornam this revelam os padrões**: 10261, 9952, 10278 retornam `this` e são chamados sobre endereços
  de membros do builder: são construtores sem argumento que definem valores da fixture no lugar.
* **Destrutores `atexit` mortos.** 22 funções são os destrutores gerados pelo compilador de estáticos de singletons
  (`std::mutex` = o corpo dobrado de stub de pthread 150; `std::unique_ptr` = reset). O Emscripten nunca executa
  handlers de `atexit` aqui; eles só sobrevivem porque seus endereços estão na tabela.
* **Funções de biblioteca atribuídas erroneamente.** A heurística de "vizinhos na tabela de dados" colocou estas aqui: do SQLite,
  `jsonCacheDelete` (12382) e os callbacks do percorredor de `ALTER TABLE ... RENAME` `renameQuotefixExprCb`,
  `renameTableExprCb`, `renameTableSelectCb`, `renameColumnExprCb` (12433-12441); o provider DES do OpenSSL
  (`cipher_des_hw.c`: cfb8/cfb1/cfb64/ofb64/cbc/copyctx/ecb/initkey, 13089-13096) e os helpers de MAC CBC do TLS
  `tls1_{sha512,sha256,sha1,md5}_final_raw` (13135-13139) + `SHA512_Transform`/`SHA256_Transform` (14363/14364),
  cujos slots da tabela são usados por `hmac_update` (`ssl3_cbc_digest_record` de `ssl/s3_cbc.c`, inlinado no provider HMAC do OpenSSL 3.0.17); da libc++,
  `std::__formatter::__escape` (11769), a view de clusters de grafemas da estimativa de largura do `<format>` (11971/11977),
  `basic_stringbuf::str()` (11016), `optional<locale>::operator=` (12020) e várias instanciações de
  vector/path/filebuf.

## 8. Verificações em tempo de execução feitas para este capítulo

Com uma cópia de `tools/run/headless.mjs` cujo JSON de `votaInit` pode ser sobrescrito:

* `"fase":"oficial"`: a fixture é gravada com a fase `'1'`, depois o carregamento falha com
  `O arquivo [/dsk/fi/estatico/o02400-cp.dat] não existe` -> `vota:error`, `votaInit` = 0. A fase só seleciona o
  prefixo dos arquivos de dados; os cenários só trazem arquivos `t`.
* Um segundo `votaInit` na mesma página com as opções `"te"` corretas falha da mesma forma (`o02400-cp.dat`):
  `eg.bin` já existe, então a fixture (e as opções) não são aplicadas de novo.
* Execução `"te"` normal (`--keys "91001  C  12  C  "`): `vota:ready`, 10 `vota:state`, um `vota:done`, e `votaTick`
  retorna 0 depois disso.

## 9. Código estranho ou arriscado

1. **Fixture persistente** (funcs 10256/10268 via `votaInit`): o estado só é fabricado se `eg.bin` estiver ausente, e
   um `votaInit` que falhou já o gravou. Qualquer `votaInit` posterior na página ignora suas opções (fase, pe,
   seção, turno). Inofensivo para a página (ela recarrega a cada eleitor), confuso para quem embute o módulo ou para testes.
2. **Tamanho da tela vindo da URL estreitado para 16 bits** (main 10307): `?screenWidth=40000` vira -25536,
   `?screen=70000x800` vira 4464; também não há limite superior (`?screen=30000x30000` pede um canvas de 3,6 GB).
   Um link forjado pode quebrar ou derrubar a aba do simulador da vítima. Nenhum problema de segurança de memória no wasm.
3. **O parsing das opções não é JSON** (14477/11571/11546): vence a primeira correspondência textual de `"key"` (um objeto aninhado
   ou uma chave anterior com o mesmo nome a encobriria), escapes não são decodificados, números não têm verificação de intervalo,
   e `zona`/`secao` são truncados para 16 bits antes da validação (`"zona":70000` vira 4464 e passa em
   `zona < 10000`). As entradas vêm do próprio manifesto de cenários da página, então o impacto é baixo.
4. **`done` = "de volta a `CAguardaMensagem`"** (10619): uma sessão descartada (eleitor suspenso, "NÃO VOTOU") também
   produziria `vota:done` e "Votação concluída" na página. Não alcançável no build web hoje (só o
   operador suspende), mas a lógica da página depende de substrings de nomes de classe do RTTI.
5. **"Gravando" é uma animação** (10619): o eleitor vê a barra de gravação e o "FIM", mas nada é gravado
   (nenhum RDV, nenhuma atualização de `vota.bin`). Esperado para um simulador de treinamento; vale saber ao ler suas telas.
6. **As assinaturas são substitutos** (11733/11818): todo `.vsu` é um texto fixo e toda validação passa
   (com `CWasmSavd`). A segurança é simulada, não reduzida: o caminho real SAVD/HSM está ausente.
7. **`votaGetStateJson` altera o cache de eventos** (10171): fazer polling nele pode suprimir um evento `vota:state`. Isso só
   importa para uma mudança feita fora de `votaTick`, p. ex. `votaSetAudioEnabled` inverte `"audioEnabled"`, que faz parte
   do JSON: um `votaGetStateJson` entre essa chamada e o próximo tick armazena o novo JSON, e o tick então
   não vê mudança e não emite nada. A página nunca chama `votaGetStateJson` (só o executor headless chama).
8. **`CEsperaAudioWasm::Cancela` nunca chama `terminado`** (2680) e o destrutor cancela: um código que descarte o
   handle retornado por `ISound::vf9` perde seu callback de conclusão. O único chamador o mantém.
9. **A ordem de registro é crucial** (10307): se algo obtivesse `IExecucaoVota`,
   `IPoliticaExecucaoEleitor` ou `ISincronismoVotoEleitor` antes de `main` registrá-los, os padrões da urna seriam
   criados (threads reais, `emscripten_sleep` -> abort sem Asyncify, gravações de RDV). O wrapper de registro de `main`
   (o `replace` da u19) então os apagaria e os **destruiria** enquanto chamadores anteriores ainda poderiam manter a referência
   crua que `instance()` retornou (pendente).
10. **Modo de `EscreveArquivo`** (10293): `ios::out | ios::binary` (20), sem `trunc` explícito (o header da u28 diz
    o mesmo); `out` puro trunca de qualquer forma. Só é alcançado com `assina == true`, nunca no build web.

## 10. Questões em aberto

* Se os padrões da fixture (10261, 9952, 10278 e os inlinados) são inicializadores padrão de membros das
  classes md como compiladas para testes, ou tipos do lado do builder com o mesmo layout. Uma restrição: a 10261 chama
  `AssinaturaFicticia` (9963) fora de linha, então as duas estão na mesma unidade de tradução. Se a 10261 for
  o próprio construtor padrão de `md::CDadoCorrespondencia`, a 9963 fica nesse arquivo md e não em um namespace
  anônimo de `cappinfobuilder.cpp` (e a 9952 seria um construtor padrão dentro da biblioteca ecourna).
* O membro em `CAppInfo+528` zerado por `CarregaGeral`.
* Os nomes reais dos slots 2-5 de `IBeep` e dos slots 2-3 de `IEsperaAudio` (inferidos a partir dos chamadores e do comportamento).
* Se os três thunks da fila do eleitor e a 11661 são helpers do código-fonte ou especializações do compilador.
* Por que `CPrng` é construído com o argumento 0 (fonte de entropia `nullptr`? semente padrão?).

## 11. Tabela de mapeamento completa (109 funções)

"executou" = observada executando nos votos gravados. "biblioteca/helper inlinado" = código da biblioteca padrão, do SQLite ou do
OpenSSL, ou uma função gerada pelo compilador, sem código-fonte próprio do TSE.

| func | tamanho | executou | nome da ferramenta | símbolo reconstruído | componente | arquivo original | reconstrução | conf. |
|---:|---:|:-:|---|---|---|---|---|---|
| 2680 | 327 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf2` | `simulador::(anonymous namespace)::CEsperaAudioWasm::Cancela` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | média |
| 8494 | 20 | ✓ | `simulador::CWasmBeep::vf6` | `simulador::CWasmBeep::BeepErro` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | média |
| 8505 | 221 |  | `simulador::CWasmBeep::vf5` | `simulador::CWasmBeep::BeepSuspensao` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | baixa |
| 8514 | 344 |  | `simulador::CWasmBeep::vf4` | `simulador::CWasmBeep::BeepFinalizacao` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | baixa |
| 8523 | 450 |  | `simulador::CWasmBeep::vf3` | `simulador::CWasmBeep::BeepAlerta` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | baixa |
| 8533 | 203 | ✓ | `simulador::CWasmBeep::vf2` | `simulador::CWasmBeep::BeepConfirma` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | média |
| 8538 | 11 | ✓ | `simulador::CWasmBeep::vf1` | `simulador::CWasmBeep::Beep` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | alta |
| 8541 | 6 |  | `simulador::CWasmBeep::vf0` | `simulador::CWasmBeep::SetVolume` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmbeep.cpp` | alta |
| 9445 | 7 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf3` | `simulador::(anonymous namespace)::CEsperaAudioWasm::Cancelada` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | média |
| 9448 | 12 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf1` | `simulador::(anonymous namespace)::CEsperaAudioWasm::~CEsperaAudioWasm (deleting)` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | alta |
| 9459 | 9 |  | `simulador::(anonymous namespace)::CEsperaAudioWasm::vf0` | `simulador::(anonymous namespace)::CEsperaAudioWasm::~CEsperaAudioWasm` | app:simulador | `uenux2/mock/app/simulador/wasm/cwasmwebsound.cpp` | `src/uenux2/mock/app/simulador/wasm/cwasmwebsound.u30.cpp` | alta |
| 9863 | 22 | ✓ | `mock_f9863` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CLocalidadeEleitoral&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 9905 | 69 |  | `mock_f9905` | `std::vector<comum::md::estadoaplicacao::CDadoCorrespondencia>::__vallocate` | rt:libcxx | `libcxx <vector>` | biblioteca/helper inlinado | alta |
| 9952 | 151 | ✓ | `wasm_entry_f9952` | `ecourna::app::dados::CIdentificadorGeradorMidia::CIdentificadorGeradorMidia() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 9963 | 336 | ✓ | `mock_f9963` | `comum::teste::(anonymous namespace)::AssinaturaFicticia` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10067 | 307 |  | `mock_f10067` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralVota&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10089 | 20 | ✓ | `mock_f10089` | `comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralSA, asn::CConversorEstadoGeralSA>::Salva` | app:comum | `uenux2/src/app/comum/appinfo/servicos/iservicoestado.h` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 10101 | 32 |  | `mock_f10101` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralSA&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10112 | 20 | ✓ | `mock_f10112` | `comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralGap, asn::CConversorEstadoGeralGap>::Salva` | app:comum | `uenux2/src/app/comum/appinfo/servicos/iservicoestado.h` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 10123 | 502 | ✓ | `mock_f10123` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeralGap&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10145 | 192 | ✓ | `comum_f10145` | `std::vector<uint8_t>::vector(const std::vector<uint8_t>&)` | rt:libcxx | `libcxx <vector>` | biblioteca/helper inlinado | alta |
| 10155 | 461 | ✓ | `mock_f10155` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CDadoCorrespondencia&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10167 | 32 | ✓ | `mock_f10167` | `comum::teste::(anonymous namespace)::Copia(const md::estadoaplicacao::CDadoCarga&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10171 | 73 | ✓ | `votaGetStateJson` | `votaGetStateJson` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | alta |
| 10176 | 25 |  | `mock_f10176` | `comum::teste::CAppInfoBuilder::SetLocal` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10181 | 18 |  | `mock_f10181` | `comum::teste::CAppInfoBuilder::SetTipoUrna` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10188 | 11 |  | `mock_f10188` | `comum::teste::CAppInfoBuilder::SetTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10197 | 11 |  | `mock_f10197` | `comum::teste::CAppInfoBuilder::SetFase` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10205 | 682 | ✓ | `mock_f10205` | `comum::teste::CAppInfoBuilder::CriaEstado(const md::estadoaplicacao::CEstadoGeral&)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10221 | 12 |  | `wasm_entry_f10221` | `std::vector<comum::teste::EApp>::vector(const std::vector<EApp>&)` | rt:libcxx | `libcxx <vector>` | biblioteca/helper inlinado | alta |
| 10239 | 15 | ✓ | `wasm_entry_f10239` | `comum::teste::CAppInfoBuilder::SalvaAppsSegundoTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10240 | 76 |  | `votaSetAudioEnabled` | `votaSetAudioEnabled` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | alta |
| 10242 | 15 | ✓ | `wasm_entry_f10242` | `comum::teste::CAppInfoBuilder::SalvaAppsPrimeiroTurno` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10256 | 386 | ✓ | `wasm_entry_f10256` | `comum::teste::CAppInfoBuilder::Salva` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10258 | 173 | ✓ | `wasm_entry_f10258` | `std::vector<uint8_t>::vector(size_type n, const uint8_t& v)` | rt:libcxx | `libcxx <vector>` | biblioteca/helper inlinado | alta |
| 10261 | 373 | ✓ | `mock_f10261` | `comum::md::estadoaplicacao::CDadoCorrespondencia::CDadoCorrespondencia() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10268 | 660 | ✓ | `mock_f10268` | `comum::teste::CAppInfoBuilder::CAppInfoBuilder` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10278 | 220 | ✓ | `wasm_entry_f10278` | `comum::md::estadoaplicacao::CEstadoGeralGap::CEstadoGeralGap() (fixture defaults)` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10286 | 258 | ✓ | `wasm_entry_f10286` | `comum::teste::CAppInfoBuilder::TodosApps` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | baixa |
| 10293 | 561 |  | `mock_f10293` | `comum::teste::EscreveArquivo` | app:mock | `uenux2/mock/app/comum/cappinfobuilder.cpp` | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp` | média |
| 10304 | 25 |  | `mock_f10304` | `std::basic_filebuf<char>::open(const std::string&, std::ios_base::openmode)` | rt:libcxx | `libcxx <fstream>` | biblioteca/helper inlinado | alta |
| 10307 | 2480 | ✓ | `main` | `main` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | alta |
| 10376 | 9 | ✓ | `wasm_entry_f10376` | `(anonymous namespace)::EnviaMensagemEleitor<5> (call-site thunk of api_f3903: vote synchronised)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 10619 | 1561 | ✓ | `votaTick` | `votaTick` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | alta |
| 10703 | 102 | ✓ | `votaPressKey` | `votaPressKey` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | alta |
| 10857 | 363 |  | `wasm_entry_f10857` | `(anonymous namespace)::ReportError` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11016 | 166 | ✓ | `wasm_entry_f11016` | `std::basic_stringbuf<char>::str() const` | rt:libcxx | `libcxx <sstream>` | biblioteca/helper inlinado | alta |
| 11026 | 9 | ✓ | `wasm_entry_f11026` | `(anonymous namespace)::EnviaMensagemEleitor<1> (call-site thunk of api_f3903: Eleitor foi habilitado)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11100 | 9 |  | `wasm_entry_f11100` | `(anonymous namespace)::EnviaMensagemEleitor<9> (call-site thunk of api_f3903: voter audio mode)` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11211 | 14 |  | `wasm_entry_f11211` | `std::filesystem::path::path(const char (&)[15])` | rt:libcxx | `libcxx <filesystem>` | biblioteca/helper inlinado | alta |
| 11266 | 9 |  | `wasm_entry_f11266` | `comum::md::estadoaplicacao::CEstadoGeralVota::SetEstadoVota` | app:comum | `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | baixa |
| 11426 | 195 | ✓ | `wasm_entry_f11426` | `std::vector<comum::teste::EMidia>::vector(std::initializer_list<EMidia>)` | rt:libcxx | `libcxx <vector>` | biblioteca/helper inlinado | alta |
| 11546 | 702 |  | `wasm_entry_f11546` | `(anonymous namespace)::JsonString` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11547 | 13 |  | `wasm_entry_f11547` | `comum::CConfiguracaoEleicao::GetCargos lambda (CCargo ordemImpressao <)` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 11550 | 10 |  | `wasm_entry_f11550` | `comum::CConfiguracaoEleicao::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11551 | 37 |  | `wasm_entry_f11551` | `comum::CConfiguracaoEleicao::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/cconfiguracaoeleicao.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11552 | 10 |  | `wasm_entry_f11552` | `comum::CFederacoes::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/cfederacoes.cpp (path inferred)` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | baixa |
| 11553 | 37 |  | `wasm_entry_f11553` | `comum::CFederacoes::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/cfederacoes.cpp (path inferred)` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | baixa |
| 11558 | 105 |  | `wasm_entry_f11558` | `comum::CCargos::OrdenaPorOrdemImpressao lambda` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 11559 | 105 | ✓ | `wasm_entry_f11559` | `comum::CCargos acquisition-order lambda (ordemAquisicao)` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 11560 | 10 |  | `wasm_entry_f11560` | `comum::CCargos::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11561 | 37 |  | `wasm_entry_f11561` | `comum::CCargos::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccargos.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11563 | 10 |  | `wasm_entry_f11563` | `comum::CCandidaturas::GetInst::s_mutex.~mutex [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccandidaturas.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11564 | 37 |  | `wasm_entry_f11564` | `comum::CCandidaturas::GetInst::s_inst.~unique_ptr [atexit]` | app:comum | `uenux2/src/app/comum/dados/ccandidaturas.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11566 | 16 |  | `mock_f11566` | `comum::CServicoEstadoGeralSA::CServicoEstadoGeralSA` | app:comum | `uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | média |
| 11571 | 328 | ✓ | `wasm_entry_f11571` | `(anonymous namespace)::JsonInt` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11572 | 1550 | ✓ | `mock_f11572` | `comum::CAppInfo::CarregaGeral` | app:comum | `uenux2/src/app/comum/appinfo/cappinfo.cpp` | `src/uenux2/src/app/comum/u30-foreign-fragments.cpp` | baixa |
| 11661 | 9 |  | `wasm_entry_f11661` | `(anonymous namespace)::PathDinamicoInterno (call-site thunk of CPath::GetPathDinamico(INTERNA))` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | baixa |
| 11676 | 12 |  | `wasm_entry_f11676` | `vota::CConfereVotoEmCargo<CMajoritarioRepetido>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11677 | 10 |  | `wasm_entry_f11677` | `vota::CConfereVotoEmCargo<CMajoritarioRepetido>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11678 | 12 |  | `wasm_entry_f11678` | `vota::CConfereVotoEmCargo<CMajoritarioNulo>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11679 | 10 |  | `wasm_entry_f11679` | `vota::CConfereVotoEmCargo<CMajoritarioNulo>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11680 | 12 |  | `wasm_entry_f11680` | `vota::CConfereVotoEmCargo<CMajoritarioBranco>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11681 | 10 |  | `wasm_entry_f11681` | `vota::CConfereVotoEmCargo<CMajoritarioBranco>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11685 | 10 |  | `wasm_entry_f11685` | `vota::CPedeMajoritario::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11686 | 12 |  | `wasm_entry_f11686` | `vota::CPedeMajoritario::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11733 | 1207 | ✓ | `wasm_entry_f11733` | `(anonymous namespace)::WriteSimulatedSignatures` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11740 | 12 |  | `wasm_entry_f11740` | `vota::CConfereVotoEmCargo<CProporcionalNulo>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11741 | 10 |  | `wasm_entry_f11741` | `vota::CConfereVotoEmCargo<CProporcionalNulo>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11742 | 12 |  | `wasm_entry_f11742` | `vota::CConfereVotoEmCargo<CCandidatoInapto>::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11743 | 10 |  | `wasm_entry_f11743` | `vota::CConfereVotoEmCargo<CCandidatoInapto>::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11746 | 10 |  | `wasm_entry_f11746` | `vota::CPedeNulo::GetInst::s_mutex.~mutex [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11747 | 12 |  | `wasm_entry_f11747` | `vota::CPedeNulo::GetInst::s_inst.~unique_ptr [atexit]` | app:vota | `uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.cpp` | `src/uenux2/src/app/vota/u30-foreign-fragments.cpp` (gerado pelo compilador; listado) | média |
| 11769 | 640 |  | `wasm_entry_f11769` | `std::__formatter::__escape<char> (std::format "{:?}" escaping)` | rt:libcxx | `libcxx <__format/formatter_output.h>` | biblioteca/helper inlinado | alta |
| 11818 | 512 | ✓ | `wasm_entry_f11818` | `(anonymous namespace)::WriteFileIfMissing` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |
| 11971 | 406 |  | `wasm_entry_f11971` | `std::__unicode::__extended_grapheme_cluster_view<char>::__consume` | rt:libcxx | `libcxx <__format/unicode.h>` | biblioteca/helper inlinado | média |
| 11977 | 133 |  | `wasm_entry_f11977` | `std::__unicode::__extended_grapheme_cluster_view<char>::__extended_grapheme_cluster_view` | rt:libcxx | `libcxx <__format/unicode.h>` | biblioteca/helper inlinado | média |
| 12020 | 33 |  | `wasm_entry_f12020` | `std::optional<std::locale>::operator=(const std::locale&)` | rt:libcxx | `libcxx <optional>` | biblioteca/helper inlinado | alta |
| 12121 | 9 | ✓ | `api_f12121` | `std::filesystem::create_directories(const std::filesystem::path&)` | rt:libcxx | `libcxx <filesystem>` | biblioteca/helper inlinado | alta |
| 12382 | 115 |  | `wasm_entry_f12382` | `jsonCacheDelete` | lib:sqlite | `sqlite3.c (json.c)` | biblioteca/helper inlinado | alta |
| 12433 | 149 |  | `wasm_entry_f12433` | `renameQuotefixExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | biblioteca/helper inlinado | alta |
| 12437 | 163 |  | `wasm_entry_f12437` | `renameTableExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | biblioteca/helper inlinado | alta |
| 12439 | 235 |  | `wasm_entry_f12439` | `renameTableSelectCb` | lib:sqlite | `sqlite3.c (alter.c)` | biblioteca/helper inlinado | alta |
| 12441 | 279 |  | `wasm_entry_f12441` | `renameColumnExprCb` | lib:sqlite | `sqlite3.c (alter.c)` | biblioteca/helper inlinado | alta |
| 13089 | 146 |  | `wasm_entry_f13089` | `cipher_hw_des_cfb8_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13090 | 228 |  | `wasm_entry_f13090` | `cipher_hw_des_cfb1_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13091 | 154 |  | `wasm_entry_f13091` | `cipher_hw_des_cfb64_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13092 | 166 |  | `wasm_entry_f13092` | `cipher_hw_des_ofb64_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13093 | 175 |  | `wasm_entry_f13093` | `cipher_hw_des_cbc_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13094 | 24 |  | `wasm_entry_f13094` | `cipher_hw_des_copyctx` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13095 | 79 |  | `wasm_entry_f13095` | `cipher_hw_des_ecb_cipher` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13096 | 23 |  | `wasm_entry_f13096` | `cipher_hw_des_initkey` | lib:openssl | `providers/implementations/ciphers/cipher_des_hw.c` | biblioteca/helper inlinado | alta |
| 13135 | 738 |  | `wasm_entry_f13135` | `tls1_sha512_final_raw` | lib:openssl | `ssl/s3_cbc.c` | biblioteca/helper inlinado | alta |
| 13136 | 346 |  | `wasm_entry_f13136` | `tls1_sha256_final_raw` | lib:openssl | `ssl/s3_cbc.c` | biblioteca/helper inlinado | alta |
| 13138 | 217 |  | `wasm_entry_f13138` | `tls1_sha1_final_raw` | lib:openssl | `ssl/s3_cbc.c` | biblioteca/helper inlinado | alta |
| 13139 | 174 |  | `wasm_entry_f13139` | `tls1_md5_final_raw` | lib:openssl | `ssl/s3_cbc.c` | biblioteca/helper inlinado | alta |
| 14363 | 48 |  | `wasm_entry_f14363` | `SHA512_Transform` | lib:openssl | `crypto/sha/sha512.c` | biblioteca/helper inlinado | alta |
| 14364 | 11 |  | `wasm_entry_f14364` | `SHA256_Transform` | lib:openssl | `crypto/sha/sha256.c` | biblioteca/helper inlinado | alta |
| 14477 | 378 | ✓ | `wasm_entry_f14477` | `(anonymous namespace)::JsonBool` | app:wasm-entry | `uenux2/wasm/vota_web/vota_web_wasm.cpp` | `src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp` | média |

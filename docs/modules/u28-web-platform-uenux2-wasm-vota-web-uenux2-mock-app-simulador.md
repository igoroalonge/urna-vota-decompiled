# u28 — plataforma web (1 de 4): o SAVD web, a política web de limpeza do teclado, o fixture dos arquivos de estado, dois destrutores de base

Unidade u28 da engenharia reversa de `vota_web_wasm.wasm` (aplicação de votação VOTA do TSE, `uenux2` + `ecourna`,
build Emscripten do simulador público de treinamento). É a menor das quatro unidades de "plataforma web"
(u28–u31 compartilham `uenux2/wasm/vota_web` e `uenux2/mock/app/…`): **7 funções, 3 delas vistas em execução** durante
os votos gravados. Os índices de função são índices de função wasm (`python3 tools/wasmmap/q.py f <n>`).

Arquivos-fonte reconstruídos (fragmentos, porque o restante de cada arquivo pertence a outra unidade):

| arquivo | conteúdo |
|---|---|
| [`src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp`](../../src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp) | `(anonymous)::CWasmSavd`, `(anonymous)::CPoliticaExecucaoEleitorWeb` |
| [`src/uenux2/mock/app/comum/cappinfobuilder.u28.h`](../../src/uenux2/mock/app/comum/cappinfobuilder.u28.h) / [`.u28.cpp`](../../src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp) | `comum::teste::CAppInfoBuilder::SalvaGeral / SalvaApps / SalvaApp`, `Converte(EUrnaTurno)` |
| [`src/uenux2/src/app/comum/iinterfaceinit.u28.cpp`](../../src/uenux2/src/app/comum/iinterfaceinit.u28.cpp) | `comum::IInterfaceInit::~IInterfaceInit` |
| [`src/uenux2/src/api/gui/iscreen.u28.h`](../../src/uenux2/src/api/gui/iscreen.u28.h) | `api::IScreen::~IScreen` (documentado; o corpo já está em `iscreen.h`, u17) |

Capítulos relacionados: [u23](u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md) §9 (o protocolo
cliente do SAVD que `CWasmSavd` responde), [u06](u06-uenux2-src-app-vota-eleitor.md) (o
`CPoliticaExecucaoEleitor` da urna), [u19](u19-uenux2-src-api-pattern-cpolysingletonlist-h.md) §2.3 (o que `main` registra),
[u03](u03-uenux2-src-app-comum-dados.md) §6 (conteúdo de `eg.bin`/`gap.bin`/`sa.bin`/`vota.bin`),
docs/03-js-wasm-interface.md §10 (a fila do teclado), docs/bu/codepath.md.

## 0. Termos

| termo | significado |
|---|---|
| SAVD | o serviço de assinatura / validação de assinaturas da urna (um daemon separado na máquina real). A aplicação nunca detém chaves: ela envia requisições binárias (assinar este arquivo, abrir a sessão do HSM, validar este pacote) por meio de `comum::IInterfaceSavd` |
| `.vsu` | arquivo de assinatura de um arquivo dinâmico (`vota.vsu`, `gap.vsu`, `eg.vsu`, …), gravado pelo SAVD na urna |
| *assinatura simulada* | o próprio termo, usado no texto gravado pelo simulador |
| *política de execução do eleitor* | um singleton substituível com um método, `LimpaBufferInput` ("limpa o buffer de entrada") |
| *CORRIGE* / *CONFIRMA* | as teclas laranja (corrige) e verde (confirma) do teclado do eleitor |
| *tela de confirmação* | a tela mostrada depois que o eleitor digita um número (foto do candidato, nome, partido) |
| *estado geral* | os arquivos de estado persistentes da urna `eg.bin` (urna), `gap.bin` (lançador / GAP), `sa.bin` (aplicação SA), `vota.bin` (aplicação VOTA), ASN.1 codificado em BER (`ModuloEstadoGeral*`) |
| *trab1* / *trab2* | diretório de trabalho do 1º / 2º *turno* dentro de `dinamico/` |
| MI / MV, *flash interna* / *externa* | flash interna `/dsk/fi` e flash externa (memória de votação) `/dsk/fe` |
| *builder* / *teste* | `comum::teste::CAppInfoBuilder` é um fixture de teste da base de código da urna |
| *mídia*, *turno*, *app* | mídia (qual flash), turno, aplicação |

## 1. O que o subsistema faz

Esta unidade reúne as peças da plataforma web que ficam **entre a aplicação de votação e o hardware/os serviços
ausentes**, além de dois destrutores de interfaces de hardware:

1. **`CWasmSavd`** substitui o daemon SAVD. Toda requisição de assinatura e toda verificação de assinatura da
   aplicação passa por ele, e ele responde "OK" a todas sem fazer nada. Observado em toda
   execução, e somente dentro de `votaInit`: `SalvaEstado` (491) assina os arquivos de estado por meio de `AssinarUE`, e
   `CPacoteArquivos::ValidarChaveEAplicacaoValida` (4625) verifica os pacotes de dados da eleição. Uma linha do tempo de
   perfil de CPU de um voto completo (`--cpu-prof`, `--keys "91001  C  12  C  "`) não mostra nenhuma chamada ao SAVD depois de `votaInit`: o
   `CSincronismoVotoEleitorWeb` web não grava nem `vota.bin` nem `rdv.dat`, então nada é assinado quando um voto é confirmado.
2. **`CPoliticaExecucaoEleitorWeb`** substitui a política da thread do eleitor que limpa o teclado depois de CORRIGE em uma
   tela de confirmação. A versão da urna espera entre as limpezas com `sleep_for`, o que abortaria este build.
3. **`CAppInfoBuilder::SalvaGeral / SalvaApps`** fabricam o estado persistente da urna (`eg.bin`, e
   `gap.bin`/`sa.bin`/`vota.bin` dos dois turnos nas duas flashes) durante `votaInit`. Em uma urna real esses arquivos vêm
   da carga (preparação das mídias) e das próprias aplicações.
4. `IScreen::~IScreen` e `IInterfaceInit::~IInterfaceInit`: destrutores de classes base. As implementações web
   (`CWasmScreen`, `CWasmInit`) os reutilizam sem alteração (os destrutores delas são aliases).

## 2. Classes (RTTI) e layouts

```
comum::IInterfaceSavd (vtable @1526736, typeinfo @1526720)                     iinterfacesavd.cpp (u23)
  └ (anonymous namespace)::CWasmSavd (vtable @1526688, typeinfo @1526708)       vota_web_wasm.cpp (path inferred)
      20 bytes = base members only: +0 vptr, +4 std::string m_mensagem, +16 ueint32 m_codigoErro
      [0] 5503 ~IInterfaceSavd   [1] 10965 deleting dtor   [2] 425 EnviaMensagem = no-op (ICF)
      [3] 10949 RecebeMensagem   [4] 10930 slot 4 (meaning unknown) -> 0x0CABECA0

vota::impl::IPoliticaExecucaoEleitor
  ├ vota::impl::CPoliticaExecucaoEleitor         (urna; cpoliticaexecucaoeleitor.cpp, func 13564, u06)
  └ (anonymous namespace)::CPoliticaExecucaoEleitorWeb (vtable @1527156, typeinfo @1527168)  vota_web_wasm.cpp:361
      4 bytes (vptr only)   [0] 174 trivial dtor   [1] 144 operator delete   [2] 10835 LimpaBufferInput

api::IScreen (vtable @1529100)                  └ simulador::CWasmScreen (vtable @1528824)   slot 0 = 5071 for both
comum::IInterfaceInit (vtable @1552160)         └ simulador::CWasmInit   (vtable @1528772)   slot 0 = 5898 for both
      IInterfaceInit: +4/+8 std::shared_ptr<bool> m_demoMode, +12 std::string m_serialMR (u23 names)

comum::teste::CAppInfoBuilder (no RTTI, no virtuals; 500 bytes; cappinfobuilder.cpp)
      +0   md::estadoaplicacao::CEstadoGeral      m_geral    180 B  -> dinamico/eg.bin
      +180 md::estadoaplicacao::CEstadoGeralGap   m_gap[2]    44 B  -> trab1|2/gap.bin
      +268 md::estadoaplicacao::CEstadoGeralSA    m_sa[2]     16 B  -> trab1|2/sa.bin
      +300 md::estadoaplicacao::CEstadoGeralVota  m_vota[2]  100 B  -> trab1|2/vota.bin
```

Por que `CWasmSavd` foi colocado em `vota_web_wasm.cpp`: classes de namespace anônimo têm ligação interna, e `main`
(func 10307) grava a vtable dela (`operator new(20)`, preenchimento com zeros, vptr @1526688, depois
`CPolySingletonList::push<comum::IInterfaceSavd>`, func 10090). `main` também grava a vtable de
`CPoliticaExecucaoEleitorWeb`, cujo método tem um srcloc em `vota_web_wasm.cpp`, então `main`, as duas classes e
`CSincronismoVotoEleitorWeb` estão na mesma unidade de tradução.

Os tipos dos membros do builder são deduzidos de tamanhos e destrutores: o construtor do builder (func 10268, u30) e
os temporários de `SalvaGeral`/`SalvaApps` usam os mesmos destrutores (2793 para o objeto EG, 1698 para o objeto
GAP), e os tamanhos são iguais aos das classes md (`CAppInfo` os guarda em cache em `std::optional`s de 184/48/104
bytes, u20).

## 3. Fluxo de controle

### 3.1 Uma troca com o SAVD (func 10949)

```
client (IInterfaceSavd::EnviaRequisicao 3830 / EnviaComando 3829)          CWasmSavd
  EnviaMensagem({0xFE, aplicação, u16 comando, u32 tamanho})   ─────────►   slot 2: nothing
  EnviaMensagem(payload)                                       ─────────►   slot 2: nothing
  RecebeMensagem(estado, header, 12)                           ─────────►   slot 3 (func 10949):
                                                                              estado = 0x0CABECA0
                                                                              tamanho != 12 ? throw std::invalid_argument(
                                                                                "CWasmSavd::RecebeMensagem - mensagem de tamanho invalido")
                                                                              header = FE 00 00 00 | 00 00 00 00 | 00 00 00 00
  DesconverteHeader: size 12 ✓, marca 0xFE ✓, erro 0 → success, no second read
```

Portanto todo comando é bem-sucedido: `0x0042` assinar um arquivo dinâmico (`AssinarUE`, chamado por `CAssinador::Assina`),
`0x0080` assinar um arquivo de resultado (`AssinarEcourna`, o `-vota.vsc` do BU), `0x1604` abrir/fechar sessão do HSM,
`0x0404` definir o "local" da seção, verificações de assinatura de pacote / arquivo: `CPacoteArquivos::ValidarChaveEAplicacaoValida`
(4625: uma requisição de pacote quando o comando é `0x2021`/`0x1011`; com `0x1001` uma requisição por arquivo por meio da
func 5892 → 3830, que é o caminho que executou na inicialização), `ValidarUE` (5890, `0x2021`, a partir de
`VerificaAssinaturaMI/MV`; não visto em execução) e `CSigVerifier` (11179). A requisição nunca é inspecionada, e o SAVD
não grava nenhum arquivo. Os arquivos `.vsu` do simulador são criados por `votaInit` (funcs 11733/11818) com o texto
`assinatura simulada para vota_web_wasm` (verificado em `analysis/runtime/memfs-after-init`).

O parâmetro de saída `estado` é uma variável local não inicializada nos dois clientes e nunca é lido depois. O slot 4 (func
10930) devolve a mesma constante e não tem chamador. O significado deles na urna (tipo de mensagem, id de canal?) não pode ser
recuperado a partir deste build. `0x0CABECA0` se lê como hex-speak "cabeça".

### 3.2 CORRIGE em uma tela de confirmação (func 10835)

Verificado em tempo de execução envolvendo o import `wasm_input_clear` com um stack trace
(`node --import <hook> tools/run/headless.mjs --named --keys "91001  D  "`):

```
wasm_input_clear  (JS: Module.uenuxKeys = [])
 ← icf_tiny_vf4@5013             (simulador::CWasmInputKbd::Flush, IInput slot 4)
 ← (anonymous)::CPoliticaExecucaoEleitorWeb::LimpaBufferInput     func 10835
 ← vota::CConfirmaVotoNominal::vf17
 ← vota::CVotacaoStateAudio::EmiteEcoCorrigeConfirma
 ← vota::CVotacaoStateAudio::ProcessInput ← vota::CEleitorVotando::vf7
```

Nessa sessão a fila foi limpa 6 vezes, contra 4 sem o `D` (reverificado nesta revisão com o mesmo
hook). As outras limpezas são uma limpeza direta de `IInputKbd` em `CEleitorVotando::IniciaCiclo` (a primeira) e
`CInteractiveForm::ClearKeyboardInput` (func 11077) quando uma tela começa.

| | urna (`CPoliticaExecucaoEleitor`, func 13564) | web (`CPoliticaExecucaoEleitorWeb`, func 10835) |
|---|---|---|
| limpezas | `IRng::Gera() % 4 + 3` → 3..6 (resto com sinal: um `Gera()` negativo dá 0..2, u06) | 1 |
| espera | um atraso sorteado antes do laço, `(IRng::Gera() % 100 + 50) & 0xFF` ms → 50..149 ms, dormido depois de **cada** limpeza (`sleep_for` → `if (byte@1584624 == 1) emscripten_sleep(ms)`) | nenhuma |
| thread do eleitor depois de CORRIGE | bloqueada por cerca de 0.15–0.9 s; teclas pressionadas nesse meio-tempo são descartadas | continua imediatamente |
| próxima tela | aparece depois desse atraso; o `ClearKeyboardInput` dela limpa de novo | aparece imediatamente; o `ClearKeyboardInput` dela limpa (a 6ª limpeza acima) |
| neste build | chamaria `emscripten_sleep` → abort (sem ASYNCIFY) | seguro |

`IPoliticaExecucaoEleitor::GetInst()` registra a política da urna de forma preguiçosa **somente se não existir nenhuma implementação**.
Como `main` registra a política web primeiro, a versão que aborta nunca é alcançada.

### 3.3 O fixture dos arquivos de estado em `votaInit` (funcs 10243, 10212)

`votaInit` (func 7840, u30) monta um `CAppInfoBuilder` com os valores do fixture e chama
`wasm_entry_f10256(builder, assina = false, midias = {0, 1})`, que encadeia:

```
SalvaGeral(false, {FlashInterna, FlashExterna})                                   func 10243
  .SalvaApps(false, {…}, '1', {Gap, SA, Vota})   via thunk 10242 → 6004(…, 49)       func 10212
  .SalvaApps(false, {…}, '2', {Gap, SA, Vota})   via thunk 10239 → 6004(…, 50)
```

(`6004` copia os dois vetores e chama `SalvaApps`. `10242`/`10239` diferem apenas na constante do turno, e é por isso que
o wasm-opt as mesclou. `10286` monta a lista de apps `{0, 1, 2}` a cada vez.)

`SalvaGeral`, para cada mídia:

1. `origem = Converte(midia)` (func 5344): midia ∉ {0, 1} (um único teste sem sinal, `i32.ge_u 2`, portanto valores
   negativos também) → `std::logic_error("Converte - midia invalida [{}] da linha [{}]")`; 0/1 são devolvidos sem alteração;
2. se `assina` (nunca aqui): grava o texto `"assinatura EG"` em `<flash>/dinamico/eg.vsu`;
3. `CServicoEstadoGeral(origem).Salva(CriaEstado(m_geral))`: reconstrói o `CEstadoGeral` por meio do seu construtor
   campo a campo (10205 → 5637), codifica-o em BER com `CConversorEstadoGeral` e o grava com `CFileASN::WriteToFile` em
   `<flash>/dinamico/eg.bin` (2894).

`SalvaApps`, para cada mídia (`Converte(midia)`, srcloc :277) e cada app, o `SalvaApp` inlinado:

1. `idx = Converte(turno)` (srcloc :287): `'1'` → 0, `'2'` → 1, qualquer outro valor →
   `std::logic_error("Converte - turno invalido [{}] da linha [{}]")` (turno impresso como inteiro);
2. `trab = CPath::GetPathTrab(origem, turno)` → `<flash>/dinamico/trab<turno>/`;
3. `switch (app)`: `Gap` → (`gap.vsu` "assinatura EG Gap" se assina) `CServicoEstadoGeralGap(origem, turno).Salva(CriaEstado(m_gap[idx]))`;
   `SA` → `sa.vsu` "assinatura EG SA", `sa.bin`; `Vota` → `vota.vsu` "assinatura EG Vota", `vota.bin`; outros
   valores: nada.

Resultado: 14 arquivos, exatamente os arquivos `.bin` de `analysis/runtime/memfs-after-init` (tamanhos: eg 396, gap 60, sa 19,
vota 56 em trab1 depois que a aplicação o regravou, 37 em trab2). Os conteúdos são decodificados em u03 §6. As
strings legíveis de `eg.bin` são `nome_maquina`, `12345678`, `99999999`, `20201231T235958`,
`1234567890123456789012340` do fixture e a versão `7.2.1.3 - TESTE EG ASN1`; `gap.bin` traz `dataSegundoTurno 20801231`.

### 3.4 Destrutores (funcs 5071, 5898)

* `~IScreen()` = `IForm<IScreen>::RemoveAll()` (func 5069): esvazia a pilha estática de formulários de tela do eleitor.
* `~IInterfaceInit()` = gerado pelo compilador: `~string(m_serialMR)`, `~shared_ptr<bool>(m_demoMode)`.

Nenhum dos dois executou nas sessões gravadas. Os singletons vivem até a página ser recarregada, e o build web atende um
eleitor por carregamento de página.

## 4. Dados lidos / gravados

| dado | direção | função | formato |
|---|---|---|---|
| header + payload da requisição ao SAVD | gravado pelo cliente, **descartado** | slot 2 (ICF 425) | `{0xFE, aplic, u16 cmd, u32 len}` + payload (u23 §9) |
| header da resposta do SAVD | produzido | 10949 | 12 bytes `FE 00 00 00 00 00 00 00 00 00 00 00` |
| `/dsk/fi|fe/dinamico/eg.bin` | gravado | 10243 → 2894 | BER `ModuloEstadoGeralUrna` via `CConversorEstadoGeral` |
| `/dsk/fi|fe/dinamico/trab{1,2}/gap.bin` | gravado | 10212 → 10112 → 2894 | BER `ModuloEstadoGeralGap` |
| `/dsk/fi|fe/dinamico/trab{1,2}/sa.bin` | gravado | 10212 → 10089 → 2894 | BER `ModuloEstadoGeralSA` |
| `/dsk/fi|fe/dinamico/trab{1,2}/vota.bin` | gravado | 10212 → 5329 → 2894 | BER `ModuloEstadoGeralVota` |
| `…/eg.vsu`, `…/trabN/{gap,sa,vota}.vsu` | seria gravado (somente com `assina == true`; nunca neste build) | 10293 | texto puro `assinatura EG[ Gap| SA| Vota]` |
| fila do teclado `Module.uenuxKeys` | limpa | 10835 → 5013 → import `wasm_input_clear` | array JS |

Nada de SQL, nada de rede, e nenhum dado da página é analisado aqui. Os valores do builder vêm do JSON de `votaInit`
(`fase`: `"oficial"`/`"o"` → '1', `"simulado"`/`"s"` → '2', senão '3'; `turno`; `municipio`/`zona`/`secao`),
que é analisado em u30.

## 5. O que é específico do build web

* **Assinatura/validação simulada**: `CWasmSavd` (§3.1). Toda verificação de integridade que passa pelo SAVD é aprovada.
* **Limpeza simplificada do teclado**: `CPoliticaExecucaoEleitorWeb` (§3.2). Foi alterada por causa da ausência de
  ASYNCIFY, e também altera o comportamento de temporização.
* **Estado fabricado**: os arquivos de estado vêm de um fixture de teste (`uenux2/mock/`, namespace `comum::teste`), não
  de uma carga. O próprio ramo de "assinatura" do fixture (`assina`) está presente mas desativado. A inicialização web grava um
  texto falso diferente nos arquivos `.vsu`.
* **Destrutores de base reutilizados**: os mocks `CWasmScreen` e `CWasmInit` não acrescentam nenhum membro destrutível.

## 6. Boletim de urna (BU)

Esta unidade não monta o BU. Ela toca a cadeia do BU em um ponto; um segundo (a verificação de impressão) está listado
porque **não** passa por este mock:

* **Assinatura dos arquivos de resultado.** No encerramento, `CAssinador::AssinaArquivosResultado` / `AssinarEcourna` (inlinados
  em `CGravaResultado`, func 12098) enviariam uma requisição `0x0080` por arquivo de resultado (`-bu.dat`, `-rdv.dat`,
  `-imgbu.dat`, …) ao SAVD, que deveria acrescentar as assinaturas a `…-vota.vsc`. Com `CWasmSavd` cada requisição recebe
  o header de OK e **nenhum conteúdo de `.vsc` é produzido**. `docs/bu/codepath.md` §8.4 mostra que esta etapa não é alcançada
  no simulador de qualquer forma.
* **Verificação de impressão (não passa por este mock).** `ImprimeBU` (func 2890) não chama o SAVD diretamente: ela entrega
  ao slot 8 de `IPaperRelatorios` a imagem `trab/bu.dat` mais um `CSigVerifier(trab, "bu.dat", "bu.vsu")`, e na urna
  o serviço de impressora executa esse verificador (que alcançaria o SAVD por 11179 → 5892 → 3830). No build web
  o papel é `simulador::CWasmNullPaper`, cujo slot 8 (func 8371) apenas libera o `shared_ptr` do verificador, então
  a verificação nunca é avaliada (u31 §11 e §13 #8); `CWasmSavd` não está envolvido.

Identidade da seção: município/zona/seção do cabeçalho do BU vêm de `CLocal`, que obtém a tripla de `eg.bin` e
carrega `<município><zona><seção>-lo.dat` com ela (`CLocal::Carrega`, func 5740, u18). No simulador `eg.bin` é o
fixture gravado por `SalvaGeral` (a partir dos valores de cenário passados a `votaInit`).

## 7. Observações sobre wasm / Emscripten

* **Aliasing de destrutores.** O slot 0 de `CWasmScreen`/`CWasmInit` aponta para o destrutor de base (5071/5898), e apenas
  os destrutores de exclusão (deleting) são funções separadas: 9388 é `free(~IInterfaceInit(this))`, enquanto 8963 inlina o
  pequeno corpo de `~IScreen` (vptr de IScreen, `RemoveAll` 5069) e depois libera. `CWasmSavd` faz o mesmo que `CWasmInit`
  (slot 0 = 5503, slot 1 = 10965 = `free(~IInterfaceSavd(this))`).
* **ICF.** `CWasmSavd::EnviaMensagem` é o corpo vazio compartilhado 425. `CWasmInputKbd::Flush` e `CWasmInputMT::Flush`
  são uma única função, 5013.
* **`source_location` como argumento padrão.** Os três srclocs de `cappinfobuilder.cpp` pertencem aos pontos de chamada de
  Converte, não às funções Converte. Depois do inlining, `Converte(turno)` mantém apenas `loc.line()`: um
  `i32.load 1528068` do campo de linha do registro estático (= 287). O `Converte(midia)` fora de linha (5344) recebe o
  ponteiro do registro e o testa contra nulo (`loc.line()` devolve 0 para um `source_location` nulo).
* **Strings de formato escondidas em constantes i64.** `std::string_view{ptr, len}` é materializado como um único `i64.const`:
  `188978796328` = `{235304, 44}` = `"Converte - turno invalido [{}] da linha [{}]"`, e `188978796373` =
  `{235349, 44}` = `"Converte - midia invalida [{}] da linha [{}]"`. É por isso que `q.py grep` não lista nenhuma referência para
  essas strings. Argumentos enum são empacotados como `handle` (tipo 15) com os slots de formatador 474/468 (formatação
  de inteiros).
* **Inlining.** `SalvaApp` (um membro com seu próprio srcloc) existe apenas dentro de `SalvaApps` (2,747 bytes). Toda chamada
  nele passa por `invoke_*` porque cada temporário (`std::filesystem::path`, `std::string`, o objeto md
  reconstruído) precisa de um landing pad.
* **Ordem de argumentos do objeto de serviço.** Em `CServicoEstadoGeralGap(o, t).Salva(CriaEstado(x))` o serviço é
  construído antes do argumento. Esse é o sequenciamento da expressão pós-fixa no C++17, e ele é visível na
  ordem de chamadas 5812 → 10123 → 10112.
* O registro (`TPolySingletonsInfo`) é obtido por uma chamada indireta através do ponteiro de função @1526320 (slot
  de tabela 284 → func 11265, u19). É o mesmo acessor para toda interface (`IInputKbd` aqui, `IRng` em 13564,
  `IPoliticaExecucaoEleitor` em 5925); a interface é escolhida pela instanciação de `instance` (455 = `IInputKbd`).

## 8. Questões em aberto

* Significado do slot 4 de `IInterfaceSavd` e do primeiro parâmetro (`ueint32&`) de `RecebeMensagem` na urna real.
  Neste build os dois carregam `0x0CABECA0`, e nada os lê.
* Se `EMidia`/`EApp` ficam em `comum` ou em `comum::teste`. O srcloc os imprime sem qualificação.
* Os nomes exatos dos helpers de reconstrução do builder (10205/10123/10101/10067, u30) e do gravador de arquivo 10293.

## 9. Código estranho ou arriscado

| # | func | o quê | impacto |
|---|---|---|---|
| 1 | 10949 | **O SAVD diz amém a tudo.** Toda requisição (assinar, HSM, validar) é respondida com "OK" sem ser lida, então a validação na inicialização dos pacotes de dados da eleição (`CPacoteArquivos::ValidarChaveEAplicacaoValida`, 4625, observada; `ValidarUE` se alcançada) e toda assinatura `.vsu`/`.vsc` são simuladas. (A verificação do BU impresso contra `bu.vsu` nem sequer chega a este mock: o papel nulo nunca executa o seu `CSigVerifier`, §6) | somente no simulador. Dados de cenário modificados seriam aceitos, e nada no build web demonstra as verificações de integridade reais |
| 2 | 10835 | limpeza web do teclado = 1 limpeza sem atraso, onde a urna faz 3–6 limpezas, cada uma seguida da mesma pausa aleatória de 50–149 ms (para números aleatórios não negativos) | depois de CORRIGE em uma tela de confirmação o simulador mostra a próxima tela imediatamente, enquanto a thread do eleitor da urna pausa 0.15–0.9 s e descarta as teclas pressionadas nesse meio-tempo. É uma pequena diferença de temporização em uma ferramenta de treinamento. A versão da urna abortaria este build (`emscripten_sleep`), e só a ordem de registro em `main` impede que ela execute |
| 3 | 10212, 10243 | código de fixture de teste (`comum::teste::CAppInfoBuilder`) cria o estado persistente da urna; os ramos `assina` dele gravariam ainda outro texto de assinatura falso (`"assinatura EG Gap/SA/Vota"`, `"assinatura EG"`) | morto neste build (`votaInit` passa `false`). Informativo: o "estado da urna" no simulador é fixo no código |
| 4 | 10949 | grava o parâmetro de saída antes de validar, e lança `std::invalid_argument` (não um erro do TSE) para qualquer tamanho ≠ 12. O código cliente só espera erros do protocolo do TSE | inalcançável: o mock nunca reporta erro, então o cliente nunca pede o corpo da mensagem |
| 5 | 10212 | `Converte(turno)` aceita apenas `'1'`/`'2'` (lança `std::logic_error` para `'0'`/`'3'`), e valores desconhecidos de `EApp` são ignorados silenciosamente | inalcançável com os argumentos que `votaInit` usa |
| 6 | 10930 | o slot 4 de `IInterfaceSavd` não tem chamador e devolve uma constante mágica | código morto |

## 10. Tabela de mapeamento (todas as 7 funções da unidade)

`ran` = vista em execução nos votos gravados (`analysis/runtime/vote_*.functions.tsv`).

| func | bytes | ran | nome nas ferramentas | símbolo reconstruído | arquivo original | reconstrução | conf. |
|---|---|---|---|---|---|---|---|
| 5071 | 17 |  | `api::IScreen::vf0` | `api::IScreen::~IScreen()` (dtor completo; também slot 0 de `simulador::CWasmScreen`) | uenux2/src/api/gui/iscreen.h | src/uenux2/src/api/gui/iscreen.u28.h (corpo já inline em iscreen.h, u17) | alta |
| 5898 | 91 |  | `comum::IInterfaceInit::vf0` | `comum::IInterfaceInit::~IInterfaceInit()` (também slot 0 de `simulador::CWasmInit`) | uenux2/src/app/comum/iinterfaceinit.cpp | src/uenux2/src/app/comum/iinterfaceinit.u28.cpp | alta |
| 10212 | 2747 | ✔ | `comum::teste::CAppInfoBuilder::SalvaApps` | `comum::teste::CAppInfoBuilder::SalvaApps` (+ `SalvaApp` :287 e `Converte(EUrnaTurno)` inlinados) | uenux2/mock/app/comum/cappinfobuilder.cpp | src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp | alta |
| 10243 | 598 | ✔ | `comum::teste::CAppInfoBuilder::SalvaGeral` | `comum::teste::CAppInfoBuilder::SalvaGeral` | uenux2/mock/app/comum/cappinfobuilder.cpp | src/uenux2/mock/app/comum/cappinfobuilder.u28.cpp | alta |
| 10835 | 62 |  | `(anonymous namespace)::CPoliticaExecucaoEleitorWeb::LimpaBufferInput` | o mesmo (srcloc vota_web_wasm.cpp:361) | uenux2/wasm/vota_web/vota_web_wasm.cpp | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | alta |
| 10930 | 8 |  | `(anonymous namespace)::CWasmSavd::vf4` | override de `CWasmSavd` do slot 4 de `IInterfaceSavd` (`Slot4()` no header da u23; significado desconhecido, devolve `0x0CABECA0`) | uenux2/wasm/vota_web/vota_web_wasm.cpp (caminho inferido) | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | baixa (nome) |
| 10949 | 160 | ✔ | `(anonymous namespace)::CWasmSavd::RecebeMensagem` | `(anonymous namespace)::CWasmSavd::RecebeMensagem(ueint32&, std::vector<uebyte>&, size_t)` | uenux2/wasm/vota_web/vota_web_wasm.cpp (caminho inferido) | src/uenux2/wasm/vota_web/vota_web_wasm.u28.cpp | alta |

Funções de outras unidades referenciadas acima (para navegação, não fazem parte da u28): 425 (ICF sem efeito, `CWasmSavd::EnviaMensagem`),
5503/10965 (`~IInterfaceSavd` / dtor de exclusão de `CWasmSavd`, u23), 3829/3830 (cliente do SAVD, u23), 5013 (`CWasmInputKbd::Flush`),
13564 (`LimpaBufferInput` da urna, u06), 5069 (`IForm<IScreen>::RemoveAll`), 8963/9388 (dtors de exclusão de `CWasmScreen`/`CWasmInit`),
10307 `main`, 7840 `votaInit`, 10256/6004/10242/10239/10286 (driver do builder), 10268 (ctor do builder), 5344 (`Converte(EMidia)`),
10293 (gravador de arquivo texto), 10205/10123/10101/10067 (reconstroem os estados md), 2894 (corpo de `IServicoEstado<…>::Salva`),
3897/5812/11566/3787/1941 (construtores de serviço), 3592/10112/10089/5329 (thunks de `Salva`), 358/1082 (`CPath::GetPathTrab/GetPathDinamico`),
5968 (`path operator/`), 10134/11637/10078 (`path(const char(&)[N])`), 9868 (handle do formatador de enum).

## 11. Revisão de fidelidade (2026-09-23)

Todas as 7 funções foram comparadas com o código descompilado e o WAT, junto com os vizinhos em que o texto se apoia
(425, 5503, 10965, 3829, 3830, 5344, 10256, 6004, 10242, 10239, 10286, 13564, 5013, 5069, 8963, 9388, 5925, 10293).
Condições de desvio, constantes, strings, ordem das chamadas e offsets de struct das reconstruções conferem. Correções:

* §1 / `vota_web_wasm.u28.cpp`: o SAVD executa somente dentro de `votaInit`. Uma linha do tempo `--cpu-prof` de um voto completo
  não tem nenhuma chamada ao SAVD depois do voto, então a afirmação de que `SalvaEstado` assina `vota.bin`/`rdv.dat` depois de cada voto foi removida.
* §6 / §9 #1: a verificação do BU impresso contra `bu.vsu` não passa por `CWasmSavd`. O `CSigVerifier` é entregue
  ao papel nulo, que nunca o executa.
* §3.1: a validação na inicialização que foi observada é `CPacoteArquivos::ValidarChaveEAplicacaoValida` (4625 → 5892 →
  3830). `ValidarUE` (5890) não executou.
* §3.2: o atraso da urna é sorteado uma vez e dormido depois de cada limpeza. Os restos são com sinal. A primeira limpeza de uma
  sessão vem de `CEleitorVotando::IniciaCiclo`, não de `ClearKeyboardInput`. A contagem de 6 contra 4 limpezas foi
  reproduzida.
* §3.3: `Converte(EMidia)` usa um teste sem sinal, então todo valor fora de {0, 1} lança exceção.
* Header/comentários: `EscreveArquivo` abre com `out|binary` (modo 20) depois de `create_directories`, não `out|trunc`.
  `CEstadoGeral` também tem um subobjeto em +60 (dtor 857). `RemoveAll` também desbloqueia o resíduo do mutex de formulário em +28.
  O `EUrnaTurno` declarado em `cpath.h` (char) conflita com o de `cappinfo.h` (int). O wasm sustenta o
  de tamanho int.

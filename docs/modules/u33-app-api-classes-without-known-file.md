# u33: classes de `app:api` sem arquivo conhecido (os mocks de hardware do simulador, a classe base de TTS, `mr.ver` e helpers avulsos)

A unidade u33 tem 72 funções wasm que as ferramentas atribuíram ao componente `app:api` mas não conseguiram situar
em um arquivo-fonte. As ferramentas as agruparam como cinco classes (`api::teste::CUrnaMock`,
`api::teste::CPowerMock`, `api::ITextToSpeech`, `api::Logger`, `comum::CGravadorVersoesArquivos`) e
três grupos de "funções livres". Depois que cada função é lida, a unidade se revela composta de quatro coisas
diferentes:

| grupo | funções | o que são |
|---|---:|---|
| **Mocks de hardware do build web** | 15 | `api::teste::CUrnaMock` (a identidade da urna e suas tabelas secretas) e `api::teste::CPowerMock` (fonte de alimentação / bateria) |
| **Text-to-speech e arquivos de resultado** | 11 | `api::ITextToSpeech` slots 1-9 (velocidade da fala, parâmetros de voz, destrutor), `api::Logger::log` (o log de erros do RHVoice) e `comum::CGravadorVersoesArquivos::GravaResultado`, o gravador de **`mr.ver`**, um dos arquivos de resultado do encerramento |
| **Helpers avulsos do TSE** | 29 | funções de `api/gui`, `api/pattern`, `api/util`, `api/ipc`, `comum` e `vota` cujos chamadores estão espalhados por vários arquivos: helpers do construtor de formulários, instâncias do registro de poly-singletons, helpers de strings, singletons preguiçosos de estados do operador (mesário), e **corpos mesclados** que o wasm-opt compartilhou entre classes |
| **Código de biblioteca** | 17 | 10 funções do RHVoice 1.14.0 (consultas a tabelas Unicode, scanner de emoji, cursor do dicionário do usuário, `fst::translate` …), 5 templates da libc++ instanciados para tipos do RHVoice, e 2 outras instanciações da libc++ |

11 das 72 funções executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`): 1005, 1562
(registros na inicialização), 1840, 2199, 2792, 3266, 3684 (inicialização), 2779, 3058, 3889 (telas do eleitor) e
3903 (as mensagens que `votaInit`/`votaTick` postam para a thread do eleitor).

Arquivos-fonte reconstruídos (os arquivos `.u33.*` são fragmentos de arquivos pertencentes a outras unidades; devem ser incorporados lá):

```
src/uenux2/src/api/hwil/iurna.h                                   IUrna interface (path inferred)
src/uenux2/mock/api/hwil/curnamock.h                              api::teste::CUrnaMock (path inferred)
src/uenux2/mock/api/hwil/cpowermock.h                             api::teste::CPowerMock + SStatusEnergia (path inferred)
src/uenux2/src/api/audio/itexttospeech.h / .cpp                   api::ITextToSpeech (path inferred)
src/uenux2/src/api/audio/crhvoicetexttospeech.u33.cpp             api::Logger::log + index of the RHVoice helpers
src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.h / .cpp   mr.ver writer (path inferred)
src/uenux2/src/api/pattern/cpolysingletonlist.u33.cpp             1005, 1562, 2860, 3853
src/uenux2/src/api/pattern/iobservable.u33.cpp                    3940
src/uenux2/src/api/gui/cdatatext.u33.cpp                          1565, 1566
src/uenux2/src/api/gui/cformbuilder.u33.cpp                       2244 AddLine, 3058 AddDataTextFmt
src/uenux2/src/api/gui/cframedtext.u33.cpp                        2779, 3677
src/uenux2/src/api/gui/ctextfield.u33.cpp                         3889
src/uenux2/src/api/gui/iformfield.u33.cpp                         2904
src/uenux2/src/api/util/cstringutils.u33.cpp                      2199, 2677, 2763
src/uenux2/src/api/hash/chasharquivo.u33.cpp                      2723
src/uenux2/src/api/ipc/cmessagequeue.u33.h                        3903
src/uenux2/src/app/comum/dados/celeitores.u33.cpp                 1936
src/uenux2/src/app/comum/relatorios/crelutil.u33.cpp              2792
src/uenux2/src/app/comum/comparecimentomesario/estados/estadosregistromesarios.u33.cpp   2895
src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.u33.cpp                 1535, 2746, 3620
src/uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.u33.cpp        2743
src/uenux2/src/app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp     3883
src/uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.u33.cpp              3628
src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.u33.cpp  2288
```

A 3684 (`CApplicationContextStack::Push`) já é reconstruída pela unidade u15 e é apenas mapeada aqui.

---

## 1. Propósito e termos em português

* **urna / UE (urna eletrônica)**: a máquina de votação. Os modelos são nomeados pelo ano (UE2009 … UE2020, UE2022).
  **eleitor**: quem vota. **mesário**: membro da mesa receptora. **MT (microterminal)**: o teclado do mesário com um
  LCD de 4 × 40.
* **encerramento**: o fim da votação, quando a urna grava os seus **arquivos de resultado** (o
  BU, o RDV, `jufa.dat`, `hash.dat`, `log.jez`, **`mr.ver`** …) e os copia para a **MR (mídia de
  resultado)**, o pen drive levado ao TSE.
* **BU (boletim de urna)**: a apuração assinada de cada urna. **RDV (registro digital do voto)**: o registro
  embaralhado de todos os votos. **CEPESC**: a biblioteca de criptografia do TSE, usada para cifrar o BU e as imagens
  de digitais quando os parâmetros da eleição pedem.
* **código de carga**: o identificador de 24 dígitos da carga das mídias da urna. Seus seis últimos
  dígitos são impressos como **"RESUMO DA CORRESPONDÊNCIA"** no BU e na zerésima (o relatório impresso na
  abertura que prova que a urna não contém votos).
* **voto com áudio / áudio do eleitor**: votação acessível. A urna lê cada tela em voz alta por
  fones de ouvido para eleitores com deficiência visual (TTS = text-to-speech, RHVoice, ver `docs/libraries/rhvoice.md`).
* **liberar / habilitar o eleitor**: o mesário libera o eleitor identificado para votar.
* **inspeção**: uma verificação aleatória da cabine de votação que a thread do operador agenda.

Como a unidade se encaixa no processo de votação:

1. **Inicialização** (`votaInit`, `main`). O bootstrap do simulador (func 8302) registra `CUrnaMock` e
   `CPowerMock` como o hardware da urna. `main` registra o resto por meio do `push` de ponteiro bruto (1562), cujo
   construtor de elemento é a 1005. `votaInit` posta a mensagem 9 (áudio ligado) para a thread do eleitor por meio da 3903, mas só
   quando a opção da página `audioEleitorHabilitado` é verdadeira, e depois a mensagem 1 (iniciar o eleitor), sempre.
2. **Telas do eleitor.** Toda caixa de dígito é desenhada pela 2779; a caixa delimitadora de todo campo de texto vem da 3889; o
   relógio do cabeçalho de status é montado pela 3058. Com a acessibilidade ligada, o eleitor muda a velocidade da fala com
   as teclas 4/6 (`ITextToSpeech` slots 6/7).
3. **Lado do operador** (morto no build web, onde a thread do operador nunca executa). O mesário habilita o
   eleitor (3883), recebe o pedido de conectar os fones de ouvido (2743), ou volta para "digite o seu título" depois de um erro
   (2895).
4. **Encerramento.** `CGravaResultado` registra `CGeracaoVersoesContratos` (2860) e grava `mr.ver`
   (11582). O BU cifrado usa a tabela CEPESC do slot 4 de `IUrna`, que é **1024 × 0x01** neste build.

## 2. Classes e como se relacionam (RTTI)

```
api::IUrna (typeinfo @1530936, no vtable of its own)            src/api/hwil/iurna.h (inferred)
 └ api::teste::CUrnaMock      vtable @1530888, 28 B               mock/api/hwil/curnamock.h (inferred)
api::IPower (typeinfo @1531032)                                   src/api/hwil/ipower.h (u18)
 └ api::teste::CPowerMock     vtable @1530952, 48 B, 17 slots     mock/api/hwil/cpowermock.h (inferred)
api::ITextToSpeech (typeinfo @1526340, vtable @1526528, 12 slots) src/api/audio/itexttospeech.h (inferred)
 ├ api::CRHVoiceTextToSpeech  vtable @1585556, 112 B              (u32)
 └ simulador::CWasmNullTextToSpeech  vtable @1528704, 96 B        (u28/u29)
RHVoice::event_logger
 └ api::Logger                vtable @1586196 = {174, 144, 10815}, 4 B
comum::IResultado ─ comum::IGravador (u23)
 └ comum::CGravadorVersoesArquivos  vtable @1557248, 64 B          comum/gravadores/cgravadorversoesarquivos.* (inferred)
```

O namespace `api::teste` diz o que os dois mocks são: dublês de teste das interfaces de hardware,
reutilizados pelo simulador. Seus caminhos são inferidos do único caminho de mock que o binário conhece,
`uenux2/mock/app/comum/cappinfobuilder.cpp`, que espelha `src/app/comum`.

### 2.1 `api::teste::CUrnaMock` (IUrna)

| slot | func | método (todos os nomes inferidos) | comportamento do mock |
|---|---|---|---|
| 0 | 1661 (ICF `return this[+4]`) | `int GetModelo()` | **2020** |
| 1 | 8168 | `std::string GetModeloAbreviado()` | `std::format("{:02}", modelo - 2000)` = `"20"` |
| 2 | 2587 (ICF `return this[+8]`) | `GetNumeroInterno()` ? | **87654321** (o `numeroInternoUrna` do fixture `eg.bin` do simulador) |
| 3 | 3383 (ICF `return this[+12]`) | `GetRevisaoHardware()` ? | 255 |
| 4 | 8144 | `GetTabelaCepesc(array<uebyte,1024>&)` | a tabela configurada, completada com 0x01. Nada a configura, então a tabela é **1024 × 0x01** |
| 5 | 8134 | `GetDados32(array<uebyte,32>&)` ? | 32 × 0x02 |
| 6 | 8131 | `GetTabelaRdv(array<uebyte,128>&)` | 128 × 0x03 |
| 7 / 8 | 8130 / 8129 | destrutor / destrutor de exclusão | libera o vetor da tabela (+16) |

Layout: `+4 int modelo = 2020`, `+8 uint32 = 87654321`, `+12 int = 255`, `+16 std::vector<uebyte>` (vazio).
O construtor é inlinado na func 8302 como três stores `i64`.

Só os slots 0, 4 e 6 têm chamadores (14 pontos de chamada em 13 funções, todos por meio de `CPolySingletonList::instance<IUrna>`,
func 923).
O slot 0 decide o comportamento dependente do modelo: a instrução de áudio diz que o teclado fica "abaixo" (da
tela) para modelos > 2019, a captura de digital aceita o primeiro quadro em 2009/2010/2020/2022,
`CEscolheOpcao` e `CPedeIdentidade` mudam o modo de menu do microterminal (IScreenMT slot 19) a partir de 2020,
`CGravaResultado` registra uma sessão no módulo de segurança "MSE" em vez do "MSD", `CThreadMonitor` registra as
tensões de alimentação só a partir de 2020, e `IInterfaceInit` escolhe a porta da MR. Portanto, o build web é sempre uma
**UE2020**.
O slot 4 alimenta a cifragem CEPESC do BU (`CGravadorBU::GravaResultado`, srcloc `cgravadorbu.cpp:484`)
e das imagens de digitais (`CControlaArmazenamentoDeImagens`). O slot 6 é a tabela da qual a chave do RDV
é derivada (u01/u02: 32 bytes escolhidos pelo SHA-512 da lista de cargos, depois HKDF). **As três tabelas "secretas"
são constantes públicas neste build.** Isso é esperado em um simulador. Para o RDV a consequência é direta:
a sua chave AES é função de dados públicos, então qualquer pessoa com o binário pode decifrar um `rdv.dat` do simulador (u01/u02).
Para o CEPESC a tabela é só uma das entradas, ao lado de uma semente aleatória de 32 bytes (`IRng`) e da chave pública do TSE, então o quanto
uma tabela pública enfraquece não pode ser avaliado a partir desta unidade.

### 2.2 `api::teste::CPowerMock` (IPower)

Apesar do nome da interface, a palavra de status de 16 bytes de `IPower` é o status geral do hardware da urna. Os
significados abaixo vêm do código que testa os bits (CPowerInformation, CFormBuilder, e as mensagens de log de
`vota::CThreadMonitor`):

```
+0  u32 flags  & 0x06 fonte (0 rede elétrica, 2 bateria interna, 4 bateria externa)
               & 0x18 bateria interna (0 cheia, 0x08 parcial, 0x10 crítica, 0x18 ausente)
               & 0x60 bateria externa (same codes)      0x0200 mídia externa ausente (error 9390)
               0x4000 chave "URNA DESLIGADA"            0x2000 slot 0      0x0800 ? (set by the mock)
+4  u16  bit 1 = fone de ouvido desconectado            +6  u16 tensão bateria interna (V x 100)
+8  u16  tensão bateria externa (V x 100)               +10 u16 tensão rede CA (V x 100)
+12 u16  corrente (A x 100), read as "percentual" by the mock's slot 7
+14 u8   ? (slot 6)                                     +15 u8 bit 0 = teclado do eleitor desconectado (9391)
```

| slot | func | método (`?` = inferido) | corpo |
|---|---|---|---|
| 0 | 8128 | `EhAlimentacaoExterna()` ? | `!(flags & 0x2000)` |
| 1 | 8120 | `GetTensaoBateriaInterna()` (u18: `vf1`) | +6 |
| 2 | 8116 | `GetCorrenteBateria()` | `±`corrente, negativa quando a tensão CA é 0; 0 se a bateria interna estiver ausente |
| 3 | 8113 | `GetTensaoBateriaExterna()` (u18: `GetTensaoBateria`) | +8 |
| 4 | 8111 | `GetTensaoRedeCA()` (u18: `GetTensaoCarregador`) | +10 |
| 5 | 8109 | `GetCargaEstimada()` ? | `round(fator(tensão) × corrente)`, uma tabela de tensões de 20 degraus (ver §9, um bug) |
| 6 | 8108 | `GetTemperatura()` ? | +14 (byte) |
| 7 | 8107 | `GetPercentualBateria()` | o próprio +32 do mock (sua cópia de +12), limitado a 100 **no lugar** |
| 8-12 | 8105 … 8074 | padrões de `IPower` (`"Not supported"`, `ipower.h:354…424`) | |
| 13/14 | 174/144 | destrutores triviais | |
| 15 | 8071 | `AtualizaStatus(SStatusEnergia&)` | copia o status simulado de 16 bytes (+20..+35) |
| 16 | 434 (ICF `return 1`) | ? | |

Chamadores: slot 15 mais testes de bits inline (cabeçalho de status, ícone de bateria, `CMonitoraAlimentacao`, os estados de
desligamento automático, `CRelatorioTesteImpressora`, `CThreadMonitor`); slot 7 (o texto do cabeçalho `"{: >3}%"`); slots
1-4 (`CThreadMonitor::LogaStatusRedeAcBateria`, somente modelos ≥ 2020: "Urna ligada conectada na rede CA em
[{:3.2f}V] e na Bateria Interna com [{:2.2f}V/{}A]"). Os slots 0, 5, 6 e 16 não têm chamador. A thread de monitoramento
nunca executa no build web, então nas gravações só os slots 15 e 7 são alcançáveis.

Os slots 0-6 atualizam `IPower::m_status` (+4) por meio do slot 15 e depois leem só esse membro da base. Eles podem ser
corpos padrão em `ipower.h` em vez de overrides do mock; a vtable não permite distinguir. **Divergência entre unidades:** o
`ipower.h` da u18 não declara nenhum membro de dados (o status de 16 bytes em +4 precisa ser acrescentado lá como `m_status`) e nomeia
os slots 1/3/4 `vf1` / `GetTensaoBateria` / `GetTensaoCarregador`, então os `override`s de `cpowermock.h` ainda não
batem com ele. Os nomes usados aqui vêm de `CThreadMonitor::LogaStatusRedeAcBateria`, que imprime o slot 4 como "rede CA"
e usa o slot 3 ("Bateria Externa") quando ele é > 0, senão o slot 1 ("Bateria Interna"). O status constante do mock
`{0x840, 2, 0, 0, 0, 100}` significa: alimentação pela rede, bateria interna cheia, bateria externa "crítica" (0x40),
fones de ouvido reportados como desconectados, todas as tensões 0, 100 %. O ícone de bateria é portanto `img-ac-bateria-full`
e o cabeçalho mostra "100%". `+40` guarda `steady_clock::now()` da construção e nunca é lido.

### 2.3 `api::ITextToSpeech` e `api::Logger`

Layout (96 bytes; construtor inlinado nas duas implementações, funcs 13877 e 10842):

```
+4   CLruCache<string, SharedWav>  m_cacheRecente   capacity 256, list + unordered_map (max_load 1.0)
+40  unordered_map<string, SharedWav> m_cachePermanente
+60  int m_nivelVelocidade = 2      +64 std::string m_perfilVoz = ""
+76  int m_p76 = 50  (?)            +80 int m_p80 = 100 (tom/pitch %)
+84  int m_taxa = 100 (rate %)      +88 int64 m_p88 = 0 (?)
```

| slot | func | método | corpo |
|---|---|---|---|
| 1 | 11518 | `SetPerfilVoz(const string&)` | `m_perfilVoz = s` |
| 2 | 11511 | `SetTom(int)` | `m_p80 = v` |
| 3 | 11507 | `SetP76(int)` ? | `m_p76 = v` |
| 4 | 11505 | `SetVelocidade(int nivel)` | `nivel`, `taxa = nivel*20 + 60` (sem verificação de intervalo) |
| 5 | 11504 | `GetVelocidade()` | `nivel` |
| 6 | 11500 | `AumentaVelocidade()` | `nivel = min(nivel+1, 4)` |
| 7 | 11494 | `DiminuiVelocidade()` | `nivel = max(nivel-1, 0)` |
| 8 | 11491 | `SetP88(int64)` ? | `m_p88 = v` |
| 9 | 11489 | `~ITextToSpeech()` | string, cache permanente (libc++ 3746), cache LRU (3764); também o slot 9 do TTS nulo |
| 10 | 325 | destrutor de exclusão da base abstrata | um `unreachable` puro (o clang emite um trap para o D0 de uma classe abstrata) |

Taxas: nível 0..4 = **60, 80, 100, 120, 140 %**. **Verificação em tempo de execução** (`headless.mjs --audio --save-audio`,
teclas `6 6 6 4 4 4 4 4` na tela de instrução de áudio): as respostas "Fala mais rápida" (120 %, 37,832 B → 140 %,
32,192 B), "Fala em velocidade máxima" (em 4, 140 %), "Fala mais lenta" em 120/100/80/60 % (33,544 / 40,044 /
50,044 / 66,544 B, razões 1.19, 1.25, 1.33 = 120/100, 100/80, 80/60), depois "Fala em velocidade mínima".
Isso confirma a tabela e a saturação dos slots 6 e 7.

Os slots 1, 2, 3 e 8 nunca são chamados, então toda requisição usa perfil `""`, tom 100 % e os campos constantes.
A chave do cache (func 5758, u02) formata `(texto, taxa, perfil, tom, p88)`, nessa ordem. `p76` não está nem na
chave nem na requisição ao RHVoice (a func 10841 lê só `taxa` e `tom`). É um dado morto neste build.

`api::Logger::log` (10815) sobrescreve `RHVoice::event_logger::log(tag, level, message)`. Ela imprime só
`RHVoice_log_level_error` (4), como `RHVoice [<tag>]: <message>` + `std::endl`, em **`std::clog`**
(@1923832). `docs/libraries/rhvoice.md` diz `std::cerr`. O código de inicialização em `__wasm_call_ctors` mostra 1923688 = cerr
(unitbuf, vinculado a cout) e 1923832 = clog, ambos em stderr. Na página, o stderr vai para `console.error`.

## 3. O arquivo `mr.ver` (`CGravadorVersoesArquivos::GravaResultado`, func 11582)

`mr.ver` é um dos 11 arquivos de resultado do encerramento. Ele registra a **versão de cada contrato ASN.1**
(módulo) usado pelos outros arquivos de resultado, para que o software de leitura do TSE saiba como decodificá-los. Esta
seção é a parte do fluxo do BU/encerramento que pertence a esta unidade. O BU propriamente dito está na u23 (`CGravadorBU`) e em
docs/10-boletim-de-urna.md.

Passo a passo (gravador nº 10 de `vota::CGravaResultado::StartState`, func 12098, unidades u07/u09):

1. `CGravaResultado` reúne os módulos ASN.1 dos gravadores (`ModuloBoletimUrna`, `ModuloEnvelopeGenerico`,
   `ModuloResultadoUrnaCadastro`, `ModuloHashes`, `ModuloAssinaturaEcourna`, `ModuloVersaoArquivos`).
2. `CPolySingletonList::exists<comum::CGeracaoVersoesContratos>()` (**func 2860**, esta unidade). Se o objeto
   ainda não estiver registrado, ele é construído a partir de `<root>/etc/dependencias.properties` e `<root>/etc/versoes.properties`.
   Os dois devem trazer `tag=20260601173148`. No simulador os dois são stubs de 0 bytes, então isso lança 8662 (u09). O
   mesmo `exists` também é avaliado uma vez por `__wasm_call_ctors`.
3. A lista de módulos é fechada sobre as suas dependências transitivas, e cada módulo recebe a sua versão →
   `md::CVersoesArquivos(TAG_CONTRATOS, map<módulo, versão>)` (func 5867: 8694 "Versão da TAG inválida" se a
   tag estiver vazia, 8695 se o map estiver vazio).
4. `new CGravadorVersoesArquivos(município, zona, local, seção, fase, versoes)`: `IGravador` com
   `EExtensaoArquivoResultado 19` = `mr.ver` e id de arquivo SAVD 70. O objeto tem 64 bytes, com a tag em +40 e o map em +52.
5. `IGravador::Grava()` / `GravaMV()` (slots 4/5, u23) abrem o arquivo nomeado por `CGravadorUtil::DeterminaNomeArquivo`
   (u23: `<fase><pleito:05><uf><mun:05><zona:04><seção:04>-` + o sufixo `mr.ver`) no diretório de trabalho, com
   `"wb"`, na MI (flash interna) e na MV (mídia de votação), e chamam o **slot 7 = func 11582**:
   * `CConversorVersoesArquivos::DoConverte` (10264, u21) monta
     `EntidadeVersaoArquivos ::= SEQUENCE { versaoTag, arquivos SEQUENCE OF ArquivoAssinatura { nome, versão } }`
     na ordem do `std::map` (ordenado pelo nome do módulo);
   * `isValid() && isStrictlyValid()`, ou `CBaseError<EUeComumAsnError>` **7653** "Entidade deixada em estado
     inválido: {}" + `ASN1::trace_invalid` de `N20ModuloVersaoArquivos22EntidadeVersaoArquivosE` (iconversorasn.h:56);
   * contexto `"WriteToFile de " + arquivo.GetName()`; `isValid()` de novo ou `CBaseError<EUeIoError>` **5955**
     "Objeto com conteúdo inválido para {}: {}" (cfileasn.h:161);
   * codificação BER (`ASN1::CoderEnv::encodeBER`), ou **5956** "Arquivo não foi codificado para " + contexto
     (cfileasn.h:171);
   * `CFile::RawWrite(buffer)` (func 1886).
6. O arquivo é copiado para os diretórios de resultado (IGravador slots 2/3/6), assinado pelo SAVD junto com os outros
   arquivos de resultado em `…-vota.vsc` (`CAssinador::AssinaArquivosResultado`, u09), e copiado para a MR. Neste build
   as assinaturas são o literal `assinatura simulada para vota_web_wasm`.

A página web nunca chega a esta etapa: ela permanece no modo de treinamento de eleitores e nunca executa o encerramento
(docs/bu/codepath.md). Outras peças relacionadas ao BU nesta unidade:

* **2792** `CRelUtil::FormataCorrespondencia`: o "RESUMO DA CORRESPONDÊNCIA" do BU impresso, da zerésima
  e das telas do horário da zerésima = `codigoCarga.substr(18,3) + "." + codigoCarga.substr(21)` (últimos 6 dos 24
  dígitos: `537864991559016480376254` → `376.254`, fixture → `901.234`).
* **IUrna slot 4** (`CUrnaMock` 8144): a tabela CEPESC de 1024 bytes do BU cifrado (`CGravadorBU`,
  `cgravadorbu.cpp:484`). No simulador ela é 1024 × 0x01.
* **2244** `CFormBuilder::AddLine`: as linhas separadoras de `CMostraQRCodeBU` (a tela do QR code do "BU digital")
  e de `CImprimirBUOutrasObrigatorias`.
* **2763** `CStringUtils::NomeArquivo`: a parte do nome de arquivo do caminho do pacote de assinatura (`…-vota.vsc`) em
  `CGravaResultado`, e os nomes gravados em `hash.dat` por `CMontadorHash`.

## 4. Os outros helpers do TSE

### 4.1 GUI (`api/gui`)

* **2244** `CFormBuilder::AddLine(inicio, fim, cor)`: `make_shared<CLineField>` (36 B, vtable @1579364) + `Add` (426).
* **3058** `CFormBuilder::AddDataTextFmt(fonte, pos, período, fonte, formato, alinhamento)`:
  `CDataTextFmt<std::string (*)(const std::string&)>` dentro de um `CTextFieldUpdate`. O cabeçalho de status o usa
  para o relógio `"A DD/MM/YYYY hh:mm:ss"`, atualizado a cada 500 ms. Observada.
* **2779** `CFramedText::DesenhaMoldura(i, cor, tela)` → `IScreen::DrawRect(caixa i, cor, 1)`, e
  **3677** `CFramedText::Rect()`: as caixas de dígitos. Largura da caixa = o glifo mais largo + margens, espaçamento =
  `size <= 7 ? 1 : size / 8`.
* **3889**: o corpo compartilhado de `CTextField::Rect`, `CTextFieldBlinking::Rect` e `CTextFieldUpdate::Rect`
  (thunks 10951, 10943, 10913). Ela mede o texto com `IScreen::GetFontMetrics` e o desloca conforme o
  alinhamento (direita: x−w, centro: x−w/2).
* **1566 / 1565**: corpos de destrutor completo / de exclusão compartilhados por `IFormFieldBase<MEDIA>` e
  `CDataTextFmt<SRC>` (um `IText`). Ambos são `{vptr, 2 words, std::string @+12}`. A 1565 também é o D0 de nove
  classes folha de `IFormField` sem membro a destruir, por meio de três thunks ICF que passam a vtable da *base*
  (o store do próprio vptr delas é morto): 2777 `IFormFieldBase<IScreenMT>` (CBeepFieldMT, CLedFieldMT, CBuzzFieldMT,
  CClockFieldMT), 3681 `IFormFieldBase<IScreen>` (CFillField, CLineField, CRectField) e 5516
  `IFormFieldBase<IPaper>` (CCutFieldPaper, CNewLineFieldPaper).
* **2904**: `std::string` a partir de um literal de 11 caracteres. É compartilhada por `CMovieField/CImageField/CInputField::GetClassName`
  e pelo `userdict::empty_string::describe` do RHVoice ("EmptyString").
* **3940**: `IObservable<std::vector<FormHandle<MEDIA>>>::~IObservable` (MEDIA = IScreen/IScreenMT/IPaper).
* **3684**: `CApplicationContextStack::Push` (u15). Um contexto genérico é recusado sobre um específico.

### 4.2 Registro (`api/pattern`)

* **1005**: o construtor de elemento `SPolySingleton{nome, tipo, instancia}` que toda instanciação de `emplace_back`
  passa por slot de tabela (12 slots, um corpo dobrado). Observada.
* **1562**: `push<INTERFACE>(INTERFACE* p, info)` = `push(std::unique_ptr<INTERFACE>(p), info)`. O `push` a
  chamar é um parâmetro. É usada pelos 8 wrappers que `main` invoca (IExecucaoVota, as 3 fábricas de IPC,
  fábrica de IThreadImpl, IFingerPrepare, ISymmetricCipherFactory, IRng). Observada.
* **2860 / 3853**: `exists<comum::CGeracaoVersoesContratos>` e `exists<vota::testeteclado::impl::IGeradorTeclas>`
  (esta última a partir do teste de teclado `CTesteTeclado::StartState`). Não foram dobradas com os outros corpos de `exists`
  porque o nome com mangling é montado inline e o seu comprimento (34, 42) não tem par.

### 4.3 Strings, erros, mensagens

* **2199** `CStringUtils::ProcuraRegex(expressão, texto)` → `{início, fim}` do primeiro casamento do boost::regex,
  ou `{-1,-1}`. O segundo campo é o **fim**, não o comprimento (a u20 declarou `{posicao, tamanho}`). Toda
  exceção é engolida. Usada por `GetVersionNumber` ("10.23.0.1 - DESENVOLVIMENTO" → "10.23.0.1") e por
  `CTradutorFrase::TraduzLabel` (templates de rótulos de relatórios). Observada apenas por meio de `TraduzLabel` (654, chamada por
  4160/4161; `GetVersionNumber` nunca aparece nos perfis). TraduzLabel a chama três vezes por iteração e toda
  chamada compila o seu padrão de novo (`basic_regex::do_assign` 9401 responde por 139 das 182 amostras do profiler da 2199 na
  sessão municipal: cerca de 2.8 ms de 3.6 ms a 20 µs por amostra).
* **2677** `CStringUtils::ReplaceAll` (cópia + laço no lugar 9407): `CProgressBar` "%p", `CIniStrings`,
  `CSubstituidorTitulo`.
* **2763** `CStringUtils::NomeArquivo`: o texto depois da última `/`.
* **2723**: o thunk do construtor de `CBaseError<api::EUeHashError>` (5100-5103 de `api/hash`).
* **1936** `MontaCaminhos(dir, nomes)` → `{dir + nome}` (arquivos do cadastro de eleitores de `CEleitores`).
* **3903** `CPriorityMessageQueue<SMessage>::Envia(id)` = `Add({id, this}, 1)`. As três lambdas do ponto de entrada
  web postam MSG_AUDIO_HABILITADO (9, só se `audioEleitorHabilitado`) e depois MSG_INICIA_ELEITOR (1) em
  `votaInit`, e MSG_SINCRONIZA_VOTO (5) a partir de `votaTick` depois dos quatro passos da barra de progresso. Observada (só o
  caminho `votaInit` → 11026 → 3903 tem amostras). O `Add(msg, 1)` que ela chama é uma função separada de uma linha (7708),
  isto é, uma sobrecarga `Envia(const SMessage&)` que o `cmessagequeue.h` da u18 não declara.

### 4.4 Estados do operador (mesário): mortos no build web

```
CNomeEleitor / CDigitalReconhecida / CControlaReconhecimento / CInformaEleitorPodeVotar
CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo
        │ CONFIRMA (3883 = merged ProcessInput; the same sequence is inlined in the first three)
        ├─ IInformacaoThreadOperador::DeveHabilitarAudio() (2746)? ── yes ─▶ CHabilitaAudioEleitor (2743)
        │                                                                 CONFIRMA: msgs 8|9 then 1 (u19 10523)
        └─ no: CThreadEleitor fila.Add({1 MSG_INICIA_ELEITOR}, 1) ─────▶ CMostraEleitorVotando (1150)
```

* **2743** `CHabilitaAudioEleitor::GetInst()`, com o construtor inlinado: formulário do MT com buzzer (51, 10), relógio
  {33,1}, "Este eleitor necessita de áudio" {1,2}, "Coloque o fone de ouvido na urna" {1,3},
  "CONFIRMA: continuar" {1,4} com alinhamento 1, e um controle de entrada. (Para alinhamento à direita o MT ignora x:
  `CWasmScreenMT::Write`, func 8799, escreve na coluna `40 - len`, então a linha termina na coluna 40.)
* **2895**: o `ProcessInput` compartilhado das quatro telas de erro do registro de mesários (`CTituloMesarioVazio`,
  `…Invalido`, `…JaRegistrado` com CORRIGE, `CDigitalMesarioNaoReconhecida` com CONFIRMA) → volta para
  `CPedeTituloMesario`.
* **1535 / 2746 / 3620**: `IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor()` fora de linha (por valor;
  o ponteiro sret vem antes de `this` na ABI do wasm), `.DeveHabilitarAudio()` e `.SorteiaProximaInspecao()`.
* **3628** `TextoVazio()` → `" "` (fonte de dados de linhas em branco do MT). **2288** `CMenuVisualizarCandidatos::GetInst()`.

## 5. Funções de biblioteca (não reconstruídas)

10 funções do RHVoice 1.14.0 e 5 templates da libc++ instanciados para tipos do RHVoice (conferidos com a tag
upstream), chamados a partir do pipeline de TTS inlinado (func 10841) ou de outro código do RHVoice:
as consultas a tabelas Unicode `unicode::properties` (590), `tolower` (854) e `category` (1144) sobre
`records[23697]` @626784 (`{code, category, upper, lower, properties}`); `emoji_scanner::process` (1896);
`userdict::position::set_token` (2703) / `forward_token` (2210) e `dict::should_ignore_token` (3547);
`item::append_child(item&)` (2214); `voice_search_criteria::operator()` (3529); `fst::translate<text_iterator,
item::back_insert_iterator>` (3574); as duas instâncias de `std::find_if` de `str::tokenizer<is_space>` (2756, 3571);
a cópia de text-iterator (2754); e o destruidor de nós de `std::map<string, shared_ptr<T>>` (2715) e a
inserção com dica em `set<string, str::less>` (2716). Mais da libc++: `filesystem::path::__parent_path()` (1840) e a
inserção com dica em `map<uebyte, string>` (3266). Ver o índice em `crhvoicetexttospeech.u33.cpp`.

## 6. Particularidades do build web

* Os dois mocks são todo o "hardware" da urna para identificação, segredos e alimentação: modelo fixo 2020,
  identificação fixa 87654321, tabelas públicas (0x01 / 0x02 / 0x03), alimentação permanente pela rede a 100 %.
* A classe base de TTS é compartilhada pelo motor real (RHVoice, só quando a página ativa a acessibilidade) e pelo
  `CWasmNullTextToSpeech` silencioso. Os votos gravados executaram com o silencioso, então nenhum slot de `ITextToSpeech` aparece
  nos perfis. A execução com áudio acima exercitou os slots 4-7.
* `mr.ver` (e todo o encerramento) nunca é gravado pela página.
* Os estados do operador (2743, 2895, 3883, 1535, 2746, 3620, 3628) estão mortos: `CThreadOperador::Run` nunca começa.
  O próprio `votaInit` posta a mensagem de "eleitor habilitado" (3903).

## 7. Observações sobre wasm / Emscripten

* **merge-similar-functions em toda parte.** Um terço desta unidade são corpos que o Binaryen compartilhou entre funções
  não relacionadas, transformando a constante que difere em um parâmetro:
  * um **`std::source_location`** como parâmetro: 3889 (três `Rect()`), 3883 (três `ProcessInput`), 2895 (quatro
    `ProcessInput`, mais a tecla esperada);
  * uma **vtable** como parâmetro: 1565/1566 (destrutores), 3940 (`~IObservable`), 2723 (construtor de `CBaseError`);
  * uma **função (slot de tabela)** como parâmetro: 1562 (qual `push` chamar);
  * **endereços de literais de string** como parâmetros: 2904 (`s` e `s+7` de um literal de 11 caracteres).
  As ferramentas arquivam um corpo assim sob a classe do primeiro chamador que encontram, e é por isso que estas acabaram
  "sem arquivo conhecido".
* **ICF entre classes não relacionadas.** Os slots 0/2/3 de `CUrnaMock` são os corpos genéricos `return this->[+4/+8/+12]`
  (1661, 2587, 3383). A 1005 é o único construtor de elemento por trás de 12 slots de tabela.
* **sret antes de `this`.** A 1535 é um thunk para o slot 25, que devolve uma classe por valor. A chamada aparece como
  `vf25(result, instance)` porque a ABI do wasm (Itanium) passa o slot de retorno primeiro.
* **Destrutores de exclusão abstratos são traps.** `ITextToSpeech` slot 10 = func 325 = `unreachable`.
* **Strings curtas são imediatos.** A 3628 devolve `" "` gravando `0x0020` e o byte de tamanho. 2743, 2860 e 3853
  montam os seus literais com leituras de 8 bytes (ver o capítulo da libcxx).
* **Tabelas de ponto flutuante ficam no código.** `CPowerMock::vf5` é uma cadeia de `f64.const` / `br_if` (uma cadeia de `select`
  no descompilador), e é assim que o erro de digitação de 10× em duas constantes aparece em §9.

## 8. Tabela de mapeamento (todas as 72 funções)

`run` = vista em `analysis/runtime/*.functions.tsv`. Os caminhos estão sob `src/`; `(u15)` etc. = reconstruído por aquela unidade.

| func | tamanho | run | símbolo reconstruído | componente | reconstrução |
|---:|---:|:-:|---|---|---|
| 590 | 139 | | `RHVoice::unicode::properties(utf8::uint32_t)` | lib:rhvoice | biblioteca (RHVoice `src/core/unicode.cpp`), índice em `api/audio/crhvoicetexttospeech.u33.cpp` |
| 854 | 135 | | `RHVoice::unicode::tolower(utf8::uint32_t)` | lib:rhvoice | biblioteca (unicode.cpp) |
| 1005 | 61 | * | `api::SPolySingleton::SPolySingleton(nome, tipo, instancia)` (ctor de elemento do emplace_back, 12 slots) | app:api | `api/pattern/cpolysingletonlist.u33.cpp` (orig. cpolysingletonlist.h) |
| 1144 | 153 | | `RHVoice::unicode::category(utf8::uint32_t)` | lib:rhvoice | biblioteca (unicode.cpp) |
| 1535 | 20 | | `vota::IdentidadeEleitor()` = `IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 1562 | 142 | * | `api::CPolySingletonList::push<I>(I*, TPolySingletonsInfo&)` (corpo mesclado) | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 1565 | 39 | | corpo de destrutor de exclusão compartilhado por `CDataTextFmt<SRC>` e (via ICF 2777/3681/5516) nove classes folha de `IFormField`; recebe a vtable | app:api | `api/gui/cdatatext.u33.cpp` |
| 1566 | 36 | | `IFormFieldBase<MEDIA>::~IFormFieldBase` / `CDataTextFmt<SRC>::~CDataTextFmt` (corpo mesclado) | app:api | `api/gui/cdatatext.u33.cpp` |
| 1840 | 316 | * | `std::filesystem::path::__parent_path() const` | rt:libcxx | biblioteca/helper inlinado (libc++ `filesystem/path.cpp`) |
| 1896 | 294 | | `RHVoice::emoji_scanner::process(utf8::uint32_t)` | lib:rhvoice | biblioteca (emoji.cpp) |
| 1936 | 370 | | `comum::(anon)::MontaCaminhos(dir, nomes)` | app:comum | `app/comum/dados/celeitores.u33.cpp` |
| 2199 | 723 | * | `api::CStringUtils::ProcuraRegex(expressao, texto)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2210 | 119 | | `RHVoice::userdict::position::forward_token()` | lib:rhvoice | biblioteca (userdict.cpp) |
| 2214 | 189 | | `RHVoice::item::append_child(item&)` | lib:rhvoice | biblioteca (item.cpp) |
| 2244 | 167 | | `api::CFormBuilder::AddLine(inicio, fim, cor)` | app:api | `api/gui/cformbuilder.u33.cpp` |
| 2288 | 22 | | `vota::CMenuVisualizarCandidatos::GetInst()` | app:vota | `app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.u33.cpp` |
| 2677 | 128 | | `api::CStringUtils::ReplaceAll(texto, de, para)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2703 | 224 | | `RHVoice::userdict::position::set_token(item&)` | lib:rhvoice | biblioteca (userdict.hpp) |
| 2715 | 109 | | `std::__tree<string, shared_ptr<T>>::destroy(node)` (ICF, maps do RHVoice) | lib:rhvoice | biblioteca/helper inlinado (template da libc++) |
| 2716 | 704 | | `std::__tree<string, str::less>::__emplace_hint_unique_key_args` | lib:rhvoice | biblioteca/helper inlinado (template da libc++) |
| 2723 | 18 | | `CBaseError<api::EUeHashError>::CBaseError(code, msg, loc)` (thunk para 710) | app:api | `api/hash/chasharquivo.u33.cpp` |
| 2743 | 882 | | `vota::CHabilitaAudioEleitor::GetInst()` + construtor inlinado | app:vota | `app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.u33.cpp` |
| 2746 | 20 | | `vota::DeveHabilitarAudio()` = `IInformacaoThreadOperador::GetInst().DeveHabilitarAudio()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 2754 | 1398 | | cópia de um intervalo de `utf::text_iterator` para `utf8::uint32_t*` (`std::copy` / `__uninitialized_allocator_copy`) | lib:rhvoice | biblioteca/helper inlinado (`next` do utfcpp inlinado) |
| 2756 | 484 | | `std::find_if(first, last, std::not1(str::is_space))` | lib:rhvoice | biblioteca (tokenizer de str.hpp) |
| 2763 | 281 | | `api::CStringUtils::NomeArquivo(caminho)` | app:api | `api/util/cstringutils.u33.cpp` |
| 2779 | 166 | * | `api::CFramedText::DesenhaMoldura(i, cor, tela)` | app:api | `api/gui/cframedtext.u33.cpp` |
| 2792 | 499 | * | `comum::CRelUtil::FormataCorrespondencia(correspondencia)` | app:comum | `app/comum/relatorios/crelutil.u33.cpp` |
| 2860 | 381 | | `api::CPolySingletonList::exists<comum::CGeracaoVersoesContratos>` | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 2895 | 127 | | `ProcessInput` mesclado de CTituloMesarioVazio/Invalido/JaRegistrado, CDigitalMesarioNaoReconhecida | app:comum | `app/comum/comparecimentomesario/estados/estadosregistromesarios.u33.cpp` |
| 2904 | 57 | | corpo mesclado de `GetClassName()` para literais de 11 caracteres (CMovieField, CImageField, CInputField, EmptyString) | app:api | `api/gui/iformfield.u33.cpp` |
| 3058 | 352 | * | `api::CFormBuilder::AddDataTextFmt(fonte, pos, periodo, fonte, formato, alinhamento)` | app:api | `api/gui/cformbuilder.u33.cpp` |
| 3266 | 217 | * | inserção com dica em `std::map<uebyte, std::string>` (`__emplace_hint_unique_key_args`) | rt:libcxx | biblioteca/helper inlinado (construção por initializer-list) |
| 3529 | 191 | | `RHVoice::voice_search_criteria::operator()(const voice_info&)` | lib:rhvoice | biblioteca (voice.cpp) |
| 3547 | 454 | | `RHVoice::userdict::dict::should_ignore_token(const position&)` | lib:rhvoice | biblioteca (userdict.cpp) |
| 3571 | 486 | | `std::find_if(first, last, str::is_space)` | lib:rhvoice | biblioteca (tokenizer de str.hpp) |
| 3574 | 361 | | `RHVoice::fst::translate<utf8_string_iterator, item::back_insert_iterator>` | lib:rhvoice | biblioteca (fst.hpp) |
| 3620 | 20 | | `vota::SorteiaProximaInspecao()` = `IInformacaoThreadOperador::GetInst().SorteiaProximaInspecao()` | app:vota | `vota/operador/comum/cinformacaothreadoperador.u33.cpp` |
| 3628 | 16 | | `vota::(anon)::TextoVazio()` → `" "` | app:vota | `app/vota/operador/leidentidade/ieleitorimpedidovotar.u33.cpp` |
| 3677 | 133 | | `api::CFramedText::Rect()` | app:api | `api/gui/cframedtext.u33.cpp` |
| 3684 | 628 | * | `api::CApplicationContextStack::Push(CApplicationContext)` | app:api | `api/gui/capplicationcontextstack.u15.cpp` (u15) |
| 3853 | 393 | | `api::CPolySingletonList::exists<vota::testeteclado::impl::IGeradorTeclas>` | app:api | `api/pattern/cpolysingletonlist.u33.cpp` |
| 3883 | 173 | | `ProcessInput` mesclado de CInformaEleitorPodeVotar / CInformaAnoDesabilitadoDemo / CInformaBioDesabilitadaDemo | app:vota | `app/vota/operador/confirmaidentidade/cinformaeleitorpodevotar.u33.cpp` |
| 3889 | 285 | * | `Rect()` mesclado de CTextField / CTextFieldBlinking / CTextFieldUpdate | app:api | `api/gui/ctextfield.u33.cpp` |
| 3903 | 44 | * | `api::CPriorityMessageQueue<SMessage>::Envia(int16 id)` | app:api | `api/ipc/cmessagequeue.u33.h` |
| 3940 | 167 | | `IObservable<vector<FormHandle<MEDIA>>>::~IObservable` (corpo mesclado) | app:api | `api/pattern/iobservable.u33.cpp` |
| 8071 | 22 | | `api::teste::CPowerMock::AtualizaStatus(SStatusEnergia&)` (slot 15) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8107 | 47 | | `CPowerMock::GetPercentualBateria()` (slot 7) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8108 | 25 | | `CPowerMock::GetTemperatura()` ? (slot 6) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8109 | 401 | | `CPowerMock::GetCargaEstimada()` ? (slot 5) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8111 | 25 | | `CPowerMock::GetTensaoRedeCA()` (slot 4) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8113 | 25 | | `CPowerMock::GetTensaoBateriaExterna()` (slot 3) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8116 | 62 | | `CPowerMock::GetCorrenteBateria()` (slot 2) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8120 | 25 | | `CPowerMock::GetTensaoBateriaInterna()` (slot 1) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8128 | 29 | | `CPowerMock::EhAlimentacaoExterna()` ? (slot 0) | app:mock | `uenux2/mock/api/hwil/cpowermock.h` |
| 8129 | 47 | | `api::teste::CUrnaMock::~CUrnaMock()` de exclusão (slot 8) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8130 | 44 | | `api::teste::CUrnaMock::~CUrnaMock()` (slot 7) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8131 | 12 | | `CUrnaMock::GetTabelaRdv(array<uebyte,128>&)` → 128 × 0x03 (slot 6) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8134 | 62 | | `CUrnaMock::GetDados32(array<uebyte,32>&)` ? → 32 × 0x02 (slot 5) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8144 | 87 | | `CUrnaMock::GetTabelaCepesc(array<uebyte,1024>&)` → 1024 × 0x01 (slot 4) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 8168 | 366 | | `CUrnaMock::GetModeloAbreviado()` → `"20"` (slot 1) | app:mock | `uenux2/mock/api/hwil/curnamock.h` |
| 10815 | 187 | | `api::Logger::log(tag, nivel, mensagem)` | app:api | `api/audio/crhvoicetexttospeech.u33.cpp` |
| 11489 | 39 | | `api::ITextToSpeech::~ITextToSpeech()` (slot 9) | app:api | `api/audio/itexttospeech.cpp` |
| 11491 | 9 | | `ITextToSpeech::SetP88(int64)` ? (slot 8) | app:api | `api/audio/itexttospeech.cpp` |
| 11494 | 37 | | `ITextToSpeech::DiminuiVelocidade()` (slot 7) | app:api | `api/audio/itexttospeech.cpp` |
| 11500 | 42 | | `ITextToSpeech::AumentaVelocidade()` (slot 6) | app:api | `api/audio/itexttospeech.cpp` |
| 11504 | 7 | | `ITextToSpeech::GetVelocidade()` (slot 5) | app:api | `api/audio/itexttospeech.cpp` |
| 11505 | 22 | | `ITextToSpeech::SetVelocidade(int)` (slot 4) | app:api | `api/audio/itexttospeech.cpp` |
| 11507 | 9 | | `ITextToSpeech::SetP76(int)` ? (slot 3) | app:api | `api/audio/itexttospeech.cpp` |
| 11511 | 9 | | `ITextToSpeech::SetTom(int)` (slot 2) | app:api | `api/audio/itexttospeech.cpp` |
| 11518 | 13 | | `ITextToSpeech::SetPerfilVoz(const std::string&)` (slot 1) | app:api | `api/audio/itexttospeech.cpp` |
| 11582 | 1668 | | `comum::CGravadorVersoesArquivos::GravaResultado(api::CFile&)` (slot 7) | app:comum | `app/comum/gravadores/cgravadorversoesarquivos.cpp` |

## 9. Código suspeito ou digno de nota

1. **`CPowerMock::GetCargaEstimada` (8109): duas constantes dez vezes pequenas demais.** Os fatores de tensão para carga
   são 0.035, 0.055, 0.086, **0.0133**, **0.0208**, 0.29, 0.357 … (uma progressão geométrica com razão de cerca de 1.56
   em todos os outros pontos). Entre 8.20 e 10.19 V a estimativa cai para cerca de 1-2 % em vez de 13-21 %, e ela
   não é monotônica. Abaixo de 2.81 V ("sem leitura") o fator é 1.0 (cheia). Nada neste binário chama o slot 5
   (os slots vizinhos 1-4 são usados por `CThreadMonitor`), e as tensões do mock são 0. O corpo só usa
   membros de `IPower`, então pode ser um padrão de `ipower.h` que outras aplicações da urna (ou um driver real que
   não o sobrescreva) herdariam. Impacto neste binário: nenhum.
2. **"Segredos de hardware" públicos** (`CUrnaMock` 8131/8134/8144): a tabela da chave do RDV (128 × 0x03), a tabela CEPESC
   (1024 × 0x01) e um bloco de 32 bytes (0x02) são constantes. A chave do RDV pode ser recalculada a partir de dados públicos (u01/u02
   já mostram isso). Para o CEPESC (BU, arquivos de digitais) a tabela é uma das entradas, ao lado de uma semente aleatória e da chave
   pública do TSE, então o efeito é desconhecido. Esperado em um simulador; só importa se a saída do simulador algum dia fosse
   tomada por saída real.
3. **`CUrnaMock::GetTabelaCepesc` (8144) não tem verificação de limites.** Ela faz `memcpy` de todo o vetor configurado para
   o buffer de 1024 bytes do chamador, e depois faz `memory.fill` de `1024 - n` bytes. Um vetor com mais de 1024 bytes
   estouraria o buffer na pilha do chamador e depois geraria um trap. Inalcançável (nada preenche o vetor).
4. **Identidade fixa**: o modelo 2020 e a identificação 87654321 estão fixos no código. Todo caminho dependente de modelo do build
   web segue o ramo da UE2020 (teclado "abaixo", captura de digital, porta da MR, log do MSE, modo de menu do MT).
5. **`CStringUtils::ProcuraRegex` (2199) engole toda exceção** (`catch (...)` → `{-1,-1}`). Um padrão
   malformado, ou o `logic_error` do boost, vira "sem casamento". O segundo campo é o offset do fim, não o comprimento, como
   diz a declaração da u20. Os chamadores que o usam como comprimento só funcionam porque exigem um casamento na posição 0.
6. **`CRelUtil::FormataCorrespondencia` (2792)**: sem verificação de comprimento. Um `codigoCarga` com menos de 21 caracteres
   lança `std::out_of_range`. Ela é alcançada a partir das telas montadas pelo blob de `CTelasVota` (7787; uma amostra do profiler
   na sessão de eleição geral, a partir da tela "O horário de emissão da zerésima passou",
   `ctelasvota.cpp:2382`), então um cenário cujo estado tenha um código curto poderia falhar enquanto as telas são montadas. Dados que passaram por `CCarga::ValidaCriacao` (24 dígitos, u05) não conseguem provocá-lo.
7. **Habilitação pelo operador no modo de demonstração (3883).** `CInformaAnoDesabilitadoDemo` e
   `CInformaBioDesabilitadaDemo` compartilham o corpo de "ELEITOR(A) PODE VOTAR". No *modo demonstração* a
   verificação do ano de nascimento / da digital é pulada e CONFIRMA habilita o eleitor. Isso é por projeto (demonstração), e está morto
   no build web.
8. **Lock com espera ativa em 2860/3853**: o `CUpgradeMutex::lock_shared` inlinado fica em laço sobre
   `condition_variable::wait`, que retorna imediatamente sem pthreads. Uma requisição bloqueada giraria para sempre
   (mesmo achado de u19 §9.1; não disparado nas sessões gravadas).
9. **Parâmetros de TTS mortos**: os slots 1/2/3/8 de `ITextToSpeech` nunca são chamados. Ninguém lê `m_p76`, e o
   perfil continua `""` (a u32 observa que o perfil vazio substitui "Letícia-F123" na primeira requisição).
   `SetVelocidade` aceita qualquer nível sem verificação de intervalo (só é chamada com 2).

## 10. Questões em aberto

* O significado real dos slots 1, 2, 3, 5 de `IUrna` (sem chamador) e dos bits de status 0x800/0x2000 de `IPower` e dos
  campos de status +4/+6/+8/+10/+14. Os nomes dados são palpites.
* Se os slots 0-6 de `IPower` são overrides do mock ou padrões de `ipower.h` (isso decide se o achado 9.1 pode
  alcançar a urna real).
* O propósito de `m_p76` / `m_p88`. (O formato da chave do cache de TTS é conhecido: `"{}:{}:{}:{}:{}"` @1147, func 5758,
  já presente no `MontaChave` da u02.)
* Se 2763/2677/2199 são membros de `api::CStringUtils` ou funções livres de outro arquivo de `api/util` (sem srcloc).

## 11. Revisão de fidelidade (passada adversarial)

Comparadas linha a linha com `q.py f`/`wat`: todas as 15 funções de mock (8071-8168, mais os construtores inlinados em
8302), os 9 slots de `ITextToSpeech`, 10815, 11582, 2199, 2792, 2743, 3883, 2895, 3889, 2779, 3677, 3058, 2244,
1005, 1562 (e os seus 8 wrappers/slots), 2860, 3853, 3903, 1565, 1566, 3940, 2904, 3628, 2288, 1535, 2746, 3620,
2723, 1936, 2677, 2763, e os corpos de biblioteca 590, 854, 1144, 1896, 2214, 3266, 3547. A execução de velocidade da fala de
§2.3 foi repetida (mesmos tamanhos de WAV, byte a byte). Correções feitas:

* A 1565 não é apenas o D0 de `~CDataTextFmt`: os corpos ICF 2777/3681/5516 que a chamam são o D0 de nove
  classes folha de `IFormField` (CBeepFieldMT … CNewLineFieldPaper), passando a vtable de `IFormFieldBase<MEDIA>`.
* `votaInit` posta a mensagem 9 somente quando `audioEleitorHabilitado` está definido, e antes da mensagem 1.
* A 2677 recebe `texto` por `const&` e o copia ela mesma (chama `__init_copy_ctor_external`), não por valor.
* A 2199 é observada apenas por meio de `CTradutorFrase::TraduzLabel`, nunca por meio de `GetVersionNumber`.
* O formato da chave do cache de TTS é `"{}:{}:{}:{}:{}"` (@1147); a u02 já o tinha, com a mesma ordem de argumentos.
* `IUrna` tem 14 pontos de chamada em 13 funções (CEscolheOpcao tem dois), não 13 pontos.
* `cpowermock.h` dependia de um membro `IPower::m_status` e de nomes de slots que o `ipower.h` da u18 não tem
  (agora sinalizado nos dois lugares); o fragmento da 3883 usava `EInputResult::CONFIRMA` em vez do `Confirma` da u15;
  o fragmento de hash não tinha o erro 5102 (`CHashDiretorio: nome inválido.`, `chashdiretorio.cpp:41`).
* Fora desta unidade, `docs/libraries/rhvoice.md` (linha 179) e a nota curada da 10815 em
  `analysis/names.override.json` ainda dizem `std::cerr`; o stream é `std::clog` (§2.3).

# u02 — `ecourna-lib/ecourna/api/security` (`chkdfseed.cpp`) e a função de inicialização 7787 do VOTA

A unidade u02 foi montada em torno de um arquivo-fonte, `ecourna-lib/ecourna/api/security/chkdfseed.cpp`, e tem
160 funções wasm (63 delas executaram durante os votos gravados). Apenas duas delas, os destrutores 11497/11498,
vêm de fato desse arquivo. O seu construtor e `GetSeed()` estão inlinados em outros lugares. O montador de unidades anexou o resto por causa de uma
função com nome errado:

* **A func wasm 7787** tem 148 912 bytes e 65 253 instruções, a maior função de aplicação do módulo. As ferramentas
  primeiro a chamaram de `ecourna::api::security::CHKDFSeed::GetSeed` porque o primeiro registro `std::source_location`
  no seu corpo é `chkdfseed.cpp:47`. Na verdade, ela é a **rotina de inicialização do lado do eleitor do
  VOTA**. `votaInit` a chama uma vez (`invoke_v`, slot 39 da tabela). O LTO inlinou nela cerca de vinte funções de
  outros arquivos, e `CHKDFSeed::GetSeed` é uma delas. O seu nome não está no binário. Eu a chamo de
  `vota::CInformacaoEleitor::Inicializar()` (nome inferido, veja §3.1), e as ferramentas agora mostram esse nome.
* Quase todas as outras funções da unidade são **chamadas apenas pela 7787**. Exemplos são os montadores de telas do eleitor de
  `ctelasvota.cpp`, o cache de TTS, instanciações de contêineres, e singletons e destrutores dos conjuntos de dados do
  `comum`. A votação por vizinhança em `tools/wasmmap/units.py` também os atribuiu a `chkdfseed.cpp`.
  Alguns estados do vota chegaram da mesma forma, por meio de chamadas compartilhadas.

O que a unidade contém, portanto:

| parte | funções | reconstrução |
|---|---|---|
| o `chkdfseed.cpp` real: gerador de sementes HKDF-SHA512 `CHKDFSeed` | 11497, 11498 + ctor/`GetSeed` inlinados | `src/ecourna/api/security/chkdfseed.{hpp,cpp}` |
| `crdv.cpp`: derivação do cifrador do RDV (`GetCifradorCryptoTable`, `CRdv::CRdv`), apenas inlinada na 7787 | inlinado | `src/uenux2/src/app/comum/dados/crdv.cpp` |
| a função de inicialização 7787 e os membros de `CInformacaoEleitor` inlinados nela | 7787 | `src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.{h,cpp}` |
| pedaços fora de linha de arquivos de outras unidades (telas, montador de GUI, cache de TTS, conjuntos de dados, estados) | 145 | **fragmentos** `*.u02.cpp` ao lado do arquivo do dono (veja §10) |
| instanciações da libc++ (`__tree::destroy`, `std::sort<uebyte*>`, helpers de hash map e de vector) | ~45 | apenas listadas (tabela de mapeamento) |

`crdv.cpp` e `cinformacaoeleitor.cpp` não têm nenhuma função wasm própria. O único código deles que sobreviveu está inlinado
na 7787, por isso esta unidade é dona deles. Um arquivo `*.u02.cpp` contém reconstruções de funções cujo arquivo
original pertence a outra unidade. O dono deve incorporá-lo; a u02 não editou os arquivos do dono.

---

## 1. Lugar no processo de votação (glossário)

* **Urna** é a máquina de votação. O VOTA roda dois lados: o **eleitor** (terminal do eleitor: telas, teclas,
  áudio) e o **operador/mesário** (terminal do mesário: identificação, habilitação do eleitor).
* **Dados estáticos** são os dados da eleição carregados de `/dsk/fi/estatico` (arquivos de cenário como
  `t02411ac00001-ca.dat`). **Dados dinâmicos** são os arquivos que a urna grava em
  `/dsk/{fi,fe}/dinamico/trab<turno>`: `vota.bin` (estado), `rdv.dat`, `uenux.db` e as assinaturas `*.vsu`.
  `fi` é a flash interna e `fe` a externa (o cartão de memória).
* **RDV**, *Registro Digital do Voto*, é a lista embaralhada de todos os votos,
  armazenada cifrada em `rdv.dat`. O BU (*boletim de urna*, o resultado por seção) é calculado a partir dela.
* **Zerésima** é o relatório impresso antes da votação, que comprova que todos os contadores estão em zero.
* **MR / MI**: *mídia de resultado* (o cartão de memória com os resultados) / *memória interna* (o texto "Gravando o banco de dados na MI").
* **EstadoVota** é a máquina de estados do VOTA persistida em `vota.bin` (§3.2).

A 7787 executa uma vez, durante `votaInit`, antes que qualquer tela seja mostrada. Ela restaura o estado persistido, valida o
pacote de software, abre o banco de dados SQLite, carrega cada conjunto de dados estáticos no seu singleton, verifica
a integridade referencial deles, monta todas as telas do eleitor e pré-carrega no cache o áudio de acessibilidade (uma operação nula no
build web, veja §3.3 passo 10).

---

## 2. `chkdfseed.cpp` e a chave do RDV

### 2.1 `ecourna::api::security::CHKDFSeed`

RTTI `class` (sem base), vtable @1560096 = `[0] ~CHKDFSeed()` (11498), `[1]` dtor de exclusão (11497).
40 bytes: `vptr, vector<uebyte> m_salt (+4), m_chave (+16), m_info (+28)`.

`GetSeed()` está inlinada na 7787 (dcmp 15947-16260). Ela retorna 128 bytes calculados pelo HKDF do OpenSSL:

| linha do fonte | chamada (slot da tabela) | código de erro (`ESecurityError`, faixa 1325..1725) |
|---|---|---|
| 47 | `EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF=1036, NULL)` (6218) | 1397 |
| 50 | `EVP_PKEY_derive_init(ctx)` (6219 → func 7373, cujas strings de erro dizem `EVP_PKEY_derive_init_ex`: o `derive_init(ctx)` do OpenSSL 3 é `derive_init_ex(ctx, NULL)` com o NULL propagado como constante) | 1398 |
| 53 | `EVP_PKEY_CTX_set_hkdf_md(ctx, EVP_sha512())` (6221; `EVP_sha512()` reduzido à constante 1646408) | 1399 |
| 56 | `EVP_PKEY_CTX_set1_hkdf_salt(ctx, m_salt)` (6222) | 1400 |
| 59 | `EVP_PKEY_CTX_set1_hkdf_key(ctx, m_chave)` (6223 — o `.dcmp` a imprime como `622 /*…*/3`, um erro de parsing) | 1401 |
| 62 | `EVP_PKEY_CTX_add1_hkdf_info(ctx, m_info)` (6224) | 1402 |
| 67 | `EVP_PKEY_derive(ctx, out, &len=128)` (6225) | 1403 |
| — | `EVP_PKEY_CTX_free(ctx)` (6226), **apenas no caminho de sucesso** | |

Todo erro lança `CBaseError<ESecurityError>` com a mensagem "Falha ao gerar as sementes criptográficas HKDF.".

### 2.2 Seu único usuário: `comum::(anonymous)::GetCifradorCryptoTable(cargos)` (crdv.cpp:60/78)

`CRdvVota::CreateInst()` → `CRdvVota()` → `CRdv(make_unique<CRdvPosicionadorVota>(), cargos,
CConfiguracaoEleicao +164)` → `m_cifrador = GetCifradorCryptoTable(cargos)`, tudo inlinado:

1. `cargos` é a lista **ordenada** de códigos de cargo (byte 0 de cada `CCargo` de 140 bytes, ordenada com
   `std::sort<uebyte*>`, funcs 4811–4820). Cenário municipal: `{11 Prefeito, 13 Vereador}`.
2. `tabela[128]` = slot 6 da vtable de `CPolySingleton<api::IUrna>::instance()` (linha 60). **No simulador, IUrna é
   `api::teste::CUrnaMock`. O seu slot 6 (func 8131) é `memset(tabela, 3, 128)`.**
3. `hash = SHA-512(cargos)` (CSha512 = 2684, Update 3519, Finish 3518).
4. `indice[i] = hash[32+i] ^ ((hash[i] & 0xC0) << 2)` para i = 0..31 (`vector<uint16_t>`, `.at()`),
   `chave[i] = tabela[indice[i] & 0x7F]`. Depois `tabela` é zerada.
5. `CHKDFSeed(salt = hash, chave, info = "RDV").GetSeed()` → 128 bytes.
6. Slot 3 de `CPolySingleton<ISymmetricCipherFactory>::instance()` (`CSymmetricCipherFactory::vf3`, 9489, linha 78)
   com **chave = seed[0..32) (AES-256), IV = seed[32..48)**. O resultado é um `CBlockCipher<CAesCipher, CTrng>` no modo CBC.
7. O construtor de `CRdv` então verifica o comprimento do número de partido (byte +164): 0 → `EUeRdvError 4650` "Número de dígitos
   do partido nulo" (linha 90), ≥ 6 → 4651 "Número de dígitos do partido ({}) supera o limite (5)" (linha 93).

**Verificação em tempo de execução.** Uma reimplementação em Python puro (hmac/hashlib) dos passos 1–6 desta reconstrução, com
`cargos = [11, 13]` e uma tabela de `0x03`, dá a chave `c4f9eff4…495a4952` e o IV `568d9298776ea63bc8a58367190a4d51`.
São os mesmos valores que a u01 registrou a partir de `EVP_EncryptInit_ex` em tempo de execução. Eles decifram
`analysis/runtime/memfs-after-init/dsk/fi/dinamico/trab1/rdv.dat` (80 bytes; idêntico em `fe`):

```
openssl enc -d -aes-256-cbc -nopad -K c4f9eff475648bef6767d69e414babb26429a0e0f4aa4cc5227d2aa6495a4952 \
            -iv 568d9298776ea63bc8a58367190a4d51 -in rdv.dat
30 38 | 02 02 09 6a | 0a 01 03 | 30 0f (30 06 02 01 01 02 01 01) 02 02 03 f3 02 01 01 | 30 00 |
a1 1c 30 1a 02 02 09 6b 30 14 (30 08 81 01 0b 02 01 01 30 00) (30 08 81 01 0d 02 01 01 30 00)
06 06 06 06 06 06 | 10 × 16
```

Isto é `ModuloRegistroDigitalVoto.EntidadeRegistroDigitalVoto`: pleito 2410, fase 3 (treinamento),
identificação {município 1, zona 1}, local 1011, seção 1, `historicoCodigosCarga` vazio, `eleicoesVota`
{idEleicao 2411, cargos 11 e 13 com `quantidadeEscolhas` 1 e nenhum voto}. Em seguida vêm um padding PKCS#7 da aplicação
(6 × 06) e o padding do EVP (16 × 10). O snapshot depois do voto tem os mesmos bytes, porque o build web não persiste votos no
RDV (u05 §3.3, u07 §4.3).

Notas de segurança (veja "suspicious"):

* No simulador, qualquer pessoa pode decifrar `rdv.dat`: a chave depende apenas da lista pública de cargos e da
  tabela constante do mock. Isso é esperado em um simulador.
* O **IV é determinístico**: ele é fixo para um dado par (tabela, lista de cargos). Toda regravação de `rdv.dat` em uma urna real
  que execute o mesmo código usa a mesma chave e o mesmo IV no modo CBC.
* O XOR do passo 4 não tem efeito: `(x & 0xC0) << 2` só afeta os bits 8–9, e `& 0x7F` em seguida os zera. O bit 7 de cada
  `hash[32+i]` também é descartado. O índice de 10 bits sugere uma tabela de 1024 entradas. O slot 4 de `IUrna` de fato preenche 1024 bytes
  (`CUrnaMock::vf4`), mas este código lê a tabela de 128 bytes.
* `EVP_PKEY_CTX` vaza em todo caminho de erro de `GetSeed`.

---

## 3. Func wasm 7787 — `CInformacaoEleitor::Inicializar()` (nome inferido)

### 3.1 Por que este nome

`vota::CInformacaoEleitor` é um singleton de 12 bytes (`GetInst` = wasm 509, ponteiro @1833284). O seu layout é
`+0 dadosEstaticosCarregados`, `+1 dadosDinamicosCarregados`, `+4 m_modoAudio` (2 = desligado; veja `cinformacaoeleitor.u07.cpp` da u07),
`+8 persistenciaInicializada`, `+9/+10` flags. A 7787 lê e liga essas flags. A função contém
o único `std::source_location` de `cinformacaoeleitor.cpp` (linha 181, dentro de
`CarregarDadosEstaticos()`). `votaInit` a chama logo antes de `CInformacaoEleitor::GetInst()` + wasm 6737
(`GerarDadosDinamicos`, também chamada por `vota::CGeraDadosDinamicos::StartState`). A 6734 (ferramentas:
`CEleitores::CompleteLoad`) recebe o mesmo objeto como `this` e testa a flag +1. Isso faz dela
`CarregarDadosDinamicos()`. A 7787 não recebe parâmetros, então é um membro estático ou uma função livre. O
nome `Inicializar` é um palpite.

### 3.2 `EstadoVota` em C++

O enum C++ de `CEstadoGeralVota` é `'1' + ASN.1 value`, como mostram `CConversorEstadoGeralVota::Converte/DesconverteEstadoVota`
(11390/11391): INICIAL `'1'`, GERABASEDINAMICA `'2'`, AGUARDAHORAZERESIMA `'3'`, GERARZE `'4'`,
ZERESIMAGERADA `'5'`, ZERESIMAIMPRESSA `'6'`, REGISTROMESARIOINICIAL `'7'`, VOTAR `'8'`, FIMAQUISICAOVOTOS
`'9'`, REGISTROMESARIOFINAL `':'`, GERARBU `';'`, GERARRELATORIOS `'<'`, IMPRIMIRBU `'='`, GRAVARRESULTADOS `'>'`,
COPIARESULTADOSMR `'?'`, ENCERRADA `'@'`. O valor ASN.1 16 (`exibealertadesligamento`) lança "Estado não deve ser
usado: {}".

### 3.3 Passo a passo (faixas de linhas do dcmp em `decompiled/app-api/ecourna-lib/ecourna/api/security/chkdfseed.cpp.dcmp`)

| # | dcmp | o que acontece (função inlinada → dono) |
|---|---|---|
| 1 | 6674 | global @1577208 = `CAppInfo::GetGeral()` +48, a **fase** da urna (`CEstadoGeral` +48, veja u06/u20; o mesmo campo inicia a identificação do nome de arquivo 5728 e alimenta o separador de relatório 2785). O cabeçalho de status de `CFormBuilder` (5564) mostra '2' como "SIMULADO" e '3' como "TREINAMENTO" |
| 2 | 6675–7132 | `CAppInfo::CarregaVotaInternoEmCache()`: `vota.bin` da flash interna (`CServicoEstadoGeralVota`) → `CFileASN::ReadFromFile<EstadoGeralVota>` (limite de 5 MiB) → `CConversorEstadoGeralVota` → cache por turno |
| 3 | 7133–7200 | `estado = GetVota(TURNO_ATUAL).estadoVota`. Se for INICIAL, passa a GERABASEDINAMICA e é salvo (wasm 491, "Gravando o estado da urna"). Se for GERABASEDINAMICA (uma geração interrompida), `ApagarDadosDinamicos()` remove `rdv.dat` e `uenux.db` nas duas flashes (1551/1701/2826/5762) |
| 4 | 7202–7215 | `CPacoteArquivos::ValidarChaveEAplicacaoValida(GetPathTrab(interna))` e `(externa)` |
| 5 | 7216–7607 | `InicializarPersistencia()` (uma vez, flag +8): abre `trab/uenux.db` e registra `CComparecimentoMesarioDAO` e `CJustificadorDAO` em `CDAORepositorio` (map @1909964 indexado por `typeid(I).name()`). Cada DAO executa `CREATE TABLE IF NOT EXISTS` (abaixo). Depois @1838488 = 1 e `CSincronizaVota::SincronizaBancoDados()` (4662: `VerificaUrnaDesligando()` inlinada, depois 4657: assina `uenux.db`, copia-o para a flash externa, depois a cópia para a MV 4682) |
| 6 | 7608–18393 | se `estado` ∈ '1'..'@': `CarregarDadosEstaticos()` (uma vez, flag +0), §3.4 |
| 7 | 18394–18395 | ainda dentro do teste de faixa do passo 6 (o desvio `estado - '1' > 15` também pula por cima dele): se `estado` ≥ AGUARDAHORAZERESIMA, `CarregarDadosDinamicos()` (6734). Um estado fora da faixa não carrega nem os dados estáticos nem os dinâmicos |
| 8 | 18396–25754 | `CTelasVota::CreateInst()` (ctelasvota.cpp:3435/3502): monta todas as telas do eleitor; objeto de 252 bytes @1833396 |
| 9 | 25755–25776 | `CInfoMTLCD::CreateInst()` (cinfomtlcd.cpp:33): objeto de 64 bytes @1838572 |
| 10 | 25777–25928 | `CInstrucaoVotacaoAcessibilidade::GetInst().CacheInstructionAudio()`: para cada uma das 5 velocidades de fala (60–140 %), `CacheMessage` (921) coloca no cache permanente de TTS os nomes das teclas 0–9, BRANCO, CORRIGE, CONFIRMA e o texto de instrução. Um '{' no texto lança 9315 "Não é permitido fazer cache de mensagem dinâmica". **No build web isto não sintetiza nada:** o singleton `ITextToSpeech` neste ponto é o `simulador::CWasmNullTextToSpeech` que `votaInit` empilha primeiro; `votaInit` cria e empilha `CRHVoiceTextToSpeech` só depois que a 7787 retorna (depois de 509/6737/11733). O slot 11 do objeto nulo (9436) retorna um `shared_ptr` vazio, então as 70 entradas são WAVs nulos no cache do objeto nulo, e o objeto RHVoice começa com o cache vazio. Em tempo de execução: `votaInit` leva 76 ms com `--audio` e 74 ms sem (headless, municipal-t1) |

SQL executado no passo 5 (strings @435498 e @435360):

```sql
CREATE TABLE IF NOT EXISTS comparecimento_mesario (
  numero_titulo BIGINT NOT NULL, tipo_identificador INTEGER CHECK(tipo_identificador IN (1,2,3)) NOT NULL,
  periodo_presente INTEGER NOT NULL, pertence_secao INTEGER NOT NULL, estado_biometria INTEGER NOT NULL,
  dedo_habilitacao INTEGER NOT NULL, data_hora_registro DATETIME NOT NULL, id_arquivo INTEGER,
  UNIQUE (numero_titulo, periodo_presente) )
CREATE TABLE IF NOT EXISTS registro_justificativa (
  numero_titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, ano_nascimento SMALLINT )
```

### 3.4 `CarregarDadosEstaticos()`: conjuntos de dados estáticos, na ordem de carga

| dcmp | conjunto de dados (dono) | arquivo(s) / tipo | singleton |
|---|---|---|---|
| 7613–8090 | `CAppInfo::CarregaGapInternoEmCache()` | `gap.bin` → `EstadoGeralGap` | cache de CAppInfo |
| 8095 | `CLocal` (401 + 5740) | `*-lo.dat` → `ModuloLocal::Local` | @1838900 |
| 8096–10270 | `CConfiguracaoEleicao::CreateInst(eg, secoes, INTERNA)`: ProcessoEleitoral, ParametrizacaoUrna (`t02400ac-pu`, `t00000br-pu`), ConfiguracaoMunicipios, `CPleito` (3713), `CEleicaoDataHora` ("Datas inválidas"), depois `CCargos::CreateInst` e `CRespostas::CreateInst` | vários arquivos `ModuloProcessoEleitoral`/`ParametrizacaoUrna`/`ConfiguracaoMunicipios` | @1838752, @1838720, @1838984 |
| 10271–10978 | `CHV::CreateInst` (horário de verão / fuso horário do município) | `ComplementosMunicipios` | @1838872 |
| 10979–11760 | `CPartidos` (819) | `*-pa.dat` por abrangência (nomes vindos de 2811) → `EntidadePartidos` | @1838928 |
| 11761–13091 | `CFederacoes::Load` | `*-fe.*` → `EntidadeFederacoes` | @1838748 |
| 13092–14028 | `CFotos::Load` | `*-fo.dat` (leitor ASN.1 parcial) | @1838844 |
| 14029–15643 | `CCandidaturas` (`LoadFromFile`) | `*-ca.dat` + `CabecalhoPacote` | @1838692 |
| 15644–16951 | `CRdvVota::CreateInst` → `CRdv` → **§2.2** | — | @1838956 |
| 16953–17153 | se `CConfiguracaoEleicao` +665 (urna biométrica): slot 2 de `CPolySingleton<IFingerPrepare>` (linha 181). No build web isto é uma operação nula (`CFingerPrepareSimulador`) | — | — |
| 17154–18039 | `CEleitores::StaticLoad` (celeitores.cpp:395/407) | `*-el.*`, `*-tte.dat` (nomes 3745/5727/3772) | @1838792 |
| 18040–18392 | quatro verificações de integridade, cada uma seguida de `CIntegridadeReferencial::Lanca` (2263): cargo de cada candidatura (5747), partido de cada candidatura, foto do candidato e dos suplentes (5746), partidos de cada federação. Depois a flag +0 = true | — | — |

---

## 4. Telas do eleitor montadas pelo construtor inlinado de `CTelasVota`

Os montadores de telas fora de linha desta unidade (eles pertencem a `ctelasvota.cpp`, dono u07) seguem todos um padrão:

```cpp
api::CFormBuilder builder;                                  // zero-initialised by the caller
adicionaAnimacao(tipo, builder, conferencia);               // wasm 1593 (only for vote/review screens with a GIF):
                                                            //   votoLegenda/votoBranco/votoNulo.gif
adicionaBase<Tela>(builder, __func__, cargo, ...);          // srcloc-named helpers 6602..6678 (u07)
adicionaInstrucoesConfirmaCorrige(builder, "CONFIRMAR este voto", largura)   // 1102, or
adicionaRodapeConfiraSeuVoto(builder, largura)                               // 1190 (review screens)
builder.AddControlInput() | AddNumberInput(...)            // 901 / 2383
return CriaFormInterativo(builder, make_shared<CPreShowProgressBar>(), "tela<Nome>");   // 554
```

`__func__` fica visível porque os helpers o recebem para as suas mensagens de erro. A string
"CriaTelaVotoNuloCandidato" fica ao lado do nome de formulário "telaVotoNuloCandidato", então os dois nomes podem ser recuperados
com alta confiança.

| func | tela | nome do formulário |
|---|---|---|
| 1591 / 1767 | voto nulo para um cargo de candidato / a sua conferência | `telaVotoNuloCandidato` / `telaConferenciaVotoNuloCandidato` |
| 3063 / 3067 | voto em branco / conferência | `telaVotoBrancoCandidato` / `telaConferenciaVotoBrancoCandidato` |
| 6639 / 6681 | tela completa de candidato sem foto (Com0) / conferência | `telaCompletaCandidatoCom0` / `telaConferenciaCandidatoCom0` |
| 6626 / 6616 | voto nulo em uma *consulta* (referendo) / conferência | `telaVotoNuloConsulta` / `telaConferenciaVotoNuloConsulta` |
| 6601 | "FIM" (fim do voto) | `telaFim` |
| 2380, 6557 | mensagem centralizada ('&' = nova linha); "CONTINUAÇÃO DA VOTAÇÃO&IDENTIFIQUE O ELEITOR" | `telaNeutra` |
| 2376 | mensagem com rótulos CONFIRMA/CORRIGE opcionais | `telaTextoConfirmaCorrige` |
| 6592 | confirmar a impressão da zerésima | `telaConfirmaImpressaoZeresima` |
| 6595 | "Esta urna eletrônica só funcionará a partir de HHhMM do dia DD/MM/YYYY." | `telaAntesHorarioZeresima` |
| 6599 | menu "Mais informações": Estado da urna / Lista de eleitores / Versões de pacotes / Parâmetros de urna ({vias}/{max}), Visualizar candidatos; Processo eleitoral, Pleito, "Local com biometria: Sim/Não" | `telaMaisInformacoes` |

Helpers: 4161 faixa inferior "Município: 00001 - NAME     Zona: 0001     Seção: 0001", com os rótulos `<MCSN>/<ZCSN>/<SCSN>`
traduzidos por `CTradutorFrase`. Para uma urna de contingência (sem seção: `md::CLocal` +120 desligado, +136 ligado) o texto é
"Município: 00001 - NAME     Zona: 0001     CONTINGÊNCIA" (constante @335107); sem nenhuma das duas flags a faixa não tem texto. A 4160 é o cabeçalho da zerésima (`<ZCSA>/<SCSA>`, seções agregadas). A 2384 é o bloco de entrada do número.
As primitivas do montador de GUI (202 AddText, 3680 AddRect, 2245 AddFill, 3678/2243 teclas com rótulo, 6689 imagem
de dados, 2024 ctor de `IInputField`, 554/5550 fábricas de formulários) estão em `cformbuilder.u02.cpp`.

---

## 5. Outro código trazido para a unidade

### 5.1 Estados do VOTA

* **`vota::CAguardaMensagem::ProcessMessage(int)`** (7481, slot 6 da vtable) é o terminal do eleitor esperando o
  mesário. Mensagem 1 → log "Eleitor foi habilitado" → `CEleitorVotando` (visto em
  `logd.dat` depois do voto gravado). Mensagem 7 → **início do encerramento** (§6). Mensagem 11 → `CIniciodeCiclo`. Mensagem 12 →
  `CInspecionaUrna` ("Por favor, inspecione cabina e urna.", tecla CONFIRMA = "Continuar"). Mensagem 14 →
  `CMostraTelaContinuaVotacao`.
* `CInspecionaUrna::ProcessInput` (11797): CONFIRMA → `CUrnaInspecionada`, log "Inspeção da urna confirmada",
  mensagem 11 na fila de prioridade.
* *Teste do teclado*: `CTesteFalhou`/`CEnviarManutencao::ProcessInput` (11809/11812) e
  `CEsperaRetestar::GetInst` (5936, "Por favor, espere {}s para a realização de uma nova tentativa").
* Visualização de candidatos: `CMenuFiltrarCandidatosPorNumero::ProcessInput` (11891, "Candidato não encontrado!" por 2 s),
  o preenchedor da lista de candidatos dos filtros por partido/cargo (5953) e o slot 2 de `CItemVisualizarCandidatosVota` (12244).
* `CRetirarMR::GetInst` (2291) são as telas de "retirar/manter o cartão de memória de resultado", incluindo a tela do modo de demonstração
  `telaMRModoDemo`.
* Contadores do operador: `TextoHabilitacao{Biometrica,Biografica,SemBiometria}` (10701/10700/10699) mostram "n.a." quando
  o local de votação não é biométrico.

Os objetos de estado são singletons preguiçosos. Os simples compartilham o corpo mesclado 764
(`vota_f764(mutex, &inst, vtable, arg)`), por exemplo `CIniciodeCiclo::GetInst` = 4433.

### 5.2 Cache de TTS (`api::ITextToSpeech`, 921, 5750/5754/5758, 11436 …)

`ITextToSpeech` tem dois caches: um LRU limitado (`std::list` + `unordered_map<string, list::iterator>` em +4) usado pela
síntese normal (slot 0 da vtable, 11530), e um `unordered_map<string, shared_ptr<CWavFile>>` permanente em +40
preenchido por `CacheMessage` (921). A chave (5758) é `std::vformat("{}:{}:{}:{}:{}", texto, taxa (+84), perfil (+64),
int +80, int64 +88)`; o int em +76 não faz parte dela. Cada velocidade de fala tem, portanto, as suas próprias entradas. O passo 10 da §3.3
chama `CacheMessage` para 14 textos × 5 velocidades, mas no build web ele roda contra `CWasmNullTextToSpeech` (veja o passo 10),
então nenhuma síntese acontece na inicialização. Depois de `votaInit`, o TTS é `CRHVoiceTextToSpeech` (áudio ligado) ou um novo
`CWasmNullTextToSpeech`.

### 5.3 Helpers do comum

Os acessores de `CLocal` (1004 GetMunicipio, 1077 GetNomeMunicipio, 1702 GetUF, 5743 GetLocalData, 820
UrnaBiometrica) passam o seu próprio nome para `CLocal::VerificaLido`. `CNomeArquivo::MontaNome` (3772) monta
`<fase><pleito:05><uf><mun:05><zona:04><secao:04>-<suf>.<ext>`, por exemplo `t02400ac0000100010001-tte.dat`.
`CMenuBase` (2285 AdicionaItem, 5913 Monta: "[{}] - {}", "Escolha a sua opção:"). Apenas os itens cujo slot 2
(`Disponivel`, por exemplo 12244) retorna true adicionam o seu id às opções aceitas; os itens indisponíveis são desenhados com
o estilo 5 em vez do 2. O slot 3 do relatório
`CGeradorRelVersaoPacoteDados` (11223) imprime "Versões de Pacotes" com "Código do processo eleitoral",
"Código do pleito 1/2". `comum::GravaBancoDadosNaMI` (4657) assina `uenux.db` e o copia para a
flash externa (CArquivosSavd ids 110/111 = `uenux.db`), depois chama 4682 ("Gravando o banco de dados na MV":
copia CArquivosSavd 199 → 201 no 1º turno, 200 → 202 caso contrário).

---

## 6. BU / encerramento: o que esta unidade contribui

A unidade não gera o BU. Ela tem dois pedaços do caminho que leva até lá:

1. **O gatilho.** Quando o mesário encerra a votação, o lado do operador envia a **mensagem 7** ao lado do eleitor.
   `CAguardaMensagem::ProcessMessage(7)` (7481) então faz, nesta ordem:
   `GetVota(TURNO_ATUAL).estadoVota = GERARBU (';')` → `comum::SalvaEstadoVota()` (wasm 491: assina e sincroniza
   `vota.bin` nas duas flashes; texto de erro "Ocorreu um erro durante a sincronização do estado geral do VOTA.") →
   `CLogVota` "Inicio do Encerramento" → próximo estado `CGeraBU::GetInst()` (6129, preguiçoso @1833496). Esse estado
   primeiro mostra "Votação encerrada" e "Preparando dados para encerramento" (`telaVotacaoEncerrada`,
   `telaPreparandoDadosEncerramento`). A geração do BU em si está na u08 (`cgerabu.cpp`) e na u25.
2. **A chave do RDV** (§2.2). O BU é calculado a partir do RDV (u05: slots 0–10 de `CRdvVota`). `rdv.dat` é protegido por
   AES-256-CBC com a chave e o IV de `GetCifradorCryptoTable`. Ao reiniciar, o passo 7 da 7787 o recarrega (6734 → `CEncryptedFile::Load`
   → `Desconverte`) e a 2543 verifica de novo o RDV contra cargos, candidaturas, partidos e respostas.

`CRetirarMR` (2291) é o fim do fluxo de encerramento na tela do eleitor: "Retire a mídia de resultado e faça a entrega
conforme as instruções." e "Lacre a tampa do compartimento da mídia de resultado conforme as instruções."

---

## 7. Relações entre classes (RTTI)

```
ecourna::api::security::CHKDFSeed                         (no base; vtable @1560096)
comum::CRdv  <- comum::CRdvVota                            (vtables @1559900 / @1560172)
comum::asn::CConversorEleicoesVota <- CConversorRegistroDigitalVoto<CConversorEleicoesVota>   (@1571224 / @1560304)
comum::IConversorASN<EstadoGeralVota, CEstadoGeralVota> <- comum::asn::CConversorEstadoGeralVota
api::IUrna <- api::teste::CUrnaMock                        (simulator: slot 6 = memset 3, slot 4 = 1024 bytes, slot 5 = 32 × 0x02)
api::IFingerPrepare <- simulador::CFingerPrepareSimulador  (slot 2 = no-op)
api::ITextToSpeech <- api::CRHVoiceTextToSpeech, simulador::CWasmNullTextToSpeech
ecourna::api::security::ISymmetricCipherFactory <- CSymmetricCipherFactory
comum::CAppState (api::CState) <- vota::CAguardaMensagem, CInspecionaUrna, CUrnaInspecionada, CRetirarMR,
     CMostraTelaContinuaVotacao, CIniciodeCiclo, CGeraBU, CEleitorVotando, testeteclado::{CTesteFalhou,
     CEnviarManutencao, CErroTesteTecladoFim, CEsperaRetestar}, CMenuFiltrarCandidatosPorNumero, ...
comum::CMenuBase <- vota::CMenuMaisInformacoesVota;  comum::CItemMenu <- CItemImprimeEstadoUrna(Vota), CItemImprimeListaEleitores(Vota),
     CItemVersoesPacotes(Vota), CItemParametrosUrna(Vota), vota::CItemVisualizarCandidatosVota
comum::dao::IComparecimentoMesarioDAO <- CComparecimentoMesarioDAO;  IJustificadorDAO <- CJustificadorDAO
```

Singletons não polimórficos que a 7787 preenche: `vota::CInformacaoEleitor`, `comum::CAppInfo`, `CLocal`, `CConfiguracaoEleicao`,
`CCargos`, `CRespostas`, `CHV`, `CPartidos`, `CFederacoes`, `CFotos`, `CCandidaturas`, `CEleitores`, `vota::CTelasVota`,
`comum::CInfoMTLCD`. Cada um tem um handler de atexit (por exemplo 11481, 11495, 11535, 12882, 11551, 11553, 11561).

---

## 8. Particularidades do build web

* `api::IUrna` é `api::teste::CUrnaMock`. A "tabela de criptografia" do RDV é 128 × `0x03`, então a chave do RDV é pública (§2.2).
* `IFingerPrepare` é `CFingerPrepareSimulador`, cujo `Prepara` é uma operação nula. Os dados do cenário decidem se a urna é biométrica.
* As assinaturas `.vsu` são falsas: `votaInit` grava "assinatura simulada para vota_web_wasm", e `CWasmSavd` assina.
  `ValidarChaveEAplicacaoValida` e `AssinarUE` nos passos 4–5 trabalham, portanto, sobre assinaturas simuladas.
* Depois do passo 5, `uenux.db` é copiado para `/dsk/fe/dinamico/trab1` (4657, observado em tempo de execução).
* `CSincronismoVotoEleitorWeb` nunca grava votos. `rdv.dat` mantém o RDV vazio gravado na inicialização (§2.2).
* A 7787 não tem chamadas a imports de JS nem `emscripten_sleep`. Ela roda de forma síncrona dentro de `votaInit`, e qualquer exceção vai para
  o `catch (std::exception&)` de votaInit ("erro desconhecido em votaInit" para as demais).

## 9. Observações sobre Wasm / Emscripten

* **LTO + um único chamador = uma função gigante.** Cerca de 20 funções de 15 arquivos (só `CTelasVota::CTelasVota` tem ~7 000 linhas
  de dcmp) foram inlinadas na 7787. O resultado fica muito acima do limite do inliner, por isso não foi inlinado em `votaInit`. A chamada fica
  no bloco `try` de votaInit, então é um `invoke_v` pelo slot 39 da tabela. As ferramentas primeiro nomearam a função pelo *primeiro* srcloc
  do seu corpo (agora o nome é curado). Trate com desconfiança qualquer nome dado a uma função acima de ~20 KB.
* **merge-similar-functions** produziu vários corpos compartilhados com constantes como parâmetros: 6117 (`CriaForm<PRESHOW>`,
  vtables como argumentos), 3942 (copiar e transformar string, a transformação como slot da tabela), 6035/6036 (o mesmo helper de caminho
  especializado para a flash 0/1), 764 (singletons preguiçosos de estado).
* **O detalhe de `__constexpr_memmove` da libc++.** A 3713 copia `n-1` bytes para um vector de `n` bytes. A libc++ copia
  `(n-1)*sizeof(T) + __datasizeof(T)`, e para uma classe vazia `__datasizeof` é 0. Portanto isto não é um bug. Isso mostra
  que o tipo do elemento (as "situações" de `CPleito`) é uma classe vazia.
* **A emulação de shared_mutex inlinada em `CPolySingletonList::instance`.** Ela aparece duas vezes na 7787 (dcmp 16304–16470, 16982–17146).
  `condition_variable::wait` é uma operação nula neste build de thread única, então o laço `while (writer || writersWaiting) wait()` giraria
  para sempre se uma flag de escritor ficasse ligada.
* **Fontes** são passadas como endereços de descritores estáticos (474880 = 40 px, 474888 = 30, 474896 = 20, 475016 = 35, …).
  As coordenadas de tela são pares `{x, y}` int16 compactados guardados como um único int32 (por exemplo 26542081 → {1, 405}).

---

## 10. Arquivos reconstruídos

| arquivo | conteúdo |
|---|---|
| `src/ecourna/api/security/chkdfseed.hpp`, `.cpp` | `CHKDFSeed` (ctor, dtor 11497/11498, `GetSeed`) — **o arquivo próprio da unidade** |
| `src/uenux2/src/app/comum/dados/crdv.cpp` | `GetCifradorCryptoTable`, `CRdv::CRdv` (arquivo órfão, inlinado na 7787) |
| `src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.{h,cpp}` | layout de `CInformacaoEleitor`, `Inicializar` (7787), `ApagarDadosDinamicos`, `InicializarPersistencia`, `CarregarDadosEstaticos` |
| `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp` | fragmento para a u07: CriaTela* e helpers de rodapé, `~CTelasVota` |
| `src/uenux2/src/api/gui/cformbuilder.u02.cpp` | fragmento para a u15/u17: primitivas do montador, fábricas de formulários, ctor de `IInputField`, wrappers de formulários do VOTA |
| `src/uenux2/src/api/audio/itexttospeech.u02.cpp` | fragmento: cache permanente/LRU de TTS |
| `src/uenux2/src/app/comum/dados/{clocal,cconfiguracaoeleicao,cintegridadereferencial,crdvvota,cdadosestaticos}.u02.cpp` | fragmentos para a u03/u04/u05 (helpers de conjuntos de dados, destrutores, singletons) |
| `src/uenux2/src/app/comum/{cpath,cmenubase}.u02.cpp`, `comum/nomearquivo/cnomearquivo.u02.cpp`, `comum/relatorios/cgeradorrelversaopacotedados.u02.cpp`, `comum/util/cstringutil.u02.cpp` | fragmentos do comum |
| `src/uenux2/src/api/util/cdatetime.u02.cpp`, `src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp` | fragmentos de api/ecourna |
| `src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp` | `CAguardaMensagem::ProcessMessage`, `CInspecionaUrna::ProcessInput`, `CRetirarMR::GetInst`, `CacheInstructionAudio` (inlinada) |
| `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp`, `.../auxiliares/cmenufiltrarcandidatos.u02.cpp`, `src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp`, `src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp` | fragmentos do vota |

## 11. Suspeito / digno de nota (resumo; detalhes no StructuredOutput)

1. **Chave do RDV derivável de dados públicos no simulador.** A tabela do `IUrna` mock é 128 × `0x03`. Isto foi verificado decifrando `rdv.dat`.
2. **IV determinístico na cifragem do RDV.** A chave e o IV são ambos fixos por (tabela da urna, lista de cargos). Isso importa para a urna real se ela rodar o mesmo código.
3. **XOR morto / bits de índice descartados** em `GetCifradorCryptoTable`: apenas 7 dos 10 bits de índice calculados são usados.
4. **Vazamento de `EVP_PKEY_CTX`** em todo caminho de erro de `GetSeed`.
5. **Padding PKCS#7 duplo** em `rdv.dat` (padding da aplicação + padding do EVP). Isso é inofensivo, mas incomum. Os decodificadores precisam remover os dois.
6. **Risco de giro infinito** no lock compartilhado inlinado de `CPolySingletonList` (`condition_variable::wait` nulo).
7. `ApagarDadosDinamicos` apaga as duas cópias de `rdv.dat` e `uenux.db` sempre que `vota.bin` diz GERABASEDINAMICA
   (uma geração interrompida da base dinâmica). A regra depende apenas do valor de estado persistido.
8. **Espera ativa de 2 segundos em um handler de tecla** (11891): depois de um número de candidato desconhecido, `CWait(2000)` + `CWait::Espera`
   (5446: laço de `gettimeofday` + `usleep(200)`; o `nanosleep` do Emscripten (6265) gira sobre `emscripten_get_now`) roda dentro da
   chamada wasm que processa a tecla (`votaPressKey`/`votaTick`). O laço só roda enquanto o byte @1584624 vale 1. Esse byte é
   inicializado com 1 no segmento de dados, nenhuma instrução grava nele pelo seu endereço constante, e ele também é lido por
   `CApplication::EnterLoop*`, então presumivelmente é a flag de aplicação em execução. A thread do navegador, portanto, congela por 2 s.
   Não confirmado em tempo de execução: que a caixa "Candidato não encontrado!" é substituída antes que o navegador consiga exibi-la (o que
   significaria que o usuário nunca a vê). Apenas no build web; este é o menu pré-eleição "Mais informações → Visualizar candidatos".
9. **Global aparentemente só de escrita** @1838488. Ela é posta em 1 depois do registro dos DAOs, e `i32.const 1838488` não aparece em nenhum outro lugar do módulo.
10. O pré-carregamento no cache de 70 falas de TTS na inicialização (passo 10) é **ineficaz no build web**: ele roda contra
    `CWasmNullTextToSpeech` (o RHVoice é registrado por `votaInit` só depois da 7787), então não custa nada e põe no cache apenas
    WAVs nulos em um objeto que depois é substituído. Com o áudio ligado, o motor RHVoice sintetiza os nomes das teclas e a
    instrução sob demanda, mais tarde. (Uma versão anterior desta lista afirmava 70 sínteses síncronas na inicialização; o
    código e os tempos de `votaInit` de 76 ms / 74 ms com/sem `--audio` a contradizem.)

## 12. Tabela de mapeamento completa (todas as 160 funções)

`run` = visto pelo profiler por amostragem do V8 durante os votos gravados. "reconstrução" é o arquivo sob `src/` que contém
o código, ou "helper de biblioteca/inlinado" para instanciações da libc++ que estão apenas listadas. "arquivo original" é o arquivo-fonte
da função (`(unknown)` quando nenhuma evidência nomeia um; veja os headers dos fragmentos para os caminhos inferidos).

| idx | tamanho | run | símbolo reconstruído | arquivo original | reconstrução | conf. |
|---:|---:|:-:|---|---|---|:-:|
| 202 | 347 | * | `api::CFormBuilder::AddText` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 401 | 85 | * | `comum::CLocal::GetInst` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | média |
| 436 | 18 |  | `comum::CPath::GetPathTrab` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u02.cpp | média |
| 554 | 418 | * | `api::CriaFormInterativo` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 576 | 144 | * | `vota::CriaFormInterativoVota` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 820 | 143 |  | `comum::CLocal::UrnaBiometrica` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | alta |
| 886 | 21 | * | `vota::CriaFormVota` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 901 | 279 | * | `api::CInteractiveFormBuilder::AddControlInput` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 921 | 584 | * | `api::ITextToSpeech::CacheMessage` | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | média |
| 1004 | 122 |  | `comum::CLocal::GetMunicipio` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | alta |
| 1077 | 122 |  | `comum::CLocal::GetNomeMunicipio` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | alta |
| 1102 | 1002 | * | `vota::(anonymous namespace)::adicionaInstrucoesConfirmaCorrige` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 1159 | 45 |  | `std::__tree<map<K, map<K2, vector<X>>>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1160 | 147 |  | `std::__tree<map<TEleicaoID, vector<comum::md::CCargo>>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1190 | 305 | * | `vota::(anonymous namespace)::adicionaRodapeConfiraSeuVoto` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 1268 | 135 |  | `std::__tree<map<K, vector<X16>>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1314 | 84 |  | `std::__tree<map<K, shared_ptr<T>>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1507 | 53 |  | `std::__sort3<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1591 | 658 | * | `vota::CTelasVota::CriaTelaVotoNuloCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 1594 | 32 |  | `std::__tree<set<T>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1701 | 15 | * | `comum::ArquivoRdvExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | média |
| 1702 | 101 |  | `comum::CLocal::GetUF` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | alta |
| 1709 | 156 |  | `std::__tree<map<K, comum::md::CCandidatura>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1767 | 509 | * | `vota::CTelasVota::CriaTelaConferenciaVotoNuloCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 1835 | 72 |  | `std::__tree<map<TCargoID, comum::md::CCargo>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1929 | 82 | * | `std::__tree<map<uedword, SResposta>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1932 | 82 |  | `std::__tree<map<TNumeroPartido, CPartido>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 1934 | 107 |  | `std::__tree<map<std::string, CIndiceFoto>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2024 | 441 | * | `api::IInputField<api::IScreen>::IInputField` | uenux2/src/api/gui/iinputfield.h (caminho inferido) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 2243 | 68 | * | `api::CFormBuilder::AddLabeledKey` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | baixa |
| 2245 | 244 |  | `api::CFormBuilder::AddFill` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 2262 | 11 |  | `comum::CLocal::CLocal` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | baixa |
| 2267 | 1092 |  | `ecourna::app::dados::CParametrosUrna::~CParametrosUrna` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (caminho inferido) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp | média |
| 2285 | 534 | * | `comum::CMenuBase::AdicionaItem` | uenux2/src/app/comum/cmenubase.cpp (caminho inferido) | src/uenux2/src/app/comum/cmenubase.u02.cpp | média |
| 2291 | 3426 |  | `vota::CRetirarMR::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | média |
| 2376 | 935 | * | `vota::CTelasVota::CriaTelaTextoConfirmaCorrige` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 2380 | 566 | * | `vota::CTelasVota::CriaTelaNeutra` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 2383 | 297 | * | `api::CInteractiveFormBuilder::AddNumberInput` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 2384 | 784 | * | `vota::(anonymous namespace)::adicionaCampoNumero` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | baixa |
| 2543 | 104 |  | `comum::CIntegridadeReferencial::VerificaRdv` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | média |
| 2579 | 79 |  | `std::__sort3<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2685 | 17 | * | `api::FormataAAAAMMDDhhmmss` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u02.cpp | média |
| 2804 | 61 |  | `std::unique_ptr<__hash_node<string, list::iterator>, __hash_node_destructor>::reset` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2808 | 187 | * | `std::unordered_map<std::string, ...>::find` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2811 | 678 | * | `comum::CNomeArquivo::NomesPorAbrangencia` | uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u02.cpp | média |
| 2819 | 84 |  | `comum::CFotos::GetInst` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 2831 | 265 |  | `std::__uninitialized_allocator_move_if_noexcept<ecourna::app::dados::CFederacao>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2832 | 148 |  | `comum::CFederacoes::GetInst` | uenux2/src/app/comum/dados/cfederacoes.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | alta |
| 2834 | 918 | * | `comum::CCargos::GetMapaCargos` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | baixa |
| 2839 | 57 |  | `std::__tree<map<int, std::string>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 2870 | 323 |  | `std::vector<comum::md::CDadosCandidato>::vector(first, last)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3063 | 617 | * | `vota::CTelasVota::CriaTelaVotoBrancoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 3067 | 507 | * | `vota::CTelasVota::CriaTelaConferenciaVotoBrancoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 3678 | 564 | * | `api::CFormBuilder::AddLabeledKey` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 3680 | 200 |  | `api::CFormBuilder::AddRect` | uenux2/src/api/gui/cformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 3713 | 437 | * | `comum::(anonymous namespace)::CriaPleito` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 3722 | 61 |  | `std::unique_ptr<__hash_node<string, shared_ptr<CWavFile>>, __hash_node_destructor>::reset` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3729 | 123 | * | `std::hash<std::string>::operator()` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3732 | 767 |  | `std::unordered_map<std::string, list::iterator>::operator[]` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3745 | 175 | * | `comum::CEleitores::NomeArquivoEleitores` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | baixa |
| 3754 | 42 | * | `comum::md::CHorarioVerao::CHorarioVerao(const CHorarioVerao&)` | (unknown) | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 3772 | 1233 | * | `comum::CNomeArquivo::MontaNome` | uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u02.cpp | média |
| 3776 | 278 |  | `std::vector<std::unique_ptr<T>>::push_back(unique_ptr&&)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3777 | 2274 | * | `ecourna::app::dados::CParametrosUrna::CParametrosUrna(const CParametrosUrna&)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (caminho inferido) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.u02.cpp | média |
| 3778 | 361 |  | `std::set<uebyte>::insert(first, last)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3779 | 173 |  | `std::__split_buffer<ecourna::app::dados::CFederacao>::~__split_buffer` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3780 | 235 |  | `ecourna::app::dados::CFederacao::CFederacao(const CFederacao&)` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | baixa |
| 3781 | 108 |  | `std::__tree<map<K, CFederacao>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3785 | 85 |  | `std::__tree<map<K, {string, vector}>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 3942 | 123 | * | `comum::CopiaTransformada` | (unknown) | src/uenux2/src/app/comum/util/cstringutil.u02.cpp | baixa |
| 4137 | 45 |  | `std::__tree<map<TCargoID, CTelasCargo>>::destroy` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4160 | 3713 | * | `vota::(anonymous namespace)::adicionaCabecalhoZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | baixa |
| 4161 | 2019 | * | `vota::(anonymous namespace)::adicionaRodapeLocal` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 4433 | 22 |  | `vota::CIniciodeCiclo::GetInst` | uenux2/src/app/vota/eleitor/ciniciodeciclo.cpp | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | média |
| 4657 | 1048 | * | `comum::GravaBancoDadosNaMI` | (unknown) | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | baixa |
| 4662 | 38 | * | `vota::CSincronizaVota::SincronizaBancoDados` | uenux2/src/app/vota/comum/csincronizavota.cpp | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | baixa |
| 4811 | 268 |  | `std::__floyd_sift_down<uebyte*> (heap sort fallback)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4812 | 101 |  | `std::__swap_bitmap_pos<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4817 | 287 |  | `std::__insertion_sort_incomplete<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4818 | 151 |  | `std::__sort5<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4819 | 198 |  | `std::__sort4<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 4820 | 2432 | * | `std::__introsort<uebyte*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5501 | 58 |  | `api::CRectField::CRectField` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 5548 | 16 | * | `api::IForm<api::IScreen>::IForm` | uenux2/src/api/gui/iform.h | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 5550 | 296 | * | `api::CriaForm` | uenux2/src/api/gui/cinteractiveformbuilder.cpp | src/uenux2/src/api/gui/cformbuilder.u02.cpp | média |
| 5649 | 257 | * | `comum::(anonymous namespace)::EleicoesDoMunicipio` (filtra `CEleicaoPE::municipios`; antes "EleicoesDoTurno") | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 5654 | 243 |  | `comum::(anonymous namespace)::CodigosDosCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 5671 | 101 |  | `std::vector<std::unique_ptr<T>>::~vector` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5677 | 53 |  | `comum::md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio` | uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | média |
| 5679 | 30 |  | `comum::md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio` | uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | média |
| 5685 | 725 | * | `std::unordered_map<std::string, SharedWav>::__emplace_unique_key_args` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5701 | 106 |  | `std::list<std::pair<std::string, SharedWav>>::splice(pos, list, it)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5703 | 251 |  | `std::copy<SFotoRef>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5704 | 762 |  | `std::vector<SFotoRef>::assign(first, last)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5707 | 93 |  | `std::shared_ptr<CWavFile>::operator=` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5727 | 190 |  | `comum::CEleitores::NomeArquivoTTE` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | baixa |
| 5728 | 37 | * | `comum::(anonymous namespace)::SIdentificacaoCarga::SIdentificacaoCarga` | (unknown) | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 5730 | 37 |  | `comum::CRespostas::~CRespostas` | uenux2/src/app/comum/dados/crespostas.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 5734 | 247 |  | `comum::CRdvVota::~CRdvVota` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | média |
| 5735 | 1046 |  | `comum::CRdvVota::CriaMapaEleicoesCargos` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | baixa |
| 5743 | 119 |  | `comum::CLocal::GetLocalData` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp | alta |
| 5746 | 1021 | * | `comum::CIntegridadeReferencial::VerificaFotoCandidatura` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | média |
| 5747 | 1495 | * | `comum::CIntegridadeReferencial::VerificaCargoCandidatura` | uenux2/src/app/comum/dados/cintegridadereferencial.cpp | src/uenux2/src/app/comum/dados/cintegridadereferencial.u02.cpp | média |
| 5750 | 133 | * | `api::ITextToSpeech::Armazena` | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | média |
| 5754 | 169 | * | `api::ITextToSpeech::Busca` | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | média |
| 5758 | 839 | * | `api::ITextToSpeech::MontaChave` | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | baixa |
| 5762 | 15 |  | `comum::ArquivoBancoExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | média |
| 5773 | 394 |  | `comum::CEleitores::~CEleitores` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 5787 | 682 |  | `std::insert_iterator<std::set<uebyte>>::operator=` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5788 | 482 |  | `std::__lower_bound_onesided<set<uebyte>::const_iterator>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5790 | 3572 | * | `comum::(anonymous namespace)::TodosOsCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | baixa |
| 5791 | 87 |  | `comum::IdsEleicoes` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | baixa |
| 5792 | 294 |  | `comum::CConfiguracaoEleicao::~CConfiguracaoEleicao` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u02.cpp | média |
| 5796 | 373 |  | `std::vector<ecourna::app::dados::CFederacao>::__construct_at_end(first, n)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5797 | 348 |  | `std::move_backward<ecourna::app::dados::CFederacao*>` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5800 | 60 |  | `comum::CFederacoes::~CFederacoes` | uenux2/src/app/comum/dados/cfederacoes.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 5805 | 60 |  | `comum::CCargos::~CCargos` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 5808 | 501 | * | `std::map<K, comum::md::CCandidatura>::insert(first, last)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 5913 | 1334 | * | `comum::CMenuBase::Monta` | uenux2/src/app/comum/cmenubase.cpp (caminho inferido) | src/uenux2/src/app/comum/cmenubase.u02.cpp | média |
| 5936 | 1802 |  | `vota::testeteclado::CEsperaRetestar::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cesperaretestar.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | média |
| 5953 | 527 |  | `vota::CMenuFiltrarCandidatosBase::CarregaCandidaturas` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | baixa |
| 6035 | 250 |  | `comum::ArquivoTrabExterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | baixa |
| 6036 | 250 |  | `comum::ArquivoTrabInterno` | (unknown) | src/uenux2/src/app/comum/cpath.u02.cpp | baixa |
| 6117 | 136 | * | `vota::CriaForm<PRESHOW>` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | baixa |
| 6155 | 1623 | * | `api::FormataDataHora` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/api/util/cdatetime.u02.cpp | baixa |
| 6548 | 1530 |  | `vota::CTelasVota::~CTelasVota` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6557 | 166 |  | `vota::CTelasVota::CriaTelaContinuacaoVotacao` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6592 | 942 | * | `vota::CTelasVota::CriaTelaConfirmaImpressaoZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6594 | 332 |  | `std::vector<std::pair<char, std::string>>::__emplace_back_slow_path` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 6595 | 1740 | * | `vota::CTelasVota::CriaTelaAntesHorarioZeresima` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6599 | 4974 | * | `vota::CTelasVota::CriaTelaMaisInformacoes` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6601 | 504 | * | `vota::CTelasVota::CriaTelaFim` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |
| 6616 | 490 |  | `vota::CTelasVota::CriaTelaConferenciaVotoNuloConsulta` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 6626 | 600 |  | `vota::CTelasVota::CriaTelaVotoNuloConsulta` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 6639 | 622 | * | `vota::CTelasVota::CriaTelaCompletaCandidatoCom0` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 6681 | 476 |  | `vota::CTelasVota::CriaTelaConferenciaCandidatoCom0` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | alta |
| 6689 | 281 |  | `api::CFormBuilder::AddDataImage` | (unknown) | src/uenux2/src/api/gui/cformbuilder.u02.cpp | baixa |
| 7481 | 742 | * | `vota::CAguardaMensagem::ProcessMessage` | uenux2/src/app/vota/eleitor/caguardamensagem.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | média |
| 7787 | 148912 | * | `vota::CInformacaoEleitor::Inicializar` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | baixa |
| 10699 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoSemBiometria` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | média |
| 10700 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoBiografica` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | média |
| 10701 | 871 |  | `vota::(anonymous namespace)::TextoHabilitacaoBiometrica` | uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp | src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.u02.cpp | média |
| 10786 | 5 |  | `vota::CControladorRegistraMesariosVota::SincronizaBancoDados` | (unknown) | src/uenux2/src/app/vota/comum/csincronizavota.u02.cpp | baixa |
| 11223 | 2163 |  | `comum::CGeradorRelVersaoPacoteDados::MontaCorpo` | (unknown) | src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.u02.cpp | baixa |
| 11353 | 415 |  | `std::unordered_map<std::string, list::iterator>::erase(const key&)` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 11436 | 471 |  | `api::CLruCache<std::string, SharedWav>::Insere` | uenux2/src/api/audio/itexttospeech.cpp (caminho inferido) | src/uenux2/src/api/audio/itexttospeech.u02.cpp | média |
| 11447 | 52 | * | `std::unordered_map<std::string, SharedWav>::operator[]` | (instância da libc++) | helper de biblioteca/inlinado | média |
| 11481 | 37 |  | `comum::CRespostas static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/crespostas.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 11484 | 132 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::~CConversorRegistroDigitalVoto` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | alta |
| 11485 | 129 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::~CConversorRegistroDigitalVoto` | (unknown) | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | alta |
| 11495 | 37 |  | `comum::CRdvVota static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/crdvvota.cpp | src/uenux2/src/app/comum/dados/crdvvota.u02.cpp | média |
| 11497 | 103 |  | `ecourna::api::security::CHKDFSeed::~CHKDFSeed` | ecourna-lib/ecourna/api/security/chkdfseed.cpp | src/ecourna/api/security/chkdfseed.cpp | alta |
| 11498 | 100 |  | `ecourna::api::security::CHKDFSeed::~CHKDFSeed` | ecourna-lib/ecourna/api/security/chkdfseed.cpp | src/ecourna/api/security/chkdfseed.cpp | alta |
| 11535 | 37 |  | `comum::CEleitores static unique_ptr reset (atexit)` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | média |
| 11797 | 593 |  | `vota::CInspecionaUrna::ProcessInput` | uenux2/src/app/vota/eleitor/cinspecionaurna.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/cestadosvota.u02.cpp | média |
| 11809 | 1111 |  | `vota::testeteclado::CTesteFalhou::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctestefalhou.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | média |
| 11812 | 1108 |  | `vota::testeteclado::CEnviarManutencao::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cenviarmanutencao.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.u02.cpp | média |
| 11891 | 999 |  | `vota::CMenuFiltrarCandidatosPorNumero::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | média |
| 12244 | 248 | * | `vota::CItemVisualizarCandidatosVota::Disponivel` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/citemvisualizarcandidatosvota.cpp (caminho inferido) | src/uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatos.u02.cpp | baixa |
| 12882 | 37 |  | `vota::CTelasVota static unique_ptr reset (atexit)` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp | média |

## 13. Questões em aberto

* O nome real da 7787: nada no binário a nomeia. `CInformacaoEleitor::Inicializar` se encaixa na evidência
  (as flags, o único srcloc de `cinformacaoeleitor.cpp` e a ordem de chamadas em `votaInit`), mas ela também poderia ser uma função
  livre da entrada web `vota_web_wasm.cpp` que chama os métodos inlinados de `CInformacaoEleitor`.
* O significado dos slots 4/5/6 de `IUrna` (tabelas de 1024 bytes, 32 bytes e 128 bytes) na urna real (segredos do TPM / do
  hardware?). O slot 6 é a tabela do RDV aqui. O índice de 10 bits montado em `GetCifradorCryptoTable` indica que o
  projeto um dia usou a tabela de 1024 bytes.
* Os valores numéricos de `api::EInputResult` (5 = CORRIGE, 9 = CONFIRMA) são inferidos a partir dos rótulos das teclas das
  telas de teste do teclado.
* A global @1577208 é `CEstadoGeral` +48, a fase (respondido: u06/u20 e os usos em 5728/2785). O nome do seu membro
  em C++ ainda é desconhecido.
* A 6689 sempre usa o id de imagem de dados 1086. Ela é um método de `CFormBuilder` ou um helper de `ctelasvota.cpp`?
* (respondido) O segundo formato da 4161 (seis argumentos) é usado para uma urna de contingência (`md::CLocal` +136 ligado, sem seção);
  o seu sexto argumento é a constante "CONTINGÊNCIA".

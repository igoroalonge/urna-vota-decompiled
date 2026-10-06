# u05 — `uenux2/src/app/comum/dados` (parte 3): RDV, identidade do eleitor, objetos de domínio, `eleitor_dinamico`, tradução de rótulos

A unidade u05 tem 100 funções wasm. Elas vêm de 21 arquivos originais da camada de dados compartilhada
`comum`, mais funções avulsas que o gerador de unidades anexou a ela (instanciações de template, corpos
mesclados pelo wasm-opt e algumas funções de arquivos vizinhos). 14 delas executaram durante os votos
gravados: 566, 654, 753, 946, 3509, 5157, 5158, 5660, 5770, 5771, 5772, 11272, 11378 e 11488. Todas
executam durante `votaInit` (carregamento dos dados da eleição e criação de `rdv.dat`) ou enquanto textos
de tela são traduzidos.

Arquivos-fonte reconstruídos (todos em `src/`):

| arquivo | conteúdo |
|---|---|
| `uenux2/src/app/comum/dados/crdvvota.{h,cpp}` | esboço da interface `comum::CRdv`, `comum::CRdvVota` e os corpos inlinados de `CVotosEleicoesVota` |
| `uenux2/src/app/comum/dados/crespostas.{h,cpp}` | `comum::CRespostas::GetInst` |
| `uenux2/src/app/comum/dados/dao/celeitordinamicodao.{h,cpp}` | `comum::dao::CEleitorDinamicoDAO` (tabela SQLite `eleitor_dinamico`) |
| `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.{h,cpp}` | `CCandidatura` |
| `uenux2/src/app/comum/dados/md/{clocal,cinfomunicipio,chorarioveraomunicipio}.{h,cpp}` | objetos de local e de município |
| `uenux2/src/app/comum/dados/md/correspondencia/{ccarga,ccorrespondenciaresultado}.{h,cpp}` | correspondência do arquivo de resultado |
| `uenux2/src/app/comum/dados/md/{cvalidadoridentidade,iregraidentidade}.{h,cpp}`, `cregracpf.cpp`*, `cregratitulo.cpp`* | validação de identificadores |
| `uenux2/src/app/comum/dados/md/eleitor/{celeitor,celeitordecorator,celeitordinamico,cbiometriaeleitor}.{h,cpp}`, `celeitoridentidade.{h,cpp}`* | registros de eleitores |
| `uenux2/src/app/comum/dados/md/estadoaplicacao/{cajustedatahora,cdadocarga,cestadogeral,clocalidadeeleitoral}.{h,cpp}` | objetos de estado da aplicação |
| `uenux2/src/app/comum/dados/md/municipiozona/{ccomplementomunicipio,cmunicipio}.{h,cpp}` | município |
| `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.{h,cpp}` | tradutor de placeholders |
| `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp`* | foto do eleitor no display do mesário (func 3616 e helpers) |
| `uenux2/src/api/util/cajustedatahora.cpp`* | `api::CAjusteDataHora` (uma classe diferente que tem o mesmo nome) |
| `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` | funções desta unidade que pertencem a arquivos de outras unidades (§12) e as instanciações de biblioteca |

`*` = caminho inferido (nenhum `std::source_location` cita este arquivo). A unidade também tem funções cujo
arquivo original pertence a outra unidade (u04, u20, u21, u22, u25, u27, u35). Elas **não** são escritas nos
arquivos dessas unidades. Seguindo a convenção de fragmentos usada por u04, elas estão em
`src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp`, uma seção por arquivo original, e são
resumidas na seção 12.

## 1. O que esta parte do programa faz (com os termos em português)

`comum/dados` contém o **modelo de domínio** ("md" = *modelo de domínio*) que a aplicação VOTA carrega das
mídias da eleição, mais alguns serviços construídos sobre ele:

* **RDV, *Registro Digital do Voto***: a lista de votos por eleição e por cargo. `comum::CRdvVota` é o RDV
  em memória que os relatórios BU e zerésima leem, e ele é serializado em `rdv.dat`.
* **Validação de identificadores**: *título de eleitor* (o número de 12 dígitos do título, com um código de
  UF e dois dígitos verificadores) e **CPF** (*Cadastro de Pessoas Físicas*, o número de 11 dígitos). Toda
  identidade de eleitor ou de mesário construída em qualquer ponto do programa passa pelo construtor de
  `CEleitorIdentidade`, que a valida.
* **Registros de eleitores**: `CEleitor` é a entrada estática do cadastro (de `*-el.dat`).
  `CEleitorDecorator` é um eleitor mais o seu tipo de identificador "principal". `CEleitorDinamico` é o estado
  que muda no dia da eleição (*habilitação* do eleitor para votar, *comparecimento*, tentativas biométricas,
  acessibilidade por áudio, foto apresentada). `CEleitorDinamicoDAO` o armazena em SQLite.
* **Biometria** (`CBiometriaEleitor`: foto + até dez impressões digitais) e a **apresentação da foto do
  eleitor** no display do terminal do mesário (*MT*, microterminal; func 3616).
* **Objetos de valor validados** carregados dos arquivos de dados ASN.1: *candidatura* (com *suplentes*,
  isto é, substitutos/vice), *município*, *fuso horário* e *horário de verão*, *local* (país, UF, *seção*,
  *urna de contingência*), identificação da carga (*carga*, *correspondência de resultado*), *dado de carga*
  (*turno*, modelo, *fase*) e *ajuste de data/hora*.
* **`CTradutorFrase`** (tradutor de frases) expande os placeholders de todos os textos de telas e relatórios.
  Ele usa os rótulos e o gênero gramatical do arquivo de parametrização da urna (`*-pu.dat`).

## 2. Classes e hierarquia (RTTI)

```
comum::CRdv                     (abstract, 14 pure virtuals, no virtual destructor)   vtable @1559900
 └─ comum::CRdvVota             vtable @1560172, singleton @1838956, sizeof 100
      has: comum::md::CVotosEleicoesVota m_votos (+20)
           comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota> (+44)
             : comum::asn::IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto,
                                        comum::md::CVotosEleicoesVota>
      uses: comum::CRdvPosicionadorVota : comum::md::CRdvPosicionador  (vtable @1560252)

api::persistencia::IDAO
 └─ api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>
     └─ comum::dao::IEleitorDinamicoDAO
         └─ comum::dao::CEleitorDinamicoDAO   vtable @1558904
            (siblings with the same slot layout: CJustificadorDAO, CComparecimentoMesarioDAO)

comum::md::IRegraIdentidade
 ├─ comum::md::CRegraTitulo            vtable @1574700
 ├─ comum::md::CRegraCPF               vtable @1574740
 └─ comum::md::CRegraIdentidadeLivre   vtable @1574652 (unit u35)
comum::md::CValidadorIdentidade        singleton @1839028, vector<unique_ptr<IRegraIdentidade>>

api::IAjusteDataHora
 └─ api::CAjusteDataHora               vtable @1584164  (calls api::ISystemDateTime; in the web build
                                        simulador::CWasmSystemDateTime)
```

As demais classes da unidade não têm vtable e são tipos de valor simples: `CEleitorIdentidade` (16 B),
`CEleitor` (104 B), `CEleitorDecorator` (108 B), `CEleitorDinamico` (84 B), `CBiometriaEleitor`,
`CCandidatura` (72 B), `CDadosCandidato` (52 B), `CCarga` (76 B), `CCorrespondenciaResultado` (88 B),
`CLocal`, `CMunicipio`, `CInfoMunicipio`, `CComplementoMunicipio`, `CHorarioVeraoMunicipio`, `CDadoCarga`
(20 B), `CAjusteDataHora` (8 B), `CLocalidadeEleitoral` (8 B), `CTradutorFrase` (todo estático).

Os erros que elas lançam são `ecourna::api::exception::CBaseError<E, SErrorLimits{lo, hi}>`:
`comum::EUeComumDadosError` {7800..8600} (typeinfo @1528076, lançado via `comum_f170`),
`api::EUeRdvError` {4650..4850} (@1559972, via func 655), `comum::EUeComumAsnError`,
`api::EUeIoError` e `EPatternError` para os singletons. Todo throw passa um
`std::source_location`, então o código reconstruído traz os números de linha originais.

## 3. O RDV (`CRdvVota`)

### 3.1 Layout e criação

`CRdvVota::CreateInst` (crdvvota.cpp:92) e os construtores de `CRdvVota` e `CRdv` (crdv.cpp) estão
totalmente inlinados na função de inicialização 7787 (`vota::CInformacaoEleitor::Inicializar`, nome inferido
em u02; antes exibida pelas ferramentas como `CHKDFSeed::GetSeed`). O código de inicialização faz o seguinte:

1. `new CRdvPosicionadorVota` (o "posicionador" de votos, isto é, a política que decide onde cada voto fica
   no RDV para que a ordem de votação não possa ser recuperada).
2. `CRdv::CRdv(posicionador, cargos, CConfiguracaoEleicao +164 = digitosPartido)`. `cargos` é a lista
   ordenada dos códigos de cargo de todas as eleições. A unidade u02 reconstruiu isto em
   `src/uenux2/src/app/comum/dados/crdv.cpp` e `crdvvota.u02.cpp`:
   * `digitosPartido == 0` → `EUeRdvError 4650 "Número de dígitos do partido nulo"` (crdv.cpp:90);
     `> 5` → 4651 `"Número de dígitos do partido ({}) supera o limite (5)"` (crdv.cpp:93).
   * `(anonymous)::GetCifradorCryptoTable(cargos)` (crdv.cpp:60/78) deriva a cifra simétrica que
     `api::CEncryptedFile` usa para `rdv.dat`. As entradas são o SHA-512 de `cargos`, 32 bytes escolhidos
     da tabela criptográfica de 128 bytes de `IUrna` (toda `0x03` no `CUrnaMock` do simulador) e
     `HKDF-SHA512` com info `"RDV"`.
3. Os membros de `CRdvVota` são inicializados. `m_votos` é construído a partir da configuração da eleição:
   um `CVotosCargos` por eleição, um `(SCargoInfo, CVotos)` por cargo, onde `SCargoInfo` é tirado das
   entradas `CCargo` de 140 bytes (campos +4, +12 e +13 = `qtdEscolhas`, o número de escolhas).
   `CVotosCargos` lança `4664 "Vetor de cargos vazio"` e `4665 "Cargo duplicado "`. `m_conversor` é
   carregado com os campos de identificação (município, zona, seção, eleições) que vão no cabeçalho do RDV.
4. Se uma instância já existe, lança `4658 "Instancia ja criada"`.

Layout: `+0 vptr`, `+4 posicionador`, `+8 cipher (shared)`, `+16 uebyte digitosPartido`,
`+20 CVotosEleicoesVota {map<TEleicaoID, CVotosCargos> +20; map<TCargoID, TEleicaoID> +32}`,
`+44 converter` (até +100).

### 3.2 Interface (slot da vtable → método)

| slot | func | método | o que retorna |
|---|---|---|---|
| 0 | 5733 | `GetVotos()` | `map<TCargoID, CVotos>` sobre todas as eleições (a primeira eleição prevalece se um cargo se repete) |
| 1 | 11490 | `GetVotos(TEleicaoID)` | o mesmo para uma eleição (`"Eleicao (N) nao encontrada"` 4733) |
| 2 | 11492 | `Comparecimento(eleição)` | votos do primeiro cargo ÷ `qtdEscolhas` = eleitores que votaram |
| 3 | 1931 | `Candidato(cargo, número, dígitos)` | votos de um candidato (número preenchido à esquerda com `'0'`) |
| 4 | 3749 | `Legenda(cargo, partido)` | votos de legenda (só no partido) |
| 5 | 2814 | `Partido(cargo, partido)` | todos os votos do partido (legenda + seus candidatos) |
| 6 | 2813 | `Nominais(cargo)` | votos em candidatos |
| 7 | 5732 | `Legendas(cargo)` | todos os votos de legenda |
| 8 | 3748 | `Nulos(cargo)` | votos nulos |
| 9 | 3747 | `Brancos(cargo)` | votos brancos |
| 10 | 1930 | `Cargo(cargo)` | total do cargo ("Total Apurado") |
| 11 | 11488 | `Converte()` | bytes BER de `ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto` |
| 12 | 11487 | `Desconverte(bytes)` | carrega os bytes; `4660 "Conteudo do arquivo eh incompatível com objeto"` se a estrutura difere |
| 13 | 11486 | `ConfereConteudo(bytes)` | true se os bytes decodificam exatamente para o RDV em memória |

Os slots 3–10 são funções de uma linha que repassam para `m_votos`. O LTO inlinou os corpos de
`CVotosEleicoesVota`. O wasm-opt então os mesclou nos corpos compartilhados 2296 (cinco consultas só com
`TCargoID`), 6033 (Legenda/Partido) e 3707 (busca `cargo → eleição`, `"Cargo N nao encontrado"`, códigos
4725..4732 por consulta). Os srclocs dentro deles apontam para `cvotoseleicoesvota.cpp:113..190`.

### 3.3 Ciclo de vida de `rdv.dat`

* **Criação (`votaInit`)**: `CGeraDadosDinamicos` (6737) chama `Converte()` e grava o resultado via
  `api::CEncryptedFile::MemWrite` + `Flush` em `rdv.dat` nas duas áreas de flash (`/dsk/fi/dinamico/trab1`
  e `/dsk/fe/...`; funcs 5736/5737). Esse é o arquivo cifrado de 80 bytes em
  `analysis/runtime/memfs-after-init` (um RDV vazio). É por isso que 11488 aparece como "observada".
* **Carga**: `CEleitores::CompleteLoad` (6734) faz `CEncryptedFile::Load` + `MemRead` → `Desconverte`.
  Em seguida roda `CIntegridadeReferencial` sobre `CRdvVota`, `CCargos` e `CRespostas` (comum_f2543).
* **Sincronização nativa do voto** (`vota::impl::CSincronismoVotoEleitor::vf2`, 7174, não usada no build
  web) grava `rdv.dat.tmp` = cifrar(`Converte()`). Ela relê o arquivo, o decifra e chama
  `ConfereConteudo`. Só se isso retornar true ela chama `PrepareReplace` (renomeia sobre `rdv.dat`) e
  atualiza a linha `eleitor_dinamico` do eleitor (`CEleitorDinamicoDAO::Atualizar`). Essa verificação por
  releitura é a verificação de integridade de gravação do RDV.
* **A inclusão de votos** não está nesta unidade. `vota::CEleitorVotando::NeedChangeState` (7352) chama a
  func 4454, que inlina `CVotosEleicoesVota::RecebeCedula` (cvotoseleicoesvota.cpp:215) →
  `CVotosCargos::InsereCedula` (o posicionador escolhe o slot) → `CVotos::Insere`, seguido de
  `CVotosCargos::ConfereCedula` (relê a cédula do RDV e compara). O profiler viu 4454 executar nos dois
  votos gravados, então **o RDV em memória de fato recebe cada cédula no simulador**.
* **Build web**: `main` registra `(anonymous)::CSincronismoVotoEleitorWeb`. O seu slot 2 é o corpo ICF
  `return 1`: a sincronização "tem sucesso" sem gravar nada. Assim `rdv.dat` fica com o conteúdo de
  `votaInit` (os votos gravados alteram apenas `logd.dat`, ver `analysis/runtime/README.md`),
  `ConfereConteudo` nunca é exercitada e a tabela `eleitor_dinamico` nunca é criada.
  Segundo a análise de u04, `CGeraDadosDinamicos` (6737) pula `CEleitores::DynamicCreate` (5761)
  no modo de *treinamento do eleitor*, que é o modo de todos os cenários do simulador. O
  construtor do DAO tem dois outros chamadores: `CEleitores::CompleteLoad` (6734) e o
  `CSincronismoVotoEleitor::vf2` nativo (7174). Nenhum dos dois aparece nos profiles, e como o construtor
  sempre executa `CREATE TABLE IF NOT EXISTS`, a ausência da tabela mostra que nenhum dos três executou.

## 4. Identificadores do eleitor: `CEleitorIdentidade`, `CValidadorIdentidade`, regras

`CEleitorIdentidade(std::string, ETipoIdentificadorEleitor)` é a func 566. As ferramentas a nomearam
`CValidadorIdentidade::Valida` porque essa função está inlinada nela. Ela executa para toda identidade
construída a partir do cadastro de eleitores (`CVisitanteEleitor::visit`, `CConversorEntidadeEleitores`,
`CConversorImpedido::MontaEleitorIdentidade`), a partir dos DAOs (linhas SQL de `eleitor_dinamico` e
`comparecimento_mesario`) e a partir dos fluxos do operador (`CMostraEleitorVotando::SalvaHabilitacaoEleitor`,
as funções `GetControlador` do mesário, `vota::TipoToStr`, ...):

1. **Normalizar** (`Formata`, func 2798). A largura é 11 para CPF (tipo 2) e 12 nos demais casos. Se a
   string for mais longa, remove os seus zeros à esquerda. Depois a preenche à esquerda com `'0'` até a largura.
2. **Escolher uma regra** (`CValidadorIdentidade::Valida`, cvalidadoridentidade.cpp:61). Pega a primeira
   regra cujo `GetTipo()` é `3` (identificador livre) **ou** o tipo pedido. Se nenhuma corresponde:
   `8108 "Tipo de identidade inválido: {}"`.
3. **Validar** (`IRegraIdentidade::Valida`, iregraidentidade.cpp:23). A string deve ser não vazia e conter
   só dígitos (`find_first_not_of("0123456789")`). O `Verifica` da regra (slot 4) deve aceitar a string
   sem os seus zeros à esquerda (func 5159). Caso contrário: `8109 "Identidade inválida: " + id`.
   A func 3723 é o mesmo teste sem o throw (`EhValida`).
4. Normalizar de novo.

As regras são construídas na inicialização (inlinadas em 7787) a partir dos tipos de identificador que
`CConfiguracaoEleicao` habilita: `1 → CRegraTitulo`, `2 → CRegraCPF`, `3 → CRegraIdentidadeLivre`, outros
valores → `7825`.

**Verificação do título de eleitor** (`CRegraTitulo::Verifica`, 11272). A string tem no máximo 12 dígitos,
preenchida até 12, dígitos `d0..d11`:
* `uf = d8*10 + d9` deve estar em 1..28 (compilado como `(uf-29) <u -28`).
* `r1 = (9*d0 + 8*d1 + 7*d2 + 6*d3 + 5*d4 + 4*d5 + 3*d6 + 2*d7) mod 11`,
  `dv1 = r1 < 2 ? (uf < 3 ? r1 ^ 1 : 0) : 11 - r1`, e `d10 == dv1`.
* `r2 = (4*d8 + 3*d9 + 2*d10) mod 11`, o mesmo mapeamento, e `d11 == dv2`.

Os pesos são o complemento mod 11 dos pesos publicados 2..9 / 7,8,9, o que dá os mesmos dígitos. O ramo
`uf < 3` é o caso especial publicado para SP (01) e MG (02): resto 0 dá o dígito 1.

**Verificação do CPF** (`CRegraCPF::Verifica`, 11270). No máximo 11 dígitos, preenchido até 11. É a
verificação padrão de dois dígitos mod 11 (pesos 10..2, depois 11..3 mais `2*dv1`; resto < 2 dá 0). CPFs
com dígitos repetidos (`000.000.000-00`, `111.111.111-11`, ...) são aceitos.

`CRegraX::Formata` (slot 3, 11269/11271) é `PadLeft(s, '0', 11 | 12)` (func 753).

## 5. Registros de eleitores e a tabela `eleitor_dinamico`

* `CEleitor::ValidaCriacao` (5667): `Trim(nome)` deve ser não vazio (`8064 "Nome vazio"`, linha 103);
  `necessidadeEspecial < 2` (`8065`, linha 107); se `nomeSocial` estiver definido, `Trim(nomeSocial)` deve
  ser não vazio (`8066`, linha 113). As funcs 946/337 são o construtor de cópia e a atribuição por
  movimento membro a membro (vetor de identidades, quatro strings, um vetor de bytes, flags).
* `CEleitorDecorator` = `CEleitor` + tipo de identificador principal (+104). `GetIdentidadePorTipo` (708)
  retorna a identidade do eleitor desse tipo ou lança `7833 "Tipo de identificador inválido: {}"`.
  `operator<=>` (456) compara as identidades principais: primeiro a string de dígitos, depois o tipo.
  `CEleitores::GetEleitoresEstaticos` (5772, arquivo da unidade u04) carrega os arquivos do cadastro
  (`*-el.dat` e `*-tte.dat`, os eleitores em transferência temporária) por meio de uma `std::function`,
  decora cada eleitor e ordena o vetor com `std::sort` usando essa ordenação (instanciação de introsort
  945/3761/5755/5756/5769). O cadastro fica, portanto, ordenado pela identidade principal, presumivelmente
  para a busca rápida de um título/CPF digitado.
* `CEleitorDinamico` (84 B): título, a identidade usada para habilitar o eleitor, `estado_comparecimento`,
  `tipo_habilitacao`, `dedo_habilitacao` (dedo), `score_habilitacao` (u16), `numero_tentativa` (u8),
  `tipo_ativacao_audio`, `erro_decifrar_biometria`, um título opcional do mesário que habilitou
  o eleitor manualmente (`GetTituloMesarioHabilitacao`, 1548: `8067 "Eleitor não habilitado
  manualmente"`), e o par de apresentação da foto (estado 0..2, resultado 0..8).

`CEleitorDinamicoDAO` (SQLite, via `ecourna::api::sql`):

| slot | func | método | SQL |
|---|---|---|---|
| ctor | 3768 | `CEleitorDinamicoDAO(path)` | `CREATE TABLE IF NOT EXISTS eleitor_dinamico ( titulo BIGINT PRIMARY KEY UNIQUE NOT NULL, tipo_identificador …, titulo_mesario BIGINT, tipo_identificador_mesario INTEGER CHECK(tipo_identificador IN (1,2,3) ) )` |
| 2 | 11541 | `Clone()` | cópia do DAO que compartilha a conexão |
| 3 | 5776 | `Inserir(e)` | `INSERT INTO eleitor_dinamico (15 columns) VALUES (?×15)`; títulos vinculados como `BIGINT` via `CStringUtils::ToQWord`; colunas do mesário `NULL` quando ausentes |
| 4 | (11540) | `Excluir` | padrão da base: lança `"Excluir não implementada para entidade …"` |
| 5 | 11543 | `ExcluirID` | lança `7983 "Exclusão de eleitor dinâmico"` (linha 130) |
| 6 | 5775 | `Atualizar(e)` | `UPDATE eleitor_dinamico SET identidade_habilitacao = ?, … WHERE titulo = ?` |
| 7 | 11542 | `Recuperar(titulo)` | `SELECT … from eleitor_dinamico WHERE titulo = ?` → `shared_ptr(new CEleitorDinamico)` ou null. Peculiaridade: quando `titulo_mesario` é NULL, a identidade do eleitor é construída a partir do **parâmetro** `titulo`, não da coluna 0 (que é formatada e depois descartada); com mesário, ela usa a coluna 0 |
| 8 | 3766 | `RecuperarTodos()` | `SELECT … from eleitor_dinamico` → `vector<CEleitorDinamico>` |

As linhas voltam a ser objetos via `CEleitorIdentidade` (portanto são revalidadas) e os dois
construtores 5663/5664. A DDL tem **duas** restrições `CHECK` na coluna errada
(`resultado_decifracao_foto` verifica `estado_apresentacao_foto`, e `tipo_identificador_mesario` verifica
`tipo_identificador`). A tabela não guarda nenhum voto: comparecimento e cédulas são mantidos separados.

## 6. Biometria e a foto do eleitor

`CBiometriaEleitor::CriaComum` (2799): no máximo 10 dedos (`8056 "Mais de dez dedos na biometria do
eleitor: {}"`); duplicados lançam `8057 "Dedo já existente ou duplicado"`. `GetDedo` (3720):
`8058 "Dedo não encontrado."`. `GetFoto` (linha 115, `8059 "Não há informação de foto."`) existe apenas
inlinada na func 3616.

A func 3616 (`vota::ApresentaFotoEleitor`, nome e arquivo inferidos) executa no `StartState` de
`CNomeEleitor`, `IConfirmaJustificativa` e `IEleitorImpedidoVotar` (10625). Ela mostra a foto do eleitor
identificado no LCD do mesário e registra o resultado em `IInformacaoThreadOperador`
(slots 27/28/29 = sem foto / apresentada / erro). Esse resultado depois se torna
`eleitor_dinamico.estado_apresentacao_foto/resultado_decifracao_foto` e
`ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor`:

| condição | imagem no LCD | código de resultado (`ResultadoApresentacaoFotoEleitor`) | log |
|---|---|---|---|
| eleitor sem biometria (+100) | `semBiometria.jpg` | estado "sem foto" | – |
| estado de decifração 9 / outro > 0 | `fotoIndisponivel.jpg` | 7 erroDecifrandoBuffer / 8 erroDesconhecidoDeCriptografia | – |
| sem foto no pacote | `fotoIndisponivel.jpg` | estado "sem foto" | – |
| nenhum marcador SOF encontrado | `fotoIndisponivel.jpg` | 1 erroFormato | "Erro ao carregar a foto do eleitor" |
| largura ∉ 120..240 ou altura ∉ 160..320 | `fotoIndisponivel.jpg` | 3 erroResolucao | "Largura/Altura da foto do eleitor inválida: {} px" |
| 8 × componentes ≠ 8 (não é escala de cinza) | `fotoIndisponivel.jpg` | 5 erroProfundidadeCores | "Quantidade de cores não conforme: {} bits por pixel" |
| caso contrário | o próprio JPEG | estado "apresentada" | "Foto do eleitor apresentada" |

A varredura do cabeçalho JPEG está inlinada. Ela roda sobre um segundo objeto, construído a partir de uma
cópia dos bytes por `shared_f5111` = `{int 1; vector}` (o construtor que
`CConversorFoto::DeconverteFormatoImagem` também usa, provavelmente `CFoto(formato, bytes)`). Ela percorre
os marcadores `0xFF xx`, lê altura e largura de SOF0..SOF15 (exceto DHT/JPG/DAC) e DHP, pula
DHT/DQT/DRI/DNL/EXP/APPn/COM/0x01/0xF0 pelo seu comprimento (RST0 = 0xD0 também, embora marcadores RST não
tenham campo de comprimento), memoriza APP1 (flag) e o comprimento do COM, e para em SOS. Todo índice tem
verificação de limites, e um segmento truncado levanta `std::out_of_range` (`vector::__throw_out_of_range`,
isto é, `vector::at`, ver §13).

## 7. Objetos de valor validados

| classe (func) | regra → código de erro, mensagem, linha |
|---|---|
| `CCandidatura::CCandidatura` (5660) | cargo < 100 (7991 "Número do cargo inválido", 37); partido < 100 (7992, 40); número < 100000 (7993, 43); `ordemSuplencia == 0` do titular (7994 "Titular é suplente", 46); ≤ 9 suplentes (7995, 49); o suplente *i* tem ordem *i+1* (7996 "Ordem de suplente errada ({} [{}])" com ordem, nomeUrna, 55: tipos de argumento empacotados 419 = int, string_view, então o texto fica, por exemplo, "(2 [FULANO])") |
| `CCandidatura::GetSuplente(n)` (1389) | 1 ≤ n ≤ tamanho (7997 "Suplente inexistente: {}", 65) |
| `CMunicipio::CMunicipio(codigo, nome, comBiometria)` (3712) | `Trim(nome)` não vazio (8119 "Nome vazio", 27) |
| `CInfoMunicipio::ValidaCriacao` (5675) | código < 100000 (8009, 61); nome não vazio (8010, 66); fuso em −720..720 minutos (8011, 69) |
| `CInfoMunicipio::GetHorarioVerao` (3725) | horário de verão presente (8008, 51) |
| `CComplementoMunicipio::ValidaCriacao` (5647) / `GetHorarioVerao` (2795) | código < 100000 (8113, 47); fuso ±720 (8114, 52) / horário de verão presente (8112, 37) |
| `CHorarioVeraoMunicipio::ValidaCriacao` (5678) | código < 100000 (8006, 47) |
| `md::CLocal::ValidaCriacao` (5673) | país não vazio (8014, 75); sigla da UF com 2 caracteres (8015, 78); nome da UF não vazio (8016, 81); município da seção == município do local (8017, 84); município da contingência == local (8018, 89) |
| `md::CLocal::GetSecao` / `GetContingencia` (943 / 3724) | optional presente (8012 / 8013, "Não há informação.") |
| `CCarga::ValidaCriacao` (5670) | serial do MC/flash = 8 caracteres hex (8019 / 8020, 36/41); `codigoCarga` = 24 dígitos decimais (8021 / 8022, 46/51) |
| construtor de `CCorrespondenciaResultado` (2801) | município < 100000 (8027), zona < 10000 (8028), seção < 10000 (8029), tipo de urna ∈ '1'..'4' (8030) (linhas 42..57) |
| construtor de `CLocalidadeEleitoral` (5631) | município < 100000 (8095), zona < 10000 (8096), seção < 10000 (8097) (18..24) |
| construtor de `CDadoCarga` (5633) | modelo ∈ 2013..2022 (8075 "Modelo inválido: {}", 84) |
| `CDadoCarga::GetFaseChar` (2253) | fase '1'/'2'/'3' → `'o'`/`'s'`/`'t'` (oficial/simulado/treinamento), senão 8077 "Fase inválida: {}" (108) |
| `CAjusteDataHora::AdicionaDeltaT` (3702) | apenas quando tipo == 2 `eAlterarDataSistema` (8071), então valor += delta |

`CEstadoGeral::RecuperarCertificado` (5635, linha 147) lê o certificado da urna uma vez via
`api::pkcs11::IPkcs11` (slots 23 / 10 / 24 = abrir, ler, fechar; nomes desconhecidos) e o guarda em cache
num vetor estático. Nenhuma classe no binário implementa `IPkcs11` (sem RTTI, sem `CPolySingletonList::push`),
então neste build a chamada só pode lançar `EPatternError 1301 "PolySingleton - solicitada uma instancia nao criada
N3api6pkcs117IPkcs11E [<path>/cestadogeral.cpp:147]"` (a func 3704 acrescenta o `std::source_location` do chamador).
A evidência estática: nenhum typeinfo menciona `pkcs11`, e o nome de typeid `N3api6pkcs117IPkcs11E` é
referenciado apenas pela busca 3704 (um `push<IPkcs11>` também o referenciaria).

## 8. `CTradutorFrase` (tradutor de placeholders)

`CTradutorFrase::Traduz` (654, a entrada pública; nome inferido) é chamada para textos de tela e de
relatório. Se nenhum rótulo está carregado, ela retorna o texto sem alteração. Caso contrário, constrói três
regexes do boost. `K` é o conjunto de chaves de rótulo carregadas do arquivo PU, por exemplo `S` seção,
`Z` zona, `M` município, `P` partido:

* `<[K][CL][PS][ABN]>` → `TraduzLabel`. `C/L` = *curto/longo*, `S/P` = singular/plural,
  `A/B/N` = maiúsculas/minúsculas/sem alteração (com suporte a Latin-1, 3509/5158). Exemplo: `<PLSN>` →
  "Partido". Erros 8129/8130/8131 "Label inválido: ".
* `<[K]\|[^|>]*\|[^|>]*\|[^|>]*>` → `TraduzTexto`: escolhe a parte 1/2/3 pelo gênero do rótulo
  (neutro/masculino/feminino). Exemplo: `"não pertence <S|a|ao|à> <SCSN>"`. Erros 8132/8133
  "Texto inválido: ".
* `<DT[+-][0-9]+>` → data de referência ± N dias, formatada `DD/MM/YYYY` (operadores de `api::CDate`
  5478/5477/3648).

Ela pega repetidamente o casamento mais à esquerda, acrescenta o texto anterior a ele e acrescenta a
tradução. `RecuperaLabel` (5658) busca `token[0]` (`8128 "Token inválido: "`). O mapa de rótulos e a data
de referência são estáticos (@1839056, @1839068) carregados com a parametrização.

## 9. Dados lidos e gravados

| dado | formato | quem |
|---|---|---|
| `rdv.dat` (+ `.tmp`) em `/dsk/f{i,e}/dinamico/trab1` | BER `ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto`, cifrado por `api::CEncryptedFile` com a cifra da tabela criptográfica de `CRdv` | `CRdvVota::Converte/Desconverte/ConfereConteudo` |
| tabela `eleitor_dinamico` de `uenux.db` | SQLite 3.50.4, 15 colunas (§5) | `CEleitorDinamicoDAO` |
| `*-ca.dat` (candidatos), `*-lo.dat` (local), `*-el.dat` (eleitores), `*-pu.dat` (rótulos), `eg.bin` | módulos ASN.1 `ModuloCandidatos`, `ModuloLocal`, `ModuloComplementosMunicipios`, `ModuloEleitores`, `ModuloParametrizacaoUrna`, `ModuloEstadoGeralUrna` | os objetos de valor da §7 validam o que os conversores (unidades u03/u21) decodificam |
| token PKCS#11 | bytes do certificado | `CEstadoGeral::RecuperarCertificado` |
| LCD do MT | JPEG / imagens de recurso | func 3616 |

## 10. BU (*Boletim de Urna*): o que esta unidade fornece

O gerador do BU em si (`vota::CGeraBU::StartState` 12110, `comum::CGeradorBUBase<CRdvVota, CEleitores>`,
`CDataSourcesRelatorio<...>`, `CGravadorBU`) pertence a outras unidades. Esta unidade fornece os números e
os campos de identificação:

1. **As contagens de votos** vêm todas de `CRdvVota::GetInst()` (§3.2). Elas são lidas por cargo enquanto o
   cursor `CCargos` percorre os cargos:
   * cargos majoritários (`ImprimeMajoritario` 11254): para cada candidatura `Candidato(cargo, número,
     dígitos)` mais `Cargo(cargo)`; rodapé `TrailerMajoritario` (11980) imprime
     `"Brancos {:04}"` = `Brancos(cargo)`, `"Nulos {:04}"` = `Nulos(cargo)`, `"Total Apurado {:04}"` =
     `Cargo(cargo)`, e também lê `Nominais(cargo)`;
   * cargos proporcionais (`ImprimeProporcionalPartido` 11255): por partido, a linha de cabeçalho da func
     11261 (`"{}: {} - {}\n"` = `Traduz("<PLSN>")`, número do partido, nome/sigla do partido), seguida do
     cabeçalho de detalhe de candidatos apenas quando `Legenda(cargo, partido) != Partido(cargo, partido)`
     (isto é, o partido tem votos em candidatos). Depois, por candidato, `Candidato(...)`, e
     `TrailerProporcionalPartido` (11260): `"    Total <P|de|do|da> <PLSB>"` com `Partido(...)` e os votos
     de legenda com `Legenda(...)`. `TrailerProporcional` (11981): `"Total de votos de Legenda {:04}"` =
     `Legendas(cargo)`, Brancos, Nulos, Total Apurado, e uma string `"{:04}{:04}{:04}{:04}{:04}"` (entrada
     do código verificador) construída a partir de Nominais/Legendas/Brancos/Nulos/Cargo;
   * consultas (`ImprimeConsulta` 11253): por resposta, `Candidato(cargo, número, dígitos)` (respostas com 0
     votos são puladas), formatado `"{:05}{:04}"` para o código verificador;
   * comparecimento: `CRdvVota::Comparecimento()` (sobrecarga sem argumentos, func 1269: o maior
     `Comparecimento(eleição)` entre as eleições configuradas). Ela também decide "houve votos?"
     (`CQuerReimprimirZeresima`, `CReinicioVotacao`, `CGravaResultado`, `CGeraBU`).
2. **Identificação do arquivo**: todo arquivo de resultado (BU, RDV e outros envelopes) carrega um
   `CCorrespondenciaResultado` construído por 2801: município (< 100000), zona (< 10000), seção (< 10000), o
   `CCarga` (número interno da urna, serial do cartão de memória = 8 caracteres hex, código de carga de 24
   dígitos, identificação do gerador) e o tipo de urna ('1'..'4'). `CGravadorBU::vf7` / `CGravadorRDV::vf7` /
   `IGravadorEnvelope::vf7` o constroem. O construtor o valida, então uma identificação de carga corrompida
   aborta a gravação.
3. **Letra da fase**: `CDadoCarga::GetFaseChar` ('o'/'s'/'t') é usada por `CGeraBU`, `CGravaResultado`,
   `CCopiaResultadoParaMR` e `CCargos::GetCurrentEleicaoVersaoPacote` para montar os nomes dos arquivos de
   resultado/pacote (o prefixo `t…` dos arquivos de treinamento).
4. **QR codes**: `comum_f5634` (chamada por `CGeraBU` com "incluir certificado" = 1) chama
   `CEstadoGeral::RecuperarCertificado()`. Quando o certificado é incluído, ela calcula o número de QR codes
   do BU como `ceil(2 * len(cert) / 1082)`. Neste build essa chamada lança exceção (§7, §13).
5. **Zerésima** (o relatório impresso antes do início da votação): `vota::CriaTituloExtratoRDV` (5971) chama
   `CRdvVota::GetVotos()` e verifica por assert que a lista de votos de cada cargo está vazia (`"Assert (votosCargos.Total()
   == 0)"`) antes de imprimir `"-----------EXTRATO DO RDV-------------"`. As linhas de resposta de consulta da
   zerésima vêm da func 11478 (`"  " + PadRight(nome, 26) + PadLeft(format("{:0{}}", número, dígitos),
   10)`).
6. **Arquivo RDV** (companheiro do BU): `rdv.dat` = cifrar(BER(`EntidadeRegistroDigitalVoto`)), gravado
   com a verificação por releitura `ConfereConteudo` (§3.3).

## 11. Particularidades do build web

* `CSincronismoVotoEleitorWeb` retorna sucesso sem persistir nada. `rdv.dat` fica como foi criado por
  `votaInit` (um RDV vazio cifrado, 80 bytes), e `uenux.db` nunca recebe a tabela `eleitor_dinamico`
  (verificado: `sqlite3 .tables` nos dois snapshots do MEMFS lista apenas `comparecimento_mesario` e
  `registro_justificativa`; o motivo é o modo de treinamento do eleitor, ver §3.3). O RDV em memória de fato
  recebe as cédulas (4454 observada).
* Não existe implementação de `api::pkcs11::IPkcs11`, então `CEstadoGeral::RecuperarCertificado` não pode
  ter sucesso.
* `api::CAjusteDataHora` (10874/10875) repassa para o slot 1 de `ISystemDateTime`. No simulador, esse é
  `simulador::CWasmSystemDateTime::vf1` (7945). Ele não mexe em relógio nenhum: armazena
  `novo − js_obter_data_hora_local_navegador()` como um deslocamento. O `api::CSystemDateTime::vf1` nativo
  (10853) é um stub vazio neste build.
* O código da foto do eleitor (3616) tem como alvo o LCD do microterminal do mesário (`CInfoMTLCD`, cujo
  `GetInst` é a func 2284). O simulador nunca executa os fluxos do mesário.
* Todo o resto da unidade (regras de validação, dígitos verificadores, tradução de rótulos) é código comum
  da aplicação, sem nenhum mock envolvido.

## 12. Funções desta unidade cujo arquivo original pertence a outra unidade

As versões completas estão em `src/uenux2/src/app/comum/dados/u05-foreign-fragments.cpp`, com uma seção
por arquivo original, para que as unidades donas possam incorporá-las. Resumo:

```cpp
// func 10625 — vota::IEleitorImpedidoVotar::StartState (vtable slot 2), file of unit u27
// (path inferred: uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp).
// Base of CEleitorNaoEncontrado, CEleitorOptouPorVotarEmTransito, CEleitorImpedidoJustificar,
// CEleitorNaoTemIdadeMinima, CEleitorNaoPossuiCargosParaVotar, CEleitorImpedidoJustificarVotoTransito.
void IEleitorImpedidoVotar::StartState()
{
    m_form->Exibe();          // member at +12, its vtable slot 2
    this->vf9();              // hook, icf_nop in the base
    ApresentaFotoEleitor();   // func 3616
    m_proximo = this;         // CState "next state" (+4) = stay here
}

// func 11261 — static std::string CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderProporcionalPartido()
// (name inferred; cdatasourcesrelatorio.h, unit u25). Passed as a text source (table slot 2921) to the BU.
static std::string HeaderProporcionalPartido()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    const md::CPartido& partido = CPartidos::GetInst().GetCurrent();          // ecourna_f819
    std::string linha = std::format("{}: {} - {}\n", md::CTradutorFrase::Traduz("<PLSN>"),
                                    partido.GetNumero(), partido.GetNome());   // +0 u16, +4 string
    const auto& rdv = CRdvVota::GetInst();
    if (rdv.Legenda(cargo, partido.GetNumero()) != rdv.Partido(cargo, partido.GetNumero())) {
        linha += CCargoDSLabelRelatorio::HeaderDetalhe();   // two appends, no operator+ temporary
        linha += "\n";
    }
    return linha;
}

// func 11478 — zerésima line of a consulta answer (name inferred; table slot 3051 next to
// CRelUtil::DSCodigoVerificador and (anonymous)::TituloPartido, i.e. comum/relatorios/crelutil.cpp).
static std::string DSLinhaRespostaZE()
{
    const uebyte digitos = CCargos::GetInst().GetCurrent().GetQtdDigitos();       // +12
    const auto& resposta = CRespostas::GetInst().GetCurrent();                   // CDataMap cursor
    const std::string numero = std::format("{:0{}}", resposta.GetNumero(), digitos);
    return "  " + PadRight(resposta.GetNome(), 26) + api::CStringUtils::PadLeft(numero, ' ', 10);
}

// func 11479 — current consulta answer name (text source, slot 1096), used by
// vota::(anonymous)::adicionaBaseTelaCompletaConsulta (ctelasvota.cpp).
static std::string NomeRespostaCorrente() { return CRespostas::GetInst().GetCurrent().GetNome(); }

// func 11378 — comum::asn::CConversorMunicipio::DoDesconverte (slot 3; DoConverte 11379 is in u21)
md::CMunicipio CConversorMunicipio::DoDesconverte(const ModuloTiposCadastro::Municipio& e) const
{
    return md::CMunicipio(e.codigo, e.nome,
                          e.hasOptionalField(1 /*comBiometria*/) ? bool(e.comBiometria) : false);
}

// func 11389 — comum::asn::CConversorLocalidadeEleitoral::DoDesconverte (class of unit u35)
md::estadoaplicacao::CLocalidadeEleitoral DoDesconverte(const TEntidade& e) const
{
    return {e.municipio, e.zona, e.secao};      // constructor 5631 validates
}

// func 5772 — std::vector<md::CEleitorDecorator> CEleitores::GetEleitoresEstaticos(const std::string& dir) const
//   (u04's file; name and signature from the RTTI of its lambda, GetEleitoresEstaticos(std::string const&)::$_0)
//   lists = file-name lists at this+76 and this+64 (api_f1936 builds full paths);
//   eleitores = Concatena(lista1, [](const std::string& arquivo) { /* $_0: partial ASN.1 decode of
//               ModuloEleitores::EntidadeEleitores via CVisitanteEleitor (11523) */ });
//   eleitores.insert(end, Concatena(lista2, same lambda))           // 5770
//   for (auto& e : eleitores) result.push_back(CEleitorDecorator(e, tipoPrincipal /* this+100 */));
//   std::sort(result.begin(), result.end());                          // 5769, operator<=> 456
// func 5771 — Concatena(const std::vector<std::string>& chaves (arrives as begin/end),
//                        std::function<std::vector<md::CEleitor>(const std::string&)> f /* by value */):
//   for each key: auto v = f(key); r.insert(r.end(), v.begin(), v.end());   (bad_function_call if empty)

// func 753  — std::string api::CStringUtils::PadLeft(const std::string& s, char c, size_t n)
//             { std::string r = s; if (r.size() < n) r.insert(0, n - r.size(), c); return r; }
// func 2284 — comum::CInfoMTLCD& CInfoMTLCD::GetInst(): lazy `new` (64 bytes) singleton @1838572; its
//             constructor 5904 stores vtable comum::CInfoMTLCD (the tools named 5904 after the srcloc of
//             its inlined member ctor api::BatteryIconDataSource<Vertical>)
// funcs 5478 / 5477 / 3648 — api::CDate::operator+=(int) / operator-=(int) / operator-(CDate, int):
//             day arithmetic with a month-length table; a negative argument delegates to the other one
// func 2794 — md::CVotos::CVotos(std::vector<CVoto> v) : m_votos(std::move(v)) { m_votos.reserve(1000); }
// func 655  — CBaseError<api::EUeRdvError,{4650,4850}> constructor thunk (ecourna_f710 + vtable)
```

## 13. Observações de Wasm/Emscripten

* **Inlining do LTO entre arquivos.** Muitos srclocs dentro de uma função citam *outra* função: 566
  contém cvalidadoridentidade.cpp:61 e iregraidentidade.cpp:23, 3616 contém cbiometriaeleitor.cpp:115,
  11478/11479 contêm crespostas.cpp:28, e 1930/2813/... contêm cvotoseleicoesvota.cpp:1xx. As ferramentas
  nomeiam uma função pelo seu primeiro srcloc, então várias entradas da unidade estavam com o nome errado
  (566, 654, 2801, 3616, 5631, 5633, 11478/11479). Elas foram renomeadas nos overrides.
* **Construtores retornam `this`** (ABI C++ do wasm). É assim que 566, 2801, 5663/5664 e 3712 são
  reconhecidas como construtores.
* **merge-similar-functions**: CRdvVota vf6..vf10 são thunks de 22 bytes para 2296 que passam
  `(srcloc, error code, lambda vtable)`. vf4/vf5 vão para 6033. `CLocal::GetContingencia` é um thunk para
  o getter de optional 6029, compartilhado com `CCargo::GetDetalheConsulta`. O pseudo-código anota a sua
  constante 8013 como uma string ("Falha ao validar assinatura UE...") por acidente: é o código de erro.
* **Identical code folding**: os três destrutores de DAO compartilham 1704/3767. `CRegraTitulo::GetTipo` é
  `icf_ret_1_vf3` (434) e `CRegraCPF::GetTipo` é `icf_tiny_vf2` (2254).
* **Estáticos locais de função** (bytes de guarda @1911832/@1911848, raízes de set @1911820/@1911836) para
  as tabelas de maiúsculas/minúsculas Latin-1. O global `std::string TABELA = "eleitor_dinamico"` (@1838780)
  é construído inline em `__wasm_call_ctors`, e o seu destrutor é registrado como func 11545.
* **`__THREW__`** (trampolins invoke_*) aparece nesta unidade apenas em 753 e 5159 (insert/erase de string
  após uma cópia, limpeza da cópia), 5157 (`operator new` dentro do construtor de `std::set<int>`) e 3509
  (o insert no set ao construir o seu estático local). Falhas de `vector::at` / `string::at` chamam
  `__throw_out_of_range` diretamente, e nada nesta unidade as captura.
* Verificações de faixa sem sinal aparecem como `(x - hi - 1) <u -(hi - lo + 1)`: UF 1..28, modelo
  2013..2022, tipo de urna '1'..'4', largura/altura da foto.
* Nada nesta unidade chama `emscripten_sleep`, acessa a rede ou lê entrada de URL/JS.

## 14. Código suspeito ou arriscado (resumo; detalhes no relatório estruturado)

1. **`CEstadoGeral::RecuperarCertificado` (5635) não pode ter sucesso neste build.** Nenhuma
   implementação de `api::pkcs11::IPkcs11` é compilada, então a etapa de QR code de `CGeraBU` (`comum_f5634`)
   lançaria `EPatternError 1301` assim que executasse. Ela é código morto nas sessões gravadas (não há
   encerramento da votação).
2. **Restrições `CHECK` na coluna errada** na DDL de `eleitor_dinamico` (3768). As restrições de
   `resultado_decifracao_foto` e de `tipo_identificador_mesario` testam outras colunas, então essas duas
   colunas não são validadas pelo SQLite. Este é o código original (o mesmo na urna).
3. **`map::find(...)->second` sem verificação** nas consultas inlinadas de `CVotosEleicoesVota` (2296/6033/1931).
   A busca `cargo → eleição` é verificada, mas o `CVotosCargos` da eleição não. Um RDV inconsistente
   faria a consulta ler o nó final do map como um objeto (comportamento indefinido, contagens erradas ou um
   trap). Só é alcançável se os dois maps discordarem. `EstruturaCompativel` protege o caminho de carga.
4. **`std::out_of_range` não tratado na varredura JPEG da foto do eleitor (3616).** Um segmento truncado na
   foto (decifrada) lança a partir de `vector::at` dentro do `StartState` do operador. Nada em 3616 nem nos
   três chamadores `StartState` o captura. Não foi verificado se o driver da máquina de estados acima deles o
   captura.
5. **A regra de CPF aceita CPFs com dígitos repetidos** (inclusive todos zeros). Em `CValidadorIdentidade`,
   uma regra de "identificador livre" registrada em primeiro lugar prevalece para *todo* tipo, então os
   dígitos verificadores de título/CPF são pulados (depende da configuração).
6. **`CTradutorFrase::Traduz`** coloca as chaves de rótulo sem escape dentro de uma classe de caracteres de
   regex. Ela compila três `boost::regex` por placeholder (pesado em CPU no wasm). O helper de regex engole
   exceções, então um padrão malformado deixa o texto silenciosamente sem tradução. `<DT±N>` com N > 65535
   passa por `ToWord`.
7. `CDadoCarga` aceita qualquer número de modelo em 2013..2022 (inclusive modelos que não existem) e
   rejeita qualquer coisa mais nova que 2022.
8. **`CEleitorDinamicoDAO::Recuperar` (11542) constrói a identidade do eleitor a partir de duas fontes
   diferentes.** Com o override do mesário, o título vem da linha (coluna 0). Sem ele, vem da chave de
   busca. Eles são iguais sob `WHERE titulo = ?` (ambos são normalizados por `CEleitorIdentidade`), então o
   único efeito é que os dois ramos do código original são inconsistentes.

## 15. Tabela de mapeamento completa (100 funções)

`run` = observada executando durante os votos gravados. "src" é o arquivo reconstruído em `src/`, ou onde a
função está documentada quando é uma instanciação de biblioteca ou pertence ao arquivo de outra unidade.

| func | tamanho | run | nome antes desta unidade | símbolo reconstruído | src |
|---|---|---|---|---|---|
| 337 | 554 |  | `comum_f337` | `comum::md::CEleitor::operator=(CEleitor&&)` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 456 | 237 |  | `comum_f456` | `comum::md::CEleitorDecorator::operator<=> (name inferred)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.cpp` |
| 555 | 24 |  | `comum::CRdvVota::GetInst` | `comum::CRdvVota::GetInst` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 566 | 997 | ✓ | `comum::md::CValidadorIdentidade::Valida` | `comum::md::CEleitorIdentidade::CEleitorIdentidade(std::string, ETipoIdentificadorEleitor)` | `uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp` |
| 654 | 4752 | ✓ | `comum::md::CTradutorFrase::TraduzLabel` | `comum::md::CTradutorFrase::Traduz (name inferred; tools: TraduzLabel)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 655 | 18 |  | `comum_f655` | `ecourna::api::exception::CBaseError<api::EUeRdvError,{4650,4850}>::CBaseError (merged ctor thunk)` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 708 | 581 |  | `comum::md::CEleitorDecorator::GetIdentidadePorTipo` | `comum::md::CEleitorDecorator::GetIdentidadePorTipo` | `uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.cpp` |
| 753 | 173 | ✓ | `api_f753` | `api::CStringUtils::PadLeft(const std::string&, char, size_t) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: uenux2/src/api/util/cstringutils.cpp, unidade u20) |
| 943 | 74 |  | `comum::md::CLocal::GetSecao` | `comum::md::CLocal::GetSecao` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 945 | 1833 |  | `comum_f945` | `std::__sort3<_ClassicAlgPolicy, __less<>&, comum::md::CEleitorDecorator*>` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 946 | 584 | ✓ | `api_f946` | `comum::md::CEleitor::CEleitor(const CEleitor&)` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 1389 | 512 |  | `comum::md::CCandidatura::GetSuplente` | `comum::md::CCandidatura::GetSuplente` | `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.cpp` |
| 1548 | 76 |  | `comum::md::CEleitorDinamico::GetTituloMesarioHabilitacao` | `comum::md::CEleitorDinamico::GetTituloMesarioHabilitacao` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 1550 | 32 |  | `comum_f1550` | `std::__tree<pair<TCargoID,TEleicaoID>>::destroy (map node free)` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 1904 | 22 |  | `comum_f1904` | `vota::(anonymous)::RegistraErroFoto (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 1926 | 24 |  | `comum::md::CValidadorIdentidade::GetInst` | `comum::md::CValidadorIdentidade::GetInst` | `uenux2/src/app/comum/dados/md/cvalidadoridentidade.cpp` |
| 1930 | 22 |  | `comum::CRdvVota::vf10` | `comum::CRdvVota::Cargo` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 1931 | 343 |  | `comum::CRdvVota::vf3` | `comum::CRdvVota::Candidato` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2253 | 510 |  | `comum::md::estadoaplicacao::CDadoCarga::GetFaseChar` | `comum::md::estadoaplicacao::CDadoCarga::GetFaseChar` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.cpp` |
| 2284 | 88 |  | `api_f2284` | `comum::CInfoMTLCD::GetInst (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: uenux2/src/app/comum/cinfomtlcd.cpp, unidade u22) |
| 2296 | 239 |  | `comum_f2296` | `comum::md::CVotosEleicoesVota::Consulta (merged body of Nominais/Legendas/Nulos/Brancos/Cargo, inlined into CRdvVota)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2794 | 290 |  | `comum_f2794` | `comum::md::CVotos::CVotos(std::vector<CVoto>)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: md/rdv, unidade u22) |
| 2795 | 73 |  | `comum::md::CComplementoMunicipio::GetHorarioVerao` | `comum::md::CComplementoMunicipio::GetHorarioVerao` | `uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp` |
| 2798 | 84 |  | `comum_f2798` | `comum::md::CEleitorIdentidade::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp` |
| 2799 | 878 |  | `comum::md::CBiometriaEleitor::CriaComum` | `comum::md::CBiometriaEleitor::CriaComum` | `uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp` |
| 2801 | 1953 |  | `comum::md::CCorrespondenciaResultado::ValidaCriacao` | `comum::md::CCorrespondenciaResultado::CCorrespondenciaResultado (+ inlined ValidaCriacao)` | `uenux2/src/app/comum/dados/md/correspondencia/ccorrespondenciaresultado.cpp` |
| 2812 | 24 |  | `comum::CRespostas::GetInst@2812` | `comum::CRespostas::GetInst` | `uenux2/src/app/comum/dados/crespostas.cpp` |
| 2813 | 22 |  | `comum::CRdvVota::vf6` | `comum::CRdvVota::Nominais` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 2814 | 24 |  | `comum::CRdvVota::vf5` | `comum::CRdvVota::Partido` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3509 | 530 | ✓ | `comum_f3509` | `comum::md::ConverteMaiusculas (name inferred)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 3616 | 3477 |  | `comum::md::CBiometriaEleitor::GetFoto` | `vota::ApresentaFotoEleitor (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 3617 | 222 |  | `comum_f3617` | `vota::(anonymous)::MostraFotoIndisponivel (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 3648 | 19 |  | `comum_f3648` | `api::operator-(const CDate&, int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: uenux2/src/api/util/cdate.cpp, unidade u20) |
| 3702 | 494 |  | `comum::md::estadoaplicacao::CAjusteDataHora::AdicionaDeltaT` | `comum::md::estadoaplicacao::CAjusteDataHora::AdicionaDeltaT` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cajustedatahora.cpp` |
| 3707 | 173 |  | `comum_f3707` | `comum::md::CVotosEleicoesVota::EleicaoDoCargo (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3712 | 107 |  | `comum::md::CMunicipio::CMunicipio` | `comum::md::CMunicipio::CMunicipio` | `uenux2/src/app/comum/dados/md/municipiozona/cmunicipio.cpp` |
| 3720 | 249 |  | `comum::md::CBiometriaEleitor::GetDedo` | `comum::md::CBiometriaEleitor::GetDedo` | `uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp` |
| 3723 | 192 |  | `vota_f3723` | `comum::md::IRegraIdentidade::EhValida (name inferred)` | `uenux2/src/app/comum/dados/md/iregraidentidade.cpp` |
| 3724 | 22 |  | `comum::md::CLocal::GetContingencia` | `comum::md::CLocal::GetContingencia` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 3725 | 73 |  | `comum::md::CInfoMunicipio::GetHorarioVerao` | `comum::md::CInfoMunicipio::GetHorarioVerao` | `uenux2/src/app/comum/dados/md/cinfomunicipio.cpp` |
| 3747 | 22 |  | `comum::CRdvVota::vf9` | `comum::CRdvVota::Brancos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3748 | 22 |  | `comum::CRdvVota::vf8` | `comum::CRdvVota::Nulos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3749 | 24 |  | `comum::CRdvVota::vf4` | `comum::CRdvVota::Legenda` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 3761 | 1110 |  | `comum_f3761` | `std::__sort4<..., comum::md::CEleitorDecorator*>` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 3766 | 3613 |  | `comum::dao::CEleitorDinamicoDAO::vf8` | `comum::dao::CEleitorDinamicoDAO::RecuperarTodos (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 3768 | 618 |  | `comum::dao::CEleitorDinamicoDAO::CEleitorDinamicoDAO` | `comum::dao::CEleitorDinamicoDAO::CEleitorDinamicoDAO` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 5157 | 288 | ✓ | `comum_f5157` | `std::set<int>::set(std::initializer_list<int>)` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5158 | 291 | ✓ | `comum_f5158` | `comum::md::ConverteMinusculas (name inferred)` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 5159 | 283 |  | `comum_f5159` | `comum::md::(anonymous)::SemZerosEsquerda (name inferred)` | `uenux2/src/app/comum/dados/md/iregraidentidade.cpp` |
| 5413 | 20 |  | `comum_f5413` | `vota::(anonymous)::RegistraEleitorSemFoto (name inferred)` | `uenux2/src/app/vota/operador/comum/capresentacaofotoeleitor.cpp` |
| 5477 | 949 |  | `comum_f5477` | `api::CDate::operator-=(int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: uenux2/src/api/util/cdate.cpp, unidade u20) |
| 5478 | 945 |  | `comum_f5478` | `api::CDate::operator+=(int dias) (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: uenux2/src/api/util/cdate.cpp, unidade u20) |
| 5631 | 204 |  | `comum::md::estadoaplicacao::CLocalidadeEleitoral::ValidaCriacao` | `comum::md::estadoaplicacao::CLocalidadeEleitoral::CLocalidadeEleitoral (+ inlined ValidaCriacao)` | `uenux2/src/app/comum/dados/md/estadoaplicacao/clocalidadeeleitoral.cpp` |
| 5633 | 525 |  | `comum::md::estadoaplicacao::CDadoCarga::ValidaModelo` | `comum::md::estadoaplicacao::CDadoCarga::CDadoCarga (+ inlined ValidaModelo)` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.cpp` |
| 5635 | 279 |  | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado` | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado` | `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp` |
| 5638 | 598 |  | `comum_f5638` | `comum::md::CVotosEleicoesVota::EstruturaCompativel (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5647 | 136 |  | `comum::md::CComplementoMunicipio::ValidaCriacao` | `comum::md::CComplementoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp` |
| 5658 | 193 |  | `comum::md::CTradutorFrase::RecuperaLabel` | `comum::md::CTradutorFrase::RecuperaLabel` | `uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp` |
| 5660 | 1196 | ✓ | `comum::md::CCandidatura::CCandidatura` | `comum::md::CCandidatura::CCandidatura` | `uenux2/src/app/comum/dados/md/candidatura/ccandidatura.cpp` |
| 5663 | 275 |  | `comum_f5663` | `comum::md::CEleitorDinamico::CEleitorDinamico (with título do mesário)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 5664 | 210 |  | `comum_f5664` | `comum::md::CEleitorDinamico::CEleitorDinamico (without título do mesário)` | `uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.cpp` |
| 5667 | 293 |  | `comum::md::CEleitor::ValidaCriacao` | `comum::md::CEleitor::ValidaCriacao` | `uenux2/src/app/comum/dados/md/eleitor/celeitor.cpp` |
| 5670 | 1221 |  | `comum::md::CCarga::ValidaCriacao` | `comum::md::CCarga::ValidaCriacao` | `uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp` |
| 5673 | 343 |  | `comum::md::CLocal::ValidaCriacao` | `comum::md::CLocal::ValidaCriacao` | `uenux2/src/app/comum/dados/md/clocal.cpp` |
| 5675 | 1033 |  | `comum::md::CInfoMunicipio::ValidaCriacao` | `comum::md::CInfoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/cinfomunicipio.cpp` |
| 5678 | 478 |  | `comum::md::CHorarioVeraoMunicipio::ValidaCriacao` | `comum::md::CHorarioVeraoMunicipio::ValidaCriacao` | `uenux2/src/app/comum/dados/md/chorarioveraomunicipio.cpp` |
| 5732 | 22 |  | `comum::CRdvVota::vf7` | `comum::CRdvVota::Legendas` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5733 | 1192 |  | `comum::CRdvVota::vf0` | `comum::CRdvVota::GetVotos` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 5755 | 1469 |  | `comum_f5755` | `std::__sort5<..., comum::md::CEleitorDecorator*>` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5756 | 1027 |  | `comum_f5756` | `std::__insertion_sort_incomplete<..., comum::md::CEleitorDecorator*>` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5757 | 423 |  | `api_f5757` | `std::vector<comum::md::CEleitor>::__move_range` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5759 | 277 |  | `comum_f5759` | `std::vector<comum::md::CEleitorDinamico>::__push_back_slow_path` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5768 | 520 |  | `api_f5768` | `std::vector<comum::md::CEleitorDecorator>::__swap_out_circular_buffer` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5769 | 6341 |  | `comum_f5769` | `std::__introsort<..., comum::md::CEleitorDecorator*>` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5770 | 1515 | ✓ | `api_f5770` | `std::vector<comum::md::CEleitor>::__insert_with_size (range insert)` | helper de biblioteca/inlinado (listado em u05-foreign-fragments.cpp) |
| 5771 | 487 | ✓ | `api_f5771` | `comum::(anonymous)::ConcatenaEleitores (name inferred: for each file path, f(path) appended)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: celeitores.cpp, unidade u04) |
| 5772 | 2143 | ✓ | `api_f5772` | `comum::CEleitores::GetEleitoresEstaticos(const std::string&) const (lambda RTTI)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: celeitores.cpp, unidade u04) |
| 5775 | 931 |  | `comum::dao::CEleitorDinamicoDAO::vf6` | `comum::dao::CEleitorDinamicoDAO::Atualizar` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 5776 | 942 |  | `comum::dao::CEleitorDinamicoDAO::vf3` | `comum::dao::CEleitorDinamicoDAO::Inserir (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 6033 | 339 |  | `comum_f6033` | `comum::md::CVotosEleicoesVota::Consulta (merged body of Legenda/Partido, inlined into CRdvVota)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 10625 | 45 |  | `vota::IEleitorImpedidoVotar::vf2` | `vota::IEleitorImpedidoVotar::StartState` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: vota/operador, unidade u27) |
| 10874 | 23 |  | `api::CAjusteDataHora::vf1` | `api::CAjusteDataHora::AjustaDataHora(time_t) (name inferred)` | `uenux2/src/api/util/cajustedatahora.cpp` |
| 10875 | 23 |  | `api::CAjusteDataHora::vf0` | `api::CAjusteDataHora::AjustaDataHora(const CDateTime&) (name inferred)` | `uenux2/src/api/util/cajustedatahora.cpp` |
| 11261 | 699 |  | `comum_f11261` | `comum::CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderProporcionalPartido (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: cdatasourcesrelatorio.h, unidade u25) |
| 11269 | 13 |  | `comum::md::CRegraCPF::vf3` | `comum::md::CRegraCPF::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/cregracpf.cpp` |
| 11270 | 571 |  | `comum::md::CRegraCPF::vf4` | `comum::md::CRegraCPF::Verifica (name inferred)` | `uenux2/src/app/comum/dados/md/cregracpf.cpp` |
| 11271 | 13 |  | `comum::md::CRegraTitulo::vf3` | `comum::md::CRegraTitulo::Formata (name inferred)` | `uenux2/src/app/comum/dados/md/cregratitulo.cpp` |
| 11272 | 537 | ✓ | `comum::md::CRegraTitulo::vf4` | `comum::md::CRegraTitulo::Verifica (name inferred)` | `uenux2/src/app/comum/dados/md/cregratitulo.cpp` |
| 11378 | 153 | ✓ | `comum::asn::CConversorMunicipio::DoDesconverte` | `comum::asn::CConversorMunicipio::DoDesconverte` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (arquivo da classe pertence à unidade u21) |
| 11389 | 36 |  | `comum::asn::CConversorLocalidadeEleitoral::vf3` | `comum::asn::CConversorLocalidadeEleitoral::DoDesconverte` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (classe pertence à unidade u35) |
| 11478 | 721 |  | `comum::CRespostas::GetInst@11478` | `comum::CRelUtil::DSLinhaRespostaZE (name inferred; zeresima consulta answer line)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: comum/relatorios, unidade u25) |
| 11479 | 137 |  | `comum::CRespostas::GetInst@11479` | `vota::(anonymous)::NomeRespostaCorrente (name inferred)` | `uenux2/src/app/comum/dados/u05-foreign-fragments.cpp` (dono: ctelasvota.cpp) |
| 11486 | 768 |  | `comum::CRdvVota::vf13` | `comum::CRdvVota::ConfereConteudo (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11487 | 707 |  | `comum::CRdvVota::Desconverte` | `comum::CRdvVota::Desconverte` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11488 | 1661 | ✓ | `comum::CRdvVota::vf11` | `comum::CRdvVota::Converte (name inferred)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11490 | 597 |  | `comum::CRdvVota::vf1` | `comum::CRdvVota::GetVotos(TEleicaoID)` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11492 | 12 |  | `comum::CRdvVota::vf2` | `comum::CRdvVota::Comparecimento` | `uenux2/src/app/comum/dados/crdvvota.cpp` |
| 11541 | 12 |  | `comum::dao::CEleitorDinamicoDAO::vf2` | `comum::dao::CEleitorDinamicoDAO::Clone (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 11542 | 2930 |  | `comum::dao::CEleitorDinamicoDAO::vf7` | `comum::dao::CEleitorDinamicoDAO::Recuperar (name inferred)` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |
| 11543 | 50 |  | `comum::dao::CEleitorDinamicoDAO::ExcluirID` | `comum::dao::CEleitorDinamicoDAO::ExcluirID` | `uenux2/src/app/comum/dados/dao/celeitordinamicodao.cpp` |

## 16. Questões em aberto

* Os nomes exatos dos slots 0/1/11/13 de CRdv (`GetVotos`, `Converte`, `ConfereConteudo`) e dos slots
  2/3/7/8 do DAO (`Clone`, `Inserir`, `Recuperar`, `RecuperarTodos`) são inferidos a partir do
  comportamento. Nenhum srcloc os nomeia.
* O tipo retornado por `CRdvVota::GetVotos()` é mostrado como `std::map<TCargoID, CVotos>`. O texto do
  assert da zerésima `votosCargos.Total()` sugere uma classe wrapper fina com um método `Total()`.
* O arquivo original e o nome da func 3616 (apresentação da foto do eleitor) e das fontes de texto 11478 /
  11479 são desconhecidos. O caminho do arquivo de `api::CAjusteDataHora` é inferido.
* Os slots 10/23/24 de `IPkcs11` não têm nome. Confirmar que `CGeraBU` realmente lança exceção no simulador
  exige um encerramento roteirizado em `tools/run/headless.mjs`. Os cenários gravados param depois dos votos.
* Vários offsets de membros de `CEleitor`/`CCarga`/`CLocal` (marcados com `?` nos headers) são conhecidos
  apenas pelo código de cópia.

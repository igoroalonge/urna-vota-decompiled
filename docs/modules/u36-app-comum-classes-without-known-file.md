# u36: funções de `app:comum` sem arquivo-fonte conhecido

A unidade u36 contém **101 funções wasm** que as ferramentas colocaram no componente `app:comum`, mas não conseguiram associar a um
arquivo original, porque nenhuma delas contém um registro `std::source_location` próprio. A maioria delas acaba sendo
uma destas coisas:

* **pequenos métodos de classes conhecidas do `comum`** que não foram inlinados: `CLocal::GetAgregadas`,
  os construtores de `CEnvelopeGenerico`, `CCabecalhoQRCodeBuilder::SetHistoricoCargas`, `CVoto::EhBranco`...;
* **corpos de "merge-similar-functions" do wasm-opt**: várias funções do código-fonte que diferiam apenas em uma constante
  (um `std::source_location`, um código de erro, uma vtable, um literal de string) foram fundidas em um único corpo que recebe as
  constantes como parâmetros. As funções do código-fonte sobrevivem como thunks de 1 linha, a maioria delas já reconstruída por
  outras unidades;
* **código gerado pelo compilador**: destrutores implícitos, membros de cópia/movimentação e **40 destrutores de armazenamento estático**
  (handlers de "atexit") de singletons. Nenhum dos handlers de atexit pode rodar neste build (§6.1);
* **instâncias de templates da biblioteca C++** que as ferramentas atribuíram ao `comum` pelo chamador: o `std::deque` por trás de
  `std::filesystem::recursive_directory_iterator`, `std::sort`, cópias de nós de `std::map` e arrays estáticos de `<locale>`
  da libc++.

O profiler por amostragem viu **7 das 101 funções** durante os votos gravados (`analysis/runtime/*.functions.tsv`):
`GetEleicoesCargos` (3774), a cópia de `std::map` 3870, `AssinaDadosDinamicos` (4668, assina `rdv.dat` +
`uenux.db` na inicialização), `CApplicationContext::operator==` (5556), os campos do QR code do estado da urna (5636),
`vector<string>::emplace_back` para `CStringUtils::Split` (9404) e a fonte de texto do número do partido na tela do eleitor
(11499). O profiler deixa passar funções curtas. Por isso rodamos o simulador com um **contador de entradas em cada uma das 101
funções** (execução de revisão: uma cópia do wasm com um global contador exportado por função, conduzida por uma cópia de
`tools/run/headless.mjs`; cenários municipal-t1 com as teclas `91001  C  12C  C  `, e geral-t1, municipal-t2,
geral-zz-t1, geral-df-t2 com `--auto mixed`). **19 funções executam**. 18 delas rodam em todos os cenários; 11499 roda
apenas em municipal-t1 e geral-t1, onde o eleitor chega a uma tela de partido:

* durante `votaInit`: 3512 `PreencheZerosEsquerda` (4 chamadas), 3774 (2), 3870 (7..18), 3896 `GetPathArquivo` de
  vota/gap/sa.bin (18), 4668 (1), 5556 (11), 5636 e 5783 (5 cada: o QR code das telas da zerésima), 5653
  `PossuiCargoEletivo` (6..12), 5726 `SIdentificacaoCarga` (1), 5744 `~md::CLocal` (5), 5763 (1), **5905
  `NomeArquivoSavd` (5: avaliada antes de cada assinatura, §3.5)**, 6030 conversores de enum (4..8), 6048
  `CPath::GetPathRoot` (1), 9404 (2);
* enquanto o eleitor vota: 3774 (+1), 5556 (+1), 5681 (1..5, a cópia do mapa `CVotosCargos` de `GravaVotos`), 11499
  (apenas quando aparece uma tela de partido) e 11501 (8..20 chamadas, a sigla do partido nas telas de candidato, §3.7).

As outras 82 nunca rodaram, em nenhum dos cinco cenários. Pertencem ao encerramento, ao registro de mesários no MT,
às impressões digitais, a caminhos de erro, a seções agregadas (nenhum cenário tem alguma) e a handlers de atexit.

Achados mais interessantes:

* **O QR code do estado da urna** (§3.4). A func 5636 constrói o registro com tags `PROC TURN FASE UNFE MUNI ZONA SECA IDUE
  MDUE IDFL IDCA DTCA HRCA NOME SERT SERI ASSI` a partir de `eg.bin`: a identificação da urna e da sua *carga*,
  incluindo o serial do certificado TPM do gerador de mídias e a assinatura inteira da carga em hex. Ele
  aparece nas telas da zerésima e no relatório impresso "ESTADO DA URNA". A func 5783 acrescenta `ORIG:T` ou `ORIG:I`.
* **Peças do BU** (§4): o campo de histórico de cargas `HIQT`/`HICA` do QR code do BU (5622), os dois
  construtores de `CEnvelopeGenerico` que envolvem o `-bu.dat` (5861 em claro, 3821 cifrado), o construtor delegante de `CPlainText`
  usado para cifrar o BU com o CEPESC (5168), o predicado de voto branco `CVoto::EhBranco` (tipos 3 e 5,
  11315) e o destrutor atexit do estado da cadeia de hashes do BU (10274).
* **Uma validação que o otimizador apagou** (§3.1): o construtor de `CIdentificacaoUrnaContingencia` (5888) tinha uma verificação
  de tipo em `cidentificacaournacontingencia.cpp:32` ("Tipo inválido para urna de contingência: {}"). Todo chamador passa
  `'2'`, então a verificação e o seu throw desapareceram. Restam apenas um registro `std::source_location` **não referenciado**, uma
  string não referenciada e um frame de pilha de 480 bytes não usado. Isso também explica uma falsa anotação de "string" em
  duas outras funções (§6.4).
* **Plural cortando um caractere** (§3.7): a linha de total do Boletim de Justificativa, `"{:05} Justificativas"`,
  vira singular por `pop_back()` na string de formato quando a contagem é 1 (11474).
* A **tabela de nomes de arquivos do SAVD** (§3.5, 96 nomes, ids 25..120) que as mensagens de erro usam quando a assinatura falha, ex.:
  83 = "RDV MI", 110 = "UENUXDB INT".

Glossário: *MT* (terminal do mesário) o microterminal do mesário; *seções agregadas* seções pequenas que votam na urna
de outra seção; *urna de contingência* urna reserva que substitui uma com defeito; *cargo* (Prefeito, Vereador...); *eleição*
uma das eleições do *pleito* (o conjunto de eleições do dia); *carga* a carga de software e de dados da eleição na
urna (com um *código de carga*); *BU (boletim de urna)* o resultado por urna; *BUJ* boletim de justificativa;
*RDV (registro digital do voto)* a tabela embaralhada de votos; *zerésima* o relatório zerado impresso antes da votação; *MI /
ME (MV)* memória interna (flash interna, `/dsk/fi`) / memória externa ou de votação (o cartão removível, `/dsk/fe`);
*SAVD* o daemon de assinatura da urna; *CEPESC* o esquema de cifração do TSE; *DS* data source, fonte de dados (uma função que produz
o texto de um campo de tela/relatório no momento do desenho).

## 1. O que a unidade contém

| tema | funções | reconstrução | § |
|---|---|---|---|
| identificação do local de votação e da urna | 3752, 3753, 5744, 5726, 5888 | `dados/clocal.u36.cpp`, `nomearquivo/cnomearquivo.u36.cpp`, `md/cidentificacaournacontingencia.{h,cpp}` (arquivo novo) | 3.1 |
| cargos, votos, contadores | 3774, 3782, 3784, 5653, 11315, 11531, 3870, 5681, 5717-5719 | `dados/cconfiguracaoeleicao.u36.cpp`, `dados/ccargos.u36.cpp`, `…/celeicaope.u36.cpp`, `…/rdv/cvoto.u36.cpp` | 3.2 |
| arquivo do BU e QR code do BU | 5861, 3821, 5168, 5622, 5624, 10274 | `gravadores/md/cenvelopegenerico.u36.cpp`, `ecourna/…/cplaintext.u36.cpp`, `relatorios/ccabecalhoqrcodebuilder.u36.cpp` | 3.3, 4 |
| QR code do estado da urna | 5636, 5783 | `relatorios/cqrcodeds.cpp` (caminho inferido) | 3.4 |
| assinaturas SAVD | 4668, 5905 | `vota/eleitor/comum/cinformacaoeleitor.u36.cpp`, `iinterfacesavd.u36.cpp` | 3.5 |
| registro de mesários (MT) | 5383, 6007, 6008, 6009, 6010, 11140, 19 stubs de atexit | `comparecimentomesario/cregistradormesario.u36.cpp` + arquivos donos (u22) | 3.6 |
| fontes de texto de telas / relatórios | 11474, 11499, 11501 | `justificativa/cjustificador.u36.cpp`, `dados/cpartidos.u36.cpp` | 3.7 |
| impressões digitais (biometria) | 3719, 3794, 3898, 5816, 5817, 10519-10522 | `…/cbiometriaeleitor.u36.cpp`, `cpath.u36.cpp` | 3.8 |
| outros corpos mesclados e pequenos membros | 3894, 3896, 6030, 6043, 6044, 10877, 5552, 5556, 5760, 3834, 6048, 3510, 3512 | `u36-foreign-fragments.cpp`, `api/io/cinisection.u36.cpp`, `api/gui/capplicationcontextstack.u36.cpp` | 3.9 |
| destrutores de armazenamento estático | 34 funções (10274 … 11660) | listados em `u36-foreign-fragments.cpp` §C1 | 6.1 |
| biblioteca C++ | 3368, 4778, 4779, 8097, 8099, 9404, 9873, 5956, 5763, 7765-7776 | listados em `u36-foreign-fragments.cpp` §C2 | 3.10 |

**Fontes reconstruídas escritas por esta unidade** (todas sob `src/`; os `*.u36.cpp` são fragmentos de arquivos pertencentes a outras
unidades, a serem incorporados a eles):

```
src/uenux2/src/app/comum/u36-foreign-fragments.cpp                 merged bodies, implicit members, atexit + library lists
src/uenux2/src/app/comum/dados/clocal.u36.cpp                      CLocal::GetAgregadas (3752), GetUFMinuscula (3753)
src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u36.cpp        CConfiguracaoEleicao::GetEleicoesCargos (3774)
src/uenux2/src/app/comum/dados/ccargos.u36.cpp                     CCargos::LimpaFiltro (3784)
src/uenux2/src/app/comum/dados/cpartidos.u36.cpp                   CPartidosDSNumero/DSSigla::Text (11499/11501)  (path inferred)
src/uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.u36.cpp  CBiometriaEleitor::PossuiDedo (3719)
src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.u36.cpp  CEleicaoPE::PossuiCargoEletivo (5653)
src/uenux2/src/app/comum/dados/md/rdv/cvoto.u36.cpp                CVoto::EhBranco (11315)
src/uenux2/src/app/comum/md/cidentificacaournacontingencia.{h,cpp} CIdentificacaoUrnaContingencia (5888) - new file
src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u36.cpp          SIdentificacaoCarga ctor (5726)
src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp   CEnvelopeGenerico ctors (5861, 3821)
src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp  builder ctor (5624), SetHistoricoCargas (5622)
src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp                  urna-state QR fields (5636, 5783)  (path inferred) - new file
src/uenux2/src/app/comum/iinterfacesavd.u36.cpp                    NomeArquivoSavd (5905) + the 96-name table
src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u36.cpp  QuantidadeRegistrados (6007)  (path inferred)
src/uenux2/src/app/comum/justificativa/cjustificador.u36.cpp       CJustificadorDadoQuantidade::Text (11474)
src/uenux2/src/app/comum/cpath.u36.cpp                             external WSQ directories (3898, 3794, 5816, 5817)
src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u36.cpp   AssinaDadosDinamicos (4668)
src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp            CApplicationContext operator= / operator== (5552, 5556)
src/uenux2/src/api/io/cinisection.u36.cpp                          CIniSection::Find (10877)
src/ecourna/api/security/cepesc/cplaintext.u36.cpp                 CPlainText delegating ctor (5168)
```

Sete funções da unidade já tinham sido escritas, com o mesmo significado, pelas unidades donas (3510/3512 em
`celeitoridentidade.cpp`, 3782 em `ccargos.cpp`, 3834/6048 em `cpath.cpp`, 5383 em `cgestordadomesario.cpp`,
11531 em `celeitores.cpp`). A tabela de mapeamento (§9) aponta para elas.

## 2. Classes e como se relacionam

A unidade não tem slot de vtable próprio: toda função é alcançada por chamada direta, como alvo de thunk, ou por meio de um
slot da tabela de funções (comparadores, fontes de texto, handlers de atexit). As classes que ela toca, com o seu RTTI onde ele
existe:

```
comum::md::CIdentificacaoUrna                         (not polymorphic, 12 bytes: tipo, município, zona)
 ├ comum::md::CIdentificacaoSecao                       tipo '1'  (5887)
 └ comum::md::CIdentificacaoUrnaContingencia            tipo '2'  (5888, this unit)
comum::IResultado (vtable @1541028)                    +20 std::string m_nome
 └ comum::IGravador (@1553656)
     ├ comum::CGravadorBU (@1554140)                   member CDadoCorrespondencia m_correspondencia
     ├ comum::CGravadorRDV (@1556856)                   … at +56   -> destructor body 6043 (this unit)
     └ comum::IGravadorEnvelope (@1554456)              … at +76   -> destructor body 6043
comum::CAssinador (@1553260) └ vota::CAssinadorVota (@1532312)   ctor 1501, used by 4668
api::IText └ api::CDataTextFmt<const std::string (*)(const std::string&)> (@1538280)   11499, 11501
           └ api::CDataTextFmt<std::string (*)(const std::string&)> (@1539168)         11474
api::CDataMap<K, V> (not polymorphic) <- comum::CPartidos ("CPartidos"), comum::CJustificador ("CJustificador")
api::IPreShow └ api::CPreShowClearScreen (@1577896)   11140 = CriaForm<CPreShowClearScreen>
```

Registros não polimórficos cujos membros são usados aqui: `md::CLocal` / `md::CSecaoEleitoral` / `md::CIdentificacaoAgregada`
(local de votação), `md::CEstadoGeral` / `md::CDadoCarga` / `md::CDadoCorrespondencia` (`eg.bin`), `md::CEnvelopeGenerico`
(envelope de arquivo de resultado), `CCabecalhoQRCode` (33 strings `"TAG:value "`), `api::CApplicationContext` (texto da tela de erro),
`md::CEleitorDinamico`, `md::CVoto`, `md::CEleicaoPE`, `md::CCargo`.

## 3. As funções, por tema

### 3.1 Identificação do local de votação e da urna

* **`CLocal::GetAgregadas` (3752).** `CLocal` é o singleton que envolve o arquivo do local de votação `<zona><secao>-lo.dat`
  (`ModuloLocal`). Como todo acessor de `CLocal`, ele primeiro chama `VerificaLido("GetAgregadas")` (func 782). Essa chamada lança
  `"GetAgregadas: o arquivo de locais ainda não foi carregado"` se o arquivo não foi lido. Depois, se o local é uma
  seção (`CSecaoEleitoral` opcional preenchido, byte +120), copia o número de seção `uint16` de cada
  `CIdentificacaoAgregada` para um novo `vector<TSecaoID>`. Uma urna de contingência recebe um vetor vazio. Usuários: o campo `AGRE:`
  do QR code do BU (11242), `CRelUtil::FormataSecoesAgregadas` (5738) e o cabeçalho da tela da zerésima (4160). Nenhum
  cenário do simulador tem seções agregadas, e a função nunca rodou nas execuções com contadores de entrada. No
  binário, `md::CLocal::GetSecao()` é chamado três vezes. Dois dos resultados não são usados, e as chamadas só sobrevivem
  porque `GetSecao` pode lançar exceção.
* **`CLocal::GetUFMinuscula` (3753).** `ToLower(GetUF())`, com `GetUF` inlinado (`VerificaLido("GetUF")`). Fornece a
  UF em minúsculas de todo nome de arquivo de resultado, `t02410`**`ac`**`0000100010001-bu.dat`. É chamado por
  `CGravadorUtil::DeterminaNomeArquivoSemLetra` (3798) e `CGravaResultado` (12098). Ao contrário de
  `comum::(anon)::FormataUF` (3773), não verifica se a UF tem 2 letras.
* **`md::CLocal::~CLocal` (5744)** é o destrutor implícito. Libera a seção opcional (e o seu vetor de
  seções agregadas), depois as strings nome do município, nome da UF, sigla da UF e país.
* **Construtor de `SIdentificacaoCarga` (5726).** Este registro de 24 bytes contém `{fase, id do processo eleitoral, uf lower-cased
  by ToLower (1879), const CPleito*}` e identifica a carga em nomes de arquivos. `CPE` o embute em +180; ali o pleito é
  o do turno atual. `CConfiguracaoEleicao` o embute em +620; ali ele aponta para o seu próprio `m_pleito`, e é
  seguido por município, zona e as seções agregadas. O nome da struct vem de `cnomearquivo.h` (u24). O quarto
  membro é novo.
* **Construtor de `md::CIdentificacaoUrnaContingencia` (5888).** É `CIdentificacaoUrna('2', município, zona)` (5886,
  que valida tipo ∈ '1'..'4' com um teste sem sinal, município < 100000 e zona < 10000). O registro `std::source_location` @1552708 dá o arquivo,
  a linha e a assinatura completa `CIdentificacaoUrnaContingencia(const EUrnaTipo, const TMunicipioID, const TZonaID)`.
  Nenhum código referencia esse registro. Os únicos chamadores, `CConversorLocal::DoDesconverte` (11452, o ramo
  `identificacaoContingencia` do `-lo.dat`) e `CGravaResultado` (12098), passam `'2'`. Então o otimizador removeu o parâmetro `tipo` e a
  verificação da linha 32 junto com a sua mensagem "Tipo inválido para urna de contingência: {}" (@6328, também não referenciada).
  O único vestígio é um frame de pilha de 480 bytes não usado, o tamanho do buffer de `std::format` de que o throw precisava
  (os outros helpers `std::format` + throw deste build, ex.: 2919 e 6044, também têm frames de 480 bytes). O otimizador
  é o **wasm-opt**, após a linkagem, não o LLVM: veja §6.3.

### 3.2 Cargos, votos e contadores

* **`CConfiguracaoEleicao::GetEleicoesCargos` (3774, executou).** Retorna um par `{TEleicaoID, TCargoID}` (8 bytes) por
  cargo de cada eleição do pleito, na ordem do arquivo. Percorre `CConfiguracaoEleicao +52` (o
  `vector<CEleicaoPE>` do pleito, elementos de 52 bytes) e, para cada eleição, `CEleicaoPE::GetCargos()` (2257, `CCargo` de 140 bytes).
  O código de inicialização o chama duas vezes, para preencher `CCargos::m_todos` e `m_cargos` (`CCargos::CreateInst`, inlinado em 7787).
  `CEleitorVotando::GravaVotos` (4454) o chama uma vez por eleitor, para encontrar a eleição, isto é, a cédula do RDV, de cada
  voto confirmado.
* **`CCargos::LimpaFiltro` (3784) e `OrdenaPorOrdemImpressao` (3782).** Enquanto um eleitor vota, `m_cargos` contém apenas
  os cargos em que ele pode votar (`FiltraPorAbrangencia`). No fim do dia, `CGeraBU` (12110) e `CGravaResultado`
  (12098) primeiro restauram a lista completa (`m_cargos = m_todos`, 3784, que não mexe no cursor; o código de outras unidades a chama
  de `Inicio()`). Depois a ordenam na ordem de impressão (3782: `std::sort` com o comparador do slot 2358 = func 11558, depois cursor
  = 0) antes de percorrer os cargos para o BU, o arquivo do RDV e os QR codes.
* **`CEleicaoPE::PossuiCargoEletivo` (5653)** é `any_of(cargos, has CDetalheCandidato)`. É falso para uma eleição
  que só tem perguntas de referendo ("consultas"), que então não recebe arquivo de candidatos (`CNomeArquivo::MontaNomesEleicao`,
  3744). Roda na inicialização (3744 e 7787: 6 chamadas nos cenários municipais, 12 nos gerais).
* **`CVoto::EhBranco` (11315)** é `tipo == 3 || tipo == 5` (branco, branco após suspensão). É passado como
  **ponteiro para função membro** `{slot 2834, adj 0}` a `CVotos::TotalQue(bool (CVoto::*)() const)` (11340, um
  `std::function` envolvendo a chamada). `CVotosEleicoesVota::Brancos` (3747 = slot 9 da vtable de `CRdvVota`) o usa. Esse
  contador é o número de votos brancos de um cargo no BU e nos relatórios. Os seus irmãos são `EhNulo` 11316 (4, 6, 7),
  `EhLegenda` 11317 (1) e `EhNominal` 11318 (2).
* **`ComparaDinamico` (11531)** é o comparador por ponteiro de função (slot 2387) que `CEleitores::CompleteLoad` passa a
  `std::sort` para as linhas dinâmicas dos eleitores. Compara o `CEleitorIdentidade` (string do título, depois o tipo). Já
  estava escrito em `celeitores.cpp`.
* Instâncias de biblioteca: 3870 (cópia de nó de `map<TEleicaoID, string>`: as versões de pacote por eleição de `CPleito`), 5681
  (cópia de nó de `map<SCargoInfo, CVotos>`, usada pela cópia transacional de `CVotosCargos::InsereCedula`), 5717/5718/5719
  (`std::sort` de `CIdentificacaoAgregada` por número de seção em `CConversorSecaoEleitoral::DoDesconverte`).

### 3.3 Arquivo do BU, cifração e cabeçalho do QR

Estas funções são descritas na §4, passo a passo: 5861/3821 (envelope), 5168 (`CPlainText`), 5624/5622 (builder do
cabeçalho do QR e `HIQT`/`HICA`), 10274 (estado da cadeia de hashes).

### 3.4 O QR code do estado da urna (5636, 5783)

`MontaCamposQRCodeEstadoUrna(const CEstadoGeral&)` (5636, executou durante `votaInit`) retorna um
`vector<pair<string,string>>`. `CQRCodeDS::operator()` (5782) depois acrescenta os campos voláteis (`VERS`, data do pleito...)
e junta tudo como `TAG:value TAG:value …`. Campos, em ordem:

| tag | valor | origem (`eg.bin`, `ModuloEstadoGeralUrna`) | valor no simulador (fixture de `CAppInfoBuilder`, u30) |
|---|---|---|---|
| `PROC` | id do processo eleitoral | `idPE` (+4), `to_string(unsigned)` | PE do cenário |
| `TURN` | `1` se turno = '1', senão `2` | apenas quando o `TipoUrnaOperacao` do turno atual **não** é `contingencia` ('2') nem `contingenciavotarecupera` ('4') | `1` (`2` nos cenários de 2º turno) |
| `FASE` | `toupper(GetFaseChar())` | `DadoCarga.fase` → `O`/`S`/`T` | `T` (treinamento) |
| `UNFE` | UF, copiada como armazenada | +8 | definida por `votaInit` (maiúsculas) |
| `MUNI`, `ZONA` | município, zona | +20, +24 | cenário |
| `SECA` | seção | apenas quando o tipo é `vota` ('1') ou `contingenciavota` ('3') | cenário |
| `IDUE` | número interno da urna | `correspondencia.carga.numeroInternoUrna` (+60) | `87654321` |
| `MDUE` | modelo da urna | `DadoCarga.modelo` (+44), como int | `2015` |
| `IDFL` | série da flash de carga (8 hex) | +64 | `12345678` |
| `IDCA` | código da carga | +88 | `123456789012345678901234` |
| `DTCA` / `HRCA` | data da carga `YYYYMMDD` / hora `hhmmss` | +76 / +84 | `20201231` / `235958` |
| `NOME`, `SERT`, `SERI` | gerador de mídias: nome do host, serial do certificado TPM, serial da instalação | +120, +132, +144 | `nome_maquina`, `12345678`, `99999999` |
| `ASSI` | a **assinatura** da correspondência da carga, em hex maiúsculo (func 1243) | +108 | 278 dígitos hex da assinatura falsa |

`AdicionaOrigemQRCode(campos, variante)` (5783) acrescenta `ORIG:T` para a variante 0 (a tela da zerésima "antes do horário"),
`ORIG:I` para a 1 (o relatório impresso ESTADO DA URNA, `CRelatorioTesteImpressora` 11200), e nada nos outros casos (2 = a
tela "confirma impressão da zerésima"). Os significados *tela* / *impresso* são palpite nosso. Na tela, a imagem do QR é
regenerada a cada 15 s (`CImageFieldUpdate`, 3059).

### 3.5 Assinaturas SAVD

* **`AssinaDadosDinamicos` (4668, executou em `votaInit`).** Constrói um `vota::CAssinadorVota` com o pacote 122 (1º turno)
  ou 123 (caso contrário) e pede ao SAVD para assinar o arquivo de id **83 "RDV MI"** (`rdv.dat`) e o de id **110 "UENUXDB INT"**
  (`uenux.db`). `CInformacaoEleitor::GerarDadosDinamicos` (6737) a chama logo após o RDV ser criado, uma vez no
  ramo de treinamento do eleitor e uma vez no ramo normal. No build web, o cliente do SAVD é `(anonymous)::CWasmSavd`,
  que responde "OK" e não faz nada (u23). Os arquivos `.vsu` que existem são os falsos que `votaInit` grava.
* **`NomeArquivoSavd` (5905)** mapeia um id de arquivo SAVD (25..120) para um nome por meio de uma tabela `const char*` de 96 entradas @1551420.
  Outros ids dão "Arquivo não identificado". O nome vai parar no texto da tela de erro "Ocorreu um erro durante a
  assinatura do arquivo: …" (`CAssinador::Assina` 1277, `CGravaResultado` 12098). `CAssinador::Assina` constrói esse texto
  **antes** de chamar o SAVD, para o `api::CApplicationContextGuard` (676, título "Erro na assinatura") que fica na
  pilha de contextos de erro durante a assinatura. Então 5905 roda em **toda** assinatura, não só quando uma falha. Rodou 5 vezes
  em `votaInit` nas execuções com contadores de entrada. O texto só é mostrado se a assinatura falhar. A tabela completa está em
  `iinterfacesavd.u36.cpp`. Algumas entradas: 25 "EG Geral MI", 31 "EG VOTA MI", 35 "BU do VOTA MI (res)", 37 "RDV MI (res)",
  43 "BU imp. do VOTA MI (res)", 44 "ZE imp. do VOTA MI (res)", 60 "Log MI (res)", 83 "RDV MI", 88 "Zerésima MI",
  110 "UENUXDB INT", 112 "Arquivo de local". O id 120 mapeia para uma string vazia (um literal mesclado com o final de outro).

### 3.6 Registro de mesários no MT

Antes do início da votação e de novo no fim, os mesários se registram no microterminal. Eles digitam o seu título,
depois fornecem uma impressão digital (u22 reconstruiu a máquina de estados: `CRegistrarMesarios` → `CPedeTituloMesario` →
`CTituloMesarioVazio` / `CTituloMesarioInvalido` / `CTituloMesarioJaRegistrado` → `CPedeDigitalMesario` →
`CGestorDadoMesario*` → `CConfirmaFimRegistroMesarios` → `CEncerraRegistroMesarios`). Esta unidade contém as peças que o
compilador compartilhou ou gerou:

* 6007 é `CRegistradorMesario::QuantidadeRegistrados(periodo)`: conta as linhas `comparecimento_mesario` em cache cujo
  período da chave é igual ao argumento. É um corpo de merge-similar. Os thunks são `shared_f3606` (período 1) e
  `shared_f2727` (período 2). Usuários: os contadores do MT, `CControladorRegistraMesariosVota` e o relatório BIM.
* 6008 é o corpo dos dois `CNomeMesariosUrnaDS::Text` (linha do MT com o nome do mesário). Se o mesário não é eleitor
  desta seção, retorna o título bruto **sem** aplicar o formato. Caso contrário, formata os primeiros 40 **bytes**
  do nome social, ou do nome quando o nome social está vazio.
* 6009 é o corpo das duas fontes de texto de "título".
* 6010 é o corpo de `CTituloMesarioVazio::StartState` e `CTituloMesarioInvalido::StartState`. Os dois diferem apenas no
  seu srcloc, então um título **vazio** é registrado no log com a mesma chamada ao controlador (slot 26) que um inválido. No VOTA,
  esse slot é `CControladorRegistraMesariosVota::vf26` (10776), que registra "Digitado título inválido para o registro de
  mesário" (`CLoga::loga`, nível 2). Um título vazio, portanto, deixa a mesma linha de log que um errado.
* 5383 é `CGestorDadoMesario::GetInst` (corpo de singleton mesclado `vota_f764`). 11140 é
  `api::CriaForm<api::CPreShowClearScreen>`, o formulário da tela do eleitor mostrado durante o registro (§7).
* 19 handlers de atexit dos singletons de estado (§6.1): 10320/10321, 10334/10336, 10346, 10350/10351, 10354/10355, 10371,
  10378, 10381/10382, 10386/10387, 10389/10390, 10393/10394.

### 3.7 Fontes de texto de telas e relatórios

* **`CPartidosDSNumero::Text` (11499, executou) e `CPartidosDSSigla::Text` (11501)** retornam
  `std::vformat(formato, current party number / sigla)`. O partido atual é obtido de `CPartidos` (um singleton preguiçoso
  inlinado aqui: `unique_ptr` @1838928, um `CDataMap<TPartidoID, CPartido>` chamado "CPartidos", construtor 3751).
  `GetCurrent()` (1283) lança "O registro corrente estava inválido {}" quando nenhum partido está selecionado. O slot 1100 (número)
  é a tela de partido de um voto proporcional (`CTelasVota::CriaTelaPartido`, `ctelasvota.cpp:1741`, construída na inicialização):
  é mostrada enquanto o eleitor digitou apenas os dígitos do partido ("91" de "91001") ou vota apenas no partido (voto de
  legenda). O slot 1089 (sigla) é a linha "partido" da tela de candidato, acrescentada por `vota_f2028` (`adicionaPartido`)
  quando `CConfiguracaoEleicao +484` ("apresentar partido") está definido. Uma execução headless
  (`node tools/run/headless.mjs --scenario municipal-t1 --keys "91001 " --draw`) desenha `fillText("PEsp", 20, 451)` na
  tela de candidato a Vereador. "PEsp" é a sigla do partido 91 em `t02411ac00001-pa.dat` ("Partido dos Esportes" é
  o seu nome), então 11501 também roda, embora o profiler não a tenha amostrado. Os contadores de entrada dão de 8 a 20 chamadas por
  eleitor, incluindo municipal-t2, onde o único cargo é Prefeito.
* **`CJustificadorDadoQuantidade::Text` (11474)** é a linha de total do Boletim de Justificativa impresso,
  `CDataTextFmt(slot 2948, "{:05} Justificativas")`. Quando a contagem é exatamente 1, a função remove o **último
  caractere do formato**, então a saída é "00001 Justificativa". A contagem é o tamanho do mapa de `CJustificador` (+8).

### 3.8 Impressões digitais

* **`CBiometriaEleitor::PossuiDedo` (3719)** retorna `m_dedos.has_value() && m_dedos->contains(tipo)`. O byte +32 é a
  flag de preenchimento de um mapa **opcional** em +20; `cbiometriaeleitor.h` (u05) precisa ser corrigido nesse ponto (§7).
* **Diretórios WSQ externos (3898 + thunks 3794, 5816, 5817)** constroem
  `<flash externa>/dinamico/trab<turno>/wsq/{operador,nao-habilitado,habilitado}/`, onde o turno vem de `eg.bin`.
  Três funções do código-fonte diferiam apenas no literal, e o wasm-opt as mesclou. O literal `"habilitado/"` é o final
  de `"nao-habilitado/"`. Os gêmeos da flash interna são `vota_f2298` + 5 thunks (outras unidades). Usuários:
  `CGravadorWSQ::CompactaWsq` (empacota os arquivos de resultado `wsq*.jez`), `CMostraEleitorVotando::SalvaHabilitacaoEleitor`,
  `CPedeDigitalMesario`. Nenhum deles roda no simulador.

### 3.9 Outros corpos mesclados e pequenos membros

* 3894: `Clonar()` de DAO, `new T(*this)`, copia o vptr + o `shared_ptr` para a conexão SQLite. Três DAOs o usam.
* 3896: `IServicoEstado::GetPathArquivo` para `vota.bin` / `gap.bin` / `sa.bin`. Retorna
  `CPath::GetPathTrab(midia, turno) / nome`, ou lança "Urna sem turno em contexto onde turno era esperado"
  (`CUeComumAppInfoError` 7607 / 7605 / 7606, linha 32 do srcloc de cada arquivo de serviço) quando o turno é `'0'`. Roda 18
  vezes em `votaInit` (contadores de entrada), onde `trab1/` e `trab2/` recebem `vota.bin`, `gap.bin` e `sa.bin`.
* 6030: verificação de conversor de enum `unsigned(v - 1) >= n` → `CUeComumDadosError` (8184 "Tipo de identificador de eleitor
  inválido", n = 3; 7953 "Origem configuração inválida", n = 2).
* 6044 + 10877: `CVersoesContratos::GetValor` / `CDependenciasContratos::GetValor`. Procuram a chave no
  mapa de `api::CIniSection` (`Find` = 10877). Uma chave ausente lança "Propriedade inexistente: {}" (8697 em `cversoescontratos.cpp:42`, 8663 em `cdependenciascontratos.cpp:59`).
  Caso contrário, retornam o valor da chave (`CIniKey +12`). O simulador traz `/etc/*.properties` de 0 bytes, então toda busca
  lançaria exceção.
* 6043: corpo do destrutor de `CGravadorRDV` e `IGravadorEnvelope`: `~CDadoCorrespondencia` (857) no membro em +56 / +76,
  depois `IResultado::m_nome`.
* 5552 / 5556: atribuição por movimentação e `operator==` de `api::CApplicationContext` (52 bytes). `~CApplicationContextGuard`
  (675, executou) os usa para remover o seu contexto da pilha global de contextos de erro: procura a partir do topo um elemento
  igual e o apaga.
* 5760: construtor de cópia implícito de `md::CEleitorDinamico` (84 bytes). 5763: construtor de movimentação implícito do
  valor do mapa de `CEleitores`, `pair<const CEleitorIdentidade, CEleitorDetalhe>` (220 bytes; a chave const é copiada).

### 3.10 Código da biblioteca C++

* 3368, 4778, 4779, 8097, 8099: o `std::deque<std::filesystem::__dir_stream>` (elementos de 80 bytes, 51 por bloco de 4080
  bytes, alocado com `operator new(size_t, align_val_t 16)`). É a pilha de diretórios de
  `recursive_directory_iterator` (8096/8101), que o glob do ecourna (`ICompressor::Add`) usa ao empacotar arquivos WSQ.
* 9404: `vector<string>::__emplace_back_slow_path(str, pos, n)` (`CStringUtils::Split`, 1880; executou).
* 9873: destruidor de rollback por exceção de uma cópia de `vector<CDadoCorrespondencia>` (slot de tabela 473).
* 5956: `~vector<md::CMunicipioDisponivel>` (o relatório "dados disponíveis na carga").
* 7765/7768/7770/7772/7774/7776: arrays estáticos de `__time_get_c_storage` do `locale.cpp` da libc++ (`"%m/%d/%y"`, AM/PM,
  meses, dias da semana, `char` e `wchar_t`). São destrutores atexit, então também estão mortos (§6.1).

## 4. Boletim de urna: o que esta unidade contribui, passo a passo

O fluxo do BU como um todo está em `docs/10-boletim-de-urna.md` e `docs/bu/*.md`. Os passos abaixo são os que passam
por funções de u36, na ordem de execução durante o encerramento (`vota::CGeraBU::StartState` 12110 e
`vota::CGravaResultado::StartState` 12098; nenhum deles roda no simulador público, que nunca encerra a votação).

1. **Ordem dos cargos.** Antes da contagem, `CCargos::LimpaFiltro()` (3784) restaura a lista completa de pares (eleição, cargo),
   que `GetEleicoesCargos` (3774) construiu na inicialização na ordem do arquivo do pleito. Depois `OrdenaPorOrdemImpressao()` (3782) os ordena
   por `CSituacoesEleicoes.ordemImpressao` (byte +5) e depois por `CCargo.ordemImpressao` (+16), e rebobina o cursor.
   As entidades do BU, o arquivo do RDV, o BU impresso e os QR codes percorrem todos os cargos nesta ordem.
2. **Contagem dos votos brancos.** Para cada cargo, `CVotosEleicoesVota::Brancos(cargo)` conta os votos do RDV para os quais
   `CVoto::EhBranco()` (11315) é verdadeiro: tipos **3 (branco)** e **5 (branco após suspensão)**. O BU não tem contador separado
   de "branco após suspensão"; os dois tipos contam como branco (`TipoVoto branco(2)` em `ModuloBoletimUrna`). Votos
   nulos são `EhNulo` (4, 6, 7). Os tipos 8/9 (nulo para um cargo sem candidatos) são contados à parte (veja
   `docs/bu/build-a-bu.md` §4).
3. **Estado da cadeia de hashes.** `CConversorEntidadeBU::DoConverte` (10273) encadeia hashes SHA-512 das tuplas de votos em um
   `std::vector<uebyte>` estático @1910004 (estado de `CAssinaVotavelBU`, `docs/bu/build-a-bu.md` §6). O seu
   destrutor estático é a func 10274 (libera o buffer). Por causa de `EXIT_RUNTIME=0` ele nunca é registrado, então o último hash fica
   na memória até a página ser descarregada.
4. **Cifração opcional do BU** (parâmetro `criptografarBU` dos parâmetros da urna, `CConfiguracaoEleicao +160`,
   **e** `CGravadorBU +212`; `False` nos dados do simulador e nos BUs publicados). `CGravadorBU::GravaResultado`
   (11629) constrói o texto em claro com o **construtor delegante `CPlainText(zona, seção, tabela, aleatório, chave,
   conteúdo)` (5168)**, que repassa ao construtor completo (9465, `cplaintext.cpp:89..104`) com
   `tipoArquivo = 0`, `idCriptografia = 1` e nenhum `CInfoSalt`:
   * `tabela`: a tabela criptográfica de 1024 bytes da urna (`IUrna` slot 4),
   * `aleatório`: 32 bytes aleatórios (`IRng` slot 4),
   * `chave`: a chave pública do BU lida do arquivo por `LeChavePublica`,
   * `conteúdo`: os bytes DER de `EntidadeBoletimUrna`.

   O construtor completo rejeita ids fora da faixa (1517/1518) e vetores vazios (1519..1522). `CCepescCipher::Cifra` então
   produz o texto cifrado e `md::CSeguranca(0, 1, cifrado)`.
5. **O envelope** (`ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico`, arquivo `<fase><pleito:05><uf><mun:05><zona:04>
   <secao:04>-bu.dat`, ex.: `t02410ac0000100010001-bu.dat`; a `uf` em minúsculas vem de `CLocal::GetUFMinuscula`,
   3753). `CGravadorBU` o constrói com um dos dois construtores desta unidade:
   * **em claro** `CEnvelopeGenerico(cabecalho, fase, município, zona, local, seção, tipo, conteúdo, tipoUrna)` (5861):
     `urna` ausente, `seguranca` ausente, `conteudo` = os bytes DER do BU;
   * **cifrado** `CEnvelopeGenerico(…, tipo, seguranca, conteúdo, tipoUrna)` (3821): `seguranca` presente,
     `conteudo` = o texto cifrado pelo CEPESC.

   Ambos copiam o `CCabecalhoEntidade` de 20 bytes (dataGeracao + idPleito), os campos e o conteúdo, depois chamam
   `ValidaCriacao()` (3822). Essa chamada rejeita fase == '0' ou fase ≥ '4' (8678, teste com sinal: um valor abaixo de '0'
   passaria), município ≥ 100000 (8679), zona ≥ 10000 (8680), um local preenchido ≥ 10000 (8681), **tipoUrna ∉ '1'..'4'**
   (8682: `(tipoUrna - '5') ≤ -5` compilado como `i32.le_u`, portanto é um teste de faixa sem sinal e rejeita também valores acima
   de '4'), seção ≥ 10000 (8683) e tipo de envelope ≥ 5 (8684, com sinal). Por fim, **o local é
   descartado a menos que `tipoUrna` seja '1' (seção), '3' ou '4'** (compilado como `(unsigned)(tipoUrna - '1') > 3 || tipoUrna ==
   '2'`; após `ValidaCriacao` só '2' pode chegar ao reset), isto é, uma urna de contingência pura ('2') grava uma
   `identificacaoContingencia` sem local. `CConversorEnvelopeGenerico` (10271) codifica o resultado. O tipo de envelope 0 é
   `envelopeBoletimUrna (1)`.
6. **Os QR codes do BU** (BU impresso: partes de ≤ 1100 caracteres; "BU digital" na tela: ≤ 2500; menos a última
   parte, que pode chegar a 1245 (Ed521) ou 1259 (ECDSA) caracteres impressa e a cerca de 2659 na tela, porque no formato
   6.0 sua parte fixa ocupa 422–436 caracteres, mais que os 277 reservados; dados das urnas de 2026:
   `investigation/LEIAME.md`, achado H5). O cabeçalho é
   construído com `CCabecalhoQRCodeBuilder`, cujo construtor (5624) preenche com zeros as 33 strings do cabeçalho. `SetHistoricoCargas`
   (5622) preenche o campo `HistoricoCarga` (+168) com **ambas** as tags:

   ```
   HIQT:<n>                       n = number of carga codes in gap.bin's correspondence history (size_t)
   HICA:<i>:<código, max 24 chars>   repeated n times, i = 1..n
   ```

   Cada item é seguido de um espaço (`"HIQT:{} "`, `"HICA:{}:{:.24s} "`, strings @440013/@440426). `preBuild()` depois
   exige que o campo não esteja vazio (`EUeComumRelatoriosError` 9050, "Campo (HistoricoCarga) não informado."), e
   `HIQT:0 ` satisfaz isso. `docs/bu/qrcode.md` dá o payload inteiro e o hash/assinatura da cadeia de QR.
7. **Assinatura** (`CAssinador::Assina` → SAVD, simulado no build web). Antes de cada assinatura, o texto do contexto de erro
   nomeia o arquivo por meio de `NomeArquivoSavd` (5905); ele só é mostrado em caso de falha: 35 "BU do VOTA MI (res)", 63 "BU do VOTA ME (res)", 43/66 BU impresso, 37/65 RDV...
8. **Destruição do gravador do RDV.** `CGravadorRDV` e `IGravadorEnvelope` (o gravador de `imgbu.dat` / `imgze.dat`) são destruídos
   por meio do corpo compartilhado 6043. Ele libera a cópia que eles têm da correspondência da carga (`CDadoCorrespondencia`, 96 bytes) e
   o nome do arquivo.

Não está nesta unidade, mas é relacionado: o QR code do estado da urna (§3.4) **não** é o QR code do BU. Ele identifica a urna e a sua
carga (com a assinatura da carga em `ASSI`) nas telas da zerésima e no relatório ESTADO DA URNA.

## 5. Especificidades do build web

* Os contadores de entrada (veja a introdução) mostram que `votaInit` roda 3512, 3774, 3870, 3896, 4668, 5556, 5636, 5653, 5726,
  5744, 5763, 5783, 5905, 6030, 6048 e 9404. Enquanto o eleitor vota, 11499 (número do partido) e 11501 (sigla do partido)
  produzem texto de tela, 5556 roda mais uma vez (guard de contexto de erro), e 3774 e 5681 rodam quando os votos são armazenados
  (GravaVotos). As outras 82 funções pertencem ao encerramento, ao registro de mesários no MT, aos caminhos de
  impressão digital, a seções agregadas, ao tratamento de erros ou ao atexit, e nenhuma delas rodou nos cinco cenários.
* 4668 assina por meio de `CWasmSavd`, um no-op que sempre responde OK: nenhuma assinatura é feita.
* A fixture de eg.bin do build web (`CAppInfoBuilder`, u30) é o que o QR code do estado da urna (5636) mostra:
  urna 87654321, modelo 2015, flash 12345678, carga `123456789012345678901234` de 31/12/2020 23:59:58, gerador
  `nome_maquina` / `12345678` / `99999999`, e uma assinatura falsa de 139 bytes.
* `/etc/versoes.properties` e `/etc/dependencias.properties` são arquivos de 0 bytes, então 6044 lançaria "Propriedade
  inexistente" em qualquer busca (só no encerramento).

## 6. Observações sobre wasm / Emscripten

### 6.1 Destrutores estáticos que nunca podem rodar (40 funções)

O build usa `EXIT_RUNTIME=0` (`docs/libraries/libc-and-emscripten-runtime.md`): `__cxa_atexit` é um no-op e nenhum
registro sobrevive em `__wasm_call_ctors` nem nas funções `GetInst()` preguiçosas. Os destrutores dos estáticos ainda
existem porque os seus endereços foram tomados antes de o wasm-opt rodar, e a tabela de funções os mantém (slots 2140..8600).
Verificação: nenhuma instrução do módulo carrega como constante qualquer um dos slots de tabela desses handlers (ex.: 4476 para 10274, 4388 para
10320, 2914 para 11268, 8600 para 7765; os poucos acertos de `i32.const` para 2158/2159/4090 são endereços de strings e máscaras de bits
em 706, 2919, 2056 e 11390). Os handlers também estão ausentes de todas as execuções com contadores de entrada. A
unidade tem 34 deles para estáticos da aplicação (lista com endereços e donos em `u36-foreign-fragments.cpp` §C1) e 6
para arrays de `<locale>` da libc++. Um destrutor de `std::mutex` aparece como uma chamada da func 150 (o resíduo no-op de pthread) sobre o
endereço do mutex. Um destrutor de `unique_ptr` é um reset por meio de `shared_f349`, `shared_f3886` ou `unknown_f763` (este último
com um delete virtual). Os singletons dos estados de mesário, `CLogComum`, `CArquivosSavd`, `CArquivosResultado`,
`CRespostas`, `CSubstituidorTitulo`, as strings de raiz de `CPath`, o vetor da cadeia de hashes do BU, o cache do certificado da urna
(`CEstadoGeral::RecuperarCertificado`) e os dois singletons auxiliares de biometria, portanto, nunca são destruídos. Isso é
inofensivo em uma página que é recarregada entre eleitores.

### 6.2 merge-similar-functions

Dez corpos da unidade foram produzidos pelo "merge similar functions" do wasm-opt: 3894, 3896, 3898, 6007, 6008, 6009, 6010,
6030, 6043, 6044 (mais 6117 por trás de 11140). Em cada caso, duas ou três funções do código-fonte diferiam apenas em constantes, e
o passo moveu essas constantes para parâmetros extras: um endereço de registro `std::source_location` (então o srcloc nomeia o
arquivo do thunk, não o do corpo), um código de erro, uma vtable, um literal `string_view` como `(begin, end)`, um período numérico. Os
thunks mantêm os nomes reais. As ferramentas nomearam os corpos mesclados como `comum_fNNNN`, ou `shared_fNNNN` para os thunks chamados
a partir de vários componentes.

### 6.3 A propagação de constantes removeu uma verificação (5888)

O construtor de `CIdentificacaoUrnaContingencia` tinha uma verificação de `tipo` na linha 32. Todos os pontos de chamada passam `'2'`, então a verificação
é sempre falsa, e ela foi apagada junto com a sua chamada a `std::format`. O que sobra: o construtor perdeu o seu primeiro
parâmetro, um frame de pilha de 480 bytes é alocado e nunca usado, e o registro de srcloc @1552708 e a mensagem @6328 não têm
nenhuma referência. O comportamento não muda.

As sobras mostram que foi o **wasm-opt** que fez isso após a linkagem, não o LLVM. Os dois chamadores estão em outras unidades de tradução
(`comum/dados/asn`, `vota`), então o LLVM sem LTO não poderia conhecer o argumento. O LLVM também teria descartado o frame morto de
480 bytes, e `wasm-ld --gc-sections` teria descartado os dados somente leitura não referenciados. A eliminação de argumentos mortos
do wasm-opt viu a mesma constante `50` em todo ponto de chamada, removeu o parâmetro e o propagou, e
o ramo foi eliminado. O wasm-opt não reescreve o prólogo da pilha sombra nem os segmentos de dados, então ambos sobrevivem.

### 6.4 Artefatos das ferramentas encontrados durante a leitura da unidade

* O anotador imprimiu `str 6328 'Tipo inválido para urna de contingência: {}'` em `comum_f5168` e
  `CGravadorRCSecao::LeChavePublica` (11616). Ali 6328 é o **slot de tabela** de `CPlainText::CPlainText` (9465), passado
  a `invoke_iiiiiiiiiii`. O comentário inline `632 /* &… */8 /* "…" */` mostra a divisão. Os dois valores coincidem por acaso.
* Os chamadores antes mostrados como `ecourna::api::security::CHKDFSeed::GetSeed` são a func 7787, a rotina de inicialização
  (`vota::CInformacaoEleitor::Inicializar`, u02; as ferramentas agora mostram esse nome). `api::CImageFieldUpdate::CImageFieldUpdate` como chamador de 5636/5783 é
  a func 3059, `adicionaBlocoMensagem` das telas da zerésima (u15). `api::CPolySingletonList::instance@1956` é
  `vota::GetQRDSInst` (u19).
* Os cinco grupos "free functions #2..#7" de `unit.py` são apenas agrupamentos por endereço, não classes.

## 7. Correções às reconstruções de outras unidades

* `cbiometriaeleitor.h` (u05): `m_dedos` é `std::optional<std::map<CDedo::TipoDedo, CDedo>>` (flag de preenchimento +32),
  não um mapa simples mais uma flag.
* `capplicationcontextstack.u15.h`: o membro em +48 de `CApplicationContext` é armazenado, copiado e comparado como um
  inteiro de 4 bytes (`a[12]:int = f` em 5557; `i32.eq` em 5556), não como um `bool`.
* `cregistrarmesarios.cpp` (u22) escreve `b.CriaFormInterativo("", std::make_shared<api::CPreShowClearScreen>())` para a
  func 11140. A função é o não interativo `api::CriaForm<api::CPreShowClearScreen>(builder, nome)`. Ela aloca
  o pre-show com `new` em um `shared_ptr` (bloco de controle `__shared_ptr_pointer`, não `make_shared`) e envolve um
  `api::IForm<IScreen>` simples (72 bytes, func 5550).
* A func 3784 é referida como `CCargos::Inicio()` em `cgerabu.cpp`, `cgeradorbu.cpp` e `cgravaresultado.cpp`. Ela
  copia `m_todos` para `m_cargos` e não rebobina o cursor, então esta unidade a nomeia `LimpaFiltro()`.
* A func 1277, usada por `AssinaDadosDinamicos`, é `CAssinador::Assina(ESavdArquivoUE)` (como u23 escreveu). O nome do analisador,
  `comum::AssinarUE`, é a função chamada inlinada.
* `gravadores/md/cenvelopegenerico.cpp` (u23), `CEnvelopeGenerico::ValidaCriacao` (3822): o teste do tipo de urna está escrito
  `if (static_cast<int>(m_tipoUrna) <= '0')`. O wasm é `i32.load offset=220; i32.const 53; i32.sub; i32.const -5;
  i32.le_u`, um teste **sem sinal** que lança 8682 para todo valor fora de '1'..'4' (forma no código-fonte:
  `m_tipoUrna < EUrnaTipo{'1'} || m_tipoUrna > EUrnaTipo{'4'}`). O mesmo idioma sem sinal (`i32.gt_u`) valida o
  tipo em `CIdentificacaoUrna::CIdentificacaoUrna` (5886). Os outros testes dessa função estão corretos (fase: `== '0' ||
  >= '4'` com sinal; tipo de envelope: `>= 5` com sinal).

## 8. Código suspeito ou digno de nota (resumo)

| # | onde | o quê | impacto |
|---|---|---|---|
| 1 | 11474 | plural feito por `pop_back()` na string de formato quando a contagem == 1. Não há verificação de vazio, então um formato vazio seria UB (grava um byte antes do buffer), e um formato cujo último caractere não seja o `s` do plural o perderia | apenas o texto do BUJ impresso; o formato é a constante `"{:05} Justificativas"`, então o caso de UB não pode acontecer hoje |
| 2 | 5861 / 3821 | `ValidaCriacao()` roda **antes** de o local ser descartado para uma urna de contingência ('2'), então um local irrelevante fora da faixa ainda aborta o envelope (8681). Confirmado na ordem do código dos dois construtores. Não verificamos se algum chamador passa um local preenchido para uma urna '2' | urnas de contingência na máquina real; não o simulador |
| 3 | 6010 | título vazio e título inválido compartilham o mesmo corpo e a mesma chamada de log (slot 26 = `CControladorRegistraMesariosVota::vf26`, "Digitado título inválido para o registro de mesário") | precisão do log do registro de mesários |
| 4 | 6008 | nome do mesário cortado em 40 **bytes** (`substr`), o que pode partir um caractere UTF-8; o fallback (não eleitor da seção) retorna o título sem aplicar o formato | apenas o display do MT |
| 5 | 5636 | o QR code do estado da urna publica a assinatura da carga (hex) e o nome do host / serial do certificado TPM / serial da instalação do gerador de mídias | por projeto (mostrado na tela e impresso); valores de fixture no simulador |
| 6 | 4668 | as "assinaturas" do RDV e do banco de dados vão para o `CWasmSavd` no-op | simulador: segurança simulada (conhecido) |
| 7 | 5888 | validação apagada pela propagação de constantes; srcloc e mensagem órfãos | nenhum (comportamento inalterado); explica uma anotação enganosa da ferramenta |
| 8 | 34 + 6 stubs de atexit | destrutores estáticos nunca registrados (`EXIT_RUNTIME=0`) | nenhum no navegador |
| 9 | 5905 | o id 120 tem nome vazio (entrada da tabela @1551800 → 450187, o NUL de `" \t"`), então um erro sobre esse arquivo imprimiria "…arquivo: " seguido de nada. O nome é calculado antes de toda assinatura (5 vezes em `votaInit`), não só em caso de erro | cosmético |

## 9. Tabela de mapeamento completa (101 funções)

`ran` = executada: `yes` = vista pelo profiler por amostragem durante os votos gravados, `yes*` = não amostrada, mas contada pelas execuções com contadores de entrada da introdução (votaInit + um eleitor, cinco cenários). Os caminhos são os caminhos originais. "(u05)" etc. significa que a unidade
dona já escreveu a função naquele arquivo. `library` = código da biblioteca C++, não reconstruído (listado em
`src/uenux2/src/app/comum/u36-foreign-fragments.cpp`).

| idx | tamanho | ran | símbolo reconstruído | arquivo original | reconstrução | conf. |
|---:|---:|:-:|---|---|---|---|
| 3368 | 374 |  | `std::__split_buffer<std::filesystem::__dir_stream*, std::allocator<std::filesystem::__dir_stream*>>::push_back` | libcxx/include/__split_buffer | library (listado em src/uenux2/src/app/comum/u36-foreign-fragments.cpp) | média |
| 3510 | 167 |  | `comum::md::(anonymous namespace)::RemoveZerosEsquerda` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (u05) | alta |
| 3512 | 59 | yes* | `comum::md::(anonymous namespace)::PreencheZerosEsquerda` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (u05) | alta |
| 3719 | 101 |  | `comum::md::CBiometriaEleitor::PossuiDedo` | uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp | src/uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.u36.cpp | média |
| 3752 | 861 |  | `comum::CLocal::GetAgregadas` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u36.cpp | alta |
| 3753 | 106 |  | `comum::CLocal::GetUFMinuscula` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u36.cpp | média |
| 3774 | 389 | yes | `comum::CConfiguracaoEleicao::GetEleicoesCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/dados/cconfiguracaoeleicao.u36.cpp | média |
| 3782 | 86 |  | `comum::CCargos::OrdenaPorOrdemImpressao` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/ccargos.cpp (u04) | média |
| 3784 | 34 |  | `comum::CCargos::LimpaFiltro` | uenux2/src/app/comum/dados/ccargos.cpp | src/uenux2/src/app/comum/dados/ccargos.u36.cpp | média |
| 3794 | 15 |  | `comum::CPath::GetPathWsqOperadorExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | baixa |
| 3821 | 399 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp | alta |
| 3834 | 64 |  | `comum::CPath::GetRaiz` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.cpp (u22) | média |
| 3870 | 217 | yes | `std::map<unsigned int, std::string>::__emplace_hint_unique_key_args` | libcxx/include/__tree | library (listado em u36-foreign-fragments.cpp) | média |
| 3894 | 58 |  | `comum::dao::CComparecimentoMesarioDAO::Clonar [merged body: +CJustificadorDAO, CEleitorDinamicoDAO]` | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | src/uenux2/src/app/comum/{comparecimentomesario,justificativa,dados}/dao/*.cpp (u20/u24/u05) + u36-foreign-fragments.cpp | alta |
| 3896 | 223 | yes* | `comum::CServicoEstadoGeralVota::GetPathArquivo [merged body: +Gap, SA]` | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral*.cpp (u20) + u36-foreign-fragments.cpp | alta |
| 3898 | 182 |  | `comum::CPath::GetPathWsqExterno [merged body]` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | baixa |
| 4668 | 66 | yes | `vota::(anonymous namespace)::AssinaDadosDinamicos` | uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp | src/uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.u36.cpp | baixa |
| 4778 | 34 |  | `std::unique_ptr<std::filesystem::__dir_stream, std::__allocator_destructor<std::allocator<std::filesystem::__dir_stream>>>::~unique_ptr` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | média |
| 4779 | 48 |  | `std::deque<std::filesystem::__dir_stream>::__back_spare` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | média |
| 5168 | 459 |  | `ecourna::api::cepesc::CPlainText::CPlainText` | ecourna-lib/ecourna/api/security/cepesc/cplaintext.cpp | src/ecourna/api/security/cepesc/cplaintext.u36.cpp | média |
| 5383 | 22 |  | `comum::CGestorDadoMesario::GetInst` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp (u22) | alta |
| 5552 | 349 |  | `api::CApplicationContext::operator=` | uenux2/src/api/gui/capplicationcontextstack.h (header inferido) | src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp | média |
| 5556 | 387 | yes | `api::CApplicationContext::operator==` | uenux2/src/api/gui/capplicationcontextstack.h (header inferido) | src/uenux2/src/api/gui/capplicationcontextstack.u36.cpp | média |
| 5622 | 942 |  | `comum::CCabecalhoQRCodeBuilder::SetHistoricoCargas` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp | média |
| 5624 | 14 |  | `comum::CCabecalhoQRCodeBuilder::CCabecalhoQRCodeBuilder` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.u36.cpp | média |
| 5636 | 3903 | yes | `comum::MontaCamposQRCodeEstadoUrna` | uenux2/src/app/comum/relatorios/cqrcodeds.cpp (caminho inferido) | src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp | média |
| 5653 | 75 | yes* | `comum::md::CEleicaoPE::PossuiCargoEletivo` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.u36.cpp | média |
| 5681 | 350 | yes* | `std::map<comum::md::SCargoInfo, comum::md::CVotos>::__emplace_hint_unique_key_args` | libcxx/include/__tree | library (u36-foreign-fragments.cpp) | média |
| 5717 | 745 |  | `std::__sort5<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | média |
| 5718 | 1427 |  | `std::__insertion_sort_incomplete<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | média |
| 5719 | 4179 |  | `std::__introsort<std::_ClassicAlgPolicy, (lambda)&, comum::md::CIdentificacaoAgregada*, false>` | libcxx/include/__algorithm/sort.h | library (u36-foreign-fragments.cpp) | média |
| 5726 | 35 | yes* | `comum::SIdentificacaoCarga::SIdentificacaoCarga` | uenux2/src/app/comum/nomearquivo/cnomearquivo.h (header inferido) | src/uenux2/src/app/comum/nomearquivo/cnomearquivo.u36.cpp | baixa |
| 5744 | 147 | yes* | `comum::md::CLocal::~CLocal` | uenux2/src/app/comum/dados/md/clocal.cpp | u36-foreign-fragments.cpp (destrutor implícito) | alta |
| 5760 | 277 |  | `comum::md::CEleitorDinamico::CEleitorDinamico` | uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.h (header inferido) | u36-foreign-fragments.cpp (construtor de cópia implícito) | alta |
| 5763 | 695 | yes* | `std::pair<const comum::md::CEleitorIdentidade, comum::CEleitorDetalhe>::pair` | libcxx/include/__utility/pair.h | library (u36-foreign-fragments.cpp) | média |
| 5783 | 238 | yes* | `comum::AdicionaOrigemQRCode` | uenux2/src/app/comum/relatorios/cqrcodeds.cpp (caminho inferido) | src/uenux2/src/app/comum/relatorios/cqrcodeds.cpp | média |
| 5816 | 15 |  | `comum::CPath::GetPathWsqNaoHabilitadoExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | baixa |
| 5817 | 15 |  | `comum::CPath::GetPathWsqHabilitadoExterno` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.u36.cpp | baixa |
| 5861 | 289 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.u36.cpp | alta |
| 5888 | 33 |  | `comum::md::CIdentificacaoUrnaContingencia::CIdentificacaoUrnaContingencia` | uenux2/src/app/comum/md/cidentificacaournacontingencia.cpp | src/uenux2/src/app/comum/md/cidentificacaournacontingencia.cpp | alta |
| 5905 | 47 | yes* | `comum::NomeArquivoSavd` | uenux2/src/app/comum/iinterfacesavd.cpp | src/uenux2/src/app/comum/iinterfacesavd.u36.cpp | média |
| 5956 | 230 |  | `std::vector<comum::md::CMunicipioDisponivel>::__destroy_vector::operator()` | libcxx/include/__vector/vector.h | library (u36-foreign-fragments.cpp) | média |
| 6007 | 99 |  | `comum::CRegistradorMesario::QuantidadeRegistrados` | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (caminho inferido) | src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u36.cpp | média |
| 6008 | 1045 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text [merged body]` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/{cpededigitalmesario,cdigitalmesarionaoreconhecida}.cpp (u22) + u36-foreign-fragments.cpp | alta |
| 6009 | 603 |  | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text [merged body: +CTituloMesarioJaRegistradoDS]` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesario{invalido,jaregistrado}.cpp (u22) + u36-foreign-fragments.cpp | alta |
| 6010 | 102 |  | `comum::CTituloMesarioInvalido::StartState [merged body: +CTituloMesarioVazio]` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesario{invalido,vazio}.cpp (u22) + u36-foreign-fragments.cpp | alta |
| 6030 | 69 | yes* | `comum::asn::CConversorTipoIdentificadorEleitor::DoDesconverte [merged body: +CConversorOrigemConfiguracao]` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversor{tipoidentificadoreleitor,origemconfiguracao}.cpp (u03) + u36-foreign-fragments.cpp | alta |
| 6043 | 55 |  | `comum::CGravadorRDV::~CGravadorRDV [merged body: +IGravadorEnvelope]` | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (caminho inferido) | u36-foreign-fragments.cpp (corpo de destrutor mesclado) | alta |
| 6044 | 571 |  | `comum::md::CVersoesContratos::GetValor [merged body: +CDependenciasContratos]` | uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp | src/uenux2/src/app/comum/gravadores/md/c{versoes,dependencias}contratos.cpp (u23) + u36-foreign-fragments.cpp | alta |
| 6048 | 335 | yes* | `comum::CPath::GetPathRoot` | uenux2/src/app/comum/cpath.cpp | src/uenux2/src/app/comum/cpath.cpp (u22) | média |
| 7765 | 11 |  | `std::__time_get_c_storage<char>::__x()::s (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 7768 | 30 |  | `std::__time_get_c_storage<wchar_t>::__am_pm()::am_pm (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 7770 | 30 |  | `std::__time_get_c_storage<char>::__am_pm()::am_pm (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 7772 | 30 |  | `std::__time_get_c_storage<wchar_t>::__months()::months (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 7774 | 30 |  | `std::__time_get_c_storage<char>::__months()::months (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 7776 | 30 |  | `std::__time_get_c_storage<wchar_t>::__weeks()::weeks (atexit dtor)` | libcxx/src/locale.cpp | library (u36-foreign-fragments.cpp) | alta |
| 8097 | 107 |  | `std::deque<std::filesystem::__dir_stream>::pop_back` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | média |
| 8099 | 1118 |  | `std::deque<std::filesystem::__dir_stream>::push_back` | libcxx/include/deque | library (u36-foreign-fragments.cpp) | média |
| 9404 | 608 | yes | `std::vector<std::string>::__emplace_back_slow_path<const std::string&, std::size_t&, std::size_t&>` | libcxx/include/__vector/vector.h | library (u36-foreign-fragments.cpp) | média |
| 9873 | 54 |  | `std::_AllocatorDestroyRangeReverse<std::allocator<comum::md::estadoaplicacao::CDadoCorrespondencia>, comum::md::estadoaplicacao::CDadoCorrespondencia*>::operator()` | libcxx/include/__memory/uninitialized_algorithms.h | library (u36-foreign-fragments.cpp) | média |
| 10274 | 39 |  | `comum::asn::(anonymous namespace)::s_hashAnterior (atexit dtor)` | uenux2/src/app/comum/gravadores/asn/cassinavotavelbu.cpp | u36-foreign-fragments.cpp (lista de atexit) | média (objeto certo: vetor @1910004 de 10273; o nome da variável foi inventado por u23) |
| 10320 | 10 |  | `comum::CPedeDigitalMesario::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10321 | 12 |  | `comum::CPedeDigitalMesario::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10334 | 10 |  | `comum::CGestorDadoMesarioInicial::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10336 | 12 |  | `comum::CGestorDadoMesarioInicial::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10346 | 12 |  | `comum::CTituloMesarioJaRegistrado::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10350 | 10 |  | `comum::CTituloMesarioInvalido::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10351 | 12 |  | `comum::CTituloMesarioInvalido::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10354 | 10 |  | `comum::CTituloMesarioVazio::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10355 | 12 |  | `comum::CTituloMesarioVazio::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10371 | 10 |  | `comum::CPedeTituloMesarioFinal::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesariofinal.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10378 | 10 |  | `comum::CPedeTituloMesarioInicial::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesarioinicial.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10381 | 10 |  | `comum::CEncerraRegistroMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10382 | 12 |  | `comum::CEncerraRegistroMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10386 | 10 |  | `comum::CConfirmaFimRegistroMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10387 | 12 |  | `comum::CConfirmaFimRegistroMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10389 | 10 |  | `comum::CPedeTituloMesario::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10390 | 12 |  | `comum::CPedeTituloMesario::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10393 | 10 |  | `comum::CRegistrarMesarios::GetInst()::mutex (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10394 | 12 |  | `comum::CRegistrarMesarios::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 10519 | 10 |  | `ConfiguracaoExtrator::GetInst()::mutex (atexit dtor)` | (arquivo desconhecido: singleton de configuração do extrator de impressões digitais, vota_f1903) | u36-foreign-fragments.cpp (lista de atexit) | baixa (objeto certo; o nome da classe é um substituto, nenhuma string ou RTTI o fornece) |
| 10520 | 12 |  | `ConfiguracaoExtrator::GetInst()::s_inst (atexit dtor)` | (arquivo desconhecido: singleton de configuração do extrator de impressões digitais, vota_f1903) | u36-foreign-fragments.cpp (lista de atexit) | baixa (objeto certo; o nome da classe é um substituto, nenhuma string ou RTTI o fornece) |
| 10521 | 10 |  | `CodificadorWsq::GetInst()::mutex (atexit dtor)` | (arquivo desconhecido: singleton do codificador WSQ, comum_f2742) | u36-foreign-fragments.cpp (lista de atexit) | baixa (objeto certo; o nome da classe é um substituto, nenhuma string ou RTTI o fornece) |
| 10522 | 12 |  | `CodificadorWsq::GetInst()::s_inst (atexit dtor)` | (arquivo desconhecido: singleton do codificador WSQ, comum_f2742) | u36-foreign-fragments.cpp (lista de atexit) | baixa (objeto certo; o nome da classe é um substituto, nenhuma string ou RTTI o fornece) |
| 10877 | 12 |  | `api::CIniSection::Find` | uenux2/src/api/io/cinisection.cpp | src/uenux2/src/api/io/cinisection.u36.cpp | baixa |
| 11140 | 21 |  | `api::CriaForm<api::CPreShowClearScreen>` | uenux2/src/api/gui/cformbuilder.h (header inferido) | u36-foreign-fragments.cpp (instância de template); usado em comparecimentomesario/estados/cregistrarmesarios.cpp | média |
| 11174 | 10 |  | `comum::CSubstituidorTitulo::s_mutex (atexit dtor)` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11268 | 39 |  | `comum::md::estadoaplicacao::CEstadoGeral::RecuperarCertificado()::certificado (atexit dtor)` | uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11315 | 18 |  | `comum::md::CVoto::EhBranco` | uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | src/uenux2/src/app/comum/dados/md/rdv/cvoto.u36.cpp | média |
| 11474 | 695 |  | `comum::CJustificadorDadoQuantidade::Text` | uenux2/src/app/comum/justificativa/cjustificador.cpp | src/uenux2/src/app/comum/justificativa/cjustificador.u36.cpp | baixa |
| 11480 | 10 |  | `comum::CRespostas::s_mutex (atexit dtor)` | uenux2/src/app/comum/dados/crespostas.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11499 | 531 | yes | `comum::CPartidosDSNumero::Text` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/cpartidos.u36.cpp | baixa |
| 11501 | 573 | yes* | `comum::CPartidosDSSigla::Text` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/cpartidos.u36.cpp | baixa |
| 11531 | 120 |  | `comum::(anonymous namespace)::ComparaDinamico` | uenux2/src/app/comum/dados/celeitores.cpp | src/uenux2/src/app/comum/dados/celeitores.cpp (u04) | alta |
| 11639 | 10 |  | `comum::CLogComum::s_mutex (atexit dtor)` | uenux2/src/app/comum/log/clogcomum.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11640 | 12 |  | `comum::CLogComum::s_pInstancia (atexit dtor)` | uenux2/src/app/comum/log/clogcomum.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11648 | 36 |  | `comum::CPath::ms_raiz (atexit dtor)` | uenux2/src/app/comum/cpath.cpp | u36-foreign-fragments.cpp (lista de atexit); cpath.h | alta |
| 11649 | 70 |  | `comum::CPath::ms_raizesFlash (atexit dtor)` | uenux2/src/app/comum/cpath.cpp | u36-foreign-fragments.cpp (lista de atexit); cpath.h | alta |
| 11657 | 10 |  | `comum::CArquivosSavd::GetInst()::s_mutex (atexit dtor)` | uenux2/src/app/comum/carquivossavd.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11659 | 10 |  | `comum::CArquivosResultado::GetInst()::s_mutex (atexit dtor)` | uenux2/src/app/comum/carquivosresultado.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |
| 11660 | 12 |  | `comum::CArquivosResultado::GetInst()::s_inst (atexit dtor)` | uenux2/src/app/comum/carquivosresultado.cpp | u36-foreign-fragments.cpp (lista de atexit) | alta |

## 10. Questões em aberto

* O significado exato de `ORIG:T` / `ORIG:I` no QR code do estado da urna (tela / impresso é um palpite) e por que a variante 2
  o omite.
* Se as três funções de diretório WSQ externo (3794/5816/5817) são membros de `CPath` ou funções livres de um
  helper de biometria. Elas são chamadas tanto do `comum` quanto do `vota`, então precisam estar declaradas em um header compartilhado.
* O tipo do membro de 4 bytes em `CApplicationContext +48`.
* A classe de erro e o código da verificação apagada em `CIdentificacaoUrnaContingencia` (linha 32).
* Os períodos 1 e 2 de `CRegistradorMesario::QuantidadeRegistrados`: inicial / final é inferido a partir dos chamadores (contadores
  do MT, relatório BIM). Um terceiro período (votação) existe nos nomes dos estados, mas nenhum thunk o usa.
* Os nomes do singleton de configuração do extrator de impressões digitais (vota_f1903: `{8, 500, 196.85}` = 8 bits por pixel?,
  500 dpi, 500/2,54 pixels por cm) e do singleton do codificador WSQ (2742), cujos handlers de atexit são 10519-10522.

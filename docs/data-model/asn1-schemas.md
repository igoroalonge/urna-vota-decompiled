# Os esquemas ASN.1 do TSE dentro de `vota_web_wasm.wasm`

Todo arquivo de dados que o aplicativo de votação lê ou escreve é um valor ASN.1 codificado em BER:
a configuração da eleição carregada na urna, o cadastro de eleitores, os candidatos e suas
fotos, os arquivos de estado da própria urna e os arquivos de resultado — o **BU** (*boletim de urna*,
a apuração de uma seção) e o **RDV** (*registro digital do voto*, a lista de todos os votos
dados, sem vínculo com o eleitor). O código C++ desses tipos foi gerado a partir de módulos `.asn1`
chamados `Modulo*`. Os fontes `.asn1` não estão no binário, mas as tabelas que o código
gerado entrega ao runtime ASN.1 estão, e elas contêm o esquema inteiro.

Este capítulo mostra como os esquemas foram reconstruídos a partir dessas tabelas, o que cada módulo significa para
a eleição, como eles se comparam com as especificações que o TSE publicou e qual
arquivo de cenário usa qual tipo. Os módulos reconstruídos estão em [`src/asn1/`](../../src/asn1/)
(32 arquivos, um por módulo). As ferramentas que os produziram e verificaram estão em
[`src/asn1/tools/`](../../src/asn1/tools/).

**Resumo**

* Foram recuperados 186 tipos construídos: **118 SEQUENCE, 10 CHOICE, 58 ENUMERATED**. Junto com eles
  vêm 5 tipos de string nomeados, em **32 módulos**. Para cada tipo temos os nomes dos membros, sua
  ordem, quais membros são OPTIONAL, as tags, as faixas de INTEGER, as restrições de SIZE e os
  nomes e valores das enumerações. Tudo isso vem diretamente das tabelas que o codificador e o decodificador usam.
* Os nomes dos tipos vêm de quatro fontes. Toda SEQUENCE carrega o próprio nome em um campo que o TSE
  acrescentou à tabela (118/118). Os demais tipos são nomeados a partir do RTTI do C++, de assinaturas de
  `std::source_location` ou da especificação publicada pelo TSE. **11 tipos CHOICE/ENUMERATED
  não têm nome em nenhum lugar do binário.** Eles estão marcados como placeholders nos arquivos `.asn`.
* **Verificação.** Um compilador ASN.1 independente, [`asn1tools`](https://github.com/eerimoq/asn1tools)
  0.169.0, compila os 32 módulos. Ele decodifica em BER, e recodifica **byte a byte**, 289 arquivos:
  todos os 272 arquivos de cenário decodificáveis dos 8 cenários do simulador, os 7 arquivos de estado escritos em
  tempo de execução e **10 envelopes oficiais de BU impresso (`*-imgbu.dat`) produzidos em 2024 pelo software
  real da urna `9.29.0.1 - Kayapó`**. O conteúdo deles é o texto enviado à impressora, então esses 10
  testam os tipos de envelope e de identidade da urna (`EntidadeEnvelopeGenerico`, `Urna`, `Carga`…),
  não o corpo do BU (`EntidadeBoletimUrna`), que é verificado apenas contra a especificação publicada (§5).
* Os módulos do BU e do RDV concordam, tipo a tipo, com a **especificação ASN.1 v2 (2024)
  publicada pelo TSE**. As únicas diferenças são 3 enumerações. Elas apontam para uma versão
  **posterior a 2024**, o que bate com a string de build `10.23.0.1 - DESENVOLVIMENTO`. O
  módulo de assinatura do TSE (`assinatura.asn1`) **não** está compilado no binário.

---

## 1. Contexto: ASN.1, BER e os arquivos da eleição

**ASN.1** (ITU-T X.680) é uma linguagem para descrever estruturas de dados. **BER** (X.690) é uma
forma de transformar valores em bytes. Todo valor é escrito como um **TLV**: uma **T**ag que diz o que ele
é, um comprimento (**L**ength) e um **V**alor. O valor de uma SEQUENCE é simplesmente a sequência dos TLVs
de seus membros, um após o outro:

```
DataHoraJE ::= GeneralString (SIZE(15))          -- "YYYYMMDDThhmmss"
CabecalhoEntidade ::= SEQUENCE { dataGeracao DataHoraJE, idEleitoral IDEleitoral }
IDEleitoral ::= CHOICE { idProcessoEleitoral [1] INTEGER (0..99999), idPleito [2] ..., idEleicao [3] ... }

30 15                               SEQUENCE, 21 bytes
   1b 0f 32303236...3137            GeneralString (UNIVERSAL 27), "20260910T162217"
   81 02 09 60                      context tag [1], 2 bytes: INTEGER 2400  -> idProcessoEleitoral
```

Um módulo declarado com `IMPLICIT TAGS` substitui a tag própria de um tipo quando um membro recebe uma
tag `[n]`. É por isso que `[1]` acima é `0x81` (classe de contexto, primitivo, número 1) e não
`0x02` (INTEGER). Um decodificador que não conhece o esquema vê apenas tags e bytes. É
por isso que recuperar o esquema importa.

O vocabulário do TSE usado a seguir:

| termo | significado |
|---|---|
| *processo eleitoral* / *pleito* / *eleição* | O "processo" eleitoral (por exemplo, as eleições gerais de 2026) contém *pleitos* (1º e 2º turno), que contêm *eleições* (federal, estadual…). Cada nível tem um id numérico. Nos cenários de treinamento eles são 2400 / 2410 / 2411. |
| *cargo* | Um cargo em disputa (presidente, vereador…). Uma pergunta de *consulta popular* (referendo) também é modelada como cargo. |
| *votável* | Tudo o que pode receber um voto: um número de candidato, ou um número de partido para um *voto de legenda*. |
| *seção* / *zona* / *município* / *local* | Seção eleitoral / zona eleitoral / município / local de votação. |
| *urna de contingência* | Urna reserva que substitui uma urna com defeito. |
| *SA* (Sistema de Apuração) | Sistema usado quando a urna não pode ser usada, alimentado a partir das cédulas de papel ou do cartão de memória. |
| *carga* | Carregamento dos dados da eleição em uma urna. O *código de carga* identifica cada carga. |
| *mesário* | Membro da mesa receptora de votos. *zerésima*: o relatório impresso antes do início da votação, que prova que todas as contagens estão zeradas. |
| *TTE* | *transferência temporária de eleitores*: eleitores transferidos temporariamente para outra seção (por exemplo, em serviço, presos, com necessidades de acessibilidade). |
| *fase* | `oficial` (eleição real), `simulado` (ensaio), `treinamento`. O simulador usa apenas `treinamento`. |

---

## 2. Qual ferramenta ASN.1: III ASN.1

Os nomes das classes C++ entregam a ferramenta. `ASN1::SEQUENCE`, `ASN1::CHOICE`,
`ASN1::SEQUENCE_OF<ModuloBoletimUrna::TotalVotosCargo, ASN1::EmptyConstraint>`,
`ASN1::Constrained_INTEGER<(ASN1::ConstraintType)2, 0, 9999u>` estão todos em `analysis/classes.tsv`.
Eles são o runtime da **III ASN.1 Tool** (Institute for Information Industry, Taiwan,
MPL 1.0; código-fonte: <https://github.com/Kampbell/III-ASN.1>). O runtime em si está documentado em
[`docs/libraries/asn1-runtime.md`](../libraries/asn1-runtime.md). O que importa aqui é como
o compilador dela, `asnparser`, transforma um tipo em C++.

Para `Federacao ::= SEQUENCE { identificador INTEGER (100..999), ... }`, o `asnparser` escreve uma
classe `ModuloFederacoes::Federacao : public ASN1::SEQUENCE` quase sem código. Ele também escreve
arrays estáticos e uma struct **`theInfo`** (de `asnparser/main.cxx`,
`SequenceType::GenerateInfo`):

```c++
const void* Federacao::fieldInfos[4] = { &identificador::value_type::theInfo, ... };
int         Federacao::fieldIds[4]   = { -1, -1, -1, -1 };      // -1 = mandatory, n = n-th OPTIONAL
const char* Federacao::fieldNames[4] = { "identificador", "sigla", "nome", "partidos" };
const Federacao::InfoType Federacao::theInfo = {
    Federacao::create /* = inherited ASN1::SEQUENCE::create */, 0x00000010 /*tag*/, 0, false /*extensible*/,
    fieldInfos, fieldIds, 4 /*numFields*/, 0 /*knownExtensions*/, 0 /*numOptional*/,
    NULL, &ASN1::SEQUENCE::defaultTag /*tags*/, fieldNames };
```

O codificador e o decodificador BER nunca veem `Federacao`. Eles percorrem `theInfo`. Portanto, essa struct, e as
que ela aponta, são o esquema. Ela sobrevive à compilação porque o programa precisa dela em tempo de
execução. Três coisas **não** sobrevivem:

1. **Sintaxe no nível do módulo**: `DEFINITIONS IMPLICIT TAGS`, `IMPORTS`, comentários.
2. **Nomes de aliases de tipos simples.** `QtdEleitores ::= INTEGER (0..9999)` vira
   `typedef ASN1::Constrained_INTEGER<FixedConstraint, 0, 9999> QtdEleitores;`. Assim, todo alias com
   a mesma faixa compartilha **uma** instância de template (e um `theInfo`). O nome se
   perde.
3. Os nomes de classe dos tipos que o programa nunca constrói diretamente. O construtor e a
   vtable deles foram removidos como código morto, e só resta a struct `theInfo`, alcançável a partir do
   `fieldInfos` de um pai.

O TSE modificou o runtime (veja o capítulo do runtime, §7). A mudança que mais nos ajuda:
**`SEQUENCE::InfoType` tem um campo extra em +48, `const char* name`**, o nome do tipo ASN.1.
Ele é retornado pelo slot 6 da vtable (func 11627) para diagnóstico. Isso resolve o problema 3 para toda
SEQUENCE.

---

## 3. Recuperando o esquema, passo a passo

### 3.1 Do nome de um membro até sua tabela

Comece por uma string que você espera que seja um nome de membro: `q.py grep qtdEleitoresAptos` a encontra em
**@65671**. Nenhuma função a referencia, porque apenas dados apontam para ela. Procure na imagem de memória
o valor de 4 bytes 65671 (0x00010087). Ele é encontrado em 1625092, dentro de um array de
ponteiros para strings:

```
$ python3 tools/wasmmap/q.py mem 1625088 28          # pointer lines only
  [1625088] -> 141711: 'idEleicao'
  [1625092] -> 65671:  'qtdEleitoresAptos'
  [1625096] -> 141786: 'qtdEleitoresAptosSecao'
  [1625100] -> 330231: 'qtdEleitoresAptosTTE'
  [1625104] -> 142098: 'resultadosVotacao'
  [1625108] -> 154639: 'ultimoHashVotosVotavel'
  [1625112] -> 154662: 'assinaturaUltimoHashVotosVotavel'
```

Isso é um array `fieldNames[]`. Procurar o ponteiro 1625088 encontra a palavra em
1144092+44, o slot `names` de um `SEQUENCE::InfoType` cujo `name` (+48) é
`"ResultadoVotacaoPorEleicao"`.

Ir uma tabela por vez não escala, então `extract_asn1.py` **varre a imagem inteira pela
forma**. Uma palavra *W* inicia uma tabela SEQUENCE se:

* *W*+24 (`numFields`) mais *W*+28 (`knownExtensions`) é pequeno,
* o byte em *W*+12 é um bool,
* *W*+44 aponta para exatamente essa quantidade de ponteiros, cada um para uma string com cara de identificador.

CHOICE (`names` em +32, contagem em +24) e ENUMERATED (`names` em +20, `maxEnumValue`+1
entradas) são encontrados da mesma forma. A varredura encontra 118 + 10 + 58 tabelas e nenhum falso positivo.
Todas elas acabam compartilhando o mesmo slot de tabela de funções de `create` por categoria (abaixo).

### 3.2 Lendo uma tabela: `EntidadeBoletimUrna` (o BU)

```
$ python3 tools/wasmmap/q.py mem 1144144 52          # condensed and annotated
1144144  7054 16 0 0            create=slot 7054 (SEQUENCE::create), tag=UNIVERSAL 16, parent=0, extensible=false
1144160  1625120 1625168 11 0   fieldInfos, ids, numFields=11, knownExtensions=0
1144176  2 0 1625216 1625264    numOptional=2, nonOptionalExtensions=NULL, tags, names
1144192  (name) -> "EntidadeBoletimUrna"

ids  @1625168: -1 -1 -1 -1 -1 -1 -1  0 -1 -1  1        members 7 and 10 are OPTIONAL (#0 and #1)
tags @1625216:  0  0  0  0  0  0  0  0x80000001  16 16 16
names@1625264: cabecalho fase urna identificacaoSecao dataHoraEmissao dadosSecaoSA
               qtdEleitoresCompareceram detalhamentoComparecimento resultadosVotacaoPorEleicao
               historicoCodigosCarga historicoVotoImpresso
fieldInfos @1625120: 1141220 (CabecalhoEntidade) 1140604 (Fase) 1144660 (Urna) ...
                     1913292 (DataHoraJE) ... 1597636 (SEQUENCE OF ResultadoVotacaoPorEleicao) ...
```

Uma tag é armazenada como `class << 30 | explicit << 29 | number`. Portanto, `0x80000001` é a tag de contexto
`[1]` (implícita), e `16` é UNIVERSAL 16, a tag padrão de um membro SEQUENCE OF, que significa
"sem nova tag". Em [`ModuloBoletimUrna.asn`](../../src/asn1/ModuloBoletimUrna.asn) isso vira:

```asn1
EntidadeBoletimUrna ::= SEQUENCE {
    cabecalho                              CabecalhoEntidade,
    fase                                   Fase,
    urna                                   Urna,
    identificacaoSecao                     IdentificacaoSecaoEleitoral,
    dataHoraEmissao                        DataHoraJE,
    dadosSecaoSA                           DadosSecaoSA,
    qtdEleitoresCompareceram               INTEGER (0..9999),        -- spec: QtdEleitores
    detalhamentoComparecimento             [1] DetalhamentoComparecimento OPTIONAL,
    resultadosVotacaoPorEleicao            SEQUENCE OF ResultadoVotacaoPorEleicao,
    historicoCodigosCarga                  SEQUENCE OF GeneralString,
    historicoVotoImpresso                  SEQUENCE OF HistoricoVotoImpresso OPTIONAL
}
```

### 3.3 Os tipos dos membros: um slot de `create` por categoria

Toda tabela começa com um ponteiro de função `create`. Em wasm, um ponteiro de função é um índice
na tabela de funções, então todas as tabelas de uma mesma categoria compartilham um slot. Esse slot identifica a categoria
de uma tabela de membro anônima:

| slot da tabela | func | corpo (descompilado) | categoria; layout depois de `{create, tag, parent}` |
|---|---|---|---|
| 7054 | 8962 | `return SEQUENCE::SEQUENCE(operator_new(52), info)` (func 219) | SEQUENCE; veja acima |
| 7055 | 8999 | `new(16)`, vtable `ASN1::CHOICE`, choiceID −1 | CHOICE: `ext, selectionInfos, numChoices, totalChoices, tags, names` |
| 7053 | 8941 | `new(12)`, vtable `ASN1::ENUMERATED` | ENUMERATED: `ext, maxEnumValue, names` (um nome por valor, `"<undefined>=n"` para lacunas) |
| 2284 | 2654 | `new(12)`, vtable `ASN1::INTEGER` | INTEGER: `constraintType, lower, upper` |
| 2445 | 9010 | vtable `ASN1::SEQUENCE_OF_Base` | SEQUENCE OF: `constraintType, lower, upper, elementInfo` |
| 4444 | 8967 | vtable `ASN1::OCTET_STRING` | OCTET STRING: `constraintType, lower, upper` |
| 7052 | 9003 | vtable `ASN1::AbstractString` | strings: `constraintType, lower, upper, charset, ...`; tag UNIVERSAL 27 = GeneralString, 18 = NumericString |
| 7115 | 8950 | `new(12)`, vtable `ASN1::BOOLEAN` | BOOLEAN |

`constraintType` 2 é `FixedConstraint`, então `{2, 100, 999}` significa `INTEGER (100..999)`.
Para strings, a mesma tripla é uma restrição SIZE. BOOLEAN, INTEGER, OCTET STRING, GeneralString,
NumericString, ENUMERATED, SEQUENCE, SEQUENCE OF e CHOICE são as únicas categorias em uso: não há
BIT STRING, UTF8String, SET nem tipo de tempo em nenhum esquema do TSE, e nenhum módulo usa marcadores
de extensão (`...`).

### 3.4 Tags

* `SEQUENCE::InfoType.tags` aponta ou para um array por membro, ou para
  **`&ASN1::SEQUENCE::defaultTag` (@1149108)**. Este último significa "todo membro mantém sua própria
  tag" (99 das 118 SEQUENCEs). Em um array, 0 ou um valor UNIVERSAL também significa "sem nova tag".
* `CHOICE::InfoType.tags == NULL` significa tags automáticas `[0]`, `[1]`, … (6 dos 10 CHOICEs).
* O bit 29 marca uma tag EXPLICIT. Ele aparece em `ModuloLocal.IdentificacaoLocal`
  (`[1] EXPLICIT`, `[2] EXPLICIT`), `ModuloEstadoGeralVota.EstadoGeralVota` (`[1]..[5] EXPLICIT`)
  e `ModuloInformacaoMidia.Autenticacao`. Todas as outras tags são implícitas. Isso, junto com os próprios
  arquivos publicados pelo TSE, é o motivo de os módulos reconstruídos declararem `DEFINITIONS IMPLICIT TAGS`.

### 3.5 Tabelas que só existem em tempo de execução

Algumas tabelas de membros apontam para fora do segmento de dados inicial, por exemplo `DataHoraJE` em **1913292**.
O segmento de dados do wasm termina em 1811674. Tudo acima disso é `.bss`, que é zero no arquivo.
Todo tipo **string** restrito (`GeneralString (SIZE(15))`, `NumericString (SIZE(1..5))`…) tem
uma tabela cujos campos `parent_info` e de conjunto de caracteres são copiados de `GeneralString::theInfo` ou
`NumericString::theInfo`. A cópia é feita por **inicializadores dinâmicos** do C++ que rodam em
`__wasm_call_ctors` antes de `main`. Por isso `q.py mem` não consegue mostrar essas 47 tabelas (40 delas
são usadas por tipos recuperados). A solução é executar o módulo e despejar a memória depois da inicialização:
[`dump_heap.mjs`](../../src/asn1/tools/dump_heap.mjs) modifica `tools/run/headless.mjs` em
memória para gravar `HEAPU8` a partir de `onRuntimeInitialized`. Lá,

```
1913292: create=7052 tag=27 parent=1148180 (GeneralString::theInfo) type=2 lower=15 upper=15
         -> DataHoraJE ::= GeneralString (SIZE(15))
```

### 3.6 Nomeando os tipos

| fonte | como | tipos |
|---|---|---|
| **campo `name` em `SEQUENCE::InfoType`+48** (acréscimo do TSE) | lido diretamente | todas as 118 SEQUENCE |
| **RTTI** | Construtores gerados armazenam o endereço da tabela junto da vtable da classe. Um exemplo é a func 10273: `l = ASN1_SEQUENCE_SEQUENCE_1(a, 1144144); l[0] = 1597580 /* vtable ModuloBoletimUrna::EntidadeBoletimUrna */` (func 219 é `ASN1::SEQUENCE::SEQUENCE(const void*)`). A forma com helper armazena a vtable primeiro: func 9162 `ecourna_app_dados_asn_ConverteEnum0_merged_body(..., 1128864 /* vtable FormaSuspenderComVoto */, 1138880, ...)`. `extract_asn1.py` emparelha essas constantes em `work/full.wat` e decide por maioria de votos. | módulo de 97 SEQUENCE; 35 ENUMERATED, 4 CHOICE, 5 tipos string |
| **RTTI + estrutura** | A classe existe, mas seu código faz construção por cópia a partir de um objeto já decodificado. A tabela é encontrada através do membro que o código lê: a func 11411 lê `fields[7]` de `EleitorUrna` e armazena a vtable `NecessidadeEspecial`. | módulo de `EleitorSequencia`, `EleitorUrna`; `NecessidadeEspecial`, `TipoImpedimento` |
| **assinaturas de `std::source_location`** | por exemplo `static ModuloBoletimUrna::TipoVoto::NamedNumber comum::asn::CConversorEntidadeBU::ConverteTipoVoto(...)`, `IConversorASN<ModuloEleicao::CargoPergunta, comum::md::CCargo>` | módulo de 8 SEQUENCE; 9 ENUMERATED, 1 CHOICE |
| **especificação publicada pelo TSE** | um tipo com exatamente os mesmos membros/valores e posição | `CargoConstitucional`, `TipoApuracao`, `TipoCedulaSA`, `OrigemVotosSA`, `DadosSecaoSA`, `IdentificacaoUrna` |
| **placeholder** (sem nome no binário) | a partir do membro que o usa; marcado com `<<< placeholder name` | CHOICE `IdentificacaoLocal`, `DetalheCargoPergunta`, `InfoDadosComparecimento`; ENUMERATED `TipoEleicao`, `TipoTransferenciaTemporaria`, `TipoSituacaoEleicao`, `SituacaoEleitor`, `ComposicaoBiometria`, `SituacaoCandidatura`, `SituacaoCassacao`, `TipoChave` |

Dois placeholders, `TipoEleicao` e `TipoTransferenciaTemporaria`, também são nomes de tipos de valor C++
do TSE: seus nomes para mensagens de erro são montados no inicializador estático func 14478 (strings
@141566, @229277). Isso os torna prováveis, não certos.

Os nomes das SEQUENCEs foram atribuídos primeiro sem o campo +48. Quando o campo foi lido, ele
confirmou todos os nomes que vieram de RTTI, srcloc ou da especificação (108). Dos 10 nomes que tinham
sido adivinhados a partir dos nomes dos membros, ele confirmou 4 (`SituacaoEleicao`, `EleitorImpedido`,
`ComplementosMunicipiosUF`, `DescritorChave`) e corrigiu 6 (`Calendario` virou
`CalendarioEleicao`, `Domicilio` virou `DomicilioEleitoral`, …). É por isso que os placeholders
restantes de CHOICE/ENUM devem ser lidos como rótulos, não como nomes.

**Módulos.** O módulo de um tipo é conhecido quando um nome qualificado de RTTI ou srcloc o informa
(158 dos 186 tipos construídos, mais os 5 tipos string). Para os outros 28, ele é inferido
a partir dos tipos que o usam, e o arquivo `.asn` diz isso.
Exemplo típico: `CargoConstitucional` é usado por `ModuloTiposEleitorais::CodigoCargoConsulta`.

### 3.7 Verificação

1. **Nosso próprio decodificador.** `extract_asn1.py --decode FILE` contém um decodificador BER de ~100 linhas guiado
   apenas pelas tabelas recuperadas. Sem um tipo informado, ele tenta cada SEQUENCE como tipo de nível
   superior e relata aquelas que consomem o arquivo exatamente. Para a maioria dos arquivos, exatamente um tipo se encaixa
   (§6).
2. **Um compilador independente.** [`verify_asn1tools.py`](../../src/asn1/tools/verify_asn1tools.py)
   compila os 32 arquivos `.asn` com `asn1tools` e verifica decodificar → codificar → mesmos bytes:

```
$ python3 src/asn1/tools/verify_asn1tools.py  <10 official 2024 files> ModuloEnvelopeGenerico EntidadeEnvelopeGenerico
compiled 32 modules, 191 types
289 files decode and re-encode byte-identically, 0 failures
```

Uma tag errada, um OPTIONAL faltando, uma ordem de membros errada ou um tipo base errado quebra ou a
decodificação ou a recodificação idêntica byte a byte. As faixas não são testadas pelo BER. Elas são lidas diretamente
e batem com a especificação publicada sempre que a especificação as mostra (§5).

---

## 4. Os módulos

Os arquivos: [`src/asn1/<Module>.asn`](../../src/asn1/). Acima de cada tipo, um comentário dá o
endereço da tabela, a fonte do nome e o grau de confiança.

### 4.1 Tipos compartilhados

| módulo | conteúdo |
|---|---|
| [`ModuloTiposEleitorais`](../../src/asn1/ModuloTiposEleitorais.asn) (32 tipos) | `DataHoraJE` = `GeneralString (SIZE(15))` `YYYYMMDDThhmmss`; `DataJE` (8 dígitos), `HoraJE` (6), `SiglaUF` (2). `IDEleitoral`: CHOICE do id de processo/pleito/eleição. `CabecalhoEntidade {dataGeracao, idEleitoral}` inicia **todo** arquivo de entidade. `CabecalhoPacote` é o cabeçalho de pacote armazenado em `*.pid`. `MunicipioZona`, `IdentificacaoSecaoEleitoral {municipioZona, local, secao}`, `IdentificacaoUrna` (seção ou contingência). `CodigoCargoConsulta`: CHOICE `[1] CargoConstitucional` / `[2] INTEGER (25..99)` (número livre de cargo/pergunta). Enums `Fase`, `Turno`, `TipoCargoConsulta`, `TipoDedo` (dedos), `CodigoSexo`, `Sistema` (os sistemas do TSE que produzem pacotes: `configurador`, `candidaturas`, `gedai`, `urnaEletronica`, `simulador`, `padaUE`…). `Seguranca` (parâmetros de cifração para a biblioteca CEPESC, a biblioteca de criptografia do governo). `IdentificadorEleitor`: CHOICE (número do título de eleitor / CPF / id livre). |
| [`ModuloTiposEcoUrna`](../../src/asn1/ModuloTiposEcoUrna.asn) | `Carga {numeroInternoUrna, numeroSerieFC, identificadorGeradorMidia, dataHoraCarga, codigoCarga}`, o registro da carga. `IdentificadorGeradorMidia {nome, serialCertificadoTPM, serialInstalacao}` identifica o computador que gerou a mídia de carga (pelo seu certificado TPM). `ModeloEquipamento {tpm20(2), ue2013(13), ue2015(15), ue2020(20), ue2022(22)}`. |
| [`ModuloTiposResultadosEcoUrna`](../../src/asn1/ModuloTiposResultadosEcoUrna.asn) | `Urna {tipoUrna, versaoVotacao, correspondenciaResultado, tipoArquivo, numeroSerieFV, motivoUtilizacaoSA}`, a identidade da urna gravada em todo arquivo de resultado. `TipoArquivo` (produzido pela urna, pelo recuperador de dados RED ou pelo SA em seus modos). `TipoApuracaoSA` e os enums `Motivo*` dizem por que o SA foi usado. |
| [`ModuloTiposCadastro`](../../src/asn1/ModuloTiposCadastro.asn) | `Municipio {codigo, nome, capital, [1] codigoIBGE, [2] comBiometria}`, `TipoLocalVotacao` (normal, emTransito, presoProvisorio, temporario). |
| [`ModuloTiposPacotes`](../../src/asn1/ModuloTiposPacotes.asn) | `TipoPacote`: 56 tipos de pacote nomeados (valores 1..65) trocados entre sistemas do TSE (`pacoteEleitores(1)`, `pacoteParametrosUrna(23)`, `pacoteFotosCandidatosTRE(8)`…). |

### 4.2 Resultados produzidos pela urna

**`ModuloBoletimUrna` — o BU.** `EntidadeBoletimUrna` (§3.2) contém, para cada eleição, um
`ResultadoVotacaoPorEleicao`. Ele lista as quantidades de eleitores aptos (total, originalmente na
seção, TTE), um `ResultadoVotacao` por tipo de cargo e, para cada cargo, um `TotalVotosCargo` com
um `TotalVotosVotavel {tipoVoto, quantidadeVotos, identificacaoVotavel, ordemGeracaoHash,
hash}` por linha de candidato/partido/branco/nulo. Os dois últimos membros formam uma **cadeia de hashes**. A
documentação do TSE descreve o primeiro elo como
`idPleito(5)|idEleicao(5)|municipio(5)|zona(4)|secao(4)|codigoCarga(24)`. O binário tem a
string de formato `"{:05}|{:05}|{:05}|{:04}|{:04}|{:24}"` (@8483), usada apenas pela func 10273. Essa
função é o conversor do BU, `comum::asn::CConversorEntidadeBU::DoConverte`: ela é o slot 2 da
vtable dessa classe e constrói o objeto da tabela 1144144. `functions.tsv` atualmente
a rotula erroneamente como `CDadosBUVota::GetDataHoraDesligamentoVotoImpresso`, por causa do srcloc de uma
chamada inlinada. Cada linha faz o hash do hash anterior e dos seus próprios campos. `ResultadoVotacaoPorEleicao`
termina com `ultimoHashVotosVotavel` e sua assinatura `assinaturaUltimoHashVotosVotavel`
(`comum::asn::CAssinaVotavelBu::Assinar`, srcloc `cassinavotavelbu.cpp:89`). O `TipoVoto` do BU
é `nominal(1), branco(2), nulo(3), legenda(4), cargoSemCandidato(5)`.

**`ModuloRegistroDigitalVoto` — o RDV.** `EntidadeResultadoRDV {cabecalho, urna, rdv}` encapsula
`EntidadeRegistroDigitalVoto {pleito, fase, identificacao, historicoCodigosCarga, eleicoes}`.
`Eleicoes` é um CHOICE entre `[1] SEQUENCE OF EleicaoVota` (urna) ou `[2] SEQUENCE OF EleicaoSA`
(sistema de apuração, com o tipo de cédula e a origem dos votos). Cada `VotosCargo` contém
`SEQUENCE OF Voto {tipoVoto, digitacao VotoDigitado OPTIONAL}`. `VotoDigitado` é
`NumericString (SIZE(1..5))`: os dígitos **como digitados** pelo eleitor. O `TipoVoto` do RDV é
mais detalhado que o do BU: `legenda(1) nominal(2) branco(3) nulo(4) brancoAposSuspensao(5)
nuloAposSuspensao(6) nuloPorRepeticao(7) nuloCargoSemCandidato(8)
nuloAposSuspensaoCargoSemCandidato(9)`. Neste build web os votos não são persistidos no RDV: `rdv.dat` fica com 80 bytes (veja
`analysis/runtime/README.md`). O tipo e seu codificador (`CFileASN::CodeObjectFunction`, func
11488 / 11587) são compilados mesmo assim.

**`ModuloEnvelopeGenerico`.** `EntidadeEnvelopeGenerico {cabecalho, fase, urna OPTIONAL,
identificacao, tipoEnvelope, seguranca OPTIONAL, conteudo OCTET STRING}` encapsula um arquivo de resultado.
`tipoEnvelope`: `envelopeBoletimUrna(1)`, `envelopeRegistroDigitalVoto(2)`,
`envelopeBoletimUrnaImpresso(4)` (o texto do BU impresso, `*-imgbu.dat`),
`envelopeImagemBiometria(5)`, **`envelopeZeresimaImpressa(6)`** (a zerésima impressa). Quando
`seguranca` está presente, `conteudo` é cifrado.

**`ModuloResultadoUrnaCadastro`.** Dados devolvidos ao cadastro eleitoral: para cada eleitor,
`SituacaoComparecimentoEleitor` (faltou/naoVotou/votou/semCargoParaVotar), como ele foi habilitado
(tentativas biométricas, dedo, último score, habilitado por código por um mesário, áudio), situação da
exibição da foto, `justificativas` (eleitores que justificaram a ausência, com ano de nascimento) e os mesários
presentes na abertura e no encerramento. `InfoDadosComparecimento` seleciona a variante em claro ou a cifrada
(`DadosComparecimentoCifrado {dadosCifracao {chave, salt, informacaoAdicional}, conteudo}`).

**Também:** `ModuloHashes.EntidadeHashes` (hashes dos arquivos instalados, `versaoUENUX` = versão
do SO), `ModuloVersaoArquivos.EntidadeVersaoArquivos` e
`ModuloEnvelopeChave.EntidadeChave` (uma chave `{tagChaves, descritor {nomeUsuario, serial},
cifrado, tipo, chave}`). `DescritorChave` é o único tipo que o binário compartilha com a
especificação de assinatura do TSE.

### 4.3 Dados da eleição carregados na urna

| módulo | tipo de nível superior | o que descreve |
|---|---|---|
| `ModuloProcessoEleitoral` | `EntidadeProcessoEleitoral` | nome, `pleito1`/`pleito2` (`Pleito {id, nome, data}`), tipo e abrangência da eleição principal, se mesa receptora de justificativa e biometria são usadas, tipos de TTE permitidos, tipos de identificação do eleitor, UFs |
| `ModuloEleicao` | `EntidadeEleicao` | uma eleição: `cargos` como `SEQUENCE OF CargoPergunta {codigo, tipo, numeroDigitos, qtdeEscolhas, podeRepetir, ordemAquisicao/Impressao/Apuracao, paginaImpressaoVoto, detalhe}`. `detalhe` é ou um `DetalheCargo` (nomes nas formas neutra/masculina/feminina/abreviada, `suplencias` = companheiros de chapa, como vice ou suplente) ou um `DetalhePergunta` (pergunta de consulta popular com respostas `RespostaPergunta` e `textoFonetico` para o áudio). `CalendarioEleicao` opcional |
| `ModuloSituacoesEleicoes` | `EntidadeSituacoesEleicoes` | por eleição: `ativa/suspensa/cancelada/excluida` e as três ordenações |
| `ModuloCandidatos` | `EntidadeCandidatos` | `CandidatosPorCargos` → `CandidatoPorPartido` → `Candidatura {numero, titular DadosCandidato, drap, suplentesVices}`. `DadosCandidato` tem nome, nome de urna, nome social, nome fonético, data de nascimento, sexo, `SituacaoCandidatura` (deferido, indeferidoComRecurso, …) e `SituacaoCassacao` |
| `ModuloFotosCandidatos` | `EntidadeFotosCandidatos` | `FotoCandidato {codigoCandidato, foto Foto {formato JPEG/BMP, imagem}}` (lido com o decodificador "parcial" em streaming do TSE) |
| `ModuloPartidos` / `ModuloFederacoes` | `EntidadePartidos` / `EntidadeFederacoes` | partidos `{numero, sigla, nome}`; federações `{identificador (100..999), sigla, nome, partidos}` |
| `ModuloParametrizacaoUrna` | `EntidadeParametrizacaoUrna` | a **PU**, 47 parâmetros: como um voto suspenso é encerrado (`FormaSuspenderSemVoto/ComVoto`), vias de cada relatório (zerésima, BU para Vota/RED/SA), tempos-limite (confirmação do voto, suspensão, desligamento automático), cabeçalhos e rodapés de relatórios, rótulos ("Município", "Zona"… com gênero e plural) e flags `imprimirQrCodeNoBU`, `aceitarJustificativa`, `aceitarBrancoNulo`, `exibirScoreBiometria`, `registrarMesarios`, `pedeAnoNascimentoEleitor`… |
| `ModuloConfiguracaoMunicipios` | `EntidadeConfiguracaoMunicipios` | por município: horários da zerésima, do início, do fim e do encerramento da votação |
| `ModuloComplementosMunicipios` | `EntidadeComplementosMunicipios` | por UF e município: fuso horário (`fuso` −720..720 minutos), horário de verão, `desligaColetaBiometria` |
| `ModuloLocal` | `Local` | o local desta urna: país, `UF`, `Municipio`, complementos e `IdentificacaoLocal` = `[1] SecaoEleitoral` (com seções agregadas) ou `[2]` contingência |
| `ModuloEleitores` | `EntidadeEleitores` | os eleitores da seção: `EleitorSequencia {sequencial, eleitor EleitorUrna, [1] tipoTransferenciaTemporaria}`. `EleitorUrna` tem ids, nome, nome social, nome da mãe, data de nascimento, `SituacaoEleitor`, `[2] DomicilioEleitoral` (UF/zona/seção de origem e datas; provavelmente usado para eleitores em TTE), `NecessidadeEspecial` (`necessitaAudio`) e `BiometriaEleitorCifrada {composicaoBiometria, conteudo, salt}` (templates cifrados de foto/impressão digital; `BiometriaEleitor`/`Dedo {tipo, qtdMinucias, minucias}` depois de decifrados) |
| `ModuloImpedidos` | `EntidadeImpedidos` | eleitores que não podem votar aqui, por turno: `TipoImpedimento` (votaNaSecaoOriginal, solicitouVotoEmTransito, presoProvisorio, suspenso, cancelado, semIdadeMinima, tte*…) |
| `ModuloInformacaoMidia` | `InformacaoMidia` | o que é uma mídia flash: `tipoMidia` (`mr` = memória de resultado, `fc` = flash de carga, `fv` = flash de votação), fase, pleito, UF, turno, `DadosGeracaoMidia {serialMidia, usuario, identificadorGeradorMidia, data}` e os aplicativos que ela habilita (`vota`, `sa`, `red`, …, com autenticação opcional por senha) |
| `ModuloDadosDisponiveisCarga` | `DadosDisponiveisCarga` | o que a mídia de carga oferece: municípios com suas seções e os cabeçalhos dos pacotes importados |

### 4.4 O estado persistente da urna

| módulo | arquivo | conteúdo |
|---|---|---|
| `ModuloEstadoGeralUrna` | `dinamico/eg.bin` | `EstadoGeralUrna`: pleito, `EstadoUrna` (carregando/carregada/testada), `DadoCarga` (turno, tipo nos turnos 1 e 2, modelo, fase), `DadoLocal`, ajuste do relógio, o `DadoCorrespondencia` da carga (com uma assinatura), versão do software, hash das versões dos pacotes |
| `ModuloEstadoGeralVota` | `trab1/vota.bin`, `trab2/vota.bin` | `EstadoGeralVota`: a máquina de estados do VOTA (`EstadoVota`: inicial … gerarze, zeresimaimpressa, registromesarioinicial, **votar**, fimaquisicaovotos … gerarbu, imprimirbu, gravarresultados, encerrada), contagem de BUs, comparecimento, justificativas e membros opcionais `[1]..[5] EXPLICIT` (urna que gerou a zerésima; timestamps de início/fim da aquisição de votos, do último voto e da emissão do BU) |
| `ModuloEstadoGeralSA` | `sa.bin` | máquina de estados do sistema de apuração (`EstadoSA`, 24 estados) e seção |
| `ModuloEstadoGeralGap` | `gap.bin` | o lançador de aplicativos: até 10 correspondências de carga, aplicativo atual e anterior (`UrnaAplicativo`: apvotatreinaeleitor1 … apvotaoficial2, apapuracao*), RED executado, áudio, data do 2º turno |
| `ModuloEstadoGeralDefs` | — | `NumViasImpressasRelatorios` (vias já impressas) |

Um `eg.bin` decodificado mostra o quanto desse estado o simulador inventa. Ele diz:
`modelo ue2015, numeroInternoUrna 87654321, codigoCarga "123456789012345678901234",
dataHoraCarga "20201231T235958", identificadorGeradorMidia {nome "nome_maquina",
serialCertificadoTPM "12345678", serialInstalacao "99999999"}, versao "7.2.1.3 - TESTE EG ASN1"`.
São fixtures de teste fixas no código do binário: @354064 (a `versao`) é usada pela func 10268,
@227209 `nome_maquina` (com @341300 / @339893) pela func 9952, e @346764 (o `codigoCarga`) pela
func 10261.

---

## 5. Comparação com as especificações publicadas pelo TSE

O TSE publica um ASN.1 simplificado dos arquivos de resultado junto com sua *documentação técnica do
software da urna* / páginas de dados abertos: `bu.asn1`, `rdv.asn1`, `assinatura.asn1`, com exemplos
em Python. A versão de 2022 (v1) e a de 2024 (v2) foram usadas aqui, a partir de espelhos públicos
(`github.com/danarrib/TSEParser` `TSE_Docs/`, `github.com/doccaz/urnas-br` `doc/` e `docv2/`).
O cabeçalho da v2 diz que ela "agrega, para fins didáticos, `tiposeleitorais.asn1,
tiposecourna.asn1, tiposresultadosecourna.asn1, envelopegenerico.asn1, boletimurna.asn1`".
Esses são exatamente os módulos `ModuloTiposEleitorais`, `ModuloTiposEcoUrna`,
`ModuloTiposResultadosEcoUrna`, `ModuloEnvelopeGenerico` e `ModuloBoletimUrna` encontrados no
binário.

Comparação por tipo, considerando membros/valores, ordem, OPTIONAL, tags e faixas:

| arquivo publicado | idênticos | diferentes |
|---|---|---|
| `bu.asn1` v2 (2024), 41 tipos | 38 | 3 enumerações (abaixo) |
| `rdv.asn1` v2 (2024), 35 tipos | 33 | 2 enumerações (abaixo) |
| `assinatura.asn1` v2, 12 tipos | `DescritorChave` | `ModeloEquipamento` tem menos valores; os outros 10 tipos estão **ausentes** do binário |
| `bu.asn1` v1 (2022) | 32 | `EntidadeBoletimUrna`, `ResultadoVotacaoPorEleicao`, `TotalVotosVotavel`, `Carga` mudaram; `IdentificacaoMesaJustificativa` foi removido; as 3 enumerações abaixo |

Todo INTEGER, OCTET STRING e alias de string restrito nos arquivos v2 tem a mesma faixa que
a tabela no binário. As diferenças em relação à v2:

| tipo | v2 publicada (2024) | binário (10.23.0.1) | efeito nos bytes |
|---|---|---|---|
| `TipoEnvelope` | 1, 2, 4, 5 | acrescenta **`envelopeZeresimaImpressa(6)`** | valor novo |
| `TipoUrna` | `reservaSecao(4)`, `reservaEncerrandoSecao(6)` | `contingenciaSecao(4)`, `contingenciaEncerrandoSecao(6)` | mesmos números, outros nomes |
| `MotivoApuracaoMistaComBU` | `urnaChegouAposInicioVotacao(5)` (também na v1) | `urnaEncerradaComEleitoresNaFila(5)` | **mesmo número, significado diferente** |
| `ModeloEquipamento` | `tpm12(1)`, `ue2009`, `ue2010`, `ue2011` … | apenas `tpm20, ue2013, ue2015, ue2020, ue2022` | modelos antigos removidos |

De 2022 para 2024 o modelo de dados mudou:

* `Carga` ganhou `identificadorGeradorMidia`.
* O RDV ganhou `historicoCodigosCarga`.
* O BU substituiu `qtdEleitoresLibCodigo`/`qtdEleitoresCompBiometrico` por
  `qtdEleitoresCompareceram` + `[1] DetalhamentoComparecimento`, substituiu
  `historicoCorrespondencias` por `historicoCodigosCarga` e trocou a assinatura de uma única
  `chaveAssinaturaVotosVotavel` pela cadeia de hashes por linha.
* A identificação da mesa de justificativa desapareceu.
* As tags do CHOICE `Eleicoes` do RDV mudaram de `[0]`/`[1]` (texto da v1) para `[1]`/`[2]`.

O binário tem a forma de 2024 **mais** as mudanças posteriores acima. Coerente com isso, os dois BUs
de exemplo da v1 e os dois RDVs de exemplo da v1 (`TSE_Docs/exemplos`, `s02100-…` e `t02510-…`) falham ao
decodificar com o esquema recuperado exatamente em `Carga.identificadorGeradorMidia`.

**Arquivos oficiais de 2024 decodificam byte a byte.** Os exemplos de BU de 2024 do TSE (o `leiame.txt`
do zip tem o título "QR Code no Boletim de Urna") contêm 10 envelopes de exemplo de BU impresso
(`TSE-exemplos-boletim-urna-eleicoes-2024.zip`, do *simulado* de 2024: seção
normal, RED, apuração manual no SA, uma *consulta popular*, uma disputa para o Senado). Cada um é um
`EntidadeEnvelopeGenerico` com `urna.versaoVotacao = "9.29.0.1 - Kayapó"`. O gerador de mídia
é `tsesevinw09`, com um serial de TPM real, e `tipoEnvelope = envelopeBoletimUrnaImpresso`. Todos os 10
decodificam com os módulos recuperados e são recodificados em bytes idênticos.

Saiba o que esse teste cobre. O `leiame.txt` do zip chama esses arquivos de "cópia do conteúdo enviado
para a impressora da urna". `conteudo` guarda esse texto da impressora (por exemplo `"Justiça Eleitoral … Boletim de Urna …"`), não um `EntidadeBoletimUrna`. Assim,
o resultado byte a byte prova que o software real de 2024 usa os mesmos tipos de **envelope e cabeçalho**:
`EntidadeEnvelopeGenerico`, `CabecalhoEntidade`, `Fase`, `Urna` (`TipoUrna` 1 e 3,
`TipoArquivo` votacaoUE / votacaoRED / saManual), `CorrespondenciaResultado`, `Carga`,
`IdentificadorGeradorMidia`, `IdentificacaoSecaoEleitoral` e `TipoEnvelope`. O corpo do BU
(`EntidadeBoletimUrna`, `ResultadoVotacaoPorEleicao`, `TotalVotosVotavel`…) e o corpo do RDV são
verificados apenas contra o texto publicado da v2 (tabela acima). Nenhum `-bu.dat` ou `-rdv.dat` v2 real
foi decodificado.

**O pacote de 2026.** O pacote "Formato dos arquivos de BU, RDV e assinatura digital" de 2026 do
TSE (README datado de 2026-09-10) foi lido de uma cópia de terceiros, e não do site do TSE (ver
"Fontes e procedência" no relatório da investigação). Comparado com o binário:

* O `bu.asn1` é igual a `src/asn1` em 41/41 tipos estruturados, incluindo as três mudanças de
  enumeração da tabela acima (`envelopeZeresimaImpressa(6)`, `contingenciaSecao(4)`,
  `contingenciaEncerrandoSecao(6)`, `urnaEncerradaComEleitoresNaFila(5)`).
* O `assinatura.asn1` tem o `ModeloEquipamento {tpm20, ue2013, ue2015, ue2020, ue2022}` do binário
  e acrescenta `OrigemAssinaturaHardware {modeloEquipamento, algoritmoAssinatura}` e
  `InfoChave ::= CHOICE {tagChaves [0], certificadoDigital [1]}`. Ele decodifica e recodifica
  110/110 arquivos de assinatura reais de 2026; o texto da v2 falha em todos os 110.
* O `rdv.asn1` é idêntico byte a byte ao arquivo v2 de 2024 e ainda diz `reservaSecao(4)`,
  `reservaEncerrandoSecao(6)` e `urnaChegouAposInicioVotacao(5)`. A decodificação não é afetada
  (mesmos números), mas `MotivoApuracaoMistaComBU = 5` quer dizer "chegou depois do início da
  votação" em um arquivo do TSE e "encerrada com eleitores ainda na fila" no outro.

Os arquivos reais de 2026 de 135 seções não trazem nenhum valor codificado fora das enumerações do
binário (dados das urnas de 2026: [`investigation/LEIAME.md`](../../investigation/LEIAME.md),
achados H6, H7, H8, B22).

---

## 6. Quais arquivos de cenário usam qual tipo

Arquivos em `upstream/fs/municipal-t1/dsk/fi/estatico/`. **Método:** cada arquivo foi decodificado com
cada SEQUENCE recuperada como tipo de nível superior (`extract_asn1.py --decode`). Os arquivos de *pacote*
(`*.pid`) dizem a qual pacote o `.dat` pertence. As funções leitoras dão uma terceira verificação
(srclocs `api::CFileASN::ReadFromFile [T = …]`, funcs 3735, 3771, 5706, 5740, 5778, 11517,
11905…).

**Padrão de nomes.** `t` = fase *treinamento*; `02400` = processo eleitoral, `02410` = pleito,
`02411` = eleição; `ac` = UF; depois município (5 dígitos), zona (4) e seção (4).
Portanto, `t02400ac000010001-el` é "eleitores, processo 2400, AC, município 1, zona 1". O
`IDPacote` correspondente no `.pid` diz `{idProcessoEleitoral 2400, fase treinamento, siglaUF
"AC", codigoMunicipio 1, numeroZona 1}`.

| arquivo (tamanho) | tipo | observações |
|---|---|---|
| `*.pid` (46–68 B) | `ModuloTiposEleitorais.CabecalhoPacote` | por exemplo, `t02411ac00001-ca.pid` = `pacoteCandidatosTRE`, `nomepacote "t02411ac00001-ca.jez"`, `versao "202609101622"`, `origem padaUE` |
| `t00000br-pu.dat`, `t02400ac-pu.dat` (1421) | `ModuloParametrizacaoUrna.EntidadeParametrizacaoUrna` | pacote `pacoteParametrosUrna` |
| `t02400-cp.dat` (169) | `ModuloProcessoEleitoral.EntidadeProcessoEleitoral` | `pacoteConfiguracaoProcessoEleitoral` |
| `t02410ac-cfm.dat` (254) | `ModuloConfiguracaoMunicipios.EntidadeConfiguracaoMunicipios` | |
| `t02410ac-cm.dat` (74) | `ModuloComplementosMunicipios.EntidadeComplementosMunicipios` | `pacoteComplementosMunicipios` |
| `t02410ac-ste.dat` (45) | `ModuloSituacoesEleicoes.EntidadeSituacoesEleicoes` | `pacoteSituacoesEleicoes` |
| `t02411ac-ce.dat` (305) | `ModuloEleicao.EntidadeEleicao` | `pacoteEleicao`; Prefeito (vice = cargo livre 99) e Vereador |
| `t02411ac00001-ca.dat` (1704) | `ModuloCandidatos.EntidadeCandidatos` | `pacoteCandidatosTRE` |
| `t02411ac00001-fo.dat` (330 018) | `ModuloFotosCandidatos.EntidadeFotosCandidatos` | `pacoteFotosCandidatosTRE` |
| `t02411ac00001-pa.dat` (227) | `ModuloPartidos.EntidadePartidos` | |
| `t02411ac00001-fe.dat` (25) | `ModuloFederacoes.EntidadeFederacoes` | apenas cabeçalho, `federacoes` ausente (OPTIONAL) |
| `t02400ac0000100010001-el.dat` (137) | `ModuloEleitores.EntidadeEleitores` | um eleitor, `NOME OMITIDO`, `nomeMae "Não utilizado"` |
| `t02400ac0000100010001-tte.dat` (42) | `ModuloEleitores.EntidadeEleitores` (confiança média) | sem eleitores; o `.pid` diz `pacoteTransferenciaTemporariaEleitores`. `EntidadeImpedidos` tem a mesma forma quando vazio |
| `t02400ac0000100010001-imp.dat` (66) | `ModuloImpedidos.EntidadeImpedidos` | |
| `0000100010001-lo.dat` (87) | `ModuloLocal.Local` | nome montado com `"{:05}{:04}{:04}-lo.dat"` (@60623) |
| `infomidia-fv-1-t.dat` (172) | `ModuloInformacaoMidia.InformacaoMidia` | gerado por `"simulador-votacao-ng"`, serial de TPM todo zerado |
| `t02400ac-mme.dat` (27), `t02411ac00001-pi.dat` (27) | indeterminado | cabeçalho + `SEQUENCE OF` vazio: encaixa em 4 tipos. O `.pid` de `mme` diz `pacoteMigracaoMunicipioEleicao` |
| `t02411ac00001-co.dat`, `-le.dat` (41) | indeterminado | cabeçalho + `Abrangencia` + lista vazia: encaixa em `EntidadeCandidatos`, `EntidadePartidos` e `EntidadeFotosCandidatos`. Ambos são assinados dentro do pacote de candidatos (veja `*.vsc`), com `-pi`, `-fe`, `-pa` e `-rdj` |
| `t02400ac-mz.dat` (59) | **módulo ausente do binário** | `pacoteMunicipiosZonas`: `{cabecalho, SEQUENCE OF {uf, SEQUENCE OF MunicipioZona}}` |
| `t02400ac-mu.dat` (118) | **módulo ausente do binário** | `{cabecalho, SEQUENCE OF {UF, SEQUENCE OF Municipio}}`: os elementos são `ModuloTiposEleitorais.UF` e `ModuloTiposCadastro.Municipio`; assinado no pacote `-mz` |
| `t02400ac000010001-se.dat` (150) | **módulo ausente do binário** | `pacoteSecoes`: locais de votação com suas seções |
| `scueconf-t1.dat` (78) | **módulo ausente do binário** | uma configuração de carga: `INTEGER 2410` e uma lista de pacotes com `"pacoteunico.sh"` |
| `t02411ac00001-rdj.dat` (29) | não é ASN.1 | texto Latin-1 `"registro de decisão judicial"` |
| `*.vsc` (0 B, ou 14–73 KB) | `EntidadeAssinatura` do `assinatura.asn1` **publicado** (não compilado no wasm) | Assinatura de pacote: `algoritmoHash sha512`, `algoritmoAssinatura cepesc, 256 bits`, serial da chave `20260223`, depois `[0] "202602231028"` (conjunto de chaves). A `Assinatura` interna lista os arquivos assinados. O signatário (`nomeUsuario`) é a unidade do TSE que produziu o pacote: `SEVIN` (PU), `SEINT` (processo, eleição, situações, mme), `SECAD` (eleitores, impedidos, TTE, municípios/zonas, complementos), `SECINP` (candidatos e fotos). `t02411ac00001-ca.vsc` assina `-ca`, `-co`, `-fe`, `-le`, `-pa`, `-pi`, `-rdj` + `.pid` + um `.ver`. Os pacotes de zona `-el`/`-imp`/`-tte` assinam seis seções (0001–0006), das quais só a 0001 é distribuída. Muitos arquivos `.vsc` estão vazios (0 bytes): os dos arquivos por seção (cobertos pelo pacote da zona) e os de `-se`, `-lo`, `infomidia` e `scueconf`, que nenhuma assinatura no cenário cobre |

Os quatro arquivos com "módulo ausente do binário" são de sistemas anteriores ao VOTA na cadeia (a
preparação da carga). Em vez deles, o aplicativo da urna lê os já montados `Local` / `DadosDisponiveisCarga`.

Arquivos escritos em tempo de execução (`analysis/runtime/memfs-after-vote/dsk/…`): `eg.bin`
(`EstadoGeralUrna`), `vota.bin` (`EstadoGeralVota`; depois de um voto, `estadoVota = votar`,
`treinamentoEleitor = TRUE`), `sa.bin`, `gap.bin`. O resto não é ASN.1: `rdv.dat` (80 bytes,
não é BER, parece cifrado), `uenux.db` (SQLite), `*.vsu` (o texto `assinatura simulada para
vota_web_wasm`), `logd.dat` (log de texto) e `serialv.dat` (`ABCDDCBA`).

### 6.1 Decodificando um arquivo à mão: `t02400-cp.pid` (46 bytes)

```
30 2c                          CabecalhoPacote ::= SEQUENCE, 44 bytes
   0a 01 0e                    tipoPacote   ENUMERATED 14 = pacoteConfiguracaoProcessoEleitoral
   30 07                       idPacote     IDPacote ::= SEQUENCE
      81 02 09 60                 idPacoteEleitoral: IDEleitoral is a CHOICE, [1] = idProcessoEleitoral = 2400
      0a 01 03                    fase ENUMERATED 3 = treinamento
                                  (siglaUF [1], codigoMunicipio [2], numeroZona [3]: OPTIONAL, absent)
   1b 0d 74 30 32 ... 7a       nomepacote   GeneralString "t02400-cp.jez"
   12 0c 32 30 32 36 ... 32    versao       NumericString (UNIVERSAL 18) "202609101622"
                               abrangencia  [1] OPTIONAL, absent
   0a 01 0e                    origem       Sistema ENUMERATED 14 = padaUE
```

Observe que o mesmo byte `0x81` significa `idProcessoEleitoral` dentro de `IDEleitoral` e significaria
`siglaUF` diretamente dentro de `IDPacote`. Tags de contexto só têm significado em relação ao
tipo que as contém. Sem o esquema, um dump BER não consegue distingui-las.

---

## 7. O que os esquemas dizem sobre "este é o software da urna real?"

Essa pergunta surge com as listas de hashes do TSE na raiz do projeto. `pc1hsfg.html` é
*Eleições Gerais 2026 — HOT SWAP FLASH — 02/09/2026*: digests SHA-512 de
`D:\Aplic\SEVIN\HSF\HotSwapFlash.exe`, `NotificadorHSF.exe`, um ícone e DLLs do MinGW.
`pc1geda.html` é o *GEDAI-UE*, o aplicativo do TSE que prepara as mídias da urna. Ele lista
`gedai-ue.exe`, DLLs do Qt 6 e os pacotes que grava nas mídias: `vota_apl.jez`,
`vota_ofi.jez`, `vota_sim.jez`, `vota_tre.jez`, `dsk*.img`, `vpe99_*.jez`…
[`docs/00-provenance.md`](../00-provenance.md) trata do argumento geral. A camada ASN.1 acrescenta
estes fatos:

1. **Mesmo modelo de dados e mesma base de código.** Os módulos do simulador são os que o TSE documenta para
   o BU e o RDV da urna real, com os mesmos nomes de tipos, membros, tags e faixas. Envelopes oficiais
   de BU impresso gravados em 2024 pelo software real da urna (`9.29.0.1 - Kayapó`) decodificam
   e recodificam exatamente com o esquema extraído deste wasm. Esse teste cobre os tipos de envelope,
   cabeçalho, identidade da urna e registro de carga; os corpos do BU e do RDV batem com o texto publicado,
   mas não foram testados contra arquivos de resultado reais (§5). O código gerado, o runtime
   III ASN.1 modificado pelo TSE e a nomenclatura `Modulo*` são os da urna.
2. **Uma versão posterior a 2024.** `envelopeZeresimaImpressa`, os valores renomeados de `TipoUrna`, o
   significado alterado de `MotivoApuracaoMistaComBU(5)` e o `ModeloEquipamento` mais curto não estão
   em nenhuma especificação publicada. Isso bate com `"10.23.0.1 - DESENVOLVIMENTO"`: um build de desenvolvimento do
   código do ciclo de 2026, não a versão lacrada.
3. **Não é a cadeia de carga real.** O `InformacaoMidia` da mídia diz que ela foi gerada por
   `"simulador-votacao-ng"` com um serial de TPM todo zerado. Uma mídia real é gerada pelo GEDAI-UE
   em uma máquina com TPM; as amostras de 2024 trazem `tsesevinw09` e um serial de TPM real. O
   estado da urna `eg.bin` é uma fixture de teste fixa no código (§4.4). O módulo de assinatura
   (`EntidadeAssinatura`, arquivos `.vscmr`) não está compilado, e as "assinaturas" `.vsu` são um
   texto fixo. Os **pacotes de entrada**, porém, trazem arquivos de assinatura no formato oficial
   (`*.vsc`, algoritmo `cepesc`). Os signatários são `SEVIN`, `SEINT`, `SECAD` e `SECINP`;
   `SEVIN` também é o diretório nos caminhos dos `pc1*.html` (`D:\Aplic\SEVIN\…`). Não foi verificado se essas
   assinaturas são válidas.
4. **Por que as listas de hashes não resolvem a questão.** `pc1hsfg.html` contém hashes apenas da ferramenta Windows *Hot Swap
   Flash* (7 arquivos e um "hash geral" combinado); não cita nenhum pacote da urna.
   `pc1geda.html` contém hashes do GEDAI-UE e dos pacotes `.jez`/`.img` que ele instala. Nenhum deles é,
   ou contém, este módulo wasm. Verificado: nenhum dos 78 digests dos dois arquivos (75
   distintos) é igual ao SHA-512 de algum dos 1,047 arquivos locais (o wasm, o glue, os pacotes de
   dados, o sistema de arquivos descompactado, os arquivos escritos em tempo de execução). Um build WebAssembly dos
   mesmos fontes nunca tem os mesmos bytes que o build nativo dentro de `vota_*.jez`. No
   nível do formato de dados a resposta é **sim**: a mesma base de código (`uenux2`, `ecourna`) e os mesmos
   formatos de arquivo. Se o código do aplicativo corresponde linha a linha à versão de 2026 não pode ser
   decidido a partir desses arquivos. No nível do binário a resposta é **não**: um build diferente,
   de desenvolvimento, com o hardware, a cadeia de carga e as assinaturas simulados.

---

## 8. Limites e pontos em aberto

* **Placeholders.** 11 nomes de CHOICE/ENUMERATED são placeholders (§3.6). Os *valores* das enumerações
  e os nomes dos membros são certos; apenas os identificadores dos tipos são adivinhados.
* **Módulos inferidos.** 28 tipos têm módulo inferido (as listas de IMPORTS são montadas a partir disso).
  Mover um tipo para outro módulo não altera um único byte codificado.
* **Nomes de aliases.** Aliases de inteiro/OCTET STRING não têm nome no binário. Os comentários `-- spec:`
  trazem os nomes publicados quando os arquivos do TSE os têm. Dois aliases de string são
  compartilhados por vários membros: @1913468 `GeneralString (SIZE(0..18))` (códigos de candidato, 4 membros)
  e @1913028 `NumericString (SIZE(12))` (2 membros). Eles têm sua própria tabela, mas nenhum nome. A classe RTTI `ModuloTiposEleitorais::VersaoMontagem` é provavelmente
  @1913028, `NumericString (SIZE(12))`, usada por `CabecalhoPacote.versao` e
  `EntidadeChave.tagChaves`. Seu único construtor copia de outro objeto (func 12110), então
  isso não está provado.
* **Módulos não usados.** Só são recuperados os tipos alcançáveis a partir de uma tabela compilada. Módulos inteiros
  que o VOTA não linka (municipios/zonas, seções, o módulo de assinatura, entidades exclusivas de SA/RED/GEDAI)
  estão ausentes. A existência deles só é conhecida pelos arquivos (§6) e pela documentação publicada.
* **Padrão de tags.** `IMPLICIT TAGS` é inferido: todas as tags fora de CHOICE são implícitas, a menos que marcadas como
  explícitas, e os arquivos publicados o usam. A saída do `asnparser` seria a mesma para um módulo
  que escrevesse `IMPLICIT` em cada tag.

---

## 9. Reproduzir

```sh
# 1. memory after static constructors (needs node; ~3 s)
node src/asn1/tools/dump_heap.mjs /tmp/vota_heap.bin
# 2. rebuild src/asn1/*.asn (scan + name + print); --json dumps the model
python3 src/asn1/tools/extract_asn1.py --heap /tmp/vota_heap.bin --out src/asn1
# 3. decode one file with the recovered tables (all candidate types, or a given one)
python3 src/asn1/tools/extract_asn1.py --heap /tmp/vota_heap.bin \
        --decode upstream/fs/municipal-t1/dsk/fi/estatico/t02411ac-ce.dat EntidadeEleicao
# 4. independent check with asn1tools (pip install asn1tools)
python3 src/asn1/tools/verify_asn1tools.py
```

Comandos `q.py` úteis para este capítulo:

```
q.py mem 1144144 52        # SEQUENCE table of EntidadeBoletimUrna (name at +48)
q.py mem 1625264 44        # its member names
q.py mem 1140568 36        # CHOICE table IDEleitoral (tags [1] [2] [3] at 1621776)
q.py mem 1143720 24        # ENUMERATED table of the BU TipoVoto (max 5, names at 1624672)
q.py f 10273               # BU converter: builds EntidadeBoletimUrna, hash-chain header format
q.py f 5825                # CFileASN::DecodeObjectFunction<EntidadeRegistroDigitalVoto>
```

As funções nomeadas a partir deste trabalho (as entradas `create` da §3.3 e a func 10273 = conversor do BU) estão
nos resultados estruturados para o coordenador. As funções do runtime em si (visitors,
codificadores, construtores) são tratadas em [`docs/libraries/asn1-runtime.md`](../libraries/asn1-runtime.md).

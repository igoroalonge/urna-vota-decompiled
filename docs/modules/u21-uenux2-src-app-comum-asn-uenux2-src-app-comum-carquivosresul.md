# u21 — `comum::asn`: o framework de conversores ASN.1 da aplicação de votação, seus conversores e helpers, e as tabelas de nomes de arquivos de resultado / SAVD

A unidade u21 tem **103 funções**; **36** foram vistas em execução durante os votos gravados (`analysis/runtime/*.functions.tsv`).
Seus arquivos originais atestados são:

| arquivo original | o que contém |
|---|---|
| `uenux2/src/app/comum/asn/iconversorasn.h` | o template de classe `comum::asn::IConversorASN<ENTIDADE, DADO>`: base de todo conversor entre um objeto ASN.1 e um objeto de dados da aplicação |
| `uenux2/src/app/comum/asn/iconversorbiometriaasn.h` | a mesma ideia para digitais cifradas (`IConversorBiometriaASN`, `md::CParametro` extra) |
| `uenux2/src/app/comum/asn/legravaentidade.h` | `LeEntidadeEm` / `LeEntidadeBiometria`: leem UM elemento BER em um offset de bytes de um arquivo grande |
| `uenux2/src/app/comum/asn/util.cpp` | `comum::asn::Utils`: conversões estáticas de enum / data (fase, abrangência, tipo de local, tipo de pacote, sistema…) |
| `uenux2/src/app/comum/asn/cconversorabrangencia.cpp` | `CConversorAbrangencia` (abrangência de um arquivo de dados) |
| `uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp` | `CConversorCabecalhoEntidade` (cabeçalho de todo arquivo de dados / resultado, incluindo o BU) |
| `uenux2/src/app/comum/carquivosresultado.cpp` | `CArquivosResultado::operator[]`: sufixo de cada arquivo de resultado (`bu.dat`, `rdv.dat`, `hash.dat`, …) |
| `uenux2/src/app/comum/carquivossavd.cpp` | `CArquivosSavd::operator[]`: caminho de cada arquivo assinado / pacote de assinatura |

Como o template é instanciado e inlinado em toda parte, a unidade também recebeu métodos de conversores de cerca de
vinte outras classes `comum::asn::CConversorXxx` (suas vtables dão os nomes reais), duas funções de fotos de
`cfotos.cpp`, uma fonte de dados de GUI de `ccandidaturas.cpp`, dois construtores de `md::CComplementoMunicipio` e
uma dúzia de helpers do runtime ASN.1 / libc++. A seção 14 mapeia cada índice.

---

## 1. Glossário

| termo | significado |
|---|---|
| *conversor* | objeto que converte entre uma *entidade* ASN.1 e um *dado* da aplicação |
| *entidade* (`TEntidade`) | uma classe gerada a partir dos módulos ASN.1 do TSE (`ModuloTiposEleitorais::CabecalhoEntidade`, …) sobre o runtime ASN.1 da III (`docs/libraries/asn1-runtime.md`); codificada em BER nos arquivos |
| *dado* (`TDado`) | o objeto de dados da aplicação (`comum::md::CCabecalhoEntidade`, `md::CAbrangencia`, …), frequentemente uma classe de valor com verificação de intervalo |
| **Converte** / **Desconverte** | dado → entidade (para GRAVAR um arquivo) / entidade → dado (depois de LER um arquivo). O ecourna grafa o segundo como *Deconverte* |
| *DoConverte* / *DoDesconverte* | os métodos virtuais que fazem o trabalho (slots 2 e 3 da vtable), sobrescritos por cada conversor concreto |
| *cabeçalho* | cabeçalho de um arquivo: data-hora de geração + id eleitoral (`IDEleitoral`: processo eleitoral / pleito / eleição) |
| *abrangência* | escopo geográfico de um arquivo de dados: municipal / estadual / federal (+ UF e município) |
| *fase* | fase da instalação: **oficial** (eleição real), **simulado** (simulação pública), **treinamento**. A aplicação a armazena como o caractere `'1'`/`'2'`/`'3'` (`EUrnaFase`); o ASN.1 a numera simulado 1, oficial 2, treinamento 3 |
| *pacote* | um pacote de dados produzido pelos sistemas do TSE e importado pelo sistema de carga; `CabecalhoPacote` o descreve |
| *carga* / *flash de carga* | a preparação de uma urna com os dados da eleição, e a mídia usada para isso |
| *seção agregada* | uma seção pequena incorporada a esta: os eleitores dela votam nesta urna |
| *suplência* | o(s) substituto(s) / vice associados a um cargo (vice-prefeito, 1º/2º suplente de senador) |
| *resultado* / *MR* | os arquivos de resultado gravados no fim do dia / *Mídia de Resultado*, a mídia removível levada ao ponto de transmissão |
| **SAVD** | o serviço de assinatura / verificação da urna (arquivos `*.vsu` na urna, `*.vsc` na MR); expansão inferida ("Serviço de Assinatura e Verificação Digital") |
| **BU** | *boletim de urna*, o resultado da seção (`-bu.dat`); **RDV** *registro digital do voto* (`-rdv.dat`) |
| *jufa* | "justificativas e faltosos": o arquivo de comparecimento `-jufa.dat` (`EntidadeResultadoUrnaCadastro`) |
| *zerésima* | o relatório impresso antes do início da votação que prova que todo contador está zerado (`-imgze.dat` = sua imagem) |

---

## 2. Classes e como se relacionam (RTTI)

```
comum::asn::IConversorASN<ENTIDADE, DADO>                typeinfo only (no vtable of its own); iconversorasn.h
 │   vtable of every subclass: [0] ~T (ICF 174 "return this")  [1] deleting ~T (144 free)
 │                             [2] DoConverte(const TDado&)    [3] DoDesconverte(const TEntidade&)
 ├─ CConversorCabecalhoEntidade       <CabecalhoEntidade, CCabecalhoEntidade>          11589 / 11588   (comum/asn)
 ├─ CConversorAbrangencia             <Abrangencia, CAbrangencia>                      11462 / 11461   (comum/asn)
 ├─ CConversorCabecalhoPacote         <CabecalhoPacote, CCabecalhoPacote>              11438 / 11437*  (comum/asn, inferred)
 ├─ CConversorDadosDisponiveisCarga   <DadosDisponiveisCarga, CDadosDisponiveisCarga>  11424 / 11423
 ├─ CConversorMunicipio               <Municipio, CMunicipio>                          11379 / 11378*
 ├─ CConversorComplementoMunicipio    <ComplementoMunicipio, CComplementoMunicipio>    11383 / 11382
 ├─ CConversorSecaoEleitoral          <SecaoEleitoral, CSecaoEleitoral>                11454 / 11453*
 ├─ CConversorIdentificacaoAgregada   <IdentificacaoAgregada, CIdentificacaoAgregada>  11456* / 11455
 ├─ CConversorDadoLocal               <DadoLocal, CDadoLocal>                          11403 / 11404*
 ├─ CConversorEntidadePartidos        <EntidadePartidos, CEntidadePartidos>            11458 / 11457
 ├─ CConversorCandidaturas            <EntidadeCandidatos, vector<CCandidatura>>       11444 (base) / 11445
 ├─ CConversorCandidatura             <Candidatura, CCandidatura>  (+cargo, partido, apto)  11446 (base) / 11448*
 ├─ CConversorSuplencia               <Suplencias, CSuplencia>                         11358 (base) / 11359
 ├─ CConversorEntidadeHashes          <EntidadeHashes, CEntidadeHashes>                10266 / 10265 (base)
 ├─ CConversorVersoesArquivos         <EntidadeVersaoArquivos, CVersoesArquivos>       10264 / 10263*
 ├─ CConversorRegistroDigitalVoto<CConversorEleicoesVota>
 │                                    <EntidadeRegistroDigitalVoto, CVotosEleicoesVota> 11483 / 11482  (own ~T 11485/11484)
 ├─ CConversorEntidadeBU, CConversorHistoricoVotoImpresso: only the base DoDesconverte (10272, 10275) is here
 └─ … 14 more classes whose only method in this unit is the base-class DoConverte thunk (list in §4.4)
                                                            (* = method in another unit)
comum::asn::IConversorBiometriaASN<BiometriaEleitorCifrada, CBiometriaEleitor>
 └─ CConversorBiometriaEleitorCifrada  [2] 11418 (base DoConverte, throws)  [3] 11419 (other unit)

ecourna::api::exception::CBaseError<comum::EUeComumAsnError, SErrorLimits{7650, 7750}> : CError
    typeinfo 1528108, vtable 1528128, constructor thunk func 255 (shared body 2294)
ecourna::api::exception::CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}>   (CArquivosResultado/Savd)
    typeinfo 1551372, constructor thunk func 480
```

Todo conversor é sem estado (`sizeof` 4, só o vptr) e é construído na pilha do chamador, exceto
`CConversorCandidatura` (cargo, partido, apto: 12 bytes), `CConversorEleicoesVota` (um map de cargos) e
`CConversorRegistroDigitalVoto` (os campos de cabeçalho do RDV, 56 bytes), que são configurados uma vez pelo código de inicialização.

Classes que não são conversores tocadas pela unidade: `comum::CArquivosResultado` (sem estado), `comum::CArquivosSavd`
(dois `std::map`s), `comum::CFotos` (singleton do índice de fotos), `api::CDataImage<comum::CCandidaturasDSFoto>`
(subclasse de `api::IImage`: `[0] 174 [1] 144 [2] 12641 GetImage`).

---

## 3. O que o subsistema faz no processo de votação

A urna lê todos os seus dados da eleição de arquivos ASN.1 codificados em BER preparados pelo TSE (candidatos `-ca.dat`, partidos
`-pa.dat`, fotos `-fo.dat`, complementos de municípios `-cm.dat`, o local `-lo.dat`, a configuração da eleição, os
eleitores…) e grava o seu próprio estado (`eg.bin`, `vota.bin`, `sa.bin`, `gap.bin`), o RDV e, no fim do dia,
os arquivos de resultado (BU, RDV, jufa, hashes, imagens, versões). Entre a camada de arquivos (`api::CFileASN`, que só
decodifica/codifica BER) e os objetos da aplicação (`comum::md::*`) fica esta camada de conversores:

```
 file bytes ──CFileASN::DecodeObject──▶ TEntidade ──IConversorASN::Desconverte──▶ TDado   (reading data files)
                                          │ 1. isValid() && isStrictlyValid()  (every ASN.1 constraint) else 7654
                                          │ 2. DoDesconverte (vtable slot 3) — may throw md-level errors
 TDado ──IConversorASN::Converte──▶ TEntidade ──CFileASN::CodeObjectFunction──▶ file bytes (writing state/results)
            1. DoConverte (slot 2)
            2. isValid() && isStrictlyValid() else 7653 "Entidade deixada em estado inválido"
```

Portanto nada que a urna grava pode violar o esquema (uma contagem acima do limite do INTEGER, uma string mais longa que a restrição
SIZE, um CHOICE obrigatório ausente) sem uma exceção, e nada que ela lê chega à aplicação a menos que
satisfaça o esquema. Os construtores md então acrescentam verificações semânticas (por ex. `md::CAbrangencia`: "UF informada para
abrangência federal").

---

## 4. O template `IConversorASN` (iconversorasn.h)

### 4.1 Contrato (reconstrução: `src/uenux2/src/app/comum/asn/iconversorasn.h`)

| método | linha | código | mensagem | wasm |
|---|---|---|---|---|
| `TEntidade Converte(const TDado&) const` | 56 | 7653 | `Entidade deixada em estado inválido: {}` | corpo mesclado **1563**; cópia com tratamento de exceções **2893** |
| `TDado Desconverte(const TEntidade&) const` | 71 | 7654 | `Entidade está inválida: {}` | corpo mesclado **1008** |
| `TDado Desconverte(const std::vector<uebyte>&) const` (sobrecarga, nome inferido) | — | — | decodifica os bytes, depois a verificação da linha 71 | **5680** (somente RDV) |
| `virtual TEntidade DoConverte(const TDado&) const` (padrão) | 89 | 7655 | `Método DoConverte() não implementado para {}` | corpo mesclado **731** |
| `virtual TDado DoDesconverte(const TEntidade&) const` (padrão) | 98 | 7656 | `Método DoDesconverte() não implementado para {}` | corpo mesclado **731** |

O `{}` das duas primeiras é o texto de `ASN1::trace_invalid(stream, typeid(TEntidade).name(), entidade)`: o nome bruto
do tipo com mangling (por ex. `N21ModuloTiposEleitorais17CabecalhoEntidadeE`) seguido do relatório em português do
InvalidTracer (caminho pontuado do campo com problema + motivo). O `{}` das duas últimas é o nome com mangling.

Diferenças em relação ao gêmeo do ecourna (`ecourna::api::asn::IConversorASN`, unidade u11): os padrões não são virtuais
puros (um arquivo só de leitura simplesmente deixa `DoConverte` de lado, um só de gravação `DoDesconverte`), o enum de erro é
`EUeComumAsnError` (7650..7750) em vez de `EApiAsnError` (1900..), `std::ostringstream` em vez de
`std::stringstream`, e nenhum sufixo `": "` é acrescentado ao nome do tipo.

### 4.2 Códigos de erro de `comum::EUeComumAsnError` vistos nesta unidade e em suas vizinhas

| código | onde | mensagem |
|---|---|---|
| 7650 | CConversorAbrangencia::DoConverte (cconversorabrangencia.cpp:45) | `Tipo inválido [{}]` |
| 7651 | CConversorCabecalhoEntidade::DoConverte (:35) | `Tipo inválido [{}]` |
| 7652 | CConversorCabecalhoEntidade::DoDesconverte (:56) | `Tipo de id de cabeçalho inválido` |
| 7653 / 7654 / 7655 / 7656 | IConversorASN (:56 / :71 / :89 / :98) | ver §4.1 |
| 7658 / 7659 | IConversorBiometriaASN (:62 / :81) | `Entidade está inválida: {}` / `Método DoConverte() não implementado para {}` |
| 7667 / 7668 | LeEntidadeEm (legravaentidade.h:92 / :98) | `CLeitorASN::{} - não foi possível fazer o seek em {}` / `… ler a entidade de {}` |
| 7669 / 7670 | LeEntidadeBiometria (:119 / :125) | as mesmas duas mensagens |
| 7671 | Utils::DesconverteDataHoraJE (util.cpp:41) | `Formato de data inválido.` |
| 7674 / 7675 | Utils::ConverteFase (:102) / DesconverteFase (:116) | `Fase inválida: {}` |
| 7678 | Utils::ConverteTipoPacote (:231) | `Tipo de pacote inválido: {}` |
| 7680 | Utils::ConverteIdPacote (:344) | `Tipo IDEleitoral inválido.` |
| 7681 | Utils::ConverteIdSistema (:419) | `Sistema inválido: {}` |
| 7685 / 7686 | Utils::ConverteAbrangencia (:513) / DesconverteAbrangencia (:498) | `Abrangência inválida: {}` |
| 7687 / 7688 | Utils::ConverteTipoLocalVotacao (:530) / DesconverteTipoLocalVotacao (:549) | `Tipo de local de votação inválido: {}` |

(7673, 7679, 7682, 7684, 7689, 7690 são os `Desconverte*` de util.cpp inlinados em funções de outras unidades.)

### 4.3 Como o template aparece no wasm

* **merge-similar-functions.** Todas as instanciações fora de linha de `Converte` viraram thunks de 20 bytes
  `1563(result, this, dado, &srcloc(:56), typeid-name)`, todos os thunks de `Desconverte` chamam `1008(…, &srcloc(:71), name)`,
  e todos os `DoConverte`/`DoDesconverte` padrão são thunks de 29 bytes
  `731(result, this, arg, &srcloc(:89|:98), 7655|7656, fmt_ptr2, fmt_ptr1, name)`. A string de formato viaja como dois
  ponteiros para dentro do literal (2923 e 2967 para "Método DoConverte() não implementado para {}"; 2875/2922 para o
  texto de DoDesconverte), de modo que a 731 mesclada a formata em tempo de execução (`__vformat_to` diretamente, sem
  `__try_constant_folding`).
* **Dois modelos de exceção.** A 2893 é o mesmo `Converte` que a 1563 compilado *com* landing pads `invoke_*` (no unwind
  ela executa `~ostringstream`, `~string`, `__cxa_free_exception` e `ASN1::SEQUENCE::~SEQUENCE` sobre a entidade
  meio construída). Seus únicos usuários são os quatro thunks de EstadoGeral (9997 vota.bin, 10019 sa.bin, 10051 gap.bin, 10170
  eg.bin): o linker manteve a cópia de uma unidade de tradução compilada com suporte a exceções. Toda outra
  instanciação (1563/1008 e todas as cópias inlinadas) não tem landing pads: uma exceção vaza as strings locais
  e os objetos ASN.1 (ver u11 §4.2 para o mesmo padrão).
* **Entidades CHOICE / ENUMERATED não são mescladas.** Para `CodigoCargoConsulta` (um CHOICE) o teste de validade é
  `CHOICE::isValid` (1068) + `CHOICE::isStrictlyValid` (1067) (func 5828); para o ENUMERATED
  `TipoIdentificadorEleitor` ele é inlinado como `value > info->maxEnumValue` (func 5684). O mesmo vale para
  `Eleicoes` (CHOICE) em 11482/11483.

### 4.4 Armadilhas de nomes: funções nomeadas a partir de um método de template inlinado

Um registro `std::source_location` pertence à função que o contém depois do inlining. Por isso a ferramenta nomeou
14 métodos de conversores a partir do `Converte`/`Desconverte` que eles inlinam. O slot da vtable dá o nome real:

| func | nome na ferramenta (método inlinado) | identidade real | evidência |
|---|---|---|---|
| 10266 | `IConversorASN<ArquivoAssinatura, CHashArquivo>::Converte` | `CConversorEntidadeHashes::DoConverte` | vtable @1599092 slot 2 |
| 11382 | `IConversorASN<HorarioVerao, CHorarioVerao>::Desconverte` | `CConversorComplementoMunicipio::DoDesconverte` | vtable @1569480 slot 3 |
| 11383 | `IConversorASN<HorarioVerao, CHorarioVerao>::Converte` | `CConversorComplementoMunicipio::DoConverte` | slot 2 |
| 11424 | `IConversorASN<CabecalhoPacote, CCabecalhoPacote>::Converte` | `CConversorDadosDisponiveisCarga::DoConverte` | vtable @1565796 slot 2 |
| 11454 | `IConversorASN<IdentificacaoAgregada, …>::Converte` | `CConversorSecaoEleitoral::DoConverte` | vtable @1561712 slot 2 |
| 11457 | `IConversorASN<Partido, CPartido>::Desconverte` | `CConversorEntidadePartidos::DoDesconverte` | vtable @1561344 slot 3 |
| 11458 | `IConversorASN<Abrangencia, CAbrangencia>::Converte` | `CConversorEntidadePartidos::DoConverte` | slot 2 |
| 11482 | `IConversorASN<Eleicoes, CVotosEleicoesVota>::Desconverte` | `CConversorRegistroDigitalVoto<CConversorEleicoesVota>::DoDesconverte` | vtable @1560304 slot 3 |
| 11483 | `IConversorASN<Eleicoes, CVotosEleicoesVota>::Converte` | `CConversorRegistroDigitalVoto<…>::DoConverte` | slot 2 |
| 5710 | `IConversorASN<Candidatura, CCandidatura>::Desconverte` | helper que acrescenta uma lista de candidatos (`DesconverteLista`, nome inferido) | assinatura `(vector&, converter, SEQUENCE OF)`, laço por índice + push_back |
| 5680 | `IConversorASN<EntidadeRegistroDigitalVoto, …>::Desconverte` | a sobrecarga `Desconverte(const std::vector<uebyte>&)` | decodifica os bytes primeiro (5825), os chamadores passam bytes de rdv.dat |
| 3755 | `(anonymous)::LeEntidadeEm` | `comum::CFotos::GetImagem` (sem `this`: estática, ou o seu `this` não usado foi removido) | string de contexto "CFotos::GetImagem(id)", registros cfotos.cpp:39/51 |
| 11423 | `CConversorDadosDisponiveisCarga::vf3` (candidato a nome: ctor de `md::CDadosDisponiveisCarga`) | `CConversorDadosDisponiveisCarga::DoDesconverte` com o construtor md inlinado | slot 3 |
| 11438 | `CConversorCabecalhoPacote::vf2` (candidatos `Utils::Converte*`, `CIDPacote::UF…`) | `CConversorCabecalhoPacote::DoConverte` com Utils/CIDPacote inlinados | slot 2 |

Thunks do **padrão da classe base** (→ 731) nesta unidade (para a maioria destas classes é a sua única função aqui;
Suplencia, Candidaturas e Candidatura também têm 11359, 11445 e o construtor 5712): `DoConverte` de
`CConversorSuplencia` (11358), `CConversorSituacoesEleicoes` (11360), `CConversorProcessoEleitoral` (11362),
`CConversorPleito` (11364), `CConversorNomesCargo` (11368), `CConversorEleicaoPE` (11370), `CConversorCargo` (11372),
`CConversorDetalheCandidato` (11374), `CConversorDetalheConsulta` (11376), `CConversorComplementosMunicipios`
(11380), `CConversorImpedido` (11409), `CConversorDedo` (11415), `CConversorBiometriaEleitor` (11421),
`CConversorFotoCandidato` (11439), `CConversorCandidaturas` (11444), `CConversorCandidatura` (11446),
`CConversorDadosCandidato` (11449) — todos dados só de leitura; e `DoDesconverte` de `CConversorEntidadeHashes`
(10265), **`CConversorEntidadeBU` (10272)** e **`CConversorHistoricoVotoImpresso` (10275)** — resultados só de gravação.

---

## 5. `comum::asn::Utils` (util.cpp)

Tudo estático. Mapeamentos (tabelas lidas do segmento de dados):

| função | direção | mapeamento / verificação | wasm |
|---|---|---|---|
| `DesconverteDataHoraJE` | `"YYYYMMDDTHHMMSS"` → `api::CDateTime` | `substr(0,8)` → `DataJE` → `DesconverteDataJE` (2276); `substr(8,1) != "T"` → 7671; `substr(9,6)` → `HoraJE` → `api::CTime` (3643, "Hora inválida") | 1713 |
| `ConverteDataHoraJE` | `CDateTime` → texto | `CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")` | 1080 (outra unidade) |
| `ConverteFase` | `EUrnaFase` → `Fase` | `'1'`→oficial(2), `'2'`→simulado(1), `'3'`→treinamento(3) (tabela @497848); senão 7674 | 1712 |
| `DesconverteFase` | `Fase` → `EUrnaFase` | 1→`'2'`, 2→`'1'`, 3→`'3'` (tabela @497860); senão 7675 | 2843 |
| `ConverteTipoPacote` | `md::ETipoPacote` 0..32 → `TipoPacote` | tabela @497872 = {1,2,5,7,8,9,10,13,14,15,16,17,18,21,…,37,39,40,48}; ≥33 → 7678. O enum ASN.1 tem 56 valores até 65: 23 deles (41..47, 49, 50, 52..65) são inalcançáveis a partir do enum md | inlinada em 11438 |
| `ConverteIdPacote` | `md::CIDPacote` → `IDPacote` | alternativa de IDEleitoral = tipo−1 (1..3 senão 7680), fase, depois cada opcional (UF / município / zona) incluído somente se presente | inlinada em 11438 |
| `ConverteIdSistema` | `ESistemaJE` 1..12 → `Sistema` | {1,2,3,**13**,**12**,6,7,8,9,10,11,**14**} (tabela @498196): 4→simon, 5→seweb, 12→padaUE; senão 7681 | inlinada em 11438 |
| `DesconverteAbrangencia` / `ConverteAbrangencia` | identidade municipal 0 / estadual 1 / federal 2 | ≥3 → 7686 / 7685 | 5827 / inlinada em 11462 |
| `ConverteTipoLocalVotacao` / `DesconverteTipoLocalVotacao` | identidade normal 1 / emTrânsito 2 / presoProvisório 3 / temporário 4 | fora de 1..4 → 7687 / 7688 | 3801 / 3800 |

Detalhe de formatação: `EUrnaFase` (slot 2285), `md::ETipoPacote` / `ESistemaJE` (2286/2287, em 11438) e
`ETipoLocalVotacao` (2290) são formatados por especializações de `std::formatter` (tipo de argumento de formato 15 "handle",
corpo 536), e o mesmo vale para o `TipoAbrangencia::NamedNumber` ASN.1 de `DesconverteAbrangencia` (slot 2289, corpo 1711).
Os valores lidos de um objeto ASN.1 (`Fase`, `TipoLocalVotacao` via `asInt()`) e `md::ETipoAbrangencia` são
formatados como `int` simples (tipo 3).

---

## 6. Os conversores desta unidade (o que leem e gravam)

Os números de campo seguem a ordem da SEQUENCE de `src/asn1/*.asn`.

* **`CConversorCabecalhoEntidade`** (11589 / 11588). Gravação: `dataGeracao = ConverteDataHoraJE(dado.dataGeracao)`;
  `tipo` (+16) ≥ 3 → 7651; `idEleitoral.select(tipo, INTEGER(0..99999)) = id` (+12). Leitura: `choiceID` (sem sinal) ≥ 3
  → 7652 (também pega um CHOICE não selecionado, −1), depois `CCabecalhoEntidade(DesconverteDataHoraJE(…), valor, tipo)`
  (1945). Observado: todo cabeçalho de arquivo de dados lido em `votaInit`.
* **`CConversorAbrangencia`** (11462 / 11461). Gravação: tipo via `ConverteAbrangencia`; municipal → `id{siglaUF,
  codigoMunicipio}`, estadual → `id{siglaUF}` (município omitido), federal → sem `id`; padrão → 7650 (morto na
  prática: o `ConverteAbrangencia` inlinado já lançou 7685 para tipo ≥ 3). Leitura:
  `id` ausente → `CAbrangencia(tipo, "", 0)`; município ausente → `(tipo, UF, 0)`; a UF é copiada pela sua
  string C. `md::CAbrangencia` (3739) então impõe a combinação.
* **`CConversorCabecalhoPacote::DoConverte`** (11438): tipoPacote, idPacote, nome, versão, origem; o opcional
  `abrangencia` de `CabecalhoPacote` **nunca é gravado** (md::CCabecalhoPacote não tem esse membro).
* **`CConversorDadosDisponiveisCarga`** (11424 / 11423), a descrição do que a mídia de carga oferece, lida apenas pelo
  relatório "versão dos pacotes" (`vota::CImpressaoVersaoPacotes`, 11905). A leitura monta, por município,
  `CInfoMunicipio(código, nome, fuso, comBiometria [, horário de verão])` + as suas seções, e os cabeçalhos dos pacotes
  importados; depois as verificações do construtor `md::CDadosDisponiveisCarga` inlinado (8040..8048): processo ≠ 0, pleito ≠ 0,
  **serial da mídia de carga com exatamente 8 caracteres**, país / sigla UF / nome UF não vazios, zona ≠ 0, pelo menos um
  município. `comBiometria` = `Municipio.comBiometria` se presente, senão false.
* **`CConversorMunicipio::DoConverte`** (11379): código, nome, **`capital` sempre FALSE**, `codigoIBGE` ausente,
  `comBiometria` incluído.
* **`CConversorComplementoMunicipio`** (11383 / 11382): código, fuso (−720..720 minutos, verificado por
  `CComplementoMunicipio::ValidaCriacao`), período opcional de horário de verão por meio de `CConversorHorarioVerao`.
  **`desligaColetaBiometria` não é lido nem gravado** (o objeto md não tem campo para ele; os construtores 3710 /
  3711 não armazenam nada em +6).
* **`CConversorSecaoEleitoral::DoConverte`** (11454): tipo, `IdentificacaoSecaoEleitoral{município, zona, local,
  seção}`, e as seções agregadas (omitidas quando o vetor está vazio, senão cada uma convertida por
  `CConversorIdentificacaoAgregada`). **`CConversorIdentificacaoAgregada::DoDesconverte`** (11455): número + tipo
  de local de origem → `CIdentificacaoAgregada` (ctor 5672: +0 tipo, +4 número).
* **`CConversorDadoLocal::DoConverte`** (11403), parte de `eg.bin`: tipo de local, UF, e a seção por meio de
  `CConversorLocalidadeEleitoral`. Observado (o mock web grava eg.bin durante `votaInit`).
* **`CConversorEntidadePartidos`** (11458 / 11457), `-pa.dat`: cabeçalho, abrangência, lista de
  `CPartido{número, sigla, nome}` (28 bytes). Observado (lido em `votaInit`).
* **`CConversorCandidaturas::DoDesconverte`** (11445), `-ca.dat`: para cada `CandidatosPorCargos`: código do cargo (CHOICE
  `CodigoCargoConsulta`, 5828); se `partidos` estiver presente, para cada partido duas passagens com
  `CConversorCandidatura(cargo, partido, apto = true)` sobre `candidatosAptos` e `(…, false)` sobre
  `candidatosInaptos` (5712 + 5710). `cabecalho`, `abrangencia` e `quantidadeVagas` não são usados aqui.
* **`CConversorSuplencia::DoDesconverte`** (11359): `CSuplencia{ordem (field 1), temFoto (field 3), nomes}`; o código
  do cargo (campo 0) é descartado. (Isso corrige o palpite da u03 "+0 codigo, ordem ?": +0 é `ordem`, +1 `temFoto`.)
* **`CConversorEntidadeHashes::DoConverte`** (10266; o próprio `CConversorHashArquivo` é reconstruído pela u17 em
  `comum/asn/cconversorhasharquivo.u17.cpp`), **`CConversorVersoesArquivos::DoConverte`** (10264),
  **`CConversorRegistroDigitalVoto`** (11483 / 11482): arquivos de resultado, ver §9.

---

## 7. Leitura de um elemento de um arquivo grande: `legravaentidade.h`, fotos de candidatos

`LeEntidadeEm<CONVERSOR>(arquivo, CIndexer{offset, tamanho}, contexto)` (legravaentidade.h:92/98) abre o arquivo
(`CFile(arquivo, "rb")`), faz seek até `offset` (falha → 7667), lê exatamente `tamanho` bytes (leitura incompleta → 7668),
decodifica-os com `api::CFileASN::DecodeObjectFunction<TEntidade>(buffer, "DecodeObject de <typeid>")`
(cfileasn.h:135 "Conteúdo não foi decodificado para {}: {}" 5953, :143 "Conteúdo inválido para {}: {}" 5954) e
devolve `CONVERSOR().Desconverte(entidade)`. `LeEntidadeBiometria` (:119/:125, códigos 7669/7670) é igual, com um
`CParametro` para a decifração da digital; ela é inlinada em `CEleitorDetalhe::GetBiometria` (1937, outra unidade).

A única instância de `LeEntidadeEm` é inlinada em **`CFotos::GetImagem(id)`** (3755):

```
CFotos::GetInst()                     singleton @1838844 (created on first use: operator new(28) + ctor 3756)
  m_fotos.find(id)                    map<codigo, SIndiceFoto{id, CIndexer{offset, tamanho}, arquivo}>
    not found → CUeComumDadosError 7861 "{} - id de foto [{}] não encontrado"          (cfotos.cpp:51)
  api::ExisteArquivo(arquivo)         stat() + S_ISREG (func 412 → 6019)
    missing  → 7860 "{} - arquivo [{}] da foto [{}] não encontrado"                   (cfotos.cpp:39)
  LeEntidadeEm<CConversorFotoCandidato>(arquivo, indexer, "LeFotoCandidato(" + id + ")")
  return foto.GetFoto().GetImagem()   copy of the JPEG bytes
```

Ela tem dois chamadores. A fonte de imagem da tela de votação **`CDataImage<CCandidaturasDSFoto>::GetImage`** (12641):
candidatura atual (`GetCandidaturaAtual("CCandidaturasDSFoto")`), titular (`m_indice == 0`) ou suplente
`m_indice`, busca do código do candidato em `CFotos`; se encontrado, chama `GetImagem` uma vez para verificar os marcadores JPEG
(`at(0)==FF, at(1)==D8`, dois últimos bytes `FF D9`) e **a chama uma segunda vez** para devolver a imagem. Toda foto
desenhada é, portanto, aberta, lida e decodificada em BER duas vezes (mais duas validações). Foto ausente ou não JPEG → vetor
vazio (nada é desenhado). Confirmado em tempo de execução: com `FS.open` interceptado em uma cópia de rascunho de `tools/run/headless.mjs`, digitar para Vereador
`91001` abriu `t02411ac00001-fo.dat` duas vezes depois do 5º dígito e mais duas vezes quando a tela se estabilizou em
`CConfirmaVotoNominal` (um par por desenho). O segundo chamador, a imagem de
`vota::CTelasVota::CriaTelaVisualizacaoCandidato` (`CDataImage<std::__bind<…>>::GetImage`, 12520, um corpo de 16 bytes),
chama `GetImagem` uma vez, sem a verificação de JPEG. Os dois chamadores chamam `CFotos::GetInst()` (2819) logo antes e descartam
o resultado.

---

## 8. Tabelas de nomes de arquivos: `CArquivosResultado`, `CArquivosSavd`

**`CArquivosResultado::operator[](EExtensaoArquivoResultado)`** (347, carquivosresultado.cpp:99) é um switch puro
sobre um singleton sem estado (`GetInst` = func 348 → 2900, outra unidade; o `this` não usado foi removido pela
eliminação de argumentos mortos do wasm-opt). Valor desconhecido → `CUeComumError` 7200
"Extensão de arquivo de resultado associada ao identificador {} não encontrada".

| id | sufixo | | id | sufixo | | id | sufixo |
|---|---|---|---|---|---|---|---|
| 1 | `vota.vsc` | | 8 | `jufa.dat` | | 15 | `logsa.jez` |
| 2 | `sa.vsc` | | 9 | `imgbu.dat` | | 16 | `wsqbio.jez` |
| 3 | `red.vsc` | | 10 | `imgbusa.dat` | | 17 | `wsqman.jez` |
| 4 | `bu.dat` | | 11 | `imgze.dat` | | 18 | `wsqmes.jez` |
| 5 | `busa.dat` | | 12 | `hash.dat` | | 19 | `mr.ver` |
| 6 | `rdv.dat` | | 13 | `log.jez` | | 20 | `asw.vsc` |
| 7 | `rdvred.dat` | | 14 | `log.jez` (igual ao 13) | | 21 | `ahw.vsc` |

O nome completo é `<fase><pleito:05><UF><município:05><zona:04><seção:04>-<suffix>` (montado por
`CGravadorUtil::DeterminaNomeArquivoSemLetra` / `CCopiaResultadoParaMR`, u08/u09).

**`CArquivosSavd`** (36 bytes) contém `std::map<ESavdPacote, std::string>` (+0, ids 120..203),
`std::map<ESavdArquivoUE, std::string>` (+12, ids 25..115) e `std::map<ESavdAplicacao, std::string>` (+24, nunca
lido), preenchidos pelo construtor do singleton inlinado em `GetInst` (func 1164, 77 KB; tabela completa reconstruída pela u22
em `src/uenux2/src/app/comum/carquivossavd.u22.cpp`: `CPath::GetPathTrab/GetPathDinamico/GetPathResult` + nomes como
`uenux.vsu`, `rdv.vsu`, `bu.vsu`, `eg.vsu`, e o prefixo de resultado `"{:c}{:05}{}{:05}{:04}{:04}-"` + um
sufixo de `CArquivosResultado`). `operator[](ESavdPacote)` (275, :49) e
`operator[](ESavdArquivoUE)` (680, :73) devolvem uma cópia do caminho ou lançam 7201 "Arquivo de assinatura associado
ao identificador {} não encontrado" / 7203 "Arquivo assinado associado ao identificador {} não encontrado". Os
caminhos de pacotes contêm marcadores `{}` que os chamadores preenchem com `std::vformat` (fase, pleito, UF, município,
zona, seção — ver u09 `CAssinador`).

---

## 9. BU e arquivos de resultado: o que esta unidade contribui

Esta unidade não monta o conteúdo do BU (isso é `CGeraBU` / `CGravadorBU` / `CConversorEntidadeBU`, ver
`docs/10-boletim-de-urna.md`), mas todo arquivo de resultado do *encerramento* passa por
código desta unidade. Passo a passo, para os arquivos gravados por `CGravacaoResultados` (u09 §5):

1. **Nomes de arquivos.** Cada gravador (`IResultado`) tem um `EExtensaoArquivoResultado`; `CArquivosResultado` dá o
   sufixo (tabela §8): `bu.dat` (4), `rdv.dat` (6), `jufa.dat` (8), `imgbu.dat` (9), `imgze.dat` (11),
   `hash.dat` (12), `log.jez` (13), `wsqbio/wsqman/wsqmes.jez` (16/17/18), `mr.ver` (19), e o pacote de assinatura
   `vota.vsc` (1). Existem variantes do SA (*sistema de apuração*, apuração de contingência) (`busa.dat`, `imgbusa.dat`,
   `sa.vsc`, `logsa.jez`), mas não são gravadas pelo VOTA. `CArquivosSavd` dá o caminho do id SAVD de cada arquivo e do
   pacote de assinatura `vota.vsc` (pacotes 158 / 159 = turno 1 / 2 na flash interna `res1`/`res2`,
   160 / 161 = o mesmo na flash externa; ver `carquivossavd.u22.cpp`).
2. **Cabeçalho de toda entidade de resultado.** `CConversorCabecalhoEntidade::DoConverte` grava `CabecalhoEntidade {
   dataGeracao = "YYYYMMDDTHHMMSS" (CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")), idEleitoral =
   CHOICE alternative ETipoCabecalho (0 idProcessoEleitoral, 1 idPleito, 2 idEleicao) = INTEGER (0..99999) }`.
3. **Fase.** `Utils::ConverteFase` (1712) mapeia a fase da instalação: oficial `'1'` → `Fase 2`, simulado `'2'` →
   `1`, treinamento `'3'` → `3` (usado pelos conversores de BU, RDV, hash, envelope e carga).
4. **Validação antes da codificação.** Toda entidade de resultado passa por `IConversorASN::Converte` (inlinado em
   iconversorasn.h:56 dentro de `CGravadorBU` 11629 para o BU, de `CFileASN::CodeObjectFunction` 5367 para hash.dat,
   de `CGravadorVersoesArquivos` 11582 para mr.ver, de `CRdvVota::Converte` 11488 para o RDV): depois de `DoConverte`, `isValid()` e `isStrictlyValid()` devem ser verdadeiros,
   caso contrário `CUeComumAsnError 7653 "Entidade deixada em estado inválido: <type>: <tracer text>"` aborta o gravador.
   Por ex., uma contagem de votos acima de um limite de INTEGER de `ModuloBoletimUrna` não pode ser gravada.
5. **O BU é só de gravação para o VOTA.** `CConversorEntidadeBU` e `CConversorHistoricoVotoImpresso` mantêm o
   `DoDesconverte` da base (10272, 10275): tentar ler um BU de volta com eles lança 7656 "Método DoDesconverte() não
   implementado para N17ModuloBoletimUrna19EntidadeBoletimUrnaE". A verificação de integridade da cópia do BU na MR é uma
   comparação de bytes (u08), não uma decodificação.
6. **hash.dat** (`CConversorEntidadeHashes::DoConverte`, 10266, usado por `CGravadorHashes` por meio de
   `CFileASN::CodeObjectFunction` 5367): `EntidadeHashes { cabecalho, fase = ConverteFase, siglaUF, identificacaoUrna =
   CHOICE [0] IdentificacaoSecaoEleitoral{municipioZona{município, zona}, local, seção} if the
   `optional<md::CIdentificacaoSecao>` (+36, flag +56) is engaged, else [1] IdentificacaoContingencia{municipioZona}
   if the `optional<md::CIdentificacaoUrnaContingencia>` (+60, flag +72) is, else omitted, versaoUENUX,
   hashesArquivos = one ArquivoAssinatura{nomeArquivo, assinatura} per api::hash::CHashArquivo }`, cada elemento
   validado (7653) por `CConversorHashArquivo::Converte`. Não é lido de volta (`DoDesconverte` da base, 10265).
7. **rdv.dat** (`CConversorRegistroDigitalVoto<CConversorEleicoesVota>`): `DoConverte` (11483, observado: o RDV
   vazio é gravado em `votaInit`) monta `EntidadeRegistroDigitalVoto { pleito, fase, identificacao{municipioZona,
   local, seção}, historicoCodigosCarga (vector<string> of the converter), eleicoes = CConversorEleicoesVota::Converte
   (CHOICE validated with CHOICE::isValid/isStrictlyValid) }`. Os valores do cabeçalho são membros do conversor
   (+20..+44), definidos uma vez na inicialização. Quando o RDV é lido de volta (`CRdvVota::Desconverte` / `ConfereConteudo`, via
   a sobrecarga de vetor de bytes 5680 → `CFileASN::DecodeObject` 5825 → validação → 11482), **apenas `eleicoes` é
   convertido**: pleito, fase, identificação e histórico de carga do arquivo não são comparados com os da própria urna.
8. **mr.ver** (`CConversorVersoesArquivos::DoConverte`, 10264, usado por `CGravadorVersoesArquivos` 11582):
   `EntidadeVersaoArquivos { versaoTag, arquivos = ArquivoAssinatura{nome, versão} per std::map entry (sorted by
   name) }`.
9. **Assinatura.** Não está nesta unidade (u09 `CAssinador`); neste build os arquivos de assinatura contêm o literal
   `assinatura simulada para vota_web_wasm` (analysis/runtime/README.md).

---

## 10. Particularidades do build web e observações em tempo de execução

* As 36 funções vistas em execução estão todas no caminho de carga de dados de `votaInit` (os corpos mesclados 1563/1008, os helpers
  ASN.1 515/1067, cabeçalhos 11588/3738/1713, abrangência 11461/5721, partidos 11457, candidatos 11445/5710/5711,
  suplências 11359/5688, complementos de municípios 11382/3737/3711 (alcançados a partir do arquivo Local da urna:
  `CConversorLocal::DoDesconverte` 11452 → 3737 → 1008 → 11382), seção 5715 e município 5716, pleito 5686,
  cabeçalho de pacote 5705, correspondência 5691/5692, dado secao 5696/5697, dado local 11403,
  NumViasImpressasRelatorios 3726, as quatro gravações de EstadoGeral 9997/10019/10051/10170 → 2893, o RDV inicial 11483
  chamado a partir de `CRdvVota` 11488), mais o caminho das fotos 12641 → 3755 durante as telas de candidato. `Utils::
  DesconverteAbrangencia` (5827) e a verificação de `CodigoCargoConsulta` (5828) são chamadas diretas das 11461
  e 11445 observadas, então certamente executaram, mas o amostrador não as capturou (elas não estão entre as 36).
* Os arquivos de estado `eg.bin`, `vota.bin`, `sa.bin`, `gap.bin` são produzidos no build web pelo mock
  `uenux2/mock/app/comum/cappinfobuilder.cpp` (func 2894) por meio dos conversores de EstadoGeral e do corpo de
  `Converte` com tratamento de exceções 2893.
* Nada nesta unidade é específico do web: nenhum mock, nenhum import JS, nenhum `emscripten_sleep`. As assinaturas do SAVD por trás de
  `CArquivosSavd` são simuladas em outro lugar (texto fixo em todo `.vsu`).
* O "mutex" de `CFotos` (`@1838820`) é o resíduo sem efeito de `std::mutex::unlock` (func 150) do build single-threaded.

---

## 11. Observações notáveis sobre wasm / Emscripten

* **Eliminação de argumentos mortos.** `CArquivosResultado::operator[]` é `(sret, id)`: o seu `this` não usado foi removido
  (o objeto é um singleton sem estado, func 348).
  `CFotos::GetImagem` também não tem `this` (estática, ou a mesma eliminação; ver §15).
* **Inlining entre TUs.** `CIDPacote::UF/Municipio/Zona` (cidpacote.cpp), o construtor de
  `md::CDadosDisponiveisCarga`, código de `CFotos` e os templates de `cfileasn.h` são inlinados em conversores de outros arquivos: o build
  usa LTO, e é por isso que tantos registros de srcloc ficam em funções de outro arquivo-fonte.
* **memcpy sem o padding final.** Cópias de `std::vector<md::CIdentificacaoAgregada>` (`{int32, uint16}`, 8 bytes) são
  `memcpy(dst, src, bytes - 2)` (funcs 2807, 11454): o LLVM não copia o padding do último elemento. Não é um bug.
* **Literais de string montados no lugar.** `CArquivosResultado` monta cada sufixo no buffer SSO do resultado:
  os quatro de 8 caracteres (`'vota.vsc'`, `'busa.dat'`, `'jufa.dat'`, `'hash.dat'`) como constantes imediatas de 64 bits,
  os outros (`bu.dat`, `rdv.dat`, …) por leituras não alinhadas a partir dos dados de string; só `"imgbusa.dat"` (11 caracteres)
  precisa de uma alocação no heap (`operator new(16)`).
* **Temporários de INTEGERs restritos.** Vários `DoConverte`s montam um temporário INTEGER restrito só para
  atribuir o seu valor: `Constrained_INTEGER<2, 1, 99999>` (vtable @1567692) para os códigos de município em 11379 e
  11383, `Constrained_INTEGER<2, 0, 99999>` (vtable @1560492) para idPE / idPleito em 11424 e pleito em 11483:
  idioma `entidade.set_x(INTEGER(v))` no nível do código-fonte.

---

## 12. Código estranho / arriscado

| # | func | achado | impacto |
|---|---|---|---|
| 1 | 12641 → 3755 | Cada foto de candidato é aberta, lida, decodificada em BER e validada **duas vezes** por desenho (verificação de JPEG, depois a leitura real). Confirmado em tempo de execução (§7: um par de aberturas de `-fo.dat` por desenho). | Simulador e urna real, se o código for o mesmo: E/S e CPU extras em toda tela de confirmação. |
| 2 | 12641 | A verificação de JPEG usa `vector::at(0)` / `at(1)`: uma imagem vazia, ou uma imagem de 1 byte cujo byte é `FF` (o `&&` para em um primeiro byte ≠ `FF`), lança `std::out_of_range` (não um `CBaseError`) a partir de uma fonte de dados de GUI. `Foto.imagem` é um OCTET STRING sem restrição e `CConversorFoto` só verifica o código de formato, então uma imagem vazia passa, sim, pela validação ASN.1. | Só com dados `-fo.dat` malformados (dados assinados do TSE na urna). |
| 3 | 11382/11383, 3710/3711 | `ComplementoMunicipio.desligaColetaBiometria` é descartado silenciosamente nas duas direções; o objeto md não tem campo para ele. | Se o TSE o definir para desligar a captura biométrica em um município, este conversor não o propaga (a chave que é usada vem de `Municipio.comBiometria`). |
| 4 | 11482 (com 5680) | A leitura de volta de `rdv.dat` ignora o pleito, a fase, a identificação da seção e o histórico de carga do arquivo; só a estrutura `eleicoes` é convertida e comparada (EstruturaCompativel). | Um RDV de outra seção/pleito com a mesma estrutura de cargos seria aceito por `CRdvVota::Desconverte`; atenuado pela cifragem e pelas assinaturas do SAVD. |
| 5 | 347 | Os ids 13 e 14 mapeiam ambos para `"log.jez"`. | Dois tipos de resultado receberiam o mesmo nome de arquivo; inofensivo se o 14 nunca for usado para um arquivo separado. |
| 6 | 11379 | `Municipio.capital` é sempre gravado FALSE e `codigoIBGE` é omitido. | Arquivos regravados pela urna (`-lo.dat`, dados de carga) perdem esses campos. |
| 7 | 11438, util.cpp:231 | `CabecalhoPacote.abrangencia` nunca é gravado; `md::ETipoPacote` (33 valores) não consegue expressar 23 tipos de pacote ASN.1 (41..47, 49, 50, 52..65). | Informativo (os cabeçalhos de pacote são produzidos pelos sistemas do TSE, não pelo VOTA). |
| 8 | 1563/1008 vs 2893 | Só as instanciações de EstadoGeral têm landing pads; em todos os outros lugares uma exceção de validação ASN.1 vaza a entidade meio construída e as strings. | Vazamento de memória apenas em caminhos de erro. |
| 9 | 10272, 10275, 10265 | BU, HistoricoVotoImpresso e hashes não têm `DoDesconverte`: uma tentativa de leitura lança 7656 em tempo de execução (não um erro de compilação). | Por projeto (só de gravação); nenhum leitor futuro deve usar estes conversores. |
| 10 | 11423 | `md::CDadosDisponiveisCarga` rejeita um serial de mídia de carga cujo comprimento não seja exatamente 8. | O relatório "versão dos pacotes" (11905, o único usuário) falha com 8043 se `dadoscarga.dat` trouxer outro formato de serial. Nenhum dos cenários do simulador inclui um `dadoscarga.dat` (`upstream/fs/`), então a verificação nunca é alcançada ali. |

---

## 13. Arquivos-fonte reconstruídos

Arquivos atestados da unidade:

* `src/uenux2/src/app/comum/asn/iconversorasn.h`, `iconversorbiometriaasn.h`, `legravaentidade.h`
* `src/uenux2/src/app/comum/asn/util.h`, `util.cpp` (+ `util.u13.cpp` da u13 para `DesconverteDataJE`)
* `src/uenux2/src/app/comum/asn/cconversorabrangencia.h/.cpp`, `cconversorcabecalhoentidade.h/.cpp`
* `src/uenux2/src/app/comum/carquivosresultado.h/.cpp`, `carquivossavd.h/.cpp`

Classes de conversores sem arquivo atestado (caminho inferido das convenções do TSE: diretório da classe md espelhado em
`asn/`, conversores irmãos atestados no mesmo diretório, e caminhos de include já usados pelas u03/u05):

* `src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.h/.cpp` (md em `comum/md/`, como Abrangencia/CabecalhoEntidade)
* `src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.h/.cpp`, `cconversorcomplementomunicipio.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.h/.cpp`, `cconversoridentificacaoagregada.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.h/.cpp`
* `src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.h/.cpp`, `cconversorcandidatura.h`
* `src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.h/.cpp`
* `src/uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.h/.cpp`, `cconversorversoesarquivos.h/.cpp`,
  `cconversorregistrodigitalvoto.h`

Fragmentos para arquivos pertencentes a outras unidades: `src/uenux2/src/app/comum/dados/cfotos.u21.cpp` (3755, 3756, 2818, 11513),
`src/uenux2/src/app/comum/dados/ccandidaturas.u21.cpp` (12641),
`src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp` (3710, 3711).

---

## 14. Tabela de mapeamento completa (todas as 103 funções)

"ran" = vista nas amostras do V8 dos votos gravados. Helpers de biblioteca não têm código-fonte do TSE.

| # | func | tamanho | ran | símbolo reconstruído | arquivo original | arquivo src / status | conf. |
|---|---|---|---|---|---|---|---|
| 1 | 255 | 21 |  | `ecourna::api::exception::CBaseError<comum::EUeComumAsnError, ecourna::api::exception::SErrorLimits{7650, 7750}>::CBaseError` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp | biblioteca/helper inlinado (sem código-fonte do TSE) | alta |
| 2 | 275 | 612 |  | `comum::CArquivosSavd::operator[](ESavdPacote) const` | uenux2/src/app/comum/carquivossavd.cpp | src/uenux2/src/app/comum/carquivossavd.cpp | alta |
| 3 | 347 | 1322 |  | `comum::CArquivosResultado::operator[](EExtensaoArquivoResultado) const` | uenux2/src/app/comum/carquivosresultado.cpp | src/uenux2/src/app/comum/carquivosresultado.cpp | alta |
| 4 | 515 | 233 | sim | `ASN1::SEQUENCE::includeOptionalField(int optional, int field)` | (runtime ASN.1 da III, build do TSE) | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 5 | 680 | 612 |  | `comum::CArquivosSavd::operator[](ESavdArquivoUE) const` | uenux2/src/app/comum/carquivossavd.cpp | src/uenux2/src/app/comum/carquivossavd.cpp | alta |
| 6 | 731 | 410 |  | `comum::asn::IConversorASN<E, D>::DoConverte / DoDesconverte (merged default body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 7 | 751 | 14 |  | `ASN1::ENUMERATED::operator=(const ENUMERATED&) (value copy)` | (runtime ASN.1 da III) | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 8 | 1008 | 572 | sim | `comum::asn::IConversorASN<E, D>::Desconverte (merged body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 9 | 1067 | 44 | sim | `ASN1::CHOICE::isStrictlyValid() const` | (runtime ASN.1 da III) | biblioteca/helper inlinado (sem código-fonte do TSE) | alta |
| 10 | 1076 | 496 |  | `std::vector<ASN1::AbstractData*>::insert(const_iterator, const value_type&)` | libcxx/include/vector | biblioteca/helper inlinado (sem código-fonte do TSE) | alta |
| 11 | 1563 | 572 | sim | `comum::asn::IConversorASN<E, D>::Converte (merged body)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 12 | 1712 | 524 |  | `comum::asn::Utils::ConverteFase` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 13 | 1713 | 556 | sim | `comum::asn::Utils::DesconverteDataHoraJE` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 14 | 2275 | 20 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, comum::md::CCabecalhoEntidade>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 15 | 2807 | 237 |  | `std::vector<comum::md::CSecaoEleitoral>::__init_with_size(first, last, n) (vector copy)` | libcxx/include/vector | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 16 | 2818 | 37 |  | `comum::CFotos::~CFotos` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | média |
| 17 | 2843 | 494 |  | `comum::asn::Utils::DesconverteFase` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 18 | 2893 | 1021 | sim | `comum::asn::IConversorASN<E, D>::Converte (merged body, exception-safe copy)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 19 | 3710 | 60 |  | `comum::md::CComplementoMunicipio::CComplementoMunicipio(TMunicipioID, int16, const CHorarioVerao&)` | uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp | src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp | média |
| 20 | 3711 | 37 | sim | `comum::md::CComplementoMunicipio::CComplementoMunicipio(TMunicipioID, int16)` | uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.cpp | src/uenux2/src/app/comum/dados/md/municipiozona/ccomplementomunicipio.u21.cpp | média |
| 21 | 3726 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios, comum::md::estadoaplicacao::CNumViasImpressasRelatorios>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 22 | 3727 | 20 |  | `comum::asn::IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios, comum::md::estadoaplicacao::CNumViasImpressasRelatorios>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 23 | 3730 | 20 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::Seguranca, comum::md::CSeguranca>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 24 | 3736 | 20 |  | `comum::asn::IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, comum::md::CComplementoMunicipio>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 25 | 3737 | 20 | sim | `comum::asn::IConversorASN<ModuloComplementosMunicipios::ComplementoMunicipio, comum::md::CComplementoMunicipio>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 26 | 3738 | 20 | sim | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, comum::md::CCabecalhoEntidade>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 27 | 3755 | 4858 | sim | `comum::CFotos::GetImagem` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | média |
| 28 | 3756 | 53 |  | `comum::CFotos::CFotos` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp | média |
| 29 | 3799 | 20 |  | `comum::asn::IConversorASN<ModuloTiposResultadosEcoUrna::Urna, comum::md::CUrna>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 30 | 3800 | 483 |  | `comum::asn::Utils::DesconverteTipoLocalVotacao` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 31 | 3801 | 518 |  | `comum::asn::Utils::ConverteTipoLocalVotacao` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 32 | 5672 | 18 |  | `comum::md::CIdentificacaoAgregada::CIdentificacaoAgregada(TSecaoID numero, ETipoLocalVotacao tipo)` | uenux2/src/app/comum/dados/md/csecaoeleitoral.h | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp (comentário) | baixa |
| 33 | 5680 | 739 |  | `comum::asn::IConversorASN<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto, comum::md::CVotosEleicoesVota>::Desconverte(const std::vector<uebyte>&)` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | média |
| 34 | 5684 | 574 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::TipoIdentificadorEleitor, ecourna::app::dados::ETipoIdentificadorEleitor>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 35 | 5686 | 20 | sim | `comum::asn::IConversorASN<ModuloProcessoEleitoral::Pleito, comum::md::CPleitoDTO>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 36 | 5688 | 20 | sim | `comum::asn::IConversorASN<ModuloEleicao::NomesCargo, comum::md::CNomesCargo>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 37 | 5691 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia, comum::md::estadoaplicacao::CDadoCorrespondencia>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 38 | 5692 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia, comum::md::estadoaplicacao::CDadoCorrespondencia>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 39 | 5696 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoSecao, comum::md::estadoaplicacao::CLocalidadeEleitoral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 40 | 5697 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::DadoSecao, comum::md::estadoaplicacao::CLocalidadeEleitoral>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 41 | 5705 | 20 | sim | `comum::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoPacote, comum::md::CCabecalhoPacote>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 42 | 5710 | 1431 | sim | `comum::asn::(anonymous namespace)::DesconverteLista(std::vector<md::CCandidatura>&, const CConversorCandidatura&, const SEQUENCE_OF<Candidatura>&)` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | baixa |
| 43 | 5711 | 20 | sim | `comum::asn::IConversorASN<ModuloCandidatos::DadosCandidato, comum::md::CDadosCandidato>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 44 | 5712 | 35 |  | `comum::asn::CConversorCandidatura::CConversorCandidatura(TCargoID, TPartidoID, bool)` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.h | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.h | média |
| 45 | 5713 | 20 |  | `comum::asn::IConversorASN<ModuloLocal::SecaoEleitoral, comum::md::CSecaoEleitoral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 46 | 5714 | 20 |  | `comum::asn::IConversorASN<ModuloTiposCadastro::Municipio, comum::md::CMunicipio>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 47 | 5715 | 20 | sim | `comum::asn::IConversorASN<ModuloLocal::SecaoEleitoral, comum::md::CSecaoEleitoral>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 48 | 5716 | 20 | sim | `comum::asn::IConversorASN<ModuloTiposCadastro::Municipio, comum::md::CMunicipio>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 49 | 5721 | 20 | sim | `comum::asn::IConversorASN<ModuloTiposEleitorais::Abrangencia, comum::md::CAbrangencia>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 50 | 5731 | 264 |  | `ASN1::SEQUENCE_OF<ASN1::GeneralString>::operator=(const SEQUENCE_OF&)` | (template do runtime ASN.1 da III) | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 51 | 5827 | 490 |  | `comum::asn::Utils::DesconverteAbrangencia` | uenux2/src/app/comum/asn/util.cpp | src/uenux2/src/app/comum/asn/util.cpp | alta |
| 52 | 5828 | 575 |  | `comum::asn::IConversorASN<ModuloTiposEleitorais::CodigoCargoConsulta, unsigned char>::Desconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 53 | 5950 | 399 |  | `std::__uninitialized_allocator_relocate<std::allocator<comum::md::CCandidatura>> (vector<CCandidatura> growth)` | libcxx/include/__memory/uninitialized_algorithms.h | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 54 | 9209 | 492 |  | `ModuloFederacoes::EntidadeFederacoes::set_federacoes(const SEQUENCE_OF<Federacao>&)` | (código gerado do ASN.1 da III, ModuloFederacoes) | biblioteca/helper inlinado (sem código-fonte do TSE) | baixa |
| 55 | 9843 | 88 |  | `std::make_format_args<std::format_context>(std::string&)` | libcxx/include/__format/format_arg_store.h | biblioteca/helper inlinado (sem código-fonte do TSE) | média |
| 56 | 9997 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralVota::EstadoGeralVota, comum::md::estadoaplicacao::CEstadoGeralVota>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 57 | 10019 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralSA::EstadoGeralSA, comum::md::estadoaplicacao::CEstadoGeralSA>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 58 | 10051 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralGap::EstadoGeralGap, comum::md::estadoaplicacao::CEstadoGeralGap>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 59 | 10170 | 20 | sim | `comum::asn::IConversorASN<ModuloEstadoGeralUrna::EstadoGeralUrna, comum::md::estadoaplicacao::CEstadoGeral>::Converte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 60 | 10264 | 909 |  | `comum::asn::CConversorVersoesArquivos::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp (caminho inferido) | src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp | alta |
| 61 | 10265 | 29 |  | `comum::asn::IConversorASN<ModuloHashes::EntidadeHashes, comum::md::CEntidadeHashes>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 62 | 10266 | 1338 |  | `comum::asn::CConversorEntidadeHashes::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.cpp (caminho inferido) | src/uenux2/src/app/comum/gravadores/asn/cconversorentidadehashes.cpp | alta |
| 63 | 10272 | 29 |  | `comum::asn::IConversorASN<ModuloBoletimUrna::EntidadeBoletimUrna, comum::md::CEntidadeBU>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 64 | 10275 | 29 |  | `comum::asn::IConversorASN<ModuloBoletimUrna::HistoricoVotoImpresso, comum::md::CHistoricoVotoImpresso>::DoDesconverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 65 | 11358 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::Suplencias, comum::md::CSuplencia>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 66 | 11359 | 419 | sim | `comum::asn::CConversorSuplencia::DoDesconverte` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsuplencia.cpp | alta |
| 67 | 11360 | 29 |  | `comum::asn::IConversorASN<ModuloSituacoesEleicoes::EntidadeSituacoesEleicoes, std::vector<comum::md::CSituacoesEleicoes>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 68 | 11362 | 29 |  | `comum::asn::IConversorASN<ModuloProcessoEleitoral::EntidadeProcessoEleitoral, comum::md::CProcessoEleitoralDTO>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 69 | 11364 | 29 |  | `comum::asn::IConversorASN<ModuloProcessoEleitoral::Pleito, comum::md::CPleitoDTO>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 70 | 11368 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::NomesCargo, comum::md::CNomesCargo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 71 | 11370 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::EntidadeEleicao, comum::md::CEleicaoPE>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 72 | 11372 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::CargoPergunta, comum::md::CCargo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 73 | 11374 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::DetalheCargo, comum::md::CDetalheCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 74 | 11376 | 29 |  | `comum::asn::IConversorASN<ModuloEleicao::DetalhePergunta, comum::md::CDetalheConsulta>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 75 | 11379 | 257 |  | `comum::asn::CConversorMunicipio::DoConverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversormunicipio.cpp | alta |
| 76 | 11380 | 29 |  | `comum::asn::IConversorASN<ModuloComplementosMunicipios::EntidadeComplementosMunicipios, comum::md::CComplementosMunicipiosUF>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 77 | 11382 | 691 | sim | `comum::asn::CConversorComplementoMunicipio::DoDesconverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp | alta |
| 78 | 11383 | 753 |  | `comum::asn::CConversorComplementoMunicipio::DoConverte` | uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementomunicipio.cpp | alta |
| 79 | 11403 | 295 | sim | `comum::asn::CConversorDadoLocal::DoConverte` | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp | alta |
| 80 | 11409 | 29 |  | `comum::asn::IConversorASN<ModuloImpedidos::EntidadeImpedidos, std::vector<comum::md::CImpedido>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 81 | 11415 | 29 |  | `comum::asn::IConversorASN<ModuloEleitores::Dedo, comum::md::CDedo>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 82 | 11418 | 418 |  | `comum::asn::IConversorBiometriaASN<ModuloEleitores::BiometriaEleitorCifrada, comum::md::CBiometriaEleitor>::DoConverte` | uenux2/src/app/comum/asn/iconversorbiometriaasn.h | src/uenux2/src/app/comum/asn/iconversorbiometriaasn.h | alta |
| 83 | 11421 | 29 |  | `comum::asn::IConversorASN<ModuloEleitores::BiometriaEleitor, comum::md::CBiometriaEleitor>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 84 | 11423 | 5864 |  | `comum::asn::CConversorDadosDisponiveisCarga::DoDesconverte` | uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp | alta |
| 85 | 11424 | 3377 |  | `comum::asn::CConversorDadosDisponiveisCarga::DoConverte` | uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/correspondencia/cconversordadosdisponiveiscarga.cpp | alta |
| 86 | 11438 | 2119 |  | `comum::asn::CConversorCabecalhoPacote::DoConverte` | uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp (caminho inferido) | src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp | alta |
| 87 | 11439 | 29 |  | `comum::asn::IConversorASN<ModuloFotosCandidatos::FotoCandidato, comum::md::CFotoCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 88 | 11444 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::EntidadeCandidatos, std::vector<comum::md::CCandidatura>>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 89 | 11445 | 281 | sim | `comum::asn::CConversorCandidaturas::DoDesconverte` | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.cpp | alta |
| 90 | 11446 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::Candidatura, comum::md::CCandidatura>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 91 | 11449 | 29 |  | `comum::asn::IConversorASN<ModuloCandidatos::DadosCandidato, comum::md::CDadosCandidato>::DoConverte` | uenux2/src/app/comum/asn/iconversorasn.h | src/uenux2/src/app/comum/asn/iconversorasn.h | alta |
| 92 | 11454 | 1522 |  | `comum::asn::CConversorSecaoEleitoral::DoConverte` | uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp | alta |
| 93 | 11455 | 71 |  | `comum::asn::CConversorIdentificacaoAgregada::DoDesconverte` | uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp | alta |
| 94 | 11457 | 2011 | sim | `comum::asn::CConversorEntidadePartidos::DoDesconverte` | uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp | alta |
| 95 | 11458 | 1955 |  | `comum::asn::CConversorEntidadePartidos::DoConverte` | uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp | alta |
| 96 | 11461 | 526 | sim | `comum::asn::CConversorAbrangencia::DoDesconverte` | uenux2/src/app/comum/asn/cconversorabrangencia.cpp | src/uenux2/src/app/comum/asn/cconversorabrangencia.cpp | alta |
| 97 | 11462 | 1402 |  | `comum::asn::CConversorAbrangencia::DoConverte` | uenux2/src/app/comum/asn/cconversorabrangencia.cpp | src/uenux2/src/app/comum/asn/cconversorabrangencia.cpp | alta |
| 98 | 11482 | 594 |  | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h (caminho inferido) | src/uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h | alta |
| 99 | 11483 | 1181 | sim | `comum::asn::CConversorRegistroDigitalVoto<comum::asn::CConversorEleicoesVota>::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h (caminho inferido) | src/uenux2/src/app/comum/gravadores/asn/cconversorregistrodigitalvoto.h | alta |
| 100 | 11513 | 37 |  | `std::unique_ptr<comum::CFotos>::~unique_ptr (atexit handler of the CFotos singleton @1838844)` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp (comentário) | média |
| 101 | 11588 | 119 | sim | `comum::asn::CConversorCabecalhoEntidade::DoDesconverte` | uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | src/uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | alta |
| 102 | 11589 | 701 |  | `comum::asn::CConversorCabecalhoEntidade::DoConverte` | uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | src/uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp | alta |
| 103 | 12641 | 544 | sim | `api::CDataImage<comum::CCandidaturasDSFoto>::GetImage` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/dados/ccandidaturas.u21.cpp | média |

---

## 15. Questões em aberto

* Nomes dos enumeradores de `EExtensaoArquivoResultado` (só os sufixos estão no binário); significado de `red.vsc`,
  `rdvred.dat`, `asw.vsc`, `ahw.vsc`, e por que os ids 13 e 14 compartilham `log.jez`.
* O `DoDesconverte` padrão de `IConversorBiometriaASN` está ausente (virtual puro ou nunca instanciado).
* Os nomes de membros de `CFotos` (`Busca`, `Existe`, `m_atual`) e os nomes de campos de `SIndiceFoto` / `md::CIndexer` são
  inferidos. Os dois chamadores de `GetImagem` (12641 e 12520) chamam `CFotos::GetInst()` (2819) e descartam o resultado logo
  antes dela, o que é compatível tanto com uma forma de código-fonte `CFotos::GetInst().GetImagem(id)` com o `this` não usado removido quanto com uma
  `GetImagem` estática precedida de uma instrução `GetInst()` separada.
* Os caminhos dos arquivos de conversores sem registros de srcloc são inferidos (§13).

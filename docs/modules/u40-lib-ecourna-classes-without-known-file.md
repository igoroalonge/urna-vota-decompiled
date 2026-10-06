# u40: classes da `ecourna` sem arquivo-fonte conhecido (erros, RNGs, factory de cifras, stub do CEPESC, conversores ASN.1)

A unidade u40 contém **101 funções wasm** que as ferramentas colocaram no componente `lib:ecourna`, mas não conseguiram associar a um
arquivo-fonte: as classes não têm registro de `std::source_location` (não lançam nada próprio, ou são elas mesmas as
classes de exceção). Cerca de 60 são código real da `ecourna`; o restante são instanciações de templates de biblioteca,
corpos mesclados pelo wasm-opt, destrutores de objetos estáticos e alguns pedaços de `uenux2/src/app/comum` que
vieram parar aqui por causa dos seus vizinhos na tabela de funções. Este documento dá nome a cada uma delas.

Só **10 das 101 executaram** nos votos gravados (`analysis/runtime/*.functions.tsv`): três conversores/helpers do
carregamento de arquivos na inicialização (`ConverteDataHoraJE` 1877, `CConversorHorariosUrna::DoDeconverte` 9132,
`CConversorIdentificadorGeradorMidia::DoDeconverte` 9224, esta última alcançada a partir de
`comum::asn::CConversorDadoCorrespondencia::DoDesconverte` 11400 enquanto `eg.bin` é lido), a construção da cifra do RDV em `votaInit`
(`CSha512::CSha512` 2684, `CSymmetricCipherFactory::Create(CAesKey)` 9489), a lista de candidatos da tela
de votação (`CCandidaturas::GetNumerosCandidatosAptos` 2840) e quatro helpers da libc++ (986, 1108, 2898, 3340). Várias
afirmações abaixo também foram verificadas **chamando as funções no simulador em execução** (uma cópia de
`tools/run/headless.mjs` que chama slots da tabela depois da execução; §4.3, §6). A revisão de fidelidade executou de novo essa sonda
de forma independente (slots 6735, 114 e a vtable de `CPrng`) e obteve os mesmos valores.

Glossário: *urna*; *eleitor*; *mesário*; *seção*; *zona* eleitoral; *município*; *título (de eleitor)*, o número de
inscrição do eleitor; *comparecimento*; *justificativa*, a declaração de um eleitor que não pode votar na sua seção;
*habilitação*, a liberação de um eleitor para votar (*por biometria* = pela digital, *por biografia* = com o mesário conferindo os dados do eleitor, *por código* = com um
código de liberação); *BU (boletim de urna)*, o resultado da seção; *zerésima*, o relatório zero impresso antes da votação; *RDV
(registro digital do voto)*, a tabela embaralhada dos votos dados; *carga*, a preparação do cartão flash da urna; *mídia*,
o meio de armazenamento (cartão flash, pen drive); *cabeçalho*, o header; *pleito*, o dia da eleição; *federação*, a federação partidária; *CEPESC*,
a biblioteca criptográfica do governo usada para os arquivos de resultado; `-jufa.dat`, o "registro de comparecimento de
eleitores e mesários" do TSE, enviado de volta ao cadastro de eleitores (*cadastro*).

## Arquivos-fonte reconstruídos

Todos os caminhos são *(caminho inferido)*, a menos que marcados como "attested" (o arquivo aparece em `analysis/srcloc.tsv`). Os arquivos
novos estão completos; os arquivos `*.u40.cpp` são fragmentos de arquivos que pertencem a outras unidades (a unidade dona é indicada dentro deles).

```
src/ecourna/api/exception/cerror.hpp, cerror.cpp            CError (1143, 4636, 1940, 11562)
src/ecourna/api/exception/cbaseerror.hpp                    SErrorLimits, CBaseError<E,L> (710, 1011, 2379; thunk list incl. 9181)
src/ecourna/api/security/irng.hpp                           IRng (slot layout, return codes)
src/ecourna/api/security/ctrng.hpp, ctrng.cpp               CTrng (9477, 9476, 9475, 5173; static 9478)
src/ecourna/api/security/cprng.hpp, cprng.cpp               CPrng (9503, 9502, 9500, 9499, 5178; twist 5179)
src/ecourna/api/security/isymmetriccipherfactory.hpp        ISymmetricCipherFactory
src/ecourna/api/security/csymmetriccipherfactory.hpp/.cpp   CSymmetricCipherFactory (1884, 9489)
src/ecourna/api/security/cepesc/ccepesccipher.hpp/.cpp      IAsymmetricCipher<>, CCepescCipher (2681; 5171 repeated from u03)
src/ecourna/api/security/security.u40.cpp                   fragments: CSha512 ctor 2684, ~CAesKey 3517, ~CBlockCipher D0 9488,
                                                            s_cipherTable dtor 9496, vector::insert 9492, CInfoSalt copy 9470, unique_ptr 5170
src/ecourna/api/util/datahora.hpp, datahora.cpp             ConverteDataHoraJE (1877), FormataDataHoraJE (9220)
src/ecourna/api/util/cstringutils.u40.cpp                   CStringUtils::Replace (9398)          [cstringutils.cpp attested]
src/ecourna/api/compression/clzmacompress.u40.cpp           CLzmaCompress::OnProgress (9585), static mutex 8378 [attested]
src/ecourna/api/pattern/iobservableprogresswithdescription.u40.cpp   deleting dtor 9528, signals2 map insert 9551
src/ecourna/app/dados/ccabecalhoentidade.h/.cpp             CCabecalhoEntidade (5112)
src/ecourna/app/dados/cidentificacaosecaoeleitoral.h/.cpp   CMunicipioZona, CIdentificacaoSecaoEleitoral (5110)
src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp   (5087, 2657)  [attested]
src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.u40.cpp    (2658)        [attested]
src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp   (3483, 3482)  [attested]
src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp  (3484)
src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.h/.cpp      (9114, 9112)
src/ecourna/app/dados/asn/cconversormunicipiozona.h/.cpp                     (9131; 9130 for completeness)
src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.u40.cpp      (9129)
src/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.h/.cpp        (9222; 9221) - BU field
src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h/.cpp         (9225, 9224)
src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.u40.cpp (9060)
src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp              (9062, helper 9063)
src/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.u40.cpp (9065)
src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.u40.cpp        (9126, 9122 note)
src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.h/.cpp                    (9133, 9132)
src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.u40.cpp          (9144)
src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.u40.cpp         (9141, 9139 note)
src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.u40.cpp            (9173)
src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.u40.cpp                            (9212)
src/ecourna/app/dados/asn/federacoes/cconversorfederacao.u40.cpp                             (9216; 9214/5082/1671 notes)
src/ecourna/app/dados/asn/midias/cconversorautenticacao.u40.cpp                              (9201)
src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.h/.cpp                          (9203, 9202)
src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.u40.cpp         CEleitorIdentidade::ToString (1924)
src/uenux2/src/app/comum/dados/ccandidaturas.u40.cpp                         GetNumerosCandidatosAptos (2840) [attested]
src/uenux2/src/app/comum/dados/cpartidos.u40.cpp                             ~CPartidos (2815), statics 11502/11503
```

Evidência de diretório para os caminhos inferidos: `ecourna/api/security/` (atestados: `cblockcipher.hpp`, `csha.cpp`,
`isymmetriccipher.cpp`), `ecourna/api/security/cepesc/` (atestados: `cplaintext.cpp`, `ccipheredout.cpp`,
`ccipheredin.cpp`, `cinfosalt.cpp`), `ecourna/api/util/` (`cstringutils.cpp`), `ecourna/app/dados/` (classes de dados de
tipos compartilhados, p. ex. `cregistroidentificacaoeleitor.cpp`), `ecourna/app/dados/asn/` e seus subdiretórios (conversores
atestados dos mesmos módulos ASN.1). O diretório `api/exception/` em si não é atestado; ele segue o
namespace `ecourna::api::exception`, como todos os outros diretórios `api/<x>/`.

---

## 1. Onde este código fica no processo de votação

`ecourna` é a biblioteca do TSE compartilhada pelos aplicativos da urna. A parte dela que cabe à u40 faz quatro trabalhos:

1. **Erros.** Toda exceção do código do TSE é um `CError`. Seu construtor monta o longo texto de `what()`
   (assinatura da função, linha, código, mensagem). Quando uma thread morre, a tela de "urna inoperante" mostra a
   mensagem simples e o código, não esse texto (§3.1).
2. **Aleatoriedade e cifras.** O gerador aleatório de toda a aplicação (`IRng`), a factory de cifras AES usadas para
   o RDV e para os arquivos de chave, e a cifra híbrida CEPESC que protege os arquivos de resultado (BU, comparecimento, imagens).
3. **Conversores ASN.1** entre as classes de dados da ecourna e os objetos ASN.1 da III para os arquivos que a urna lê na
   inicialização (horários por município `-cfm.dat`, parametrização `-pu.dat`, federações `-fe.dat`, a descrição da
   mídia `infomidia`) e grava no final (o resultado de comparecimento `-jufa.dat` e um campo do BU).
4. **Pequenas classes de dados** usadas por esses conversores (cabeçalho, identificação da seção, registros de comparecimento).

No build web, só a leitura de arquivos na inicialização (1, 3) e a construção da cifra do RDV executam. O código de fim de dia
(BU, arquivo de comparecimento) está linkado, mas nunca é alcançado pela página (docs/bu/codepath.md §1.3); ele foi exercitado pelo
harness separado do BU (`tools/bu/operator_harness.mjs`), o que confirma partes desta unidade (§5, §10).

---

## 2. Classes e hierarquia (RTTI)

```
std::exception
 └─ ecourna::api::exception::CError                      vtable @1526468: [0] ~ 4636 [1] D0 1940 [2] what 11562
     └─ CBaseError<E, SErrorLimits{min,max}>             45 families (EIoError{1175,1275}, ESecurityError{1325,1725},
                                                          EDadosError{1935,2135}, vota::EUeVotaError, ...): same 3 slots
                                                          (42 vtables; CUePrinterError, CUeDesligandoError and
                                                          io::CIoError derive from three of them)
ecourna::api::security::IRng                              (no base)
 ├─ CTrng                                                 vtable @1114668: [2] 9477 [3] 9476 [4] 9475 [5] 5173
 └─ CPrng                                                 vtable @1113176: [2] 9502 [3] 9500 [4] 9499 [5] 5178
ecourna::api::pattern::NonCopyable
 ├─ ISymmetricCipherFactory
 │   └─ CSymmetricCipherFactory                           vtable @1113840: [2] Create(string) 1884 [3] Create(CAesKey) 9489
 └─ IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>>
     └─ ecourna::api::cepesc::CCepescCipher               vtable @1115004: [2] Encrypt 2681 [3] Decrypt 5171
ecourna::api::pattern::IObservableProgressWithDescription vtable @1111068: [1] D0 9528
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>          [0] ~ (174) [1] D0 (144) [2] DoConverte [3] DoDeconverte
 ├─ CConversorRegistroIdentificacaoEleitor   <ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, CRegistroIdentificacaoEleitor>   9114 / 9112
 ├─ CConversorIdentificadorGeradorMidia      <ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>        9225 / 9224
 ├─ CConversorMunicipioZona                  <ModuloTiposEleitorais::MunicipioZona, CMunicipioZona>                            9131 / (9130)
 ├─ CConversorIdentificacaoSecaoEleitoral    <ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, ...>                          9129 / (9127)
 ├─ CConversorDetalhamentoComparecimento     <ModuloBoletimUrna::DetalhamentoComparecimento, CDetalhamentoComparecimento>       9222 / (9221)
 ├─ CConversorHorariosUrna                   <ModuloConfiguracaoMunicipios::HorariosUrna, CHorariosUrna>                       9133 / 9132
 ├─ CConversorConfiguracaoMunicipio(s)       <...ConfiguracaoMunicipio / EntidadeConfiguracaoMunicipios>                        9144, 9141 / (9142, 9137)
 ├─ CConversorParametrizacaoUrna             <ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna, CParametrizacaoUrna>       9173 / (9171)
 ├─ CConversorFederacao / CConversorFederacoes <ModuloFederacoes::Federacao / EntidadeFederacoes>                               (9217) 9216 / 9212 (9207)
 ├─ CConversorAutenticacao                   <ModuloInformacaoMidia::Autenticacao, CAutenticacao>                              9201 / (9200)
 ├─ CConversorDadosGeracaoMidia              <ModuloInformacaoMidia::DadosGeracaoMidia, CDadosGeracaoMidia>                    9203 / 9202
 ├─ CConversorComparecimentoSecao            <ModuloResultadoUrnaCadastro::ComparecimentoSecao, CComparecimentoSecao>          9126 / (9120)
 ├─ CConversorIdentificacaoJustificativa     <...::IdentificacaoJustificativa, CIdentificacaoJustificativa>                    9065 / (9064)
 ├─ CConversorDadosCifracao                  <...::DadosCifracao, CDadosCifracao>                                              9062 / (9061)
 └─ CConversorDadosComparecimentoCifrado     <...::DadosComparecimentoCifrado, CDadosComparecimentoCifrado>                    9060 / (9057)
```

Os números entre parênteses pertencem a outras unidades. A listagem do unit.py nomeou os slots dos conversores `vf2`/`vf3`; o
layout de `IConversorASN` (u11) os torna `DoConverte`/`DoDeconverte`. Os conversores não têm estado (4 bytes, só o
vptr) e são sempre construídos na pilha do chamador.

---

## 3. Exceções: `CError` e `CBaseError<E, L>`

### 3.1 Layout e `what()`

`CError` (40 bytes, o tamanho que todo ponto de lançamento aloca):

| offset | membro | definido pelo construtor (func 1143) |
|---|---|---|
| +0 | vptr | CError @1526468, depois sobrescrito pela vtable da família |
| +4 | `int m_codigo` | o código de erro |
| +8 | `std::string m_mensagem` | a mensagem, **movida** do parâmetro passado por valor |
| +20 | `std::source_location m_local` | ponteiro para o registro estático `{file, function, line, column}` do ponto de lançamento |
| +24 | ? | nunca escrito (padding ou um membro não usado) |
| +28 | `std::string m_what` | primeiro uma cópia da mensagem, depois a forma longa abaixo |

A forma longa é

```
m_what = function_name() + ":"s + to_string(line()) + ":"s + to_string(codigo) + " - "s + mensagem
```

dentro de um `try { } catch (...) { }`: todo landing pad do corpo vai para um catch-all que retorna normalmente, então
uma falha durante a formatação (só `std::bad_alloc` é possível) deixa `what()` = a mensagem simples. O nome do arquivo
da localização não é usado. Exemplo real capturado pelo harness do BU
(`samples/bu-real/run-full/exceptions.json`):

```
static T &api::CPolySingleton<api::IGenericFactory<ecourna::api::security::IHash>>::instance(
  TPolySingletonsInfo &, const std::source_location &) [T = ...]:78:1301 - PolySingleton - solicitada uma instancia
  nao criada N3api15IGenericFactoryIN7ecourna3api8security5IHashEEE [/home/rubio/tse/uenux2/src/api/hash/cmontadorhash.cpp:49]
```

A tela **não** mostra esse texto. `CThreadVota::TrataExcecao` (func 7710, u26 §4) monta a página de "urna
inoperante" a partir do título do contexto + `" ({})"` com `GetCodigo()` e de `m_mensagem` (+8), seguidos das
ações do contexto. `what()` só é passado como último argumento de `CApplication::ShowExceptionMsg` (5568), cujo
corpo neste build é apenas um lançamento de `std::system_error` (u15 §2.4), então o que ela faz com esse texto não é visível.
Para uma `std::exception` que não é `CError`, a 7709 chega a remover um prefixo no estilo do TSE com a regex
`.*?\).*?:\d+:-?\d+ - ((.|\n)*)` (@403127) antes de mostrá-la. A assinatura de desenvolvedor, portanto, aparece nos logs
e na saída do harness (como acima), não na tela do eleitor.

### 3.2 `CBaseError` e os três corpos de construtor mesclados

`CBaseError<E, SErrorLimits{min, max}>` não acrescenta nada a `CError`; os limites fazem parte apenas do nome do tipo (nenhuma
função compara o código com eles). Seu construtor é
`CBaseError(E codigo, std::string mensagem, std::source_location local = current()) : CError(int(codigo), std::move(mensagem), local) {}`.
O *merge-similar-functions* do wasm-opt dobrou as 45 instanciações (contagem do RTTI; mais três entradas de RTTI são classes
derivadas de um `CBaseError<>`) em corpos compartilhados que recebem a vtable da família como argumento, e cada família mantém um
thunk de 18-21 bytes:

| corpo | variante | famílias (thunk) |
|---|---|---|
| **1011** | ctor de CError via `invoke` (slot 169) + landing pad que libera a mensagem movida | ecourna: EDadosResultadoUrnaCadastro 9025, EDadosProcessoEleitoral 9026, EDadosMidias 9041, EDadosFederacoes 9046, EAsnResultadoUrnaCadastro 9107, EAsnParametrizacaoUrna 9158, **EAsnMidias 9181 (esta unidade)**, EAsn 9229, EDados 9266, EUtil 9410, ESql 9461, ESecurity 9505, ECompression 9584 |
| **710** | chamada direta (sem landing pad) | uenux2 + uma família de header: vota::EUeVota 253, EUeComumGravadores 283, EUeUtil 346, EUeGui 406, EUeAssert 463, EUeComum 480, EUeComumRelatorios 580, EUeComumMd 591, EUeRdv 655, EApiAsn 1074, EUeHwil 1229, EUeComumAppInfo 1710, EUePersistencia 1715, EUeIpc 1889, EUeComumJustificativa 2260, ... |
| 2294 + **2379** | a 2294 (rt:shared) chama uma função por família por meio de um slot da tabela e depois armazena o vptr; as cinco funções por família eram idênticas e o ICF as dobrou na 2379 (slots 144/159/469/477/483) | EPatternError (331), EUePatternError (235), EUeComumDadosError (170), EUeComumAsnError (255), EUeIoError (210) |

As três cópias diferem apenas no tratamento de exceções da chamada, o que reflete como cada unidade de tradução foi
compilada (ver u11 sobre TUs compiladas sem `invoke`). As cinco famílias da variante 2294/2379 são exatamente as cinco
cujo destrutor de exclusão é a 1940 e não a 454 (§12), o que aponta para as unidades de tradução que definem essas
famílias, e não para o template.

---

## 4. Geradores aleatórios: `IRng`, `CTrng`, `CPrng`

### 4.1 Interface

| slot | método (nomes inferidos) | CTrng | CPrng |
|---|---|---|---|
| 2 | `uebyte GeraByte()` | 9477: `random_device("/dev/urandom")() & 0xFF` | 9502: primeira saída de um mt19937 novo, byte **mais alto** |
| 3 | `int Gera()` | 9476: uniforme sobre [INT_MIN+1, INT_MAX-1] | 9500: mesmo intervalo, mt19937 novo |
| 4 | `int Gera(vector<uebyte>& b)` | 9475: `Gera(b, b.size())` | 9499: igual |
| 5 | `int Gera(vector<uebyte>& b, size_t n)` | 5173: 0 ok, 1 n==0, 2 b pequeno demais, **-1** em `std::exception` | 5178: 0 / 1 / 2 (sem catch) |

O "int uniforme" é o laço de rejeição `do r = rng(); while (r > 0xFFFFFFFD); return r - 0x7FFFFFFF`
(2^32 - 2 valores começando em -2147483647). Os chamadores tiram restos dele (`CPoliticaExecucaoEleitor`: `% 4 + 3`
passadas e `% 100 + 50` ms, restos com sinal; `CControlaArmazenamentoDeImagens`: `% 999999` sem sinal).

### 4.2 `CTrng`

Sem estado. Toda chamada constrói um `std::random_device` a partir de uma `std::string` estática "/dev/urandom" (@1911784, construída
por `__wasm_call_ctors`, liberada pela 9478). Esta libc++ não aceita nenhum outro token e sorteia com `getentropy` =
`random_get` do WASI = `crypto.getRandomValues` no navegador. Seu único usuário é `CBlockCipher<CAesCipher, CTrng>`
(u01). `CAesCipher::Encrypt` (9493) sorteia um IV de 16 bytes com `Gera(iv, 16)` **somente quando sua `CAesKey` não tem IV**.
A chave do RDV vem com um IV fixo (`CHKDFSeed` seed[32..48), u01 §3.1), então os votos gravados nunca sortearam de
`CTrng` (a 5173 não foi observada executando). Só uma cifra vinda de `Create(segredo)` (IV vazio) sortearia. Todo
chamador dessa factory no binário lê (decifra) um arquivo de chave (§10, u01 §2.6), e a revisão encontrou apenas
chamadas de 3 argumentos ao slot 3 (`Decrypt`) nessas cifras.

### 4.3 `CPrng`: determinístico por construção

`CPrng` guarda apenas uma semente de 32 bits (+4). Todo método constrói um novo **`mt19937` do Boost.Random** na pilha (o
código de inicialização da semente contém o `normalize_state()` do boost: `x[0]` ajustado a partir de `x[396] ^ x[623]` com `0x321161BF`, depois a
verificação de tudo zero), sorteia e descarta o engine. Os bytes vêm de `boost::random::uniform_int_distribution`
(saída `>> 24`, não `& 0xFF` como daria o std::). `main` (func 10307) registra `CPrng(0)` como o `IRng` da
aplicação. Consequências, **verificadas chamando as funções no simulador em execução**:

| chamada (semente 0) | resultado, toda vez |
|---|---|
| `GeraByte()` | 140 |
| `Gera()` | 209652397 |
| `Gera(buf, 32)` | `8c97b7d89adb8bd86c9fa562704ce40ef645627acacf877a9164ecd6125616a5` |
| `Gera(buf, 0)` / `Gera(buf(32), 33)` | 1 / 2 |

Uma segunda peculiaridade: `GeraByte` e `Gera(buf, n)` carregam a semente com `i32.load8_u` (só o byte baixo), `Gera()` com
`i32.load`. A sonda confirmou isso: a semente 256 dá os mesmos bytes que a semente 0 (`140`, `8c97b7d8...`), mas um
`Gera()` diferente (-1950843743). Usuários do singleton `IRng`: `comum::CGravadorBU` (o "número aleatório
da UE" CEPESC de 32 bytes do BU, cgravadorbu.cpp:485), `comum::CGravadorRCSecao` (32 bytes, cgravadorrcsecao.cpp:110),
`CControlaArmazenamentoDeImagens` (nomes de arquivo das imagens de digitais), `CPoliticaExecucaoEleitor` (não usado pela
página web, que registra `CPoliticaExecucaoEleitorWeb`).

A constante é visível em uma saída real: o arquivo de comparecimento gravado pelo harness do BU
(`samples/bu-real/run-full/analysis/jufa.decoded.txt`) tem `DadosCifracao.salt` = `8c97b7d89adb8bd86c9fa562704ce40e`
e `informacaoAdicional` = `f645627acacf877a9164ecd6125616a5`, isto é, as duas metades de `Gera(buf, 32)` acima, e
`chave` = 32 bytes zero (o stub do CEPESC, §5.2). Toda execução do simulador produz o mesmo salt.

---

## 5. Cifras simétricas e CEPESC

### 5.1 `CSymmetricCipherFactory`

* **`Create(const CAesKey&)`** (9489, observada: constrói a cifra do RDV em `votaInit`, chave/IV vindos de `CHKDFSeed`, u02):
  `new CBlockCipher<CAesCipher, CTrng>(key, CBC)` (36 bytes, o ctor 9474 de `ISymmetricCipher` verifica o tamanho da chave),
  envolvida em um `std::shared_ptr` criado a partir do ponteiro cru (não `make_shared`).
* **`Create(const std::string& segredo)`** (1884): derivação de chave para os arquivos de chave.
  1. `SHA-512(segredo)` com `CSha512` (2684: digest "SHA2-512" do OpenSSL, 64 bytes);
  2. **chave AES-256 = bytes 16..47 do digest** (o wasm copia 4 x 8 bytes dos offsets 16, 24, 32, 40 do digest);
  3. IV vazio, tamanho de chave 256, depois o `Create(CAesKey)` virtual.
  O doc da u01, `openssl.md` e o comentário de `csha.hpp` da u13 dizem "first 32 bytes" / `[0..32)`; isso está errado. O harness do BU já depende de `[16:48]`
  (`tools/bu/bu_real_seeds.py` cifra `jufa.pk1` com `SHA-512(32 zero bytes)[16:48]` e o código real
  o decifra), o que confirma a leitura.

  Chamadores e o que é `segredo`: `vota::CGeraBU` (chave do *código verificador* `cv.ber.pri`), `comum::CGravadorBU`
  (`bu.pk1`), `comum::CGravadorRCSecao` (`jufa.pk1`), `CControlaArmazenamentoDeImagens`,
  `CConversorBiometriaEleitorCifrada`. Em todos eles o segredo vem de `api::IKernelHSM`, que não tem
  implementação no build web (u01 §5).

### 5.2 `CCepescCipher`: uma "cifra" identidade neste build

`Encrypt(const CPlainText&)` (2681) retorna `CCipheredOut(32 zero bytes, plain.conteudo[, *plain.infoSalt])`; o
tipo de arquivo, o id de criptografia, a zona/seção, a tabela criptográfica de 1024 bytes da urna, o número aleatório e a chave pública
são ignorados. `Decrypt` (5171, u03) retorna o conteúdo sem alteração. Assim, o envelope "cifrado" do BU, o arquivo de comparecimento
e as imagens de digitais do simulador são texto claro com uma chave de 32 bytes zero (`Seguranca.idArquivoChave` do
envelope, `DadosCifracao.chave` do arquivo de comparecimento; esta última e seu `conteudo` BER em claro são visíveis em
`samples/bu-real/run-full/analysis/jufa.decoded.txt`).

---

## 6. Helpers de DataHoraJE

`DataHoraJE` (ModuloTiposEleitorais) é uma GeneralString `"YYYYMMDDThhmmss"`.

* `FormataDataHoraJE(ptime)` (9220) = `std::format("{:04}{:02}{:02}T{:02}{:02}{:02}", ...)`, um thunk para o
  corpo mesclado 6155, que ela compartilha com `api::FormataAAAAMMDDhhmmss` (thunk 2685). Nesse corpo, o ano passa por
  `std::stoi(std::to_string(year))`. Mês e dia são os campos simples de 16 bits.
* `ConverteDataHoraJE(string)` (1877): se o texto tem >= 9 caracteres e termina em `'Z'`, ele é remontado como
  `s.substr(0,8) + "T" + s.substr(8,6)` (GeneralizedTime do ASN.1 `YYYYMMDDhhmmssZ`; o fuso é descartado, sem conversão
  para UTC), depois `boost::date_time::parse_iso_time(s, 'T')`. Verificado no módulo em execução:

| entrada | resultado |
|---|---|
| `20240101T120000` | 212570913600000000 µs |
| `20240101120000Z` | o mesmo |
| `20240101T120000Z` | lança `boost::wrapexcept<boost::bad_lexical_cast>` |
| `20241301T120000` | lança `boost::wrapexcept<boost::gregorian::bad_month>` |

Datas malformadas em um arquivo de dados, portanto, levantam exceções do Boost, não `CError`.

---

## 7. Conversores ASN.1 e classes de dados

### 7.1 Padrão

Todo conversor implementa `DoConverte` (classe de dados -> objeto ASN.1) e `DoDeconverte` (o inverso); os
`Converte`/`Deconverte` públicos de `IConversorASN` (u11) validam o lado ASN.1 (`isValid() && isStrictlyValid()`, erros
1900/1902). Nenhum dos corpos de conversores da u40 valida nada por conta própria: as verificações de intervalo acontecem nos construtores
`CBaseType` do lado dos dados (CPatternError 1300, `cbasetype.hpp:39`) e no runtime da III. Conversores compostos
constroem os subconversores na pilha e chamam o `Converte` deles; listas passam por `ConverteLista` (u14, corpo 1970)
e são atribuídas com o copy-and-swap do runtime (thunks `SEQUENCE_OF(first,last)` 9122/9139 para o corpo 2926).
Campos opcionais usam o `includeOptionalField` gerado (func 515) / `removeOptionalField` (shared_f432).

### 7.2 O que cada conversor mapeia

| conversor | tipo ASN.1 (módulo) | classe de dados (layout) | observações |
|---|---|---|---|
| RegistroIdentificacaoEleitor 9114/9112 | `{identificacaoUtilizada, identificacaoPrincipal OPTIONAL}` (TiposEcoUrna) | `CRegistroIdentificacaoEleitor` (20 B) | principal presente se e somente se o opcional está preenchido; usado para eleitores, mesários e justificativas do arquivo de comparecimento |
| IdentificadorGeradorMidia 9225/9224 | `{nome, serialCertificadoTPM, serialInstalacao}` | 3 x std::string (36 B) | dados do simulador: "simulador-votacao-ng", 64 x '0'; 9224 observada por meio de `CConversorDadoCorrespondencia` (11400, eg.bin) |
| MunicipioZona 9131 | `{municipio 1..99999, zona 1..9999}` | `CMunicipioZona` (8 B) | |
| IdentificacaoSecaoEleitoral 9129 | `{municipioZona, local, secao}` | `CIdentificacaoSecaoEleitoral` (16 B; ctor 5110) | |
| DetalhamentoComparecimento 9222 | BU `[1] {semBiometria, porBiometria, porBiografia}` | 3 x u16 | **BU**, §10 |
| HorariosUrna 9133/9132 | 4 x DataHoraJE | `CHorariosUrna` (4 x ptime) | observada na inicialização (`-cfm.dat`) |
| ConfiguracaoMunicipio 9144 | `{codigoMunicipio, horariosUrna}` | POD de 40 B | |
| ConfiguracaoMunicipios 9141 | `{cabecalho, SEQUENCE OF ConfiguracaoMunicipio}` | `CConfiguracaoMunicipios` (cabeçalho + map) | map achatado na ordem das chaves |
| ParametrizacaoUrna 9173 | `{cabecalho, parametros}` | `CParametrizacaoUrna` | só do lado do gravador; a urna lê `-pu.dat` |
| Federacao 9216 | `{identificador 100..999, sigla, nome, partidos SEQUENCE OF 0..99}` | `CFederacao` (40 B) | `CBaseType<u16,100,999,39>` "FederacaoID" inlinado (BASIC_TYPE a partir da assinatura do srcloc @1124484) |
| Federacoes 9212 | `{cabecalho, federacoes OPTIONAL}` | `CFederacoes` | lista vazia -> campo ausente |
| Autenticacao 9201 | `{[1] dataInicial OPT, [2] dataFinal OPT, tamanhoSenha, numeroTentativas, hashSenha}` | `CAutenticacao` (56 B) | datas gravadas só se ambas estiverem presentes |
| DadosGeracaoMidia 9203/9202 | `{serialMidia, usuario, identificadorGeradorMidia, data}` | `CDadosGeracaoMidia` (72 B) | serial como texto hexadecimal (9257) / `CSerialMidia` (9259) |
| ComparecimentoSecao 9126 | `{identificacao, SEQUENCE OF EstadoComparecimento}` | `CComparecimentoSecao` | uma entrada por eleitor do cadastro |
| IdentificacaoJustificativa 9065 | `{identificacaoEleitor, anoNascimentoEleitor 0..9999}` | `CIdentificacaoJustificativa` (24 B) | |
| DadosCifracao 9062 | 3 x OCTET STRING (chave, salt, informacaoAdicional) | `CDadosCifracao` (36 B) | salt/info >= 16 bytes verificado pela classe de dados; cada vetor passa pelo helper por valor 9063 |
| DadosComparecimentoCifrado 9060 | `{dadosCifracao, conteudo}` | `CDadosComparecimentoCifrado` (48 B; ctor 3484) | usado quando o parâmetro `criptografarJUFA` está ligado |

### 7.3 Construtores das classes de dados

`CCabecalhoEntidade(ptime, int id, ETipoId)` (5112; ETipoId = índice do CHOICE: processo eleitoral / pleito / eleição),
`CIdentificacaoSecaoEleitoral` (5110), `CHabilitacaoBiometrica` sem (5087) e com (2657) um
`CEstadoHabilitacaoPorCodigo`, `CEstadoComparecimento(registro, situação)` (2658), `CResultadoUrnaCadastro` em claro
(3483) e cifrado (3482), `CDadosComparecimentoCifrado` (3484). Todos são membro a membro (cópias ou moves de parâmetros
passados por valor), sem verificações. Seus layouts concordam com os headers da u14.

### 7.4 O fluxo do arquivo de resultado de comparecimento (`-jufa.dat`)

No fim da votação, `comum::CGravadorRCSecao` (func 11616, u23) monta um `CResultadoUrnaCadastro`:
cabeçalho (5112), fase, versão da votação, situação (final/parcial) e ou `CDadosComparecimento` em claro (3483) ou,
com `criptografarJUFA`, um `CDadosComparecimentoCifrado` (3484, 3482) formado pela saída CEPESC (2681) do BER dos
dados de comparecimento mais `CDadosCifracao{chave, salt, info}`. Os registros de comparecimento são montados com 2658/5087/2657.
`CConversorResultadoUrnaCadastro` (u14) então chama, por meio de `CConversorDadosComparecimento::DoConverte` (9102,
u14), os conversores desta unidade: 9126 (seção: 9129 + 9131, e `CConversorEstadoComparecimento` por eleitor), 9065
(justificativas), 9114 (toda identificação de eleitores e mesários) e 9060/9062 para a variante cifrada.
Os 32 bytes aleatórios da variante cifrada vêm do `CPrng` determinístico (§4.3): eles acabam como o
`salt` e o `informacaoAdicional` de `DadosCifracao`, constantes no simulador; a cifra stub (§5.2) os ignora.

---

## 8. Código da `uenux2` que as ferramentas arquivaram aqui

| func | símbolo real | onde é usado |
|---|---|---|
| 1924 | `comum::md::CEleitorIdentidade::ToString() const` | exibição de um identificador: título `"xxxx xxxx xxxx"` (espaços inseridos em 8 e depois em 4), CPF `"xxx.xxx.xxx-xx"` (`-` em 9, `.` em 6 e 3), id livre como está, tipo desconhecido `""`. Textos do operador (hospedeiro de `TipoToStr` 10586), relatório da lista de eleitores (11233), `CGeraRelatorios` (12105) |
| 2840 | `comum::CCandidaturas::GetNumerosCandidatosAptos(TCargoID) const` | números das candidaturas de um cargo cuja palavra de situação é 0, na ordem do map. Observada: telas de votação, o JSON `"candidates"` da página web (u29), QR codes do BU (5604) e impressão dos candidatos do BU (11214) |
| 2815 | `comum::CPartidos::~CPartidos()` | singleton da tabela de partidos (map + nome "CPartidos") |
| 5615 | `comum::CCalculaCV::Reinicia(const std::string&)` | código verificador do BU, já reconstruído em `ccalculacv.cpp` (u25) |
| 3692 | `comum::md::CNomesCargo::operator=` (implícito, 4 strings) | dentro de `CCargo::operator=` (unknown_f5594) |
| 5593 | laço de cópia de `md::CRespostaConsulta` (itens de 28 bytes) | idem |
| 11502 / 11503 | atexit: mutex estático de `CPartidos::GetInst` / `unique_ptr<CPartidos>` | cpartidos.cpp |
| 11506 | atexit: mutex estático de `CLocal::GetInst` (@1838876) | clocal.cpp |
| 11509 / 11510 | atexit: `CHV::s_mutex` / ponteiro da instância de `CHV` (@1838848 / @1838872) | chv.cpp |
| 11512 | atexit: mutex estático de `CFotos::GetInst` (@1838820) | cfotos.cpp |

---

## 9. Código de biblioteca nesta unidade

* **libc++**: `string::assign(const char*, n)` 986, `vector<string>::__push_back_slow_path` 1108,
  corpo mesclado de `vector<T>::__construct_at_end` 2898, `locale::operator==` 3340, `__throw_bad_any_cast` 3861,
  `ErrorHandler<void>::report` do filesystem 4792, `vector<uint16_t>::push_back` 9214, `vector<uebyte>::insert` 9492,
  o laço de cópia de `CRespostaConsulta` 5593.
* **Runtime ASN.1 da III** (código inline/de template instanciado em TUs da ecourna): ctor de cópia de `AbstractString` 1671,
  ctor de cópia de `INTEGER` 5082, thunks `SEQUENCE_OF(first,last)` 9122/9139.
* **helper 9063 da ecourna**: constrói um `ASN1::OCTET_STRING` a partir de um `std::vector<uebyte>` e o retorna por valor. Ele
  **não** é um construtor: seu tipo é `(i32,i32)->void` e os dois chamadores (9060, 9062) o alcançam por meio de `invoke_vii`.
  Todo construtor neste build retorna `this` e seria invocado como `invoke_iii`. Ele é reconstruído como
  `ConverteOctetString` (nome inferido) em `cconversordadoscifracao.u40.cpp`.
* **Boost**: `mt19937::twist` 5179; o insert por intervalo de `std::map` de `signals2::detail::grouped_list` 9551
  (comparador de chave de grupo: meta grupo, depois o int opcional só para `grouped_slots`).
* **7-Zip** (linkado para `CLzmaCompress`, que é código morto, u12): `CBuffer<Byte>::Alloc` 2614,
  `CArcErrorInfo::operator=` 3437, `SetProperties` 4993.
* **SQLite**: o corpo compartilhado de `sqlite3_column_text`/`_text16` 6077.
* **Helpers de enum mesclados pelo wasm-opt** dos conversores (as funções do código-fonte são reconstruídas pela u14):
  6144 (`v` ASN -> `v-1`, intervalo 3, lança `CAsnResultadoUrnaCadastroError`), 6148 (base 0 -> `v+1`, também intervalo 3, mensagem
  copiada de uma string estática, `CAsnParametrizacaoUrnaError`), 6151 (identidade com intervalo `[1,k]` para um ENUMERATED).
* **Destrutores estáticos** (atexit): 8378 (mutex de CLzmaCompress), 9431 (mutex de CSqlStatement), 9478 (token de CTrng),
  9496 (`s_cipherTable` de CAesCipher), 9150/9151 (mensagens de titulo-relatório), 9170 (mensagem de label), 9166
  (`"Forma de validar título inválido."`, um estático que nenhum código usa).

---

## 10. Boletim de Urna (BU): o que esta unidade contribui

O BU é gerado por `vota::CGeraBU` / `comum::CGeradorBU*`, gravado por `comum::CGravadorBU` e convertido por
`comum::asn::CConversorEntidadeBU` (unidades u06-u09, u21, u23; docs/10-boletim-de-urna.md, docs/bu/codepath.md).
A u40 fornece estas peças, na ordem em que o encerramento as usa:

1. **O arquivo de chave do CV** (`vota::CGeraBU::StartState`, 12110). `/dsk/fi/estatico/chave/cv.ber.pri` é uma
   `EntidadeChave`; se sua chave estiver cifrada, a cifra é `CSymmetricCipherFactory::Create(segredo)` (1884) com
   `segredo` vindo de `api::IKernelHSM`: **AES-256-CBC, chave = SHA-512(segredo)[16..48)**, IV = últimos 16 bytes da
   entrada. Os primeiros 16 bytes da chave resultante são a chave SipHash de `comum::CCalculaCV`.
2. **A cadeia do código verificador.** Cada vez que um novo CV é iniciado, `CCalculaCV::Reinicia` (5615) redefine a entrada
   do MAC para `tipo ('F' or 'A') + identificação + cv` e memoriza `cv` (docs/bu/codigo-verificador.md).
3. **Listas de candidatos.** `CCandidaturas::GetNumerosCandidatosAptos` (2840) fornece, por cargo, os números dos candidatos
   aptos usados pelo gerador de QR code do BU (`CGeradorBUQRCode`, 5604: campos `CARG`/`TIPO`/`VERC` ...) e pela
   parte de candidatos da zerésima (11214). A cadeia de hashes do payload do QR usa `CSha512` (2684).
4. **`detalhamentoComparecimento`.** `vota::CGravaResultado` (12098) monta
   `CDetalhamentoComparecimento(semBiometria, porBiometria, porBiografia)` (ctor 5094) a partir de três contadores de
   `CEleitores`. `CGravadorBU::MontaEntidadeBU` só o mantém para uma urna biométrica (u23 §3.4);
   `CConversorEntidadeBU` então preenche `EntidadeBoletimUrna.[1] detalhamentoComparecimento` com
   `CConversorDetalhamentoComparecimento::Converte` (9222 + a verificação de validade de `IConversorASN`), ou omite o
   campo. ASN.1 exato: `DetalhamentoComparecimento ::= SEQUENCE { qtdEleitoresCompareceramSemBiometria INTEGER
   (0..9999), qtdEleitoresHabilitadosPorBiometria INTEGER (0..9999), qtdEleitoresHabilitadosPorBiografia INTEGER
   (0..9999) }`. O harness do BU produziu `{1, 0, 0}` para o seu único eleitor (docs/bu/codepath.md §4).
5. **O envelope do arquivo** (`CGravadorBU::GravaResultado`, 11629). A chave pública do BU `bu.pk1` é decifrada com a
   mesma cifra 1884; depois `CPlainText(tipoArquivo 0, idCriptografia 1, zona, seção, tabela[1024], número aleatório,
   chave pública, BER(EntidadeBoletimUrna))` passa por `CCepescCipher::Encrypt` (2681) e o resultado vira
   `Seguranca{idTipoArquivo 0, idCriptografia 1, idArquivoChave = out.chave}` + `conteudo` do
   `EntidadeEnvelopeGenerico` gravado como `…-bu.dat`. Neste build: `idArquivoChave` = 32 bytes zero, `conteudo` =
   o BU não cifrado, e o "número aleatório" (32 bytes de `IRng::Gera(buf)`) é sempre
   `8c97b7d8…16a5` (§4.3), ignorado de qualquer forma pelo stub.
6. **Erros.** Qualquer falha desse caminho é um `CBaseError<…>` cujo `what()` tem a forma da §3.1.

O arquivo de comparecimento (`-jufa.dat`) gravado ao lado do BU por `CGravadorRCSecao` é descrito na §7.4.

---

## 11. O que é específico do build web

* `main` registra `CPrng(0)` como o único `IRng`: todo valor "aleatório" da aplicação é uma constante (§4.3).
* `CCepescCipher` não cifra (§5.2); `api::IKernelHSM` não tem implementação, então `Create(segredo)` (1884)
  nunca é alcançado pela página (o harness do BU fornece um HSM falso que retorna 32 bytes zero).
* `CTrng` sortearia de `crypto.getRandomValues` (`random_get` do WASI), mas a página nunca chega a ele: o IV da cifra
  do RDV é fixado por `CHKDFSeed` (u01 §3.1), então o arquivo do RDV é idêntico byte a byte de uma execução para outra.
* Os conversores de fim de dia (comparecimento, detalhe do BU) estão linkados, mas só são alcançáveis pelo harness do BU.
* Nada nesta unidade lê parâmetros da URL, mexe com a rede ou com o DOM.

---

## 12. Observações notáveis sobre wasm / Emscripten

* **merge-similar-functions por toda parte.** Três corpos para os 45 construtores de exceção (§3.2); três para os helpers
  de enum dos conversores (6144/6148/6151, que recebem a mensagem, a localização no código-fonte, o código, o slot do construtor
  e o typeinfo como parâmetros); um para `vector::__construct_at_end` (2898, copiador de elementos passado como slot da tabela);
  um para as duas funções de texto de coluna do SQLite (6077); `FormataDataHoraJE` compartilha o corpo de um formatador de `api`.
* **ICF.** Cinco funções "parte CError" por família dobradas na 2379 (cinco slots da tabela). O destrutor de exclusão
  das famílias de erro existe duas vezes. A 1940 chama `~CError` (4636) e serve CError mais as 5 famílias da
  variante 2294/2379 (EPatternError, EUePatternError, EUeComumDadosError, EUeComumAsnError, EUeIoError).
  `icf_shared_vf1@454` tem os dois destrutores de string inlinados e serve 39 vtables: 37 famílias `CBaseError<>` mais
  as derivadas `io::CIoError` e `api::CUeDesligandoError`. `api::CUePrinterError` tem o seu próprio (4910). São
  decisões de inlining diferentes em TUs diferentes, cada uma depois dobrada pelo ICF.
* **Construtores retornam `this`** (ABI C++ do wasm), por isso 5110/5112/2657/2658/3482-3484 terminam com `return a`.
* **`std::string`/`std::optional`/vetores por valor são movidos** com uma cópia i64 + i32 do objeto de 12 bytes seguida
  do zeramento da origem (`c[0]:long@4 = 0; c[2] = 0`), p. ex. em 1143, 3482-3484.
* **Boost vs. std** podem ser distinguidos no código de RNG inlinado: a constante `0x321161BF` do `normalize_state` do boost e
  a extração de byte com `>> 24` identificam o Boost.Random em `CPrng`; `CTrng` usa a distribuição e o random device da libc++.
* **Objetos estáticos** (mensagens `std::string`, o token `/dev/urandom`, `std::map`s, `unique_ptr`s de singletons e os
  `std::mutex`es com pthread stubado) ganham cada um uma pequena função atexit (9478, 9496, 9150/9151/9166/9170, 8378, 9431,
  11502-11512); um destrutor de mutex é só o resíduo noexcept dobrado pelo ICF (func 150) aplicado ao seu endereço.
* **Um estático não usado.** A 9166 destrói `"Forma de validar título inválido."` (@1912244): construído na inicialização, lido por
  nada - a mensagem de um conversor que não está linkado.

---

## 13. Código suspeito ou digno de nota

| # | func | o quê | impacto | severidade |
|---|---|---|---|---|
| 1 | 9502/9500/9499/5178 (+ main 10307) | `CPrng` reinicializa um mt19937 novo com a mesma semente a cada chamada, e o `main` web o inicializa com 0: `GeraByte()` = 140, `Gera()` = 209652397, `Gera(buf,32)` = `8c97b7d8…` para sempre (verificado em tempo de execução) | simulador: o "número aleatório da UE" CEPESC do BU, o `salt`/`informacaoAdicional` gravado em todo arquivo de comparecimento (visto em `samples/bu-real/run-full`) e os ids das imagens de digitais armazenadas (`Gera() % 999999` = 652606) são constantes; `GerarCaminhosUnicos` repete `while (file exists)` com essa constante, então uma segunda imagem WSQ no mesmo diretório travaria a página (de thread única) (u24 #1; latente, a página web não armazena nenhuma imagem). Em uma urna real, mesmo com uma semente aleatória, esta classe retornaria um único valor por instância | média |
| 2 | 9502, 5178 | os métodos que produzem bytes usam como semente os **8 bits baixos** da semente (`i32.load8_u`), `Gera()` usa todos os 32 bits (verificado: semente 256 == semente 0 para bytes) | no máximo 256 fluxos de bytes distintos, seja qual for a semente | baixa |
| 3 | 2681 | `CCepescCipher::Encrypt` retorna o texto claro com uma "chave de sessão" de 32 bytes zero; tabela, número aleatório e chave pública ignorados | simulador: todo arquivo de resultado "cifrado" é texto claro (segurança simulada); se o mesmo stub fosse linkado em uma urna, os dados do BU/JUFA/biométricos não estariam protegidos | média |
| 4 | 5173 (+ CAesCipher 9493) | `CTrng::Gera(buf,n)` captura `std::exception` e retorna -1; `CAesCipher::Encrypt` ignora o status | só uma cifragem cuja chave **não** tem IV sorteia de `CTrng`. Se a fonte de entropia falhasse, esse IV seria silenciosamente os 16 bytes zero que a 9493 pré-preenche, em vez de um erro. O RDV não é afetado (seu IV é fixado por `CHKDFSeed`), e as cifras sem IV deste binário só são usadas para decifrar arquivos de chave, então a falha é latente | baixa |
| 5 | 1884 | chave = SHA-512 puro do segredo do HSM, bytes 16..47 (sem salt/KDF); docs anteriores do projeto afirmam bytes 0..31 | erro de documentação corrigido aqui (confirmado pelo harness do BU); a derivação em si é tão forte quanto o segredo do HSM | info |
| 6 | 1877 | `ConverteDataHoraJE`: um `Z` final significa "GeneralizedTime sem T": `YYYYMMDDThhmmssZ` é desfigurado e rejeitado; o marcador UTC é descartado sem conversão; erros de parsing são exceções do Boost (`bad_lexical_cast`, `bad_month`), não `CError` (verificado) | uma data malformada em `-cfm.dat`/infomidia/dados de comparecimento escapa dos handlers de `CError` e termina no caminho genérico de "exceção desconhecida" da thread | baixa |
| 7 | 1143 | `what()` embute a assinatura C++ completa e a linha da função que lança; a formatação é envolvida em `catch (...) {}` | o texto chega aos logs e a `ShowExceptionMsg`, mas a tela de "urna inoperante" usa `GetMensagem()` + código (7710), e a 7709 remove o prefixo com uma regex, então ele **não** é mostrado aos eleitores. Resta apenas o fallback silencioso para a mensagem simples em caso de `bad_alloc` | info |
| 8 | 9201 (e 9200, u11) | a janela de validade de Autenticacao só é gravada/lida quando AMBAS as datas existem; uma data isolada é descartada silenciosamente | uma mídia com apenas `dataFinal` perderia sua expiração na ida e volta | baixa |
| 9 | 9166 | mensagem estática "Forma de validar título inválido." sem usuário | indício de código morto (conversor removido ou não linkado) | info |
| 10 | 9585 | `CLzmaCompress::OnProgress` trunca o progresso de 64 bits para 32 bits; a classe nunca é construída | código morto | info |

---

## 14. Tabela de mapeamento completa (todas as 101 funções da u40)

`run` = observada executando durante os votos gravados. As linhas "biblioteca" são código de libc++/Boost/7-Zip/SQLite/runtime ASN.1,
corpos mesclados pelo wasm-opt de funções reconstruídas em outro lugar, ou destrutores estáticos gerados pelo compilador; elas são
descritas em comentários do arquivo listado ou nas §8-§9, e nenhum código-fonte foi escrito para elas. *(caminho inferido)* = sem
`std::source_location` para o arquivo.

| func | tamanho | run | símbolo reconstruído | arquivo original | src | conf. |
|---:|---:|:-:|---|---|---|---|
| 710 | 114 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (merged body, no landing pad)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (caminho inferido) | src/ecourna/api/exception/cbaseerror.hpp | alta |
| 986 | 164 | ✔ | `std::string::assign(const char*, size_t) (__assign_external)` | libcxx/include/string | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 1011 | 181 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (merged body, EH variant)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (caminho inferido) | src/ecourna/api/exception/cbaseerror.hpp | alta |
| 1108 | 268 | ✔ | `std::vector<std::string>::__push_back_slow_path(const std::string&)` | libcxx/include/vector | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 1143 | 1778 |  | `ecourna::api::exception::CError::CError` | ecourna-lib/ecourna/api/exception/cerror.cpp (caminho inferido) | src/ecourna/api/exception/cerror.cpp | alta |
| 1671 | 44 |  | `ASN1::AbstractString::AbstractString(const AbstractString&)` | runtime ASN.1 da III asn1.h (inline) | biblioteca: lib:asn1-runtime (nenhum arquivo escrito) | alta |
| 1877 | 897 | ✔ | `ecourna::api::util::ConverteDataHoraJE` | ecourna-lib/ecourna/api/util/datahora.cpp (caminho inferido) | src/ecourna/api/util/datahora.cpp | média |
| 1884 | 1167 |  | `ecourna::api::security::CSymmetricCipherFactory::Create(const std::string&)` | ecourna-lib/ecourna/api/security/csymmetriccipherfactory.cpp (caminho inferido) | src/ecourna/api/security/csymmetriccipherfactory.cpp | alta |
| 1924 | 334 |  | `comum::md::CEleitorIdentidade::ToString` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.u40.cpp | média |
| 1940 | 10 |  | `ecourna::api::exception::CError::~CError (deleting)` | ecourna-lib/ecourna/api/exception/cerror.cpp (caminho inferido) | src/ecourna/api/exception/cerror.cpp | alta |
| 2379 | 123 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (CError part, 5 instantiations ICF)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (caminho inferido) | src/ecourna/api/exception/cbaseerror.hpp | média |
| 2614 | 72 |  | `CBuffer<Byte>::Alloc (7-Zip CByteBuffer)` | CPP/Common/MyBuffer.h | biblioteca: lib:7zip (nenhum arquivo escrito) | alta |
| 2657 | 197 |  | `ecourna::app::dados::CHabilitacaoBiometrica::CHabilitacaoBiometrica (with CEstadoHabilitacaoPorCodigo)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp | alta |
| 2658 | 167 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(const CRegistroIdentificacaoEleitor&, ESituacaoComparecimento)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.u40.cpp | alta |
| 2681 | 345 |  | `ecourna::api::cepesc::CCepescCipher::Encrypt` | ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp (caminho inferido) | src/ecourna/api/security/cepesc/ccepesccipher.cpp | alta |
| 2684 | 276 | ✔ | `ecourna::api::security::CSha512::CSha512` | ecourna-lib/ecourna/api/security/csha.hpp (caminho inferido) | src/ecourna/api/security/security.u40.cpp | alta |
| 2815 | 37 |  | `comum::CPartidos::~CPartidos` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | média |
| 2840 | 150 | ✔ | `comum::CCandidaturas::GetNumerosCandidatosAptos(TCargoID)` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/dados/ccandidaturas.u40.cpp | média |
| 2898 | 77 | ✔ | `std::vector<T>::__construct_at_end(first, last, n) (merged body)` | libcxx/include/vector | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 3340 | 63 | ✔ | `std::locale::operator==` | libcxx/src/locale.cpp | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 3437 | 58 |  | `CArcErrorInfo::operator=(const CArcErrorInfo&) (7-Zip, implicit)` | CPP/7zip/UI/Common/OpenArchive.h | biblioteca: lib:7zip (nenhum arquivo escrito) | média |
| 3482 | 365 |  | `ecourna::app::dados::CResultadoUrnaCadastro::CResultadoUrnaCadastro (encrypted data)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp | alta |
| 3483 | 445 |  | `ecourna::app::dados::CResultadoUrnaCadastro::CResultadoUrnaCadastro (clear data)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp | alta |
| 3484 | 236 |  | `ecourna::app::dados::CDadosComparecimentoCifrado::CDadosComparecimentoCifrado` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp | alta |
| 3517 | 60 |  | `ecourna::api::security::CAesKey::~CAesKey (implicit)` | ecourna-lib/ecourna/api/security/isymmetriccipher.hpp (caminho inferido) | src/ecourna/api/security/security.u40.cpp | alta |
| 3692 | 449 |  | `comum::md::CNomesCargo::operator=(const CNomesCargo&) (implicit)` | uenux2/src/app/comum/dados/md/processoeleitoral/cnomescargo.h (caminho inferido) | biblioteca: app:comum (nenhum arquivo escrito) | média |
| 3861 | 44 |  | `std::__throw_bad_any_cast` | libcxx/include/any | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 4636 | 32 |  | `ecourna::api::exception::CError::~CError` | ecourna-lib/ecourna/api/exception/cerror.cpp (caminho inferido) | src/ecourna/api/exception/cerror.cpp | alta |
| 4792 | 181 |  | `std::filesystem::detail::ErrorHandler<void>::report(const error_code&, const char*, ...)` | libcxx/src/filesystem/error.h | biblioteca: rt:libcxx (nenhum arquivo escrito) | média |
| 4993 | 1359 |  | `SetProperties(IUnknown*, const CObjectVector<CProperty>&) (7-Zip)` | CPP/7zip/UI/Common/SetProperties.cpp | biblioteca: lib:7zip (nenhum arquivo escrito) | média |
| 5082 | 40 |  | `ASN1::INTEGER::INTEGER(const INTEGER&)` | runtime ASN.1 da III asn1.h (inline) | biblioteca: lib:asn1-runtime (nenhum arquivo escrito) | alta |
| 5087 | 46 |  | `ecourna::app::dados::CHabilitacaoBiometrica::CHabilitacaoBiometrica` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp | alta |
| 5110 | 34 |  | `ecourna::app::dados::CIdentificacaoSecaoEleitoral::CIdentificacaoSecaoEleitoral` | ecourna-lib/ecourna/app/dados/cidentificacaosecaoeleitoral.cpp (caminho inferido) | src/ecourna/app/dados/cidentificacaosecaoeleitoral.cpp | alta |
| 5112 | 34 |  | `ecourna::app::dados::CCabecalhoEntidade::CCabecalhoEntidade` | ecourna-lib/ecourna/app/dados/ccabecalhoentidade.cpp (caminho inferido) | src/ecourna/app/dados/ccabecalhoentidade.cpp | alta |
| 5170 | 84 |  | `std::unique_ptr<ecourna::api::cepesc::CInfoSalt>::reset` | ecourna-lib/ecourna/api/security/cepesc/cinfosalt.hpp (caminho inferido) | src/ecourna/api/security/security.u40.cpp | média |
| 5173 | 233 |  | `ecourna::api::security::CTrng::Gera(std::vector<uebyte>&, std::size_t)` | ecourna-lib/ecourna/api/security/ctrng.cpp (caminho inferido) | src/ecourna/api/security/ctrng.cpp | alta |
| 5178 | 1059 |  | `ecourna::api::security::CPrng::Gera(std::vector<uebyte>&, std::size_t)` | ecourna-lib/ecourna/api/security/cprng.cpp (caminho inferido) | src/ecourna/api/security/cprng.cpp | alta |
| 5179 | 680 |  | `boost::random::mersenne_twister_engine<uint32_t,32,624,397,31,...>::twist` | boost/random/mersenne_twister.hpp | biblioteca: lib:boost (nenhum arquivo escrito) | alta |
| 5593 | 293 |  | `std::__copy_loop for comum::md::CRespostaConsulta (copy-assign range)` | libcxx/include/__algorithm/copy.h | biblioteca: rt:libcxx (nenhum arquivo escrito) | média |
| 5615 | 249 |  | `comum::CCalculaCV::Reinicia` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp (unidade u25) | alta |
| 6077 | 246 |  | `sqlite3_column_text/_text16 shared body (columnMem + sqlite3ValueText + columnMallocFailure)` | sqlite3.c | biblioteca: lib:sqlite (nenhum arquivo escrito) | alta |
| 6144 | 258 |  | `enum Deconverte helper (ASN v -> v-1, range 3) merged body` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/ (wasm-opt merge-similar) | biblioteca: lib:ecourna (nenhum arquivo escrito) | alta |
| 6148 | 295 |  | `enum Converte helper (0-based -> ASN v+1, message from static string) merged body` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/ (wasm-opt merge-similar) | biblioteca: lib:ecourna (nenhum arquivo escrito) | alta |
| 6151 | 268 |  | `enum Converte helper (identity, range [1,k]) merged body` | ecourna-lib/ecourna/app/dados/asn/(wasm-opt merge-similar) | biblioteca: lib:ecourna (nenhum arquivo escrito) | alta |
| 8378 | 10 |  | `atexit: ~std::mutex of CLzmaCompress s_mutex (@1923244)` | ecourna-lib/ecourna/api/compression/clzmacompress.cpp | src/ecourna/api/compression/clzmacompress.u40.cpp | alta |
| 9060 | 394 |  | `ecourna::app::dados::asn::CConversorDadosComparecimentoCifrado::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp (caminho inferido) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.u40.cpp | alta |
| 9062 | 634 |  | `ecourna::app::dados::asn::CConversorDadosCifracao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp (caminho inferido) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp | alta |
| 9063 | 141 |  | `ecourna::app::dados::asn::ConverteOctetString(const std::vector<uebyte>&)` (helper por valor que retorna `ASN1::OCTET_STRING`; não é um construtor: `(i32,i32)->void`, chamado via `invoke_vii`) | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/ (caminho inferido) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp | baixa |
| 9065 | 268 |  | `ecourna::app::dados::asn::CConversorIdentificacaoJustificativa::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp (caminho inferido) | src/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.u40.cpp | alta |
| 9112 | 613 |  | `ecourna::app::dados::asn::CConversorRegistroIdentificacaoEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp | alta |
| 9114 | 843 |  | `ecourna::app::dados::asn::CConversorRegistroIdentificacaoEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp | alta |
| 9122 | 26 |  | `ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::EstadoComparecimento>::SEQUENCE_OF(first, last)` | runtime ASN.1 da III (instanciação de template) | biblioteca: lib:asn1-runtime (nenhum arquivo escrito) | alta |
| 9126 | 592 |  | `ecourna::app::dados::asn::CConversorComparecimentoSecao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (caminho inferido) | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.u40.cpp | alta |
| 9129 | 251 |  | `ecourna::app::dados::asn::CConversorIdentificacaoSecaoEleitoral::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.u40.cpp | alta |
| 9131 | 54 |  | `ecourna::app::dados::asn::CConversorMunicipioZona::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversormunicipiozona.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversormunicipiozona.cpp | alta |
| 9132 | 134 | ✔ | `ecourna::app::dados::asn::CConversorHorariosUrna::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp (caminho inferido) | src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp | alta |
| 9133 | 1852 |  | `ecourna::app::dados::asn::CConversorHorariosUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp (caminho inferido) | src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp | alta |
| 9139 | 26 |  | `ASN1::SEQUENCE_OF<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio>::SEQUENCE_OF(first, last)` | runtime ASN.1 da III (instanciação de template) | biblioteca: lib:asn1-runtime (nenhum arquivo escrito) | alta |
| 9141 | 1272 |  | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipios::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp (caminho inferido) | src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.u40.cpp | alta |
| 9144 | 297 |  | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipio::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp (caminho inferido) | src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.u40.cpp | alta |
| 9150 | 36 |  | `atexit: ~std::string MSG_ALINHAMENTO_INVALIDO (@1912268)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp (u14, estático declarado lá) | alta |
| 9151 | 36 |  | `atexit: ~std::string MSG_ESTILO_INVALIDO (@1912256)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp (u14, estático declarado lá) | alta |
| 9166 | 36 |  | `atexit: ~std::string "Forma de validar título inválido." (@1912244, unused static)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | biblioteca: lib:ecourna (nenhum arquivo escrito) | média |
| 9170 | 36 |  | `atexit: ~std::string MSG_GENERO_INVALIDO (@1912232)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp (u14, estático declarado lá) | alta |
| 9173 | 349 |  | `ecourna::app::dados::asn::CConversorParametrizacaoUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp (caminho inferido) | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.u40.cpp | alta |
| 9181 | 18 |  | `ecourna::api::exception::CBaseError<ecourna::app::dados::asn::EAsnMidiasError, SErrorLimits{2485,2510}>::CBaseError` | ecourna-lib/ecourna/app/dados/dadoserros.h (caminho inferido) | src/ecourna/api/exception/cbaseerror.hpp (lista de thunks) | alta |
| 9201 | 1371 |  | `ecourna::app::dados::asn::CConversorAutenticacao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp (caminho inferido) | src/ecourna/app/dados/asn/midias/cconversorautenticacao.u40.cpp | alta |
| 9202 | 621 |  | `ecourna::app::dados::asn::CConversorDadosGeracaoMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp (caminho inferido) | src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp | alta |
| 9203 | 1234 |  | `ecourna::app::dados::asn::CConversorDadosGeracaoMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp (caminho inferido) | src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp | alta |
| 9212 | 492 |  | `ecourna::app::dados::asn::CConversorFederacoes::DoConverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (caminho inferido) | src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.u40.cpp | alta |
| 9214 | 252 |  | `std::vector<unsigned short>::push_back(const unsigned short&)` | libcxx/include/vector | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 9216 | 1629 |  | `ecourna::app::dados::asn::CConversorFederacao::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp (caminho inferido) | src/ecourna/app/dados/asn/federacoes/cconversorfederacao.u40.cpp | alta |
| 9220 | 17 |  | `ecourna::api::util::FormataDataHoraJE` | ecourna-lib/ecourna/api/util/datahora.cpp (caminho inferido) | src/ecourna/api/util/datahora.cpp | média |
| 9222 | 307 |  | `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.cpp | alta |
| 9224 | 423 | ✔ | `ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp | alta |
| 9225 | 666 |  | `ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp (caminho inferido) | src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp | alta |
| 9398 | 203 |  | `ecourna::api::util::CStringUtils::Replace(std::string&, char, const std::string&)` | ecourna-lib/ecourna/api/util/cstringutils.cpp | src/ecourna/api/util/cstringutils.u40.cpp | média |
| 9431 | 10 |  | `atexit: ~std::mutex of CSqlStatement (@1911796)` | ecourna-lib/ecourna/api/sql/sqlite/csqlstatement.cpp | src/ecourna/api/sql/sqlite/csqlstatement.cpp (comentário da u13) | alta |
| 9470 | 491 |  | `ecourna::api::cepesc::CInfoSalt::CInfoSalt(const CInfoSalt&) (implicit)` | ecourna-lib/ecourna/api/security/cepesc/cinfosalt.cpp | src/ecourna/api/security/security.u40.cpp | alta |
| 9475 | 20 |  | `ecourna::api::security::CTrng::Gera(std::vector<uebyte>&)` | ecourna-lib/ecourna/api/security/ctrng.cpp (caminho inferido) | src/ecourna/api/security/ctrng.cpp | alta |
| 9476 | 105 |  | `ecourna::api::security::CTrng::Gera` | ecourna-lib/ecourna/api/security/ctrng.cpp (caminho inferido) | src/ecourna/api/security/ctrng.cpp | alta |
| 9477 | 90 |  | `ecourna::api::security::CTrng::GeraByte` | ecourna-lib/ecourna/api/security/ctrng.cpp (caminho inferido) | src/ecourna/api/security/ctrng.cpp | alta |
| 9478 | 36 |  | `atexit: ~std::string TOKEN "/dev/urandom" of CTrng (@1911784)` | ecourna-lib/ecourna/api/security/ctrng.cpp (caminho inferido) | src/ecourna/api/security/ctrng.cpp | alta |
| 9488 | 75 |  | `ecourna::api::security::CBlockCipher<CAesCipher, CTrng>::~CBlockCipher (deleting)` | ecourna-lib/ecourna/api/security/cblockcipher.hpp | src/ecourna/api/security/security.u40.cpp | alta |
| 9489 | 282 | ✔ | `ecourna::api::security::CSymmetricCipherFactory::Create(const CAesKey&)` | ecourna-lib/ecourna/api/security/csymmetriccipherfactory.cpp (caminho inferido) | src/ecourna/api/security/csymmetriccipherfactory.cpp | alta |
| 9492 | 1177 |  | `std::vector<uebyte>::insert(const_iterator, first, last)` | libcxx/include/vector | biblioteca: rt:libcxx (nenhum arquivo escrito) | alta |
| 9496 | 18 |  | `atexit: ~std::map s_cipherTable of CAesCipher (@1911772)` | ecourna-lib/ecourna/api/security/caescipher.cpp | src/ecourna/api/security/security.u40.cpp | alta |
| 9499 | 20 |  | `ecourna::api::security::CPrng::Gera(std::vector<uebyte>&)` | ecourna-lib/ecourna/api/security/cprng.cpp (caminho inferido) | src/ecourna/api/security/cprng.cpp | alta |
| 9500 | 370 |  | `ecourna::api::security::CPrng::Gera` | ecourna-lib/ecourna/api/security/cprng.cpp (caminho inferido) | src/ecourna/api/security/cprng.cpp | alta |
| 9502 | 332 |  | `ecourna::api::security::CPrng::GeraByte` | ecourna-lib/ecourna/api/security/cprng.cpp (caminho inferido) | src/ecourna/api/security/cprng.cpp | alta |
| 9503 | 21 |  | `ecourna::api::security::CPrng::CPrng` | ecourna-lib/ecourna/api/security/cprng.cpp (caminho inferido) | src/ecourna/api/security/cprng.cpp | alta |
| 9528 | 185 |  | `ecourna::api::pattern::IObservableProgressWithDescription::~IObservableProgressWithDescription (deleting)` | ecourna-lib/ecourna/api/pattern/iobservableprogresswithdescription.hpp (caminho inferido) | src/ecourna/api/pattern/iobservableprogresswithdescription.u40.cpp | alta |
| 9551 | 1095 |  | `std::map<group_key_type, list_iterator, group_key_less<int>>::insert(first, last) (boost::signals2 grouped_list)` | boost/signals2/detail/slot_groups.hpp | biblioteca: lib:boost (nenhum arquivo escrito) | média |
| 9585 | 769 |  | `ecourna::api::compression::CLzmaCompress::OnProgress` | ecourna-lib/ecourna/api/compression/clzmacompress.cpp | src/ecourna/api/compression/clzmacompress.u40.cpp | média |
| 11502 | 10 |  | `atexit: ~std::mutex of CPartidos::GetInst (@1838904)` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | alta |
| 11503 | 37 |  | `atexit: ~std::unique_ptr<comum::CPartidos> (@1838928)` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | alta |
| 11506 | 10 |  | `atexit: ~std::mutex of comum::CLocal::GetInst (@1838876)` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp (comentário da u02) | alta |
| 11509 | 10 |  | `atexit: ~std::mutex comum::CHV::s_mutex (@1838848)` | uenux2/src/app/comum/dados/chv.cpp | src/uenux2/src/app/comum/dados/chv.h (estático declarado lá) | alta |
| 11510 | 12 |  | `atexit: ~std::unique_ptr<comum::CHV> s_inst (@1838872)` | uenux2/src/app/comum/dados/chv.cpp | src/uenux2/src/app/comum/dados/chv.h (estático declarado lá) | alta |
| 11512 | 10 |  | `atexit: ~std::mutex of comum::CFotos::GetInst (@1838820)` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp (comentário da u21) | alta |
| 11562 | 21 |  | `ecourna::api::exception::CError::what` | ecourna-lib/ecourna/api/exception/cerror.cpp (caminho inferido) | src/ecourna/api/exception/cerror.cpp | alta |

---

## 15. Questões em aberto

* `CError` +24: quatro bytes que o construtor nunca escreve (padding ou um membro definido em outro lugar).
* A forma exata, no código-fonte, do truncamento da semente de `CPrng` (`static_cast<uebyte>`, um helper com template ou um
  argumento do engine com tipo `uebyte`), e se a urna real registra `CPrng`, `CTrng` ou um RNG de hardware como `IRng`
  (o `main` que faz o registro neste build é o da web).
* Se `CCepescCipher` é um stub só da web ou a única implementação do CEPESC nesta árvore de código-fonte.
* 2379: por que cinco famílias de CBaseError chamam sua parte CError por meio de uma função por família (um construtor de encaminhamento
  com template explicaria cinco corpos idênticos). As mesmas cinco famílias são as únicas usuárias do destrutor de exclusão
  fora de linha 1940, então a resposta provavelmente está em como as TUs que as definem foram compiladas.
* (resolvido) O tipo "FederacaoID" é `CBaseType<unsigned short, 100, 999, 39>`. O registro de srcloc @1124484 que a
  verificação inlinada passa contém a assinatura completa.
* O nome real e o lugar da 9063: é um helper por valor (não um construtor de `OCTET_STRING`) compartilhado pela 9060 e pela 9062.
* Nomes de arquivo: todas as classes da u40 não têm srcloc; as escolhas de diretório (p. ex. `asn/` vs. `asn/resultadournacadastro/` para
  `CConversorRegistroIdentificacaoEleitor`, `asn/` vs. `asn/boletimurna/` para `CConversorDetalhamentoComparecimento`,
  `api/util/datahora.*` para os helpers de data) são inferências. Outras unidades incluem alguns desses headers sob
  diretórios diferentes.
* Divergência de layout a resolver no `cinformacaomidia.h` da u14: a 9201 lê `numeroTentativas` de +32 e
  `tamanhoSenha` de +36 (a u11 concorda; o comentário da u14 os tem invertidos).

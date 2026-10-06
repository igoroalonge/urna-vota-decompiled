# u11 — `ecourna::api::asn`: o template validador de conversores ASN.1 (`IConversorASN`) e os conversores do ecourna que o inlinam

A unidade u11 tem **90 funções**; **16** executaram durante os votos gravados (`analysis/runtime/*.functions.tsv`).
O seu único arquivo original atestado é o header **`ecourna-lib/ecourna/api/asn/iconversorasn.hpp`** da biblioteca
*ecourna* do TSE (pacote Conan `libecea1da310e5107`). O header define um template de classe,
`ecourna::api::asn::IConversorASN<ENTIDADE, DADO>`: a base de todo *conversor* do ecourna entre um
objeto ASN.1 decodificado de / codificado para um arquivo de dados (a *entidade*) e um objeto de dados C++ simples (o *dado*). Os seus dois
métodos não virtuais verificam o objeto ASN.1 com os validadores ASN.1 da III e lançam um `CBaseError<EApiAsnError>`
(códigos **1900** / **1902**) quando ele é inválido. Uma execução com um `-cfm.dat` deliberadamente corrompido (§6.1) mostra que a
camada de arquivos (`api::CFileASN`) faz a mesma verificação antes, de modo que, para arquivos de dados, este template é uma segunda linha de defesa;
para o BU, é a verificação que protege as contagens de `detalhamentoComparecimento` (§7.1).

Como o template é inlinado em toda parte, os nomes de `std::source_location` da ferramenta se espalham para os chamadores: a unidade
também recebeu 13 métodos de conversores, 5 instâncias de `std::transform`, 5 construtores, 3 destrutores implícitos e 12
helpers de contêineres das classes de dados do ecourna. A tabela abaixo resume; a §12 mapeia cada índice.

| grupo | # | funções |
|---|---|---|
| instanciações out-of-line de `Converte`/`Deconverte` (thunks de 18–21 bytes + 2 cópias de CHOICE) | 43 | 27 `Converte` + 16 `Deconverte` |
| corpos de template mesclados (wasm-opt *merge-similar-functions*) | 6 | 682, 1167, 6031, 6032, 6149, 6150 |
| helpers do template | 2 | 858 `std::stringstream()`, 1074 construtor de `CBaseError<EApiAsnError>` |
| corpos `DoConverte`/`DoDeconverte` de conversores (nomes reais recuperados das vtables) | 13 | 9057 9061 9064 9095 9102 9120 9127 9137 9142 9171 9200 9207 11440 |
| instâncias de `std::transform` cuja lambda é um `Deconverte` inlinado | 5 | 9092 9094 9119 9176 9206 |
| construtores de classes de dados | 5 | 9029 9034 9037 9040 9045 |
| destrutores implícitos de classes de dados | 3 | 1006 1876 5097 |
| helper inline OCTET STRING → `std::vector<uebyte>` | 1 | 5095 |
| instanciações de contêineres da libc++ / do ASN.1 da III | 12 | 306 2660 2661 3488 5096 5099 5100 6145 6146 9099 9118 9174 |

Fontes reconstruídas (§13): `src/ecourna/api/asn/iconversorasn.hpp` (+ `iconversorasn.instances.cpp`, um mapa das
instanciações), dez arquivos de conversores sob `src/ecourna/app/dados/asn/…` (caminhos inferidos), três pequenos arquivos de classes de dados,
dois fragmentos `.u11.cpp` para arquivos pertencentes a u14, `src/ecourna/app/dados/u11-foreign-fragments.cpp` e
`src/uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp`.

---

## 1. Glossário

| termo | significado |
|---|---|
| *entidade* (`TEntidade`, `ENTIDADE`) | uma classe gerada a partir dos módulos ASN.1 do TSE (`ModuloTiposEleitorais::CabecalhoEntidade`, `ModuloResultadoUrnaCadastro::DadosComparecimento`, …), construída sobre o runtime ASN.1 da III (`docs/libraries/asn1-runtime.md`). Codificada como BER nos arquivos |
| *dado* (`TDado`, `DADO`) | a classe de dados do ecourna com que a aplicação trabalha (`ecourna::app::dados::CDadosComparecimento`, `CFoto`, …) ou um escalar com verificação de faixa (`CBaseType<T, MIN, MAX, ID>`) |
| **Converte** / **Deconverte** | dado → entidade (para gravar um arquivo) / entidade → dado (após ler um arquivo). O ecourna escreve *Deconverte*; o gêmeo do uenux2, `comum::asn::IConversorASN`, escreve *Desconverte* |
| *DoConverte* / *DoDeconverte* | os métodos de trabalho virtuais puros (slots 2 e 3 da vtable) que cada conversor concreto implementa |
| *cabeçalho* (`CabecalhoEntidade`) | cabeçalho de todo arquivo de dados: data-hora de geração (`DataHoraJE`, texto `YYYYMMDDTHHMMSS`) + id eleitoral (CHOICE `IDEleitoral`) |
| *comparecimento* | quem veio votar na seção (*votou*, *faltou*, *não votou*…) e como cada eleitor foi habilitado |
| *habilitação* | habilitar o eleitor na urna: por impressão digital (*biometria*), digitando um código (*por código*, feito pelo *mesário*) ou por áudio |
| *mesário* | `ComparecimentoMesario` registra os mesários identificados na *abertura* e no *encerramento* |
| *justificativa* | um eleitor fora do seu domicílio que justifica, nesta urna, não votar, em vez de votar (título + ano de nascimento são registrados) |
| *parametrização da urna* (`-pu.dat`) | chaves de comportamento, títulos de relatórios e rótulos da aplicação de votação |
| *configuração de municípios* (`-cfm.dat`) | horário por município: impressão da zerésima, início e fim da votação, fim do dia de trabalho |
| *federação* (`-fe.dat`) | federação partidária: um grupo de partidos que concorre como um só |
| *mídia* / *InformacaoMidia* | um cartão flash ou pendrive preparado para a urna e o seu arquivo de descrição `infomidia-*.dat` |
| *JUFA* | o nome do arquivo de chave pública `jufa.pk1` usado para cifrar os dados de comparecimento (veja §7.2) |

---

## 2. Classes e como se relacionam

### 2.1 RTTI

```
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>        35 instantiations, typeinfo only (__class_type_info: abstract,
 │                                                       no vtable of its own), iconversorasn.hpp
 └─ ecourna::app::dados::asn::CConversorXxx             35 converters, one per instantiation, all "si" (single
                                                         inheritance), sizeof 4 (vptr only), built on the caller's stack
      vtable: [0] ~T() = ICF 174 "return this"   [1] ~T() deleting = 144 free
              [2] DoConverte(const TDado&) -> TEntidade     [3] DoDeconverte(const TEntidade&) -> TDado

users outside ecourna (they build ecourna converters on their stack): comum::asn::CConversorCarga,
CConversorDadoCorrespondencia, CConversorBiometriaEleitor, CConversorFotoCandidato, CConversorVotosCargo,
CConversorEleicoesVota, CConversorEntidadeBU (BU), comum::CGravadorRCSecao (attendance file), api::CFileASN

ecourna::api::exception::CBaseError<ecourna::api::asn::EApiAsnError, SErrorLimits{1900, 1910}> : CError
     typeinfo 1563144, vtable 1563164; constructor thunk = func 1074 (shared body 710)
```

Toda vtable de conversor tem quatro slots; o par `DoConverte`/`DoDeconverte` é **virtual puro** no template do
ecourna (todas as 35 subclasses implementam ambos). Isso difere do gêmeo do uenux2, `comum::asn::IConversorASN`
(`uenux2/src/app/comum/asn/iconversorasn.h`, unidade u21), cujos slots da base lançam *"Método DoConverte() não implementado
para {}"*.

Instanciações presentes no binário (ENTIDADE → DADO):

| módulo ASN.1 | entidades convertidas por meio de `IConversorASN` |
|---|---|
| `ModuloTiposEleitorais` | CabecalhoEntidade, Fase (→ `CFaseID`), Turno (→ `CBaseType<ushort,1,2,11>`), Foto, MunicipioZona, IdentificacaoSecaoEleitoral, IdentificadorEleitor (CHOICE), CodigoCargoConsulta (→ `CBaseType<ushort,1,99,3>`) |
| `ModuloTiposEcoUrna` | IdentificadorGeradorMidia, RegistroIdentificacaoEleitor |
| `ModuloResultadoUrnaCadastro` | EntidadeResultadoUrnaCadastro, DadosComparecimento, DadosComparecimentoCifrado, DadosCifracao, ComparecimentoSecao, ComparecimentoMesario, IdentificacaoJustificativa, EstadoComparecimento, EstadoHabilitacaoPorCodigo, HabilitacaoBiometrica, ApresentacaoFotoEleitor |
| `ModuloConfiguracaoMunicipios` | EntidadeConfiguracaoMunicipios, ConfiguracaoMunicipio, HorariosUrna |
| `ModuloParametrizacaoUrna` | EntidadeParametrizacaoUrna, ParametrosUrna, TituloRelatorio, LabelParametrizado |
| `ModuloInformacaoMidia` | InformacaoMidia, DadosGeracaoMidia, Aplicativo, Autenticacao |
| `ModuloFederacoes` | EntidadeFederacoes, Federacao |
| `ModuloBoletimUrna` | **DetalhamentoComparecimento** (o detalhamento do comparecimento no BU, §7.1) |

### 2.2 Classes de conversores cujos métodos estão nesta unidade

| classe (namespace `ecourna::app::dados::asn`, salvo indicação) | vtable | slot 2 `DoConverte` | slot 3 `DoDeconverte` | arquivo (todos os caminhos inferidos) |
|---|---|---|---|---|
| `CConversorDadosComparecimento` | 1132868 | **9102** | **9095** | `asn/resultadournacadastro/cconversordadoscomparecimento.cpp` |
| `CConversorDadosComparecimentoCifrado` | 1137068 | 9060 (u40) | **9057** | `…/cconversordadoscomparecimentocifrado.cpp` |
| `CConversorDadosCifracao` | 1136812 | 9062 (u40) | **9061** | `…/cconversordadoscifracao.cpp` |
| `CConversorIdentificacaoJustificativa` | 1136564 | 9065 (u40) | **9064** | `…/cconversoridentificacaojustificativa.cpp` |
| `CConversorComparecimentoSecao` | 1131064 | 9126 (u40) | **9120** | `…/cconversorcomparecimentosecao.cpp` |
| `CConversorIdentificacaoSecaoEleitoral` | 1130788 | 9129 (u40) | **9127** | `asn/cconversoridentificacaosecaoeleitoral.cpp` |
| `CConversorConfiguracaoMunicipios` | 1129768 | 9141 (u40) | **9137** | `asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp` |
| `CConversorConfiguracaoMunicipio` | 1129456 | 9144 (u40) | **9142** | `asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp` |
| `CConversorParametrizacaoUrna` | 1127436 | 9173 (u40) | **9171** | `asn/parametrizacaourna/cconversorparametrizacaourna.cpp` |
| `CConversorAutenticacao` | 1125420 | 9201 (u40) | **9200** | `asn/midias/cconversorautenticacao.cpp` |
| `CConversorFederacoes` | 1124660 | 9212 (u40) | **9207** | `asn/federacoes/cconversorfederacoes.cpp` |
| `comum::asn::CConversorFotoCandidato` (uenux2) | 1562588 | 11439 (base, lança) | **11440** `DoDesconverte` | `uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp` |

Escolha de diretório: os conversores de u14 atestados por srcloc ficam em `ecourna/app/dados/asn/` (tipos de ModuloTiposEleitorais)
e em subdiretórios que espelham as classes de dados (`asn/midias/`, `asn/parametrizacaourna/`,
`asn/resultadournacadastro/`). Os novos arquivos seguem essa regra.

---

## 3. O template: contrato e fluxo de controle

Reconstrução: `src/ecourna/api/asn/iconversorasn.hpp`. Os números de linha 49 e 66 e a coluna 19 dos dois throws coincidem
exatamente com os registros de source_location (12 espaços de indentação + `throw `).

```cpp
TEntidade Converte(const TDado& dado) const {                      // "write" direction
    TEntidade entidade = DoConverte(dado);                          // vtable slot 2
    if (!entidade.isValid() || !entidade.isStrictlyValid()) {       // III ValidChecker + StrictlyValidChecker
        const std::string nome = std::string(typeid(TEntidade).name()) + ": ";
        std::stringstream erro;
        ASN1::trace_invalid(erro, nome.c_str(), entidade);          // InvalidTracer, Portuguese messages
        throw CApiAsnError(EApiAsnError(1900), erro.str());         // iconversorasn.hpp:49
    }
    return entidade;
}
TDado Deconverte(const TEntidade& entidade) const {                // "read" direction: check FIRST
    if (!entidade.isValid() || !entidade.isStrictlyValid()) { … throw CApiAsnError(EApiAsnError(1902), …); }   // :66
    return DoDeconverte(entidade);                                  // vtable slot 3
}
```

* **O que é verificado.** `isValid` (func 221) e `isStrictlyValid` (func 231) percorrem toda a árvore do objeto por meio das
  tabelas de info: faixas de INTEGER, restrições SIZE de strings e de SEQUENCE OF, a alternativa de CHOICE selecionada e, para
  ENUMERATED, apenas o limite superior `value <= maxEnumValue` (`ValidChecker::do_visit(const ENUMERATED&)`, func 8984; não
  há limite inferior, veja §10.4). Um arquivo decodificado cujos valores violam o esquema é recusado *antes* que qualquer objeto de dados seja construído, e um
  objeto de dados que produziria um campo fora da faixa é recusado *antes* que o arquivo seja codificado.
* **Mensagem de erro.** `"<typeinfo name>: <tracer text>"`. O tracer imprime o caminho pontuado do campo inválido e o
  motivo, então uma contagem do BU fora da faixa apareceria como
  `N17ModuloBoletimUrna26DetalhamentoComparecimentoE: .qtdEleitoresHabilitadosPorBiometria Valor 10000 para campo
  INTEGER é maior que seu limite superior 9999\n` (formato confirmado em tempo de execução com o mesmo tracer, veja §6.1;
  `trace_invalid`, func 208, grava prefixo + texto do tracer + `endl`, então a mensagem termina com uma quebra de linha). O nome
  é a string bruta do typeinfo (o ponteiro constante é passado por cada thunk), não um nome demangled. O InvalidTracer
  imprime caminhos, tamanhos, limites e no máximo o único caractere problemático de uma string (`" O caractere 'x' não é
  válido"`), não valores completos.
* **Tipo de erro.** `CBaseError<EApiAsnError, SErrorLimits{1900, 1910}>`, objeto de exceção de 40 bytes, construído pela
  func 1074 (um thunk que passa a vtable para o corpo compartilhado do construtor de `CBaseError`, 710) e lançado com
  `__cxa_throw(obj, typeinfo 1563144, dtor slot 69)`. Só 1900 e 1902 ocorrem no binário (1901 não é usado).
* **Nenhum catch aqui.** A exceção vai para quem carrega o arquivo (ex.: o carregador de dados estáticos, func 7787, ou
  `api::CFileASN`) ou quem o grava (os gravadores do BU e do RC).
* **Comparação com o gêmeo do uenux2** (`comum::asn::IConversorASN`, u21/u03): mesma estrutura, mas códigos
  `EUeComumAsnError` 7653 (`Converte`, iconversorasn.h:56, `"Entidade deixada em estado inválido: {}"`) e 7654
  (`Desconverte`, :71, `"Entidade está inválida: {}"`), formatados com `std::format` em vez do prefixo com o nome bruto do tipo.

---

## 4. Como o template aparece no wasm

### 4.1 Thunks e corpos mesclados

O wasm-opt mesclou todas as instanciações out-of-line que diferem apenas em constantes em **seis corpos compartilhados**. Cada
instanciação é um thunk de 20/21 bytes `body(result, this, arg, &srcloc, typeid(TEntidade).name())`:

| corpo | direção | tipo | chamadores |
|---|---|---|---|
| **682** | Converte | entidades SEQUENCE, `invoke_*` + landing pads (no unwind destrói a string, o stringstream e a entidade via `ASN1::SEQUENCE::~SEQUENCE`) | 22 thunks |
| **1167** | Deconverte | `invoke_*` + landing pads | 11 thunks |
| **6031** | Converte | o mesmo código-fonte **sem nenhum landing pad** | 3734, 5365 |
| **6032** | Deconverte | o mesmo código-fonte sem landing pads | 3733, 5708 |
| **6149** | Deconverte | entidade ENUMERATED: `isValid && isStrictlyValid` inlinado em `value > info->maxEnumValue` (com sinal, `info + 16`); `TDado` pequeno retornado em um registrador | 5103 (Fase), 9178 (Turno) |
| **6150** | Converte | resultado ENUMERATED, mesmo teste inlinado | 9187 (Turno), 9188 (Fase) |

A instanciação de **CHOICE** (`IdentificadorEleitor`: título / CPF / identificação livre) não é mesclada: a busca de nomes
encontra `ASN1::CHOICE::isValid` (func 1068) e `ASN1::CHOICE::isStrictlyValid` (func 1067, ainda rotulada
"comum_f1067") em vez das versões de `AbstractData`, então 9111 (Deconverte) e 9113 (Converte) são cópias completas.

### 4.2 Dois modelos de exceção para o mesmo template

6031/6032 têm a lógica de 682/1167, mas são compilados **sem nenhum `invoke_*`**: nada é destruído se
`trace_invalid` ou o throw fizerem unwind através deles. A geração de código difere em mais um ponto: eles chamam
`std::string::append(const char*)` (`strlen` em tempo de execução de `": "`) onde 682/1167 chamam `append(": ", 2)` com o comprimento
já resolvido na compilação, então vêm de uma unidade de tradução compilada de forma diferente. As suas quatro instanciações são exatamente as out-of-line
que o código do uenux2 também chama (`CConversorCarga`, `CConversorDadoCorrespondencia`, `CGravadorRCSecao`,
`CConversorBiometriaEleitor`, `CConversorFotoCandidato`); nenhum dos 33 thunks de 682/1167 tem chamador no uenux2.
Inferência (não demonstrável a partir do binário): o linker manteve a cópia COMDAT do uenux2, então os chamadores do ecourna
(`CConversorDadosGeracaoMidia::DoDeconverte` 9202 / `DoConverte` 9203, `CConversorResultadoUrnaCadastro::DoConverte` 9056)
também a usam.
Isso coincide com um padrão mais amplo: na saída descompilada por arquivo, **189 de 402 funções do ecourna** usam `invoke_*`,
contra **14 de 634** em `app-comum` e **1 de 388** em `app-vota`. A aplicação uenux2 é compilada com a captura de exceções
desabilitada para quase todas as funções (o capítulo do RHVoice, `docs/libraries/rhvoice.md` §8.5, constatou o mesmo para
aquela biblioteca), enquanto a biblioteca ecourna compilada via Conan tem suporte completo a exceções C++. `comum::asn::CConversorFotoCandidato
::DoDesconverte` (11440, nesta unidade) é um exemplo de função do uenux2 sem landing pads.

### 4.3 Armadilha de nomes: dez funções nomeadas a partir de um `Deconverte` inlinado

Um registro de source_location pertence à função que o contém após o inlining. Quando `Deconverte` é inlinado em
outra função, a ferramenta nomeia essa função como `IConversorASN<X>::Deconverte`. O slot da vtable ou o padrão de chamada
decide a identidade real:

| func | nome da ferramenta | identidade real | evidência |
|---|---|---|---|
| 9057 | `IConversorASN<DadosCifracao,…>::Deconverte` | `CConversorDadosComparecimentoCifrado::DoDeconverte` | slot 3 da vtable |
| 9120 | `IConversorASN<IdentificacaoSecaoEleitoral,…>::Deconverte` | `CConversorComparecimentoSecao::DoDeconverte` | slot 3 da vtable |
| 9127 | `IConversorASN<MunicipioZona,…>::Deconverte` | `CConversorIdentificacaoSecaoEleitoral::DoDeconverte` | slot 3 da vtable |
| 9142 | `IConversorASN<HorariosUrna,…>::Deconverte` | `CConversorConfiguracaoMunicipio::DoDeconverte` | slot 3 da vtable |
| 9171 | `IConversorASN<ParametrosUrna,…>::Deconverte` | `CConversorParametrizacaoUrna::DoDeconverte` | slot 3 da vtable |
| 9092 | `IConversorASN<ComparecimentoMesario,…>::Deconverte` | `std::transform<…, back_insert_iterator<vector<CComparecimentoMesario>>, λ>` | assinatura `(first, last, out, op) -> out`, laço sobre `AbstractData*` |
| 9094 | `IConversorASN<IdentificacaoJustificativa,…>::Deconverte` | `std::transform` (justificativas) | idem |
| 9119 | `IConversorASN<EstadoComparecimento,…>::Deconverte` | `std::transform` (eleitores da seção) | idem |
| 9176 | `IConversorASN<Aplicativo,…>::Deconverte` | `std::transform` (aplicativos de uma mídia, `CConversorInformacaoMidia` de u14) | idem |
| 9206 | `IConversorASN<Federacao,…>::Deconverte` | `std::transform` (federações) | idem |

As instâncias de `std::transform` têm a lambda `[&conversor](const X& x) { return conversor.Deconverte(x); }` inlinada:
`op` é passado como um único ponteiro (o conversor capturado), e o laço chama `conversor->vtable[3]` após a
verificação de validade inlinada. É `std::transform` (e não `std::ranges::transform`) porque o resultado é um único `i32` (o
`back_insert_iterator`), não um `in_out_result` retornado pela memória.

### 4.4 Outras notas sobre o wasm

* **Artefato de anotação.** O descompilador anota a chamada do construtor como
  `env_invoke_iiiii(671 /* &simulador::CWasmBeep::vf0 */4 /* "Chave desconhecida: {}" */, …)`. O operando real
  é o slot de tabela **6714** = func 1074 (construtor de `CBaseError<EApiAsnError>`). O anotador casou o slot 671 dentro do
  número 6714 e depois decodificou o `4` restante como um endereço de string. A saída bruta de `q.py wat` mostra
  `i32.const 6714`.
* A func 858, rotulada `ecourna_f858`, é o **construtor padrão de `std::basic_stringstream<char>`** da libc++ (três
  vtables, `ios_base::init`, modo do stringbuf 24 = `in|out`). Os seus 28 chamadores são todos caminhos de erro de validação ASN.1: os
  seis corpos mesclados e as doze outras funções desta unidade que contêm o código do template (9057, 9092, 9094,
  9111, 9113, 9119, 9120, 9127, 9142, 9171, 9176, 9206), os conversores de u14 9066 (`CConversorHabilitacaoBiometrica`) e 9196 (`CConversorAplicativo`), o
  conversor do BU 10273, os conversores do RDV 11349/11351/11352, `CFileASN` 3735/5366, o carregador 7787 e o leitor parcial
  do arquivo de eleitores 11523 (lambda de `CEleitores::GetEleitoresEstaticos`, `CPartialFileASN`). Todos, exceto 11523, também chamam 1074.
* A func 1006 (`~CEstadoComparecimento`) e a 1876 (`~CRegistroIdentificacaoEleitor`) mostram o layout dos registros
  de comparecimento: um `CRegistroIdentificacaoEleitor` é `shared_ptr<IIdentificadorEleitor>` (+0) mais
  `optional<shared_ptr<…>>` (+8, flag +16), 20 bytes.
* Construtores retornam `this`, e resultados maiores que um registrador voltam por um ponteiro de resultado oculto (sret),
  e foi assim que 9029/9034/9045 foram identificados como os construtores do resultado do seu único chamador.

---

## 5. Os corpos de conversores nesta unidade (o que leem e constroem)

Todos os conversores são sem estado; um `DoDeconverte` constrói os seus conversores filhos na pilha (apenas a gravação do vptr) e chama
o `Deconverte` deles, que valida primeiro a entidade filha. Os números de campo se referem à ordem na SEQUENCE de
`src/asn1/*.asn`.

### 5.1 Dados de comparecimento — `ModuloResultadoUrnaCadastro` (o arquivo de `comum::CGravadorRCSecao`; "RC" ≈ *resultado para o cadastro*, expansão inferida)

```
EntidadeResultadoUrnaCadastro { cabecalho, fase, versaoVotacao, situacao, infoDadosComparecimento CHOICE {
    [0] dadosComparecimento        DadosComparecimento,
    [1] dadosComparecimentoCifrado DadosComparecimentoCifrado { dadosCifracao { chave, salt, informacaoAdicional }, conteudo } } }
DadosComparecimento { justificativas SEQUENCE OF IdentificacaoJustificativa, identificacaoComparecimento ComparecimentoSecao,
                      mesariosAbertura [1] SEQUENCE OF ComparecimentoMesario OPTIONAL, mesariosEncerramento [2] … OPTIONAL }
ComparecimentoSecao { identificacao IdentificacaoSecaoEleitoral, eleitores SEQUENCE OF EstadoComparecimento }
```

* **`CConversorDadosComparecimento::DoConverte` (9102)** — a direção que a urna usa no fim do dia. Ordem:
  (1) `identificacaoComparecimento` ← `CConversorComparecimentoSecao::Converte(dado.secao)` (thunk 9101 → 682 → 9126),
  atribuído com o helper de atribuição de SEQUENCE (func 339); (2) o vetor `justificativas` é **copiado**, convertido
  elemento a elemento pelo helper de lista 1970 (thunk 9100, `Converte` = 9097) e atribuído com o copy-and-swap da III
  (`SEQUENCE_OF(first,last)` = 9099, swap, clear); (3) se o opcional `mesariosAbertura` estiver preenchido (byte +52), o
  vetor retornado por `GetMesariosAbertura()` (func 9024, cdadoscomparecimento.cpp:38, lança *"Comparecimento de
  mesários na abertura não definido para este objeto."* quando vazio) é copiado, convertido (9098 → 1970, `Converte` =
  9096), `includeOptionalField(0, 2)` (func 515) e cada elemento clonado no campo 2; caso contrário,
  `removeOptionalField(0)` (func 432); (4) o mesmo para `mesariosEncerramento` (byte +68, 9023, opcional 1 / campo 3). O
  resultado é verificado pelo `Converte` do chamador (thunk 5365 → corpo sem EH 6031).
* **`CConversorDadosComparecimento::DoDeconverte` (9095)** — o inverso: copia a SEQUENCE OF `justificativas`
  (`auto`, func 2656), transforma-a (9094), converte a seção (9093 → 1167 → 9120), depois, para cada lista opcional
  presente, copia-a e a transforma (9092) em `std::optional<std::vector<CComparecimentoMesario>>`, e
  constrói `CDadosComparecimento` (func 3485) a partir de **cópias** das quatro partes (parâmetros por valor).
* **`CConversorComparecimentoSecao::DoDeconverte` (9120)** — id da seção (`Deconverte` inlinado → 9127) + um
  `CEstadoComparecimento` (96 bytes) por eleitor via `std::transform` 9119 → `CComparecimentoSecao(id, eleitores)` (5092).
* **`CConversorIdentificacaoSecaoEleitoral::DoDeconverte` (9127)** — `CMunicipioZona` (`Deconverte` inlinado → 9130),
  depois `CBaseType<uint,0,9999,8>(local)` e `CBaseType<ushort,0,9999,9>(secao)` (com verificação de faixa; `cbasetype.hpp:39`
  lança), → func 5110. Layout `{+0 município (uint), +4 zona (ushort), +8 local (uint), +12 seção (ushort)}`.
* **`CConversorIdentificacaoJustificativa::DoDeconverte` (9064)** — `CRegistroIdentificacaoEleitor` (thunk 9105) +
  `CBaseType<ushort,0,9999,36>(anoNascimentoEleitor)` → `CIdentificacaoJustificativa` (func 1875, que copia o
  registro: dois add-refs de `shared_ptr`).
* **`CConversorDadosCifracao::DoDeconverte` (9061)** — três OCTET STRINGs → três `std::vector<uebyte>` (helper
  5095) → `CDadosCifracao(chave, salt, informacaoAdicional)` (func 5091, por valor; `salt` e `informacaoAdicional`
  precisam ter ≥ 16 bytes, *"O tamanho de salt não pode ser menor do que 16."*).
* **`CConversorDadosComparecimentoCifrado::DoDeconverte` (9057)** — `CDadosCifracao` (`Deconverte` inlinado → 9061)
  + `conteudo` (5095) → `CDadosComparecimentoCifrado` (func 3484).

As funções `…::DoDeconverte` desta família **não** foram observadas em tempo de execução: a urna grava esta estrutura;
lê-la de volta é tarefa de outras ferramentas.

### 5.2 Horário da urna — `ModuloConfiguracaoMunicipios` (`-cfm.dat`)

* **`CConversorConfiguracaoMunicipios::DoDeconverte` (9137)**: cabeçalho (2665), depois, para cada `ConfiguracaoMunicipio`,
  um `CConfiguracaoMunicipio` (9136 → 1167 → 9142) inserido em um vetor; o vetor é transformado em
  `std::map<município, CConfiguracaoMunicipio>` pela func 9134, que constrói e retorna o mapa (sret) com
  semântica de `std::insert_iterator` (cada inserção usa o sucessor da posição anterior como dica; `insert` não
  sobrescreve, então um município repetido mantém a **primeira** entrada) e passado a `CConfiguracaoMunicipios(cabecalho, mapa)`
  (func 9028).
* **`CConversorConfiguracaoMunicipio::DoDeconverte` (9142)**: `CBaseType<uint,0,99999,6>(codigoMunicipio)` primeiro,
  depois `CHorariosUrna` (`Deconverte` inlinado → 9132: quatro textos `DataHoraJE` convertidos em tempos de 64 bits pela func 1877).
* Exemplo decodificado, cenário `municipal-t1`, `t02410ac-cfm.dat` (dump BER):
  cabeçalho `20260910T162219`, idEleitoral `[2] 2410`; municípios 1, 2, 3, cada um com
  zerésima `20261004T070000`, início `20261004T080000`, encerramento `20261004T170000`, término `20261005T040000`.

### 5.3 Parâmetros da urna — `ModuloParametrizacaoUrna` (`-pu.dat`)

**`CConversorParametrizacaoUrna::DoDeconverte` (9171)**: cabeçalho (2665) e `ParametrosUrna` (`Deconverte` inlinado
→ `CConversorParametrosUrna::DoDeconverte` 9156, u14), depois `CParametrizacaoUrna(cabecalho, parametros)` (func 9029,
copia o `CParametrosUrna` de ~400 bytes com a func 3777) e destrói o temporário (2267). Os thunks de `TituloRelatorio` e
`LabelParametrizado` (9153/9154) rodam dentro de 9156 (9153 por meio do helper de lista 5102). Esta é a função mais cara da unidade em tempo de execução
(46/54 amostras), e mais da metade dela é o cabeçalho (a func 9218 interpreta a data-hora com boost).

### 5.4 Federações — `ModuloFederacoes` (`-fe.dat`)

**`CConversorFederacoes::DoDeconverte` (9207)**: cabeçalho, depois — apenas se a lista OPTIONAL estiver presente — um
`std::transform` (9206) de cada `Federacao` (`Deconverte` inlinado → 9216) em `std::vector<CFederacao>`, movido para
a lista do resultado, e `CFederacoes(cabecalho, lista)` (func 9045). **Todos os 12 arquivos `-fe.dat` dos cenários
do simulador têm 25 bytes: contêm apenas o cabeçalho** (verificado com dump BER), então a lista está sempre vazia
no simulador.

### 5.5 Descrição de mídia — `ModuloInformacaoMidia` (`infomidia-*.dat`)

* **`CConversorAutenticacao::DoDeconverte` (9200)**: lê `tamanhoSenha` (campo 2), `numeroTentativas` (campo 3),
  copia `hashSenha` e, **somente se ambas** `dataInicial` e `dataFinal` estiverem presentes, interpreta-as (func 1877) e usa
  o construtor com janela de validade (9040); caso contrário, o construtor sem datas (9037). O objeto armazena
  `numeroTentativas` em +32 e `tamanhoSenha` em +36 (as duas direções de conversão concordam, §9).
* Construtor de `CDadosGeracaoMidia` (9034): serial da mídia, usuário, `IdentificadorGeradorMidia` (3 strings; no
  simulador, `"simulador-votacao-ng"` e um serial de TPM todo zerado, veja `docs/00-provenance.md`) e a data.
* `std::transform` 9176 / caminho lento 9174: a lista `aplicativos` de uma mídia de resultado (`CConversorInformacaoMidia` de u14).

### 5.6 Fotos de candidatos (lado do uenux2)

**`comum::asn::CConversorFotoCandidato::DoDesconverte` (11440)**: copia `codigoCandidato`, converte a `Foto`
por meio do `CConversorFoto` do ecourna (5708 → 6032 → 9226) e retorna `md::CFotoCandidato{codigo, foto}`. Observada em
tempo de execução em ambos os votos, chamada a partir de `(anonymous)::LeEntidadeEm` (3755), que decodifica um `FotoCandidato` no
offset indexado por `CVisitanteFoto` (u03 §5.6). `DoConverte` não é sobrescrito (slot 2 = base 11439, que lança
*"Método DoConverte() não implementado para {}"*): fotos nunca são gravadas.

---

## 6. Comportamento em tempo de execução (simulador web)

As 16 funções vistas nos perfis rodam quando os dados estáticos são carregados durante `votaInit` (o grande carregador,
func 7787, `vota::CInformacaoEleitor::Inicializar`, antes mostrado pelas ferramentas como `CHKDFSeed::GetSeed`, chama 9137,
9171 e 9207), quando o estado geral da urna é decodificado ou
codificado (`comum::asn::CConversorEstadoGeral::DesconverteEstadoUrna` 11398 / `ConverteEstadoUrna` 11397 →
`CConversorDadoCorrespondencia`), ou quando uma foto de candidato é decodificada:

| arquivo lido | cadeia observada |
|---|---|
| `-pu.dat` (nacional + UF) | 7787 → **9171** → 2665 → **1167** → 9218; 9171 → **9029**; 9171 → 9156 → 5102 → **9153**; 9156 → **9154** |
| `-cfm.dat` | 7787 → **9137** → 2665; **9136** → 1167 → **9142**; 9134; 9028 |
| `-fe.dat` | 7787 → **9207** → 2665 (a lista está ausente) |
| estado geral, `DadoCorrespondencia` (`CConversorDadoCorrespondencia`, u21) | 11397 → 5691 → 1563 → 11399 → **3734** → **6031**; 11398 → 5692 → 1008 → 11400 → **3733** → **6032** → 9224 |
| foto `-fo.dat` | 3755 → **11440** → **5708** → 6032 → 9226 |

Nenhum erro `1900`/`1902` é lançado nas sessões gravadas. Os conversores de comparecimento (§5.1) não rodaram: pertencem
ao fluxo de fim do dia.

### 6.1 Experimento: um valor fora da faixa em um arquivo de dados

Uma cópia do cenário `municipal-t1` foi feita em um diretório temporário com um byte de `t02410ac-cfm.dat` alterado: o primeiro
`codigoMunicipio` `02 01 01` → `02 01 00` (0 está fora de `INTEGER (1..99999)`). Executar
`node tools/run/headless.mjs --scenario <relative path to the copy> --events` produz:

```
[vota_web_wasm] static T api::CFileASN::DecodeObjectFunction(const std::vector<char> &, const std::string &)
  [T = ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios]:143:5954 - Conteúdo inválido para ReadFromFile de
  /dsk/fi/estatico/t02410ac-cfm.dat: .configuracoes[0].codigoMunicipio Valor 0 para campo INTEGER é menor que seu limite inferior 1
event vota:error {...same message...}
votaInit(...) -> 0
```

A camada de arquivos (`api::CFileASN::DecodeObjectFunction`, `cfileasn.h:143`, `EUeIoError` 5954, unidade u18) roda o mesmo
`isValid`/`isStrictlyValid` + `trace_invalid` logo após a decodificação BER, então o arquivo inválido é recusado **antes** que o
conversor seja chamado. Para arquivos lidos por meio de `CFileASN`, a verificação de `Deconverte` deste template (1902) é uma segunda
validação, redundante, e conversores aninhados validam as mesmas subárvores de novo em cada nível (o cabeçalho de
`-cfm.dat` é validado pela camada de arquivos, por `IConversorASN<EntidadeConfiguracaoMunicipios>::Deconverte` inlinado em
7787 (srcloc :66) e pelo thunk 2665 chamado a partir de 9137; cada `ConfiguracaoMunicipio` mais duas vezes, por 9136 e pela
verificação inlinada de `HorariosUrna` em 9142). Simetricamente, `CFileASN::CodeObjectFunction` (`cfileasn.h:161/171`, 5955/5956) reverifica o que `Converte`
(1900) já verificou antes da codificação. `votaInit` falha de forma limpa (retorna 0, emite `vota:error`).

---

## 7. BOLETIM DE URNA (BU) e os arquivos de fim do dia

Esta unidade não gera o BU, mas o seu template protege duas coisas que são gravadas no fim do dia
(*encerramento*).

### 7.1 Campo `detalhamentoComparecimento` do BU

Em `EntidadeBoletimUrna` (`ModuloBoletimUrna`), o 8º campo (índice 7) é
`detalhamentoComparecimento [1] DetalhamentoComparecimento OPTIONAL` com
`{ qtdEleitoresCompareceramSemBiometria, qtdEleitoresHabilitadosPorBiometria, qtdEleitoresHabilitadosPorBiografia }`,
cada um `INTEGER (0..9999)`. Passo a passo, dentro de `comum::asn::CConversorEntidadeBU::DoConverte` (func 10273,
`cconversorentidadebu.cpp:232`, unidade u23):

1. o modelo do BU (`comum::md::CEntidadeBU`) retorna o seu `ecourna::app::dados::CDetalhamentoComparecimento` por meio de
   `GetDetalhamentoComparecimento()` (`centidadebu.cpp:206`; o registro de srcloc implica que ele lança quando o BU não tem um);
2. um `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento` é construído na pilha (vtable 1123424) e o seu
   **`IConversorASN<DetalhamentoComparecimento, CDetalhamentoComparecimento>::Converte` é inlinado** (srcloc
   `iconversorasn.hpp:49`): `DoConverte` (func 9222) cria os três campos `Constrained_INTEGER<0..9999>`;
3. o resultado é validado (`isValid` + `isStrictlyValid`); uma contagem acima de 9999 faz a geração do BU falhar com
   `CBaseError<EApiAsnError>` **1900** e a mensagem
   `"N17ModuloBoletimUrna26DetalhamentoComparecimentoE: <tracer>"` (a constante de string @545375 é referenciada por 10273);
4. caso contrário, a SEQUENCE se torna o campo opcional da entidade do BU, que o código do BU de outras unidades (u23, u08/u09)
   então codifica em BER, calcula o hash/assina, armazena e imprime.

Assim, o detalhamento do comparecimento impresso no BU e armazenado nele tem a faixa verificada por este template no momento da codificação, como
todo outro campo do BU que passa por um `IConversorASN` (o gêmeo do uenux2 protege o resto).

### 7.2 O arquivo de comparecimento ("RC") gravado com o BU

`comum::CGravadorRCSecao::GravaResultado` (inlinado na func 11616, `cgravadorrcsecao.cpp:110`, unidade u23) grava o
comparecimento da seção (quem votou, justificou, mesários) para o sistema do cadastro eleitoral. As partes nesta unidade:

1. `CDadosComparecimento` → `DadosComparecimento` com `IConversorASN<DadosComparecimento>::Converte` (thunk 5365 →
   corpo sem landing pads 6031 → `CConversorDadosComparecimento::DoConverte` 9102, §5.1), depois validado (1900 em caso de erro);
2. o BER de `DadosComparecimento` é armazenado em claro (`infoDadosComparecimento [0]`) ou cifrado com o
   arquivo de chave pública **`jufa.pk1`** (`LeChavePublica`, erros 8655 "O arquivo … " em cgravadorrcsecao.cpp:65 e 8656
   "O arquivo … está vazio" em :83) em `DadosComparecimentoCifrado { dadosCifracao { chave, salt (≥16),
   informacaoAdicional (≥16) }, conteudo }` (`[1]`). Antes disso, o `DadosComparecimento` é codificado em BER por um
   `api::CFileASN::CodeObjectFunction<DadosComparecimento>` inlinado (cfileasn.h:161/171 em 11616). Segundo u14, a escolha
   segue o parâmetro *criptografarJUFA* do `-pu.dat`, que é **TRUE em todos os cenários do simulador**. **No build web
   o CEPESC não cifra** (u01 §4.1), então uma variante "cifrada" carregaria o texto em claro e uma chave toda zerada; mas
   como nenhum cenário traz `jufa.pk1`, `LeChavePublica` lançaria 8655 primeiro, e a página web nunca alcança o
   fluxo de fim do dia de qualquer forma (u14 §10);
3. o envelope `EntidadeResultadoUrnaCadastro { cabecalho, fase, versaoVotacao, situacao, infoDadosComparecimento }`
   é convertido com o `IConversorASN<EntidadeResultadoUrnaCadastro>::Converte` do ecourna inlinado em
   `api::CFileASN::CodeObjectFunction` (func 5366, srcloc :49, código 1900) e codificado em BER
   (`cfileasn.h:161/171`: *"Objeto com conteúdo inválido para {}: {}"*). A string `"10.23.0.1 - DESENVOLVIMENTO"`
   referenciada por 11616 é presumivelmente `versaoVotacao`.

Os conversores que LEEM essas estruturas de volta (9057, 9061, 9095, 9120, …) estão compilados no binário, mas não são usados pelo VOTA.

---

## 8. Especificidades do build web

* Nada nesta unidade é mock: o template, os conversores e os arquivos de dados são os reais.
* A segurança que é simulada em outros lugares transparece aqui: o caminho de `DadosComparecimentoCifrado` carregaria
  texto em claro (stub do CEPESC, u01). Nenhum pacote de cenário traz um arquivo `jufa.pk1` (buscado em `upstream/fs/`), e todo
  cenário define `criptografarJUFA = TRUE`, então gravar o RC falharia em `LeChavePublica` (8655) antes que qualquer coisa fosse
  cifrada; a página nem sequer roda o fluxo de fim do dia (u14 §10).
* Modelo de exceção (§4.2): a maior parte do código do uenux2 deste build, incluindo `CConversorFotoCandidato` e as cópias sem EH
  6031/6032 deste template, não tem landing pads. Um erro de validação ASN.1 lançado ali se propaga como exceção
  JS sem rodar destrutores (apenas vazamento de memória; o primeiro `invoke_*` acima na pilha ainda o captura).
* Os cenários do simulador não têm federações partidárias (§5.4) e têm horários idênticos para todos os municípios (§5.2).

---

## 9. Notas para outras unidades (verificações cruzadas)

* **Layout de `CAutenticacao` em u14.** `src/ecourna/app/dados/midias/cinformacaomidia.h` atualmente coloca `m_tamanhoSenha` em
  +32 e `m_numeroTentativas` em +36. As duas direções do conversor dizem o contrário: `DoConverte` (9201) grava o
  campo ASN.1 3 = `numeroTentativas` (tabela de nomes @1627872: dataInicial, dataFinal, tamanhoSenha, numeroTentativas,
  hashSenha) a partir de +32 e o campo 2 = `tamanhoSenha` a partir de +36; `DoDeconverte` (9200) passa o campo 3 primeiro, que os
  construtores 9037/9040 armazenam em +32. Além disso, `sizeof(CAutenticacao)` é 56 (o vetor termina em +52, alinhamento de 8 bytes).
* **`CDadosComparecimento` em u14.** O RTTI nomeia uma classe separada `ecourna::app::dados::CComparecimentoSecao`
  (`IConversorASN<ComparecimentoSecao, CComparecimentoSecao>`), e 9102 passa `&dado + 12` ao seu `Converte`: os
  bytes +12..+40 são um membro `CComparecimentoSecao` (`CIdentificacaoSecaoEleitoral` 16 bytes + `vector
  <CEstadoComparecimento>`), não dois membros achatados. O seu destrutor implícito é a func 5097.
* **Construtores de `CDadosGeracaoMidia` / `CAutenticacao` em u14**: as funcs 9034, 9037, 9040 (desta unidade) devem ser acrescentadas às
  declarações em `cinformacaomidia.h`; `CConfiguracaoMunicipios` é construído a partir de um `TMapConfiguracaoMunicipio`
  produzido pela func 9134 (§5.2).
* **u02** chamou a func 9029 de construtor de `std::pair<const K, CParametrosUrna>`. O seu único chamador constrói o seu próprio
  resultado com ela, e o tipo do resultado é a classe nomeada pelo RTTI `CParametrizacaoUrna`, então ela é o construtor
  dessa classe.
* **Nomes a corrigir no banco de dados**: `comum_f1067` = `ASN1::CHOICE::isStrictlyValid() const`; `ecourna_f858` =
  `std::basic_stringstream<char>::basic_stringstream()`; `ecourna_f1074` = o construtor de `CBaseError<EApiAsnError>`;
  as dez funções da §4.3; os nomes `vfN` de 9061, 9102, 9137, 9200, 11440.

---

## 10. Código estranho ou arriscado

1. **Dois modelos de exceção para um template (6031/6032 vs 682/1167), e nenhum landing pad no código do uenux2 (11440).**
   As cópias usadas para `IdentificadorGeradorMidia`, `DadosComparecimento` (`Converte`) e `Foto` (`Deconverte`)
   não destroem nada quando um erro de validação ASN.1 faz unwind através delas (a string, o stringstream e, para `Converte`, a
   entidade já construída vazam). A escolha de COMDAT do linker também entrega essas cópias aos chamadores do ecourna. Isso é uma
   propriedade das flags de build do Emscripten; se o build nativo da urna tem a mesma divisão é desconhecido. Impacto: um
   vazamento em um caminho de erro que aborta o carregamento de qualquer forma (e a §6.1 mostra que a camada de arquivos normalmente rejeita dados inválidos antes).
   Baixo.
2. **A janela de validade da senha de uma mídia é tudo ou nada (9200, e 9201).** Uma `Autenticacao` com apenas
   `dataInicial` ou apenas `dataFinal` é ASN.1 válido, mas o conversor descarta a data isolada e constrói um objeto
   sem nenhuma janela. Isso é um invariante da classe, e não um deslize: o próprio `GetDataHoraInicial` (9036) testa
   **ambas** as flags de preenchimento (+8 e +24) antes de retornar, e lança 3041 caso contrário. O que a perda significa para a verificação
   da senha não pôde ser estabelecido: neste binário, o único chamador de `GetDataHoraInicial/Final` (9036/9035) é o
   recodificador `CConversorAutenticacao::DoConverte` (9201), então o VOTA nunca avalia a janela (uma data isolada só
   desapareceria se a descrição da mídia fosse regravada). Info.
3. **Municípios duplicados em `-cfm.dat` são ignorados silenciosamente (9137 + 9134).** A inserção no mapa mantém o primeiro
   horário de um município e descarta os seguintes sem log nem erro. Os dados são produzidos por ferramentas do TSE, então só um
   arquivo malformado é afetado. Baixo.
4. **A validação de ENUMERATED é apenas um limite superior (6149/6150).** `value > maxEnumValue` (com sinal) é toda a
   verificação, então `Fase 0` ou valores negativos passam por `IConversorASN`. O inlining é exato: o próprio runtime da III só
   verifica o limite superior (`ValidChecker::do_visit(const ENUMERATED&)`, func 8984, e `InvalidTracer` 8937), então a camada
   de arquivos tem a mesma lacuna para todo ENUMERATED. `CConversorFase::DoDeconverte` (9193) os rejeita de novo (*"Fase
   inválida."*, 2250/2251) e `CConversorTurno::DoDeconverte` (9191) aceita apenas 1..2 (2253, que também rejeita o
   `semTurno (0)` do próprio esquema), então não há efeito hoje. Info.
5. **Mensagens de erro carregam nomes de tipo mangled** (`N27ModuloResultadoUrnaCadastro19DadosComparecimentoE: …`), que é
   o que um usuário ou o log veria se uma verificação de `IConversorASN` falhasse, ex.: uma contagem de comparecimento do BU acima de 9999 (§7.1). Um
   arquivo de dados rejeitado não a mostra: a mensagem de `CFileASN` (caminho do arquivo, prefixo do tracer vazio) vem antes (§6.1,
   reproduzido). Cosmético. Info.
6. **Cópias profundas em toda conversão.** `DoDeconverte` 9095 copia árvores inteiras de `SEQUENCE OF` com `auto`, e 9102 /
   9095 / 9057 / 9061 copiam cada vetor de novo para chamar construtores por valor. Inofensivo para uma seção (algumas centenas de
   eleitores). Info.
7. **Comparecimento "cifrado" em texto claro no build web** (§7.2): a unidade só transporta `DadosCifracao`, mas
   neste build o seu conteúdo viria do stub do CEPESC (chave zero, texto em claro). Na prática, inalcançável: nenhum cenário
   traz `jufa.pk1` (com `criptografarJUFA = TRUE` o gravador lança 8655 primeiro) e a página nunca roda o
   fluxo de fim do dia. Segurança simulada, apenas no simulador. Info.
8. **Validação redundante** (§6.1): todo arquivo é validado por `CFileASN` e de novo por cada `Deconverte` aninhado; o
   caminho 1902 é praticamente inalcançável para arquivos e os percursos repetidos custam tempo de inicialização (os perfis mostram a
   conversão de `-pu.dat` como a função mais pesada desta unidade). Info.

---

## 11. Questões em aberto

* A forma exata no código-fonte da func 9134 (vetor → `std::map`: uma função out-of-line que constrói o mapa que retorna
  e o preenche com semântica de `std::insert_iterator`) e do helper de lista 1970 (`ConverteLista` aqui) não é
  recuperável; a reconstrução usa código equivalente marcado com `?` (`ConverteParaMapa`, nome inferido).
* 5095 é out-of-line e compartilhado por dois conversores, o que sugere um helper inline em um header (chamado
  `ConverteOctetString` aqui, nome e local inferidos) ou um `std::ranges::to` do C++23.
* O código 1901 de `EApiAsnError` nunca é lançado neste binário; o seu significado é desconhecido.
* Se o simulador web consegue alcançar o fluxo de fim do dia que grava o RC (§7.2) não foi testado aqui.

---

## 12. Tabela de mapeamento completa (todas as 90 funções)

"run" = vista em execução durante os votos gravados. `:49` / `:66` = linha de `iconversorasn.hpp` cujo srcloc nomeia a
função. Os arquivos reconstruídos ficam sob `src/` com o mesmo caminho (`ecourna-lib/ecourna/…` → `src/ecourna/…`).

| func | tamanho | run | símbolo reconstruído | tipo | arquivo original |
|---|---|---|---|---|---|
| 306 | 16 |  | `std::__exception_guard_exceptions<std::vector<T>::__destroy_vector>::~__exception_guard_exceptions` | ICF: todo T trivialmente destrutível (42 chamadores: vetores de bytes do ecourna, boost regex, blobs do sqlite, …) | biblioteca (libc++) |
| 682 | 654 |  | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Converte (merged body)` | 22 thunks | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 858 | 304 |  | `std::basic_stringstream<char>::basic_stringstream` | construtor padrão (modo in\|out = 24) | biblioteca (libc++) |
| 1006 | 257 |  | `ecourna::app::dados::CEstadoComparecimento::~CEstadoComparecimento` | destrutor implícito | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h (implícito; u11-foreign-fragments.cpp) |
| 1074 | 18 |  | `ecourna::api::exception::CBaseError<ecourna::api::asn::EApiAsnError, ecourna::api::exception::SErrorLimits{1900, 1910}>::CBaseError` | thunk -> 710 | template de classe CBaseError (header não atestado); instanciado por ecourna-lib/ecourna/api/asn/iconversorasn.hpp |
| 1167 | 516 | sim | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Deconverte (merged body)` | 11 thunks | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 1876 | 114 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::~CRegistroIdentificacaoEleitor` | destrutor implícito | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.h (implícito; u11-foreign-fragments.cpp) |
| 2660 | 15 |  | `std::vector<ecourna::app::dados::CComparecimentoMesario>::__destroy_vector::operator()` | -> 6145(40,36,28,24) | biblioteca (libc++) |
| 2661 | 15 |  | `std::vector<ecourna::app::dados::CComparecimentoMesario>::~vector` | -> 6146(40,...) | biblioteca (libc++) |
| 2665 | 20 | sim | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, ecourna::app::dados::CCabecalhoEntidade>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 3488 | 15 |  | `std::vector<ecourna::app::dados::CIdentificacaoJustificativa>::~vector` | -> 6146(24,...) | biblioteca (libc++) |
| 3733 | 20 | sim | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, ecourna::app::dados::CIdentificadorGeradorMidia>::Deconverte` | thunk -> 6032 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 3734 | 20 | sim | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, ecourna::app::dados::CIdentificadorGeradorMidia>::Converte` | thunk -> 6031 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 5095 | 221 |  | `ecourna::app::dados::asn::ConverteOctetString` | helper inline | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h (nome e caminho inferidos) |
| 5096 | 87 |  | `std::__exception_guard_exceptions<std::vector<ecourna::app::dados::CEstadoComparecimento>::__destroy_vector>::~__exception_guard_exceptions` |  | biblioteca (libc++) |
| 5097 | 75 |  | `ecourna::app::dados::CComparecimentoSecao::~CComparecimentoSecao` | destrutor implícito | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.h (implícito; u11-foreign-fragments.cpp) |
| 5099 | 15 |  | `std::vector<ecourna::app::dados::CIdentificacaoJustificativa>::__destroy_vector::operator()` | -> 6145(24,20,12,8) | biblioteca (libc++) |
| 5100 | 75 |  | `std::vector<ecourna::app::dados::CEstadoComparecimento>::~vector` |  | biblioteca (libc++) |
| 5103 | 18 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Fase, ecourna::app::dados::CFaseID>::Deconverte` | thunk -> 6149 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 5365 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, ecourna::app::dados::CDadosComparecimento>::Converte` | thunk -> 6031 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 5708 | 20 | sim | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Foto, ecourna::app::dados::CFoto>::Deconverte` | thunk -> 6032 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6031 | 151 | sim | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Converte (merged body, no landing pads)` | thunks 3734, 5365 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 6032 | 151 | sim | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Deconverte (merged body, no landing pads)` | thunks 3733, 5708 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6145 | 192 |  | `std::vector<T>::__destroy_vector::operator() (merged body, T starts with CRegistroIdentificacaoEleitor)` | thunks 2660, 5099 | biblioteca (libc++) |
| 6146 | 188 |  | `std::vector<T>::~vector (merged body, T starts with CRegistroIdentificacaoEleitor)` | thunks 2661, 3488 | biblioteca (libc++) |
| 6149 | 513 |  | `ecourna::api::asn::IConversorASN<ENUMERATED, DADO>::Deconverte (merged body)` | thunks 5103, 9178 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6150 | 515 |  | `ecourna::api::asn::IConversorASN<ENUMERATED, DADO>::Converte (merged body)` | thunks 9187, 9188 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9029 | 34 | sim | `ecourna::app::dados::CParametrizacaoUrna::CParametrizacaoUrna` | construtor | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.cpp (caminho inferido) |
| 9034 | 474 |  | `ecourna::app::dados::CDadosGeracaoMidia::CDadosGeracaoMidia` | construtor | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.cpp (caminho inferido) |
| 9037 | 275 |  | `ecourna::app::dados::CAutenticacao::CAutenticacao` | construtor (sem datas) | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp (u14; fragmento .u11.cpp) |
| 9040 | 291 |  | `ecourna::app::dados::CAutenticacao::CAutenticacao` | construtor (com datas) | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp (u14; fragmento .u11.cpp) |
| 9045 | 569 |  | `ecourna::app::dados::CFederacoes::CFederacoes` | construtor | ecourna-lib/ecourna/app/dados/federacoes/cfederacoes.cpp (caminho inferido) |
| 9048 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, ecourna::app::dados::CDadosComparecimentoCifrado>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9050 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, ecourna::app::dados::CDadosComparecimento>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9053 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, ecourna::app::dados::CDadosComparecimentoCifrado>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9057 | 1263 |  | `ecourna::app::dados::asn::CConversorDadosComparecimentoCifrado::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp (caminho inferido) |
| 9059 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosCifracao, ecourna::app::dados::CDadosCifracao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9061 | 1204 |  | `ecourna::app::dados::asn::CConversorDadosCifracao::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp (caminho inferido) |
| 9064 | 343 |  | `ecourna::app::dados::asn::CConversorIdentificacaoJustificativa::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp (caminho inferido) |
| 9067 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo, ecourna::app::dados::CEstadoHabilitacaoPorCodigo>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9076 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, ecourna::app::dados::CApresentacaoFotoEleitor>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9078 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, ecourna::app::dados::CHabilitacaoBiometrica>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9082 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, ecourna::app::dados::CHabilitacaoBiometrica>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9084 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, ecourna::app::dados::CApresentacaoFotoEleitor>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9092 | 888 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::ComparecimentoMesario>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CComparecimentoMesario>>, CConversorDadosComparecimento::DoDeconverte::$lambda>` | instância de lambda | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (caminho inferido) |
| 9093 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, ecourna::app::dados::CComparecimentoSecao>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9094 | 878 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CIdentificacaoJustificativa>>, CConversorDadosComparecimento::DoDeconverte::$lambda>` | instância de lambda | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (caminho inferido) |
| 9095 | 3926 |  | `ecourna::app::dados::asn::CConversorDadosComparecimento::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (caminho inferido) |
| 9096 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoMesario, ecourna::app::dados::CComparecimentoMesario>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9097 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa, ecourna::app::dados::CIdentificacaoJustificativa>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9099 | 26 |  | `ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa>::SEQUENCE_OF` | (first, last) -> 2926 | biblioteca (template ASN.1 da III) |
| 9101 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, ecourna::app::dados::CComparecimentoSecao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9102 | 3374 |  | `ecourna::app::dados::asn::CConversorDadosComparecimento::DoConverte` | slot 2 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (caminho inferido) |
| 9105 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, ecourna::app::dados::CRegistroIdentificacaoEleitor>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9109 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, ecourna::app::dados::CRegistroIdentificacaoEleitor>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9111 | 521 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, ecourna::app::dados::CIdentificadorEleitor>::Deconverte` | cópia completa (CHOICE::isValid/isStrictlyValid) | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9113 | 659 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, ecourna::app::dados::CIdentificadorEleitor>::Converte` | cópia completa (CHOICE) | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9118 | 1106 |  | `std::vector<ecourna::app::dados::CEstadoComparecimento>::push_back` | push_back(T&&) caminho rápido + caminho lento | biblioteca (libc++) |
| 9119 | 881 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::EstadoComparecimento>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CEstadoComparecimento>>, CConversorComparecimentoSecao::DoDeconverte::$lambda>` | instância de lambda | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (caminho inferido) |
| 9120 | 818 |  | `ecourna::app::dados::asn::CConversorComparecimentoSecao::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (caminho inferido) |
| 9121 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::EstadoComparecimento, ecourna::app::dados::CEstadoComparecimento>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9124 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, ecourna::app::dados::CIdentificacaoSecaoEleitoral>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9127 | 631 |  | `ecourna::app::dados::asn::CConversorIdentificacaoSecaoEleitoral::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp (caminho inferido) |
| 9128 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::MunicipioZona, ecourna::app::dados::CMunicipioZona>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9136 | 21 | sim | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, ecourna::app::dados::CConfiguracaoMunicipio>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9137 | 833 | sim | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipios::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp (caminho inferido) |
| 9138 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, ecourna::app::dados::CConfiguracaoMunicipio>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9142 | 643 | sim | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipio::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp (caminho inferido) |
| 9143 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::HorariosUrna, ecourna::app::dados::CHorariosUrna>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9153 | 21 | sim | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, ecourna::app::dados::CTituloRelatorio>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9154 | 21 | sim | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, ecourna::app::dados::CLabelParametrizado>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9157 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, ecourna::app::dados::CTituloRelatorio>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9159 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, ecourna::app::dados::CLabelParametrizado>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9171 | 677 | sim | `ecourna::app::dados::asn::CConversorParametrizacaoUrna::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp (caminho inferido) |
| 9172 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::ParametrosUrna, ecourna::app::dados::CParametrosUrna>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9174 | 620 |  | `std::vector<ecourna::app::dados::CAplicativo>::__push_back_slow_path` | caminho lento de T&& | biblioteca (libc++) |
| 9176 | 903 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloInformacaoMidia::Aplicativo>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CAplicativo>>, CConversorInformacaoMidia::DoDeconverte::$lambda>` | instância de lambda | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (u14; fragmento .u11.cpp) |
| 9177 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, ecourna::app::dados::CDadosGeracaoMidia>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9178 | 18 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Turno, CBaseType<unsigned short, 1, 2, 11>>::Deconverte` | thunk -> 6149 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9180 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::Aplicativo, ecourna::app::dados::CAplicativo>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9186 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, ecourna::app::dados::CDadosGeracaoMidia>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9187 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Turno, CBaseType<unsigned short, 1, 2, 11>>::Converte` | thunk -> 6150 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9188 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Fase, ecourna::app::dados::CFaseID>::Converte` | thunk -> 6150 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9197 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::Autenticacao, ecourna::app::dados::CAutenticacao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9200 | 657 |  | `ecourna::app::dados::asn::CConversorAutenticacao::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp (caminho inferido) |
| 9206 | 961 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloFederacoes::Federacao>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CFederacao>>, CConversorFederacoes::DoDeconverte::$lambda>` | instância de lambda | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (caminho inferido) |
| 9207 | 410 | sim | `ecourna::app::dados::asn::CConversorFederacoes::DoDeconverte` | slot 3 da vtable | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (caminho inferido) |
| 9208 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloFederacoes::Federacao, ecourna::app::dados::CFederacao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9211 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, ecourna::app::dados::CCabecalhoEntidade>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 11440 | 346 | sim | `comum::asn::CConversorFotoCandidato::DoDesconverte` | slot 3 da vtable | uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp (caminho inferido) |

---

## 13. Arquivos reconstruídos

| arquivo | conteúdo |
|---|---|
| `src/ecourna/api/asn/iconversorasn.hpp` | `IConversorASN<ENTIDADE, DADO>` (Converte linha 49, Deconverte linha 66), `EApiAsnError`, `CApiAsnError`, notas sobre os corpos mesclados |
| `src/ecourna/api/asn/iconversorasn.instances.cpp` | mapa de instanciações explícitas: cada índice de thunk, o seu corpo, registro de srcloc e chamadores |
| `src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.{h,cpp}` | 9095, 9102 (+ notas sobre 9092, 9094, 9099, a família 6145/6146, 5097) |
| `…/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp` | 9057 |
| `…/resultadournacadastro/cconversordadoscifracao.{h,cpp}` | 9061, helper 5095 |
| `…/resultadournacadastro/cconversoridentificacaojustificativa.{h,cpp}` | 9064 |
| `…/resultadournacadastro/cconversorcomparecimentosecao.{h,cpp}` | 9120 (+ notas sobre 9119, 9118, 5100, 5096, 1006) |
| `src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.{h,cpp}` | 9127 |
| `src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.{h,cpp}` | 9142 |
| `src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp` | 9137 |
| `src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp` | 9171 |
| `src/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp` | 9200 |
| `src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.u11.cpp` | fragmento para u14: 9176, 9174 |
| `src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp` | 9207 (+ 9206) |
| `src/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.{h,cpp}` | 9029 |
| `src/ecourna/app/dados/federacoes/cfederacoes.{h,cpp}` | 9045 |
| `src/ecourna/app/dados/midias/cdadosgeracaomidia.cpp` | 9034 (classe declarada por u14 em `cinformacaomidia.h`) |
| `src/ecourna/app/dados/midias/cautenticacao.u11.cpp` | fragmento para u14: 9037, 9040 |
| `src/ecourna/app/dados/u11-foreign-fragments.cpp` | destrutores implícitos 1876, 1006, 5097 |
| `src/uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp` | 11440 |

# u23 — `comum/gravadores` (gravadores dos arquivos de resultado), `IInterfaceInit`, `IInterfaceSavd`, `CInformacaoEleicao`

Unidade u23 da engenharia reversa de `vota_web_wasm.wasm` (aplicação de votação VOTA do TSE, `uenux2` + `ecourna`,
build Emscripten do simulador público de treinamento). 107 funções, 10 delas vistas executando durante as votações
gravadas. Fontes reconstruídas: `src/uenux2/src/app/comum/gravadores/**`, `src/uenux2/src/app/comum/iinterfaceinit.*`,
`src/uenux2/src/app/comum/iinterfacesavd.*`, `src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp`.
Os índices de função são índices de função wasm (`python3 tools/wasmmap/q.py f <n>`).

Capítulos relacionados: docs/10-boletim-de-urna.md (o que é um BU),
docs/bu/codepath.md (o encerramento, executado de verdade com um harness),
[u07](u07-uenux2-src-app-vota-eleitor.md) (`vota::CGravaResultado`, que constrói e executa todos os gravadores desta
unidade), [u14](u14-ecourna-lib-ecourna-app-dados.md) (as classes de comparecimento `ecourna::app::dados` gravadas por
`CGravadorRCSecao`), [u12](u12-ecourna-lib-ecourna-api-compression-ecourna-lib-ecourna-api-.md) (empacotamento do
`CGravadorWSQ`), [u05](u05-uenux2-src-app-comum-dados.md) (o RDV a partir do qual o BU é lido).

## 0. Termos

| termo | significado |
|---|---|
| *gravador* | uma classe por arquivo de resultado (`CGravadorBU`, `CGravadorRCSecao`, …) |
| *resultado* / *arquivos de resultado* | os arquivos que a urna produz no fim do dia e copia para a *mídia de resultado* (MR, um pendrive) para transmissão ao TSE |
| *BU*, *boletim de urna* | a apuração da seção (`…-bu.dat`) |
| *RC*, *JUFA* | registro de comparecimento da seção: quem votou, as *justificativas* de ausência, os mesários presentes (`…-jufa.dat`) |
| *WSQ* | formato de imagem de impressão digital; `wsq*.jez` são pacotes ZIP das capturas de impressões digitais do dia |
| MI / MV / MR | *memória interna* (`/dsk/fi`), *memória de votação* (flash externa, `/dsk/fe`), *mídia de resultado* (`/dsk/mr`) |
| diretórios *trab* / *resultado* | diretório de trabalho por flash (`dinamico/trab1/`) e diretório de resultado (`dinamico/res…`) |
| SAVD | o daemon de assinatura/validação da urna (chaves, HSM). A aplicação conversa com ele por um pequeno protocolo binário |
| *init* | o serviço de init/hardware da urna (desligamento, dispositivo de boot, porta da MR, flag de demonstração) |
| *modo demonstração* | modo de demonstração da urna (quantidades fixas de relatórios, sem registro de mesários) |
| *fase* | `o` oficial, `s` simulado, `t` treinamento (prefixo de arquivo); ASN.1/`EUrnaFase` `'1'`/`'2'`/`'3'` |
| *pleito* / *eleição* | o evento eleitoral (ex.: 2410) / uma eleição dentro dele (ex.: 2411 municipal) |
| *abrangência* | escopo de uma eleição (municipal / estadual / federal); as contagens de eleitores aptos são por abrangência |
| *hash encadeado* | a cadeia SHA-512 do BU sobre suas linhas de votos |
| CEPESC | a biblioteca de criptografia do TSE (`ecourna::api::cepesc`), usada para cifrar o conteúdo do BU/JUFA quando os parâmetros pedem |

## 1. O que o subsistema faz

No *encerramento* (fim da votação), `vota::CGravaResultado::StartState` (func 12098, unidade u07) constrói um
`comum::IGravador` por arquivo de resultado, e um `comum::CGravacaoResultados` os executa. Cada gravador:

1. calcula o nome do seu arquivo a partir da seção (`CGravadorUtil::DeterminaNomeArquivo`, func 3798):
   `"{fase:c}{pleito:05}{uf}{municipio:05}{zona:04}{secao:04}-" + suffix`, ex.: `t02410ac0000100010001-bu.dat`
   (a UF é o `CLocal::GetUF()` em minúsculas; a tabela de sufixos é `CArquivosResultado`);
2. grava o arquivo no diretório de trabalho da MI (`IGravador::Grava`, slot 4 → seu próprio `GravaResultado(CFile&)`,
   slot 7) e o copia para o diretório de resultado da MI (slot 2), para o diretório de trabalho da MV (slot 3) e para o
   diretório de resultado da MV (slot 6);
3. depois é assinado via SAVD (`CAssinador::AssinaArquivosResultado`, u07) em `…-vota.vsc`.

A unidade contém quatro desses gravadores — **BU**, **RC da seção (jufa)**, **hashes**, **WSQ** — com seus conversores
ASN.1 e modelos de dados C++ (`md::`), além de três clientes de serviço usados em toda a aplicação:

* `IInterfaceInit`: requisições ao serviço init (montagem e serial da MR, desligamento, flag de demonstração);
* `IInterfaceSavd` + `CAssinador::Assina`: toda assinatura de um arquivo dinâmico (`vota.bin`, `rdv.dat`, relatórios,
  `uenux.db` …) passa por `CAssinador::Assina` (func 1277) → SAVD. Este caminho executa muitas vezes por voto;
* `CInformacaoEleicao`: getters de parâmetros que respondem valores fixos no modo demonstração.

## 2. Classes (RTTI) e layouts

```
comum::IResultado (vtable @1541028)                      iresultado.cpp
  └ comum::IGravador (@1553656)  slots: 2 CopiaParaResultado 3 CopiaParaMV 4 Grava 5 GravaMV
      │                                 6 CopiaResultadoParaMV 7 GravaResultado(api::CFile&) [pure]
      ├ comum::CGravadorBU        (@1554140, 304 B)  bu.dat     SAVD file 35   cgravadorbu.cpp
      ├ comum::CGravadorRCSecao   (@1554748,  60 B)  jufa.dat   SAVD file 39   cgravadorrcsecao.cpp
      ├ comum::CGravadorHashes    (@1554600,  96 B)  hash.dat   SAVD file 46   cgravadorhashes.cpp
      ├ comum::CGravadorWSQ       (@1557300,  88 B)  wsq*.jez   SAVD 48/49/50  cgravadorwsq.cpp
      ├ comum::CGravadorRDV, CGravadorLog, CGravadorVersoesArquivos, IGravadorEnvelope─CGravadorEnvelopeArquivo
      │     (other units: u05/u07/u12/u21)
comum::CAssinador (@1553260) └ vota::CAssinadorVota (@1532312)                     cassinador.cpp
comum::IInterfaceInit (@1552160) └ simulador::CWasmInit (@1528772, web mock)        iinterfaceinit.cpp
comum::IInterfaceSavd (@1526736) └ (anonymous)::CWasmSavd (@1526688, web mock)     iinterfacesavd.cpp
comum::asn::CConversorEntidadeBU (@1597536)       IConversorASN<EntidadeBoletimUrna, md::CEntidadeBU>
comum::asn::CConversorEnvelopeGenerico            IConversorASN<EntidadeEnvelopeGenerico, md::CEnvelopeGenerico>
comum::asn::CConversorUrna (@1596460)             IConversorASN<Urna, md::CUrna>
comum::asn::CConversorCorrespResultado (@1596224) IConversorASN<CorrespondenciaResultado, md::CCorrespondenciaResultado>
comum::asn::CConversorTipoApuracaoSA (@1596864)   IConversorASN<TipoApuracaoSA, md::CTipoApuracaoSA>
comum::asn::CConversorVersoesArquivos (@1599296)  IConversorASN<EntidadeVersaoArquivos, md::CVersoesArquivos>
```

Layout de `IResultado`: `+4` município, `+8` zona (u16), `+12` local, `+16` seção (u16), `+18` char da fase,
`+20` std::string nome, `+32` `EExtensaoArquivoResultado`, `+36` `ESavdArquivoUE`. O único construtor fora de linha
é o de `IGravador` (func 1396, com o de IResultado inlinado). Ele valida a fase (`'o'`, `'s'`, `'t'`, senão
8657 "Fase inválida: {}") **depois** de calcular o nome.

`CGravadorBU` (construído inline pela func 12098; veja `src/.../gravadores/cgravadorbu.h` para todos os campos):
`+40` dhGeração, `+52` dhEmissão, `+64` fase (`'1'..'3'`), `+68` tipo de arquivo `'1'` (votacaoUE), `+72`
`CDadoCorrespondencia` (carga: nº interno da urna, serial MC, data da carga, código da carga, gerador; município
/ zona / seção), `+168` histórico de códigos de carga, `+180` map abrangência → {aptos seção, aptos TTE},
`+192` urna biométrica, `+194` comparecimento, `+196` `IRdv*` (o RDV em memória), `+200` `"bu.pk1"`, `+212`
cifra permitida (true), `+216` `CDadosBUVota` opcional {abertura, encerramento, desligamento do voto
impresso opcional} (flag +256), `+260` `CTipoApuracaoSA` opcional (flag +268), `+272` `CDadosBUSA` opcional (flag +280),
`+284` map cargo → ordem de impressão, `+296` habilitações (sem biometria, por biometria, por biografia).

Códigos de erro: os gravadores lançam `CBaseError<comum::EUeComumGravadoresError, SErrorLimits{8600, 8800}>`
(thunk de construtor func 283); os clientes de serviço lançam `CBaseError<comum::EUeComumError, {7200, 7600}>` (func
480). Códigos usados aqui: 8601–8698 e 7231–7340 (listados ao lado de cada `throw` na reconstrução).

## 3. O arquivo do BU, passo a passo (`CGravadorBU::GravaResultado`, func 11629, e `CConversorEntidadeBU::DoConverte`, func 10273)

Este é o código que transforma o RDV em memória em `…-bu.dat`. Ele executa uma vez, no encerramento, dentro de
`vota::CGravaResultado` (u07) — só quando `treinamentoEleitor` é 0, portanto **nunca na página web** (a página fica
no modo de treinamento do eleitor e nunca chega ao encerramento; veja codepath.md §1.3). O harness de
codepath.md o executou de fato e produziu um BU de 711 bytes que decodifica com o schema recuperado e passa na própria
verificação de cadeia de hashes do TSE. Tudo abaixo foi lido das duas funções; os números de linha são as linhas de
`std::source_location` dos arquivos originais.

### 3.1 Entradas (membros de `CGravadorBU`, preenchidos por `CGravaResultado`)

| entrada | origem (u07) |
|---|---|
| seção: município, zona, local, seção, fase | `CLocal`, `CEstadoGeral` |
| dhGeração / dhEmissão | "agora" / `EstadoGeralVota.dtHrEmissaoBU` (definido por `CGeraBU`) |
| `CDadoCorrespondencia` | `EstadoGeral` +60: nº interno da urna, serial MC, data/hora e código da carga, gerador da mídia |
| histórico de códigos de carga | `CEstadoGeralGap` (todas as cargas desta urna) |
| aptos por abrangência | `CEleitores::GetQtdAptos()` (`SQtdeAptos` = {seção, TTE}) |
| comparecimento | `CEleitores` +104 (verificado por assert como igual a `rdv.Comparecimento()`) |
| urna biométrica + 3 contadores de habilitação | `CLocal`, `CEleitores` (sem biometria / por biometria / por biografia) |
| `CDadosBUVota` | início / fim da aquisição de votos (`EstadoGeralVota`) |
| ordem dos cargos | primeira posição de cada cargo em `CCargos` (contando a partir de 1) |
| `IRdv*` | `CRdvVota::GetInst()`: a **única** fonte das contagens de votos (não há apuração separada) |

### 3.2 Valores de contexto lidos por `GravaResultado`

1. `tipoUrna` = `CEstadoGeral` +36 (1º turno) ou +40 (2º turno) — `'1'` seção, `'2'` contingência, `'3'`, `'4'`.
2. Serial da flash de votação (`numeroSerieFV`): se `/dsk/fe/estatico/infomidia.dat` existir
   (`CPath` estático da flash **externa**), `CFileASN::ReadFromFile<ModuloInformacaoMidia::InformacaoMidia>`
   e o hex dos seus bytes de serial (func 1243); senão `"00000000"`. Nenhum cenário traz um `infomidia.dat` simples
   (eles trazem `infomidia-fv-1-t.dat` para os cenários de 1º turno e `infomidia-fv-2-t.dat` para os de 2º turno,
   em `/dsk/fi/estatico/`), então os dados web dão `"00000000"`.
3. `cfg = CConfiguracaoEleicao::GetInst()`: pleito (+28), as eleições (+52..+56, itens de 52 bytes), parâmetros
   (+88 = `CParametrosUrna`; `criptografarBU` em cfg +160).

### 3.3 Linhas de votos (`MontaResultados`, inlinada; `MontaVotosCargos`, func 3819; `AcrescentaVotoVotavel`, func 3820)

Para **cada eleição** do pleito, na ordem da configuração:

1. `cargos = cfg.GetCargos(eleição, true)` (func 3775); separados com `copy_if` (func 3818) em majoritários
   (`CCargo::m_tipo` 0), proporcionais (1) e consultas (2).
2. Comparecimento por grupo = `rdv.Cargo(first cargo)` (slot 10 de IRdv, total do cargo) **÷** `qtdEscolhas` desse
   cargo (divisão inteira; 2 numa disputa de Senado com 2 vagas). Majoritário e consulta compartilham o número do
   majoritário (o número da consulta só quando não há majoritário); o grupo proporcional tem o seu próprio.
3. Para cada cargo de um grupo (`MontaVotosCargos`):
   * **cargo sem candidatos** (não é consulta e `CCandidaturas::PossuiCandidatos` é falso): uma única linha
     `NULO_CARGO_SEM_CANDIDATO` (8) com os votos de `rdv.Cargo(cargo)`, sem identificação;
   * caso contrário, nesta ordem:
     1. linhas nominais: para cada número de candidato do cargo (`CCandidaturas`, func 2272) — ou cada resposta de uma
        consulta (respostas de `CDetalheConsulta`) — `AcrescentaVotoVotavel(NOMINAL, n, rdv.Candidato(cargo, n,
        nDígitos))` (slot 3 de IRdv);
     2. `BRANCO` (3) com `rdv.Brancos(cargo)` (slot 9) — só se > 0;
     3. `NULO` (4) com `rdv.Nulos(cargo)` (slot 8) — só se > 0;
     4. para um cargo proporcional que tenha detalhes de candidatos: `AcrescentaVotoVotavel(LEGENDA, p,
        rdv.Legenda(cargo, p))` (slot 4) para **todo partido de `CPartidos`**, em ordem crescente de número do partido.
   * `AcrescentaVotoVotavel` descarta contagens zero (incondicionalmente: o parâmetro do PU `gravarZeradosBU`,
     `CParametrosUrna` +393, nunca é lido pelo gravador do BU, então um candidato ou partido com 0 votos nunca aparece
     em `bu.dat`); as linhas de branco/nulo/sem-candidato não têm identificação; uma linha de consulta
     recebe `{codigo, partido 0}`; caso contrário, o partido é o do candidato (`CCandidaturas::Busca`) ou, se o
     número não for de um candidato, o número precisa existir em `CPartidos` (legenda) — senão
     **8638 "Candidato {} / cargo {} não existe"** (cgravadorbu.cpp:221).
   * o registro do cargo é `{codigo, ordemImpressao = ordemCargos.at(codigo), linhas}` (`std::out_of_range
     "map::at:  key not found"` se o cargo não estiver em `CCargos`).
4. Os `CResultadoVotacao{tipo, comparecimento, cargos}` são acrescentados na ordem **proporcional, majoritário,
   consulta**, cada um só se o grupo tiver pelo menos um cargo.
5. `CResultadoVotacaoPorEleicao{idEleição, aptosSeção, aptosTTE, resultados}` com os aptos da abrangência da
   eleição (`map::at`).

### 3.4 Cabeçalho, urna e entidade (`MontaEntidadeBU`, cgravadorbu.cpp:636, inlinada)

* `CCabecalhoEntidade(dhGeração, pleito, tipoId = 1 /*pleito*/)`.
* `CCorrespondenciaResultado(município, zona, seção, CCarga(...), seção ≠ 0 ? '1' : '2')`: município/zona/seção vêm
  do `CDadoCorrespondencia` (+112/+116/+118), e a escolha entre `'1'`/`'2'` vem da própria seção do gravador
  (`IResultado` +16); o construtor de `CCarga` (func 2802) executa `CCarga::ValidaCriacao`.
* `CUrna(tipoUrna, "10.23.0.1 - DESENVOLVIMENTO", correspondência, tipoArquivo '1' /*votacaoUE*/, serialMV
  [, motivo SA])` → `CUrna::ValidaCriacao` (func 5862): tipo ≠ '0' (8689), tipo de arquivo ≠ '0' (8690), serial de
  exatamente 8 dígitos hex (8691/8692 "Serial da MV inválido [..]").
* `CEntidadeBU`: com `CDadosBUVota` (VOTA) ou `CDadosBUSA` (Sistema de Apuração); nenhum dos dois →
  **8641 "Dados específicos do aplicativo não presentes"**. `detalhamentoComparecimento` (os 3 contadores
  de habilitação) só numa urna **biométrica**. `CEntidadeBU::ValidaCriacao` (centidadebu.cpp:159–177): fase `== '0'` ou
  `>= '4'` (comparação com sinal; um valor abaixo de `'0'` passaria) → 8665, município < 100000 (8666), zona < 10000 (8667),
  local < 10000 (8668), seção < 10000 (8669).

### 3.5 Conversão ASN.1 e a cadeia de hashes (`CConversorEntidadeBU::DoConverte`, func 10273)

Campos de `EntidadeBoletimUrna`, na ordem em que são preenchidos:

| campo | valor |
|---|---|
| `cabecalho` | `CConversorCabecalhoEntidade` (dataGeração, idEleitoral = pleito) |
| `fase` | `Utils::ConverteFase` |
| `urna` | `CConversorUrna::DoConverte` (func 10287): tipoUrna `'1','2','3','4'` → 1, 3, 4, 6 (8627); versão; correspondência (func 10289: identificação de seção `{município, zona, local = constant 1, seção}` ou de contingência quando seção 0 / tipo `'2'`; `'0'` → 8601; tipo > `'4'` a deixa sem valor); tipoArquivo `'1'..'6'` → 1..6 (8629); `numeroSerieFV` = 4 bytes a partir dos 8 caracteres hex (func 3605; 8631/8632); motivo SA opcional |
| `identificacaoSecao` | município, zona, local, seção |
| `dataHoraEmissao` | `"YYYYMMDDThhmmss"` (comum_f1080) |
| `dadosSecaoSA` | CHOICE `[0] dadosSecao {abertura, encerramento, desligamentoVotoImpresso OPTIONAL}` (VOTA) ou `[1] dadosSA {junta, turma, numeroInternoUrnaOrigem OPTIONAL (omitted when 0)}`; nenhum dos dois → **8608 "Objeto de dados do BU mal formado"** (:232) |
| `qtdEleitoresCompareceram` | CEntidadeBU +174 |
| `detalhamentoComparecimento` | opcional (urnas biométricas) |
| `resultadosVotacaoPorEleicao` | abaixo |
| `historicoCodigosCarga` | todos os códigos de carga, como `GeneralString` |
| `historicoVotoImpresso` | opcional, só quando não vazio (nunca no VOTA: histórico do hardware de voto impresso) |

O cabeçalho precisa identificar um **pleito** (tipo 1), senão **8607 "Erro no hash encadeado: Tipo de identificador do
cabeçalho da entidade BU precisa ser um pleito."** (:175). Para cada `ResultadoVotacaoPorEleicao`:

1. **hash inicial** = SHA-512 de `"{pleito:05}|{eleição:05}|{município:05}|{zona:04}|{seção:04}|{códigoCarga:24}"`
   (string de formato @8483; `CalculaHash`, func 3602, sobre `ecourna::api::security::CSha512`). O valor corrente é um
   `std::vector` estático @1910004 do singleton (de 1 byte) `CAssinaVotavelBu` (func 3603);
2. `idEleicao`; `qtdEleitoresAptos = aptosSeção + aptosTTE`; `qtdEleitoresAptosSecao`; `qtdEleitoresAptosTTE`;
3. para cada `ResultadoVotacao`: `tipoCargo` = tipo C++ + 1 (majoritário 1, proporcional 2, consulta 3; ≥ 3 → 8610),
   `qtdComparecimento`, depois, para cada cargo, `TotalVotosCargo{codigoCargo (CConversorCodigoCargoConsulta),
   ordemImpressao, votosVotaveis}`; o contador por cargo `ordemGeracaoHash` reinicia em 1;
4. cada `TotalVotosVotavel`: `tipoVoto` pela tabela @546352 (RDV 1..9 → BU `legenda 4, nominal 1, branco 2,
   nulo 3, branco 2, nulo 3, nulo 3, cargoSemCandidato 5, cargoSemCandidato 5`; fora do intervalo → 8609),
   `quantidadeVotos`, `identificacaoVotavel{partido, codigo}` opcional, `ordemGeracaoHash` e
   **hash(n) = SHA-512(`"{HEX(hash(n−1))}|{ordem}|{cargo}|{tipoVoto}|{quantidade}|{codigo}|{partido}"`)**
   (@1120; sem identificação, a forma de 5 campos @1126). O HEX é em maiúsculas, o cargo é o byte do código de cargo C++
   e `tipoVoto` é o valor do **BU**. Isto é exatamente o `calcula_hash_votacao` do TSE (`bu_assinatura_tuplas.py`);
5. `ultimoHashVotosVotavel` = o último hash (64 bytes) e `assinaturaUltimoHashVotosVotavel` =
   slot 23 de `IPkcs11` (abrir sessão) → slot 6 (assinar, recebendo o último hash bruto de 64 bytes) → slot 24 (fechar)
   (`CAssinaVotavelBu::Assinar`, cassinavotavelbu.cpp:89). Nenhum `IPkcs11` é linkado no build web (a busca
   lança "PolySingleton - solicitada uma instancia nao criada N3api6pkcs117IPkcs11E").

Todo subobjeto é verificado com `isValid() && isStrictlyValid()`; as falhas lançam **8603..8606 "Entidade
inválida"** (linhas 87, 110, 132, 164), e o arcabouço do conversor lança 7653 "Entidade deixada em estado inválido:
{}" (iconversorasn.h:56) com o trace do asn1.

### 3.6 Codificação, criptografia opcional, envelope, arquivo

1. `CFileASN::CodeObjectFunction` codifica a `EntidadeBoletimUrna` (5955 "Objeto com conteúdo inválido para {}: {}",
   5956 "Arquivo não foi codificado para …").
2. **Criptografia** só se `CGravadorBU +212` (sempre true) **e** o parâmetro do PU `criptografarBU` (False em todos os
   cenários): tabela criptográfica de 1024 bytes do slot 4 de `api::IUrna`, 32 bytes aleatórios do slot 4 de
   `ecourna::api::security::IRng`, a chave pública `/dsk/fi/estatico/chave/bu.pk1` (uma `EntidadeChave` cujo conteúdo é
   decifrado com uma chave de cifragem de chaves do slot 3 de `api::IKernelHSM` através de `CSymmetricCipherFactory`;
   **8639 "O arquivo … não existe"**, **8640 "O arquivo … está vazio"**), `CPlainText(zona, seção, tabela, aleatório, chave, bytes)` (func
   5168) → `CCepescCipher::vf2` → `CSeguranca(0, 1, …)`. (No build web o CEPESC é um mock que não cifra,
   u01/u11.)
3. **Envelope** `EntidadeEnvelopeGenerico{cabecalho (same), fase, identificação (seção, or contingência), tipoEnvelope
   = envelopeBoletimUrna (1), [seguranca], conteudo = the BER bytes (or the ciphertext)}` — sem o
   `urna` opcional (construtores comum_f5861 / comum_f3821, ambos seguidos de `CEnvelopeGenerico::ValidaCriacao`, func 3822:
   8678..8684). Ele é gravado com `CFileASN::CodeObjectFunction` no `api::CFile` aberto por `IGravador::Grava`
   (`trab1/<nome>-bu.dat` na MI).
4. Depois, `IGravador` o copia para o diretório de resultado da MI e para os diretórios de trabalho e de resultado da MV,
   e `CAssinador::AssinaArquivosResultado` faz o SAVD assiná-lo em `…-vota.vsc` (u07).

### 3.7 O que protege o BU, até onde este código vai

* cadeia de hashes + assinatura PKCS#11 do último hash (por eleição) — *dentro* do arquivo;
* a assinatura SAVD do arquivo inteiro (`vota.vsc`) — fora dele;
* a criptografia CEPESC opcional (desligada nos cenários);
* neste build: nenhum PKCS#11, um mock de SAVD que não assina nada e um mock de CEPESC (veja §9).

O BU impresso, seus QR codes e seus códigos de verificação são produzidos em outro lugar (`CGeraBU`, u08; `CGeradorBUQRCode`, u04).

## 4. Registro de comparecimento da seção: `CGravadorRCSecao` (`…-jufa.dat`, func 11616)

`EntidadeResultadoUrnaCadastro` (as classes `ecourna::app::dados` são reconstruídas pela u14):

1. cabeçalho: a data/hora de geração é formatada como `"{:04}{:02}{:02}T{:02}{:02}{:02}"` (@8913), lida de volta com
   `boost::posix_time::from_iso_string` e passada a `CCabecalhoEntidade(ptime, pleito, 1)`;
2. `CIdentificacaoSecaoEleitoral(município ≤ 99999, zona, local, seção ≤ 9999)` (`CBaseType`s com verificação de intervalo);
3. para cada eleitor do cadastro (`CEleitores`, map indexado pela identidade do eleitor) que esteja *apto* no turno atual
   (`comum_f2266`: impedimento do turno == 0), um `CEstadoComparecimento`:
   `CRegistroIdentificacaoEleitor(identidade estática, identidade de habilitação)` (`CriaIdentidadeEleitor`, func
   5845: título → `CNumeroInscricaoEleitoral`, CPF → `CNumeroCPF`, livre → `CNumeroIdentificacaoLivre`, senão 8654
   "Tipo de identidade invalida: {}"), depois conforme a situação dinâmica: 0 → só a situação; 1 → + foto + áudio;
   nos demais casos, conforme o tipo de habilitação: 0 (sem biometria) → + foto + áudio; 1 (impressão digital) → + `CHabilitacaoBiometrica`
   (tentativas, dedo 0..999, score, erro de leitura); 2 (código do mesário) → + biometria + `CEstadoHabilitacaoPorCodigo`
   (a identidade do mesário — número e tipo — é buscada entre as linhas do período de abertura da tabela
   `comparecimento_mesario`); outros tipos são ignorados;
4. justificativas (`registro_justificativa`: título + ano de nascimento), lidas apenas se aquele singleton de repositório
   (@1839012) já existir; mesários da abertura (período 1) e do encerramento (período 2), em duas passadas filtradas
   separadas sobre o repositório `comparecimento_mesario` (linhas DAO de 52 bytes). As duas listas de mesários estão **sempre
   presentes** na entidade (flags de opcional ligadas incondicionalmente), vazias quando a persistência não está inicializada;
5. `CDadosComparecimento`; se o parâmetro do PU `criptografarJUFA` (cfg +161): codifica-o em BER, sorteia 32 bytes aleatórios
   (`IRng`), lê `/dsk/fi/estatico/chave/jufa.pk1` (mesmo esquema de `bu.pk1`; 8655/8656), cifra com CEPESC e
   armazena `DadosComparecimentoCifrado{DadosCifracao{chave, salt, informação adicional}, conteúdo}`; senão, os dados
   em claro;
6. `CResultadoUrnaCadastro(cabeçalho, fase ecourna ('o'→2, 's'→1, 't'→3), "10.23.0.1 - DESENVOLVIMENTO",
   ArquivoFinal, dados)` gravado com `CFileASN`.

## 5. Hashes da instalação: `CGravadorHashes` (`…-hash.dat`, func 11620)

`md::CEntidadeHashes{cabeçalho, fase, UF (2 letters, 8673), identificação (contingência: município+zona, or seção:
município+zona+local+seção; neither → 8644 "Tipo de urna inválido"), versão, list of {caminho, hash}}`, validado pela
func 5855. A lista = `CMontadorHash::CalculaHashGeral(CPath root)` + `CalculaHashGeral("/dsk/fi/estatico/chave/")`,
cada uma achatada pelas funcs 5358/5359. Diretórios excluídos: `root/dev/`, `root/dsk/`, `root/proc/`, `root/sys/`,
`root/tmp/`, `MI/dinamico/`, `MV/dinamico/`, `MI/dinamico/tmp/`. Assim, o arquivo cobre os **arquivos do sistema e as
chaves**, não os dados da eleição em `/dsk/fi/estatico/` (só o seu subdiretório `chave/` é adicionado de volta). No build
web nenhum `IGenericFactory<IHash>` é linkado: é neste gravador que a execução do harness de codepath.md para.

## 6. Pacotes de impressões digitais: `CGravadorWSQ` (`wsqbio.jez` / `wsqman.jez` / `wsqmes.jez`)

Construtor func 3796 (ValidaTipoBiometria inlinada: tipo < 3, senão 8651). A classe sobrescreve os slots de *cópia* e
deixa `Grava`/`GravaMV`/`GravaResultado` vazios: `CopiaParaResultado` (11579) compacta as imagens da MI diretamente no
diretório de resultado da MI; `CopiaParaMV` (11577) / `CopiaResultadoParaMV` (11578) compactam as imagens da MI / MV no
diretório de resultado da MV, só quando a flag `+84` está ligada (a u07 passa `!EhFaseTreinamento()`). O zip em si é `CompactaWsq`
(func 5821, u12): nível "store", `*.wsq` de `habilitado/`, `nao-habilitado/` ou `operador/`. O membro `+56 =
"wsq.pk1"` (um nome de arquivo de chave) nunca é usado por este código: os pacotes não são cifrados aqui.

## 7. Modelos de dados (`md::`) e conversores: resumo das validações

| verificação | código | onde |
|---|---|---|
| fase `'o'/'s'/'t'` | 8657 | construtor de IGravador (iresultado.cpp:40) |
| município < 100000, zona < 10000, seção < 10000 (nome do arquivo) | 8645/8646/8647 | cgravadorutil.cpp:28/33/38 |
| fase → `'1'/'2'/'3'` | 8649 "Fase inválida: {:#x}" | cgravadorutil.cpp:97 |
| serial da flash: 8 caracteres hex / 4 bytes | 8631/8632/8633 | asn/util.cpp:26/31/54 |
| envelope: fase, município, zona, local, tipo de urna, seção, tipo de envelope (< 5) | 8678..8684 | md/cenvelopegenerico.cpp:127..156 |
| getters do envelope: "Não há informação de urna / de local / de criptografia" | 8675/8676/8677 | md/cenvelopegenerico.cpp:96/106/117 |
| urna: tipo, tipo de arquivo, serial MV | 8689..8692 | md/curna.cpp:58..69 |
| hashes: fase, UF | 8672/8673 | md/centidadehashes.cpp:59/62 |
| versões de arquivos: tag, lista | 8694/8695 | md/cversoesarquivos.cpp:28/33 |
| busca de propriedades | 8663 / 8697 "Propriedade inexistente: {}" | md/cdependenciascontratos.cpp:59, md/cversoescontratos.cpp:42 |
| envelope ↔ ASN: tipo de urna, tipo de envelope, urna+criptografia juntas | 8611..8615 | asn/cconversorenvelopegenerico.cpp:72..191 |
| urna ↔ ASN: tipo urna / tipo arquivo | 8627..8630 | asn/cconversorurna.cpp:77..139 |
| apuração SA: tipo, motivos (manual 1–3/99, eletrônica 1–3/99, mista BU 1–5/99, mista MR 1–6/99) | 8621..8626, 8687/8688 | asn/cconversortipoapuracaosa.cpp, md/ctipoapuracaosa.cpp |
| correspondência ↔ ASN | 8601/8602 | asn/cconversorcorrespresultado.cpp:53/85 |

Mapeamento de `TipoEnvelope` (C++ 0..4 ↔ ASN.1): BU 1, RDV 2, BU impresso 4, imagem de biometria 5 (?), zerésima impressa 6
(tabelas @546580 / @546600).

## 8. `IInterfaceInit`: requisições ao serviço init

Todo método envia um comando pelo virtual puro `EnviarMensagem(cmd)` (slot 5) e lê um
`std::vector<int>`; uma resposta vazia lança "Resposta nula".

| método (func) | comando | falha | resposta web (lida de `simulador::CWasmInit::EnviarMensagem`, func 9405, que registra `"CWasmInit::{} {}"` num destino que a execução headless não mostra) |
|---|---|---|---|
| `CurrentBootDevice` (11646) | 37 | 7231 | sobrescrito: sempre 1 |
| `IsMRPresenteSemHabilitar` (2862) | 18 | 7237 | 1 (`"= no"`) → false |
| `IsMRMontadoSemHabilitar` (5897) | 17 | 7238 | 1 → false |
| `MontarMRSemHabilitar` (5896) | 34, depois 10, **sleep de 500 ms**, 12 | 7239/7240, 7282/7283, 7293/7294 | 0 / 0 / 0 (dispositivo 0: sem serial) |
| `DesmontarMRSemDesabilitar` (3832) | 58 | 7241/7242 | 0 |
| `DesligarUrna` (5894) | registra em log "Urna desligada a pedido do aplicativo", 48 | 7268/7269 | 0 (nada é desligado) |
| `EnviarMensagemThrowVoid` (3833) | qualquer | 7293 "{}: Resposta nula", 7294 "Falha ao enviar mensagem {}, {}: {}" | — |
| `GetDemoMode` (729) | 67, guardado em cache num `shared_ptr<bool>` | 7295/7296 | 0 (`"= 0"`) → não é demo |

`MontarMRSemHabilitar` também lê o tamanho da MR (`/sys/block/sdX/size × queue/logical_block_size`) e o serial USB
a partir de um caminho sysfs específico do modelo (slot 0 de `api::IUrna` = modelo): 2009/2010/2011/2013 → `…/0000:00:1d.8/usb1/1-0` ou
`1-8`; 2015 → `…/1d.7/usb1/1-0` ou `1-7`; 2020/2022 → `…/15.0/usb1/1-3` ou `usb2/2-3`; outros modelos → sem serial.
Registra em log "Tamanho da MR: {}" e (syslog/`DEBUG_UENUX`) "Serial da MR: %s" quando uma MR nova é vista.

## 9. `IInterfaceSavd`: o protocolo do serviço de assinatura

Requisição = header de 8 bytes `{0xFE, aplicação, u16 comando, u32 tamanho}` + payload; resposta = header de 12 bytes `{0xFE,
erro, pad, u32 código, u32 tamanho}` + (em caso de erro) mensagem `{0xFE, texto}`. Dois auxiliares de troca: func 3830 (lança nos
erros de protocolo 7305..7309, "Motivo não informado pelo serviço." quando não há texto; o código de erro é escrito por um
parâmetro de saída explícito, `&m_codigoErro` em todo ponto de chamada) e func 3829 (retorna o texto do erro).

| requisição | comando | payload | chamador |
|---|---|---|---|
| assinar um arquivo dinâmico (`AssinarUE`) | 0x0042 | `{0xFE, '=', arquivo}` | `CAssinador::Assina` (1277) ← `SalvaEstado` (491), `CSincronizaVota`, … — **observado na inicialização e durante as votações gravadas** |
| assinar um arquivo de resultado (`AssinarEcourna`) | 0x0080 | `{0xFE, '=', arquivo}` | `CAssinador::AssinaArquivosResultado` (u07) |
| abrir/fechar sessão do HSM (`EnviarAcaoHSM`, 5889) | 0x1604 | `{0xFE, 0 or 1}` | idem |
| validar a assinatura UE de um pacote (`ValidarUE`, 5890) | 0x2021 | TLV de api_f3831 | `vota::VerificaAssinaturaMV/MI` |
| definir o "local" da seção | 0x0404 | `{0xFE, 15 chars}` | u07 (inlinado) |

Erros: 7336 "Falha ao assinar ({}-UE[{}])", 7340 "Falha ao enviar ação ao HSM. {}", 7323 "Falha ao validar
assinatura UE\npacote: ({})\nErro SAVD: ({})\n{}". `CAssinador::Assina` envolve a chamada num
`api::CApplicationContextGuard(ReinicieOuFotografeQRCode, "Erro na assinatura", "", "Ocorreu um erro durante a
assinatura do arquivo: " + nome)`, de modo que uma falha é mostrada ao operador com esse texto.

## 10. `CInformacaoEleicao`

Uma visão sobre `CParametrosUrna` (cfg +88). No modo demonstração toda contagem é 1 e o registro de mesários fica desligado;
caso contrário, os valores do PU: `numBUVotaObrigatorios` (3847), `numBUVotaAdicionais` (5914), `numRelatorioEstado` (2865,
vista executando: a tela "Emissão do estado da urna"), `numRelatorioEleitores` (5917), `numRelatorioVersoesDados`
(5916), `numRelatorioPU` (5915), `registrarMesarios` (1950). `EhModoDemonstracao` (2286) é a própria consulta estática.
As ferramentas nomearam todas as oito funções como `EhModoDemonstracao` porque esse é o único srcloc (linha 40) nelas.

## 11. O que é específico do build web

* **O SAVD é um mock** (`(anonymous)::CWasmSavd`, registrado por `main`): `EnviaMensagem` não faz nada e
  `RecebeMensagem` (func 10949) retorna o header OK de 12 bytes para qualquer requisição (e lança `std::invalid_argument`
  quando lhe pedem qualquer tamanho diferente de 12), e armazena 0x0CABECA0 (o valor que o slot 4 retorna) no seu
  parâmetro de saída `estado`. Todo `AssinarUE`, ação de HSM e `ValidarUE` portanto **tem sucesso sem que nada seja
  assinado ou verificado**; os arquivos `.vsu` do build web são o texto falso escrito por `votaInit`.
* **O serviço init é um mock** (`simulador::CWasmInit`): nunca há MR presente, a flag de demonstração está desligada e o
  desligamento não faz nada (§8).
* Os gravadores de BU / RC / hash são inalcançáveis a partir da página (sem encerramento; `CGravaResultado` precisa de
  `treinamentoEleitor == 0`). Mesmo se alcançados: nenhum `IPkcs11`, nenhum `IKernelHSM`, nenhum `IGenericFactory<IHash>`,
  nenhum arquivo de chave, `/etc/*.properties` de 0 bytes (docs/bu/codepath.md §6).
* Funções desta unidade vistas executando durante as votações gravadas: 1277 + 5891 + 3829 (assinatura em `SalvaEstado`, ≈14
  amostras), 729 + 2865 (a tela de horário da zerésima pergunta quantas cópias do "estado da urna" são permitidas), 3830 (uma
  verificação de arquivo pelo SAVD na inicialização, via func 5892), 3604 (conversão do serial ao carregar os arquivos de estado,
  chamador 11400) e 3605 (ao salvá-los, chamador 11399 `CConversorDadoCorrespondencia::DoConverte`), 819
  (`CPartidos`), 276 (atribuição de string).

## 12. Observações sobre wasm / Emscripten

* **O inlining da LTO esconde funções inteiras.** `CGravadorBU::GravaResultado` (15.9 KB) contém `MontaEntidadeBU`,
  `CEntidadeBU::ValidaCriacao`, `LeChavePublica` e o loop de agrupamento de votos; `CGravadorRCSecao::GravaResultado`
  (15.8 KB) contém `LeChavePublica` e o montador do comparecimento; `CConversorEntidadeBU::DoConverte` (12.2 KB)
  contém seis auxiliares de cconversorentidadebu.cpp e todo o cassinavotavelbu.cpp.
* **Nomes enganosos das ferramentas** corrigidos aqui: 11616 "LeChavePublica" e 11629 "vf7" são `GravaResultado`; 1396
  "IResultado::IResultado" é o construtor de `IGravador`; 1277 "AssinarUE" é `CAssinador::Assina`; 3830
  "DesconverteMensagem" é a requisição ao SAVD; 3796 "ValidaTipoBiometria" e 5867 "ValidaCriacao" são construtores;
  10284/10287 são `CConversorUrna::DoDesconverte/DoConverte`; as oito "EhModoDemonstracao" são getters diferentes.
* **merge-similar-functions**: `CVersoesContratos::GetValor`/`CDependenciasContratos::GetValor` → um único corpo (6044)
  que recebe o srcloc e o código de erro; `IsMRPresente`/`IsMRMontado` → 6046; os dois construtores das classes de erro → ecourna_f710.
* **ICF**: 12090 é o destrutor de `IResultado`, `IGravador` e `CGravadorRCSecao`; 4694 é compartilhado por
  `CAssinador`/`CAssinadorVota`; 2900 é o criador preguiçoso de todo singleton de 1 byte.
* A espera de 500 ms em `DispositivoMR` compila para `if (byte@1584624 == 1) emscripten_sleep(500)`. Não pode ser
  `std::this_thread::sleep_for` (a libc++ encaminha isso para `nanosleep`, um loop de espera ativa neste build). O byte @1584624 é
  uma variável estática com valor 1 no segmento de dados e nenhum código escreve nele (24 funções só o leem), então o sleep sempre
  executa, e sem ASYNCIFY o `_emscripten_sleep` do glue aborta o módulo.
* O empacotamento de argumentos de `std::format` é visível nas constantes: ex.: 3491 = (int, string_view, int) para "Falha ao
  enviar mensagem {}, {}: {}". Os endereços de dados escritos como `d_…[N]` no texto descompilado são `N + 1024` absolutos.

## 13. Questões em aberto

* Os nomes dos slots de `IPkcs11` (23/6/24) e dos códigos de comando do SAVD (0x42, 0x80, 0x404, 0x1604, 0x2021); o
  valor mágico retornado pelo slot 4 de `CWasmSavd` (0x0CABECA0).
* Se o módulo PKCS#11 faz um pré-hash do último hash de 64 bytes (o leitor do TSE verifica `SHA-512(ultimoHash)`, veja
  docs/10-boletim-de-urna.md §4.2).
* A semântica exata dos valores de situação de `CEleitorDinamico` usados pelo gravador do jufa (0 ausente, 1 …) e da
  busca de reconhecimento de mesários (o filtro por período é um palpite).
* Slots 3, 4, 6, 7 de `IInterfaceInit` (só as sobrescritas web são visíveis).
* O byte em `CUrna +108` e o layout exato de `CDadoCorrespondencia` (u05/u07 discordam em campos menores).

## 14. Código estranho ou arriscado

| # | func | o quê | impacto |
|---|---|---|---|
| 1 | 3829, 5891, 5889, 5890, 1277, 10949 | **A assinatura e a validação de assinaturas são simuladas**: `CWasmSavd` responde "OK" a toda requisição ao SAVD, então `AssinarUE`, a sessão do HSM, `ValidarUE` e as verificações de arquivos na inicialização têm todas sucesso sem nenhuma criptografia | só no simulador; quem estuda o build web não deve tomá-las como verificações de segurança reais |
| 2 | 5896 | `DispositivoMR` dorme 500 ms via `emscripten_sleep` (protegido por um byte estático que é sempre 1), o que aborta o módulo (sem ASYNCIFY) | inalcançável na página (o mock diz que não há MR presente, e o encerramento nunca executa); quebraria o simulador se alcançado |
| 3 | 5896 | tamanho da MR = setores `int` × tamanho de bloco em 32 bits: estoura para mídias > 2 GiB; só a linha de log "Tamanho da MR" fica errada | urna real, se o código for o mesmo: apenas um valor errado no log |
| 4 | 3829 | a mensagem de resposta é lida sem verificar o tamanho recebido (`msg[0]`, depois `msg+1 .. msg+tamanho`): um SAVD que declare uma mensagem mais longa do que a que envia faz o cliente ler além do buffer | urna real: over-read no heap, limitado a um daemon local confiável; web: nunca disparado (o mock nunca reporta erro) |
| 5 | 11620 | `hash.dat` exclui `root/dsk/` (e só adiciona de volta `/dsk/fi/estatico/chave/`), então os dados estáticos da eleição da MI e toda a MV não estão nos hashes da instalação | informativo: `hash.dat` cobre o sistema e as chaves, não os arquivos de dados da eleição (que têm suas próprias assinaturas) |
| 6 | 11629 / 3819 | `std::map::at` na ordem dos cargos e nos aptos por abrangência: um cargo ausente de `CCargos` ou uma abrangência sem aptos lança `std::out_of_range` ("map::at:  key not found"), e não um erro do TSE, no meio do encerramento | urna real: só com dados de eleição inconsistentes; abortaria a geração do BU com uma mensagem genérica |
| 7 | 11629 | o comparecimento escrito no BU é recalculado como `rdv.Cargo(1st cargo) / qtdEscolhas`, e não tirado do cadastro; as consultas reutilizam o número do majoritário | consistente na prática (verificado antes por assert como igual à contagem do cadastro), mas é um valor derivado da apuração |
| 8 | 10273 | a cadeia de hashes vive num vetor estático (@1910004) compartilhado por todas as conversões e reiniciado a cada eleição; uma segunda conversão concorrente intercalaria as cadeias | uma única thread na prática; nenhum impacto observado |
| 9 | 3604 | `DesconverteSerialFlash` formata todos os bytes antes de verificar que são exatamente 4; o texto de erro mostra a saída formatada, não a entrada | cosmético |
| 10 | 1396 | o nome do arquivo é montado (com o char bruto da fase) antes de a fase ser validada | cosmético: o objeto não é criado de qualquer forma |
| 11 | 10271 / 10289 | um `tipoUrna` desconhecido (> `'4'`) deixa a identificação do envelope/correspondência sem valor em vez de lançar exceção; a verificação de validade do ASN.1 o pega depois, com uma mensagem menos precisa | apenas uma lacuna defensiva |
| 12 | 3796 / 6041 | `CGravadorWSQ` armazena o nome do arquivo de chave `"wsq.pk1"`, mas nunca o usa: neste código, os pacotes de impressões digitais são apenas compactados (nível store), não cifrados | informativo (as imagens são dados pessoais biométricos; a proteção delas depende das camadas da MR/transmissão) |
| 13 | 11629 | a criptografia do BU depende do parâmetro do PU `criptografarBU` (False em todos os cenários); `CGravadorBU +212` é sempre true | informativo |
| 14 | 5896 | o serial da MR só é lido para os modelos de urna 2009/2010/2011/2013/2015/2020/2022; outros modelos registram um serial vazio no log | urna real: outros modelos ficam sem serial da MR no log |
| 15 | 11629 | o comparecimento por grupo é `rdv.Cargo(cargo) / cargo.qtdEscolhas` (`i32.div_u` pelo byte em `CCargo` +13) sem verificação de zero: dados de eleição com um cargo cujo `qtdEscolhas` seja 0 fazem o encerramento dar trap ("integer divide by zero") em vez de lançar um erro do TSE | urna real (mesmo código-fonte): comportamento indefinido/crash apenas com dados de eleição malformados; web: inalcançável |
| 16 | 3820 | linhas com zero votos são descartadas incondicionalmente; o parâmetro do PU `gravarZeradosBU` (+393) não é consultado por este gravador | informativo: quem lê o BU deve tratar um candidato/partido ausente como 0 votos |
| 17 | 10289 | a identificação `urna.correspondenciaResultado` de uma urna de seção sempre traz a constante local `1` (o modelo C++ não tem local); só o envelope traz o local real | informativo: os leitores devem tirar o local do envelope/`identificacaoSecao`, não da correspondência |
| 18 | 11629 / 3822 / 5855 | as verificações de fase de `CEntidadeBU`, `CEnvelopeGenerico` e `CEntidadeHashes` rejeitam apenas `'0'` e valores `>= '4'` (com sinal), e não tudo o que está fora de `'1'..'3'` | apenas uma lacuna defensiva (a fase sempre vem de `ConverteFase`, que produz `'1'..'3'`) |

## 15. Tabela de mapeamento: todas as funções da unidade

`ran` = vista executando nas votações gravadas (analysis/runtime). As linhas "instanciação de biblioteca" são templates da libc++ (crescimento de vector, cópia não inicializada, destruição de árvore) ou código gerado pelo compilador, que pertencem ao arquivo onde foram instanciados; não carregam lógica do TSE e são apenas resumidas na reconstrução. Os caminhos sob `…/comum/` são `src/uenux2/src/app/comum/`.

| func | bytes | ran | símbolo reconstruído | arquivo original | reconstrução | conf. |
|---:|---:|:-:|---|---|---|---|
| 276 | 14 | ✔ | `std::string::assign(const char*)` | biblioteca (libc++ <string>) | - | média |
| 283 | 18 |  | `ecourna::api::exception::CBaseError<comum::EUeComumGravadoresError, SErrorLimits{8600,8800}>::CBaseError` | biblioteca/auxiliar inlinado: ecourna-lib/ecourna/api/exception/cbaseerror.hpp (thunk de construtor mesclado) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 480 | 18 |  | `ecourna::api::exception::CBaseError<comum::EUeComumError, SErrorLimits{7200,7600}>::CBaseError` | biblioteca/auxiliar inlinado: ecourna-lib/ecourna/api/exception/cbaseerror.hpp (thunk de construtor mesclado) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 729 | 736 | ✔ | `comum::IInterfaceInit::GetDemoMode` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 819 | 84 | ✔ | `comum::CPartidos::GetInst` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | média |
| 1007 | 89 |  | `comum::CAssinador::~CAssinador` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | alta |
| 1274 | 18 |  | `comum::CPath::GetPathResult(EFlashOrigem) [current turn]` | uenux2/src/app/comum/cpath.cpp | …/comum/gravadores/u23-foreign-fragments.cpp | média |
| 1277 | 870 | ✔ | `comum::CAssinador::Assina` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | alta |
| 1396 | 572 |  | `comum::IGravador::IGravador` | uenux2/src/app/comum/gravadores/iresultado.cpp (construtor de IResultado inlinado; construtor de IGravador, arquivo de IGravador inferido) | …/comum/gravadores/iresultado.cpp | média |
| 1556 | 224 |  | `std::vector<comum::md::CVotosCargo>::vector(first,last,n)` | instanciação de biblioteca (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 1944 | 282 |  | `std::vector<comum::md::CVotosVotavel>::push_back` | instanciação de biblioteca (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 1950 | 72 |  | `comum::CInformacaoEleicao::IdentificaMesarios` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 2192 | 379 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(id,situacao,foto,audio,biometria)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 2193 | 176 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(id,situacao,foto,audio)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 2274 | 506 |  | `comum::CGravadorUtil::ConverteFase` | uenux2/src/app/comum/gravadores/cgravadorutil.cpp | …/comum/gravadores/cgravadorutil.cpp | alta |
| 2278 | 423 |  | `std::__uninitialized_allocator_copy<ecourna::app::dados::CEstadoComparecimento>` | instanciação de biblioteca | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | média |
| 2286 | 52 |  | `comum::CInformacaoEleicao::EhModoDemonstracao` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | alta |
| 2802 | 329 |  | `comum::md::CCarga::CCarga` | uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 2845 | 394 |  | `std::__uninitialized_allocator_move_if_noexcept<ecourna::app::dados::CEstadoComparecimento>` | instanciação de biblioteca | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | média |
| 2853 | 423 |  | `std::__uninitialized_allocator_copy<comum::md::CResultadoVotacaoPorEleicao>` | instanciação de biblioteca | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 2856 | 39 |  | `comum::md::CVotosVotavel::CVotosVotavel(tipo,codigo,quantidade)` | uenux2/src/app/comum/gravadores/md/cvotosvotavel.h (caminho inferido) | …/comum/gravadores/cgravadorbu.h | média |
| 2862 | 17 |  | `comum::IInterfaceInit::IsMRPresenteSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 2865 | 72 | ✔ | `comum::CInformacaoEleicao::GetNumRelatorioEstado` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 2900 | 61 |  | `comum::(anonymous)::GetInstLazy<1-byte T> (ICF)` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp (corpo ICF compartilhado com comum_f348/948/2742) | …/comum/gravadores/asn/cconversorentidadebu.cpp (comentário) | baixa |
| 3602 | 290 |  | `comum::asn::(anonymous namespace)::CalculaHash` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp | …/comum/gravadores/asn/cconversorentidadebu.cpp | média |
| 3603 | 15 |  | `comum::asn::CAssinaVotavelBu::GetInst` | uenux2/src/app/comum/gravadores/asn/cassinavotavelbu.cpp (caminho inferido) | …/comum/gravadores/asn/cconversorentidadebu.cpp | baixa |
| 3604 | 585 | ✔ | `comum::asn::DesconverteSerialFlash` | uenux2/src/app/comum/gravadores/asn/util.cpp | …/comum/gravadores/asn/util.cpp | alta |
| 3605 | 748 | ✔ | `comum::asn::ConverteSerialFlash` | uenux2/src/app/comum/gravadores/asn/util.cpp | …/comum/gravadores/asn/util.cpp | alta |
| 3751 | 57 |  | `comum::CPartidos::CPartidos` | uenux2/src/app/comum/dados/cpartidos.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | média |
| 3796 | 268 |  | `comum::CGravadorWSQ::CGravadorWSQ` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | alta |
| 3798 | 2070 |  | `comum::CGravadorUtil::DeterminaNomeArquivo` | uenux2/src/app/comum/gravadores/cgravadorutil.cpp | …/comum/gravadores/cgravadorutil.cpp | média |
| 3814 | 32 |  | `std::__tree<std::pair<TCargoID,uebyte>>::destroy` | instanciação de biblioteca (cgravadorbu.cpp) | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 3815 | 278 |  | `ecourna::app::dados::CInformacaoMidia::~CInformacaoMidia` | biblioteca/auxiliar inlinado (destrutor implícito do modelo de infomidia.dat) | …/comum/gravadores/cgravadorbu.cpp (comentário) | baixa |
| 3816 | 560 |  | `std::__uninitialized_allocator_move<comum::md::CCargo>` | instanciação de biblioteca | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 3817 | 796 |  | `std::vector<comum::md::CResultadoVotacao>::__push_back_slow_path` | instanciação de biblioteca | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 3818 | 659 |  | `comum::(anonymous)::FiltraCargosPorTipo (std::copy_if)` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | média |
| 3819 | 3993 |  | `comum::CGravadorBU::MontaVotosCargos` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | média |
| 3820 | 749 |  | `comum::CGravadorBU::AcrescentaVotoVotavel` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | alta |
| 3822 | 432 |  | `comum::md::CEnvelopeGenerico::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | …/comum/gravadores/md/cenvelopegenerico.cpp | alta |
| 3824 | 62 |  | `comum::md::CResultadoVotacao::CResultadoVotacao` | uenux2/src/app/comum/gravadores/md/cresultadovotacao.h (caminho inferido) | …/comum/gravadores/cgravadorbu.h | média |
| 3825 | 20 |  | `comum::md::CVersoesContratos::GetValor` | uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp | …/comum/gravadores/md/cversoescontratos.cpp | alta |
| 3826 | 20 |  | `comum::md::CDependenciasContratos::GetValor` | uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp | …/comum/gravadores/md/cdependenciascontratos.cpp | alta |
| 3829 | 1112 | ✔ | `comum::IInterfaceSavd::EnviaComando` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | média |
| 3830 | 2671 | ✔ | `comum::IInterfaceSavd::EnviaRequisicao` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | média |
| 3832 | 581 |  | `comum::IInterfaceInit::DesmontarMRSemDesabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 3833 | 1100 |  | `comum::IInterfaceInit::EnviarMensagemThrowVoid` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 3847 | 72 |  | `comum::CInformacaoEleicao::GetNumBUVotaObrigatorios` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 4694 | 13 |  | `comum::CAssinador::~CAssinador [deleting]` | uenux2/src/app/comum/gravadores/cassinador.cpp | …/comum/gravadores/cassinador.cpp | alta |
| 5092 | 313 |  | `ecourna::app::dados::CComparecimentoSecao::CComparecimentoSecao` | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 5358 | 1227 |  | `comum::(anonymous namespace)::Achata (hash tree -> list)` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | baixa |
| 5359 | 47 |  | `comum::(anonymous namespace)::ListaHashes` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | baixa |
| 5375 | 459 |  | `std::vector<mesário DAO row (52 bytes)>::__push_back_slow_path` | instanciação de biblioteca (cgravadorrcsecao.cpp) | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | baixa |
| 5503 | 23 |  | `comum::IInterfaceSavd::~IInterfaceSavd` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | alta |
| 5841 | 600 |  | `comum::(anonymous)::CriaComparecimentoMesario` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | baixa |
| 5843 | 239 |  | `std::vector<CEstadoComparecimento>::__emplace_back_slow_path(...biometria)` | instanciação de biblioteca | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | média |
| 5844 | 237 |  | `std::vector<CEstadoComparecimento>::__emplace_back_slow_path(...audio)` | instanciação de biblioteca | …/comum/gravadores/cgravadorrcsecao.cpp (comentário) | média |
| 5845 | 938 |  | `comum::CGravadorRCSecao::CriaIdentidadeEleitor` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | alta |
| 5852 | 340 |  | `std::copy<md::CHashArquivo> (pair of strings)` | instanciação de biblioteca | …/comum/gravadores/cgravadorhashes.cpp (comentário) | média |
| 5853 | 201 |  | `comum::CGravadorHashes::~CGravadorHashes` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | alta |
| 5854 | 1843 |  | `std::vector<md::CHashArquivo>::insert(pos,first,last,n)` | instanciação de biblioteca | …/comum/gravadores/cgravadorhashes.cpp (comentário) | média |
| 5855 | 598 |  | `comum::md::CEntidadeHashes::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/centidadehashes.cpp | …/comum/gravadores/md/centidadehashes.cpp | alta |
| 5857 | 224 |  | `comum::CGravadorBU::~CGravadorBU` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | alta |
| 5859 | 232 |  | `std::vector<md::CResultadoVotacaoPorEleicao>::__base_destruct_at_end` | instanciação de biblioteca | …/comum/gravadores/cgravadorbu.cpp (comentário) | média |
| 5860 | 488 |  | `comum::md::CEnvelopeGenerico::CEnvelopeGenerico(cabecalho,fase,CUrna,...)` | uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | média |
| 5862 | 435 |  | `comum::md::CUrna::ValidaCriacao` | uenux2/src/app/comum/gravadores/md/curna.cpp | …/comum/gravadores/md/curna.cpp | alta |
| 5863 | 48 |  | `comum::md::CVotosVotavel::CVotosVotavel(tipo,codigo,quantidade,CIdentificacaoVotavel)` | uenux2/src/app/comum/gravadores/md/cvotosvotavel.h (caminho inferido) | …/comum/gravadores/cgravadorbu.h | média |
| 5864 | 18 |  | `comum::md::CIdentificacaoVotavel::CIdentificacaoVotavel` | uenux2/src/app/comum/gravadores/md/cidentificacaovotavel.h (caminho inferido) | …/comum/gravadores/cgravadorbu.h | média |
| 5867 | 327 |  | `comum::md::CVersoesArquivos::CVersoesArquivos` | uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp | …/comum/gravadores/md/cversoesarquivos.cpp | média |
| 5889 | 761 |  | `comum::EnviarAcaoHSM` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | alta |
| 5890 | 717 |  | `comum::ValidarUE` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | alta |
| 5891 | 257 | ✔ | `comum::IInterfaceSavd::AssinaArquivo` | uenux2/src/app/comum/iinterfacesavd.cpp | …/comum/iinterfacesavd.cpp | média |
| 5894 | 741 |  | `comum::IInterfaceInit::DesligarUrna` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 5896 | 5233 |  | `comum::IInterfaceInit::MontarMRSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 5897 | 17 |  | `comum::IInterfaceInit::IsMRMontadoSemHabilitar` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 5914 | 72 |  | `comum::CInformacaoEleicao::GetNumBUVotaAdicionais` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 5915 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioPU` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 5916 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioVersoesDados` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 5917 | 72 |  | `comum::CInformacaoEleicao::GetNumRelatorioEleitores` | uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp | …/comum/informacao/cinformacaoeleicao.cpp | média |
| 6041 | 331 |  | `comum::CGravadorWSQ::CompactaNaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | média |
| 6046 | 115 |  | `comum::IInterfaceInit::ConsultaFlag` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | média |
| 7766 | 7 |  | `vota::CAssinadorVota::~CAssinadorVota` | uenux2/src/app/vota/comum/cassinadorvota.h (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 10263 | 1029 |  | `comum::asn::CConversorVersoesArquivos::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.cpp (caminho inferido) | …/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp | alta |
| 10270 | 1851 |  | `comum::asn::CConversorEnvelopeGenerico::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | alta |
| 10271 | 2568 |  | `comum::asn::CConversorEnvelopeGenerico::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp | …/comum/gravadores/asn/cconversorenvelopegenerico.cpp | alta |
| 10273 | 12196 |  | `comum::asn::CConversorEntidadeBU::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp | …/comum/gravadores/asn/cconversorentidadebu.cpp | alta |
| 10282 | 369 |  | `comum::asn::CConversorTipoApuracaoSA::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp | …/comum/gravadores/asn/cconversortipoapuracaosa.cpp | alta |
| 10283 | 3122 |  | `comum::asn::CConversorTipoApuracaoSA::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp | …/comum/gravadores/asn/cconversortipoapuracaosa.cpp | alta |
| 10284 | 2592 |  | `comum::asn::CConversorUrna::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp | …/comum/gravadores/asn/cconversorurna.cpp | alta |
| 10287 | 2690 |  | `comum::asn::CConversorUrna::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp | …/comum/gravadores/asn/cconversorurna.cpp | alta |
| 10288 | 910 |  | `comum::asn::CConversorCorrespResultado::DoDesconverte` | uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp | …/comum/gravadores/asn/cconversorcorrespresultado.cpp | alta |
| 10289 | 1501 |  | `comum::asn::CConversorCorrespResultado::DoConverte` | uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp | …/comum/gravadores/asn/cconversorcorrespresultado.cpp | alta |
| 10431 | 101 |  | `vota::CEleitorVotouNaoVotou::StartState` | uenux2/src/app/vota/operador/aguardaeleitor/celeitorvotounaovotou.cpp (caminho inferido) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 10842 | 23822 |  | `api::CRHVoiceTextToSpeech::CRHVoiceTextToSpeech` | uenux2/src/api/audio/crhvoicetexttospeech.cpp (caminho inferido) | docs/libraries/rhvoice.md (não reconstruído aqui) | alta |
| 10965 | 10 |  | `(anonymous namespace)::CWasmSavd::~CWasmSavd [deleting]` | uenux2/wasm/vota_web/vota_web_wasm.cpp (caminho inferido: seus outros métodos 10930/10949 são app:wasm-entry; registrado por `main`) | …/comum/gravadores/u23-foreign-fragments.cpp | alta |
| 11575 | 92 |  | `comum::CGravadorWSQ::~CGravadorWSQ [deleting]` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | alta |
| 11576 | 89 |  | `comum::CGravadorWSQ::~CGravadorWSQ` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | alta |
| 11577 | 9 |  | `comum::CGravadorWSQ::CopiaParaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | média |
| 11578 | 9 |  | `comum::CGravadorWSQ::CopiaResultadoParaMV` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | média |
| 11579 | 319 |  | `comum::CGravadorWSQ::CopiaParaResultado` | uenux2/src/app/comum/gravadores/cgravadorwsq.cpp | …/comum/gravadores/cgravadorwsq.cpp | média |
| 11615 | 42 |  | `comum::CGravadorRCSecao::~CGravadorRCSecao [deleting]` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | alta |
| 11616 | 15849 |  | `comum::CGravadorRCSecao::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp | …/comum/gravadores/cgravadorrcsecao.cpp | alta |
| 11619 | 10 |  | `comum::CGravadorHashes::~CGravadorHashes [deleting]` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | alta |
| 11620 | 4622 |  | `comum::CGravadorHashes::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorhashes.cpp | …/comum/gravadores/cgravadorhashes.cpp | alta |
| 11628 | 10 |  | `comum::CGravadorBU::~CGravadorBU [deleting]` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | alta |
| 11629 | 15903 |  | `comum::CGravadorBU::GravaResultado` | uenux2/src/app/comum/gravadores/cgravadorbu.cpp | …/comum/gravadores/cgravadorbu.cpp | alta |
| 11646 | 138 |  | `comum::IInterfaceInit::CurrentBootDevice` | uenux2/src/app/comum/iinterfaceinit.cpp | …/comum/iinterfaceinit.cpp | alta |
| 12090 | 39 |  | `comum::IResultado::~IResultado` | uenux2/src/app/comum/gravadores/iresultado.cpp | …/comum/gravadores/iresultado.cpp | alta |

## 16. Arquivos reconstruídos

* [`src/uenux2/src/app/comum/gravadores/iresultado.h`](../../src/uenux2/src/app/comum/gravadores/iresultado.h)
* [`src/uenux2/src/app/comum/gravadores/iresultado.cpp`](../../src/uenux2/src/app/comum/gravadores/iresultado.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorutil.h`](../../src/uenux2/src/app/comum/gravadores/cgravadorutil.h)
* [`src/uenux2/src/app/comum/gravadores/cgravadorutil.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorutil.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorbu.h`](../../src/uenux2/src/app/comum/gravadores/cgravadorbu.h)
* [`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp)
* [`src/uenux2/src/app/comum/gravadores/cgravadorwsq.cpp`](../../src/uenux2/src/app/comum/gravadores/cgravadorwsq.cpp)
* [`src/uenux2/src/app/comum/gravadores/cassinador.cpp`](../../src/uenux2/src/app/comum/gravadores/cassinador.cpp)
* [`src/uenux2/src/app/comum/gravadores/u23-foreign-fragments.cpp`](../../src/uenux2/src/app/comum/gravadores/u23-foreign-fragments.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorurna.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversortipoapuracaosa.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/cconversorversoesarquivos.u23.cpp)
* [`src/uenux2/src/app/comum/gravadores/asn/util.cpp`](../../src/uenux2/src/app/comum/gravadores/asn/util.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cenvelopegenerico.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/curna.cpp`](../../src/uenux2/src/app/comum/gravadores/md/curna.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/centidadehashes.cpp`](../../src/uenux2/src/app/comum/gravadores/md/centidadehashes.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cversoesarquivos.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cversoescontratos.cpp)
* [`src/uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp`](../../src/uenux2/src/app/comum/gravadores/md/cdependenciascontratos.cpp)
* [`src/uenux2/src/app/comum/iinterfaceinit.h`](../../src/uenux2/src/app/comum/iinterfaceinit.h)
* [`src/uenux2/src/app/comum/iinterfaceinit.cpp`](../../src/uenux2/src/app/comum/iinterfaceinit.cpp)
* [`src/uenux2/src/app/comum/iinterfacesavd.h`](../../src/uenux2/src/app/comum/iinterfacesavd.h)
* [`src/uenux2/src/app/comum/iinterfacesavd.cpp`](../../src/uenux2/src/app/comum/iinterfacesavd.cpp)
* [`src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp`](../../src/uenux2/src/app/comum/informacao/cinformacaoeleicao.cpp)

Os outros arquivos de `comum/gravadores` (`cgravadorrdv`, `cgravadorlog`, `cgravadorenvelopearquivo`, `cgravadorversoesarquivos`, `igravador`, `md/centidadebu`, `md/cdadosbuvota`, `asn/cassinavotavelbu`, `asn/cconversorentidadehashes`) pertencem a outras unidades ou só existem inlinados; seus corpos inlinados são reconstruídos onde aparecem (veja os comentários nos arquivos acima).

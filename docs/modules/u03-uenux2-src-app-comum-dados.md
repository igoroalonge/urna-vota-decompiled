# u03 — `uenux2/src/app/comum/dados`: conversores ASN.1 para os dados da eleição e o estado da urna, além das fontes de texto de cargo

A unidade u03 tem 89 funções. 35 delas foram vistas em execução nos votos registrados. Ela cobre 25 arquivos-fonte
originais em `uenux2/src/app/comum/dados/`, mais um arquivo cujo caminho foi inferido
(`asn/municipiozona/cconversorhorarioverao.cpp`). Cerca de três quartos do código são **conversores**.
Um conversor transforma os objetos ASN.1 decodificados via BER dos arquivos de dados e dos arquivos de estado (`Modulo*`, ver
`docs/data-model/asn1-schemas.md`) nas classes de modelo da aplicação (`comum::md::*`), e faz o caminho
inverso quando a urna grava um arquivo. O restante são **fontes de dados** de texto para as telas e os
relatórios impressos (`ccargods.cpp`), e os helpers do singleton `CCandidaturas`.

Os arquivos-fonte reconstruídos estão em `src/uenux2/src/app/comum/dados/`. Há um `.cpp` por arquivo
original, e um `.h` quando uma declaração de classe ajuda. O mapa completo de funções está no §13.

---

## 1. Glossário

| termo | significado |
|---|---|
| *conversor* (`CConversorX`) | uma classe derivada de `comum::asn::IConversorASN<ENTIDADE, DADO>`. Converte ASN.1 em md com **Desconverte** e md em ASN.1 com **Converte** |
| *md* (`comum::md`) | "modelo de dados": as classes de dados C++ simples da aplicação |
| *processo eleitoral / pleito / eleição* | o processo eleitoral, seus turnos (1º/2º *pleito*) e as eleições dentro de cada turno |
| *cargo / consulta* | um cargo em disputa. Uma *consulta* (pergunta de referendo) é modelada como um cargo cujo detalhe é uma pergunta com respostas |
| *suplente / suplência* | candidatos substitutos exibidos ao lado do titular: vice-prefeito, vice-governador, 1º/2º suplente de senador |
| *seção / zona / município / local* | seção eleitoral / zona eleitoral / município / local de votação |
| *urna de contingência* | urna reserva. Seu `Local` tem apenas município + zona, sem seção |
| *TTE* (transferência temporária de eleitor) | eleitor transferido temporariamente para outra seção (voto em trânsito, preso provisório, acessibilidade, …) |
| *impedido* | eleitor impedido de votar nesta seção, por turno (P1/P2) |
| *biometria* | a foto e os templates de impressão digital (*minúcias*) de um eleitor |
| *CEPESC* | o centro de criptografia do governo brasileiro (ABIN) e a biblioteca de cifra `ecourna::api::cepesc` |
| *RDV* (registro digital do voto) | a lista de todos os votos dados, por eleição e cargo, sem vínculo com nenhum eleitor |
| *GAP* | o lançador de aplicações da urna: decide qual aplicação (vota, apuração, treinamento…) roda em seguida |
| *SA* (sistema de apuração) | a aplicação de apuração usada quando uma urna falha |
| *zerésima / BU* | o relatório zerado emitido antes da votação e o *boletim de urna*, o resultado apurado da seção |
| *DS* (data source) | um objeto-função que retorna um texto para um campo da GUI ou para um relatório impresso (`api::CDataText<DS>`) |

---

## 2. Arquivos, classes e os dados que tratam

| arquivo original | classe(s) | tipo ASN.1 ↔ tipo md | arquivo de dados |
|---|---|---|---|
| `asn/cconversorlocal.cpp` | `CConversorLocal` | `ModuloLocal::Local` ↔ `md::CLocal` | `<mun><zona><secao>-lo.dat` |
| `asn/candidatura/cvisitantefoto.cpp` | `CVisitanteFoto` (visitor ASN1) | índice de `EntidadeFotosCandidatos` | `-fo.dat` (fotos dos candidatos) |
| `asn/eleitor/cconversorbiometriaeleitor.cpp` | `CConversorBiometriaEleitor` | `BiometriaEleitor` ↔ `md::CBiometriaEleitor` | parte decifrada de `-el.dat` |
| `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` | `CConversorBiometriaEleitorCifrada` | `BiometriaEleitorCifrada` → `md::CBiometriaEleitor` | `-el.dat` + `chave/bio.sk1` |
| `asn/eleitor/cconversordedo.cpp` | `CConversorDedo` | `Dedo` → `md::CDedo` (template de impressão digital) | dentro da biometria |
| `asn/eleitor/cconversorentidadeeleitores.cpp` | `CConversorEntidadeEleitores` (parcial) | `EntidadeEleitores` → `md::CEntidadeEleitores` | `-el.dat`, `-tte.dat` (cadastro de eleitores) |
| `asn/eleitor/cconversorimpedido.cpp` | `CConversorImpedido` | `EntidadeImpedidos` → `vector<md::CImpedido>` | `-imp.dat` |
| `asn/eleitor/cvisitanteeleitor.cpp` | `CVisitanteEleitor` (visitor ASN1) | offsets da biometria por identidade do eleitor | `-el.dat` |
| `asn/estadoaplicacao/cconversorajustedatahora.cpp` | `CConversorAjusteDataHora` | `AjusteDataHora` ↔ `CAjusteDataHora` | em `eg.bin` |
| `asn/estadoaplicacao/cconversordadocarga.cpp` | `CConversorDadoCarga` | `DadoCarga` ↔ `CDadoCarga` | em `eg.bin` |
| `asn/estadoaplicacao/cconversorestadogeral.cpp` | `CConversorEstadoGeral` | `EstadoGeralUrna` ↔ `CEstadoGeral` | `dinamico/eg.bin` |
| `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | `CConversorEstadoGeralGap` | `EstadoGeralGap` ↔ `CEstadoGeralGap` | `trab1\|2/gap.bin` |
| `asn/estadoaplicacao/cconversorestadogeralsa.cpp` | `CConversorEstadoGeralSA` | `EstadoGeralSA` ↔ `CEstadoGeralSA` | `trab1\|2/sa.bin` |
| `asn/estadoaplicacao/cconversorestadogeralvota.cpp` | `CConversorEstadoGeralVota` | `EstadoGeralVota` ↔ `CEstadoGeralVota` | `trab1\|2/vota.bin` |
| `asn/municipiozona/cconversorcomplementosmunicipios.cpp` | `CConversorComplementosMunicipios` | `EntidadeComplementosMunicipios` → `CComplementosMunicipiosUF` | `-cm.dat` |
| `asn/municipiozona/cconversorhorarioverao.cpp` *(caminho inferido)* | `CConversorHorarioVerao` (apenas DoConverte) | `HorarioVerao` ← `CHorarioVerao` | em `Local` / `-cm.dat` |
| `asn/processoeleitoral/cconversorcargo.cpp` | `CConversorCargo` | `CargoPergunta` → `md::CCargo` | `-ce.dat` (eleição) |
| `asn/processoeleitoral/cconversordetalhecandidato.cpp` | `CConversorDetalheCandidato` | `DetalheCargo` → `CDetalheCandidato` | `-ce.dat` |
| `asn/processoeleitoral/cconversororigemconfiguracao.cpp` | `CConversorOrigemConfiguracao` | `OrigemConfiguracao` ↔ `EOrigemConfiguracao` | `-cp.dat` |
| `asn/processoeleitoral/cconversorprocessoeleitoral.cpp` | `CConversorProcessoEleitoral` | `EntidadeProcessoEleitoral` → `CProcessoEleitoralDTO` | `-cp.dat` |
| `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` | `CConversorTipoIdentificadorEleitor` | `TipoIdentificadorEleitor` ↔ `ETipoIdentificadorEleitor` | `-cp.dat` |
| `asn/rdv/cconversoreleicoesvota.cpp` | `CConversorEleicoesVota` | `Eleicoes` ↔ `md::CVotosEleicoesVota` | corpo do RDV (`rdv.dat`) |
| `asn/rdv/cconversorvoto.cpp` | `CConversorVoto` | `Voto` ↔ `md::CVoto` | um voto do RDV |
| `asn/rdv/cconversorvotoscargo.cpp` | `CConversorVotosCargo` | `VotosCargo` ↔ `pair<SCargoInfo, CVotos>` | votos do RDV de um cargo |
| `ccandidaturas.cpp` | `CCandidaturas`, `GetCandidaturaAtual` | — | lista de candidaturas em memória |
| `ccargods.cpp` | `CCargoDSNomeSexoCandidato`, `CCargoDSLabelRelatorio`, `GetNomeCargoComGenero` | — | textos de tela / relatório |

---

## 3. Hierarquia de classes (RTTI) e o protocolo dos conversores

```
comum::asn::IConversorASN<ENTIDADE, DADO>                 (uenux2/src/app/comum/asn/iconversorasn.h, unit u21)
 ├─ CConversorLocal, CConversorCargo, CConversorDetalheCandidato, CConversorOrigemConfiguracao,
 │  CConversorProcessoEleitoral, CConversorTipoIdentificadorEleitor, CConversorComplementosMunicipios,
 │  CConversorHorarioVerao, CConversorBiometriaEleitor, CConversorDedo, CConversorImpedido,
 │  CConversorAjusteDataHora, CConversorDadoCarga, CConversorEstadoGeral, CConversorEstadoGeralGap,
 │  CConversorEstadoGeralSA, CConversorEstadoGeralVota, CConversorVoto, CConversorVotosCargo,
 │  CConversorEleicoesVota                                (all "si" = single inheritance)
comum::asn::IConversorBiometriaASN<BiometriaEleitorCifrada, CBiometriaEleitor>
 └─ CConversorBiometriaEleitorCifrada
comum::asn::IConversorParcialASN<EntidadeEleitores, CEntidadeEleitores, CVisitanteEleitor>
 └─ CConversorEntidadeEleitores                (sibling: CIndexadorFotos over CVisitanteFoto, in another unit)
ASN1::IAbstractVisitor
 ├─ CVisitanteEleitor
 └─ CVisitanteFoto
api::IText
 ├─ api::CDataText<CCargoDSNomeSexoCandidato>
 └─ api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>
comum::IServicoEstado<CEstadoGeral|CEstadoGeralVota|CEstadoGeralGap|CEstadoGeralSA, CConversor...>  (users)
```

**vtable de `IConversorASN`** (4 slots, reconstruída a partir de 20 vtables): `[0]` destrutor (quase sempre o
`return this` do ICF, func 174). `[1]` destrutor de deleção (func 144 `free`). `[2]` `DoConverte(const TDado&)`
retornando `TEntidade`. `[3]` `DoDesconverte(const TEntidade&)` retornando `TDado`. As assinaturas de srcloc
confirmam os nomes e o retorno por valor, p. ex. `virtual md::CLocal comum::asn::CConversorLocal::DoDesconverte(const TEntidade &) const`.
Os wrappers não virtuais da classe base são:

* `Desconverte(e)` primeiro exige `e.isValid() && e.isStrictlyValid()`. Caso contrário, lança
  `CBaseError<EUeComumAsnError>(7654, "Entidade está inválida: {}")` com o dump de `ASN1::trace_invalid`
  (`iconversorasn.h:71`). Em seguida, chama `DoDesconverte`.
* `Converte(d)` chama `DoConverte` e depois exige que o *resultado* seja válido. Caso contrário, lança `7653 "Entidade
  deixada em estado inválido: {}"` (`iconversorasn.h:56`).
* Um conversor que implementa apenas uma direção deixa o slot da base no lugar. Esse slot da base lança
  `"Método DoConverte() não implementado para {}"`. A urna nunca grava arquivos de eleitores, candidatos ou eleições.

`IConversorParcialASN` tem apenas 3 slots. Seu slot 2 é `DoDesconverte(const TEntidade&, const TVisitor&)`.
`IConversorBiometriaASN` acrescenta um parâmetro `const md::CParametro&` às duas direções.

**Armadilha de nomes.** Muitas funções de vtable foram nomeadas pela ferramenta a partir de um **helper inlinado**,
porque o registro de srcloc pertence a esse helper (p. ex., 11355 "`CConversorVoto::ConverteTipo`" é na verdade
`CConversorVoto::DoConverte`, e 11410 "`MontaEleitorIdentidade`" é `CConversorImpedido::DoDesconverte`).
O slot da vtable determina o método real. O §13 traz os nomes corrigidos.

---

## 4. Carregando a configuração da seção

Estes conversores rodam enquanto a urna carrega seus dados estáticos (`/dsk/fi/estatico/...`).

* **Local** (`CConversorLocal`, 11451/11452). Os campos são idPE, país, UF (sigla + nome), Município (+
  `comBiometria` opcional, ausente = false), ComplementoMunicipio (fuso, horário de verão) e o CHOICE `id`:
  `secao [1] SecaoEleitoral` para uma urna normal, ou `contingencia [2] {municipio, zona}` para uma urna reserva. Um
  id de CHOICE fora de 0/1 lança `7887 "CHOICE de id inválida."`. `DoConverte` (a urna grava o seu local)
  verifica contingência **antes** de seção. Se nenhum dos dois estiver definido, lança `7888 "Tipo de local indefinido."`. Ele
  também valida o próprio resultado (`7889`/`7890`) além da verificação da classe base. O objeto md é
  `md::CLocal` (140 bytes). Sua parte de município é um `md::CInfoMunicipio`, construído pelos construtores inline
  5674/5676. Ambos terminam em `CInfoMunicipio::ValidaCriacao`, e o construtor de `CLocal` termina em `CLocal::ValidaCriacao`.
* **Complementos de municípios** (11381). O arquivo deve conter **exatamente uma** UF (`7944 "Quantidade inválida
  de UFs [{}]"`). A sigla da UF deve ter 2 caracteres (8115) e a lista não pode estar vazia (8116).
  `fusoTRE` e o cabeçalho são ignorados.
* **Processo eleitoral** (11363). Mantém id, nome, pleito1, pleito2 opcional, `utilizaBiometria`,
  `origemConfiguracao` e os tipos de identificador. O tipo de identificador principal deve aparecer na lista de
  tipos permitidos (`8185 "Identidade principal ({}) ausente na lista de identidades"`). O id vem do
  CHOICE `IDEleitoral` do cabeçalho, e o código **não verifica qual alternativa está selecionada**.
  `dataEleitorado`, `tipoEleicaoPrincipal`, abrangência, `permiteMRJ`, `tiposTTEPermitidos` e `ufs` são
  descartados aqui.
* **Cargo** (11373). `TipoCargoConsulta` 1..3 é mapeado para `CCargo::ETipo` 0..2. O objeto conversor carrega
  a abrangência da eleição (definida por `CConversorEleicaoPE`, func 11371) e a copia para cada cargo.
  `detalhe` é um CHOICE: `cargo [0]` vai para `CConversorDetalheCandidato`, `pergunta [1]` vai para
  `CConversorDetalheConsulta`, e qualquer outro valor lança `7948 "Opção de Detalhe Inválida."`. O `CCargo` md tem
  140 bytes e contém `optional<CDetalheCandidato>` (flag +84) e `optional<CDetalheConsulta>` (flag
  +136). Os testes de "isto é uma consulta?" em outros pontos leem a flag em +136.
* **DetalheCandidato** (11375) impõe `qtdeSuplentes == size(suplencias)` com três erros distintos
  (7950/7951/7952). `temFoto` e `nomes` (nome do cargo neutro/masculino/feminino/abreviado) são copiados.
* **OrigemConfiguracao** (11366/11367). 1 = oficial, 2 = comunitária. `DoConverte` grava como "oficial"
  qualquer valor que não seja "comunitária".
* **TipoIdentificadorEleitor** (11356/11357). 1 = inscrição (título de eleitor), 2 = CPF, 3 = livre. As duas
  direções verificam se o valor está no intervalo (8183/8184).

---

## 5. O cadastro de eleitores e a biometria

### 5.1 Decodificação em streaming ("parcial") de `-el.dat`

O arquivo de eleitores pode conter digitais cifradas de todos os eleitores, por isso é lido com o decodificador
BER parcial do TSE (docs/libraries/asn1-runtime.md §6). O
`IConversorParcialASN<EntidadeEleitores,…>::Desconverte(const api::CFile&, shared_ptr<CVisitanteEleitor>)` inlinado
(dentro da func 11523, a `std::function` por trás de `CEleitores::GetEleitoresEstaticos`) faz quatro coisas:

1. Aloca um `CVisitanteEleitor` (64 bytes) e define como 1 a sua word em +4.
2. Registra o visitor para dois caminhos pontuados: `EntidadeEleitores.eleitores.eleitor.identificacaoEleitor.OF`
   e `EntidadeEleitores.eleitores.eleitor.biometria`.
3. Decodifica o arquivo com `api::CPartialFileASN::ReadFromFile`.
4. Chama `CConversorEntidadeEleitores::DoDesconverte(entidade, *visitor)` (func 11411).

`CVisitanteEleitor::visit` (11407) trata cada caminho registrado:

* `…identificacaoEleitor.OF`: armazena o número em uma de três strings (inscrição / CPF / livre),
  selecionada pelo id do CHOICE, e retorna 0 (manter).
* `…biometria`: armazena os offsets de arquivo `[begin, end)` do elemento. Depois, para cada identidade
  armazenada não vazia, chama `CValidadorIdentidade::Valida(numero, tipo)` e faz
  `map.emplace(identidade, {begin, end})`. Retorna 1, de modo que o blob decodificado é liberado por `ASN1::DataFree`.
* Qualquer outro caminho lança `7905 "Entidade visitada inválida: " + path`.

### 5.2 `CConversorEntidadeEleitores::DoDesconverte` (11411)

Para cada `EleitorSequencia`:

| etapa | o que o código faz |
|---|---|
| identidades | `GetInscricoesEleitor(EleitorSequencia)` (uma função de nível de namespace que recebe a SEQUENCE **por valor**). Lança 7899 se não houver identidade. Monta `CEleitorIdentidade {numero, tipo}` com `CValidadorIdentidade::Valida` (tipo 1/2/3 a partir do id do CHOICE). Rejeita duplicatas com `std::adjacent_find`, então **apenas duplicatas consecutivas** são detectadas (7900) |
| necessidade | `ConverteNecessidadeEspecial(EleitorUrna)` (de novo por valor): semNecessidade → 0, necessitaAudio → 1, caso contrário 7898 |
| nomes | `nome` e o `nomeSocial` opcional são **cortados em 40 caracteres**. O schema permite 70 |
| TTE | `DesconverteTipoTransferenciaTemporaria`: votoEmTransito, servidorEmServico, eleitorConvocado e justicaEleitoral são todos mapeados para o **valor md 1**. presoProvisorio → 2, acessibilidade → 3, ofício → 4, indígena/quilombola → 5, situação de rua → 6. Ausente **ou qualquer valor desconhecido** → 0 (sem transferência). Apenas o valor −1 lança erro (7901). Se houver transferência, o eleitor deve ter um `domicilio` (7902). `CTransferenciaTemporaria` guarda a UF e o código de município do domicílio |
| data de nascimento | `md::CDataJE` exige exatamente 8 dígitos ASCII (8002) |
| biometria | a primeira identidade do eleitor encontrada no mapa do visitor fornece os offsets. O eleitor é construído com os offsets **apenas se** o arquivo indicar que `biometria` está presente **e** um offset tiver sido encontrado. Caso contrário, a biometria é descartada silenciosamente |
| chave | se `seguranca` estiver presente, `Seguranca.idArquivoChave` é copiado para cada eleitor com biometria. O registro Seguranca é convertido duas vezes |

Nada mais é lido de `EleitorUrna`: `nomeMae`, `situacao` e a zona, a seção e as datas do domicílio são
todos ignorados. `md::CEleitor` tem 104 bytes. Seu layout está no header
`cconversorentidadeeleitores.h`. Os cenários de treinamento contêm **um eleitor** cada, sem biometria, sem
seguranca e sem TTE.

### 5.3 Impedidos (`-imp.dat`, 11410)

Para cada `EleitorImpedido`, o código converte `impedimentoP1` (`TipoImpedimento` 1..15 → 0..14, caso contrário
7904), depois a identidade (7903 para um id de CHOICE desconhecido), depois o `impedimentoP2` opcional.
`md::CImpedido` armazena `p2 = 15` e `temP2 = false` quando P2 está ausente (construtores 5661/5662). Exemplo
do cenário `geral-t1`: `{numeroInscricao XXXXXXXXXXXX, P1 semImpedimento, P2 suspenso}`.

### 5.4 Biometria cifrada (11419), passo a passo

1. `LeChave()` (inlinada, linhas 91/101/109) lê a chave:
   * caminho = diretório de chaves de `CPath` `"/dsk/fi/estatico/chave/"` + `"bio.sk1"`. Se o arquivo não existir, lança
     `7894 "O arquivo … não existe"`.
   * O arquivo é decodificado como `ModuloEnvelopeChave::EntidadeChave`, e o seu OCTET STRING `chave` é extraído.
     **`cifrado` e `tipo` são ignorados**.
   * `api::CPolySingletonList::instance<api::IKernelHSM>()` (linha de srcloc 101) → o slot 3 da vtable do HSM retorna um
     segredo. Esse segredo é transformado em uma `std::string`, depois `CSymmetricCipherFactory` constrói uma cifra,
     e o slot 3 da cifra decifra a chave. Um resultado vazio lança `7895 "O arquivo … está vazio"`.
2. `CCipheredIn(0, 1, 0, chaveSessao = CParametro+16, chaveSecreta, conteudo, CInfoSalt(salt, CParametro+0))`.
   Seu construtor recusa chave de sessão, chave secreta ou conteúdo vazios (ESecurityError 1501/1502/1503).
3. `CCepescCipher::Decifra` (func 5171) → texto claro.
4. `api::CFileASN::DecodeObject<ModuloEleitores::BiometriaEleitor>` ("DecodeObject de " + nome do tipo). Gera
   EUeIoError 5953 se a decodificação falhar e 5954 se o resultado for inválido.
5. `CConversorBiometriaEleitor` (11422): se não houver nem foto nem dedos, lança 7893 ("…mas falou
   que tem biometria"). Caso contrário, converte os dedos com `CConversorDedo` e a foto com
   `ecourna CConversorFoto`, e então constrói `CBiometriaEleitor(foto[, dedos])` ou `CBiometriaEleitor(dedos)`.

`composicaoBiometria` nunca é conferido com o que está de fato presente.

### 5.5 Formato do template de impressão digital (`CConversorDedo`, 11416 + 5702)

`Dedo ::= { tipo TipoDedo, qtdMinucias INTEGER, minucias OCTET STRING }`. O OCTET STRING é compactado em bits.
Os bits são lidos a partir do mais significativo por meio de `ecourna::api::util::CBitArray::GetValor(n)`:

```
xBase:10  bitsX:4  yBase:10  bitsY:4  qtdGrupos:8
repeat qtdGrupos:  angulo:9  quantidade:8  repeat quantidade: dx:bitsX  dy:bitsY
minutia = { x = xBase + dx, y = yBase + dy, t = angulo, 0 }   (api::SXYT, 16 bytes)
```

`DoDesconverte` desempacota o template, verifica `qtdMinucias == number of minutiae` (7897), mapeia `tipo` 1..10
(`naoIdentificado(0)` é recusado, 7896) e **desempacota o template uma segunda vez** para `md::CDedo`. O
construtor de `CDedo` limita um template a 1..200 minúcias (8061/8062). O objeto helper do desempacotador também
calcula a bounding box e as larguras em bits `ceil(log2(max-min))`. Elas seriam necessárias para reempacotar um
template, e não são usadas aqui.

### 5.6 Índice de fotos dos candidatos (`CVisitanteFoto`, 11442)

Um sequenciador de dois estados percorre `EntidadeFotosCandidatos.fotos.OF` (registra o intervalo `[begin, end)` do
`FotoCandidato`) e `EntidadeFotosCandidatos.fotos.codigoCandidato` (o código, obtido via
`dynamic_cast<AbstractString*>` e cortado no primeiro NUL). Cada par vira uma entrada de índice
`{codigo, {begin, end}}`. Visitar fora de ordem lança 7883/7884. As fotos em si são liberadas
(`DataFree`) e lidas depois por offset. Este caminho rodou durante o voto registrado da eleição geral (a func
11442 está no profile).

---

## 6. Os arquivos de estado da urna (eg.bin, vota.bin, gap.bin, sa.bin)

`comum::IServicoEstado<CEstadoX, CConversorX>` lê e grava esses arquivos. O profile de runtime mostra
`DoConverte` dos conversores eg/gap/vota (11397, 11394, 11390) e `DoDesconverte` de eg/gap (11398, 11395)
rodando durante `votaInit`. O conversor do SA e `CConversorEstadoGeralVota::DoDesconverte` não foram amostrados,
o que não prova que não sejam usados: `sa.bin` é gravado na inicialização. Os enums md são **códigos de caractere**,
enquanto os enums ASN.1 são inteiros. Cada conversor os mapeia explicitamente e recusa os valores sentinela:

| arquivo / campo | ASN.1 | md | recusado |
|---|---|---|---|
| eg.bin `estadoUrna` | carregando 1, carregada 2, testada 3 | `'1' '2' '3'` | md `'4'` (linha 106), ASN −1 |
| eg.bin `dadoCarga.turno` | semTurno 0, turno1, turno2 | `'0'..'2'` (`Utils::ConverteTurno`, util.cpp:566/581, EUeComumAsnError 7689/7690) | ≥ 3 |
| eg.bin `tipoUrnaT1/T2` | semtipo 0 … contingenciavotarecupera 4 | `'0'..'4'` | outros |
| eg.bin `modelo` | ue2013 13, ue2015 15, ue2020 20, ue2022 22 | **ano** do modelo 2013/2015/2020/2022 | `tpm20` e −1 "não suportado" (7915), outros (7916) |
| eg.bin `ajusteDataHora.tipo` | semAjuste 1, alterarDataEleicao 2, alterarDataSistema 3 | `ETipoDeltaT` 0..2 | |
| vota.bin `estadoVota` | inicial 0 … encerrada 15, exibealertadesligamento 16, ultimoid 17 | `'1'..'@'` (`'1'`+v) | 16: **"Estado não deve ser usado"** (7931); 17/−1 (7932); md `'A'` (7929) |
| vota.bin `estadoEncerramento` | inicial 0, imprimirobrigatoriabu 1, retirarmr 2, fimdostrabalhos 3, ultimoid 4 | `'1'..'4'` | ultimoid/−1 "não suportado" (7936); md `'5'` (7934) |
| gap.bin `appId/appAnteriorId` | 0..14, apultimoid 15 | os mesmos números | 15 e −1 |
| sa.bin `estadoSA` | inicial 0 … encerrada 22, ultimoid 23 | `'1'..'G'` | ultimoid/−1; md `'H'` |

O que é mantido (ver os headers para os layouts em bytes):

* **eg.bin**. O `CEstadoGeral` md mantém todos os campos. No build web, `eg.bin` é uma fixture:
  `idPE 2400, testada, turno1, vota/vota, ue2015, treinamento, AC 1/1/1, versão "7.2.1.3 - TESTE EG ASN1"`
  (decodificado de `analysis/runtime/memfs-after-init`).
* **vota.bin**. qtdBU (truncado para 8 bits), comparecimento e qtdJustificativa (truncados para 16 bits), os
  contadores de vias impressas (4 × uebyte), as flags e cinco campos opcionais (`urnaIdGerouZeresima`,
  `dhIniAquisicao`, `dhFimAquisicao`, `dhUltimoVoto`, `dhEmissao`) mantidos como `std::optional`. Após
  `votaInit`, ele decodifica para `{votar, inicial, qtdBU 0, comparecimento 0, …, treinamentoEleitor TRUE,
  dhIniAquisicao "<now>"}`. **Ele é idêntico após um voto**: o build web não atualiza o comparecimento.
* **gap.bin**. Até 10 correspondências de carga, aplicação atual e anterior, 4 flags, contadores de vias
  para os turnos 1 e 2, data opcional do 2º turno. O construtor md (5626) **sobrescreve `data2T`** com
  "a data do 2º turno está presente e é hoje ou anterior" (`CDate::Hoje()` do singleton do relógio). A
  flag armazenada no arquivo, portanto, nunca é usada. Valor observado: `appId = apsemaplicativo`,
  `dataSegundoTurno "20801231"`.
* **sa.bin**. Estado, `atualizacaoBloqueada`, município/zona/seção. Observado: `{inicial, FALSE, 1/1/2}`.

---

## 7. O corpo do RDV (`asn/rdv/`), passo a passo

O arquivo do RDV encapsula `EntidadeRegistroDigitalVoto { pleito, fase, identificacao, historicoCodigosCarga,
eleicoes }`. O wrapper é `CConversorRegistroDigitalVoto<CConversorEleicoesVota>`, em outra unidade. Ele
possui um `CConversorEleicoesVota` em +4, construído **movendo para dentro** um `std::map<TEleicaoID, std::vector<md::CCargo>>`
(os cargos configurados para cada eleição). Esta unidade converte `eleicoes`.

**md → ASN.1 (`CConversorEleicoesVota::DoConverte`, 11350)**

1. O número de eleições em `md::CVotosEleicoesVota` deve ser igual ao número configurado. Caso contrário, lança
   `7965 "Eleições incompatíveis"`.
2. Seleciona `Eleicoes.eleicoesVota` (CHOICE 0, tag `[1]`). `eleicoesSA` é a alternativa do SA.
3. Para cada `(idEleicao, CVotosCargos)` em ordem crescente de id (std::map):
   `EleicaoVota { idEleicao, votosCargos }`. Os cargos configurados são buscados (`7966 "Eleição N não
   encontrada"`, montada com `+` e não com format).
4. Para cada cargo **configurado**, na ordem da configuração:
   `SCargoInfo {codigo, tipo, numeroDigitos, qtdeEscolhas}` é copiado de `md::CCargo` (+0, +4, +12, +13).
   Os votos vêm de `CVotosCargos::GetVotos(codigo)`, que é inlinada e lança
   `CBaseError<api::EUeRdvError>(4671, "Cargo N nao encontrado")`. O resto é feito por
   `CConversorVotosCargo(info).Converte({info, votos})`.
5. `CConversorVotosCargo::DoConverte` (11352) recusa um par cujo código de cargo difere do seu próprio (`7976
   "Cargo info incompatível"`). Ele grava `idCargo` (via ecourna `CConversorCargoID`, `CBaseType<ushort,1,99,3>`),
   `quantidadeEscolhas = qtdeEscolhas`, depois um `Voto` por `md::CVoto`.
6. `CConversorVoto::DoConverte` (11355) grava `tipoVoto` a partir de `CVoto::ETipo`. Os dois compartilham a
   numeração: 1 legenda, 2 nominal, 3 branco, 4 nulo, 5 brancoAposSuspensao, 6 nuloAposSuspensao,
   7 nuloPorRepeticao, 8 nuloCargoSemCandidato, 9 nuloAposSuspensaoCargoSemCandidato. Qualquer outro valor lança
   7973. `digitacao` (os dígitos como foram digitados, NumericString 1..5) é **omitido quando vazio**.
7. Cada chamada a `Converte` revalida o seu produto (`isValid && isStrictlyValid`, 7653). Um voto com 6 dígitos
   digitados seria, portanto, recusado aqui.

**ASN.1 → md (`DoDesconverte`, 11349)**: a escolha deve ser `eleicoesVota` (`7967 "Dado não é do Vota"`),
e as contagens devem coincidir (7968). A eleição (7969) e cada código de cargo (7970 "Cargo {} não encontrado
para eleição {}") devem existir na configuração. `CConversorVotosCargo::DoDesconverte` (11351) verifica
o cargo (7977 "Cargo {} difere do esperado: {}") e `quantidadeEscolhas` (7978 "Quantidade de escolhas
difere do esperado: {}/{}"). Os votos são decodificados com `CConversorVoto::DoDesconverte` (11354). Uma
`digitacao` ausente vira "". Os resultados vão para `std::map<SCargoInfo, CVotos>`, onde um **cargo duplicado é
ignorado silenciosamente**, depois `CVotosCargos(TMapa)`, depois `CVotosEleicoesVota(TMapa)`. Ambos recebem o
mapa **por valor**. Uma **eleição duplicada** é ignorada silenciosamente da mesma forma (inserção de chave única em
`std::map<TEleicaoID, CVotosCargos>`). Como 7968 compara apenas as contagens, um RDV `{X, X}` verificado contra
uma configuração `{X, Y}` é decodificado sem erro em um resultado que não tem entrada para `Y`.

**No build web**, este código rodou apenas durante `votaInit`: `CGeraDadosDinamicos` → slot 11 de `CRdvVota` →
`CFileASN::CodeObjectFunction` → `IConversorASN<Eleicoes>::Converte` → 11350 → 11352. Ele serializa um
RDV vazio no `rdv.dat` cifrado de 80 bytes. Após um voto, o RDV não é regravado
(`analysis/runtime/README.md`).

### Relação com o BU e o encerramento

Esta unidade **não** monta o BU. Ela fornece peças das quais o fluxo de BU/encerramento (unidades u05, u06,
u21, u23, u27…) depende:

* o formato do corpo do RDV e suas verificações de consistência (§7);
* campos de `vota.bin` que conduzem o encerramento: `estadoVota` (`gerarbu → gerarrelatorios → imprimirbu →
  gravarresultados → copiaresultadosmr → encerrada`), `estadoEncerramento` (`imprimirobrigatoriabu →
  retirarmr → fimdostrabalhos`), `qtdBU` (número de BUs impressos), `urnaIdGerouZeresima`, `dhEmissao` (hora de
  emissão do BU) e `numViasImpressasRelatorios` (vias já impressas);
* os rótulos das tabelas impressas do BU/zerésima (`CCargoDSLabelRelatorio`, §8).

---

## 8. Fontes de texto de cargo e candidatura (`ccargods.cpp`, `ccandidaturas.cpp`)

* `GetNomeCargoComGenero(contexto, sexo, suplente)` (5803) retorna o nome do cargo no gênero do
  candidato atual (`CCargo::GetNome(sexo)`), ou o nome do cargo do suplente. `contexto` só é usado na
  mensagem de erro; os chamadores passam o seu próprio nome de função (pretty function).
* `CCargoDSNomeSexoCandidato{suplente, abreviaNomeLongo}()` (2268) exige que os dois cursores estejam posicionados
  e concordem: `"O candidato não foi posicionado corretamente"` (7808), 7809, e `"Cargo e candidato não
  foram posicionados corretamente"` (7810). Ele obtém o sexo do candidato de `CCandidaturasDSSexo`. Se
  `abreviaNomeLongo` estiver ativado e o nome tiver ≥ 19 caracteres, ele retorna o nome abreviado em seu lugar.
  **Confirmado no simulador em execução** (`--keys "91001C  C  91    " --draw`): a tela de confirmação de
  prefeito 91 desenha `"Prefeita"` (a candidata é mulher), `"Vice-Prefeito"` (func 6664) e
  `"Vice-Prefeito: Judô"` (func 6669).
* `CCargoDSLabelRelatorio::HeaderDetalhe/HeaderDetalheZE/TotalVotoNominal` retornam os cabeçalhos de coluna do relatório
  e a linha de totais. Eles alternam entre candidatos e respostas de consulta:
  `"Nome do candidato       Num cand Votos"` / `"Resposta                Num resp Votos"`, e
  `"Total de votos Nominais           {:04}\n"` / `"Total de votos Válidos            {:04}\n"`
  (literais Latin-1, padrões de formatação para o chamador).
* `CCandidaturas`: singleton de 44 bytes (`unique_ptr` estático @0x1C0E64). É um `api::CDataMap<md::CCandidatura>`
  chamado `"CCandidaturas"` mais um `map<TCargoID, string>` de versões de pacote (`RecuperaVersaoPacote`, 7801).
  `GetCandidaturaAtual(contexto)` retorna a candidatura atual ou lança `"{} - não posicionado no
  candidato corretamente"` (7805).

---

## 9. Particularidades do build web

* **O CEPESC é uma passagem direta.** `CCepescCipher::Decifra` (5171) retorna o conteúdo de entrada sem alteração. Sua
  irmã `Cifra` (2681, unidade u40) envolve o texto claro com uma chave de sessão zerada de 32 bytes. A
  "cifragem" da biometria dos eleitores seria a identidade neste build. Nenhum cenário traz `bio.sk1` nem eleitores com
  biometria (não há diretório `chave/` em nenhum cenário), então o §5.4 nunca roda no simulador. Nenhuma classe na RTTI do binário deriva de `api::IKernelHSM`, então, neste caminho, a
  busca em `CPolySingletonList` muito provavelmente lançaria "solicitada uma instância não criada" antes de qualquer decifragem.
* **Os arquivos de estado são fixtures ou estão congelados.** `eg.bin` vem de um `CEstadoGeral` hard-coded (mock_f10205
  chama o ctor 5637). Os construtores de GAP/SA/VOTA 5625/5626/5628 também são chamados por fixtures do simulador
  (mock_f10101/10123/10067). `vota.bin` é gravado uma vez na inicialização e não é atualizado após os votos.
* **A conversão do RDV roda apenas na inicialização** (RDV vazio); ver §7.

---

## 10. Observações sobre wasm / Emscripten

* **Os helpers que não lançam exceção de todos os conversores foram inlinados** em `DoConverte`/`DoDesconverte`, então o
  nome baseado em srcloc é o do helper (p. ex., "ConverteEstadoVota"). O slot da vtable dá o método real.
  Alguns helpers ficaram fora de linha (5689/5690/5693/5694/5699) porque são chamados duas vezes.
* **merge-similar-functions** criou corpos sem equivalente no código-fonte:
  * 6030: compartilhado por `CConversorOrigemConfiguracao::DoDesconverte` e `CConversorTipoIdentificadorEleitor::DoDesconverte`. Os parâmetros são srcloc, mensagem, código e intervalo.
  * 6038: `HeaderDetalhe`/`HeaderDetalheZE`. Os parâmetros são srcloc, código e 10 ponteiros para os dois literais de 38 caracteres, porque a cópia é feita em 5 leituras sobrepostas de 8 bytes.
  * 6075 e 6076: as quatro lambdas de fonte de texto de suplente.
* **Lambdas sem captura como slots de tabela.** 6664 ocupa os slots 1091 e 1095 (duas lambdas idênticas unificadas pelo ICF); 6669, os slots 1090 e 1092; 13301, o 1093; 13298, o 1094. Elas são passadas por `adicionaBaseTelaCompletaCandidatoCom1` (6671) e `…Com2` (6660). As fontes "cargo: nome" (slots 1090/1092/1093) vão para o construtor de linha de texto `vota_f3068`. As fontes "apenas cargo" (slots 1091/1094/1095) vão para `adicionaFotoEmoldurada` (4181) como legenda sob a foto do suplente. A execução com `--draw` do §8 mostra as duas: `"Vice-Prefeito: Judô"` em uma linha de texto em (20, 561) e `"Vice-Prefeito"` sob a foto em (1170, 759).
* **Formatação de enums.** `std::format("… {}", enum)` passa pelas lambdas `__handle` 536/1699/1711 (um
  `std::formatter` do TSE para enums e tipos ENUMERATED do ASN.1). Os enums de caractere, portanto, são impressos como números (p. ex.,
  `'4'` é impresso como 52). O tipo de argumento empacotado indica qual caso se aplica (15 = handle, 3 = int, 6 = unsigned).
  Alguns pontos formatam um `int` simples em vez do enum: 7909 (`EUrnaTipo`, 5693) e 8185
  (`ETipoIdentificadorEleitor`, 11363), além de todo `asInt()` de um switch ASN→md (7898, 7901, 7904, 7906/7907,
  7974/7975). Os erros de modelo de DadoCarga 7915/7916 formatam o próprio objeto ENUMERATED (handle).
* **O misterioso caso −1.** Todo switch de enum ASN→md tem um caso explícito para **−1**, lançado de uma linha
  *dentro* do switch e separado do throw default que vem depois dele. Os switches md→ASN, em vez disso, tratam de forma
  especial o seu próprio sentinela (`'4'`, `'A'`, `'H'`, 15…). A explicação mais simples é um enumerador
  "inválido" extra (valor −1) em cada `NamedNumber` gerado. Seu nome não está no binário.
* **Layouts de struct visíveis.** Os objetos md são passados como words empacotadas: `CCargoDSNomeSexoCandidato` é uma
  word de 16 bits (257 = `{1, true}`), `CNumViasImpressasRelatorios` ocupa 4 bytes desalinhados em +73 de
  `CEstadoGeralVota`, e `api::CDate` são 4 shorts comparados pela func 1261.
* As comparações de caminho do decodificador parcial são `memcmp` inlinados de words de 8 bytes (XOR contra constantes).
  `std::string(const char*)` a partir de `c_str()` vira `strlen` + cópia.
* O literal de 16 bytes `"DecodeObject de "` em @443005 foi decodificado errado pela ferramenta como `"duplicado …"`,
  por causa de um offset de +1024 no segmento de dados nas referências `d_…[N]` do pseudocódigo. Leia as constantes brutas a partir
  do `.wat`.

---

## 11. Código estranho ou arriscado

A mesma lista é devolvida ao orquestrador como `suspicious`:

1. **A decifragem CEPESC é a identidade (5171)** e `LeChave` ignora a flag `cifrado` do envelope. Ver §9.
2. **Identidades obsoletas em `CVisitanteEleitor`**. As três strings de identidade nunca são limpas entre
   eleitores. Um eleitor sem CPF recebe o CPF do eleitor anterior mapeado para o offset da *sua* biometria. O
   `emplace` mantém o primeiro mapeamento, então isso só cria entradas espúrias para identidades que não tinham
   biometria própria. Nesta unidade, as entradas espúrias nunca são usadas: 11411 aplica uma posição apenas quando
   o campo `biometria` do próprio eleitor está presente, e as identidades de um eleitor assim já foram mapeadas para o
   seu próprio offset. O bug só importaria se o mapa fosse reutilizado em outro lugar, por exemplo para uma busca por CPF.
3. **A verificação de identidade duplicada só detecta duplicatas adjacentes** (`adjacent_find` em uma lista não ordenada).
4. **Valores de TTE desconhecidos são aceitos silenciosamente como "sem transferência"**. Apenas −1 lança erro. Um tipo de TTE
   futuro enviado a uma urna antiga é lido como um eleitor comum.
5. **Nomes cortados em 40 caracteres.** O schema ASN.1 permite 70, então nomes longos são cortados na tela e nos
   relatórios.
6. **Conversões com truncamento.** `qtdBU` (0..999) é armazenado em 8 bits, `comparecimento`/`qtdJustificativa`
   (0..99999) em 16 bits, e os contadores de vias (0..999) em 8 bits cada.
7. **`data2T` de gap.bin é ignorado.** O construtor md o recalcula a partir da data de hoje. Em uma máquina
   com o relógio errado, a flag de "2º turno" segue o relógio.
8. **O id do processo eleitoral é lido do CHOICE IDEleitoral sem verificar a alternativa.**
9. **Cargo duplicado em um RDV é ignorado silenciosamente** na leitura (`map::insert`). Uma eleição duplicada também é
   ignorada silenciosamente e, junto com a verificação apenas de contagem (7968), uma eleição configurada pode faltar no resultado (§7).
10. **Cópias pesadas**: `GetInscricoesEleitor` e `ConverteNecessidadeEspecial` copiam a
    `EleitorSequencia`/`EleitorUrna` inteira por eleitor; cada template de digital é desempacotado duas vezes; o registro
    Seguranca é convertido duas vezes; mapas são copiados para construtores que os recebem por valor.
11. **Cálculo de largura em bits** `ceil(log2(max-min))` fica um bit abaixo quando `max-min` é uma potência de dois, e
    dá 0 para um intervalo de 1. Não é usado neste caminho, mas está errado se for reutilizado para reempacotar.
12. **Temporário morto** em `CConversorBiometriaEleitor::DoDesconverte`: uma `SEQUENCE OF Dedo` vazia é
    construída e destruída.

Nenhum acesso à rede, nenhum `emscripten_sleep`, nenhuma interação com JS e nenhum laço sem limite foram encontrados nesta unidade.
Todo laço é limitado pelo tamanho de contêineres, e todo índice é verificado com `at()`.

---

## 12. Códigos de erro lançados por esta unidade

`CBaseError<comum::EUeComumDadosError, SErrorLimits{7800, 8600}>`, salvo indicação em contrário. Os números de linha são os
originais.

| código | mensagem | onde |
|---|---|---|
| 7801 | Versão de pacote não encontrada para o cargo {} | ccandidaturas.cpp:89 |
| 7805 | {} - não posicionado no candidato corretamente | ccandidaturas.cpp:261 |
| 7806 / 7807 | O cargo não foi posicionado corretamente para {} | ccargods.cpp:25 / :41 |
| 7808 / 7809 / 7810 | O candidato… / O cargo… / Cargo e candidato não foram posicionados corretamente | ccargods.cpp:65/72/78 |
| 7811 / 7812 / 7813 | O cargo não foi posicionado corretamente | ccargods.cpp:98/113/128 |
| 7883–7886 | sequenciador de fotos / "Erro ao carregar codigo do candidato." / "Entidade visitada invalida " | cvisitantefoto.cpp:40/48/56/63 |
| 7887–7890 | Local: CHOICE / tipo indefinido / não válida / não estritamente válida | cconversorlocal.cpp:70/120/124/128 |
| 7893 | Não tem nem foto nem dedos, mas falou que tem biometria. | cconversorbiometriaeleitor.cpp:26 |
| 7894 / 7895 | O arquivo … não existe / está vazio | cconversorbiometriaeleitorcifrada.cpp:91/109 |
| 7896 / 7897 | Tipo inválido: {} / Campo qtdMinucias não confere… | cconversordedo.cpp:159/169 |
| 7898–7902 | necessidade / inscrição não encontrada / identidade duplicada / transferência / sem domicílio | cconversorentidadeeleitores.cpp:38/49/76/210/236 |
| 7903 / 7904 | Tipo desconhecido de identidade / Tipo de impedimento inválido: {} | cconversorimpedido.cpp:40/109 |
| 7905 | Entidade visitada inválida: … | cvisitanteeleitor.cpp:91 |
| 7906–7908 | TipoAjusteDataHora / ETipoDeltaT | cconversorajustedatahora.cpp:49/53/69 |
| 7909, 7911–7913, 7915–7916 | EUrnaTipo / TipoUrnaOperacao / EUrnaModelo / ModeloUrna | cconversordadocarga.cpp:61/84/88/105/127/131 |
| 7917–7920 | estado da urna | cconversorestadogeral.cpp:87/91/106/110 |
| 7921–7924 | UrnaAplicativo / EUrnaAplicativo | cconversorestadogeralgap.cpp:150/154/194/198 |
| 7925–7928 | EEstadoSA / EstadoSA | cconversorestadogeralsa.cpp:96/100/156/160 |
| 7929–7937 | EEstadoVota / EstadoVota ("Estado não deve ser usado") / (E)EstadoEncerramento | cconversorestadogeralvota.cpp:152…252 |
| 7944 | Quantidade inválida de UFs [{}] | cconversorcomplementosmunicipios.cpp:29 |
| 7948 / 7949 | Opção de Detalhe Inválida. / Tipo inválido: {} | cconversorcargo.cpp:69/83 |
| 7950–7952 | suplências | cconversordetalhecandidato.cpp:29/34/39 |
| 7953 | Origem configuração inválida | cconversororigemconfiguracao.cpp:38 |
| 7965–7970 | Eleições incompatíveis / Eleição … não encontrada / Dado não é do Vota / Cargo {} não encontrado para eleição {} | cconversoreleicoesvota.cpp:34/46/69/74/94/104 |
| 7973–7975 | Valor inválido para tipo voto eleitor: {} | cconversorvoto.cpp:44/71/75 |
| 7976–7978 | Cargo info incompatível / Cargo {} difere… / Quantidade de escolhas difere… | cconversorvotoscargo.cpp:28/50/55 |
| 8183 / 8184 / 8185 | Tipo de identificador de eleitor inválido / Identidade principal ({}) ausente… | cconversortipoidentificadoreleitor.cpp:32/51, cconversorprocessoeleitoral.cpp:50 |
| md inlinado | 8002 DataJE, 8061/8062 CDedo, 8070 CTransferenciaTemporaria, 8115/8116 CComplementosMunicipiosUF | md/*.cpp |
| outros enums | EUeComumAsnError 7653/7654 (iconversorasn.h), 7689/7690 (util.cpp Turno); EApiAsnError 1900/1902 (ecourna iconversorasn.hpp); EUeIoError 5953/5954 (cfileasn.h); ESecurityError 1501–1503 (ccipheredin.cpp); EUeRdvError 4671 (cvotoscargos.cpp:91) | |

---

## 13. Tabela de mapeamento (todas as 89 funções da u03)

"executou" = observada em execução em `analysis/runtime/vote_*.functions.tsv`. Os caminhos da 5ª coluna são relativos
a `src/uenux2/src/app/comum/dados/`.

| func | tamanho | executou | símbolo reconstruído | reconstruído em | notas |
|---:|---:|:-:|---|---|---|
| 1135 | 175 |  | `std::vector<comum::md::CSuplencia>::__base_destruct_at_end` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 2268 | 1189 | ✔ | `comum::CCargoDSNomeSexoCandidato::operator()` | `ccargods.cpp` |  |
| 2341 | 242 | ✔ | `comum::md::CSuplencia::CSuplencia(const CSuplencia&)` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/processoeleitoral/csuplencia.h` |
| 2838 | 599 |  | `comum::GetCandidaturaAtual` | `ccandidaturas.cpp` |  |
| 3018 | 231 |  | `std::vector<comum::md::CResposta>::__init_with_size` | `asn/processoeleitoral/cconversorcargo.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 3728 | 220 |  | `std::map<comum::md::CEleitorIdentidade, std::pair<size_t,size_t>>::__emplace_unique_key_args` | `asn/eleitor/cvisitanteeleitor.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 3786 | 50 |  | `comum::CCandidaturas::~CCandidaturas` | `ccandidaturas.cpp` |  |
| 3802 | 199 | ✔ | `comum::asn::Utils::ConverteDataJE` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/asn/util.cpp` |
| 3869 | 466 | ✔ | `std::map<TEleicaoID, comum::md::CVotosCargos>::__find_equal (hint)` | `asn/rdv/cconversoreleicoesvota.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5171 | 221 |  | `ecourna::api::cepesc::CCepescCipher::Decifra` | `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` | cópia out-of-line emitida nesta TU; local real `ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp` |
| 5493 | 46 | ✔ | `std::vector<unsigned char>::__vdeallocate` | `asn/eleitor/cconversordedo.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5494 | 80 | ✔ | `std::vector<unsigned char>::__construct_at_end(first,last)` | `asn/eleitor/cconversordedo.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5496 | 159 | ✔ | `std::vector<unsigned char>::__assign_with_size` | `asn/eleitor/cconversordedo.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5625 | 37 |  | `comum::md::estadoaplicacao::CEstadoGeralSA::CEstadoGeralSA` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralsa.cpp` |
| 5626 | 267 | ✔ | `comum::md::estadoaplicacao::CEstadoGeralGap::CEstadoGeralGap` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralgap.cpp` |
| 5628 | 151 |  | `comum::md::estadoaplicacao::CEstadoGeralVota::CEstadoGeralVota` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeralvota.cpp` |
| 5637 | 222 | ✔ | `comum::md::estadoaplicacao::CEstadoGeral::CEstadoGeral` | `asn/estadoaplicacao/cconversorestadogeral.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp` |
| 5659 | 65 | ✔ | `std::pair<std::string, std::pair<size_t,size_t>>::pair (CVisitanteFoto index entry)` | `asn/candidatura/cvisitantefoto.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5661 | 104 |  | `comum::md::CImpedido::CImpedido(secao, identidade, p1, p2)` | `asn/eleitor/cconversorimpedido.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/eleitor/cimpedido.h` |
| 5662 | 104 |  | `comum::md::CImpedido::CImpedido(secao, identidade, p1)` | `asn/eleitor/cconversorimpedido.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/eleitor/cimpedido.h` |
| 5666 | 477 | ✔ | `comum::md::CEleitor::CEleitor (without biometria)` | `asn/eleitor/cconversorentidadeeleitores.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 5668 | 583 |  | `comum::md::CEleitor::CEleitor (with biometria)` | `asn/eleitor/cconversorentidadeeleitores.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/eleitor/celeitor.h` |
| 5674 | 125 |  | `comum::md::CInfoMunicipio::CInfoMunicipio (with horário de verão)` | `asn/cconversorlocal.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/cinfomunicipio.h` |
| 5676 | 102 | ✔ | `comum::md::CInfoMunicipio::CInfoMunicipio (without horário de verão)` | `asn/cconversorlocal.cpp` | cópia out-of-line emitida nesta TU; local real `uenux2/src/app/comum/dados/md/cinfomunicipio.h` |
| 5683 | 44 |  | `comum::asn::CConversorVotosCargo::CConversorVotosCargo` | `asn/rdv/cconversorvotoscargo.h` |  |
| 5689 | 997 |  | `comum::asn::CConversorEstadoGeralGap::ConverteUrnaAplicativo` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 5690 | 971 |  | `comum::asn::CConversorEstadoGeralGap::DesconverteUrnaAplicativo` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 5693 | 502 |  | `comum::asn::CConversorDadoCarga::ConverteTipoUrna` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 5694 | 1005 |  | `comum::asn::CConversorDadoCarga::DesconverteTipoUrna` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 5698 | 102 | ✔ | `comum::asn::CVisitanteEleitor::~CVisitanteEleitor` | `asn/eleitor/cvisitanteeleitor.h` |  |
| 5699 | 483 |  | `comum::asn::CConversorImpedido::DesconverteTipoImpedimento` | `asn/eleitor/cconversorimpedido.cpp` |  |
| 5700 | 510 |  | `std::vector<comum::md::CEleitor>::__swap_out_circular_buffer` | `asn/eleitor/cconversorentidadeeleitores.cpp` | helper de biblioteca/inlinado (sem arquivo-fonte próprio) |
| 5702 | 2565 |  | `comum::asn::(anonymous namespace)::DescompactaMinucias` | `asn/eleitor/cconversordedo.cpp` |  |
| 5709 | 116 |  | `comum::asn::CVisitanteFoto::~CVisitanteFoto` | `asn/candidatura/cvisitantefoto.h` |  |
| 5801 | 253 |  | `comum::CCargoDSLabelRelatorio::TotalVotoNominal` | `ccargods.cpp` |  |
| 5802 | 55 |  | `comum::CCargoDSLabelRelatorio::HeaderDetalhe` | `ccargods.cpp` |  |
| 5803 | 554 | ✔ | `comum::(anonymous namespace)::GetNomeCargoComGenero` | `ccargods.cpp` |  |
| 5810 | 592 |  | `comum::CCandidaturas::RecuperaVersaoPacote` | `ccandidaturas.cpp` |  |
| 5811 | 107 |  | `comum::CCandidaturas::CCandidaturas` | `ccandidaturas.h` |  |
| 6038 | 229 |  | `wasm-opt merged body of CCargoDSLabelRelatorio::HeaderDetalhe/HeaderDetalheZE` | `ccargods.cpp` | corpo do merge-similar-functions do wasm-opt (sem equivalente no código-fonte) |
| 6075 | 37 | ✔ | `wasm-opt merged body: CCargoDSNomeSexoCandidato{packed}() text source` | `ccargods.cpp` | corpo do merge-similar-functions do wasm-opt (sem equivalente no código-fonte) |
| 6076 | 255 | ✔ | `wasm-opt merged body: cargo name + ': ' + CCandidaturasDSNome text source` | `ccargods.cpp` | corpo do merge-similar-functions do wasm-opt (sem equivalente no código-fonte) |
| 6664 | 10 | ✔ | `vota::(anonymous namespace)::<lambda: nome do cargo do 1o suplente>` | `ccargods.cpp` | lambda sem captura; local real `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferido) |
| 6669 | 12 | ✔ | `vota::(anonymous namespace)::<lambda: cargo e nome do 1o suplente>` | `ccargods.cpp` | lambda sem captura; local real `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferido) |
| 11247 | 904 |  | `api::CDataText<comum::CPadDS<comum::CToUpperDS<comum::CCargoDSNome>>>::GetText` | `ccargods.cpp` | instanciação de template (`uenux2/src/api/gui/ctextsource.h` + `ccargods.h`) |
| 11347 | 20 |  | `comum::asn::CConversorEleicoesVota::~CConversorEleicoesVota (deleting)` | `asn/rdv/cconversoreleicoesvota.h` |  |
| 11348 | 17 |  | `comum::asn::CConversorEleicoesVota::~CConversorEleicoesVota` | `asn/rdv/cconversoreleicoesvota.h` |  |
| 11349 | 5054 |  | `comum::asn::CConversorEleicoesVota::DoDesconverte` | `asn/rdv/cconversoreleicoesvota.cpp` |  |
| 11350 | 1911 | ✔ | `comum::asn::CConversorEleicoesVota::DoConverte` | `asn/rdv/cconversoreleicoesvota.cpp` |  |
| 11351 | 2825 |  | `comum::asn::CConversorVotosCargo::DoDesconverte` | `asn/rdv/cconversorvotoscargo.cpp` |  |
| 11352 | 980 | ✔ | `comum::asn::CConversorVotosCargo::DoConverte` | `asn/rdv/cconversorvotoscargo.cpp` |  |
| 11354 | 1175 |  | `comum::asn::CConversorVoto::DoDesconverte` | `asn/rdv/cconversorvoto.cpp` |  |
| 11355 | 745 |  | `comum::asn::CConversorVoto::DoConverte` | `asn/rdv/cconversorvoto.cpp` |  |
| 11356 | 23 |  | `comum::asn::CConversorTipoIdentificadorEleitor::DoDesconverte` | `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` |  |
| 11357 | 100 |  | `comum::asn::CConversorTipoIdentificadorEleitor::DoConverte` | `asn/processoeleitoral/cconversortipoidentificadoreleitor.cpp` |  |
| 11363 | 2456 | ✔ | `comum::asn::CConversorProcessoEleitoral::DoDesconverte` | `asn/processoeleitoral/cconversorprocessoeleitoral.cpp` |  |
| 11366 | 23 |  | `comum::asn::CConversorOrigemConfiguracao::DoDesconverte` | `asn/processoeleitoral/cconversororigemconfiguracao.cpp` |  |
| 11367 | 44 |  | `comum::asn::CConversorOrigemConfiguracao::DoConverte` | `asn/processoeleitoral/cconversororigemconfiguracao.cpp` |  |
| 11373 | 2358 | ✔ | `comum::asn::CConversorCargo::DoDesconverte` | `asn/processoeleitoral/cconversorcargo.cpp` |  |
| 11375 | 2626 | ✔ | `comum::asn::CConversorDetalheCandidato::DoDesconverte` | `asn/processoeleitoral/cconversordetalhecandidato.cpp` |  |
| 11381 | 1912 |  | `comum::asn::CConversorComplementosMunicipios::DoDesconverte` | `asn/municipiozona/cconversorcomplementosmunicipios.cpp` |  |
| 11385 | 391 |  | `comum::asn::CConversorHorarioVerao::DoConverte` | `asn/municipiozona/cconversorhorarioverao.cpp` |  |
| 11390 | 3335 | ✔ | `comum::asn::CConversorEstadoGeralVota::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` |  |
| 11391 | 3356 |  | `comum::asn::CConversorEstadoGeralVota::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralvota.cpp` |  |
| 11392 | 1362 |  | `comum::asn::CConversorEstadoGeralSA::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` |  |
| 11393 | 1254 |  | `comum::asn::CConversorEstadoGeralSA::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralsa.cpp` |  |
| 11394 | 1109 | ✔ | `comum::asn::CConversorEstadoGeralGap::DoConverte` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 11395 | 1421 | ✔ | `comum::asn::CConversorEstadoGeralGap::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeralgap.cpp` |  |
| 11397 | 3182 | ✔ | `comum::asn::CConversorEstadoGeral::DoConverte` | `asn/estadoaplicacao/cconversorestadogeral.cpp` |  |
| 11398 | 3672 | ✔ | `comum::asn::CConversorEstadoGeral::DoDesconverte` | `asn/estadoaplicacao/cconversorestadogeral.cpp` |  |
| 11401 | 1227 | ✔ | `comum::asn::CConversorDadoCarga::DoConverte` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 11402 | 1603 | ✔ | `comum::asn::CConversorDadoCarga::DoDesconverte` | `asn/estadoaplicacao/cconversordadocarga.cpp` |  |
| 11405 | 594 |  | `comum::asn::CConversorAjusteDataHora::DoConverte` | `asn/estadoaplicacao/cconversorajustedatahora.cpp` |  |
| 11406 | 999 |  | `comum::asn::CConversorAjusteDataHora::DoDesconverte` | `asn/estadoaplicacao/cconversorajustedatahora.cpp` |  |
| 11407 | 1313 |  | `comum::asn::CVisitanteEleitor::visit` | `asn/eleitor/cvisitanteeleitor.cpp` |  |
| 11408 | 13 | ✔ | `comum::asn::CVisitanteEleitor::~CVisitanteEleitor (deleting)` | `asn/eleitor/cvisitanteeleitor.h` |  |
| 11410 | 1444 | ✔ | `comum::asn::CConversorImpedido::DoDesconverte` | `asn/eleitor/cconversorimpedido.cpp` |  |
| 11411 | 7299 | ✔ | `comum::asn::CConversorEntidadeEleitores::DoDesconverte` | `asn/eleitor/cconversorentidadeeleitores.cpp` |  |
| 11416 | 1778 |  | `comum::asn::CConversorDedo::DoDesconverte` | `asn/eleitor/cconversordedo.cpp` |  |
| 11419 | 5935 |  | `comum::asn::CConversorBiometriaEleitorCifrada::DoDesconverte` | `asn/eleitor/cconversorbiometriaeleitorcifrada.cpp` |  |
| 11422 | 2685 |  | `comum::asn::CConversorBiometriaEleitor::DoDesconverte` | `asn/eleitor/cconversorbiometriaeleitor.cpp` |  |
| 11442 | 1198 | ✔ | `comum::asn::CVisitanteFoto::visit` | `asn/candidatura/cvisitantefoto.cpp` |  |
| 11443 | 13 |  | `comum::asn::CVisitanteFoto::~CVisitanteFoto (deleting)` | `asn/candidatura/cvisitantefoto.h` |  |
| 11451 | 1097 |  | `comum::asn::CConversorLocal::DoConverte` | `asn/cconversorlocal.cpp` |  |
| 11452 | 1671 | ✔ | `comum::asn::CConversorLocal::DoDesconverte` | `asn/cconversorlocal.cpp` |  |
| 11555 | 55 |  | `comum::CCargoDSLabelRelatorio::HeaderDetalheZE` | `ccargods.cpp` |  |
| 12632 | 12 |  | `api::CDataText<comum::CCargoDSNomeSexoCandidato>::GetText` | `ccargods.cpp` | instanciação de template (`uenux2/src/api/gui/ctextsource.h` + `ccargods.h`) |
| 13298 | 10 |  | `vota::(anonymous namespace)::<lambda: nome do cargo do 2o suplente>` | `ccargods.cpp` | lambda sem captura; local real `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferido) |
| 13301 | 12 |  | `vota::(anonymous namespace)::<lambda: cargo e nome do 2o suplente>` | `ccargods.cpp` | lambda sem captura; local real `uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp` (inferido) |

---

## 14. Questões em aberto

* O nome e o significado do enumerador −1 tratado em todo switch ASN→md (§10).
* Os nomes dos enumeradores md (`EEstadoVota`, `ETransferenciaTemporaria`, `ETipoDeltaT`…) e a
  semântica do valor 1 de TTE no md. Esse valor agrupa em trânsito, servidor em serviço, eleitor convocado e
  justiça eleitoral.
* O significado exato de `EVisitorResult` 0/1 (manter vs. liberar é inferido a partir da documentação do decodificador
  parcial), e da word em +4 de `CVisitanteEleitor`, que o conversor parcial define como 1.
* O que o slot 3 de `api::IKernelHSM` retorna em uma urna real (a chave que protege `bio.sk1`), e qual cifra
  `CSymmetricCipherFactory` constrói a partir dele.
* O layout de `CPadDS`/`CToUpperDS` entre +24 e +48 na func 11247.
* Se as quatro fontes de texto de suplente são lambdas em `ctelasvota.cpp` (mais provável) ou pequenas funções
  em um header de comum.

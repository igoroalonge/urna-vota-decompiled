# u14 — `ecourna-lib/ecourna/app/dados`: o modelo de dados do ecourna e seus conversores ASN.1

A unidade u14 tem **136 funções**. Seis delas foram vistas em execução nos votos gravados (§12). A unidade
cobre 35 arquivos originais da biblioteca do TSE **ecourna** (construída via Conan, caminho
`/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/…`), além de algumas classes cujos arquivos
tiveram de ser inferidos (§2). Este diretório (*dados*) contém as **classes de dados** da biblioteca
(identificadores de eleitor, descrições de mídia, parâmetros da urna, horários municipais, federações partidárias, o
resultado de comparecimento) e, em `asn/`, os **conversores** entre essas classes e os objetos ASN.1
dos formatos de arquivo do TSE.

Seu lugar no processo de votação:

* **Na inicialização**, o VOTA lê os parâmetros da urna (`…-pu.dat`, *parametrização da urna*: quantas vias do BU
  imprimir, regras de suspensão, time-outs, cabeçalhos e rodapés de relatórios, rótulos), o
  horário municipal (`…-cfm.dat`: zerésima, início e fim da votação), as federações partidárias (`…-fe.dat`) e as
  fotos dos candidatos. Cada arquivo passa por um desses conversores.
* **Durante a votação**, os identificadores do eleitor (título de eleitor, CPF, número livre) e os códigos de cargo
  são convertidos quando o estado do eleitor e o RDV são gravados.
* **No fim da votação** (*encerramento*), os gravadores de resultado leem a descrição da mídia
  (`infomidia.dat`), e `comum::CGravadorRCSecao` grava o **resultado de comparecimento** da seção
  (*resultado urna cadastro*: quem votou, como cada eleitor foi habilitado, quais mesários estavam presentes),
  opcionalmente cifrado (parâmetro `criptografarJUFA`). A build web nunca chega a esta etapa (§10).

Os fontes reconstruídos estão em `src/ecourna/app/dados/` (um `.cpp` por arquivo original; os headers
agrupam declarações pequenas). O mapa completo de funções está no §14.

---

## 1. Glossário

| termo | significado |
|---|---|
| *conversor* (`CConversorX`) | classe derivada de `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>`; `Converte` = classe de dados → ASN.1, `Deconverte` = ASN.1 → classe de dados |
| *entidade* | o objeto ASN.1 (runtime ASN.1 da III, `docs/libraries/asn1-runtime.md`); *dado* = a classe de dados C++ |
| *título de eleitor / número de inscrição eleitoral* | o número de inscrição do eleitor, 12 dígitos; impresso como `NNNN NNNN NNNN` |
| *identificação livre* | um número de identificação livre de 12 dígitos, o terceiro tipo de identificador de eleitor |
| *habilitação* | liberar um eleitor para votar no terminal do mesário: por impressão digital (*biométrica*), por código de liberação (*por código*), com orientação por áudio (*áudio*) |
| *mesário* | membro da mesa receptora; *registrar mesários* registra quais mesários estavam presentes na *abertura* e no *encerramento* |
| *comparecimento* | presença (votou / não votou / faltou); *justificativa* = justificativa de ausência |
| *mídia* | mídia flash removível: **MR** *mídia de resultado* (levada à Junta), **FC** *flash de carga*, **FV** *flash de votação* |
| *parametrização da urna (PU)* | o arquivo de parâmetros da urna `…-pu.dat` |
| *zerésima (ZE)* | o relatório de zeros emitido antes da votação; *BU* = *boletim de urna*, a apuração da seção |
| *RED / SA* | *recuperação de dados* / *sistema de apuração* (apuração manual), outros programas que imprimem BUs |
| *JUFA* | os dados de justificativa/comparecimento (chave `jufa.pk1`, parâmetro `criptografarJUFA`) |
| *CEPESC* | a biblioteca criptográfica do governo usada para cifrá-los |
| *federação partidária* | vários partidos atuando como um só |

---

## 2. Arquivos, classes e dados

| arquivo original | classe(s) | tipo ASN.1 ↔ classe de dados | arquivo de dados |
|---|---|---|---|
| `tiposbasicos.cpp` | `CErroLeituraBiometria` (+ os `CBaseType`s nomeados) | — | — |
| `iidentificadoreleitor.cpp` | `IIdentificadorEleitor` (abstrata) | — | — |
| `cnumeroinscricaoeleitoral.cpp`, `cnumerocpf.cpp`, `cnumeroidentificacaolivre.cpp` | os três identificadores | — | — |
| `cregistroidentificacaoeleitor.cpp` | `CRegistroIdentificacaoEleitor` | `ModuloTiposEcoUrna::RegistroIdentificacaoEleitor` (conversor na u40) | registros de eleitor / mesário |
| `cserialmidia.cpp` | `CSerialMidia` | `serialMidia` | `infomidia.dat` |
| `federacoes/cfederacao.cpp` | `CFederacao` | `ModuloFederacoes::Federacao` | `…-fe.dat` |
| `midias/caplicativo.cpp`, `cautenticacao.cpp`, `cinformacaomidia.cpp` | `CAplicativo`, `CAutenticacao`, `CInformacaoMidia` (+ `CDadosGeracaoMidia`, `CIdentificadorGeradorMidia` declaradas junto com elas) | `ModuloInformacaoMidia::*` | `infomidia.dat` |
| `processoeleitoral/cconfiguracaomunicipios.cpp` | `CConfiguracaoMunicipios` | `EntidadeConfiguracaoMunicipios` | `…-cfm.dat` |
| `parametrizacaourna/cparametrosurna.cpp` *(caminho inferido)* | `CParametrosUrna` (+ `CTituloRelatorio`, `CLabelParametrizado`) | `ModuloParametrizacaoUrna::ParametrosUrna` | `…-pu.dat` |
| `resultadournacadastro/*.cpp` (6 arquivos) + `ccomparecimentomesario.cpp` *(caminho inferido)* | `CResultadoUrnaCadastro`, `CDadosComparecimento`, `CEstadoComparecimento`, `CHabilitacaoBiometrica`, `CEstadoHabilitacaoPorCodigo`, `CDadosCifracao`, `CComparecimentoMesario` | `ModuloResultadoUrnaCadastro::*` | arquivo de comparecimento (RC) gravado no encerramento |
| `asn/cconversorturno.cpp`, `cconversorfase.cpp`, `cconversorcargoid.cpp`, `cconversorfoto.cpp`, `cconversorcabecalhoentidade.cpp`, `cconversoridentificadoreleitor.cpp` | conversores de tipos básicos | `Turno`, `Fase`, `CodigoCargoConsulta`, `Foto`, `CabecalhoEntidade`, `IdentificadorEleitor` | todos os arquivos do ecourna |
| `asn/federacoes/cconversorfederacao.cpp` *(caminho inferido)* | `CConversorFederacao` (DoConverte) | `Federacao` | `…-fe.dat` |
| `asn/midias/*.cpp` | `CConversorAplicativo`, `CConversorInformacaoMidia` | `Aplicativo`, `InformacaoMidia` | `infomidia.dat` |
| `asn/parametrizacaourna/*.cpp` | `CConversorParametrosUrna`, `CConversorTituloRelatorio`, `CConversorLabelParametrizado` | `ParametrosUrna`, `TituloRelatorio`, `LabelParametrizado` | `…-pu.dat` |
| `asn/resultadournacadastro/*.cpp` | 6 conversores | `EntidadeResultadoUrnaCadastro`, `EstadoComparecimento`, `HabilitacaoBiometrica`, `EstadoHabilitacaoPorCodigo`, `ComparecimentoMesario`, `ApresentacaoFotoEleitor` | arquivo RC |

Arquivos reconstruídos: `src/ecourna/app/dados/` — `tiposbasicos.{h,cpp}`, `dadoserros.h` (famílias de erro,
inferidas), `iidentificadoreleitor.{h,cpp}`, `cnumeroinscricaoeleitoral.{h,cpp}`, `cnumerocpf.cpp`,
`cnumeroidentificacaolivre.cpp`, `cregistroidentificacaoeleitor.{h,cpp}`, `cserialmidia.{h,cpp}`,
`federacoes/cfederacao.{h,cpp}`, `midias/{cinformacaomidia.h, caplicativo.cpp, cautenticacao.cpp,
cinformacaomidia.cpp}`, `processoeleitoral/cconfiguracaomunicipios.{h,cpp}`,
`parametrizacaourna/cparametrosurna.{h,cpp}`, `resultadournacadastro/{cdadoscifracao,
cdadoscomparecimento, cestadocomparecimento, cestadohabilitacaoporcodigo, chabilitacaobiometrica,
cresultadournacadastro}.{h,cpp}` + `ccomparecimentomesario.cpp`, `asn/cconversores.h` (todas as declarações
de conversores) e um `.cpp` por arquivo de conversor em `asn/`.

---

## 3. Hierarquia de classes (RTTI) e o protocolo dos conversores

```
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>        (ecourna/api/asn/iconversorasn.hpp, unit u11)
 ├─ asn::CConversorTurno          <Turno, CBaseType<ushort,1,2,11>>
 ├─ asn::CConversorFase           <Fase, CFaseID>
 ├─ asn::CConversorCargoID        <CodigoCargoConsulta, CBaseType<ushort,1,99,3>>
 ├─ asn::CConversorFoto           <Foto, CFoto>
 ├─ asn::CConversorCabecalhoEntidade <CabecalhoEntidade, CCabecalhoEntidade>
 ├─ asn::CConversorIdentificadorEleitor <IdentificadorEleitor, CIdentificadorEleitor>
 ├─ asn::CConversorFederacao, CConversorAplicativo, CConversorInformacaoMidia,
 │  CConversorTituloRelatorio, CConversorLabelParametrizado, CConversorParametrosUrna,
 │  CConversorApresentacaoFotoEleitor, CConversorComparecimentoMesario,
 │  CConversorEstadoComparecimento, CConversorEstadoHabilitacaoPorCodigo,
 │  CConversorHabilitacaoBiometrica, CConversorResultadoUrnaCadastro   (all "si")
 └─ (siblings in units u40/u11: CConversorAutenticacao, CConversorDadosGeracaoMidia,
    CConversorFederacoes, CConversorParametrizacaoUrna, CConversorConfiguracaoMunicipio(s),
    CConversorHorariosUrna, CConversorRegistroIdentificacaoEleitor, CConversorDadosComparecimento,
    CConversorComparecimentoSecao, CConversorIdentificacaoJustificativa, CConversorDadosCifracao, …)
ecourna::app::dados::IIdentificadorEleitor               (vtable @1122204, slot 2 pure virtual)
 ├─ CNumeroInscricaoEleitoral   (@1122028)
 ├─ CNumeroCPF                  (@1121848)
 └─ CNumeroIdentificacaoLivre   (@1121932)
ecourna::api::exception::CError
 └─ CBaseError<E, SErrorLimits{lo,hi}>  for EDadosError {1935,2135}, EDadosFederacoesError {2940,2965},
    EDadosMidiasError {3040,3065}, EDadosProcessoEleitoralError {3140,3190},
    EDadosResultadoUrnaCadastroError {3265,3315}, asn::EAsnError {2235,2335},
    asn::EAsnMidiasError {2485,2510}, asn::EAsnParametrizacaoUrnaError {2560,2585},
    asn::EAsnResultadoUrnaCadastroError {2635,2665}
```

**vtable de `IConversorASN` do ecourna** (4 slots, como a de `comum::asn` do uenux2 descrita na u03):
`[0]` destrutor (ICF `return this`, func 174), `[1]` destrutor de deleção (func 144), `[2]`
`DoConverte(const TDado&)` → `TEntidade`, `[3]` `DoDeconverte(const TEntidade&)` → `TDado`.
Os wrappers públicos não são virtuais e são inlinados nos seus chamadores: `Deconverte` primeiro exige
`isValid() && isStrictlyValid()` e, caso contrário, lança `CBaseError<EApiAsnError>(1902, typeid(E).name() +
": " + trace_invalid(...))` (srcloc `iconversorasn.hpp:66`); `Converte` valida o resultado
(`iconversorasn.hpp:49`). É por isso que os próprios conversores nunca verificam faixas ASN.1 (`numZeresimas
1..50`, `tempoDispararSuspensao 15..360`, …): o runtime da III faz isso, antes que `DoDeconverte` seja executado.

**Armadilha de nomes.** Como na u03, muitas funções de slot foram nomeadas pela ferramenta a partir de um helper estático
inlinado que carrega o `source_location` (a func 9156 "`DesconverterFormaSuspenderComVoto`" é
`CConversorParametrosUrna::DoDeconverte`; o mesmo vale para 9145, 9167, 9179, 9196, 9087, 9226, 9227). O
slot da vtable é que decide; o §14 dá os nomes corrigidos. Slots mostrados como `vf2`/`vf3` são `DoConverte`/`DoDeconverte`.

### 3.1 Como as enumerações são convertidas

As classes de dados usam os seus próprios enums C++. Aparecem quatro convenções, e o wasm-opt mesclou os helpers
que diferem apenas em constantes em corpos compartilhados:

| convenção | corpo compartilhado | usado por |
|---|---|---|
| C++ `0..n-1` → ASN `v+1` (Converte) | func 2306 (`c >= 3` → throw) | `ConverteTipoMidia`, `ConverterFormaSuspender{Com,Sem}Voto`, `ConverteSituacaoReconhecimentoMesario`, `ConverteSituacaoHabilitacaoAudio` |
| idem, mensagem de uma `std::string` estática | func 6148 | `ConverterAlinhamentoTitulo`, `ConverterGeneroLabel` |
| identidade com faixa `[1, k]` | func 6151 | `ConverteTipoAplicativo` (k=7), `ConverteEstadoColetaDigital` (k=4) |
| ASN `v` → C++ `v-1`, faixa 3 | func 6144 | `DeconverteSituacaoReconhecimentoMesario`, `DeconverteSituacaoHabilitacaoAudio` |
| tabela de consulta (não monotônica) | inline | `SituacaoComparecimentoEleitor` ↔ `ESituacaoComparecimento`: tabelas @1135428 `{1,4,2,3}` e @1135444 `{0,2,3,1}` |

Vários helpers `Desconverte` são um `switch` com um `case -1:` explícito que lança a partir de uma linha de código-fonte
diferente da do default (por exemplo, `CConversorFase` linhas 42/44, `CConversorAplicativo` 88/92). O
mesmo padrão existe no uenux2 (u03). Provavelmente é um enumerador extra "inválido" do `NamedNumber`
gerado; o seu nome não está no binário.

### 3.2 Famílias e códigos de erro

Todas as mensagens desta unidade, com os códigos (os nomes dos enumeradores são desconhecidos): veja os fontes
reconstruídos. Faixas de código usadas: EDadosError 1951, 2014, 2023–2030; EAsnError 2235–2268;
EAsnMidiasError 2485–2489; EAsnParametrizacaoUrnaError 2560–2577; EAsnResultadoUrnaCadastroError
2635–2660; EDadosFederacoesError 2940; EDadosMidiasError 3040–3044; EDadosProcessoEleitoralError
3145–3146; EDadosResultadoUrnaCadastroError 3267–3283. Os construtores são thunks de 18 bytes
(9025, 9026, 9041, 9046, 9107, 9158, 9229, 9266) sobre o corpo genérico de `CBaseError`, func 1011.

---

## 4. Identificadores de eleitor

`IIdentificadorEleitor` (24 bytes: `+4 TTipoIdentificadorEleitor m_tipo`, `+8 std::string m_numero`,
`+20 uebyte m_tamanho`) é sempre manipulado através de `std::shared_ptr` (`TSharedIdentificadorEleitor`).

Regras do construtor (func 9256 e o corpo mesclado 3939):

1. `m_numero` = o número **preenchido à esquerda com `'0'`** até o tamanho máximo (a func 753 copia a string e
   insere os zeros na posição 0). Um número mais longo é mantido como está.
2. O número preenchido não pode ser todo de zeros e deve conter apenas `'0'..'9'`; caso contrário,
   `EDadosError 2023 "O número de identificação de eleitor deve ser numérico ({})."` (linha 28).
3. A classe derivada então rejeita uma entrada mais longa do que o seu tamanho: título 12 (2024), CPF 11 (2025),
   livre 12 (2026), mensagem `"O tamanho do número de … não pode ser maior do que {} ({})."`.

Essas classes não validam nenhum dígito verificador (nem o do título nem o do CPF). Os dígitos verificadores são conferidos
antes, onde o número é digitado, pelas regras de entrada do VOTA `comum::md::CRegraTitulo` / `comum::md::CRegraCPF`
(vf4, funcs 11272 / 11270: somas ponderadas `% 11`), então isso não é uma lacuna no caminho normal de entrada. A forma de exibição (slot 2 da vtable,
nome inferido `GetNumeroFormatado`) insere separadores: título `1234 5678 9012`, CPF
`123.456.789-01`, livre inalterado.

`CRegistroIdentificacaoEleitor` (20 bytes) = *identificação utilizada* (o documento usado para habilitar o
eleitor; `m_identificadorHabilitacao`, obrigatório, códigos 2029/2030/2028) + *identificação principal* opcional
(código 2027). O construtor de dois argumentos ativa o optional mesmo quando o segundo ponteiro está vazio.

`CConversorIdentificadorEleitor` mapeia a alternativa do CHOICE para a classe: `numeroInscricao` →
`make_shared<CNumeroInscricaoEleitoral>`, `numeroCPF` → `CNumeroCPF`, `identificacaoLivre` →
`CNumeroIdentificacaoLivre`; um CHOICE não selecionado lança 2268. O inverso copia `GetNumero()` para
a alternativa selecionada.

---

## 5. Informações da mídia (`infomidia.dat`)

`CInformacaoMidia` (116 bytes): `+0 ETipoMidia` (MR 0, FC 1, FV 2), `+4 fase`, `+8 idPE`
(`CBaseType<unsigned,0,99999>` "ProcessoEleitoralID"), `+12 uf`, `+24 turno`, `+32 CDadosGeracaoMidia`
(serial, usuário, id do gerador, data), `+104 vector<CAplicativo>`.

* Somente uma **MR** (mídia de resultado) carrega `aplicativos` (os programas que ela pode iniciar — vota, sa, red, vpp,
  ste, adh, atue —, cada um com um `CAutenticacao` opcional: período de validade, tamanho da senha, número de
  tentativas, hash da senha). `GetAplicativos()` em qualquer outra lança 3044; o construtor não MR
  rejeita MR (3043).
* `CConversorInformacaoMidia::DoDeconverte` decide o tipo pela presença de `aplicativos`: se
  presente, o objeto é construído como **MR independentemente de `tipoMidia`** (func 9033); se ausente, com o
  `tipoMidia` do arquivo.
* `CAutenticacao::GetDataHoraInicial/Final` exigem **ambas** as datas (códigos 3041/3042).
* `CSerialMidia` aceita exatamente 8 dígitos hexadecimais (`std::regex_match(s, std::regex("([A-Fa-f0-9]{8})"))`,
  func 9258, construído a cada chamada) e os armazena como 4 bytes; caso contrário, 1951
  `"Número de série '{}' inválido."`.

Cada um dos 8 cenários traz `estatico/infomidia-fv-1-t.dat` (cenários de 1º turno) ou
`infomidia-fv-2-t.dat` (2º turno), de 172 bytes cada, todos diferentes. Decodificado (municipal-t1): `tipoMidia = 2 (fc)`,
`fase = 3 (treinamento)`, `idPE = 2400`, `uf = "ac"`, `turno = 1`, `dadosGeracaoMidia = {serialMidia
"A1B2C3DB", usuario "simulador-votacao-ng", identificadorGeradorMidia {nome "simulador-votacao-ng",
serialCertificadoTPM 64×'0', serialInstalacao "A1B2C3DA"}, data "20260910T185602"}`, sem `aplicativos`.
Os outros arquivos diferem apenas em `idPE` (2400 municipal, 2500 geral), `uf` (ac, df, zz), `turno` (1/2) e no
horário de geração; todos são mídias FC da fase treinamento com o mesmo serial.
Leitores: `api::CFileASN::ReadFromFile<InformacaoMidia>` (func 3735, a única função que constrói este
conversor) a partir de `comum::CGravadorBU`, `CGravadorRDV` e `IGravadorEnvelope` (vf7, GravaResultado).
`comum::impl::CValidaMidia` (func 11172) só usa os nomes `infomidia.dat`/`infomidia.vsc` (verificação de
assinatura); ela não decodifica o arquivo através desta unidade.

---

## 6. Parâmetros da urna (`…-pu.dat`)

`CParametrosUrna` (402 bytes) está embutido em **+88 de `comum::CConfiguracaoEleicao`**, e é por isso que outras
unidades veem `CConfiguracaoEleicao+88/+92` (regras de suspensão), `+484` (apresentarPartido) e `+486`
(imprimirQrCodeNoBU). Os offsets dos campos estão em `src/ecourna/app/dados/parametrizacaourna/cparametrosurna.h`.
Correção à u02: os vectors em +100… são **seis `vector<CTituloRelatorio>`** (+100..+160), e não três
vectors de `SRotulo`.

`CConversorParametrosUrna::DoDeconverte` (func 9156, observada) converte as duas regras de suspensão
(enums baseados em 0; códigos 2567/2568 e 2570/2571), copia os 24 campos escalares que vêm em seguida (22 inteiros e os
dois booleanos `criptografarBU`/`criptografarJUFA`), converte as seis listas de títulos com
`CConversorParametrosUrna::DesconverterTitulos` (func 5102, observada), copia `telaEmissaoBU`, converte os quatro
rótulos, copia os últimos 10 booleanos e chama o construtor de 47 argumentos (func 5093). `DoConverte` (func 9164)
é a imagem espelhada (`ConverterTitulos`, func 9161). Os dois helpers de títulos são funções **membro** const (nome
inferido): a sua assinatura wasm mantém um `this` não usado que os chamadores preenchem com um valor lixo, exatamente como nos
membros nomeados por srcloc `ConverterFormaSuspender{Sem,Com}Voto`; uma função local ao arquivo teria perdido o parâmetro.

Valores em todos os cenários do simulador. Os 16 arquivos `-pu.dat` (`t00000br-pu.dat` + o arquivo do estado em cada um dos 8
cenários) são 4 arquivos distintos, um por família de cenários, que diferem apenas no cabeçalho (`dataGeracao`, pleito
2400/2500); o corpo de `ParametrosUrna` é idêntico byte a byte em todos os 16:

| campo | valor | campo | valor |
|---|---|---|---|
| formaSuspensaoSemVoto | 1 tornaNaoVotou | formaSuspensaoComVoto | 2 tornaOutrosNulo |
| numZeresimas | 1 | numBUVotaObrigatorios / Adicionais | **5 / 5** |
| numBUFinalRED Obrigat/Adic | 2 / 5 | numBUParcialRED Obrigat/Adic | 2 / 3 |
| numBUFinalSA Obrigat/Adic | 2 / 5 | numBUParcialSAObrigat | 1 |
| numTentativasHabilitacao / Verificacao | 4 / 2 | numRelatorio Estado/Eleitores/VersoesDados/PU | 5/1/1/1 |
| criptografarBU | **FALSE** | criptografarJUFA | **TRUE** |
| numDigitosPartido | 2 | tempoDispararSuspensao / TE | 45 / 45 |
| tempoConfirmacaoVoto | 1000 | tempoDesligamentoAutomatico / Aviso | 1800 / 60 |
| imprimirZeradosBU / gravarZeradosBU | FALSE / FALSE | aceitarJustificativa, aceitarBrancoNulo, apresentarPartido, permitirHabManualAudio, imprimirQrCodeNoBU, exibirScoreBiometria, registrarMesarios, pedeAnoNascimentoEleitor | todos TRUE |

Rótulos: `Município/Município/Municípios/Municípios` (masculino), `Zona Eleitoral/Zona/Zonas
Eleitorais/Zonas` (feminino), `Seção Eleitoral/Seção/Seções Eleitorais/Seções` (feminino),
`Partido/…` (masculino). As strings estão em ISO-8859-1 nos arquivos.

---

## 7. Horário municipal e federações

* `CConfiguracaoMunicipios(cabecalho, map<código, CConfiguracaoMunicipio>)` (func 9028, observada)
  rejeita um map vazio (3145) e qualquer entrada cuja chave difira do seu `codigoMunicipio` (3146). Nos
  cenários, todo `-cfm.dat` (pleitos 2410/2420 municipal, 2510/2520 geral) tem 3 municípios com o mesmo
  horário: 1º turno (`t02410ac-cfm.dat` etc.) zerésima `20261004T070000`, início `…T080000`, fim `…T170000`,
  término `20261005T040000`; 2º turno (`t02420ac-cfm.dat` etc.) os mesmos horários em 2026-10-25 (término
  `20261026T040000`).
* `CFederacao(id, sigla, nome, partidos)` rejeita uma lista de partidos vazia (2940). Os arquivos `-fe.dat` do simulador
  contêm apenas o cabeçalho (`federacoes` ausente). O conversor grava `identificador`
  (INTEGER 100..999), sigla, nome e um `INTEGER(0..99)` por partido.

---

## 8. O resultado de comparecimento (*resultado urna cadastro*)

```
CResultadoUrnaCadastro (164)  = EntidadeResultadoUrnaCadastro
 ├ CCabecalhoEntidade cabecalho, CFaseID fase, string versaoVotacao, ESituacaoArquivo (final/parcial)
 ├ optional<CDadosComparecimento> (72)            ── infoDadosComparecimento [0]
 │   ├ vector<CIdentificacaoJustificativa>  {RegistroIdentificacaoEleitor, anoNascimento}
 │   ├ CIdentificacaoSecaoEleitoral
 │   ├ vector<CEstadoComparecimento> (96 each)
 │   │   ├ CRegistroIdentificacaoEleitor, ESituacaoComparecimento (faltou/semCargo/naoVotou/votou)
 │   │   ├ optional<CApresentacaoFotoEleitor>  {estado 0..2, resultado 0..8}
 │   │   ├ optional<ESituacaoHabilitacaoAudio> (automática / pelo mesário / não habilitado)
 │   │   └ optional<CHabilitacaoBiometrica> (48)
 │   │       ├ tentativas, ultimoScore (0..999, 0 = absent), dedo (0..10), erroLeitura (0..12)
 │   │       └ optional<CEstadoHabilitacaoPorCodigo> {situação do mesário, optional<registro do mesário>}
 │   └ optional<vector<CComparecimentoMesario>> abertura / encerramento {registro, dataHora, estadoColeta 0..4}
 └ optional<CDadosComparecimentoCifrado> (48)      ── infoDadosComparecimento [1]
     └ CDadosCifracao {chave, salt ≥16 B, informacaoAdicional ≥16 B} + conteudo
```

Regras de consistência impostas durante a **leitura** (Deconverte). As regras de `CEstadoHabilitacaoPorCodigo` e
`CDadosCifracao` são verificações no construtor das classes de dados, portanto também se aplicam quando `CGravadorRCSecao` (func
11616) constrói esses objetos para gravação:

* `EstadoComparecimento`: áudio ou biometria exigem os dados de foto (2637, "Eleitor que votou não possui
  informações sobre a apresentacao de fotos do eleitor."); foto ou biometria exigem os dados de áudio
  (2638). Formas válidas: nada, foto+áudio, foto+áudio+biometria.
* `HabilitacaoBiometrica`: se `dedoHabilitado ≠ naoIdentificado`, não pode haver `habilitacaoPorCodigo`
  e `erroLeituraBiometria = semErro` (2645 "Estrutura de dados mal formada."); caso contrário,
  `habilitacaoPorCodigo` é obrigatório (2646).
* `EstadoHabilitacaoPorCodigo`: identificação do mesário presente ⇔ situação ≠ NaoReconhecido (3270/3271).
* `CDadosCifracao`: salt e informacaoAdicional com pelo menos 16 bytes (3282/3283); o tamanho da chave não é verificado.
* `EntidadeResultadoUrnaCadastro`: o CHOICE precisa estar selecionado (2660).

Cifragem: com `criptografarJUFA = TRUE`, `comum::CGravadorRCSecao::GravaResultado` (func 11616, chamada de
`LeChavePublica` pela ferramenta) codifica `DadosComparecimento`, cifra-o com o CEPESC (`CPlainText`,
arquivo de chave `jufa.pk1`) e armazena `DadosComparecimentoCifrado`. Os dados ligam o identificador de cada eleitor ao
comparecimento e ao método de habilitação (nunca ao voto).

---

## 9. O que esta unidade contribui para o *boletim de urna* (BU)

O BU em si é construído por `comum::CGravadorBU` / `CConversorEntidadeBU` (uenux2, veja
`docs/bu/build-a-bu.md` e `docs/bu/qrcode.md`). Esta unidade fornece, passo a passo:

1. **Quantas vias e qual texto** (de `…-pu.dat`, §6): os parâmetros dão
   `numBUVotaObrigatorios = 5` vias obrigatórias e `numBUVotaAdicionais = 5` adicionais
   (o consumidor desses dois inteiros está no código de encerramento do VOTA, não nesta unidade; o significado
   é deduzido dos nomes e do texto de tela abaixo, que lista exatamente 5 vias).
   O texto de tela `telaEmissaoBU` diz aos mesários o que fazer com elas:
   > "Após a impressão completa do BU, colha as assinaturas necessárias e distribua as vias do seguinte
   > modo: - Duas vias vão, juntamente com a mídia de resultado, para a Junta Eleitoral; - Uma via fica
   > com o Presidente da Mesa Receptora de Votos; - Uma via fica com a fiscalização partidária; - Uma via
   > deve ser afixada em lugar visível da seção eleitoral."
2. **Linhas de cabeçalho e rodapé** (`CTituloRelatorio {alinhamento, estilo, texto}`):
   `cabecalho` = [centro/expandido "Justiça Eleitoral", centro/normal "Tribunal Regional Eleitoral
   [<uf>]"]; `rodapeBUVOTA` = [centro "O conteúdo deste BU poderá ser\nconferido no endereço\nresultados.tse.jus.br",
   centro "ASSINATURAS:", esquerdo "PRESIDENTE: … MESÁRIOS: … FISCAIS:"]; `rodapeZEVOTA` = o mesmo
   sem a primeira linha (zerésima). Para o SA: `rodapeBUSA`/`rodapeBUParcial` são assinados por "PRESIDENTE DA JUNTA,
   COMPONENTES DA JUNTA, MINISTÉRIO PÚBLICO, FISCAIS", `rodapeZESA` por "SECRETÁRIO DA JUNTA, FISCAIS".
3. **Chaves**: `imprimirQrCodeNoBU = TRUE` (os QR codes; lido em CConfiguracaoEleicao+486 pelo gerador de
   QR, `docs/bu/qrcode.md`), `imprimirZeradosBU` / `gravarZeradosBU = FALSE` (pelos seus nomes:
   se as entradas com zero votos são impressas / gravadas), `criptografarBU = FALSE`.
4. **O número de série da mídia**: `CGravadorBU` lê `infomidia.dat` através do
   `CConversorInformacaoMidia` desta unidade e grava o hex de `CSerialMidia` (8 dígitos hexadecimais validados por regex) como
   `urna.numeroSerieFV`, com o padrão `"00000000"` quando o arquivo não existe (a build web só tem
   `infomidia-fv-{1,2}-t.dat`, então o padrão é usado; `docs/bu/build-a-bu.md` §fields).
5. **Os cabeçalhos dos arquivos do ecourna** usam `CConversorCabecalhoEntidade`: `dataGeracao` como DataHoraJE
   (`"{:04}{:02}{:02}T{:02}{:02}{:02}"`, 15 caracteres) e o CHOICE `idEleitoral` (0 PE, 1 pleito, 2 eleição);
   um CHOICE não selecionado ou um índice ≥ 3 lança 2236. (O cabeçalho do próprio BU usa o gêmeo do uenux2
   `comum::asn::CConversorCabecalhoEntidade`, func 11589.)
6. **Os códigos de cargo** nos registros de voto do RDV/BU passam por `CConversorCargoID`: 1..13 →
   `cargoConstitucional` (presidente … vereador), 25..99 → `numeroCargoConsultaLivre`, qualquer outro 2245.
7. **Junto com o BU**, no encerramento, o arquivo de comparecimento do §8 é gravado (com
   `registrarMesarios = TRUE`, os mesários da abertura/encerramento são incluídos).

---

## 10. Particularidades da build web

* Nada nesta unidade é mock. As diferenças de comportamento vêm dos dados e de quais caminhos
  a página web alcança.
* A página web nunca executa o encerramento (sem thread do mesário, votos não mantidos; veja
  `docs/bu/build-a-bu.md`). Assim, os conversores de `resultadournacadastro`, o gravador de RC e a
  leitura de `infomidia.dat` por `CGravadorBU` estão **mortos na build web** (nenhum foi observado em execução).
* Os 16 arquivos `-pu.dat` trazem parâmetros idênticos (4 arquivos distintos que diferem apenas no cabeçalho);
  os arquivos `-fe.dat` contêm só o cabeçalho; os arquivos `infomidia-fv-{1,2}-t.dat` são mídias FC com um gerador falso (`simulador-votacao-ng`, serial de TPM todo zerado) e não são assinados (o seu `.vsc`
  está vazio, segundo `docs/data-model/asn1-schemas.md`).

---

## 11. Observações sobre wasm / Emscripten

* **merge-similar-functions**: os três construtores de identificadores são thunks de 33 bytes para a func 3939
  (8 parâmetros: srcloc, código, string de formato, vtable, tamanho, tipo); seis instanciações de `ConverteLista`
  são thunks de 26/27 bytes para a func 1970; os nove construtores de famílias de erro são thunks de 18 bytes para a
  func 1011 (oito estão nesta unidade; o de EAsnMidiasError, func 9181, não está na lista de índice da u14); os helpers de enum compartilham 2306/6144/6148/6151 (§3.1).
* **ICF**: `IIdentificadorEleitor::GetNumero` é o mesmo corpo que `api::CFixedText::vf2` (func 1139),
  daí os nomes de "GUI" nos chamadores; os destrutores dos identificadores derivados são corpos minúsculos unidos por ICF (3491/3492).
* **ABI v2 da libc++**: `std::shared_ptr` é `trivial_abi`, então os parâmetros shared_ptr por valor são liberados
  pela função chamada (visível nas funcs 1675/1878); `std::regex` usa `ECMAScript = 512` (flag na func 2416).
* **Strings**: quase todo literal de `ecourna/app/dados` está em **ISO-8859-1** (por exemplo, `N\xFAmero de
  s\xE9rie`); o `"Turno inválido."` de `cconversorturno.cpp` é o único em UTF-8. Algumas mensagens são
  globais `std::string` construídas por `__wasm_call_ctors` (func 14478): os nomes de tipo de CBaseType
  (`ProcessoEleitoralID` @1911992 … `TipoIdentificadorEleitor` @1912208, 19 nomes) e quatro mensagens de conversores
  @1912232..1912268, uma das quais ("Forma de validar título inválido.") nunca é usada.
* **Artefatos do descompilador** a ignorar: códigos inteiros anotados com strings sem relação (por exemplo, `2485 /*
  "Foto duplicada…" */`, `2650 /* "{}: instância…" */`), e slots de tabela divididos em dois (`671 /* … */4` =
  slot 6714, `604 … 8` = slot 6048, `677 … 3` = slot 6773, `683 … 1` = slot 6831). Uma tabela de switch
  exibida como `(b << 2)[283861]` na verdade está em 1135444 (confira com `q.py wat`).
* Desreferências de shared_ptr `null` não geram trap em wasm (o endereço 4 lê 0), veja o §13, item 6.

---

## 12. Funções observadas em execução (votos gravados)

5102 `CConversorParametrosUrna::DesconverterTitulos` e 9156 `CConversorParametrosUrna::DoDeconverte` (carregando `-pu.dat`), 9028
`CConfiguracaoMunicipios` (carregando `-cfm.dat`), 9218 `CConversorCabecalhoEntidade::DoDeconverte`
(cabeçalhos desses arquivos), 9226 `CConversorFoto::DoDeconverte` (fotos dos candidatos), 9230
`CConversorCargoID::DoConverte` (códigos de cargo dos registros de voto).

---

## 13. Código estranho ou arriscado

1. **`throw` ausente** em `CConversorResultadoUrnaCadastro::DoConverte` (func 9056, linha 48): quando um
   `CResultadoUrnaCadastro` não tem dados de comparecimento nem em claro nem cifrados, o código constrói
   `CBaseError(2658, "Classe de dados de comparecimento mal formada.")` na pilha (construtor de CError func
   1143, sem `__cxa_allocate_exception`/`__cxa_throw`), destrói o objeto e retorna uma entidade com um
   CHOICE não selecionado. O erro só reaparece como um erro genérico de entidade inválida vindo de `Converte`.
2. **Mensagens trocadas** em `CEstadoComparecimento` (funcs 9020/9021): o getter de áudio reporta
   "Dados de apresentação de foto…" (3268) e o getter de foto "Habilitação de áudio…" (3269).
3. **`tipoMidia` ignorado** quando `aplicativos` está presente (func 9179): qualquer arquivo de mídia com aplicativos
   se torna um objeto MR (mídia de resultado).
4. `CAutenticacao::GetDataHoraInicial` lança "início não definido" quando apenas a data de **fim** está ausente
   (os dois getters testam os dois optionals).
5. `ultimoScore = 0` é descartado por `CConversorHabilitacaoBiometrica::DoConverte` (0 significa "ausente").
6. `CRegistroIdentificacaoEleitor(hab, principal)` ativa o optional mesmo que `principal` esteja vazio;
   `GetIdentificadorPrincipal` retornaria então um ponteiro vazio, e `CConversorIdentificadorEleitor::DoConverte`
   (alcançada através de `CConversorRegistroIdentificacaoEleitor::DoConverte`, func 9114, que só testa a flag de
   ativação) o desreferencia (`identificador->GetTipo()`): um crash em código nativo, uma leitura silenciosa do endereço 4 em wasm.
   **Apenas latente**: os dois chamadores da func 1878 passam um ponteiro não nulo (a func 9112 passa um resultado de `make_shared` de
   `CConversorIdentificadorEleitor::Deconverte`, a func 11616 o resultado de `CriaIdentidadeEleitor`, func 5845; ambas
   lançam exceção em vez de retornar nulo).
7. `CConversorCabecalhoEntidade::DoConverte` lança apenas para o tipo de id 3; tipos ≥ 4 (e valores negativos, já que o
   índice do `br_table` é sem sinal) passam silenciosamente e deixam o CHOICE não selecionado.
8. Codificações misturadas (ISO-8859-1 vs UTF-8) nos textos de exceção; erros de digitação nas mensagens ("biomoetria",
   "habiliação", "apresentacao"); uma constante de mensagem não usada.

---

## 14. Tabela de mapeamento completa (136 funções)

`run` (coluna "executou") = observada em execução durante os votos gravados. Os caminhos de arquivo original marcados com *(caminho inferido)* não têm
`source_location`. As linhas de "biblioteca" são instanciações da libc++/do runtime ASN.1 ou corpos compartilhados do wasm-opt
que só são resumidos em um comentário do fonte listado.

| func | tamanho | executou | símbolo reconstruído | arquivo original | src | conf. |
|---:|---:|:-:|---|---|---|---|
| 778 | 12 |  | `ecourna::app::dados::IIdentificadorEleitor::~IIdentificadorEleitor` | ecourna-lib/ecourna/app/dados/iidentificadoreleitor.cpp | src/ecourna/app/dados/iidentificadoreleitor.cpp | alta |
| 1162 | 273 |  | `ecourna::app::dados::CDadosComparecimento::~CDadosComparecimento (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | média |
| 1241 | 33 |  | `ecourna::app::dados::CNumeroInscricaoEleitoral::CNumeroInscricaoEleitoral (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | alta |
| 1673 | 105 |  | `std::vector<ecourna::app::dados::CTituloRelatorio>::~vector` | biblioteca (instanciação libc++) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comentário) | alta |
| 1675 | 401 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor)` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | alta |
| 1878 | 480 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor, TSharedIdentificadorEleitor)` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | alta |
| 1943 | 201 |  | `std::optional<TVectorComparecimentoMesario>::~optional` | biblioteca (instanciação libc++) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comentário) | média |
| 1970 | 267 |  | `ecourna::api::asn::ConverteLista<TConversor> (shared merged body, 6 instantiations)` | template de ecourna/api/asn (inferido) | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | média |
| 2306 | 268 |  | `merged body of 5 enum Converte helpers (0-based -> ASN value+1, count 3)` | wasm-opt merge-similar | (expandido em cada conversor) | alta |
| 2659 | 88 |  | `ecourna::app::dados::CDadosCifracao::~CDadosCifracao (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comentário) | média |
| 2663 | 102 |  | `ecourna::app::dados::CLabelParametrizado::~CLabelParametrizado (implicit)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/clabelparametrizado.h (caminho inferido) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comentário) | média |
| 2664 | 123 |  | `std::vector<ecourna::app::dados::CAplicativo>::~vector` | biblioteca (instanciação libc++) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comentário) | média |
| 2668 | 1745 |  | `ecourna::app::dados::CErroLeituraBiometria::CErroLeituraBiometria` | ecourna-lib/ecourna/app/dados/tiposbasicos.cpp | src/ecourna/app/dados/tiposbasicos.cpp | alta |
| 2669 | 294 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::GetIdentificadorHabilitacao` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | alta |
| 2671 | 55 |  | `std::__exception_guard<vector::__destroy_vector>::~__exception_guard (ICF, also boost)` | biblioteca (libc++) | - | média |
| 3486 | 141 |  | `ecourna::app::dados::CComparecimentoMesario::CComparecimentoMesario` | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentomesario.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/ccomparecimentomesario.cpp | média |
| 3487 | 116 |  | `ecourna::app::dados::CDadosComparecimentoCifrado::~CDadosComparecimentoCifrado (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.h (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comentário) | média |
| 3489 | 464 |  | `std::__tree<map<TMunicipioID,CConfiguracaoMunicipio>>::__find_equal(hint, ...)` | biblioteca (instanciação libc++) | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp (comentário) | média |
| 3490 | 132 |  | `ecourna::app::dados::CDadosGeracaoMidia::~CDadosGeracaoMidia (implicit)` | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.h (caminho inferido) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comentário) | média |
| 3939 | 883 |  | `merged ctor body of CNumeroInscricaoEleitoral / CNumeroCPF / CNumeroIdentificacaoLivre` | wasm-opt merge-similar | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp, cnumerocpf.cpp, cnumeroidentificacaolivre.cpp | alta |
| 5088 | 389 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacao, const CRegistroIdentificacaoEleitor&)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | alta |
| 5089 | 124 |  | `std::optional<ecourna::app::dados::CRegistroIdentificacaoEleitor>::~optional` | biblioteca (instanciação libc++) | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp (comentário) | média |
| 5090 | 283 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacao)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | alta |
| 5091 | 746 |  | `ecourna::app::dados::CDadosCifracao::CDadosCifracao` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp | alta |
| 5093 | 1288 |  | `ecourna::app::dados::CParametrosUrna::CParametrosUrna (47 args)` | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (caminho inferido) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp | média |
| 5102 | 345 | * | `ecourna::app::dados::asn::CConversorParametrosUrna::DesconverterTitulos` (nome inferido; membro: `this` morto mantido na assinatura) | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | média |
| 5104 | 137 |  | `std::__exception_guard<vector<CAplicativo>::__destroy_vector>::~__exception_guard` | biblioteca (instanciação libc++) | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 5105 | 64 |  | `std::optional<TSharedIdentificadorEleitor>::~optional` | biblioteca (instanciação libc++) | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp (comentário) | alta |
| 5106 | 33 |  | `ecourna::app::dados::CNumeroIdentificacaoLivre::CNumeroIdentificacaoLivre (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumeroidentificacaolivre.cpp | src/ecourna/app/dados/cnumeroidentificacaolivre.cpp | alta |
| 5107 | 33 |  | `ecourna::app::dados::CNumeroCPF::CNumeroCPF (thunk -> 3939)` | ecourna-lib/ecourna/app/dados/cnumerocpf.cpp | src/ecourna/app/dados/cnumerocpf.cpp | alta |
| 5832 | 284 |  | `std::optional<TVectorComparecimentoMesario>::optional(const optional&)` | biblioteca (instanciação libc++) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comentário) | média |
| 5846 | 397 |  | `ecourna::app::dados::CDadosComparecimento::CDadosComparecimento(const CDadosComparecimento&) (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp (comentário) | média |
| 9014 | 262 |  | `ecourna::app::dados::CResultadoUrnaCadastro::GetDadosComparecimentoCifrado` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | alta |
| 9015 | 260 |  | `ecourna::app::dados::CResultadoUrnaCadastro::GetDadosComparecimento` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | alta |
| 9016 | 260 |  | `ecourna::app::dados::CHabilitacaoBiometrica::GetEstadoHabilitacaoPorCodigo` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | alta |
| 9017 | 260 |  | `ecourna::app::dados::CEstadoHabilitacaoPorCodigo::GetIdentificacaoMesario` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp | alta |
| 9019 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetHabilitacaoBiometrica` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | alta |
| 9020 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetSituacaoHabilitacaoAudio` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | alta |
| 9021 | 260 |  | `ecourna::app::dados::CEstadoComparecimento::GetApresentacaoFoto` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | alta |
| 9023 | 260 |  | `ecourna::app::dados::CDadosComparecimento::GetMesariosEncerramento` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | alta |
| 9024 | 260 |  | `ecourna::app::dados::CDadosComparecimento::GetMesariosAbertura` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp | alta |
| 9025 | 18 |  | `CBaseError<EDadosResultadoUrnaCadastroError,{3265,3315}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9026 | 18 |  | `CBaseError<EDadosProcessoEleitoralError,{3140,3190}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9027 | 268 |  | `std::map<TMunicipioID,CConfiguracaoMunicipio>::insert(first, last)` | biblioteca (instanciação libc++) | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp (comentário) | média |
| 9028 | 672 | * | `ecourna::app::dados::CConfiguracaoMunicipios::CConfiguracaoMunicipios` | ecourna-lib/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp | src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp | alta |
| 9030 | 260 |  | `ecourna::app::dados::CInformacaoMidia::GetAplicativos` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp | src/ecourna/app/dados/midias/cinformacaomidia.cpp | alta |
| 9031 | 457 |  | `ecourna::app::dados::CInformacaoMidia::CInformacaoMidia(ETipoMidia, ...)` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp | src/ecourna/app/dados/midias/cinformacaomidia.cpp | alta |
| 9032 | 477 |  | `ecourna::app::dados::CDadosGeracaoMidia::CDadosGeracaoMidia(const CDadosGeracaoMidia&) (implicit)` | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.h (caminho inferido) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comentário) | média |
| 9033 | 474 |  | `ecourna::app::dados::CInformacaoMidia::CInformacaoMidia(fase, idPE, uf, turno, dadosGeracao, aplicativos) [MR]` | ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp (caminho inferido) | src/ecourna/app/dados/midias/cinformacaomidia.cpp | média |
| 9035 | 272 |  | `ecourna::app::dados::CAutenticacao::GetDataHoraFinal` | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp | src/ecourna/app/dados/midias/cautenticacao.cpp | alta |
| 9036 | 269 |  | `ecourna::app::dados::CAutenticacao::GetDataHoraInicial` | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp | src/ecourna/app/dados/midias/cautenticacao.cpp | alta |
| 9041 | 18 |  | `CBaseError<EDadosMidiasError,{3040,3065}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9042 | 263 |  | `ecourna::app::dados::CAplicativo::GetAutenticacao` | ecourna-lib/ecourna/app/dados/midias/caplicativo.cpp | src/ecourna/app/dados/midias/caplicativo.cpp | alta |
| 9043 | 297 |  | `ecourna::app::dados::CAplicativo::CAplicativo(ETipoAplicativo, const CAutenticacao&)` | ecourna-lib/ecourna/app/dados/midias/caplicativo.cpp (caminho inferido) | src/ecourna/app/dados/midias/caplicativo.cpp | média |
| 9044 | 467 |  | `ecourna::app::dados::CFederacao::CFederacao(const CFederacao&) (implicit)` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp (caminho inferido) | src/ecourna/app/dados/federacoes/cfederacao.h | média |
| 9046 | 18 |  | `CBaseError<EDadosFederacoesError,{2940,2965}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9047 | 723 |  | `ecourna::app::dados::CFederacao::CFederacao` | ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp | src/ecourna/app/dados/federacoes/cfederacao.cpp | alta |
| 9051 | 489 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DeconverteSituacaoArquivo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | alta |
| 9052 | 2100 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | alta |
| 9054 | 734 |  | `ecourna::app::dados::CDadosCifracao::CDadosCifracao(const CDadosCifracao&) (implicit)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (caminho inferido) | src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp (comentário) | média |
| 9055 | 297 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::ConverteSituacaoArquivo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | alta |
| 9056 | 2089 |  | `ecourna::app::dados::asn::CConversorResultadoUrnaCadastro::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.cpp | alta |
| 9066 | 2002 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | alta |
| 9069 | 281 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::ConverteErroLeituraBiometria` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | alta |
| 9070 | 286 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::ConverteTipoDedo` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | alta |
| 9071 | 704 |  | `ecourna::app::dados::asn::CConversorHabilitacaoBiometrica::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorhabilitacaobiometrica.cpp | alta |
| 9072 | 21 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DeconverteSituacaoReconhecimentoMesario (thunk -> 6144)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | alta |
| 9073 | 503 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | alta |
| 9074 | 41 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::ConverteSituacaoReconhecimentoMesario (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | alta |
| 9075 | 481 |  | `ecourna::app::dados::asn::CConversorEstadoHabilitacaoPorCodigo::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadohabilitacaoporcodigo.cpp | alta |
| 9077 | 21 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DeconverteSituacaoHabilitacaoAudio (thunk -> 6144)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9080 | 272 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DeconverteEstadoComparecimento` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9081 | 1946 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9083 | 41 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::ConverteSituacaoHabilitacaoAudio (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9085 | 289 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::ConverteEstadoComparecimento` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9086 | 1127 |  | `ecourna::app::dados::asn::CConversorEstadoComparecimento::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorestadocomparecimento.cpp | alta |
| 9087 | 607 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | alta |
| 9088 | 281 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::ConverteResultado` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | alta |
| 9089 | 284 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::ConverteEstado` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | alta |
| 9091 | 191 |  | `ecourna::app::dados::asn::CConversorApresentacaoFotoEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorapresentacaofotoeleitor.cpp | alta |
| 9098 | 26 |  | `ConverteLista<CConversorComparecimentoMesario> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9100 | 26 |  | `ConverteLista<CConversorIdentificacaoJustificativa> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9103 | 264 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DeconverteEstadoColetaDigital` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | alta |
| 9106 | 522 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | alta |
| 9107 | 18 |  | `CBaseError<asn::EAsnResultadoUrnaCadastroError,{2635,2665}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9108 | 43 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::ConverteEstadoColetaDigital (thunk -> 6151)` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | alta |
| 9110 | 903 |  | `ecourna::app::dados::asn::CConversorComparecimentoMesario::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentomesario.cpp | alta |
| 9116 | 652 |  | `ecourna::app::dados::asn::CConversorIdentificadorEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | src/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | alta |
| 9117 | 1268 |  | `ecourna::app::dados::asn::CConversorIdentificadorEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | src/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp | alta |
| 9123 | 27 |  | `ConverteLista<CConversorEstadoComparecimento> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9140 | 26 |  | `ConverteLista<CConversorConfiguracaoMunicipio> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9145 | 1314 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | alta |
| 9147 | 325 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::ConverterEstiloTitulo` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | alta |
| 9148 | 37 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::ConverterAlinhamentoTitulo (thunk -> 6148)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | alta |
| 9149 | 385 |  | `ecourna::app::dados::asn::CConversorTituloRelatorio::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | alta |
| 9152 | 370 |  | `std::vector<ecourna::app::dados::CTituloRelatorio>::__push_back_slow_path(CTituloRelatorio&&)` | biblioteca (instanciação libc++) | src/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (comentário) | alta |
| 9156 | 3569 | * | `ecourna::app::dados::asn::CConversorParametrosUrna::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | alta |
| 9158 | 18 |  | `CBaseError<asn::EAsnParametrizacaoUrnaError,{2560,2585}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9160 | 26 |  | `ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio>::SEQUENCE_OF(first, last) (thunk -> 2926)` | código ASN.1 gerado/de template | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp (comentário) | média |
| 9161 | 309 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterTitulos` (nome inferido; membro: `this` morto mantido na assinatura) | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | média |
| 9162 | 41 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterFormaSuspenderComVoto (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | alta |
| 9163 | 41 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::ConverterFormaSuspenderSemVoto (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | alta |
| 9164 | 3821 |  | `ecourna::app::dados::asn::CConversorParametrosUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | alta |
| 9167 | 1131 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | alta |
| 9168 | 37 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::ConverterGeneroLabel (thunk -> 6148)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | alta |
| 9169 | 949 |  | `ecourna::app::dados::asn::CConversorLabelParametrizado::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | alta |
| 9179 | 2247 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | alta |
| 9182 | 492 |  | `ModuloInformacaoMidia::InformacaoMidia::set_aplicativos (generated setter)` | código ASN.1 gerado | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9183 | 27 |  | `ConverteLista<CConversorAplicativo> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | média |
| 9185 | 560 |  | `std::__uninitialized_allocator_copy<CAplicativo> (vector<CAplicativo> copy)` | biblioteca (instanciação libc++) | src/ecourna/app/dados/midias/cinformacaomidia.cpp (comentário) | média |
| 9189 | 41 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::ConverteTipoMidia (thunk -> 2306)` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | alta |
| 9190 | 1731 |  | `ecourna::app::dados::asn::CConversorInformacaoMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp | alta |
| 9191 | 274 |  | `ecourna::app::dados::asn::CConversorTurno::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorturno.cpp | src/ecourna/app/dados/asn/cconversorturno.cpp | alta |
| 9192 | 301 |  | `ecourna::app::dados::asn::CConversorTurno::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorturno.cpp | src/ecourna/app/dados/asn/cconversorturno.cpp | alta |
| 9193 | 478 |  | `ecourna::app::dados::asn::CConversorFase::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfase.cpp | src/ecourna/app/dados/asn/cconversorfase.cpp | alta |
| 9195 | 501 |  | `ecourna::app::dados::asn::CConversorFase::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfase.cpp | src/ecourna/app/dados/asn/cconversorfase.cpp | alta |
| 9196 | 1224 |  | `ecourna::app::dados::asn::CConversorAplicativo::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | alta |
| 9198 | 43 |  | `ecourna::app::dados::asn::CConversorAplicativo::ConverteTipoAplicativo (thunk -> 6151)` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | alta |
| 9199 | 434 |  | `ecourna::app::dados::asn::CConversorAplicativo::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp | alta |
| 9210 | 26 |  | `ConverteLista<CConversorFederacao> (thunk -> 1970)` | instanciação de template | src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (comentário) | média |
| 9217 | 1048 |  | `ecourna::app::dados::asn::CConversorFederacao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp (caminho inferido) | src/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp | alta |
| 9218 | 311 | * | `ecourna::app::dados::asn::CConversorCabecalhoEntidade::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | src/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | alta |
| 9219 | 953 |  | `ecourna::app::dados::asn::CConversorCabecalhoEntidade::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | src/ecourna/app/dados/asn/cconversorcabecalhoentidade.cpp | alta |
| 9226 | 631 | * | `ecourna::app::dados::asn::CConversorFoto::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfoto.cpp | src/ecourna/app/dados/asn/cconversorfoto.cpp | alta |
| 9227 | 451 |  | `ecourna::app::dados::asn::CConversorFoto::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorfoto.cpp | src/ecourna/app/dados/asn/cconversorfoto.cpp | alta |
| 9228 | 273 |  | `ecourna::app::dados::asn::CConversorCargoID::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcargoid.cpp | src/ecourna/app/dados/asn/cconversorcargoid.cpp | alta |
| 9229 | 18 |  | `CBaseError<asn::EAsnError,{2235,2335}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |
| 9230 | 1788 | * | `ecourna::app::dados::asn::CConversorCargoID::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorcargoid.cpp | src/ecourna/app/dados/asn/cconversorcargoid.cpp | alta |
| 9256 | 1295 |  | `ecourna::app::dados::IIdentificadorEleitor::IIdentificadorEleitor` | ecourna-lib/ecourna/app/dados/iidentificadoreleitor.cpp | src/ecourna/app/dados/iidentificadoreleitor.cpp | alta |
| 9258 | 319 |  | `ecourna::app::dados::(anon)::SerialValido` | ecourna-lib/ecourna/app/dados/cserialmidia.cpp | src/ecourna/app/dados/cserialmidia.cpp | média |
| 9259 | 1057 |  | `ecourna::app::dados::CSerialMidia::CSerialMidia` | ecourna-lib/ecourna/app/dados/cserialmidia.cpp | src/ecourna/app/dados/cserialmidia.cpp | alta |
| 9260 | 295 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::GetIdentificadorPrincipal` | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp | alta |
| 9262 | 289 |  | `ecourna::app::dados::CNumeroInscricaoEleitoral::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp | média |
| 9263 | 9 |  | `ecourna::app::dados::CNumeroIdentificacaoLivre::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumeroidentificacaolivre.cpp | src/ecourna/app/dados/cnumeroidentificacaolivre.cpp | média |
| 9264 | 339 |  | `ecourna::app::dados::CNumeroCPF::GetNumeroFormatado` | ecourna-lib/ecourna/app/dados/cnumerocpf.cpp | src/ecourna/app/dados/cnumerocpf.cpp | média |
| 9266 | 18 |  | `CBaseError<EDadosError,{1935,2135}>::CBaseError (thunk -> 1011)` | thunk de exceção | src/ecourna/app/dados/dadoserros.h | alta |

---

## 15. Questões em aberto

* O nome do enumerador `case -1` nos `switch`es de enum (e das sentinelas `case 0` / `case 3`
  de `CFaseID` / `CCabecalhoEntidade::ETipoId`).
* A declaração exata de `CIdentificadorEleitor` e `CFaseID` (nomes RTTI distintos; ambos com 4–8 bytes).
* O nome do arquivo de comparecimento RC e o fluxo completo de `CGravadorRCSecao` (unidade u23).
* O nome do slot 2 da vtable de `IIdentificadorEleitor` (`GetNumeroFormatado` é inferido a partir do comportamento).
* Os nomes dos enumeradores de `ESituacaoComparecimento` (a ordem C++ faltou, semCargo, naoVotou,
  votou vem das tabelas de consulta; os nomes são inferidos).

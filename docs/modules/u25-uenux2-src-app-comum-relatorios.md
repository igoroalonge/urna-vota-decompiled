# u25 — `comum/relatorios` (relatórios impressos: Boletim de Urna, zerésima, estado da urna, helpers de relatório)

Unidade u25 da engenharia reversa de `vota_web_wasm.wasm` (aplicativo de votação VOTA do TSE, `uenux2` + `ecourna`,
build Emscripten do simulador público de treinamento). **120 funções**, 11 delas vistas executando durante os votos
gravados (as 11 são helpers que pertencem a outros arquivos: construtores de data/hora, `CAppInfo::GetInst`, cópias de
`CCargo`/`CEleicaoPE`, o conversor de eleição, as fontes de dados do relógio da tela). Nenhum código de relatório propriamente dito executou: a página web
nunca chega a um relatório impresso. Os índices de função são índices de função wasm (`python3 tools/wasmmap/q.py f <n>`).

Arquivos-fonte reconstruídos (todos sob `src/uenux2/src/app/comum/relatorios/`):

| arquivo | conteúdo |
|---|---|
| `relatoriosdefs.h` (caminho inferido) | `CRelatoriosError` = `CBaseError<EUeComumRelatoriosError, {9050, 9150}>`, lista de todos os códigos, `ETipoCabecalho` |
| `crelutil.h/.cpp` | `CRelUtil`: cabeçalho padrão, separadores, id da carga, fontes de dados do CV / id da urna, faixa de demonstração, seções agregadas; `TituloPartido`; o `std::sort` das eleições |
| `csubstituidortitulo.h/.cpp` | singleton `CSubstituidorTitulo` e `IncluiTitulos` (títulos/rodapés configuráveis do `-pu.dat`) |
| `ccalculacv.h/.cpp` | `CCalculaCV`, a cadeia do *código verificador* (classe completa) |
| `cdatasourcesrelatorio.h` | `CDataSourcesRelatorio<RDV, DSAptos>`: trailers, CVs e linhas de candidatos avaliados no momento da impressão |
| `cgeradorrelbase.h` | `CGeradorRelBase<RDV, DSAptos>`: esqueleto de relatório e `GeraRelatorio(path)` |
| `cgeradorbubase.h` | `CGeradorBUBase<RDV, DSAptos>`: os blocos por cargo do BU |
| `cgeradorbu.h/.cpp` | `CGeradorBU`: as 13 partes do BU, QR codes, publicação do CV, *histórico de cargas* |
| `cgeradorbuqrcode.h/.cpp` | destrutor de `CGeradorBUQRCode` e o bloco `APTA/APTS/APTT` (o montador do payload em si: `cgeradorbuqrcode.u04-fragment.cpp`) |
| `ccabecalhoqrcodebuilder.cpp` | verificações de campos obrigatórios do cabeçalho do QR em `preBuild()` |
| `cpartecandidatos.h/.cpp` | partes de relatório da lista de candidatos da zerésima |
| `crelatoriotesteimpressora.h/.cpp` | os hooks do relatório "ESTADO DA URNA", `CQRCodeDS`, fontes de dados de data/hora |
| `csigverifier.h/.cpp` | verificação de assinatura de um arquivo de relatório antes da impressão |
| `cgeradorrellistaeleitores.cpp` | `SituacaoEleitor` (flags da lista de eleitores) |
| `../u25-foreign-fragments.cpp`, `../../vota/u25-foreign-fragments.cpp` | funções atribuídas a esta unidade que pertencem a outros arquivos (§10) |

Capítulos relacionados: docs/10-boletim-de-urna.md, docs/bu/codepath.md
(o encerramento executado de verdade: `samples/bu-real/run-full/reports/*.txt` são as saídas do código desta unidade),
docs/bu/codigo-verificador.md (a cadeia do CV reimplementada e verificada),
docs/bu/qrcode.md (payloads dos QR), [u08](u08-uenux2-src-app-vota-eleitor.md) (`vota::CGeraBU`,
`CGeraRelatorios`, que inlinam a maioria dos construtores desta unidade), [u04](u04-uenux2-src-app-comum-dados.md)
(`CCargos`, o gerador de payload do QR), [u23](u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md)
(o gravador ASN.1 do `-bu.dat`, que *não* é esta unidade).

## 0. Palavras

| termo | significado |
|---|---|
| *relatório* | relatório impresso pela impressora térmica da urna. Os relatórios são primeiro renderizados em um arquivo (`dinamico/trab1/<nome>.dat`), assinados (`.vsu`), e as *vias* são impressas a partir desse arquivo |
| *BU*, *boletim de urna* | a apuração da seção, impressa no *encerramento*. (O `-bu.dat` ASN.1 assinado para o TSE é gravado por `CGravadorBU`, unidade u23; esta unidade faz o BU **impresso**, que vira `-imgbu.dat`) |
| *zerésima* | relatório impresso antes da abertura da votação, que comprova que todos os candidatos têm zero votos |
| *via* | uma cópia impressa (`"{}ª via"`) |
| *código verificador* (CV) | código MAC de 10 dígitos impresso sob cada bloco do BU (cadeia SipHash-4-6, `CCalculaCV`) |
| *carga*, *código de carga*, *histórico de cargas* | a carga dos dados da eleição nesta urna (id de 24 dígitos impresso como `123.456.789...`) e a lista de cargas registradas em `gap.bin` |
| *correspondência* | o pareamento urna ↔ carga ↔ seção (`CDadoCorrespondencia`, `CEstadoGeral +60`); o seu *resumo* é impresso em todo cabeçalho |
| *seção agregada* | outra seção eleitoral incorporada à seção desta urna |
| *apto*, *originais*, *temporários (TTE)* | eleitores aptos: da própria seção / por transferência temporária |
| *legenda* | voto só no partido (cargos proporcionais) |
| *consulta* | pergunta de referendo (um "cargo" cujas opções são *respostas*) |
| *contingência* | urna substituta; ela carrega os dados dos dois turnos |
| *MRJ*, *mesa receptora* | mesa receptora (urnas de justificativa); o cabeçalho imprime *Mesa Receptora* / *Urna* |
| *fase* | `EUrnaFase` `'1'` oficial, `'2'` simulado, `'3'` treinamento |
| *PU* | `-pu.dat`, `EntidadeParametrizacaoUrna`: títulos/rodapés de relatórios, número de vias, rótulos, flags |
| *SAVD* | daemon de assinatura / verificação (simulado no build web) |

## 1. O que o subsistema faz

`comum/relatorios` renderiza os relatórios em papel da urna. Ele não conversa com a impressora: todo relatório é
montado como uma lista de formulários `api::IForm<api::IPaper>` com `api::CPaperFormBuilder`
(`AddText(text, font, alignment)` = `shared_f193`, `AddNewLine(n)` = 198, `AddData(&source, 0)` = 604,
`Build()` = `shared_f357`) e impresso em `api::IPaperRelatorios`, um "papel" que, na urna real, grava o
fluxo de bytes da impressora em um arquivo (`trab1/bu.dat`, `ze.dat`, `buj.dat`, ...). Os estados do `vota` então assinam o arquivo
(SAVD), verificam a assinatura (`CSigVerifier`) e imprimem as *vias* a partir dele.

Dois mecanismos definem a unidade inteira:

1. **Fontes de dados avaliadas no momento da impressão.** Muitas linhas são campos `api::CDataText<std::string(*)()>`: um ponteiro
   de função (um *slot da tabela de funções*, por exemplo o slot 1633 = `CodVerificador`) chamado quando o formulário é impresso. Os
   geradores de relatório percorrem `CCargos` (cargo corrente), `CPartidos` (partido corrente) e `CCandidaturas` (candidatura
   corrente, posicionada por `Localiza`) e imprimem o mesmo formulário repetidas vezes; a cada vez, a fonte de dados lê
   as entradas *correntes* e o RDV em memória (`CRdvVota`). Assim, os números impressos vêm direto do RDV;
   não há uma etapa de "contagem" separada.
2. **A cadeia do código verificador.** Enquanto o BU é gerado, um `CCalculaCV` é publicado em
   `api::CPolySingletonList`. Cada bloco acrescenta os números que imprime (`IncluiString`) *antes* que o formulário que
   contém o seu CV seja impresso; a fonte de dados do CV (`CodVerificador`, `DSCodigoVerificador`,
   `TrailerProporcionalPartido`) chama `Calcula()`, que calcula o MAC do buffer e recomeça a cadeia a partir do resultado.

## 2. Classes (RTTI) e como elas se relacionam

```
comum::CGeradorRelBase<CRdvVota, CEleitores>            vtable @1575404  [0] 5609  [2..5] pure
  └─ comum::CGeradorBUBase<CRdvVota, CEleitores>        vtable @1575372  [0] 5612  [3] 11255 [4] 11254 [5] 11253
       └─ comum::CGeradorBU                             vtable @1575164  [0] 3698 [1] 11258 [2] 11256
comum::CGeradorBUQRCode                                 vtable @1575920  [0] 2251  [2][3] pure
  └─ comum::CGeradorBUQRCodeVota                        vtable @1575984  [0] 11244 [1] 11243 [2] 11242 [3] 11241
api::IReportPart
  ├─ comum::CParteCandidatos                            vtable @1576396  [2] 11215
  ├─ comum::CParteCandidatosMajoritarios                vtable @1576416  [2] 11214
  ├─ comum::CParteCandidatosProporcionais               vtable @1576452  [2] 11213 (u04)
  └─ comum::CParteCandidatosConsultas                   vtable @1576472  [2] 11212
comum::CImprimirExtratoCarga (template-method base, Imprime = vota_f5591)
  └─ comum::CRelatorioTesteImpressora                   vtable @1576660  26 slots (11183..11202, 11216)
api::ISigVerifier
  └─ comum::CSigVerifier                                vtable @1576880  [0] 11178 [1] 11176 [2] 11179
no vtable: comum::CRelUtil (static helpers), comum::CCalculaCV (56 B), comum::CSubstituidorTitulo (singleton,
           12 B), comum::CDataSourcesRelatorio<RDV, DSAptos> (static data sources), comum::CCabecalhoQRCodeBuilder,
           comum::CQRCodeDS (functor in std::function<std::string()>, vtable of the __func @1538332)
```

Os layouts estão nos headers (`// +offset`). Os mais importantes:
`CGeradorRelBase` = {vptr, header +4, trailer +12, qrcode +20, linhaVazia +28} (4 `shared_ptr<IForm>`);
`CGeradorBUBase` acrescenta 10 partes em +36..+108 (headerProporcional, headerProporcionalPartido,
partidoApenasVotoLegenda, trailerProporcionalPartido, cargoSemCandidato, trailerCargoSemCandidato,
trailerProporcional, headerMajoritario, detalheCandidato, trailerMajoritario); `CGeradorBU` acrescenta
`std::vector<std::string> m_historicoCargas` em +116.

## 3. Quem chama esta unidade

| chamador (outras unidades) | usa |
|---|---|
| `vota::CGeraBU::StartState` (12110, u08) | constrói `CGeradorBU` (inlinado), `GeraRelatorio(trab1/bu.dat)`, `GetSeparadorFase`, `IncluiCabecalhoEleicoesMZS`, `GetIDCargaFormatado`, `DSCodigoIdentificacaoUE`, as fontes de dados, `exists<CCalculaCV>` |
| `vota::CGeraRelatorios::StartState` (12105, u08) | cabeçalhos de BUJ / BIM / BEHB (`IncluiCabecalhoEleicoesMZS`, `GetSeparadorFase`, `GetIDCargaFormatado`) |
| `vota::CGeraZeresimaBase` / `CGeraResumoZeresimaBase` (11946 / 11943, u09) | `CParteCandidatos*`, `DetalheCandidatoZE`, trailers, `TituloPartido`, `IncluiTitulos` (rodapé), `GetSeparadorFase` |
| `vota::CImprimindoBU`, `CImprimindoZeresima`, `CImprimirBJust`, `CImprimindoBim`, `CImprimindoBEHB` | `CSigVerifier(trab, "<rel>.dat", "<rel>.vsu").Verify()` antes de imprimir cada via |
| `vota::CVerificaHorarioZeresima` (11869), `vota::CImpressaoEstadoUrna` (11911) | `CRelatorioTesteImpressora` + `vota_f5591` ("ESTADO DA URNA") |
| `vota::CImpressaoPU` (11902), `CImpressaoListaEleitores` (11908), `CGeradorRelVersaoPacoteDados` (11223) | cabeçalho, `SituacaoEleitor` |
| telas do eleitor (`CTelasVota`, thread do operador 5343) | `FormataDataAtual` / `FormataHoraAtual` (slots 1103/1104) — as únicas funções de relatório que executam no build web |

## 4. O cabeçalho padrão de relatório: `CRelUtil::IncluiCabecalhoEleicoesMZS` (1543, crelutil.cpp:307)

Parâmetros: `(builder, titulo, municipio, zona, secao, nomeMunicipio, ETipoCabecalho tipo, bool usaPleitoContingencia)`.

1. Cópia de `CConfiguracaoEleicao::GetPleito()`. Se `usaPleitoContingencia && tipo == CONTINGENCIA`: garante que
   `CPE` existe (carrega o processo eleitoral) e, se ele tiver um pleito 2 cuja data seja hoje ou anterior, usa esse pleito.
2. `IncluiAvisoModoDemonstracao` (5583): `"IMPRESSO EM MODO DEMONSTRAÇÃO"` + 2 linhas em branco no modo de demonstração.
3. `CSubstituidorTitulo::IncluiTitulos(b, PU.cabecalho, UF)` (3689): cada `TituloRelatorio{alinhamento, estilo,
   texto}` do `-pu.dat` é traduzido (`CTradutorFrase::TraduzLabel`), quebrado em `'\n'`, com `"<uf>"` substituído pela
   UF da seção; fonte 2 quando *expandido*, alinhamento à esquerda/direita/centro. O singleton de substituição é criado na
   primeira chamada, com a UF dessa chamada.
4. linha em branco; `titulo` (centralizado) + linha em branco quando `titulo` não está vazio; `"Eleições Comunitárias"` se
   `CConfiguracaoEleicao +20 == 2`;
   nome do processo eleitoral (`+8`), nome do pleito, `"(DD/MM/YYYY)"`; linha em branco.
5. Se o pleito tem ≥ 2 eleições: os seus nomes ordenados por `CSituacoesEleicoes::ordemImpressao` + linha em branco.
6. `"{:<32s} {:05}"` município (rótulo `<MLSN>`), nome do município (centralizado), linha em branco, `"{:<33s} {:04}"` zona (`<ZLSN>`).
7. `switch (tipo)`: `SECAO` → `"Local de Votação"` (não para comunitárias), seção (`<SLSN>`) e, se houver
   agregadas, `"Quantidade de seções agregadas {:04}"` + `"Seções agregadas: 0012 0034"`; `MESA_RECEPTORA` →
   `"Mesa Receptora {secao/10:03}"`, `"Urna   {secao%10}"`; `CONTINGENCIA` → `"Urna de contingência"`;
   `3` → lança 9086 `"Tipo inválido: {}"`; qualquer valor acima de 3 não imprime nada. Depois, uma linha em branco.

Rótulos como `<MLSN>` são traduzidos com os rótulos parametrizados da PU (`labelMunicipio/Zona/Secao/Partido`:
gênero, longo/curto, singular/plural) — `<ZLSN>` → "Zona Eleitoral", `<PLSN>` → "Partido", `<SCPB>` → plural...
Os chamadores passam `tipo` 0 (BU, zerésima, BUJ, lista de eleitores), `2`/`0` conforme o tipo de urna (PU, versões de pacotes,
com `usaPleitoContingencia = true`) e, no gerador do BIM (12105), `!flag` (então 1 = layout de *Mesa Receptora*).

Outros helpers: `GetSeparadorFase` (1919, :554) → `"======================================"` de 38 colunas (oficial),
`"===============SIMULADO==============="`, `"==============TREINAMENTO============="`,
`"============DEMONSTRAÇÃO=============="` no modo de demonstração, 9087 `"Fase inválida: {}"` caso contrário.
`GetIDCargaFormatado` (2786, :69): exige > 20 caracteres (9085), grupos de 3 com pontos.
`FormataSecoesAgregadas` (5738), `DSCodigoIdentificacaoUE` (1942: `"Código identificação UE       {:08}"`,
`CEstadoGeral +60`), `TituloPartido` (11181, :58: `"<PLSN>: NN - SIGLA"`, 9084 se não houver partido corrente).

## 5. O Boletim de Urna, passo a passo

Esta é a ordem em que o código desta unidade executa quando `vota::CGeraBU::StartState` (12110) gera o BU no
*encerramento* (estado `EAVGERARBU` = 59). Saída real deste código: `samples/bu-real/run-full/reports/bu.txt`.

### 5.1 Construção de `CGeradorBU` (inlinada na 12110, fonte cgeradorbu.cpp)

`CGeradorBU(titulo = "Boletim de Urna", municipio (eg +20), zona (+24), secao (+26), nomeMunicipio,
correspondencia (eg +60), historicoCargas (codes of every CDadoCorrespondencia of gap.bin), dtHrEmissaoBU,
separadorFase)` monta 13 formulários e os entrega a `CGeradorBUBase` → `CGeradorRelBase`:

| parte | conteúdo |
|---|---|
| header (`CriaHeader`) | cabeçalho padrão (§4, `SECAO`), `"Eleitores aptos {:04}"` + originais/temporários (`CRelUtil::FormataQtdAptos` sobre `CEleitores::GetQtdAptos`), `"Comparecimento {:04}"` (`CRdvVota::Comparecimento`), quando a urna é biométrica: as contagens de habilitação biométrica / biográfica / sem biometria, `"Eleitores faltosos"` = aptos − comparecimento, linha em branco, `"Código identificação UE {:08}"`, se o estado do vota existe a data e a hora de abertura/encerramento (`GetDtHrInicioAquisicao/FimAquisicao`, erros 8090/8091), `"RESUMO DA CORRESPONDÊNCIA"` + código, **campo do CV (slot 1633)**, linha em branco |
| trailer | separador de fase, linha em branco, `"Código de identificação da carga"` + `GetIDCargaFormatado(codigoCarga)`, linha em branco, `"Ver: 10.23.0.1"`, linha em branco, `IncluiTitulos(PU.rodapeBUVOTA)` (por exemplo "O conteúdo deste BU poderá ser / conferido no endereço / resultados.tse.jus.br" / ASSINATURAS / PRESIDENTE / MESÁRIOS / FISCAIS), 20 linhas em branco, corte do papel |
| qrcode (`CriaQRCode`, :124) | modo de demonstração: `"DEMONSTRAÇÃO PRÉ-ELEIÇÃO"`, `"NÃO HÁ QR CODE"`; senão, se `PU.imprimirQrCodeNoBU` (cfg +486): `"============= BU DIGITAL ============="`, para cada payload de `CGeradorBUQRCodeVota(cabecalho{zona, seção, idUE, código de carga, histórico}, comparecimento, dtEmissão).GeraQRCodes(1100)`: `"-------------- 01 / 02 ---------------"` + bitmap do QR; `"======== CERTIFICADO DIGITAL ========="` + QRs do certificado (`QRCE/IDUE/MDUE/CERT`); `"ASSINATURA BU DIGITAL: <hex>"`; caso contrário, nenhum formulário de QR (nulo) |
| headerProporcional | separador de fase, linha em branco, nome do cargo em maiúsculas centralizado entre `-` (38) |
| headerProporcionalPartido | slot de dados 2921: `"Partido: 91 - PEsp\n"` + `"Nome do candidato       Num cand Votos"` quando o partido tem votos nominais |
| partidoApenasVotoLegenda | `"Não há votos nominais"` |
| trailerProporcionalPartido | slot de dados 2922 `TrailerProporcionalPartido` (legenda, total do partido, **o seu próprio CV**) |
| cargoSemCandidato | `"Não há candidatos concorrendo"` |
| trailerCargoSemCandidato | 38 `-`, dados 1632 (aptos + `"Comparecimento {:04}"`), CV (1633) |
| trailerProporcional | 38 `-`, dados 1625 `TrailerProporcional`, CV (1633) |
| headerMajoritario | separador de fase, linha em branco, nome do cargo, dados 2923 (cabeçalho de colunas quando há votos nominais) |
| detalheCandidato | dados 1636: `"  <nome 23 col.>  <número>  <votos:04>"` |
| trailerMajoritario | linha em branco, 38 `-`, dados 1626 `TrailerMajoritario`, CV (1633) |

Verificações: `CGeradorRelBase` lança 9066/9067 para um header/trailer nulo (h:48/51), `CGeradorBUBase` 9055..9064 para uma
parte nula (h:66..111). Depois, o corpo do construtor:

1. `CCargos::GetInst().Inicio()`; se o cursor estiver no último cargo, `Next()` (assim o CV do cabeçalho não recebe as
   strings "finais").
2. identificação = `std::format("{:05}{:04}{:04}{}{}{}{}", municipio, zona, secao,
   std::format("{:05}{:05}{}", cfg.idProcesso, pleito.id, pleito.data "YYYYMMDD"), numeroInternoUrna, pleito.id,
   faseChar 'o'/'s'/'t')`.
3. `CPolySingletonList::push(std::make_unique<CCalculaCV>(identificacao, "0000000000", DefineTipo(demo) ('A' demo /
   'F'), 16))`. O construtor de `CCalculaCV` (ccalculacv.cpp:58/65/74) valida a identificação
   (≥ 13 caracteres de `0-9sodtSODT`, 9051) e o primeiro CV (dígitos, 9052), lê a chave de 16 bytes de
   `/dsk/fi/estatico/chave/cv.ber.pri` (`EntidadeChave`, decifrada com a chave do IKernelHSM; 9053 se for mais curta) e
   inicia o buffer com `tipo + identificação + "0000000000"`.

### 5.2 `GeraRelatorio("/dsk/fi/dinamico/trab1/bu.dat")` (cgeradorrelbase.h:61)

1. `IPaperRelatorios::instance().Abre(path)` (slot 5).
2. **Cabeçalho** impresso → o seu campo de CV chama `CodVerificador` (11974): `Calcula()` sobre
   `F|identificação|0000000000|"1"` → `"Código Verificador: d.ddd.ddd.ddd"`.
3. **`ImprimePreTexto`** (11256, cgeradorbu.cpp:257): `"--------"`, `"Histórico de código de carga"`, `"--------"`,
   `"1: 123.456.789.012.345.678.901.234"` para cada carga, linha em branco, campo de CV (slot 3048 `DSCodigoVerificador`), linha em branco.
   Antes de imprimir, ele acrescenta `código₁@código₂@...@` à cadeia (um histórico vazio lançaria 9054).
4. `CCargos::OrdenaPorOrdemImpressao()`, cursor no primeiro cargo; para cada cargo: uma faixa
   `"====" / <nome da eleição> / "===="` quando a eleição muda (a comparação começa com a eleição do
   *último* cargo, então um BU de uma única eleição não tem faixa), depois por tipo:
   * **proporcional** (`ImprimeProporcionalPartido`, 11255, h:187): cabeçalho; se o cargo não tem candidatura apta:
     cadeia += `{cargo:02}{total:04}`, linha em branco, "Não há candidatos concorrendo", trailerCargoSemCandidato. Senão,
     para cada partido (em ordem crescente) com `RDV.Partido(cargo, p) > 0`: cadeia += `{cargo:02}{partido:02}`; cabeçalho do partido;
     para cada candidato apto do partido (`GetNumerosCandidatosAptos`, em ordem crescente) com votos > 0: cadeia +=
     `{numero:05}{votos:04}` e uma linha de candidato; "Não há votos nominais" se não houver nenhum; trailer do partido
     (`TrailerProporcionalPartido`, h:188: `"    Votos de legenda {:04}"`, `"    Total do partido {:04}"`, cadeia +=
     `{legenda:04}{total:04}` [+ strings finais se for o último cargo], CV). Depois `trailerProporcional`
     (`TrailerProporcional`, h:241: cadeia += `{nominais}{legendas}{brancos}{nulos}{total}` (cada um `{:04}`),
     linhas de aptos, "Total de votos Nominais", "Total de votos de Legenda", "Brancos", "Nulos", "Total Apurado") + CV.
   * **majoritário** (`ImprimeMajoritario`, 11254, h:246): cabeçalho; cadeia += `{cargo:02}`; sem candidatos →
     `{cargo:02}{total:04}` + "Não há candidatos concorrendo" + trailer; senão, cada candidatura do cargo com
     votos (na ordem de CCandidaturas) → cadeia += `{numero:05}{votos:04}` + linha de candidato; `trailerMajoritario`
     (`TrailerMajoritario`, h:305: cadeia += `{nominais}{brancos}{nulos}{total}`, aptos, totais) + CV.
   * **consulta** (`ImprimeConsulta`, 11253, h:286): cabeçalho majoritário; cadeia += `{cargo:02}`; as respostas de
     `CDetalheConsulta` ordenadas por número; cada uma com votos → cadeia += `{numero:05}{votos:04}` e uma linha ad hoc
     `"  " + resposta (26 col.) + número (4 col.) + "  " + votos:04`; `trailerMajoritario` + CV.
   As "strings finais" (`IncluiFinal(cargos, calculadora)`, 5967) são acrescentadas a todo CV calculado enquanto o
   cursor de `CCargos` está no **último** cargo (cada trailer de partido de um último cargo proporcional, e o CV do
   próprio trailer do cargo): o valor anterior de 64 bits, a data do pleito `YYYYMMDD` e `to_string(code)` de cada
   cargo na ordem de impressão. A 5967 recebe `(const CCargos&, CCalculaCV&)` nessa ordem, então não é um
   membro de `CCalculaCV` (um membro receberia a calculadora primeiro).
5. `m_qrcode->Imprime()` se presente; trailer; `Fecha()` (slot 6); `CSynchronizer::CreateInst()->Sincroniza()`.

`~CGeradorBU` (3698) apaga a calculadora de `CPolySingletonList`. `CGeraBU` então salva o estado e
`CSincronizaVota::SincronizaRelatorios("bu.dat")` o copia/assina (`bu.vsu`). Cada via é impressa mais tarde por
`CImprimindoBU::ImprimeBU` depois de `CSigVerifier(trab, "bu.dat", "bu.vsu").Verify()` (11179), que inlina
`comum::ValidarUE(savd, aplicação, pacote, arquivo)` (iinterfacesavd.cpp:898, a sobrecarga de 4 argumentos): o SAVD
precisa confirmar que `trab/bu.vsu` cobre `bu.dat` (chave 53, requisição montada pela func 5892), ou o erro 7324 "Falha ao validar
assinatura UE ..." é lançado.

O que o CV cobre e não cobre é analisado em codigo-verificador.md §3.4
(aptos, comparecimento, horários, nomes e QR **não** estão na cadeia).

## 6. Zerésima e partes de resumo

`CParteCandidatos::Imprime` (11215) escolhe, para o cargo corrente: *sem candidatos* (o cargo tem detalhes de
candidatos e nenhuma candidatura apta), *majoritários*, *proporcionais* (u04: 11213) ou *consultas* (`CCargo +136`).
`CParteCandidatosMajoritarios::Imprime` (11214, :75) imprime uma linha de detalhe por candidato apto depois de posicionar
a candidatura (9081 se `Localiza` falhar); `CParteCandidatosConsultas::Imprime` (11212, :174) coleta os números das
respostas de `CRespostas` para o cargo (chave `cargo*1000000+n`), ordena-os e os imprime (9083).
`DetalheCandidatoZE` (11973, h:152) é a linha `"  Golfe                          91001"`: nome cortado em 36,
preenchido até 31, nomes de 32–36 caracteres continuados em uma segunda linha (`------\n  ------------------------------>`),
número alinhado à direita; ele afirma que o RDV tem **0** votos para o candidato (`Assert (qtd == 0u)`, 3410).

## 7. "ESTADO DA URNA" (`CRelatorioTesteImpressora`)

Um template method (`vota_f5591`) imprime o relatório por meio de 24 hooks virtuais; `CRelatorioTesteImpressora`
os implementa para o VOTA: título `ESTADO DA URNA`, `UE DE VOTAÇÃO|CONTINGÊNCIA` (teste sem sinal sobre o tipo de urna
do turno: `'1'`, `'3'`, `'4'` → VOTAÇÃO; `'2'` e qualquer outro valor, `'0'` incluído, → CONTINGÊNCIA), UF,
município/zona, ids, `GetLinhasLocal` (11188; só para os tipos VOTAÇÃO: local (só oficial), seção, as duas
linhas de *agregadas* **só quando a seção tem seções agregadas**, tipo de local *Normal/Voto em Trânsito/Preso
provisório/Temporário/Inválido*, biometria *Sim/Não*; sempre: *Tipo de alimentação* vindo de `api::IPower`: *R. Elétrica /
B. Interna / B. Externa / Não identificada*), data/hora de emissão, resumo da correspondência, período de horário de verão (omitido em
treinamento), um QR code (`GetConteudoQRCode`: campos da carga + `ORIG` + `VERS/DTAG/HRAG/DTPL/HRVR` vindos de
`CQRCodeDS`), versão, `"URNA OPERANDO EM PERFEITAS" / "CONDIÇÕES DE FUNCIONAMENTO"`, assinaturas.

## 8. Particularidades do build web

* A impressora e o papel são `simulador::CWasmNullPrinter` / `CWasmNullPaper` (descartam tudo), e a página
  nunca chega ao encerramento, à zerésima nem aos relatórios de "Mais informações": nada das §4–§7 executou nas
  sessões gravadas. O harness modificado de codepath.md os executa de verdade.
* `CSigVerifier::Verify` conversa com `(anonymous)::CWasmSavd`, que responde "OK" a toda requisição: a verificação da
  assinatura do relatório é simulada (não pode falhar).
* O arquivo da chave do CV `/dsk/fi/estatico/chave/cv.ber.pri` não existe nos cenários públicos (o harness semeia um
  falso); o construtor de `CCalculaCV` lançaria uma exceção quando o BU fosse gerado sem ele.
* `FormataDataAtual` / `FormataHoraAtual` (5472/5473) são as únicas funções dos arquivos de relatório que executam: elas
  são as fontes de dados da data/hora do cabeçalho de status das telas (por meio de `CDataTextFmt`, slots 1103/1104).

## 9. Observações sobre WebAssembly / Emscripten

* **O inlining do LTO esconde os construtores.** `CGeradorBU`, `CGeradorBUBase`, `CGeradorRelBase`, `CCalculaCV`,
  `CSubstituidorTitulo::GetInst/CreateInst` e `GeraRelatorio` só existem dentro dos seus chamadores (12110, 3689). Os seus
  registros `std::source_location` (`cgeradorbubase.h:66..111`, `cgeradorrelbase.h:48/51/61`, `ccalculacv.cpp:58..74`,
  `csubstituidortitulo.cpp:21/26`) são o único rastro, e é por isso que as ferramentas nomearam a func 3689
  "CSubstituidorTitulo::GetInst" e a func 3696 "CGeradorBUQRCode::GetQtdAptos".
* **Uma ordenação de 97 KB.** O cabeçalho ordena as eleições com uma lambda que recebe `md::CEleicaoPE` **por valor**, então cada
  comparação faz cópia profunda de duas eleições (com todos os seus cargos de 140 bytes). A instanciação resultante de `std::__introsort`
  (5582) tem 96.893 bytes — a maior função da unidade — mais 5577/5578/3687/1386.
* **Dois sabores de ordenação.** O `std::sort` de inteiros simples (11212) instancia as redes de ordenação sem desvios e a
  partição por bitset da libc++ (4816 + 4809/4810/4813/4814/4815/1506/2578); as ordenações com comparador (eleições, cargos pelo
  slot de ponteiro de função 2363, respostas) usam as clássicas.
* **Corpos mesclados.** `CParteCandidatosMajoritarios` e `CParteCandidatosConsultas` compartilham os seus destrutores (5589 /
  5588), o construtor de erro de relatorios é um thunk de 3 instruções (580) sobre o corpo compartilhado de `CBaseError`
  (ecourna_f710), `exists<CCalculaCV>` (1954) é um thunk do corpo de nome de 20 caracteres 6057.
* **Strings de formato.** A maior parte do texto é produzida por `std::format` com strings de tempo de compilação (por exemplo `"{:<33s} {:04}"`);
  os rótulos traduzidos viram strings de formato de tempo de execução e são impressos com `std::vformat` (o laço que procura
  `'{'`/`'}'` é `__try_constant_folding`). Argumentos de enum (`EUrnaFase`, `ETipoCabecalho`) são formatados por meio de um
  handle de `std::formatter` (slots 2285/3047 → corpo 536).
* Nomes de segmentos de dados no pseudocódigo: `d_operator0033s040504040x0505T[N]` fica no endereço absoluto `N + 1024`.

## 10. Funções da unidade que pertencem a outros arquivos

O montador de unidades colocou 41 helpers aqui (mais o move/swap de `CEleicaoPE` 817/818 usado pela ordenação das eleições) porque os seus chamadores são código de relatório. Eles estão reconstruídos ou descritos em
`src/uenux2/src/app/comum/u25-foreign-fragments.cpp` e `src/uenux2/src/app/vota/u25-foreign-fragments.cpp`:
`CAppInfo` (185 GetInst, 5813, 3792, 11574), `CLocal` (2261, 11508), ctor de `CInformacaoEleicao` (603), `CPE::Existe`
(2788), `CCandidaturas::PossuiCandidatos` (2271), `CConfiguracaoEleicao::GetSituacoesEleicoes` (271) e
`GetCargos(eleição, ordenados)` (3775) com a sua ordenação (5789, 5785, 2829, 320, 761, 783, 5784), membros especiais
implícitos de `CDetalheCandidato` (365, 242), `CDetalheConsulta` (374, 267), `CEleicaoPE` (730, 817, 818, 1953, 5595,
5777), `CPleito` (1281, 5597), `vector<CCargo>` (2289), `vector<int>::assign` (3693), o conversor de eleição
`CConversorEleicaoPE::DoDesconverte` (11371, observado executando), `exists<CCalculaCV>` (1954), `api::CDate()`
(1382), `CTime()` (2230), `CDateTime(time_t)` (1000); e no vota: `make_shared<CDataText<DS_NomeCargoNeutroComEscolha>>`
(6627, observado) e os seus destrutores (12588/12589), `CMenuFiltrarCandidatosPorNumero::StartState` (11892),
`CImpressaoPU::StartState` (11902, relatório "Parâmetros de urna") e o seu formatador de horário (2686).

### Notas para outras unidades (encontradas durante a leitura destas funções)

* `CEleicaoPE +40` é preenchido por `CConversorEleicaoPE` (11371) a partir de `EntidadeEleicao.municipios`, não de uma lista de
  turnos (os comentários de `EleicoesDoTurno`/`CriaPleito` da u02 supõem turnos).
* `comum_f3871` é `CDataSourcesRelatorio::GetLinhasEleitoresAptos` (ela armazena a vtable `__func` de `GetLinhasEleitoresAptos()::lambda`
  @1543880), não um "separador" (fragmento da u04).
* `CGeradorBUQRCode::AptosCargo` (3696) sempre grava `APTA`, `APTS` **e** `APTT` (o comentário da u04 mostra os dois últimos
  como opcionais).
* `GetSeparadorFase`: `'1'` → linha simples (oficial), `'2'` → SIMULADO, `'3'` → TREINAMENTO (o comentário no
  `cgerabu.cpp` da u08 troca `'1'` e `'2'`).
* `vota_f5591` (arquivo das ferramentas `cverificahorariozeresima.cpp`) é `CImprimirExtratoCarga::Imprime`; `comum_f5586` é
  `CRelatorioTesteImpressora::CRelatorioTesteImpressora`.
* `docs/bu/codigo-verificador.md` §2 escreve a função das "strings finais" (5967) como um membro de `CCalculaCV`
  `«IncluiFinal»(const CCargos&)`. Os seus parâmetros wasm são `(CCargos, CCalculaCV)` nessa ordem (chamadores 11260 e
  11974), então ela é uma função livre (ou um static de outra classe) que recebe os cargos primeiro. O algoritmo descrito lá não é
  afetado.
* O construtor de `CInformacaoEleicao` (603) armazena o seu argumento como está; o `+88` (o `CParametrosUrna` dentro de
  `CConfiguracaoEleicao`) é somado por cada chamador. O comentário da u23 "`m_parametros = &cfg +88`" dentro da 603 é
  impreciso: o parâmetro é o subobjeto de parâmetros.
* `comum_f5584` (= `IncluiTitulos(b, lista, CLocal::GetInst().GetUF())`) atende tanto o rodapé do BU (12110) quanto o
  cabeçalho de ESTADO DA URNA (`vota_f5591`, hook [11]).

## 11. Código suspeito / digno de nota

| wasm | achado | impacto |
|---|---|---|
| 11179 | A verificação da assinatura do relatório é simulada no build web (o mock do SAVD sempre responde OK) | só no simulador; a urna real usa o seu SAVD |
| 11256 | `IncluiString("")` lança 9054 quando o *histórico de cargas* está vazio, abortando a geração do BU | na urna real, só se o gap.bin não tivesse correspondência (não esperado) |
| 11974 | `CodVerificador` não testa `exists<CCalculaCV>()` (os trailers testam); imprimir um formulário que contenha o slot 1633 sem uma calculadora publicada lança um erro de PolySingleton | só se um formulário do BU fosse reutilizado fora de `CGeraBU` |
| 1543 | `ETipoCabecalho` 3 lança "Tipo inválido", valores > 3 silenciosamente não imprimem bloco de seção | latente; nenhum chamador passa > 2 |
| 3689 | `CSubstituidorTitulo` é criado uma vez, com a UF da primeira chamada; chamadas posteriores com outra UF mantêm a primeira | latente (uma UF por processo) |
| 5582 | comparador por valor: O(n log n) cópias profundas de eleições, 97 KB de código | apenas desempenho / tamanho de código |
| 5782 | `std::regex` compilado a cada chamada; `'.'` sem escape em `"[0-9]+.[0-9]+.[0-9]+.[0-9]+"`; `VERS` recai em `0.0.0.0` | cosmético |
| 11233 | `38 - (id + flags)` sofre underflow para um tamanho enorme → `std::length_error` se a identidade mais as flags excederem 38 caracteres | inalcançável com dados reais (≤ 27) |
| 2786 | o id da carga só é verificado quanto a comprimento > 20 (não 24 dígitos) | cosmético |
| 11255 | o resultado de `Localiza` não é verificado contra nulo antes da leitura do número do candidato | inalcançável (os números vêm do mesmo mapa) |
| 11190 / 11188 | o teste do tipo de urna é sem sinal: `'0'` (*sem tipo*) ou qualquer valor inesperado é informado como `UE DE CONTINGÊNCIA` e o bloco do local é omitido | cosmético; uma urna carregada sempre tem um tipo |

## 12. Tabela de mapeamento completa (120 funções)

"executou" = visto executando nos votos gravados. "reconstrução" = arquivo deste repositório em que a função está
escrita ou explicada.

| wasm | bytes | executou | símbolo reconstruído | arquivo original | reconstrução |
|---:|---:|:-:|---|---|---|
| 185 | 85 | sim | `comum::CAppInfo::GetInst` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/appinfo/cappinfo.h (u20) |
| 242 | 138 |  | `comum::md::CDetalheCandidato::~CDetalheCandidato` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 267 | 214 |  | `comum::md::CDetalheConsulta::~CDetalheConsulta` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 271 | 12 |  | `comum::CConfiguracaoEleicao::GetSituacoesEleicoes` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 320 | 1110 |  | `std::swap<comum::md::CCargo>` | libc++ (std::sort em cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 365 | 369 | sim | `comum::md::CDetalheCandidato::CDetalheCandidato(const CDetalheCandidato&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 374 | 211 |  | `comum::md::CDetalheConsulta::CDetalheConsulta(const CDetalheConsulta&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 580 | 18 |  | `comum::CRelatoriosError::CRelatoriosError` | uenux2/src/app/comum/relatorios/relatoriosdefs.h (caminho inferido) | src/uenux2/src/app/comum/relatorios/relatoriosdefs.h |
| 603 | 11 |  | `comum::CInformacaoEleicao::CInformacaoEleicao` | uenux2/src/app/comum/informacao/cinformacaoeleicao.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 730 | 170 |  | `comum::md::CEleicaoPE::~CEleicaoPE` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 761 | 342 |  | `comum::md::CDetalheCandidato::operator=(CDetalheCandidato&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 783 | 404 |  | `std::optional<comum::md::CDetalheConsulta>::operator=(optional&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.h (implícito) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 817 | 350 |  | `comum::md::CEleicaoPE::operator=(CEleicaoPE&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.h | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 818 | 355 |  | `std::swap<comum::md::CEleicaoPE>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 942 | 107 |  | `comum::CCalculaCV::IncluiString` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp |
| 1000 | 25 | sim | `api::CDateTime::CDateTime(std::time_t)` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1281 | 439 |  | `comum::md::CPleito::CPleito(const CPleito&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1382 | 111 | sim | `api::CDate::CDate` | uenux2/src/api/util/cdate.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1386 | 6360 |  | `std::__sort3<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 1506 | 53 |  | `std::__sort3_maybe_branchless<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 1543 | 6279 |  | `comum::CRelUtil::IncluiCabecalhoEleicoesMZS` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1919 | 897 |  | `comum::CRelUtil::GetSeparadorFase` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1942 | 444 |  | `comum::CRelUtil::DSCodigoIdentificacaoUE` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1953 | 243 |  | `std::uninitialized_copy<const comum::md::CEleicaoPE*, comum::md::CEleicaoPE*>` | libc++ (cópia de vector<CEleicaoPE>) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1954 | 19 |  | `api::CPolySingletonList::exists<comum::CCalculaCV>` | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2230 | 86 | sim | `api::CTime::CTime` | uenux2/src/api/util/ctime.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2251 | 37 |  | `comum::CGeradorBUQRCode::~CGeradorBUQRCode` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 2261 | 31 |  | `comum::CLocal::~CLocal` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2271 | 105 |  | `comum::CCandidaturas::PossuiCandidatos` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2289 | 250 | sim | `std::vector<comum::md::CCargo>::vector(const vector&)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2578 | 79 |  | `std::__partially_sorted_swap<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 2686 | 739 |  | `vota::(anonymous namespace)::FormataHorario` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (caminho inferido) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 2786 | 2458 |  | `comum::CRelUtil::GetIDCargaFormatado` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 2788 | 23 |  | `comum::CPE::Existe` | uenux2/src/app/comum/dados/cpe.cpp (caminho inferido) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2791 | 562 |  | `comum::CCabecalhoQRCodeBuilder::preBuild()::(lambda)::operator()` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp |
| 2829 | 189 |  | `std::__sort4<comum::md::CCargo*, bool(*)(const CCargo&, const CCargo&)>` | libc++ (std::sort em cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3687 | 3817 |  | `std::__sort4<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 3689 | 1616 |  | `comum::CSubstituidorTitulo::IncluiTitulos` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | src/uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp |
| 3693 | 35 |  | `std::vector<int>::assign(int*, int*)` | libc++ (corpo mesclado 6027) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3696 | 1921 |  | `comum::CGeradorBUQRCode::AptosCargo` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 3698 | 244 |  | `comum::CGeradorBU::~CGeradorBU` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 3775 | 573 |  | `comum::CConfiguracaoEleicao::GetCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3792 | 702 |  | `comum::CAppInfo::~CAppInfo` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3871 | 217 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::GetLinhasEleitoresAptos` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 4809 | 269 |  | `std::__sift_down<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 4810 | 107 |  | `std::__swap_bitmap_pos<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 4813 | 290 |  | `std::__insertion_sort_incomplete<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 4814 | 151 |  | `std::__sort5_maybe_branchless<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 4815 | 198 |  | `std::__sort4_maybe_branchless<unsigned>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 4816 | 2502 |  | `std::__introsort<unsigned*, true>` | libc++ (std::sort em cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comentário) |
| 5472 | 67 | sim | `comum::FormataDataAtual` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (caminho incerto) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5473 | 64 | sim | `comum::FormataHoraAtual` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (caminho incerto) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5576 | 12 |  | `comum::CSubstituidorTitulo::~CSubstituidorTitulo` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | src/uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp |
| 5577 | 7625 |  | `std::__insertion_sort_incomplete<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 5578 | 5092 |  | `std::__sort5<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 5582 | 96893 |  | `std::__introsort<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort em crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comentário) |
| 5583 | 168 |  | `comum::CRelUtil::IncluiAvisoModoDemonstracao` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 5595 | 1070 |  | `std::__copy_loop<comum::md::CEleicaoPE> (vector<CEleicaoPE>::assign)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5597 | 2124 |  | `comum::md::CPleito::operator=(const CPleito&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5609 | 216 |  | `comum::CGeradorRelBase<comum::CRdvVota, comum::CEleitores>::~CGeradorRelBase` | uenux2/src/app/comum/relatorios/cgeradorrelbase.h | src/uenux2/src/app/comum/relatorios/cgeradorrelbase.h |
| 5610 | 5599 |  | `std::__introsort<comum::md::CRespostaConsulta*>` | libc++ (std::sort em cgeradorbubase.h) | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h (comentário) |
| 5612 | 519 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::~CGeradorBUBase` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 5738 | 1726 |  | `comum::CRelUtil::FormataSecoesAgregadas` | uenux2/src/app/comum/relatorios/crelutil.cpp (caminho inferido) | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 5777 | 886 | sim | `std::vector<comum::md::CEleicaoPE>::push_back` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5782 | 2390 |  | `comum::CQRCodeDS::operator()` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (caminho incerto) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5784 | 197 |  | `std::vector<comum::md::CRespostaConsulta>::operator=(vector&&)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5785 | 1732 |  | `std::__insertion_sort_incomplete<comum::md::CCargo*, bool(*)(...)>` | libc++ (std::sort em cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5789 | 12897 |  | `std::__introsort<comum::md::CCargo*, bool(*)(...)>` | libc++ (std::sort em cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5813 | 121 |  | `comum::CAppInfo::CAppInfo` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5967 | 554 |  | `comum::IncluiFinal(const CCargos&, CCalculaCV&)` (função livre; nome inferido) | uenux2/src/app/comum/relatorios/ccalculacv.cpp (arquivo inferido) | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp |
| 6627 | 695 | sim | `std::make_shared<api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>>` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 11175 | 37 |  | `atexit: ~unique_ptr<comum::CSubstituidorTitulo>` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | src/uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp |
| 11176 | 82 |  | `comum::CSigVerifier::~CSigVerifier (deleting)` | uenux2/src/app/comum/relatorios/csigverifier.cpp | src/uenux2/src/app/comum/relatorios/csigverifier.cpp |
| 11178 | 79 |  | `comum::CSigVerifier::~CSigVerifier` | uenux2/src/app/comum/relatorios/csigverifier.cpp | src/uenux2/src/app/comum/relatorios/csigverifier.cpp |
| 11179 | 969 |  | `comum::CSigVerifier::Verify` | uenux2/src/app/comum/relatorios/csigverifier.cpp | src/uenux2/src/app/comum/relatorios/csigverifier.cpp |
| 11181 | 549 |  | `comum::(anonymous namespace)::TituloPartido` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 11182 | 1026 |  | `comum::CRelUtil::DSCodigoVerificador` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 11183 | 16 |  | `comum::CRelatorioTesteImpressora::GetSeparadorFase` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11184 | 85 |  | `comum::CRelatorioTesteImpressora::GetMensagemFinal2` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11186 | 85 |  | `comum::CRelatorioTesteImpressora::GetMensagemFinal1` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11187 | 61 |  | `comum::CRelatorioTesteImpressora::GetTitulo` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11188 | 3304 |  | `comum::CRelatorioTesteImpressora::GetLinhasLocal` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11189 | 55 |  | `comum::CRelatorioTesteImpressora::GetUF` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11190 | 146 |  | `comum::CRelatorioTesteImpressora::GetTipoUrna` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11191 | 40 |  | `comum::CRelatorioTesteImpressora::GetPleito` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11192 | 52 |  | `comum::CRelatorioTesteImpressora::GetNomeProcessoEleitoral` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11193 | 40 |  | `comum::CRelatorioTesteImpressora::EhModoDemonstracao` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11194 | 11 |  | `comum::CRelatorioTesteImpressora::EhEleicaoComunitaria` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11195 | 9 |  | `comum::CRelatorioTesteImpressora::GetCabecalho` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11196 | 631 |  | `comum::CRelatorioTesteImpressora::ConfiguraLabels` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11197 | 17 |  | `comum::CRelatorioTesteImpressora::GetCorrespondencia` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11198 | 69 |  | `comum::CRelatorioTesteImpressora::EstaEmHorarioVerao` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11199 | 8 |  | `comum::CRelatorioTesteImpressora::GetHorarioVerao` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11200 | 330 |  | `comum::CRelatorioTesteImpressora::GetConteudoQRCode` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11201 | 14 |  | `comum::CRelatorioTesteImpressora::EhTreinamento` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11202 | 8 |  | `comum::CRelatorioTesteImpressora::PossuiHorarioVerao` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11212 | 850 |  | `comum::CParteCandidatosConsultas::Imprime` | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp |
| 11214 | 640 |  | `comum::CParteCandidatosMajoritarios::Imprime` | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp |
| 11215 | 140 |  | `comum::CParteCandidatos::Imprime` | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp |
| 11216 | 799 |  | `comum::CRelatorioTesteImpressora::IncluiEmissao` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 11233 | 1201 |  | `comum::(anonymous namespace)::SituacaoEleitor` | uenux2/src/app/comum/relatorios/cgeradorrellistaeleitores.cpp | src/uenux2/src/app/comum/relatorios/cgeradorrellistaeleitores.cpp |
| 11243 | 13 |  | `comum::CGeradorBUQRCodeVota::~CGeradorBUQRCodeVota (deleting)` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp (arquivo da classe inferido) | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 11244 | 7 |  | `comum::CGeradorBUQRCodeVota::~CGeradorBUQRCodeVota` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp (arquivo da classe inferido) | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 11253 | 2730 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeConsulta` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11254 | 1443 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeMajoritario` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11255 | 1749 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeProporcionalPartido` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11256 | 1733 |  | `comum::CGeradorBU::ImprimePreTexto` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 11258 | 13 |  | `comum::CGeradorBU::~CGeradorBU (deleting)` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 11260 | 3040 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerProporcionalPartido` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11371 | 3396 | sim | `comum::asn::CConversorEleicaoPE::DoDesconverte` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversoreleicaope.cpp (caminho inferido) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11508 | 37 |  | `atexit: ~unique_ptr<comum::CLocal>` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11574 | 37 |  | `atexit: ~unique_ptr<comum::CAppInfo>` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11892 | 1721 |  | `vota::CMenuFiltrarCandidatosPorNumero::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (caminho inferido) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 11902 | 6548 |  | `vota::CImpressaoPU::StartState` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (caminho inferido) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 11973 | 919 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::DetalheCandidatoZE` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11974 | 1240 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::CodVerificador` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11980 | 2326 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerMajoritario` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11981 | 2762 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerProporcional` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 12588 | 57 |  | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::~CDataText (deleting)` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 12589 | 54 |  | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::~CDataText` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |

## 13. Questões em aberto

* Os nomes dos enumeradores de `ETipoCabecalho` (1 imprime *Mesa Receptora*/*Urna*; o seu único chamador é o gerador do BIM com uma
  flag negada) e o significado do valor 3, que lança uma exceção.
* Os nomes exatos dos hooks de `CImprimirExtratoCarga` (inferidos a partir de como `vota_f5591` usa cada resultado) e o arquivo de
  `FormataDataAtual/FormataHoraAtual` e de `CQRCodeDS`.
* Os três parâmetros `bool` de `CriaQRCode` (cgeradorbu.cpp:124): só a flag de QR habilitado (PU +486) e as duas
  flags de `CGeradorBUQRCodeVota` (+418 origem RED, +419 data de emissão) são visíveis; todos são constantes no VOTA.
* `CCalculaCV` é criado com a identificação montada no construtor de `CGeradorBU` ou em `CGeraBU::StartState`
  (os dois estão inlinados na 12110; a ordem do código sugere o construtor).

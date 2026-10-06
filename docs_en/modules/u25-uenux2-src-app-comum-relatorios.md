# u25 — `comum/relatorios` (printed reports: Boletim de Urna, zerésima, estado da urna, report helpers)

Unit u25 of the reverse-engineering of `vota_web_wasm.wasm` (TSE voting application VOTA, `uenux2` + `ecourna`,
Emscripten build of the public training simulator). **120 functions**, 11 of them seen executing during the recorded
votes (all 11 are helpers that belong to other files: date/time constructors, `CAppInfo::GetInst`, copies of
`CCargo`/`CEleicaoPE`, the eleição converter, the screen clock data sources). No report code proper ran: the web page
never reaches a printed report. Function indices are wasm function indices (`python3 tools/wasmmap/q.py f <n>`).

Reconstructed sources (all under `src/uenux2/src/app/comum/relatorios/`):

| file | contents |
|---|---|
| `relatoriosdefs.h` (path inferred) | `CRelatoriosError` = `CBaseError<EUeComumRelatoriosError, {9050, 9150}>`, list of all codes, `ETipoCabecalho` |
| `crelutil.h/.cpp` | `CRelUtil`: standard header, separators, carga id, data sources of the CV / urna id, demo banner, seções agregadas; `TituloPartido`; the eleições `std::sort` |
| `csubstituidortitulo.h/.cpp` | `CSubstituidorTitulo` singleton and `IncluiTitulos` (configurable titles/footers of the `-pu.dat`) |
| `ccalculacv.h/.cpp` | `CCalculaCV`, the *código verificador* chain (complete class) |
| `cdatasourcesrelatorio.h` | `CDataSourcesRelatorio<RDV, DSAptos>`: trailers, CVs and candidate lines evaluated at print time |
| `cgeradorrelbase.h` | `CGeradorRelBase<RDV, DSAptos>`: report skeleton and `GeraRelatorio(path)` |
| `cgeradorbubase.h` | `CGeradorBUBase<RDV, DSAptos>`: the per-cargo blocks of the BU |
| `cgeradorbu.h/.cpp` | `CGeradorBU`: the 13 parts of the BU, QR codes, CV publication, *histórico de cargas* |
| `cgeradorbuqrcode.h/.cpp` | `CGeradorBUQRCode` destructor and the `APTA/APTS/APTT` block (payload builder itself: `cgeradorbuqrcode.u04-fragment.cpp`) |
| `ccabecalhoqrcodebuilder.cpp` | `preBuild()` mandatory-field checks of the QR header |
| `cpartecandidatos.h/.cpp` | report parts of the zerésima's candidate list |
| `crelatoriotesteimpressora.h/.cpp` | the "ESTADO DA URNA" report hooks, `CQRCodeDS`, date/time data sources |
| `csigverifier.h/.cpp` | signature check of a report file before printing |
| `cgeradorrellistaeleitores.cpp` | `SituacaoEleitor` (flags of the voters list) |
| `../u25-foreign-fragments.cpp`, `../../vota/u25-foreign-fragments.cpp` | functions assigned to this unit that belong to other files (§10) |

Related chapters: docs/10-boletim-de-urna.md, docs/bu/codepath.md
(the encerramento run for real: `samples/bu-real/run-full/reports/*.txt` are the outputs of the code of this unit),
docs/bu/codigo-verificador.md (the CV chain re-implemented and checked),
docs/bu/qrcode.md (QR payloads), [u08](u08-uenux2-src-app-vota-eleitor.md) (`vota::CGeraBU`,
`CGeraRelatorios`, which inline most constructors of this unit), [u04](u04-uenux2-src-app-comum-dados.md)
(`CCargos`, the QR payload generator), [u23](u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md)
(the `-bu.dat` ASN.1 writer, which is *not* this unit).

## 0. Words

| term | meaning |
|---|---|
| *relatório* | printed report of the urna's thermal printer. Reports are rendered into a file first (`dinamico/trab1/<nome>.dat`), signed (`.vsu`), and the *vias* (copies) are printed from that file |
| *BU*, *boletim de urna* | the section's tally, printed at the *encerramento*. (The signed ASN.1 `-bu.dat` for the TSE is written by `CGravadorBU`, unit u23; this unit makes the **printed** BU, which becomes `-imgbu.dat`) |
| *zerésima* | report printed before voting opens, proving every candidate has zero votes |
| *via* | one printed copy (`"{}ª via"`) |
| *código verificador* (CV) | 10-digit MAC code printed under each BU block (SipHash-4-6 chain, `CCalculaCV`) |
| *carga*, *código de carga*, *histórico de cargas* | the load of election data into this urna (24-digit id printed as `123.456.789...`) and the list of loads recorded in `gap.bin` |
| *correspondência* | the pairing urna ↔ carga ↔ seção (`CDadoCorrespondencia`, `CEstadoGeral +60`); its *resumo* is printed in every header |
| *seção agregada* | another polling section merged into this urna's section |
| *apto*, *originais*, *temporários (TTE)* | eligible voters: of the section / by temporary transfer |
| *legenda* | vote for a party only (proportional cargos) |
| *consulta* | referendum question (a "cargo" whose options are *respostas*) |
| *contingência* | replacement urna; it carries both rounds' data |
| *MRJ*, *mesa receptora* | receiving table (justification urnas); the header prints *Mesa Receptora* / *Urna* |
| *fase* | `EUrnaFase` `'1'` oficial, `'2'` simulado, `'3'` treinamento |
| *PU* | `-pu.dat`, `EntidadeParametrizacaoUrna`: report titles/footers, number of copies, labels, flags |
| *SAVD* | signing / verification daemon (simulated in the web build) |

## 1. What the subsystem does

`comum/relatorios` renders the paper reports of the urna. It does not talk to the printer: every report is
built as a list of `api::IForm<api::IPaper>` forms with `api::CPaperFormBuilder`
(`AddText(text, font, alignment)` = `shared_f193`, `AddNewLine(n)` = 198, `AddData(&source, 0)` = 604,
`Build()` = `shared_f357`) and printed into `api::IPaperRelatorios`, a "paper" that on the real urna writes the
printer byte stream into a file (`trab1/bu.dat`, `ze.dat`, `buj.dat`, ...). The states of `vota` then sign the file
(SAVD), verify the signature (`CSigVerifier`) and print the *vias* from it.

Two mechanisms define the whole unit:

1. **Data sources evaluated at print time.** Many lines are `api::CDataText<std::string(*)()>` fields: a function
   pointer (a *function-table slot*, e.g. slot 1633 = `CodVerificador`) called when the form is printed. The
   report generators iterate `CCargos` (current cargo), `CPartidos` (current party) and `CCandidaturas` (current
   candidacy, positioned by `Localiza`) and print the same form again and again; each time the data source reads
   the *current* entries and the in-memory RDV (`CRdvVota`). So the printed numbers come straight from the RDV,
   there is no separate "count" step.
2. **The código verificador chain.** While the BU is generated, a `CCalculaCV` is published in
   `api::CPolySingletonList`. Each block appends the numbers it prints (`IncluiString`) *before* the form that
   contains its CV is printed; the CV data source (`CodVerificador`, `DSCodigoVerificador`,
   `TrailerProporcionalPartido`) calls `Calcula()`, which MACs the buffer and restarts the chain from the result.

## 2. Classes (RTTI) and how they relate

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

Layouts are in the headers (`// +offset`). The most important:
`CGeradorRelBase` = {vptr, header +4, trailer +12, qrcode +20, linhaVazia +28} (4 `shared_ptr<IForm>`);
`CGeradorBUBase` adds 10 parts at +36..+108 (headerProporcional, headerProporcionalPartido,
partidoApenasVotoLegenda, trailerProporcionalPartido, cargoSemCandidato, trailerCargoSemCandidato,
trailerProporcional, headerMajoritario, detalheCandidato, trailerMajoritario); `CGeradorBU` adds
`std::vector<std::string> m_historicoCargas` at +116.

## 3. Who calls this unit

| caller (other units) | uses |
|---|---|
| `vota::CGeraBU::StartState` (12110, u08) | constructs `CGeradorBU` (inlined), `GeraRelatorio(trab1/bu.dat)`, `GetSeparadorFase`, `IncluiCabecalhoEleicoesMZS`, `GetIDCargaFormatado`, `DSCodigoIdentificacaoUE`, the data sources, `exists<CCalculaCV>` |
| `vota::CGeraRelatorios::StartState` (12105, u08) | BUJ / BIM / BEHB headers (`IncluiCabecalhoEleicoesMZS`, `GetSeparadorFase`, `GetIDCargaFormatado`) |
| `vota::CGeraZeresimaBase` / `CGeraResumoZeresimaBase` (11946 / 11943, u09) | `CParteCandidatos*`, `DetalheCandidatoZE`, trailers, `TituloPartido`, `IncluiTitulos` (rodapé), `GetSeparadorFase` |
| `vota::CImprimindoBU`, `CImprimindoZeresima`, `CImprimirBJust`, `CImprimindoBim`, `CImprimindoBEHB` | `CSigVerifier(trab, "<rel>.dat", "<rel>.vsu").Verify()` before printing each via |
| `vota::CVerificaHorarioZeresima` (11869), `vota::CImpressaoEstadoUrna` (11911) | `CRelatorioTesteImpressora` + `vota_f5591` ("ESTADO DA URNA") |
| `vota::CImpressaoPU` (11902), `CImpressaoListaEleitores` (11908), `CGeradorRelVersaoPacoteDados` (11223) | header, `SituacaoEleitor` |
| screens of the voter (`CTelasVota`, operator thread 5343) | `FormataDataAtual` / `FormataHoraAtual` (slots 1103/1104) — the only report functions that run in the web build |

## 4. The standard report header: `CRelUtil::IncluiCabecalhoEleicoesMZS` (1543, crelutil.cpp:307)

Parameters: `(builder, titulo, municipio, zona, secao, nomeMunicipio, ETipoCabecalho tipo, bool usaPleitoContingencia)`.

1. Copy of `CConfiguracaoEleicao::GetPleito()`. If `usaPleitoContingencia && tipo == CONTINGENCIA`: make sure
   `CPE` exists (loads the processo eleitoral), and if it has a pleito 2 whose date is today or earlier, use it.
2. `IncluiAvisoModoDemonstracao` (5583): `"IMPRESSO EM MODO DEMONSTRAÇÃO"` + 2 blank lines in demo mode.
3. `CSubstituidorTitulo::IncluiTitulos(b, PU.cabecalho, UF)` (3689): each `TituloRelatorio{alinhamento, estilo,
   texto}` of the `-pu.dat` is translated (`CTradutorFrase::TraduzLabel`), split at `'\n'`, `"<uf>"` replaced by
   the section's UF; font 2 when *expandido*, alignment left/right/centre. The substitution singleton is created on
   the first call with the UF of that call.
4. blank line; `titulo` (centred) + blank line when `titulo` is not empty; `"Eleições Comunitárias"` if
   `CConfiguracaoEleicao +20 == 2`;
   processo eleitoral name (`+8`), pleito name, `"(DD/MM/YYYY)"`; blank line.
5. If the pleito has ≥ 2 eleições: their names sorted by `CSituacoesEleicoes::ordemImpressao` + blank line.
6. `"{:<32s} {:05}"` município (label `<MLSN>`), município name (centred), blank, `"{:<33s} {:04}"` zona (`<ZLSN>`).
7. `switch (tipo)`: `SECAO` → `"Local de Votação"` (not for comunitárias), seção (`<SLSN>`), and if there are
   agregadas `"Quantidade de seções agregadas {:04}"` + `"Seções agregadas: 0012 0034"`; `MESA_RECEPTORA` →
   `"Mesa Receptora {secao/10:03}"`, `"Urna   {secao%10}"`; `CONTINGENCIA` → `"Urna de contingência"`;
   `3` → throws 9086 `"Tipo inválido: {}"`; any value above 3 prints nothing. Then a blank line.

Labels like `<MLSN>` are translated with the parametrized labels of the PU (`labelMunicipio/Zona/Secao/Partido`:
gender, long/short, singular/plural) — `<ZLSN>` → "Zona Eleitoral", `<PLSN>` → "Partido", `<SCPB>` → plural...
Callers pass `tipo` 0 (BU, zerésima, BUJ, lista de eleitores), `2`/`0` from the urna type (PU, versões de pacotes,
with `usaPleitoContingencia = true`) and, in the BIM generator (12105), `!flag` (so 1 = *Mesa Receptora* layout).

Other helpers: `GetSeparadorFase` (1919, :554) → 38-column `"======================================"` (oficial),
`"===============SIMULADO==============="`, `"==============TREINAMENTO============="`,
`"============DEMONSTRAÇÃO=============="` in demo mode, 9087 `"Fase inválida: {}"` otherwise.
`GetIDCargaFormatado` (2786, :69): > 20 characters required (9085), groups of 3 with dots.
`FormataSecoesAgregadas` (5738), `DSCodigoIdentificacaoUE` (1942: `"Código identificação UE       {:08}"`,
`CEstadoGeral +60`), `TituloPartido` (11181, :58: `"<PLSN>: NN - SIGLA"`, 9084 if no current party).

## 5. The Boletim de Urna, step by step

This is the order in which the code of this unit runs when `vota::CGeraBU::StartState` (12110) generates the BU at
the *encerramento* (state `EAVGERARBU` = 59). Real output of this code: `samples/bu-real/run-full/reports/bu.txt`.

### 5.1 Construction of `CGeradorBU` (inlined in 12110, source cgeradorbu.cpp)

`CGeradorBU(titulo = "Boletim de Urna", municipio (eg +20), zona (+24), secao (+26), nomeMunicipio,
correspondencia (eg +60), historicoCargas (codes of every CDadoCorrespondencia of gap.bin), dtHrEmissaoBU,
separadorFase)` builds 13 forms and hands them to `CGeradorBUBase` → `CGeradorRelBase`:

| part | content |
|---|---|
| header (`CriaHeader`) | standard header (§4, `SECAO`), `"Eleitores aptos {:04}"` + originais/temporários (`CRelUtil::FormataQtdAptos` over `CEleitores::GetQtdAptos`), `"Comparecimento {:04}"` (`CRdvVota::Comparecimento`), when the urna is biometric: habilitação biométrica / biográfica / sem biometria counts, `"Eleitores faltosos"` = aptos − comparecimento, blank, `"Código identificação UE {:08}"`, if the vota state exists the opening/closing date and time (`GetDtHrInicioAquisicao/FimAquisicao`, errors 8090/8091), `"RESUMO DA CORRESPONDÊNCIA"` + code, **CV field (slot 1633)**, blank |
| trailer | separador de fase, blank, `"Código de identificação da carga"` + `GetIDCargaFormatado(codigoCarga)`, blank, `"Ver: 10.23.0.1"`, blank, `IncluiTitulos(PU.rodapeBUVOTA)` (e.g. "O conteúdo deste BU poderá ser / conferido no endereço / resultados.tse.jus.br" / ASSINATURAS / PRESIDENTE / MESÁRIOS / FISCAIS), 20 blank lines, paper cut |
| qrcode (`CriaQRCode`, :124) | demo mode: `"DEMONSTRAÇÃO PRÉ-ELEIÇÃO"`, `"NÃO HÁ QR CODE"`; else if `PU.imprimirQrCodeNoBU` (cfg +486): `"============= BU DIGITAL ============="`, for each payload of `CGeradorBUQRCodeVota(cabecalho{zona, seção, idUE, código de carga, histórico}, comparecimento, dtEmissão).GeraQRCodes(1100)`: `"-------------- 01 / 02 ---------------"` + QR bitmap; `"======== CERTIFICADO DIGITAL ========="` + certificate QRs (`QRCE/IDUE/MDUE/CERT`); `"ASSINATURA BU DIGITAL: <hex>"`; otherwise no QR form (null) |
| headerProporcional | separador de fase, blank, cargo name upper case centred in `-` (38) |
| headerProporcionalPartido | data slot 2921: `"Partido: 91 - PEsp\n"` + `"Nome do candidato       Num cand Votos"` when the party has nominal votes |
| partidoApenasVotoLegenda | `"Não há votos nominais"` |
| trailerProporcionalPartido | data slot 2922 `TrailerProporcionalPartido` (legenda, total do partido, **its own CV**) |
| cargoSemCandidato | `"Não há candidatos concorrendo"` |
| trailerCargoSemCandidato | 38 `-`, data 1632 (aptos + `"Comparecimento {:04}"`), CV (1633) |
| trailerProporcional | 38 `-`, data 1625 `TrailerProporcional`, CV (1633) |
| headerMajoritario | separador de fase, blank, cargo name, data 2923 (column header when there are nominal votes) |
| detalheCandidato | data 1636: `"  <nome 23 col.>  <número>  <votos:04>"` |
| trailerMajoritario | blank, 38 `-`, data 1626 `TrailerMajoritario`, CV (1633) |

Checks: `CGeradorRelBase` throws 9066/9067 for a null header/trailer (h:48/51), `CGeradorBUBase` 9055..9064 for a null
part (h:66..111). Then the constructor body:

1. `CCargos::GetInst().Inicio()`; if the cursor is on the last cargo, `Next()` (so the header CV gets no "final"
   strings).
2. identification = `std::format("{:05}{:04}{:04}{}{}{}{}", municipio, zona, secao,
   std::format("{:05}{:05}{}", cfg.idProcesso, pleito.id, pleito.data "YYYYMMDD"), numeroInternoUrna, pleito.id,
   faseChar 'o'/'s'/'t')`.
3. `CPolySingletonList::push(std::make_unique<CCalculaCV>(identificacao, "0000000000", DefineTipo(demo) ('A' demo /
   'F'), 16))`. The `CCalculaCV` constructor (ccalculacv.cpp:58/65/74) validates the identification
   (≥ 13 characters of `0-9sodtSODT`, 9051) and the first CV (digits, 9052), reads the 16-byte key from
   `/dsk/fi/estatico/chave/cv.ber.pri` (`EntidadeChave`, deciphered with the IKernelHSM key; 9053 if shorter) and
   starts the buffer with `tipo + identificação + "0000000000"`.

### 5.2 `GeraRelatorio("/dsk/fi/dinamico/trab1/bu.dat")` (cgeradorrelbase.h:61)

1. `IPaperRelatorios::instance().Abre(path)` (slot 5).
2. **Header** printed → its CV field calls `CodVerificador` (11974): `Calcula()` over
   `F|identificação|0000000000|"1"` → `"Código Verificador: d.ddd.ddd.ddd"`.
3. **`ImprimePreTexto`** (11256, cgeradorbu.cpp:257): `"--------"`, `"Histórico de código de carga"`, `"--------"`,
   `"1: 123.456.789.012.345.678.901.234"` for each carga, blank, CV field (slot 3048 `DSCodigoVerificador`), blank.
   Before printing it appends `código₁@código₂@...@` to the chain (an empty history would throw 9054).
4. `CCargos::OrdenaPorOrdemImpressao()`, cursor on the first cargo; for each cargo: a banner
   `"====" / <nome da eleição> / "===="` when the eleição changes (the comparison starts with the eleição of the
   *last* cargo, so a single-eleição BU has no banner), then by type:
   * **proportional** (`ImprimeProporcionalPartido`, 11255, h:187): header; if the cargo has no apt candidacy:
     chain += `{cargo:02}{total:04}`, blank line, "Não há candidatos concorrendo", trailerCargoSemCandidato. Else,
     for each party (ascending) with `RDV.Partido(cargo, p) > 0`: chain += `{cargo:02}{partido:02}`; party header;
     for each apt candidate of the party (`GetNumerosCandidatosAptos`, ascending) with votes > 0: chain +=
     `{numero:05}{votos:04}` and a candidate line; "Não há votos nominais" if none; party trailer
     (`TrailerProporcionalPartido`, h:188: `"    Votos de legenda {:04}"`, `"    Total do partido {:04}"`, chain +=
     `{legenda:04}{total:04}` [+ final strings if last cargo], CV). Then `trailerProporcional`
     (`TrailerProporcional`, h:241: chain += `{nominais}{legendas}{brancos}{nulos}{total}` (each `{:04}`),
     aptos lines, "Total de votos Nominais", "Total de votos de Legenda", "Brancos", "Nulos", "Total Apurado") + CV.
   * **majoritarian** (`ImprimeMajoritario`, 11254, h:246): header; chain += `{cargo:02}`; no candidates →
     `{cargo:02}{total:04}` + "Não há candidatos concorrendo" + trailer; else every candidacy of the cargo with
     votes (CCandidaturas order) → chain += `{numero:05}{votos:04}` + candidate line; `trailerMajoritario`
     (`TrailerMajoritario`, h:305: chain += `{nominais}{brancos}{nulos}{total}`, aptos, totals) + CV.
   * **consulta** (`ImprimeConsulta`, 11253, h:286): majoritarian header; chain += `{cargo:02}`; the answers of
     `CDetalheConsulta` sorted by number; each with votes → chain += `{numero:05}{votos:04}` and an ad-hoc line
     `"  " + resposta (26 col.) + número (4 col.) + "  " + votos:04`; `trailerMajoritario` + CV.
   "Final strings" (`IncluiFinal(cargos, calculadora)`, 5967) are appended to every CV computed while the
   `CCargos` cursor is on the **last** cargo (each party trailer of a proportional last cargo, and the cargo's
   own trailer CV): the previous 64-bit value, the pleito date `YYYYMMDD`, and `to_string(code)` of every
   cargo in print order. 5967 takes `(const CCargos&, CCalculaCV&)` in that order, so it is not a
   `CCalculaCV` member (a member would receive the calculator first).
5. `m_qrcode->Imprime()` if present; trailer; `Fecha()` (slot 6); `CSynchronizer::CreateInst()->Sincroniza()`.

`~CGeradorBU` (3698) erases the calculator from `CPolySingletonList`. `CGeraBU` then saves the state and
`CSincronizaVota::SincronizaRelatorios("bu.dat")` copies/signs it (`bu.vsu`). Each via is printed later by
`CImprimindoBU::ImprimeBU` after `CSigVerifier(trab, "bu.dat", "bu.vsu").Verify()` (11179), which inlines
`comum::ValidarUE(savd, aplicação, pacote, arquivo)` (iinterfacesavd.cpp:898, the 4-argument overload): the SAVD
must confirm `trab/bu.vsu` covers `bu.dat` (key 53, request built by func 5892) or error 7324 "Falha ao validar
assinatura UE ..." is thrown.

What the CV covers and does not cover is analysed in codigo-verificador.md §3.4
(aptos, comparecimento, times, names and QR are **not** in the chain).

## 6. Zerésima and summary parts

`CParteCandidatos::Imprime` (11215) chooses, for the current cargo: *sem candidatos* (the cargo has candidate
details and no apt candidacy), *majoritários*, *proporcionais* (u04: 11213) or *consultas* (`CCargo +136`).
`CParteCandidatosMajoritarios::Imprime` (11214, :75) prints one detail line per apt candidate after positioning
the candidacy (9081 if `Localiza` fails); `CParteCandidatosConsultas::Imprime` (11212, :174) collects the answer
numbers of `CRespostas` for the cargo (key `cargo*1000000+n`), sorts them and prints them (9083).
`DetalheCandidatoZE` (11973, h:152) is the line `"  Golfe                          91001"`: name cut to 36,
padded to 31, names of 32–36 characters continued on a second line (`------\n  ------------------------------>`),
number right-aligned; it asserts that the RDV has **0** votes for the candidate (`Assert (qtd == 0u)`, 3410).

## 7. "ESTADO DA URNA" (`CRelatorioTesteImpressora`)

A template method (`vota_f5591`) prints the report through 24 virtual hooks; `CRelatorioTesteImpressora`
implements them for VOTA: title `ESTADO DA URNA`, `UE DE VOTAÇÃO|CONTINGÊNCIA` (unsigned test on the tipo de urna
of the turno: `'1'`, `'3'`, `'4'` → VOTAÇÃO; `'2'` and any other value, `'0'` included, → CONTINGÊNCIA), UF,
município/zona, ids, `GetLinhasLocal` (11188; for the VOTAÇÃO types only: local (oficial only), seção, the two
*agregadas* lines **only when the seção has aggregated seções**, tipo de local *Normal/Voto em Trânsito/Preso
provisório/Temporário/Inválido*, biometria *Sim/Não*; always: *Tipo de alimentação* from `api::IPower`: *R. Elétrica /
B. Interna / B. Externa / Não identificada*), emission date/time, resumo da correspondência, daylight-saving period (skipped in
treinamento), a QR code (`GetConteudoQRCode`: carga fields + `ORIG` + `VERS/DTAG/HRAG/DTPL/HRVR` from
`CQRCodeDS`), version, `"URNA OPERANDO EM PERFEITAS" / "CONDIÇÕES DE FUNCIONAMENTO"`, signatures.

## 8. Web-build specifics

* The printer and paper are `simulador::CWasmNullPrinter` / `CWasmNullPaper` (discard everything), and the page
  never reaches the encerramento, the zerésima or the "Mais informações" reports: nothing of §4–§7 ran in the
  recorded sessions. The patched harness of codepath.md runs them for real.
* `CSigVerifier::Verify` talks to `(anonymous)::CWasmSavd`, which answers "OK" to every request: the report
  signature check is simulated (it cannot fail).
* The CV key file `/dsk/fi/estatico/chave/cv.ber.pri` does not exist in the public scenarios (the harness seeds a
  fake one); `CCalculaCV`'s constructor would throw when the BU is generated without it.
* `FormataDataAtual` / `FormataHoraAtual` (5472/5473) are the only functions of the report files that run: they
  are the data sources of the date/time of the screens' status header (through `CDataTextFmt`, slots 1103/1104).

## 9. WebAssembly / Emscripten observations

* **LTO inlining hides the constructors.** `CGeradorBU`, `CGeradorBUBase`, `CGeradorRelBase`, `CCalculaCV`,
  `CSubstituidorTitulo::GetInst/CreateInst` and `GeraRelatorio` exist only inside their callers (12110, 3689). Their
  `std::source_location` records (`cgeradorbubase.h:66..111`, `cgeradorrelbase.h:48/51/61`, `ccalculacv.cpp:58..74`,
  `csubstituidortitulo.cpp:21/26`) are the only trace, which is why the tools named func 3689
  "CSubstituidorTitulo::GetInst" and func 3696 "CGeradorBUQRCode::GetQtdAptos".
* **A 97 KB sort.** The header sorts the eleições with a lambda that takes `md::CEleicaoPE` **by value**, so every
  comparison deep-copies two eleições (with all their 140-byte cargos). The resulting `std::__introsort`
  instantiation (5582) is 96,893 bytes — the largest function of the unit — plus 5577/5578/3687/1386.
* **Two sort flavours.** `std::sort` of plain integers (11212) instantiates libc++'s branch-free sorting networks and
  bitset partition (4816 + 4809/4810/4813/4814/4815/1506/2578); sorts with a comparator (eleições, cargos by
  function pointer slot 2363, answers) use the classic ones.
* **Merged bodies.** `CParteCandidatosMajoritarios` and `CParteCandidatosConsultas` share their destructors (5589 /
  5588), the relatorios error constructor is a 3-instruction thunk (580) over the shared `CBaseError` body
  (ecourna_f710), `exists<CCalculaCV>` (1954) is a thunk of the 20-character-name body 6057.
* **Format strings.** Most text is produced by `std::format` with compile-time strings (e.g. `"{:<33s} {:04}"`);
  translated labels are turned into runtime format strings and printed with `std::vformat` (the loop scanning for
  `'{'`/`'}'` is `__try_constant_folding`). Enum arguments (`EUrnaFase`, `ETipoCabecalho`) are formatted through a
  `std::formatter` handle (slots 2285/3047 → body 536).
* Data-segment names in the pseudo-code: `d_operator0033s040504040x0505T[N]` is at absolute address `N + 1024`.

## 10. Functions of the unit that belong to other files

The unit builder put 41 helpers here (plus the `CEleicaoPE` move/swap 817/818 used by the eleições sort) because their callers are report code. They are reconstructed or described in
`src/uenux2/src/app/comum/u25-foreign-fragments.cpp` and `src/uenux2/src/app/vota/u25-foreign-fragments.cpp`:
`CAppInfo` (185 GetInst, 5813, 3792, 11574), `CLocal` (2261, 11508), `CInformacaoEleicao` ctor (603), `CPE::Existe`
(2788), `CCandidaturas::PossuiCandidatos` (2271), `CConfiguracaoEleicao::GetSituacoesEleicoes` (271) and
`GetCargos(eleição, ordenados)` (3775) with its sort (5789, 5785, 2829, 320, 761, 783, 5784), implicit special
members of `CDetalheCandidato` (365, 242), `CDetalheConsulta` (374, 267), `CEleicaoPE` (730, 817, 818, 1953, 5595,
5777), `CPleito` (1281, 5597), `vector<CCargo>` (2289), `vector<int>::assign` (3693), the eleição converter
`CConversorEleicaoPE::DoDesconverte` (11371, observed executing), `exists<CCalculaCV>` (1954), `api::CDate()`
(1382), `CTime()` (2230), `CDateTime(time_t)` (1000); and in vota: `make_shared<CDataText<DS_NomeCargoNeutroComEscolha>>`
(6627, observed) and its destructors (12588/12589), `CMenuFiltrarCandidatosPorNumero::StartState` (11892),
`CImpressaoPU::StartState` (11902, "Parâmetros de urna" report) and its time formatter (2686).

### Notes for other units (found while reading these functions)

* `CEleicaoPE +40` is filled by `CConversorEleicaoPE` (11371) from `EntidadeEleicao.municipios`, not from a list of
  turnos (u02's `EleicoesDoTurno`/`CriaPleito` comments assume turnos).
* `comum_f3871` is `CDataSourcesRelatorio::GetLinhasEleitoresAptos` (it stores the `GetLinhasEleitoresAptos()::lambda`
  `__func` vtable @1543880), not a "separator" (u04 fragment).
* `CGeradorBUQRCode::AptosCargo` (3696) always writes `APTA`, `APTS` **and** `APTT` (u04's comment shows the last two
  as optional).
* `GetSeparadorFase`: `'1'` → plain line (oficial), `'2'` → SIMULADO, `'3'` → TREINAMENTO (the comment in u08's
  `cgerabu.cpp` swaps `'1'` and `'2'`).
* `vota_f5591` (tools file `cverificahorariozeresima.cpp`) is `CImprimirExtratoCarga::Imprime`; `comum_f5586` is
  `CRelatorioTesteImpressora::CRelatorioTesteImpressora`.
* `docs/bu/codigo-verificador.md` §2 writes the "final strings" function (5967) as a `CCalculaCV` member
  `«IncluiFinal»(const CCargos&)`. Its wasm parameters are `(CCargos, CCalculaCV)` in that order (callers 11260 and
  11974), so it is a free function (or a static of another class) taking the cargos first. The algorithm there is
  unaffected.
* `CInformacaoEleicao`'s constructor (603) stores its argument as is; the `+88` (the `CParametrosUrna` inside
  `CConfiguracaoEleicao`) is added by every caller. u23's comment "`m_parametros = &cfg +88`" inside 603 is
  inaccurate: the parameter is the parameters sub-object.
* `comum_f5584` (= `IncluiTitulos(b, lista, CLocal::GetInst().GetUF())`) serves both the BU footer (12110) and the
  ESTADO DA URNA header (`vota_f5591`, hook [11]).

## 11. Suspicious / noteworthy code

| wasm | finding | impact |
|---|---|---|
| 11179 | Report signature check is simulated in the web build (SAVD mock always OK) | simulator only; the real urna uses its SAVD |
| 11256 | `IncluiString("")` throws 9054 when the *histórico de cargas* is empty, aborting BU generation | real urna only if gap.bin had no correspondência (not expected) |
| 11974 | `CodVerificador` does not test `exists<CCalculaCV>()` (the trailers do); printing a form holding slot 1633 without a published calculator throws a PolySingleton error | only if a BU form were reused outside `CGeraBU` |
| 1543 | `ETipoCabecalho` 3 throws "Tipo inválido", values > 3 silently print no seção block | latent; no caller passes > 2 |
| 3689 | `CSubstituidorTitulo` is created once, with the UF of the first call; later calls with another UF keep the first | latent (one UF per process) |
| 5582 | by-value comparator: O(n log n) deep copies of eleições, 97 KB of code | performance / code size only |
| 5782 | `std::regex` compiled on every call; `'.'` unescaped in `"[0-9]+.[0-9]+.[0-9]+.[0-9]+"`; `VERS` falls back to `0.0.0.0` | cosmetic |
| 11233 | `38 - (id + flags)` underflows to a huge size → `std::length_error` if the identity plus flags exceed 38 characters | unreachable with real data (≤ 27) |
| 2786 | the carga id is only checked for length > 20 (not 24 digits) | cosmetic |
| 11255 | `Localiza` result is not null-checked before reading the candidate number | unreachable (numbers come from the same map) |
| 11190 / 11188 | the tipo de urna test is unsigned: `'0'` (*sem tipo*) or any unexpected value is reported as `UE DE CONTINGÊNCIA` and the local block is omitted | cosmetic; a loaded urna always has a type |

## 12. Complete mapping table (120 functions)

"ran" = seen executing in the recorded votes. "reconstruction" = file of this repository where the function is
written or explained.

| wasm | bytes | ran | reconstructed symbol | original file | reconstruction |
|---:|---:|:-:|---|---|---|
| 185 | 85 | yes | `comum::CAppInfo::GetInst` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/appinfo/cappinfo.h (u20) |
| 242 | 138 |  | `comum::md::CDetalheCandidato::~CDetalheCandidato` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 267 | 214 |  | `comum::md::CDetalheConsulta::~CDetalheConsulta` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 271 | 12 |  | `comum::CConfiguracaoEleicao::GetSituacoesEleicoes` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 320 | 1110 |  | `std::swap<comum::md::CCargo>` | libc++ (std::sort in cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 365 | 369 | yes | `comum::md::CDetalheCandidato::CDetalheCandidato(const CDetalheCandidato&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 374 | 211 |  | `comum::md::CDetalheConsulta::CDetalheConsulta(const CDetalheConsulta&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 580 | 18 |  | `comum::CRelatoriosError::CRelatoriosError` | uenux2/src/app/comum/relatorios/relatoriosdefs.h (path inferred) | src/uenux2/src/app/comum/relatorios/relatoriosdefs.h |
| 603 | 11 |  | `comum::CInformacaoEleicao::CInformacaoEleicao` | uenux2/src/app/comum/informacao/cinformacaoeleicao.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 730 | 170 |  | `comum::md::CEleicaoPE::~CEleicaoPE` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 761 | 342 |  | `comum::md::CDetalheCandidato::operator=(CDetalheCandidato&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 783 | 404 |  | `std::optional<comum::md::CDetalheConsulta>::operator=(optional&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.h (implicit) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 817 | 350 |  | `comum::md::CEleicaoPE::operator=(CEleicaoPE&&)` | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.h | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 818 | 355 |  | `std::swap<comum::md::CEleicaoPE>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 942 | 107 |  | `comum::CCalculaCV::IncluiString` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp |
| 1000 | 25 | yes | `api::CDateTime::CDateTime(std::time_t)` | uenux2/src/api/util/cdatetime.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1281 | 439 |  | `comum::md::CPleito::CPleito(const CPleito&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1382 | 111 | yes | `api::CDate::CDate` | uenux2/src/api/util/cdate.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1386 | 6360 |  | `std::__sort3<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 1506 | 53 |  | `std::__sort3_maybe_branchless<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 1543 | 6279 |  | `comum::CRelUtil::IncluiCabecalhoEleicoesMZS` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1919 | 897 |  | `comum::CRelUtil::GetSeparadorFase` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1942 | 444 |  | `comum::CRelUtil::DSCodigoIdentificacaoUE` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 1953 | 243 |  | `std::uninitialized_copy<const comum::md::CEleicaoPE*, comum::md::CEleicaoPE*>` | libc++ (vector<CEleicaoPE> copy) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 1954 | 19 |  | `api::CPolySingletonList::exists<comum::CCalculaCV>` | uenux2/src/api/pattern/cpolysingletonlist.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2230 | 86 | yes | `api::CTime::CTime` | uenux2/src/api/util/ctime.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2251 | 37 |  | `comum::CGeradorBUQRCode::~CGeradorBUQRCode` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 2261 | 31 |  | `comum::CLocal::~CLocal` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2271 | 105 |  | `comum::CCandidaturas::PossuiCandidatos` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2289 | 250 | yes | `std::vector<comum::md::CCargo>::vector(const vector&)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2578 | 79 |  | `std::__partially_sorted_swap<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 2686 | 739 |  | `vota::(anonymous namespace)::FormataHorario` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (path inferred) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 2786 | 2458 |  | `comum::CRelUtil::GetIDCargaFormatado` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 2788 | 23 |  | `comum::CPE::Existe` | uenux2/src/app/comum/dados/cpe.cpp (path inferred) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 2791 | 562 |  | `comum::CCabecalhoQRCodeBuilder::preBuild()::(lambda)::operator()` | uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp | src/uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp |
| 2829 | 189 |  | `std::__sort4<comum::md::CCargo*, bool(*)(const CCargo&, const CCargo&)>` | libc++ (std::sort in cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3687 | 3817 |  | `std::__sort4<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 3689 | 1616 |  | `comum::CSubstituidorTitulo::IncluiTitulos` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | src/uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp |
| 3693 | 35 |  | `std::vector<int>::assign(int*, int*)` | libc++ (merged body 6027) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3696 | 1921 |  | `comum::CGeradorBUQRCode::AptosCargo` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 3698 | 244 |  | `comum::CGeradorBU::~CGeradorBU` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 3775 | 573 |  | `comum::CConfiguracaoEleicao::GetCargos` | uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3792 | 702 |  | `comum::CAppInfo::~CAppInfo` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 3871 | 217 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::GetLinhasEleitoresAptos` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 4809 | 269 |  | `std::__sift_down<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 4810 | 107 |  | `std::__swap_bitmap_pos<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 4813 | 290 |  | `std::__insertion_sort_incomplete<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 4814 | 151 |  | `std::__sort5_maybe_branchless<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 4815 | 198 |  | `std::__sort4_maybe_branchless<unsigned>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 4816 | 2502 |  | `std::__introsort<unsigned*, true>` | libc++ (std::sort in cpartecandidatos.cpp) | src/uenux2/src/app/comum/relatorios/cpartecandidatos.cpp (comment) |
| 5472 | 67 | yes | `comum::FormataDataAtual` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (path uncertain) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5473 | 64 | yes | `comum::FormataHoraAtual` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (path uncertain) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5576 | 12 |  | `comum::CSubstituidorTitulo::~CSubstituidorTitulo` | uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp | src/uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp |
| 5577 | 7625 |  | `std::__insertion_sort_incomplete<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 5578 | 5092 |  | `std::__sort5<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 5582 | 96893 |  | `std::__introsort<comum::md::CEleicaoPE*, PorOrdemImpressao>` | libc++ (std::sort in crelutil.cpp) | src/uenux2/src/app/comum/relatorios/crelutil.cpp (comment) |
| 5583 | 168 |  | `comum::CRelUtil::IncluiAvisoModoDemonstracao` | uenux2/src/app/comum/relatorios/crelutil.cpp | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 5595 | 1070 |  | `std::__copy_loop<comum::md::CEleicaoPE> (vector<CEleicaoPE>::assign)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5597 | 2124 |  | `comum::md::CPleito::operator=(const CPleito&)` | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.h | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5609 | 216 |  | `comum::CGeradorRelBase<comum::CRdvVota, comum::CEleitores>::~CGeradorRelBase` | uenux2/src/app/comum/relatorios/cgeradorrelbase.h | src/uenux2/src/app/comum/relatorios/cgeradorrelbase.h |
| 5610 | 5599 |  | `std::__introsort<comum::md::CRespostaConsulta*>` | libc++ (std::sort in cgeradorbubase.h) | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h (comment) |
| 5612 | 519 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::~CGeradorBUBase` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 5738 | 1726 |  | `comum::CRelUtil::FormataSecoesAgregadas` | uenux2/src/app/comum/relatorios/crelutil.cpp (path inferred) | src/uenux2/src/app/comum/relatorios/crelutil.cpp |
| 5777 | 886 | yes | `std::vector<comum::md::CEleicaoPE>::push_back` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5782 | 2390 |  | `comum::CQRCodeDS::operator()` | uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (path uncertain) | src/uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp |
| 5784 | 197 |  | `std::vector<comum::md::CRespostaConsulta>::operator=(vector&&)` | libc++ | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5785 | 1732 |  | `std::__insertion_sort_incomplete<comum::md::CCargo*, bool(*)(...)>` | libc++ (std::sort in cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5789 | 12897 |  | `std::__introsort<comum::md::CCargo*, bool(*)(...)>` | libc++ (std::sort in cconfiguracaoeleicao.cpp) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5813 | 121 |  | `comum::CAppInfo::CAppInfo` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 5967 | 554 |  | `comum::IncluiFinal(const CCargos&, CCalculaCV&)` (free function; name inferred) | uenux2/src/app/comum/relatorios/ccalculacv.cpp (file inferred) | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp |
| 6627 | 695 | yes | `std::make_shared<api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>>` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
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
| 11243 | 13 |  | `comum::CGeradorBUQRCodeVota::~CGeradorBUQRCodeVota (deleting)` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp (class file inferred) | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 11244 | 7 |  | `comum::CGeradorBUQRCodeVota::~CGeradorBUQRCodeVota` | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp (class file inferred) | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp |
| 11253 | 2730 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeConsulta` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11254 | 1443 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeMajoritario` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11255 | 1749 |  | `comum::CGeradorBUBase<comum::CRdvVota, comum::CEleitores>::ImprimeProporcionalPartido` | uenux2/src/app/comum/relatorios/cgeradorbubase.h | src/uenux2/src/app/comum/relatorios/cgeradorbubase.h |
| 11256 | 1733 |  | `comum::CGeradorBU::ImprimePreTexto` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 11258 | 13 |  | `comum::CGeradorBU::~CGeradorBU (deleting)` | uenux2/src/app/comum/relatorios/cgeradorbu.cpp | src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp |
| 11260 | 3040 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerProporcionalPartido` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11371 | 3396 | yes | `comum::asn::CConversorEleicaoPE::DoDesconverte` | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversoreleicaope.cpp (path inferred) | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11508 | 37 |  | `atexit: ~unique_ptr<comum::CLocal>` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11574 | 37 |  | `atexit: ~unique_ptr<comum::CAppInfo>` | uenux2/src/app/comum/appinfo/cappinfo.cpp | src/uenux2/src/app/comum/u25-foreign-fragments.cpp |
| 11892 | 1721 |  | `vota::CMenuFiltrarCandidatosPorNumero::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp (path inferred) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 11902 | 6548 |  | `vota::CImpressaoPU::StartState` | uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp (path inferred) | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 11973 | 919 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::DetalheCandidatoZE` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11974 | 1240 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::CodVerificador` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11980 | 2326 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerMajoritario` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 11981 | 2762 |  | `comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>::TrailerProporcional` | uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h | src/uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h |
| 12588 | 57 |  | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::~CDataText (deleting)` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |
| 12589 | 54 |  | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::~CDataText` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | src/uenux2/src/app/vota/u25-foreign-fragments.cpp |

## 13. Open questions

* Enumerator names of `ETipoCabecalho` (1 prints *Mesa Receptora*/*Urna*; its only caller is the BIM generator with a
  negated flag) and the meaning of the value 3 that throws.
* Exact names of the `CImprimirExtratoCarga` hooks (inferred from how `vota_f5591` uses each result) and the file of
  `FormataDataAtual/FormataHoraAtual` and `CQRCodeDS`.
* The three `bool` parameters of `CriaQRCode` (cgeradorbu.cpp:124): only the QR-enabled flag (PU +486) and the two
  `CGeradorBUQRCodeVota` flags (+418 RED origin, +419 emission date) are visible; all are constant in VOTA.
* `CCalculaCV` is created with the identification built in the `CGeradorBU` constructor or in `CGeraBU::StartState`
  (both are inlined into 12110; the order of the code suggests the constructor).

# u09 — `uenux2/src/app/vota/eleitor`: result files, BU printing and QR codes, zerésima, candidate viewer

Unit u09 covers **80 wasm functions** of the VOTA application (the urna's election-day program) from
the two sub-directories of the voter-terminal state machine that frame the vote:

| directory | what it does |
|---|---|
| `eleitor/iniciovotacao/` (*início da votação*, start of the day) | builds the voters' dynamic database, checks the election date, generates and prints the **zerésima** (the report that proves the urna holds zero votes before the vote opens) and its summary, reprints it after a restart, shows the battery power-off warning, and offers a candidate viewer (`auxiliares/`) |
| `eleitor/fimvotacao/` (*fim da votação*, end of the day) | prints the **Boletim de Urna (BU)** copies, **writes and signs the official result files** (`CGravaResultado`), prints the other reports (BUJ, BIM, BEHB), asks for the result stick to be removed and shows the BU as QR codes on the screen |

Original files (all attested by `std::source_location` records, except where marked):

```
fimvotacao/  cgravaresultado.cpp  cimprimindobu.cpp  cquerimprimirbu.cpp  cimprimirbuoutrasobrigatorias.cpp
             cimprimirbjust.cpp   cimprimindobim.cpp cimprimindobehb.cpp  cretirarmr.cpp
             cverificaqtdbusadicionais.cpp  climitecopiasbuatingido.cpp
             cmostraqrcodebu.cpp  cmostraqrcodecertificado.cpp
iniciovotacao/ cgeradadosdinamicos.cpp  cverificaeleicaopassou.cpp  cexibealertadesligamento.cpp
             cgeradorresumozeresima.cpp  cimprimindozeresima.cpp  creimprimindozeresima.cpp
             creimprimindoresumozeresima.cpp  cquerreimprimirzeresima.cpp  cinformacaozeresimatardia.cpp
iniciovotacao/auxiliares/ cvisualizarcandidatos.cpp  cmenuvisualizarcandidatos.cpp
             cmenufiltrarcandidatosporcargo.cpp  cmenufiltrarcandidatosporpartido.cpp
```

The unit also received functions of other files (their callers are here): `CTelasVota::CriaTelaVisualizacaoCandidato`
(ctelasvota.cpp), three `CLogVota` messages (clogvota.cpp), `CRelVotaUtil::CriaCabecalho`, the singletons of
`CAplicacaoEncerrada` and `CDefineRotaPosReinicio`, `api::CWait::Aguarda`, report-builder helpers of `api`/`comum`,
constructors of `comum/gravadores` classes and a few libc++ `__tree` instances. §12 maps all 80.

**Reconstructed sources** (`// wasm func N` on every function):

* `src/uenux2/src/app/vota/eleitor/fimvotacao/`: `cgravaresultado.{h,cpp}`, `cimprimindobu.{h,cpp}`,
  `cquerimprimirbu.{h,cpp}`, `cimprimirbuoutrasobrigatorias.{h,cpp}`, `cimprimirbjust.{h,cpp}`, `cimprimindobim.{h,cpp}`,
  `cimprimindobehb.{h,cpp}`, `cretirarmr.{h,cpp}`, `cverificaqtdbusadicionais.{h,cpp}`, `climitecopiasbuatingido.{h,cpp}`,
  `cmostraqrcodebu.{h,cpp}`, `cmostraqrcodecertificado.{h,cpp}`, `caplicacaoencerrada.u09.cpp` (fragment)
* `src/uenux2/src/app/vota/eleitor/iniciovotacao/`: `cgeradadosdinamicos.cpp`, `cverificaeleicaopassou.cpp`,
  `cexibealertadesligamento.{h,cpp}`, `cgeradorresumozeresima.cpp`, `cimprimindozeresima.cpp`, `creimprimindozeresima.cpp`,
  `creimprimindoresumozeresima.{h,cpp}`, `cquerreimprimirzeresima.{h,cpp}`, `cinformacaozeresimatardia.cpp`,
  `auxiliares/{cvisualizarcandidatos,cmenuvisualizarcandidatos,cmenufiltrarcandidatosporcargo,cmenufiltrarcandidatosporpartido}.{h,cpp}`
* fragments of files owned by other units: `vota/eleitor/comum/ctelasvota.u09.cpp`, `vota/log/clogvota.u09.cpp`,
  `vota/comum/crelvotautil.u09.cpp`, `vota/eleitor/cdefinerotaposreinicio.u09.cpp`, `src/uenux2/src/api/util/cwait.u09.cpp`

**Runtime.** Only func 1249 (the copy constructor of `CDadoCorrespondencia`) ran in the recorded votes, from
`votaInit`. Everything else is dead code in the simulator: the web page starts the voter thread in
`CAguardaMensagem` and only drives voter sessions (unit u06 §10), so neither the start-of-day nor the
end-of-day chains are reached.

---

## 1. Glossary

| term | meaning |
|---|---|
| BU, *Boletim de Urna* | the per-section result: a printed report (several copies, *vias*) and an ASN.1 file `…-bu.dat` |
| *via*, *vias obrigatórias / adicionais* | printed copy; the configuration fixes how many are mandatory and how many extra may be requested |
| zerésima | report printed before the vote opens: every candidate with zero votes plus an "extrato do RDV" |
| RDV, *Registro Digital do Voto* | the file with the (shuffled) record of every vote |
| BUJ / BIM / BEHB | Boletim de Justificativa, Boletim de Identificação de Mesários, Boletim de Eleitores Habilitados Biograficamente |
| MI / MV / MR | *memória interna* (internal flash, `/dsk/fi`), *memória de votação* (memory card, `/dsk/fe`), *mídia de resultado* (USB result stick, `/dsk/mr`) |
| SAVD | the urna's signing/validation service (`comum::IInterfaceSavd`); HSM / MSE / MSD = the security module (MSE on urnas ≥ UE2020, MSD before, per the log texts) |
| *encerramento* | closing of the vote; `EstadoGeralVota.estadoEncerramento` |
| *treinamento do eleitor* | voter-training urna (fase `'3'` + flag `EstadoGeralVota.treinamentoEleitor`): no official results are written |
| *modo demonstração* | demonstration mode (`IInterfaceInit::GetDemoMode()`): one copy of everything, no MR |
| *contingência* | spare urna not bound to a section (seção 0): identified by município/zona only |

`EEstadoVota` = ASN.1 `EstadoGeralVota.estadoVota` + `'1'` (49); `EEstadoEncerramento` likewise
(`src/asn1/ModuloEstadoGeralVota.asn`). The C++ enumerator names come from the assert texts
(`EAVINICIAL`, `EAVGERADADOSDINAMICOS`, `EAVIMPRIMIRBU`, `EAVGRAVARRESULTADOS`, `EAVENCERRADA`,
`EAEIMPRIMIROBRIGATORIABU`, `EAERETIRAMR`, `EAEFIMDOSTRABALHOS`, …).

## 2. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState                       (+4 m_proximoEstado, +8..+10 message/key/tick flags; slots
   │                                        [2] StartState [3] NeedChangeState [4] GetNextState
   │                                        [5] FinishState [6] ProcessMessage [7] ProcessInput [8] ProcessTick)
   ├─ fimvotacao: CGravaResultado (12 B), CImprimindoBU (20), CQuerImprimirBU (20),
   │              CImprimirBUOutrasObrigatorias (12), CImprimirBJust (12), CImprimindoBim (12),
   │              CImprimindoBEHB (12), CRetirarMR (36), CVerificaQtdBUsAdicionais (12),
   │              CLimiteCopiasBUAtingido (20)
   ├─ vota::CEstadoComDesligamentoAutomatico (u06)
   │    ├─ CMostraQRCodeBU (36), CMostraQRCodeCertificado (36), CAplicacaoEncerrada (36)
   ├─ iniciovotacao: CGeraDadosDinamicos, CVerificaEleicaoPassou, CExibeAlertaDesligamento (40),
   │              CImprimindoZeresima, CReimprimindoZeresima, CReimprimindoResumoZeresima,
   │              CQuerReimprimirZeresima (20), CInformacaoZeresimaTardia (48)
   │              CGeraZeresimaBase (20) ─ CGeraZeresima, CRegerarZeresima          [9] hook [10] GetEstadoResumo
   │              CGeraResumoZeresimaBase ─ CGeraResumoZeresima, CRegeraResumoZeresima [10] GetProximoEstado
   └─ auxiliares: CVisualizarCandidatos (40), CMenuVisualizarCandidatos, CMenuFiltrarCandidatosPorCargo,
                  CMenuFiltrarCandidatosPorPartido (all 12 B except the viewer)
vota::IQRCodeBUDS            poly-singleton holding the BU QR payloads + current part (cmostraqrcodebu.cpp:38)
vota::CAssinadorVota : comum::CAssinador      signer of the result files (only a virtual dtor)
vota::CProgressoEncerramento : comum::CAbstractTelaProgresso   progress screen of CGravaResultado
comum::IResultado ─ comum::IGravador ─ CGravadorBU, CGravadorRDV, CGravadorRCSecao, CGravadorHashes,
                                       CGravadorWSQ, CGravadorVersoesArquivos, CGravadorLog,
                                       IGravadorEnvelope ─ CGravadorEnvelopeArquivo        (unit u23)
comum::CGravacaoResultados   (non-polymorphic, name inferred: ctor func 5824, Executa() inlined)
```

Every state is a lazily created singleton; several accessors are inlined in the caller, others go
through the merged body `vota_f764(mutex, &instance, vtable, flags)`. The destructor slots 0/1 of the
states that own a screen are ICF bodies shared by every class with the same layout (244/387, 785/1560,
1284/2884); the owners of dedicated bodies are 2871/11877, 2879/11984, 2883/12019.

## 3. Start of the day (`iniciovotacao`)

### 3.1 Flow

```
CGeraDadosDinamicos (11865)
   GeraDadosDinamicos (6737, u04/u05: uenux.db voter table unless voter training, empty rdv.dat)
   CEleitores::CompleteLoad (6734)
   then (only after the generation) assert estadoVota ∈ {inicial, gerabasedinamica} (:47)
   estadoVota = aguardahorazeresima (51); SalvaEstado
   -> CVerificaHorarioZeresima (u26)   waits for the configured zerésima time
CVerificaEleicaoPassou (11914)   (entered after a restart for estadoVota 49..51)
   fase != treinamento and now >= CConfiguracaoEleicao +580 (election date/limit):
       log(2) "Data da eleição inválida", show CTelasVota +76, post message 0 (prio 100) to the
       OPERATOR thread's queue (func 270 +36; u27: its Run loop shows "URNA ELETRÔNICA INOPERANTE" /
       "Siga as instruções na tela do eleitor" and stops that thread), IBeep slot 3,
       STAY here forever (no input/tick/message handler: slots 5..8 are the CAppState no-ops)
   estadoVota ∉ 49..51 -> LogaErro "Erro estado do aplicativo não esperado", throw 9376 "Estado nao esperado" (:70)
   -> testeteclado::CPreZeresima (keypad test before the zerésima, u26)
CGeraZeresima / CRegerarZeresima : CGeraZeresimaBase::StartState (11946)
   builds the report (§6) and writes trab/ze.dat, logs "Gerando relatório [ZERESIMA] [INÍCIO]/[TÉRMINO]",
   CSincronizaVota::SincronizaRelatorios("ze.dat"); slot 9 hook (CRegerarZeresima: cut paper);
   -> slot 10: CGeraResumoZeresima (or CRegeraResumoZeresima)  -> trab/rze.dat (func 11943, u20)
   (CGeraResumoZeresima's slot 9 hook, func 11940, sets estadoVota 53 and saves; CRegeraResumoZeresima's is a no-op)
CGeraResumoZeresima::GetProximoEstado (11939) -> CImprimindoZeresima (11991)
   "Por favor, aguarde…", cut paper; prints ze.dat then rze.dat, N copies each (N = CConfiguracaoEleicao +96),
   stops silently if the urna is shutting down (flag @1832936); estadoVota = zeresimaimpressa (54);
   -> CDefineRotaPreVotacao
CRegeraResumoZeresima::GetProximoEstado (11845) -> CReimprimindoZeresima
```

After a restart before voting, `CReinicioVotacao` (u07) goes to **CQuerReimprimirZeresima** (11849/11848):
refused with `LogaErro` + 9375 "Erro na verificação do comparecimento" if `CRdvVota::Comparecimento() != 0`
(nobody may have voted yet); logs "Mesário indagado se quer imprimir a zerésima"; then
`'1'`+CONFIRMA → cut paper → **CReimprimindoZeresima** (ze.dat, then the summary);
`'2'`+CONFIRMA → cut paper → **CReimprimindoResumoZeresima** (rze.dat, estadoVota 54, → CDefineRotaPosReinicio);
other text → field cleared; CORRIGE on an empty field → `CReinicioComparecimentoMesario` when mesários
must be registered (`IdentificaMesarios() && !EhTreinamentoEleitor()`, func 2520) else `CInicioVotacao`;
BRANCO → `CMaisInformacoes`.

Every printing loop is the same (inlined three times):
`MarcaImpressaoEmAndamento()` (func 1488: creates `<root>dinamico/imprimindo`), log
`Imprimindo relatório [<nome>] via nº [<n>]`, `CSigVerifier(trab, "<x>.dat", "<x>.vsu")`,
`IPaperRelatorios::ImprimeArquivo(trab/<x>.dat, verificador, "zerésima", "via n de N")` (slot 7),
`RemoveMarcaImpressaoEmAndamento()` (func 1487); after the loop slot 10 (wait for the printer).
The `imprimindo` marker lets a restarted urna know a print was interrupted.

### 3.2 Battery warning — `CExibeAlertaDesligamento` (12018/12017)

Entered from any `CEstadoComDesligamentoAutomatico` (u06) when the urna ran on battery for
`limite − aviso` seconds. `StartState` shows `CTelasVota +44`, copies `g_dataHoraDesligamento`
(@1833312) and beeps `IBeep` slot 1 (2000 Hz, 100 × 10 ms). Every 1 s tick: if the deadline passed →
log (level 2) "Tempo limite de espera usando bateria interna atingido" and `IInterfaceInit::DesligarUrna()`;
else if `IPower` no longer reports the internal battery (`status & 6 != 2`) → back to the previous state (+36).

### 3.3 Late zerésima — `CInformacaoZeresimaTardia` (11983/11982)

Reached when the mesário says the urna clock is wrong (`CImpressaoZeresimaTardia`, func 11931). On CONFIRMA:
log "Finalização de aplicativo" and "Desligando a urna", `IBeep` slot 4, `CWait::Aguarda()` (deadline =
creation of the singleton + 1 s, func 5446 busy-waits with `usleep(200)`), `DesligarUrna()`,
`m_proximoEstado = nullptr`.

### 3.4 Candidate viewer (`auxiliares/`)

`CMaisInformacoes` → **CMenuVisualizarCandidatos** (11881): logs "Opção de visualização de candidatos
selecionada" and shows a menu whose items carry a `std::any` holding `std::pair<CAppState*, CAppState*>`:

| item | chain |
|---|---|
| Por cargo | CMenuFiltrarCandidatosPorCargo → CMenuFiltrarCandidatosPorPartido → CVisualizarCandidatos |
| Por partido | CMenuFiltrarCandidatosPorPartido → CMenuFiltrarCandidatosPorCargo → CVisualizarCandidatos |
| Candidato específico | CMenuFiltrarCandidatosPorCargo → CMenuFiltrarCandidatosPorNumero → CVisualizarCandidatos |

The selection sets `first.m_proximoEstado = second`, `second.m_proximoEstado = viewer` and clears the three
filters of the viewer (`std::optional` cargo +24, número +28, partido +36). Each filter menu collects, from
the candidacies that pass the filters already set, the set of cargos (ordered by id, func 5655) or party
numbers, offers "Todos" (value 0; the cargo menu hides it when the next step is the number entry) and
stores the choice. The party menu sets the menu-wide format to `"%S"` (search text only) and gives only
the "Todos" item its own `"%S - %T"`: the parties are listed by **number only** ("0 - Todos", "12",
"13", …), with no sigla or name. The menus read the `CInputMenuField` directly
(`menu->Read(IInputKbd&)`, srcloc of the menu's own file), not through the interactive form. **CVisualizarCandidatos** (11876) filters the candidacies (9378 "Nenhuma candidatura a ser
exibida"), then loops *inside StartState*: shows `CTelasVota::CriaTelaVisualizacaoCandidato(c, i, n)` (6569:
"VISUALIZAÇÃO", cargo, Partido, Número `{:0{digits}}`, Nome, Gênero, photo or "Candidato não concorre." /
"Não há outras informações.", Situação APTO/INAPTO, Página i/n, keys 4/6 with wrap-around, CORRIGE =
Retornar), `WaitAndRead()`, and rebuilds the screen for the previous/next candidate.

## 4. End of the day (`fimvotacao`)

```
CGeraBU (u08)  -> trab/bu.dat (printed-BU image) + signature trab/bu.vsu      estadoVota 59 -> 60
CGeraRelatorios (u08) -> trab/buj.dat, bim.dat, behb.dat                        60 -> 61
CInicioBU (12062, other unit): voter training ? CQuerImprimirBU : CImprimindoBU
CQuerImprimirBU (12035/12034)   asserts 61 and qtdBU == 0 (texts say "poInfo")
    CORRIGE -> estadoVota 64, SalvaEstado -> CAplicacaoEncerrada      CONFIRMA -> CImprimindoBU
CImprimindoBU (12078/12077)     asserts 61 (:70) and qtdBU == 0 (:71); prints via 1; asks about quality
    CORRIGE (reprint): log, print via 1 again           CONFIRMA: log, qtdBU++;
       voter training -> estadoVota 64, encerramento 52 -> CEmitirMaisBU
       otherwise      -> estadoVota 62 (gravarresultados) -> CGravaResultado
CGravaResultado (12098)         writes + signs the result files (§5.2)      62 -> 63
CCopiaResultadoParaMR (u08)     copies them to /dsk/mr and verifies the BU  63 -> 64, encerramento 50
CImprimirBUOutrasObrigatorias (12065)  remaining mandatory vias (screen "telaDestinoBUs"), then
    ImprimeBoletimJustificativa ? CImprimirBJust : IdentificaMesarios ? CImprimindoBim : CRetirarMR
CImprimirBJust (12068)   buj.dat, then IdentificaMesarios ? CImprimindoBim : CRetirarMR
CImprimindoBim (12071)   bim.dat, then (!demo && biometric urna) ? CImprimindoBEHB : CRetirarMR
CImprimindoBEHB (12074)  behb.dat, then CRetirarMR      (only reachable through CImprimindoBim!)
CRetirarMR (12031/12030)        encerramento 51 (saved), waits until the MR is removed (250 ms polling),
                                encerramento 52 (saved); demo mode: 52 without waiting (not saved)
    CONFIRMA -> CEmitirMaisBU (u08)  -> extra vias -> CMostraQRCodeBU
CVerificaQtdBUsAdicionais (12023) (restart at encerramento 52): qtdBU >= obrigatórias + adicionais ?
    CLimiteCopiasBUAtingido ("Quantidade de vias adicionais excede o máximo permitido"; CONFIRMA ->
    CMostraQRCodeBU) : CEmitirMaisBU
CMostraQRCodeBU (12055/12051/5982) <-> CMostraQRCodeCertificado (12040/12038) -> CAplicacaoEncerrada
```

## 5. BOLETIM DE URNA: what this unit does, step by step

The BU exists in two forms: the **printed report** (image file `trab/bu.dat`, made by `CGeraBU`, unit u08)
and the **ASN.1 result file** `…-bu.dat` (`EntidadeBoletimUrna`, written here by `CGravadorBU`). This unit
prints the first, writes/signs the second together with every other result file, and shows the BU
QR codes on screen.

### 5.1 Printing a copy — `CImprimindoBU::ImprimeBU(uebyte numVia, PrintMessageMode modo)` (func 2890)

1. `auto& papel = api::IPaperRelatorios::GetInst()` (:111). `Assert (estVota.GetQtdBU() == numVia - 1)` (:114,
   3467): copies are printed strictly in order; `qtdBU` (`EstadoGeralVota.qtdBU`, +8, persisted in `vota.bin`)
   is incremented by the caller only after a copy was accepted.
2. Header form: text `"<n>a. VIA"` (font 2, centred) + 3 blank lines.
3. `MarcaImpressaoEmAndamento()` (creates `dinamico/imprimindo`).
4. `CSigVerifier(trab, "bu.dat", "bu.vsu")`: the printer service prints the image only if its signature
   verifies (the image and signature were produced by `CGeraBU`).
5. `modo == ComMensagem` (0): message `"boletim de urna"` and `"<n>ª via"`; `SemMensagem` (1, mandatory copies
   printed by `CImprimirBUOutrasObrigatorias`): empty texts.
6. `papel.ImprimeArquivo(trab/bu.dat, verificador, header, mensagem, via)` (IPaperRelatorios slot 8), slot 10,
   `RemoveMarcaImpressaoEmAndamento()`.

Callers and numbering:

| who | via printed | after |
|---|---|---|
| `CImprimindoBU::StartState` / reprint on CORRIGE | 1 (repeated until the mesário confirms quality) | CONFIRMA → `qtdBU = 1` |
| `CImprimirBUOutrasObrigatorias` | `qtdBU+1` … `qtdBuObrigatorio` (1 in demo mode, else parâmetro +12 of `CConfiguracaoEleicao +88`) | each copy → `qtdBU++`, `SalvaEstado()` |
| `CEmitirMaisBU` (u08) | `qtdBU+1` …, at most `GetQuantidadeMaximaBUsAdicionais()` | each → `qtdBU++`, save |
| `CVerificaQtdBUsAdicionais` | — | refuses more when `qtdBU >= obrigatórias + adicionais` (parâmetros +12 and +16; 1 and 1 in demo mode) |

Log lines (`logd.dat`): `Imprimindo relatório [BU] via nº [<n>]`, `Mesário indagado sobre qualidade do Boletim
de Urna`, `Mesário indicou reimprimir Boletim de Urna`, `Mesário indicou qualidade OK para Boletim de Urna`.
The other reports use the same mechanism with one copy ("via única"): `buj.dat/buj.vsu` ("boletim de
justificativa eleitoral", ERelatoriosUE 4), `bim.dat/bim.vsu` ("boletim de mesários", 9), `behb.dat/behb.vsu`
("boletim de eleitores\nhabilitados biograficamente", 12), IPaperRelatorios slot 7 (no header).

### 5.2 Writing and signing the result files — `CGravaResultado::StartState` (func 12098)

Preconditions (each failure throws, the encerramento stops and the error screen of the surrounding
`CApplicationContextGuard` is shown):

| check | error |
|---|---|
| `estadoVota == EAVGRAVARRESULTADOS` | assert 3456 (cgravaresultado.cpp:63) |
| `dhIniAquisicao` / `dhFimAquisicao` set | 8090 "Início da aquisição não marcado" / 8091 "Fim da aquisição não marcado" |
| `CEleitores::GetNumComparecimentos() == CRdvVota::Comparecimento()` (voters marked as having voted = ballots in the RDV) | assert 3457 (:79) |
| referential integrity RDV / cargos / respostas | `comum::VerificaIntegridadeReferencial()` (func 2543) |
| `dhEmissao` (set by CGeraBU) | 8093 "A data/hora da emissão do BU não foi registrada" |
| fase ∈ {`o`,`s`,`t`} | 8657 "Fase inválida: {}" (IResultado) / 8698 "Fase inválida: {:#x}" (ConverteFaseEcourna) |
| `etc/dependencias.properties` and `etc/versoes.properties` have `tag=20260601173148` | 8662 / 8696 "Tag do arquivo de … contratos inválida: [{}]. Tag atual: [{}]" |

Common data: `fase` = `EstadoGeralUrna` carga fase char, município / zona / local / seção and UF from
`CLocal`, `agora` = current date/time (the *data de geração* stored in every writer), the
`CDadoCorrespondencia` of `EstadoGeralUrna +60` (correspondência: numeroInternoUrna, código da carga…).

**Writers** (`std::vector<std::shared_ptr<comum::IGravador>>`, in this order). Each is a `comum::IResultado`
(`+4 município, +8 zona, +12 local, +16 seção, +18 fase, +20 nome, +32 extensão, +36 arquivo SAVD`); the file
name is `"<fase><pleito:05><UF><município:05><zona:04><seção:04>-<sufixo>"`
(`CGravadorUtil::DeterminaNomeArquivoSemLetra`):

| # | writer (unit u23) | extensão → suffix | SAVD id | content / constructor arguments seen here |
|---|---|---|---|---|
| 1 | `CGravadorBU` (304 B) | 4 → `bu.dat` | 35 | **ASN.1 `EntidadeBoletimUrna`**: dhGeração, **dhEmissão**, `ConverteFase(fase)`, correspondência, histórico of cargas (`GetGap().GetCodigosCarga()`), eleitores aptos (`CEleitores::GetQtdAptos()` map), `CRdvVota&` (the votes), comparecimento, urna biométrica, habilitações {sem biometria, biométrica, biográfica} (each checked 0..9999), dhInício/dhFim da aquisição, the order of the cargos (`map<TCargoID, n>`: first position of each cargo in `CCargos`), and the key file name **`bu.pk1`** |
| 2 | `CGravadorRDV` (168 B) | 6 → `rdv.dat` | 37 | the RDV (`CRdvVota&`), dhGeração, correspondência |
| 3 | `CGravadorRCSecao` (60 B) | 8 → `jufa.dat` | 39 | `ModuloResultadoUrnaCadastro` (justificativas/faltosos); fase as ecourna value: `o`→2, `s`→1, `t`→3 |
| 4 | `CGravadorEnvelopeArquivo` | 9 → `imgbu.dat` | 43 | `EntidadeEnvelopeGenerico` wrapping **`trab/bu.dat`** (the printed BU image), `tipoEnvelope = envelopeBoletimUrnaImpresso` (C++ `CEnvelopeGenerico::Tipo` 2) |
| 5 | `CGravadorEnvelopeArquivo` | 11 → `imgze.dat` | 44 | envelope wrapping `trab/ze.dat` (printed zerésima), `envelopeZeresimaImpressa` (C++ 4) |
| 6 | `CGravadorHashes` (96 B) | 12 → `hash.dat` | 46 | `ModuloHashes::EntidadeHashes`: UF, fase, version **"10.23.0.1 - DESENVOLVIMENTO"**, identification = `CIdentificacaoSecao(município, zona, local **= 1**, seção)` or, for seção 0, `CIdentificacaoUrnaContingencia(município, zona)` |
| 7-9 | `CGravadorWSQ` ×3 (biometric urnas only) | 16/17/18 → `wsqbio.jez`, `wsqman.jez`, `wsqmes.jez` | 48/49/50 | fingerprint images, tipo 0/1/2, last argument `!EhFaseTreinamento()` (encryption, inferred) |
| 10 | `CGravadorVersoesArquivos` (64 B) | 19 → `mr.ver` | 70 | `ModuloVersaoArquivos`: tag `20260601173148` + map module → version (below) |
| 11 | `CGravadorLog` (76 B) | 13 → `log.jez` | 60 | the log: `dinamico/log/arquivados/`, `dinamico/log/logd.dat`, trab dir |

**Versions of the ASN.1 contracts (mr.ver).** The ASN.1 modules used by the writers are collected
(`ModuloBoletimUrna`, `ModuloEnvelopeGenerico`, `ModuloResultadoUrnaCadastro`, `ModuloHashes`,
`ModuloAssinaturaEcourna`, `ModuloVersaoArquivos`), expanded with their transitive dependencies from
`etc/dependencias.properties` (`Modulo=Dep1,Dep2`, func 5869), and each module gets its version from
`etc/versoes.properties`. Both files are read once into the poly-singleton `CGeracaoVersoesContratos`, which
requires `tag == "20260601173148"` in each (the tag is compiled in).

**Signer.** `vota::CAssinadorVota(pacote)`, with pacote (ESavdPacote) = 158 in the 1st turno and 159 in the
2nd. Its inlined `comum::CAssinador` constructor sets:

* `+4` aplicação (ESavdAplic) = 1 (presumably VOTA), `+8` pacote;
* `+36` local = `std::format("{}{:05}{:04}{:04}", UF, município, zona, seção)` (must be 15 characters);
* the package file = `std::vformat(CArquivosSavd[pacote], fase, pleito (cfg +28), UF, município, zona, seção)`
  = `<result dir MI of the turno>/<fase><pleito:05><UF><município:05><zona:04><seção:04>-vota.vsc`, split
  into `+24` directory (`Diretorio()` + "/", func 5462) and `+12` file name.

**Execution** (`comum::CGravacaoResultados::Executa`, inlined; each block runs inside a
`CApplicationContextGuard` whose texts are the error screen; the progress screen
`telaProgressoEncerramento` — "Preparando dados para encerramento", "O processo pode levar alguns minutos",
a 30-step bar — advances after every file):

1. **Generate in the work directory** (`CApplicationContextGuard(2, "", "Gerando os resultados na MI", …)`):
   for each writer: log `LogaGerandoResultados(ext, INÍCIO)`, `IGravador::Grava()` (slot 4: opens
   `GetPathTrab(MI)/<nome>` with mode `"wb"` and calls the writer's slot 7 `GravaResultado(CFile&)`), log TÉRMINO.
2. **Copy to the result directory of the MI** ("Copiando arquivos para o dir. de resultados"): slot 2
   `CopiaParaResultado()`; the SAVD id of the file is appended to the list to sign; log
   `LogaCopiandoArqResParaFI`. (A second, always empty, list of writers gets the same treatment.)
3. **Sign** ("Assinando resultados da MI"): `LogaPreparandoAssinaturaArquivosResultado`,
   `LogaInicioProcedimentoAssinatura`, then `CAssinador::AssinaArquivosResultado(ids)` (cassinador.cpp:141-149):
   * empty list → 8636 "Não foi passado nenhum arquivo de resultado";
   * `EnviarAcaoHSM(savd, aplicação, ABRE)` (func 5889; message "Falha ao enviar ação ao HSM. {}" on error);
   * log "Inicia uma sessão no MSE" if `api::IUrna::GetInst()` slot 0 (model) ≥ 2020, else "…no MSD";
   * send the "local" (0xFE + the 15-char local + 0x0404 + aplicação) — **result not checked**;
   * for each file: `CApplicationContextGuard(11, "", "Erro na assinatura dos arquivos de resultado",
     "Ocorreu um erro durante a assinatura dos arquivos: <nome SAVD>")` and
     `AssinarEcourna(savd, aplicação, 128, id)` (iinterfacesavd.cpp:1022): request `{0xFE, '=', id, u16 128,
     u8 aplicação}`, failure → 7338 `"Falha ao assinar ({}-Ecourna[{}])"`;
   * `EnviarAcaoHSM(…, FECHA)`, "Finaliza a sessão no MSE/MSD", `LogaTerminoProcedimentoAssinatura`.
   The signatures accumulate in the SAVD package `…-vota.vsc` (pacote 158/159).
4. `CSynchronizer::CreateInst()->Sincroniza()` (fsync).
5. **Copy to the MV** unless the fase is training (`m_copiaParaMV = !EhFaseTreinamento()`): each writer's slot 3
   `CopiaParaMV()` (log `LogaResultadoCopiadoResFE`), then the `.vsc` signature package is **copied**
   (not re-signed) to `GetPathResult(MV)`; sync.
6. Progress bar to the end; `estadoVota = EAVCOPIARESULTADOSMR (63)`, `SalvaEstado()`,
   next `CCopiaResultadoParaMR` (u08), which copies `bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat,
   log.jez, vota.vsc, mr.ver` (+ wsq) to `/dsk/mr/` and verifies the BU copy byte by byte.

### 5.3 BU QR codes on the screen — `CMostraQRCodeBU` / `CMostraQRCodeCertificado`

* The payloads come from `IQRCodeBUDS` (`GetQRDSInst()`, cmostraqrcodebu.cpp:31/38, func 1956): a vector of
  strings built with `CGeradorBUQRCode` (the same "BU DIGITAL" payloads printed on the BU; the
  constructor rejects an empty vector with "Vetor de partes vazio") and the index of the part shown.
* Demo mode: no QR screen, straight to `CAplicacaoEncerrada`.
* Screen `telaQRCodeBU`: QR image 380 px of the current part, "Versão: <versão>", "BU digital", instruction
  "O QR code ao lado contém o resultado da votação para esta urna." (+ " Use as teclas 3 e 9 para navegar
  pelas partes do BU." when there are several parts), "Pagina i/n", keys `9` "para próxima parte", `3`
  "para parte anterior", CONFIRMA "Continuar", BRANCO "Ver certificado".
* `AjustaTela` (:195) enables/disables the labels: one part → navigation hidden, CONFIRMA enabled; first
  part → back and CONFIRMA disabled; last part → next disabled, CONFIRMA enabled; middle → CONFIRMA disabled.
  **CONFIRMA is accepted only on the last part** (so every part is shown before leaving).
* BRANCO → `CMostraQRCodeCertificado`: QR with `QRCE:1:1 IDUE:… MDUE:… CERT:<hex>` (func 12039), "Certificado
  digital", "O QR code ao lado contém o certificado desta urna.", CORRIGE "Retornar".
* Both states are `CEstadoComDesligamentoAutomatico`: while shown, the battery watchdog keeps running.

## 6. The zerésima report (`cgeradorresumozeresima.cpp`)

Parts assembled by `CGeraZeresimaBase::StartState` (11946) into an `api::CReport` and written with
`CScopedReportFile` (`IPaperRelatorios::Abre(trab/ze.dat)` … `Fecha()`):

1. header `CRelVotaUtil::CriaCabecalho("Zerésima")` (5977): `IncluiCabecalhoEleicoesMZS` (município, zona, seção,
   nome), "Eleitores aptos {:04}" (+ originais/temporários), código de identificação da UE, "Data"/"Hora" of
   emission, resumo da correspondência;
2. **extrato do RDV** (5971, inserted at position 1): asserts that there is no justification (:43) and no
   vote in any cargo (:47), prints `======` / "Não há votos ou justificativas registrados" / `======` /
   "-----------EXTRATO DO RDV-------------" and a `CParteCargos(separador de eleição, CParteRdv(…))`;
3. "---------LISTA DE CANDIDATOS----------";
4. `CParteCargos(separador de eleição, CParteCandidatos(header do cargo, "Não há candidatos concorrendo",
   majoritários, proporcionais (per party, "Não há candidato registrado para <este/esta> <PLSB>"),
   consultas))`;
5. trailer (5579): separador de fase, "Código de identificação da carga" (formatted), "Ver: 10.23.0.1",
   UF text (`CConfiguracaoEleicao +200`), 20 blank lines, paper cut.

The numbers printed come from `CDataText<std::string (*)()>` data sources evaluated at print time. The
"ids" that appear in the code (1625, 1626, 1632, 1634, 1635, 3049, 3050, 3051, 1567…) are **function-table
slots** of these data sources (e.g. 1625 → `CDataSourcesRelatorio<CRdvVota,CEleitores>::TrailerProporcional`,
1634 → `CCargoDSLabelRelatorio::HeaderDetalheZE`), not string ids.

## 7. Data read and written

| data | access |
|---|---|
| `vota.bin` (`EstadoGeralVota`: estadoVota, estadoEncerramento, qtdBU, dhIni/Fim/Emissão, treinamentoEleitor) | read / written + `SalvaEstado()` |
| `EstadoGeralUrna` (fase +48, turno +32, município/zona/seção, correspondência +60), `gap.bin` | read |
| `CConfiguracaoEleicao`: +28 pleito, +88 parâmetros (vias obrigatórias +12, adicionais +16, BJust +394, mesários +400), +96 vias da zerésima, +200 texto com UF, +260 destino das vias, +580 data limite da eleição | read |
| `trab/{bu,buj,bim,behb,ze,rze}.dat` + `.vsu` | printed (signature-checked) |
| `trab/ze.dat` | written (zerésima image) |
| `<root>dinamico/imprimindo` | created/removed around every print |
| `etc/dependencias.properties`, `etc/versoes.properties` | read (tag + module versions) |
| trab/result dirs of MI and MV: `…-bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat, wsq*.jez, mr.ver, log.jez`, `…-vota.vsc` | written / copied / signed |
| `dinamico/log/logd.dat` | log lines (Latin-1) |
| SAVD / HSM / `api::IUrna` / `IInterfaceInit` (MR present, demo mode, power off) / `IPower` / `IBeep` | services |

## 8. Web-build specifics

* None of these states is reached (§ Runtime). If they were: the printer is `simulador::CWasmNullPaper`
  (slots 7/10 no-ops, slot 8 only drops the header) — **nothing is printed and the `CSigVerifier` is never
  consulted**; `CWasmInit` reports the MR as absent (message 18 → 1 → "not present"); the SAVD is the web
  `(anonymous)::CWasmSavd` registered by `main`; `IPower` never reports the battery.
* `CGravaResultado` could not complete in the simulator: `etc/dependencias.properties` and
  `etc/versoes.properties` are 0-byte stubs in `upstream/fs/vota_web_wasm/etc/` (the CPath root @1838600 is
  `"/"`). The empty file parses into an empty `api::CIniStrings`, and the tag lookup itself,
  `CDependenciasContratos::GetValor("tag")` (func 3826 → 6044, cdependenciascontratos.cpp:59), throws
  **8663 "Propriedade inexistente: tag"** before the 8662 tag comparison is reached (static reading; the
  state is not reachable from the page).
* `emscripten_sleep` is imported but aborts (no Asyncify); see §10.

## 9. wasm / Emscripten observations

* **LTO inlining**: `CGravaResultado::StartState` (26.7 KB) is the only copy of the constructors of
  `CGravadorBU/RDV/RCSecao/Hashes/VersoesArquivos/Log`, of `CAssinador::AssinaArquivosResultado`,
  `AssinarEcourna`, `CGeracaoVersoesContratos` creation and `CProgressoEncerramento`'s constructor.
  `CGeraZeresimaBase::StartState` (6.9 KB) is the only copy of the zerésima layout.
* **Misnamed by the tools** (analyzer name → real): 11991 `ImprimeZeresima` → `CImprimindoZeresima::StartState`;
  11852/11855 likewise; 5971 `CriaTituloExtratoRDV` → outer `CriaExtratoRDV`; 11946/12098/12040 `vf2` →
  `StartState`; 12034/12026/12077/11848 `vf7` → `ProcessInput`; 3847/5914/1950/2286 "EhModoDemonstracao"
  → the `CInformacaoEleicao` getters that inline it; 11556 "CCargos::GetInst" (slot 3049) → data source of
  the current eleição name; 3796 "ValidaTipoBiometria" → `CGravadorWSQ` constructor.
* **Error codes are numbers the annotator may print as strings**: e.g. 8090, 9375, 9408, 3476 are error
  codes, not addresses ("Falha ao validar assinatura UE…", "turno2.jez", "In RTree…" in the pseudo-code are
  coincidences).
* **Latin-1**: all application strings are Latin-1 (`"zer\xE9sima"`, `"via \xFAnica"`, `"{}\xAA via"`).
* The `std::any` handlers of the menu items are table slots 1107 (`pair<CAppState*,CAppState*>`), 1108
  (`uebyte`), 1109 (`uint16_t`).
* Singleton mutexes survive only as `mutex_unlock` residues (single-threaded build).

## 10. Suspicious / notable code

1. **Abort paths through `emscripten_sleep`** (flag @1584624 = 1): `CVisualizarCandidatos::StartState`
   (11876, `WaitAndRead` with 10 ms), and the three menus 11881/11884/11888 whose `CInputMenuField::Read`
   (10894) **flushes the keyboard and then sleeps 5 ms waiting for a key**: in the web build this aborts
   immediately. Unreachable from the voter terminal the page drives. `CRetirarMR::AguardaRetiradaMR`
   (12031) also has `emscripten_sleep(250)` in its "MR still present" loop, but it is **not** an abort
   path in the simulator: `CWasmInit` answers message 18 with 1 (`"= no"`), which `IsMRPresenteSemHabilitar`
   (2862 → 6046) turns into "absent", so the loop body never runs.
2. **Modal loops inside `StartState`** (11876, 11881, 11884, 11888): the thread's event loop does not run
   while the operator browses candidates (no ticks, no messages). On the urna this is by design (the
   thread blocks on the keyboard); in a cooperative (single-thread) port it cannot work.
3. **`EntidadeHashes` identifies the section with local = 1** (12098): `CIdentificacaoSecao(município, zona,
   1, seção)` although `CLocal::GetLocalID()` is read and passed to all the other writers. Either the field is
   intentionally constant or `hash.dat` carries a wrong "local".
4. **The SAVD "local" request result is ignored** (12098, inlined `AssinaArquivosResultado`): errors
   "Argumento inválido para local." / "…tamanho do local." or a failed answer only land in the service's
   last-error string; signing proceeds.
5. **`CRetirarMR::StartState` persists before validating**: it writes `EAERETIRAMR` and calls `SalvaEstado()`
   *before* `Assert(estadoVota == EAVENCERRADA)`, and the second assert is tautological. In demo mode the
   final `EAEFIMDOSTRABALHOS` is not saved.
6. **Signature check of printed reports is skipped in the web build**: `CSigVerifier` (func 1540) only
   stores the directory and the two file names; the check belongs to the printer service, and
   `CWasmNullPaper` slot 7 (8372) is empty and slot 8 (8371) only releases the header, so the verifier is
   never consulted (nothing is "simulated"); on the urna the printer service verifies `*.vsu`.
7. **Hard-coded identity strings**: `hash.dat` records the version `"10.23.0.1 - DESENVOLVIMENTO"` and the
   contract tag `20260601173148` is compiled in; a urna with other `.properties` refuses to close the vote
   (8662/8696).
8. **`CVerificaEleicaoPassou` dead end**: when the election date has passed the state never changes (no
   input handler): only a power cycle leaves it.
9. **BEHB printing depends on the mesário report**: `CImprimindoBEHB` is only reached from
   `CImprimindoBim`, i.e. only when `IdentificaMesarios()` is configured. `CGeraRelatorios` (u08) generates
   `behb.dat` whenever the urna is biometric and not in demo mode, so with mesário identification disabled
   the BEHB is generated in `trab/` but never printed (it is not one of the files copied to the MR either).
10. **Slicing**: `CGravacaoResultados` stores a `comum::CAssinador` copied from `vota::CAssinadorVota`; harmless
   today (the subclass adds no virtual behaviour).
11. Cosmetic: assert texts say `poInfo.GetVota()` (cquerimprimirbu.cpp:40/41).
12. **Null dereference in the filter menus** (11888, 11884): when no cargo / no party passes the filters,
   the screen shows "Nenhum cargo disponível!" / "Nenhum partido disponível!" and the `CInputMenuField`
   is never created (its shared_ptr stays null: locals initialised to 0 in the wasm), yet the read loop
   right after `Exibe()` calls `menu->Read(kbd)` (vtable slot 10) through it. In the wasm this loads a vptr
   from address 0 and does `call_indirect` of table entry 0 (trap); natively it is a segfault. Reachable
   only with an election that has no candidacy at all, because each later filter step lists values taken
   from the candidacies.
13. **Party filter shows numbers only** (11884): the menu-wide format is `"%S"` and only "Todos" has
   `"%S - %T"`, so the operator chooses a party by its number without seeing its sigla or name.

## 11. Open questions

* Where `CGeraZeresimaBase::StartState` (11946) really lives. The call graph does not decide it: the
  anonymous-namespace srcloc of func 5971 belongs to `CriaTituloExtratoRDV`, which is *inlined* into 5971;
  the outer function 5971 may have external linkage, and LTO inlines across translation units anyway.
  11946 contains two nested inlined frames (64 and 400 bytes) around the report construction, which fits
  a report builder from `cgeradorresumozeresima.cpp` inlined into a StartState defined in
  `cgerazeresima.cpp` (unit u07's reading). The reconstruction keeps 11946 in `cgeradorresumozeresima.cpp`
  only because the unit definition assigns it there.
* Exact semantics of IPaperRelatorios slots 7/8/10 and IBeep slots 3/4 (names inferred from use).
* The class name of the object built by func 5824 (`CGravacaoResultados` is a guess) and the meaning of the
  `128` passed to `AssinarEcourna`.
* Whether `CIdentificacaoSecao(…, 1, …)` in the hashes is intended.
* `CConfiguracaoEleicao +580`: election date or a "data limite"? `agora >= valor` is treated as "passou".

## 12. Complete mapping table (80 functions)

* **run**: observed executing in the recorded votes (`analysis/runtime/*.functions.tsv`).
* **original file**: where the function lives in the TSE tree ("path inferred" when not attested).
* **reconstructed in**: path under `src/uenux2/src/app/` (fragments of other units' files end in `.u09.cpp`).

| idx | size | run | tools name | reconstructed symbol | original file | reconstructed in (under `src/uenux2/src/app/`) | kind / conf. |
|---|---|---|---|---|---|---|---|
| 198 | 19 |  | `comum_f198` | `api::CPaperFormBuilder::AddNewLine` | uenux2/src/api/gui/cpaperformbuilder.h (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | library-like helper / medium |
| 601 | 34 |  | `vota_f601` | `api::CFormPart::CFormPart` | uenux2/src/api/gui/cformpart.h (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | inline ctor / medium |
| 604 | 490 |  | `comum_f604` | `api::CPaperFormBuilder::AddData` | uenux2/src/api/gui/cpaperformbuilder.h (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | library-like helper / medium |
| 1127 | 491 |  | `vota_f1127` | `vota::CLogVota::LogaImpressaoRelatorio` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / medium |
| 1249 | 262 | ✓ | `comum_f1249` | `comum::md::estadoaplicacao::CDadoCorrespondencia::CDadoCorrespondencia(const CDadoCorrespondencia&)` | uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocorrespondencia.h (implicit, path inferred) | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix comment) | compiler-generated copy ctor / medium |
| 1541 | 197 |  | `comum_f1541` | `comum::CRelUtil::IncluiResumoCorrespondencia` | uenux2/src/app/comum/relatorios/crelutil.cpp (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | TSE helper / medium |
| 1588 | 640 |  | `vota_f1588` | `vota::(anonymous namespace)::AdicionaCampo` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | vota/eleitor/comum/ctelasvota.u09.cpp | TSE / low |
| 2859 | 95 |  | `api_f2859` | `std::map<std::string, api::CIniSection>::__tree::destroy` (node: key +16, CIniSection = name +28 + map<string,CIniKey> +40 via func 1397; the section maps of CDependenciasContratos / CVersoesContratos) | library: libcxx/include/__tree | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix comment) | library instantiation / medium |
| 2871 | 217 |  | `vota::CVisualizarCandidatos::vf0` | `vota::CVisualizarCandidatos::~CVisualizarCandidatos` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE (dtor) / high |
| 2877 | 135 |  | `vota_f2877` | `std::map<K, std::vector<{int,std::string}>>::__tree::destroy` | library: libcxx/include/__tree | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | library instantiation / medium |
| 2879 | 56 |  | `vota::CInformacaoZeresimaTardia::vf0` | `vota::CInformacaoZeresimaTardia::~CInformacaoZeresimaTardia` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE (dtor) / high |
| 2883 | 56 |  | `vota::CExibeAlertaDesligamento::vf0` | `vota::CExibeAlertaDesligamento::~CExibeAlertaDesligamento` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE (dtor) / high |
| 2890 | 1886 |  | `vota::CImprimindoBU::ImprimeBU` | `vota::CImprimindoBU::ImprimeBU` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / high |
| 3699 | 238 |  | `comum_f3699` | `comum::CriaHeaderCargo` | uenux2/src/app/comum/relatorios/crelutil.cpp (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | TSE helper / low |
| 3862 | 72 |  | `vota_f3862` | `std::set<comum::md::CCargo>::__tree::destroy` | library: libcxx/include/__tree | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp (comment) | library instantiation / medium |
| 3873 | 230 |  | `vota_f3873` | `vota::(anonymous namespace)::CriaHeaderDetalheZE` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / low |
| 3877 | 140 |  | `vota_f3877` | `vota::CAplicacaoEncerrada::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/caplicacaoencerrada.cpp (path inferred) | vota/eleitor/fimvotacao/caplicacaoencerrada.u09.cpp | TSE / high |
| 4551 | 174 |  | `vota_f4551` | `vota::CLogVota::LogaMesarioIndagadoQualidadeBU` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / low |
| 5446 | 101 |  | `api_f5446` | `api::CWait::Aguarda` | uenux2/src/api/util/cwait.cpp | ../api/util/cwait.u09.cpp | TSE / medium |
| 5462 | 227 |  | `comum_f5462` | `comum::(anonymous namespace)::Diretorio` | uenux2/src/app/comum/gravadores/cassinador.cpp (path inferred) | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix) | TSE helper / low |
| 5579 | 638 |  | `vota_f5579` | `vota::(anonymous namespace)::CriaTrailer` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / low |
| 5580 | 489 |  | `vota_f5580` | `vota::(anonymous namespace)::CriaSeparadorEleicao` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / low |
| 5587 | 54 |  | `vota_f5587` | `comum::CParteCargos::CParteCargos` | uenux2/src/app/comum/relatorios/cpartecandidatos.h (inline, path inferred) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (appendix) | inline ctor / medium |
| 5655 | 23 |  | `vota_f5655` | `std::compare_three_way for TCargoID (uebyte)` | library / inline comparator | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | library/inlined helper / medium |
| 5824 | 562 |  | `comum_f5824` | `comum::CGravacaoResultados::CGravacaoResultados` | uenux2/src/app/comum/gravadores/ (file unknown; class name inferred) | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / low |
| 5856 | 176 |  | `comum_f5856` | `comum::CGravadorEnvelopeArquivo::CGravadorEnvelopeArquivo` | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (path inferred) | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix) | TSE / high |
| 5866 | 51 |  | `api_f5866` | `comum::CGeracaoVersoesContratos::~CGeracaoVersoesContratos` (two api::CIniStrings: +0/+24 section maps via 2859, +12/+36 key maps via 1397) | uenux2/src/app/comum/gravadores/cgeracaoversoescontratos.cpp | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix) | TSE (dtor body) / medium |
| 5868 | 367 |  | `comum_f5868` | `std::set<std::string>::insert(const std::string&)` | library: libcxx/include/__tree | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix) | library instantiation / medium |
| 5869 | 402 |  | `comum_f5869` | `comum::(anonymous namespace)::ColetaDependencias` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp (probable) | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / low |
| 5870 | 87 |  | `comum_f5870` | `std::make_format_args for the contract-tag error (tag, "20260601173148")` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp (inlined format helper) | vota/eleitor/fimvotacao/cgravaresultado.cpp (appendix) | library/inlined helper / low |
| 5882 | 167 |  | `vota_f5882` | `vota::CLogVota::LogaMesarioIndagadoImprimirZeresima` | uenux2/src/app/vota/log/clogvota.cpp | vota/log/clogvota.u09.cpp | TSE / low |
| 5941 | 22 |  | `vota_f5941` | `vota::CReimprimindoZeresima::GetInst` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / high |
| 5943 | 98 |  | `api_f5943` | `vota::CDefineRotaPosReinicio::GetInst` | uenux2/src/app/vota/eleitor/cdefinerotaposreinicio.cpp | vota/eleitor/cdefinerotaposreinicio.u09.cpp | TSE / high |
| 5970 | 181 |  | `vota_f5970` | `vota::(anonymous namespace)::CriaTrailerMajoritario` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / low |
| 5971 | 2742 |  | `vota::(anonymous namespace)::CriaTituloExtratoRDV` | `vota::CriaExtratoRDV` (inlines the anonymous-namespace CriaTituloExtratoRDV; the outer function's linkage/namespace is unknown) | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / low |
| 5977 | 565 |  | `vota_f5977` | `vota::CRelVotaUtil::CriaCabecalho` | uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred) | vota/comum/crelvotautil.u09.cpp | TSE / high |
| 5982 | 1514 |  | `vota::CMostraQRCodeBU::AjustaTela` | `vota::CMostraQRCodeBU::AjustaTela` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / high |
| 5986 | 140 |  | `vota_f5986` | `vota::CImprimindoBU::GetInst` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / high |
| 6569 | 4055 |  | `vota_f6569` | `vota::CTelasVota::CriaTelaVisualizacaoCandidato` | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | vota/eleitor/comum/ctelasvota.u09.cpp | TSE / high |
| 11845 | 5 |  | `vota::CRegeraResumoZeresima::vf10` | `vota::CRegeraResumoZeresima::GetProximoEstado` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (probable; u07: cgeraresumozeresima.cpp) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / medium |
| 11848 | 349 |  | `vota::CQuerReimprimirZeresima::vf7` | `vota::CQuerReimprimirZeresima::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / high |
| 11849 | 250 |  | `vota::CQuerReimprimirZeresima::StartState` | `vota::CQuerReimprimirZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | vota/eleitor/iniciovotacao/cquerreimprimirzeresima.cpp | TSE / high |
| 11852 | 1153 |  | `vota::CReimprimindoZeresima::ImprimeZeresima` | `vota::CReimprimindoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | vota/eleitor/iniciovotacao/creimprimindozeresima.cpp | TSE / high |
| 11855 | 1305 |  | `vota::CReimprimindoResumoZeresima::ImprimeResumoZeresima` | `vota::CReimprimindoResumoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp | vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp | TSE / high |
| 11865 | 114 |  | `vota::CGeraDadosDinamicos::StartState` | `vota::CGeraDadosDinamicos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp | TSE / high |
| 11876 | 927 |  | `vota::CVisualizarCandidatos::StartState` | `vota::CVisualizarCandidatos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE / high |
| 11877 | 13 |  | `vota::CVisualizarCandidatos::vf1` | `vota::CVisualizarCandidatos::~CVisualizarCandidatos (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | TSE (dtor) / high |
| 11880 | 38 |  | `vota_f11880` | `vota::CVisualizarCandidatos singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp (compiler-generated) | vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp | compiler-generated / medium |
| 11881 | 2561 |  | `vota::CMenuVisualizarCandidatos::StartState` | `vota::CMenuVisualizarCandidatos::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.cpp | TSE / high |
| 11884 | 3643 |  | `vota::CMenuFiltrarCandidatosPorPartido::StartState` | `vota::CMenuFiltrarCandidatosPorPartido::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporpartido.cpp | TSE / high |
| 11888 | 3084 |  | `vota::CMenuFiltrarCandidatosPorCargo::StartState` | `vota::CMenuFiltrarCandidatosPorCargo::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatosporcargo.cpp | TSE / high |
| 11914 | 580 |  | `vota::CVerificaEleicaoPassou::StartState` | `vota::CVerificaEleicaoPassou::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cverificaeleicaopassou.cpp | vota/eleitor/iniciovotacao/cverificaeleicaopassou.cpp | TSE / high |
| 11946 | 6878 |  | `vota::CGeraZeresimaBase::vf2` | `vota::CGeraZeresimaBase::StartState` | file not settled: cgerazeresima.cpp (u07) or cgeradorresumozeresima.cpp; the report builder inlined into it (nested 64/400-byte frames) comes from cgeradorresumozeresima.cpp (§11) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp | TSE / medium |
| 11982 | 479 |  | `vota::CInformacaoZeresimaTardia::ProcessInput` | `vota::CInformacaoZeresimaTardia::ProcessInput` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE / high |
| 11983 | 29 |  | `vota::CInformacaoZeresimaTardia::vf2` | `vota::CInformacaoZeresimaTardia::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE / high |
| 11984 | 13 |  | `vota::CInformacaoZeresimaTardia::vf1` | `vota::CInformacaoZeresimaTardia::~CInformacaoZeresimaTardia (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | TSE (dtor) / high |
| 11986 | 38 |  | `vota_f11986` | `vota::CInformacaoZeresimaTardia singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp (compiler-generated) | vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp | compiler-generated / medium |
| 11991 | 2577 |  | `vota::CImprimindoZeresima::ImprimeZeresima` | `vota::CImprimindoZeresima::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cimprimindozeresima.cpp | vota/eleitor/iniciovotacao/cimprimindozeresima.cpp | TSE / high |
| 12017 | 337 |  | `vota::CExibeAlertaDesligamento::ProcessTick` | `vota::CExibeAlertaDesligamento::ProcessTick` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE / high |
| 12018 | 119 |  | `vota::CExibeAlertaDesligamento::StartState` | `vota::CExibeAlertaDesligamento::StartState` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE / high |
| 12019 | 13 |  | `vota::CExibeAlertaDesligamento::vf1` | `vota::CExibeAlertaDesligamento::~CExibeAlertaDesligamento (deleting)` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | TSE (dtor) / high |
| 12022 | 38 |  | `vota_f12022` | `vota::CExibeAlertaDesligamento singleton at-exit reset` | uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp (compiler-generated) | vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp | compiler-generated / medium |
| 12023 | 316 |  | `vota::CVerificaQtdBUsAdicionais::StartState` | `vota::CVerificaQtdBUsAdicionais::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp | vota/eleitor/fimvotacao/cverificaqtdbusadicionais.cpp | TSE / high |
| 12026 | 130 |  | `vota::CLimiteCopiasBUAtingido::vf7` | `vota::CLimiteCopiasBUAtingido::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | TSE / high |
| 12027 | 108 |  | `vota::CLimiteCopiasBUAtingido::StartState` | `vota::CLimiteCopiasBUAtingido::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp | TSE / high |
| 12030 | 208 |  | `vota::CRetirarMR::ProcessInput` | `vota::CRetirarMR::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | vota/eleitor/fimvotacao/cretirarmr.cpp | TSE / high |
| 12031 | 621 |  | `vota::CRetirarMR::StartState` | `vota::CRetirarMR::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp | vota/eleitor/fimvotacao/cretirarmr.cpp | TSE / high |
| 12034 | 169 |  | `vota::CQuerImprimirBU::vf7` | `vota::CQuerImprimirBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / high |
| 12035 | 158 |  | `vota::CQuerImprimirBU::StartState` | `vota::CQuerImprimirBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cquerimprimirbu.cpp | vota/eleitor/fimvotacao/cquerimprimirbu.cpp | TSE / high |
| 12038 | 184 |  | `vota::CMostraQRCodeCertificado::ProcessInput` | `vota::CMostraQRCodeCertificado::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | TSE / high |
| 12040 | 1932 |  | `vota::CMostraQRCodeCertificado::vf2` | `vota::CMostraQRCodeCertificado::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | vota/eleitor/fimvotacao/cmostraqrcodecertificado.cpp | TSE / high |
| 12051 | 446 |  | `vota::CMostraQRCodeBU::ProcessInput` | `vota::CMostraQRCodeBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / high |
| 12055 | 5287 |  | `vota::CMostraQRCodeBU::StartState` | `vota::CMostraQRCodeBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | vota/eleitor/fimvotacao/cmostraqrcodebu.cpp | TSE / high |
| 12065 | 1790 |  | `vota::CImprimirBUOutrasObrigatorias::StartState` | `vota::CImprimirBUOutrasObrigatorias::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp | TSE / high |
| 12068 | 1001 |  | `vota::CImprimirBJust::StartState` | `vota::CImprimirBJust::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.cpp | vota/eleitor/fimvotacao/cimprimirbjust.cpp | TSE / high |
| 12071 | 1074 |  | `vota::CImprimindoBim::StartState` | `vota::CImprimindoBim::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobim.cpp | vota/eleitor/fimvotacao/cimprimindobim.cpp | TSE / high |
| 12074 | 964 |  | `vota::CImprimindoBEHB::StartState` | `vota::CImprimindoBEHB::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.cpp | vota/eleitor/fimvotacao/cimprimindobehb.cpp | TSE / high |
| 12077 | 600 |  | `vota::CImprimindoBU::vf7` | `vota::CImprimindoBU::ProcessInput` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / high |
| 12078 | 199 |  | `vota::CImprimindoBU::StartState` | `vota::CImprimindoBU::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp | vota/eleitor/fimvotacao/cimprimindobu.cpp | TSE / high |
| 12098 | 26731 |  | `vota::CGravaResultado::vf2` | `vota::CGravaResultado::StartState` | uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp | vota/eleitor/fimvotacao/cgravaresultado.cpp | TSE / high |

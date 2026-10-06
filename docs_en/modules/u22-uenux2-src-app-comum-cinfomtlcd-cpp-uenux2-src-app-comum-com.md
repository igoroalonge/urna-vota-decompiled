# u22 — mesário registration, election-model accessors, RDV ballot recording, storage paths and signature checks (`uenux2/src/app/comum`: `comparecimentomesario/`, `dados/md/…`, `cpath`, `cpacotearquivos`, `cinfomtlcd`)

Unit u22 has **107 wasm functions**; **16 of them ran** during the recorded web votes. The builder grouped
them by source directory, so the unit is a collection of five independent pieces of `uenux2/src/app/comum`
plus a few functions of other directories that were attributed here:

| piece | original files | functions | ran in the simulator? |
|---|---|---|---|
| **A. Registration of mesários** (*comparecimento de mesários*, poll-worker attendance) | `comparecimentomesario/` (17 files: controller, DAO, 15 state files) | 52 | no (0/52) — the operator thread never runs in the web build |
| **B. Recording one voter's ballot into the in-memory RDV** | `dados/md/rdv/cvoto.cpp`, `cvotoscargos.cpp`, `cvotoseleicoesvota.cpp` (+ func 4454 of `vota/eleitor/celeitorvotando.cpp`) | 9 | **yes** (4/9, on every vote) |
| **C. Election-model accessors and validation** (cargo, pleito, eleição, processo; two ASN.1 converters; the cargo-name data source) | `dados/md/processoeleitoral/*.cpp`, `dados/asn/…`, `vota/eleitor/comum/ctelasvota.cpp` | 20 | partly (5/20: names, "1ª vaga", pleito load) |
| **D. Storage paths, the SAVD file table and the start-up signature check** | `cpath.cpp`, `carquivossavd.cpp` (constructor), `cpacotearquivos.cpp` | 14 | **yes** (5/14, during `votaInit`) |
| **E. Battery icon / pictures on the mesário-terminal LCD** | `cinfomtlcd.cpp` (+ `BatteryIconDataSource` template code) | 9 | `Update` (1/9) |
| generic library helpers emitted here | `std::to_string(int)`, `operator+(const char*, string&&)`, the `CDadosError` factory thunk | 3 | `to_string` (1/3) |

Reconstructed sources (all under `src/uenux2/src/app/`):
`comum/cinfomtlcd.{h,cpp}`, `comum/cpath.{h,cpp}`, `comum/cpacotearquivos.{h,cpp}`,
`comum/carquivossavd.u22.cpp` (fragment: the 77 KB constructor), `comum/comparecimentomesario/`
`icontroladorregistramesarios.h` *(path inferred)*, `ccontroladorreconhecimetomesario.{h,cpp}`,
`md/ccomparecimentomesario.h` *(path inferred)*, `dao/ccomparecimentomesariodao.{h,cpp}`,
`estados/estadosregistromesarios.h` *(one header for all state classes, inferred)* and one `.cpp` per state file,
`comum/dados/md/processoeleitoral/{ccargo.h,ccargo.cpp,cdetalhecandidato.cpp,celeicaope.cpp,cpleito.cpp,cprocessoeleitoral.cpp}`,
`comum/dados/md/rdv/{cvoto.cpp,cvotoscargos.h,cvotoscargos.cpp,cvotoseleicoesvota.cpp,cvotos.u22.cpp}`,
`comum/dados/asn/processoeleitoral/cconversornomescargo.cpp` *(path inferred)*,
`comum/dados/asn/cconversorpartido.cpp` *(path inferred)*, and two fragments for other owners:
`vota/eleitor/celeitorvotando.u22.cpp` (func 4454 `GravaVotos`) and `vota/eleitor/comum/ctelasvota.u22.cpp`
(func 12587). The complete function map is in §13.

---

## 1. Glossary

| term | meaning |
|---|---|
| mesário | poll worker of the section (usually 4: presidente, 1º/2º mesário, secretário) |
| comparecimento de mesários / registro de mesários | recording which mesários were present; stored per (título, período) in the SQLite table `comparecimento_mesario` of `uenux.db` and printed on the BIM |
| período presente | 1 = abertura (registered before or during the vote), 2 = encerramento (after the vote) |
| MT (micro-terminal, *terminal do mesário*) | the operator keypad with a 4×40 text LCD, a small graphic LCD, LED, buzzer and fingerprint sensor |
| título (de eleitor) | 12-digit voter-card number; the mesário identifies himself with it |
| digital / batimento | fingerprint / fingerprint comparison ("batimento de digitais") |
| WSQ | fingerprint image format; captured prints are stored encrypted as `me<nnnnnn>.wsq` |
| cédula | ballot: the (cargo, voto) pairs one voter confirmed for one eleição |
| RDV (*registro digital do voto*) | the list of all votes per eleição and cargo, not linked to any voter, kept sorted |
| comparecimento | turnout: number of ballots cast |
| cargo / consulta | office to vote for / referendum question (modelled as a cargo) |
| escolha / vaga | seat of a multi-seat cargo (Senador with 2 seats: "1ª vaga", "2ª vaga") |
| suplente / suplência | running mate (vice, 1º/2º suplente de senador) |
| pleito | round (1º / 2º turno) of the processo eleitoral |
| SAVD | the urna's file signing/verification service (`comum::IInterfaceSavd`); `.vsu` = signature package on the urna, `.vsc` = on the result media |
| MI / MV | memória interna (`/dsk/fi`, internal flash) / memória de votação (`/dsk/fe`, removable flash card) |
| MR | memória de resultado (`/dsk/mr`, the results pen drive) |
| SA | *sistema de apuração*, the counting application used when an urna fails (it has its own `dinamico/sa/<seção>/` tree) |
| BIM | *boletim de identificação de mesários*, printed at the end of the day with the mesários' names |

---

## 2. Classes and hierarchy (RTTI)

```
api::CState
└─ comum::CAppState                                        (+4 m_proximoEstado, +8..+10 msg/key/tick flags)
   ├─ comum::CRegistrarMesarios            vt @1593592     "Registrar mesário?"              28 B, flags 2
   ├─ comum::CPedeTituloMesario            vt @1593728     dispatcher by period              12 B, flags 0
   ├─ comum::IPedeTituloMesario            vt @1595584     "Informe o título do mesário:"    28 B, flags 2 (slot 9 pure)
   │    ├─ CPedeTituloMesarioInicial       vt @1593960     slot 9 -> 1 (abertura)
   │    ├─ CPedeTituloMesarioVotacao       vt @1594264     slot 9 -> 1 (abertura)
   │    └─ CPedeTituloMesarioFinal         vt @1594112     slot 9 -> 2 (encerramento)
   ├─ comum::CTituloMesarioVazio           vt @1594568     "Título inválido: número vazio"
   ├─ comum::CTituloMesarioInvalido        vt @1594656     "Título inválido: <título>"
   ├─ comum::CTituloMesarioJaRegistrado    vt @1594760     "Título: <título> / Já registrado"
   ├─ comum::CPedeDigitalMesario           vt @1595336     fingerprint capture               32 B, flags 6
   ├─ comum::CDigitalMesarioNaoReconhecida vt @1595248     time-out screen                   20 B, flags 2
   ├─ comum::CGestorDadoMesario            vt @1595176     dispatcher by period              12 B, flags 0
   ├─ comum::IGestorDadoMesario            ti @1594976     saves the row (slot 2), slot 9 = period
   │    ├─ CGestorDadoMesarioInicial       vt @1594996     slot 9 -> 1
   │    ├─ CGestorDadoMesarioVotacao       vt @1595056     slot 9 -> 1
   │    └─ CGestorDadoMesarioFinal         vt @1595116     slot 9 -> 2
   ├─ comum::CMesarioRegistrado            vt @1594880     "Mesário registrado com sucesso."
   ├─ comum::CLimiteMesariosRegistradosAtingido vt @1594416 "Limite de mesários registrados atingido"
   ├─ comum::CConfirmaFimRegistroMesarios  vt @1593800     "Finalizar registro de mesários?"
   └─ comum::CEncerraRegistroMesarios      vt @1593888     hands control back to VOTA        28 B, flags 0

comum::IControladorRegistraMesarios  (ti @1534552)  ── vota::CControladorRegistraMesariosVota (vt @1586608, 37 slots)
comum::CControladorReconhecimentoMesario      (no RTTI, singleton @1909960, 24 B)
comum::CRegistradorMesario                     (no RTTI, singleton @1909932, 52 B: CDataMap<PK,row> + CComparecimentoMesarioServico)
comum::dao::CComparecimentoMesarioDAO : IComparecimentoMesarioDAO : IUenuxGenericDAO<CComparecimentoMesario, CComparecimentoMesarioPK> : IDAO
comum::md::CComparecimentoMesario (52 B), CComparecimentoMesarioPK (20 B)          (no RTTI)

comum::CInfoMTLCD : api::IObserver<api::SharedIImage>            vt @1551828 (singleton @1838572, 64 B)
api::BatteryIconDataSource<Vertical|Horizontal> : api::IObservable<SharedIImage>   vt @1551916 / @1581580

comum::asn::CConversorNomesCargo : IConversorASN<ModuloEleicao::NomesCargo, md::CNomesCargo>   vt @1570172
comum::asn::CConversorPartido    : IConversorASN<ModuloPartidos::Partido, md::CPartido>         vt @1561268
```

The non-polymorphic classes of this unit (`CPath`, `CArquivosSavd`, `CPacoteArquivos`, `md::CCargo`,
`md::CPleito`, `md::CEleicaoPE`, `md::CDetalheCandidato`, `md::CProcessoEleitoral`, `md::CVoto`,
`md::CVotos`, `md::CVotosCargos`, `md::CVotosEleicoesVota`) have no RTTI; their names come from srcloc
signatures, their layouts from the field accesses (documented in the headers).

---

## 3. Piece A — registration of mesários (`comparecimentomesario/`)

### 3.1 What it is for

Before the vote starts, during the vote and after it ends, the mesários of the section identify themselves
at the **MT** (operator terminal) with their título; if the election uses biometrics, their fingerprint is
also captured. Each registration becomes a row of the SQLite table `comparecimento_mesario` (in
`dinamico/trab<turno>/uenux.db`, signed as `uenux.vsu` and copied to the MV). At the end of the day VOTA
prints the **BIM** ("comparecimento de mesários na abertura / no fechamento", u08/u09) from these rows, using
`CImprimirIdentificacaoMesariosFinal::GetNomeMesario` (func 5385) to print each name.

The comum states are host-independent: everything that depends on VOTA goes through
`comum::IControladorRegistraMesarios` (reconstructed with all 37 slots in
`icontroladorregistramesarios.h`; slot names are inferred from `vota::CControladorRegistraMesariosVota`,
funcs 10766–10800). The most important ones:

| slot | VOTA implementation | meaning |
|---|---|---|
| 5 `AtualizaEstadoRegistro` | `estadoVota` '6'→'7', '9'→':' (and '8'→'7' when nobody has voted and no mesário is registered, outside training) | enter the registration phase |
| 6 `GetPeriodoRegistro` | table @534720 `{1,2,0,3}` indexed by `estadoVota - '7'` | 1 INICIAL ('7' REGISTROMESARIOINICIAL), 2 VOTACAO ('8' VOTAR), 3 FINAL (':' REGISTROMESARIOFINAL), 0 otherwise |
| 7 `LimiteMesariosAtingido` | more than 5 rows for the period (abertura for INICIAL/VOTACAO, encerramento for FINAL) | maximum **6** mesários per period |
| 9/11/12 | `CAguardaInicio` / `CRegistroMesarioEncerrado` / `CFinalizaOperador` | where the operator thread goes afterwards |
| 10/15 | post message 11 / message 7 to the voter thread | 7 = start of the BU generation chain (`CAguardaMensagem::vf6(7)` → `CGeraBU`, u09) |
| 13/14 | `CEleitores` lookup of the typed título (tipo 1) | is the mesário a voter of this section? |
| 16 | `vota_f4662` = `CSincronizaVota::SincronizaBancoDados` | sign `uenux.db`, copy to the MV |
| 17/18 | `CThreadOperador` +96 | the typed título |
| 19–36 | `CLogVota` entries | "Mesário {} registrado", "Operador confirmou o registro de mesários", … (exact texts in the header) |

### 3.2 State machine (operator thread)

```
VOTA (CAguardaInicio / CEscolheOpcao "3-Registrar mesários" / CFimAquisicaoVotos)
  └─> CRegistrarMesarios  StartState: log "indagado"; AtualizaEstadoRegistro; limit? -> CLimite…
        MT: "Registrar mesário?" / "CORRIGE: Cancelar  CONFIRMA: Prosseguir"      (buzzer 51,10)
        CONFIRMA (log "confirmou") -> CPedeTituloMesario        CORRIGE (log "cancelou") -> CConfirmaFimRegistroMesarios
CPedeTituloMesario  (dispatcher, by GetPeriodoRegistro)
        1 -> CPedeTituloMesarioInicial   2 -> …Votacao   3 -> …Final   0 -> stays (nothing drawn)
IPedeTituloMesario  MT: "Informe o título do mesário: " [clock] / [12-digit input] /
                         "Quantidade de mesários registrados:" [nn] / "CORRIGE: Encerrar  CONFIRMA: Prosseguir"
        (the constructor logs "Registrando mesários antes|durante|após a votação" - once per process)
        CORRIGE with empty input -> CConfirmaFimRegistroMesarios   (with digits: the field erases them)
        CONFIRMA: empty -> CTituloMesarioVazio
                  título = format("{:0>{}}", digits, 12); SetTituloMesario(título)
                  !CValidadorIdentidade::Valida(TITULO, título) || título == "000000000000" -> CTituloMesarioInvalido
                  CRegistradorMesario already has (título, período) -> CTituloMesarioJaRegistrado
                  urna abroad (UF "ZZ") or biometrics disabled (cfg +665) -> CGestorDadoMesario
                  else -> CPedeDigitalMesario
CTituloMesarioVazio / Invalido / JaRegistrado   CORRIGE -> CPedeTituloMesario   (log "título inválido" / "já registrado")
CPedeDigitalMesario (15 s time-out tick, 50 ms read tick)
        MT: "Solicite que o(a) mesário(a)" / <name> / "posicione POLEGAR ou INDICADOR no sensor" / "CORRIGE: Cancelar"
        CORRIGE -> CPedeTituloMesario;   15 s -> CDigitalMesarioNaoReconhecida ("Mesário(a) não reconhecido(a)." CONFIRMA -> CPedeTituloMesario)
        50 ms: frames (3, entropy > 4 bits; first frame on models 2009/2010/2020/2022), finger present ->
               "Por favor, aguarde."; LED 1; [emscripten_sleep(1000)]; invert/flip image; WSQ-encode it
               (the shared image used below is the ENCODER OUTPUT, empty in this build: stub); then
               mesário is a voter of the section:
                   with fingerprints in the roll: estado = 2; compare with fingers 1,6,2,7 (threshold 20);
                        match -> log "Realizada a conferência…", dedo = finger;  no match -> log scores, store image
                   without fingerprints:        estado = 3; store image
                   ("store image" = SalvaDigitalMesario, func 5382: always sets dedo = 0; sets id_arquivo only
                    when the image was written)
                   log "Mesário {} é eleitor da seção"
               not a voter of the section:       estado = 3; store image; log "… não é eleitor da seção"
               -> CGestorDadoMesario
CGestorDadoMesario (dispatcher by period) -> CGestorDadoMesario{Inicial,Votacao,Final}
IGestorDadoMesario::StartState: row = CComparecimentoMesario(título, período (1|2), pertenceSeção, estado, dedo,
        now, [idArquivo]); CRegistradorMesario.Update(row); SaveCurrent (DAO Inserir); SincronizaBancoDados
        -> CMesarioRegistrado
CMesarioRegistrado  log "Mesário {} registrado"; limit reached? -> CLimite…; else log "indagado se continua"
        MT: "Mesário registrado com sucesso." / "Continuar registrando mesários?" / "CORRIGE: Encerrar  CONFIRMA: Prosseguir"
        CONFIRMA -> CPedeTituloMesario        CORRIGE -> CConfirmaFimRegistroMesarios
CLimiteMesariosRegistradosAtingido  MT: "Limite de mesários registrados atingido" / "Quantidade máxima permitida: 6" /
        "CONFIRMA: Finalizar";  log "Limite … Limite máximo: 6";  CONFIRMA -> CEncerraRegistroMesarios
CConfirmaFimRegistroMesarios  MT: "Finalizar registro de mesários?" / "CORRIGE: Voltar  CONFIRMA: Finalizar"
        CORRIGE -> (log "confirmou") CPedeTituloMesario        CONFIRMA -> CEncerraRegistroMesarios
CEncerraRegistroMesarios (by period)  0/1: -> VOTA slot 9 (CAguardaInicio), log "encerrou", message 11 to the voter
                                      2:   -> slot 11 (CRegistroMesarioEncerrado), log
                                      3:   -> slot 12 (CFinalizaOperador), log, message 7 (BU generation starts)
```

Meanwhile every state shows the same form on the **voter screen** (func 1255): status header 7,
"REGISTRO DE MESÁRIOS" at (320,150) and "Siga as instruções no terminal do mesário." at (320,250).

### 3.3 Data written

`CComparecimentoMesario` (52 bytes) ↔ `comparecimento_mesario` row (DDL executed at start-up, u02 §3.3):

| field (offset) | column | values |
|---|---|---|
| `CEleitorIdentidade` (+0: string, +12: tipo) | `numero_titulo` BIGINT (`CStringUtils::ToQWord`), `tipo_identificador` | tipo always 1 (título) here |
| período (+16) | `periodo_presente` | 1 abertura (Inicial and Votação!), 2 encerramento |
| pertence à seção (+20) | `pertence_secao` | IControlador slot 13 |
| estado (+24) | `estado_biometria` | 1..3 (`ConverteEstadoBiometria` rejects others: 7755) — 1 no capture, 2 fingerprints in the roll were compared, 3 capture stored |
| dedo (+28) | `dedo_habilitacao` | 0..10 (`ConverteDedoHabilitacao`, 7756) — finger that matched, 0 otherwise |
| `api::CDateTime` (+32) | `data_hora_registro` | µs since the Julian epoch: `timegm(...) * 1e6 + 210866803200000000` |
| `optional<uint32>` (+44, flag +48) | `id_arquivo` (NULL when absent) | id of the stored `me<id>.wsq` |

DML: `INSERT INTO comparecimento_mesario (...) VALUES (?, ?, ?, ?, ?, ?, ?, ?)` (Inserir, 10403);
`SELECT … WHERE numero_titulo = ? and periodo_presente = ?` (Recuperar, 10402);
`SELECT … ORDER BY pertence_secao ASC, numero_titulo ASC, periodo_presente ASC, dedo_habilitacao ASC`
(RecuperarTodos, 10401). Captured images: encrypted WSQ under `<trab>/wsq/operador/` (both flashes, via
`CControlaArmazenamentoDeImagens`, u24) and `me{:06}.wsq` under `<trab MV>/wsq/registrado/` (func 5371,
requires ≥ 5 MiB free).

### 3.4 Web build

None of these 52 functions ran: the simulator never runs the operator thread (u06/u10), no
`IControladorRegistraMesarios` is registered in web mode (`CAjusteInicial`, which pushes the VOTA
controller, is skipped), and `IFingerScanner` / `IFingerMatcher` / `IFingerDetection` have no implementation.
The table `comparecimento_mesario` is created (it exists, empty, in the MEMFS snapshots) but never written.
The template extractor and WSQ encoder are empty stubs in this build (u10 §6.4), so even
`CControladorReconhecimentoMesario::ComparaDigitais` (func 5372) compares two empty templates; its two
vector parameters were removed by LTO dead-argument elimination (the wasm function only receives `this`).

---

## 4. Piece B — recording a ballot in the RDV (`dados/md/rdv/`, func 4454)

`vota::CEleitorVotando::GravaVotos` (func 4454, named "CVotosCargos::ConfereCedula" by the tools) runs when
the voter confirms the last cargo (from `NeedChangeState`) or is suspended. It was **observed in both recorded
votes**. Step by step:

1. `CThreadEleitor::StopTick(m_tickEleitorDemorando)`.
2. **Split by eleição.** For every `(cargo, CVoto)` in `g_votosEleitor` (@1833300), find the eleição of the cargo
   in `CConfiguracaoEleicao`'s (eleição, cargo) list (func 3774); unknown cargo → eleição 0. Build
   `std::map<TEleicaoID, CCedula>` (a municipal election gives one ballot, a general election two).
3. For each ballot: `CRdvVota::GetInst().RecebeCedula(eleição, cédula)` →
   `CVotosEleicoesVota::RecebeCedula` (:215): look up the eleição (func 3708, error 4735 "Eleicao (id) nao
   encontrada"), then
   * **`CVotosCargos::ConfereCedula`** (:248–:295):
     - the ballot has exactly Σ qtdEscolhas votes (func 5642) — 4672 "Tamanho da cedula (n) difere do esperado (m)";
     - every cargo of the ballot exists — 4673; each cargo has exactly `qtdEscolhas` votes — 4674;
     - a NOMINAL vote has exactly `numeroDigitos` digits — 4675;
     - a NULO_POR_REPETICAO vote is only allowed on a multi-seat cargo (4676) and the ballot must contain the
       nominal vote it repeats (same cargo, type 2, same digits) — 4677;
   * **`CVotosCargos::InsereCedula`** (:199): copy the whole cargo map (a cargo missing from the copy throws 4669
     "Cargo nao encontrado n", :199), insert each vote at the index returned by
     the positioner (`CRdvPosicionadorVota`, func 11496 — binary search on (tipo, digitado), so the RDV stays
     **sorted** and the voting order is lost), `CVotos::Insere` (cvotos.cpp:40, 4662 if the index is out of
     range), then swap the maps (all-or-nothing per eleição).
4. `CEstadoGeralVota::MarcaUltimoVoto` (cestadogeralvota.cpp:131/136): requires acquisition started (+32, else
   8086) and not ended (+48, else 8087); stores `CDateTime::Now()` as time of the last vote (+56) and increments
   the voter counter (+52, uint16).
5. `m_proximoEstado = CSincronismoEleitor` (persists `rdv.dat`/`vota.bin` on the urna; a no-op in the web build,
   so the ballot stays in memory only), finish and drop the per-cargo sub-state.

`md::CVoto` (func 2256, :80) enforces which vote types carry digits — required for legenda (1), nominal (2)
and nulo por repetição (7); forbidden for branco (3), branco/nulo após suspensão (5, 6) and the two
cargo-sem-candidato nulls (8, 9); free for nulo (4); anything else "CVoto - tipo de voto invalido n" (4661).
`CVotosCargos(TMapa)` (func 5643) checks on load that every cargo of an eleição holds the same number of
ballots (4666 empty map, 4667 not a multiple of qtdEscolhas, 4668 different from the first cargo), and
`CVotosEleicoesVota(TMapa)` (func 5640, with `MontaMapa` inlined) that no cargo belongs to two eleições (4724)
and the map is not empty (4723). `Comparecimento(eleição)` (func 5639, :118) = votes of the first cargo /
its qtdEscolhas.

---

## 5. Piece C — election model (`dados/md/processoeleitoral/`)

| function | behaviour | errors (EUeComumDadosError) |
|---|---|---|
| `CCargo::Valida` (5657) | abrangência < 3; 1 ≤ dígitos ≤ 5; escolhas ≥ 1; 1 ≤ página de impressão ≤ 5; consulta needs `DetalheConsulta`, majoritário/proporcional need `DetalheCandidato`, other tipos invalid | 8138..8144 (:212..:240) |
| `CCargo::GetDetalheCandidato` (1388) / `GetDetalheConsulta` (1923) | optional accessors | 8134 "Não é cargo de candidato." / 8135 "Não é cargo de consulta." |
| `GetNome` (1547), `GetNomeMasculino` (3716), `GetNomeAbreviado` (2797), `GetNome(sexo)` (2796), `GetNomeSuplente(n, sexo)` (3715) | name forms of `NomesCargo` (neutro/masculino/feminino/abreviado), or the consulta's name | via 8135 / 8145 |
| `CCargo::GetOrdinalEscolha` (2258) | "" when 1 seat, else `"{}ª vaga"` (Latin-1 `ª`) | 8137 "Cargo {} com escolha invalida {}" |
| `CDetalheCandidato::GetSuplente(n)` (1545) | 1-based | 8145 "Suplente inexistente" |
| `CEleicaoPE::ValidaOrdem` (3714) / `GetCargos` (2257) | unique order values / non-empty cargo list | 8153 / 8154 |
| `CPleito::CPleito` (5652) | eleição ids unique; #versions ≥ #eleições; #situações == #eleições (counts only) | 8159 / 8160 / 8161 |
| `CPleito::GetEleicao` (5651) / `GetSituacoesEleicoes` (5648) | linear search | 8162 / 8164 |
| `CProcessoEleitoral::GetPleito2` (1267) | optional 2nd round | 8165 "Não há pleito 2." |
| `CConversorNomesCargo::DoDesconverte` (11369) | four names must be non-empty | 8155..8158 |
| `CConversorPartido::DoDesconverte` (11459) | número < 100, sigla < 25 chars, nome < 56 chars (empty allowed) | 8050..8052 |
| `CDataText<DS_NomeCargoNeutroComEscolha>` (12587) | top line of every voting screen: `GetNome()` + `" - " + GetOrdinalEscolha(g_numeroEscolha)` for multi-seat cargos | — |

---

## 6. Piece D — paths, the SAVD file table, the start-up signature check

### 6.1 `CPath`

`ms_raizesFlash = {"/dsk/fi/", "/dsk/fe/"}` (@1838576) and `ms_raiz = "/"` (@1838600) are set by
`__wasm_call_ctors`. `GetPathTrab(origem, turno)` = `<flash>dinamico/trab<1|2>/` (func 358, 7227 "Turno
invalido: {}"), `GetPathResult` = `<flash>dinamico/res<1|2>/` (func 1399, 7226), `GetPathRootSemSA(origem)` =
`ms_raiz / "/dsk/fi/".relative_path()` (func 634, 7224 "Mídia inválida: {}"). The other members
(`GetPathDinamico` 1082, `GetPathEstatico` 762, `GetPathLog` 5899, `GetPathMR` 949, `GetPathRoot` 6048) are
listed in other units but reconstructed in the same `cpath.cpp`.

### 6.2 `CArquivosSavd` (func 1164)

`CArquivosSavd::GetInst()` builds, once, three `std::map`s (the constructor is fully inlined: 77,002 bytes).
Its contents were **dumped from the running simulator** (a hook added to a copy of `tools/run/headless.mjs`
walks the three red-black trees of the object @1838544 after `votaInit`) and match the code:

* `m_pacotes` (84 entries, ids 120–203, read by `operator[](ESavdPacote)` func 275): signature packages, e.g.
  120/121 `…/dinamico/eg.vsu` (fi/fe), 122–125 `trab{1,2}/vota.vsu` (fi t1, fi t2, fe t1, fe t2), 126–129 `bu.vsu`,
  130–133 `buj.vsu`, 134–137 `rdv.vsu`, 138–141 `ze.vsu`, 142–145 `rze.vsu`, 146–149 `gap.vsu`, 150/151 `sa.vsu`
  (fi only), 152–155 `red.vsu`, 156 `/dsk/fe/dinamico/trab/tabcorr.vsc`, 157 `/dsk/fe/estatico/{:05}{:04}{:04}-lo.vsc`,
  158–161 `…/dinamico/res{1,2}/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc` (the package that signs the result files),
  162–165 `{:c}{:05}{}{:05}{:04}{:04}-sa.vsc` (**no directory**), 166–169 `…-red.vsc`, 170–189 SA packages under
  `dinamico/sa/{}/trab{1,2}/`, 190 `/dsk/mr/sieco-dados/`, 191–194 `bim.vsu`, 195–198 `behb.vsu`, 199–202
  `uenux.vsu`, 203 `/dsk/fe/estatico/dadoscarga.vsu`.
* `m_arquivos` (84 entries, ids 25–115, read by `operator[](ESavdArquivoUE)` func 680): the signed files, as names
  or `std::format` patterns — 25/26 `eg.bin`, 27–30 `gap.bin`, 31/32 `vota.bin`, 33 `sa.bin`, 34 `tabcorr.dat`,
  35 `…-bu.dat`, 36 `…-busa.dat`, 37 `…-rdv.dat`, 39/41/42 `…-jufa.dat`, 43 `…-imgbu.dat`, 44 `…-imgze.dat`,
  45 `…-imgbusa.dat`, 46/47 `…-hash.dat`, 48–59 `…-wsq{bio,man,mes}.jez`, 60 `…-log.jez`, 61 `…-logsa.jez`,
  63–82 the same result names for other applications, 83/98 `rdv.dat`, 84/85 `buj.dat`, 86/87/100 `bu.dat`,
  88/99 `ze.dat`, 89 `rze.dat`, 90–97 `bur/bujr/bimr/behbr.dat`, 101 `saraiz.bin`, 102 `sasecao.bin`, 103 `busa.dig`,
  104/105 the `uesieco-…-cert.dat` / `-aut.dat` regular expressions, 106/107 `bim.dat`, 108/109 `behb.dat`,
  110/111 `uenux.db`, 112 `{:05}{:04}{:04}-lo.dat`, 113 `dadoscarga.dat`, 115 `red.bin`.
  (`…` = `{:c}{:05}{}{:05}{:04}{:04}` = fase, pleito, UF, município, zona, seção.)
* `m_aplicacoes` (20 entries, ids 1–22): `vota.of`, `vota.si`, `vota.te`, `vota.tm`, `sa.of`, `sa.si`, `sa.tr`,
  `gap`, `red`, `vpp`, `adh`, `atue`, `ste`, `turno2.jez`, `savd`, `partido`, `partido.app`, `partido.pub`,
  `partido.id`, `infomidia.pwd`. **No function of this build reads this map.**

The full, ordered table is in `src/uenux2/src/app/comum/carquivossavd.u22.cpp`.

### 6.3 Start-up signature check (func 4625)

`CPacoteArquivos::ValidaAssinaturasArquivosTrabalho(dir)` *(name inferred)* is called twice by the voter start-up
routine (func 7787) with `GetPathTrab(INTERNA)` and `GetPathTrab(EXTERNA)`. For each pair
`{bim.dat:bim.vsu, bu.dat:bu.vsu, buj.dat:buj.vsu, rdv.dat:rdv.vsu, rze.dat:rze.vsu, vota.bin:vota.vsu, ze.dat:ze.vsu}`
whose data file exists (`fs::status` type neither none nor not_found), it builds
`CPacoteArquivos(dir/<name>.vsu, "<data file>", ESavdChaveValidar::UE ('5'), aplicação 1)`
(constructor: `ValidarChaveEAplicacaoValida` — key '1'..';' (7219), application 1..9 (7220); type
0x1001 "files listed") and calls `Validar(CInterfaceMensagemVazia)`: for every listed file
`IInterfaceSavd::ValidaAssinaturaArquivo(aplicação, chave, pacote, arquivo)` (func 5892); on failure
`TrataErroPacoteArquivo` (7223, or `TrataErroPacote` 7222 for SAVD errors 1–3). Key names
(`RetornaNomeChave`, func 5900): '1' TSE, '2' SECAD, '3' SECINP, '4' SEVIN, '5' UE, '6' SCUE, '7' CLOGI,
'8' CLOGI_CERT, '9' CLOGI_UPDATE, ':' PU, ';' SEINT (7221 otherwise).

In the simulator this ran (observed) and always succeeds: the SAVD is `(anonymous)::CWasmSavd`, whose
`RecebeMensagem` (func 10949) ignores the request and returns a fixed 12-byte answer, and the `.vsu` files hold
"assinatura simulada para vota_web_wasm".

---

## 7. Piece E — `CInfoMTLCD`

A 64-byte singleton (@1838572) that observes `BatteryIconDataSource<Vertical>` (1 s timer). `Update` (11653,
observed) centres the current battery icon on the MT's graphic LCD (`IScreenMT` slot 4 with flag = false; the
icon size comes from the voter screen's renderer, `IScreen` slot 34 → slot 13). `MostrarImagemNoLCD` (3836)
draws an arbitrary image (the voter's photo, `CBiometriaEleitor::GetFoto`); its `bool` parameter was
constant-propagated (callers pass `true`). The flag also gates the rest of the function: with `true` it first
detaches the battery observer and returns if the image is empty; the copy inlined in `Update` (flag `false`)
contains only the `IScreenMT` slot 4 call (otherwise every battery tick would detach the observer). The destructor (5902) detaches itself;
funcs 3837/5510/6049/10990/11652 are the two `BatteryIconDataSource` destructor instantiations (one merged body,
6049, taking the vtable as a parameter).

---

## 8. BOLETIM DE URNA: what this unit contributes

This unit does not generate, sign or print the BU (that is u08/u09: `CGeraBU`, `CGravaResultado`,
`CImprimindoBU`). It provides five of the BU's inputs:

1. **The votes counted by the BU.** Every confirmed ballot enters the in-memory RDV through `GravaVotos` (§4):
   one `CCedula` per eleição, validated (`ConfereCedula`: size = Σ qtdEscolhas, one vote per seat, nominal votes
   with the cargo's digit count, repeated-candidate nulls only on multi-seat cargos and only next to the nominal
   vote they repeat) and inserted sorted by (tipo, digitado). The BU's per-cargo totals (`CVotosEleicoesVota`
   `Nominais/Legendas/Nulos/Brancos/Candidato/Partido`, u05) and the section **comparecimento**
   (`Comparecimento(eleição)` = ballots of the eleição, func 5639) are computed from this structure. The
   general-state counter incremented by `MarcaUltimoVoto` (+52) and the time of the last vote (+56) come from the
   same function.
2. **The file names and signature packages** of the result files, from `CArquivosSavd` (§6.2). For the BU:
   `ESavdArquivoUE` 35 = `"{:c}{:05}{}{:05}{:04}{:04}-bu.dat"` (formatted with fase char, pleito, UF, município
   (5), zona (4), seção (4), e.g. `t02410ac0000100010001-bu.dat`), 43 `…-imgbu.dat` (envelope of the printed BU
   image), 84/85 `buj.dat`, 86/87/100 `bu.dat` (the work copies), 90/94 `bur.dat`; packages 126–129
   `dinamico/trab{1,2}/bu.vsu` (MI/MV) sign the work copy, and 158/159 (MI) and 160/161 (MV)
   `dinamico/res{1,2}/<prefix>-vota.vsc` is the package `CGravaResultado` passes to `CAssinadorVota` (158 in the
   1st turno, 159 in the 2nd; u09 §5.2) to sign all result files.
3. **Where they go:** `CPath::GetPathResult(flash, turno)` = `/dsk/fi|fe/dinamico/res1|2/` (func 1399) and
   `GetPathTrab` = `/dsk/fi|fe/dinamico/trab1|2/` (func 358).
4. **Integrity at restart:** before anything else, `votaInit`/start-up verifies `bu.dat` (and `vota.bin`,
   `rdv.dat`, `ze.dat`, `rze.dat`, `buj.dat`, `bim.dat`) of both work areas against their `.vsu` packages with the
   urna key UE (§6.3); a failure aborts the start-up with CUeComumError 7222/7223. In the simulator the SAVD is
   mocked and the check cannot fail.
5. **The encerramento flow and the BIM.** When the final mesário registration ends
   (`CEncerraRegistroMesarios`, period FINAL), the operator thread posts **message 7** to the voter thread
   (IControlador slot 15), which starts the BU chain (`CAguardaMensagem::vf6(7)` → `CGeraBU` → `CGeraRelatorios`
   → `CImprimindoBU` → `CGravaResultado`, u09). `CGeraRelatorios` prints the BIM with the mesários registered in
   `comparecimento_mesario`, each name obtained by `CImprimirIdentificacaoMesariosFinal::GetNomeMesario(título)`
   (func 5385: roll lookup by título, UEASSERT 3409 if absent, nome social if present else nome, cut at 40
   characters). The cargo names printed on the BU come from `CCargo::GetNome*` (§5).

---

## 9. Web-build specifics

* Piece A is dead code in the simulator (operator thread not run, no `IControladorRegistraMesarios`, no finger
  hardware). Template extraction and WSQ encoding are empty stubs, so biometric comparison compares empty data.
* `GravaVotos` runs for every voter; the ballot goes into the in-memory RDV but `CSincronismoVotoEleitorWeb`
  (u07) never writes `rdv.dat`, `vota.bin` or their signatures.
* `CArquivosSavd` is built during `votaInit` (observed) and used by the start-up persistence code; the SAVD
  behind `CPacoteArquivos` is `CWasmSavd` (canned answers), and every `.vsu` is the text "assinatura simulada
  para vota_web_wasm".
* `CInfoMTLCD::Update` runs on the battery timer (observed) and computes the centred position, but draws
  nothing: `simulador::CWasmScreenMT` slot 4 (the image call) is an ICF no-op (func 1870). `IPower` never
  reports a change anyway, so the icon would always be the same.
* Nothing in this unit calls a `js_*` import, reads URL parameters or touches the network.

## 10. wasm / Emscripten observations

* **Naming by first srcloc.** The tools name a function after the first `std::source_location` found in it, so
  many entries were misnamed: 4454 ("CVotosCargos::ConfereCedula" → `CEleitorVotando::GravaVotos`), 4625
  ("ValidarChaveEAplicacaoValida" → the directory check that inlines it), 10313 ("(anonymous)::GetControlador" →
  `IPedeTituloMesario::ProcessInput`), 10337 (→ `IGestorDadoMesario::StartState`), 5382, 5386, 1547/2796/2797/3715/3716
  (all "GetDetalheConsulta/Candidato" → the name getters that inline those accessors), 5640 ("MontaMapa" → the
  `CVotosEleicoesVota` constructor). Overrides are returned for all of them.
* **Whole constructors inlined into lazy `GetInst`.** Most state constructors exist only inside the accessor
  that creates the singleton, and several accessors are themselves inlined into the state that transitions to
  them (10313 contains four constructors; 10388 three). Only the `mutex_unlock` stubs of the `lock_guard`s remain
  (single-threaded build).
* **wasm-opt merged bodies with the varying constant as a parameter:** 6011 (`ProcessInput` of
  CRegistrarMesarios / CMesarioRegistrado, srclocs as parameters), 2895 (`ProcessInput` returning to
  CPedeTituloMesario on a given key: 5 or 9), 6010, 6008, 6009, 6012, 6029 (optional-or-throw with offset,
  message and code), 6049 (destructor with the vtable), 3894 (DAO `Clone` with the vtable), 170/3611 (error
  factories).
* **Dead-argument elimination / constant propagation:** `ComparaDigitais` lost both vector parameters, and
  `MostrarImagemNoLCD` its `bool` (always `true` from the out-of-line callers; the inlined copy in `Update` passes
  `false`, which also removes the observer `Detach` and the empty-image test that the flag guards).
* **libc++ 21 `__datasizeof`:** copying `std::vector<CSituacoesEleicoes>` (8-byte elements with 7 bytes of data)
  memmoves `n*8 - 1` bytes (func 5652).
* `CArquivosSavd`'s constructor is the second-largest application function of the module (77 KB, 35,493
  instructions) although it only fills three maps: each of the ~190 insertions expands a `filesystem::path`
  concatenation, a map `find`/insert and three string destructors.
* Strings are **Latin-1** in the data segment ("{}ª vaga" is 8 bytes, "REGISTRO DE MESÁRIOS" 20).

## 11. Suspicious / notable code

1. **`emscripten_sleep(1000)` in `CPedeDigitalMesario::ProcessTick` (func 10316).** After a finger is detected
   the code sleeps 1 s when the byte @1584624 is 1 (it is, and nothing writes it). The build has no Asyncify, so
   the glue aborts. Unreachable in the public simulator (operator thread not run; the fingerprint interfaces
   would throw first). Same pattern as u10's `ObtemEstadoPosReconhecimentoBiometrico`.
2. **`id_arquivo` not reset between mesários (funcs 10316/5382/5371/10337).** `CControladorReconhecimentoMesario`
   (@1909960) is a process-wide singleton. On the biometric path `m_estado` (2 or 3) and `m_dedo` (the matched
   finger, or 0: `SalvaDigitalMesario` always clears it, even when storing fails) are rewritten for every mesário,
   but `m_idArquivo` is only ever *set* (func 5371, when an image is written) and no code in this build clears it.
   So a mesário recognised by fingerprint (no image stored), or one whose image could not be stored
   (`Armazena` → 999999), after a mesário whose image was stored is saved with the previous mesário's
   `id_arquivo`. (Real-urna data quality if the same code ships; no effect in the simulator.)
3. **Ballot recording is not atomic across eleições (func 4454).** `InsereCedula` swaps in a copied map, so one
   eleição is all-or-nothing, but in a general election the federal ballot is inserted before the estadual one is
   checked; if the second `RecebeCedula` throws, the first stays in the RDV and `MarcaUltimoVoto` is skipped. It
   needs an inconsistent ballot (a programming error), and the voter context then reports "O voto do eleitor NÃO
   foi registrado".
4. **Dispatcher states can freeze the MT.** `CPedeTituloMesario::StartState` (10388) and
   `CGestorDadoMesario::StartState` (10327) leave `m_proximoEstado = this` when `GetPeriodoRegistro()` returns 0
   (their `br_table` only covers 1..3); these states have flags 0 (no keys, ticks or messages) and draw nothing,
   so the operator thread would wait forever. `CEncerraRegistroMesarios::StartState` (10380) is *not* affected by 0
   (it treats 0 like 1 and returns to VOTA slot 9); it would only stay for a value > 3, which the VOTA table
   cannot return. VOTA only enters the flow in estados '6'..'9', which `AtualizaEstadoRegistro` maps to periods
   1..3, so this is not reachable in practice.
5. **SAVD table oddities (func 1164).** Packages 162–165 (`…-sa.vsc`) have no directory while every sibling has
   one, and 172/173 and 178/179 duplicate 170/171 and 176/177 (internal flash twice where the pattern suggests
   one of them should be the external flash). Only the SA application would use them.
6. **Security simulated in the web build.** The start-up signature verification of the work files (func 4625,
   observed) cannot fail: `CWasmSavd::RecebeMensagem` returns a fixed answer and the `.vsu` files are placeholder
   text. Fingerprint comparison compares empty templates (extractor stub). Nothing here should be read as
   evidence about the real urna's checks.
7. **Minor validation gaps.** `CConversorPartido` accepts an empty sigla/nome (only upper bounds are checked);
   `CPleito` compares only the *counts* of package versions and situations with the number of eleições (an entry
   for a wrong eleição id is not detected); `CPath::GetPathTrab/GetPathResult` index the 2-element root array with
   an unchecked `EFlashOrigem` (only constants are passed).
8. **Audit-log side effect in constructors.** "Registrando mesários antes/durante/após a votação" is logged by the
   constructors of the three `CPedeTituloMesario*` singletons (10388), i.e. only the first time each kind of
   registration happens in a process run.
9. **Wasted work.** `CVotosCargos::QtdVotosPorCedula` (5642) iterates the cargo map by value, so each call
   copies (and frees) every cargo's complete vote vector, i.e. all votes recorded so far in the section. It is
   called once per ballot on the normal path (twice when 4672 is thrown), so the cost of recording a vote
   grows linearly with the number of voters (quadratic over the day; negligible at a few hundred voters but
   pointless); `SalvaDigitalMesario` (5382) builds a descriptive
   `<título>_Operador` name and never uses it (images get random `me<nnnnnn>` names, as in u10 §6.3).

## 12. Open questions

* Real names of the `IControladorRegistraMesarios` slots (no RTTI/srcloc; names inferred from the VOTA
  implementation) and of slot 8 (always `true`, never called by comum code).
* Real name of func 4625 (the outer loop over the seven work files): not in the binary; a static member of
  `CPacoteArquivos` or a free function of the start-up file are both possible.
* The meaning of `m_aplicacoes` (map +24 of `CArquivosSavd`, ids 1–22): no accessor exists in this build.
* The exact semantics of `EstadoReconhecimentoBiometrico` 1/2/3 versus ASN.1 `EstadoColetaDigital` (1..4) — the
  conversion to the ASN.1 result file (`CConversorComparecimentoMesario`, ecourna 9103/9108) belongs to u14.
* Why `CLimiteMesariosRegistradosAtingido`'s constructor looks up the controller (:53) but prints a constant 6
  (devirtualised getter, or a leftover call).

## 13. Complete mapping table (107 functions)

"✓" = observed executing in the recorded votes. "tools name" is the name in `analysis/functions.tsv`.

| idx | size | ran | tools name | reconstructed symbol | reconstruction (src/…) | original file | conf. |
|---|---|---|---|---|---|---|---|
| 170 | 21 |  | `comum_f170` | `comum::CDadosError factory thunk (CBaseError<EUeComumDadosError,{7800,8600}> ctor via func 2294)` | library/inlined helper (merged error-constructor thunk, vtable @1528096 passed to 2294) | — | high |
| 296 | 123 | ✓ | `ecourna_f296` | `std::to_string(int)` | library helper (libc++ std::to_string(int): __to_chars_itoa + string(first,last)) | — | high |
| 358 | 901 | ✓ | `comum::CPath::GetPathTrab` | `comum::CPath::GetPathTrab` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | high |
| 567 | 50 |  | `ecourna_f567` | `std::operator+(const char*, std::string&&)` | library helper (libc++ operator+(const char*, string&&) = rhs.insert(0, lhs)) | — | high |
| 634 | 1142 | ✓ | `comum::CPath::GetPathRootSemSA` | `comum::CPath::GetPathRootSemSA` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | high |
| 941 | 385 |  | `vota_f941` | `api::CFormBuilderMT::Add<api::CBuzzFieldMT>` | library/inlined helper (template instantiation of the MT form builder: push_back(shared_ptr<IFormField>(new CBuzzFieldMT(a,b))) | uenux2/src/api/gui/cformbuildermt.h | medium |
| 1164 | 77002 | ✓ | `comum_f1164` | `comum::CArquivosSavd::GetInst` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | high |
| 1255 | 612 |  | `comum_f1255` | `comum::CriaFormEleitorRegistroMesarios` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | medium |
| 1267 | 75 |  | `comum::md::CProcessoEleitoral::GetPleito2` | `comum::md::CProcessoEleitoral::GetPleito2` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cprocessoeleitoral.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cprocessoeleitoral.cpp | high |
| 1348 | 249 |  | `comum_f1348` | `std::pair<std::string,std::string>::pair(const char*, const char*)` | library helper (pair of two string(const char*) ctors), emitted with cpacotearquivos.cpp | — | high |
| 1378 | 116 |  | `comum::IPedeTituloMesario::vf0` | `comum::IPedeTituloMesario::~IPedeTituloMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | high |
| 1388 | 73 |  | `comum::md::CCargo::GetDetalheCandidato@1388` | `comum::md::CCargo::GetDetalheCandidato` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | high |
| 1399 | 745 |  | `comum::CPath::GetPathResult` | `comum::CPath::GetPathResult` | src/uenux2/src/app/comum/cpath.cpp | uenux2/src/app/comum/cpath.cpp | high |
| 1545 | 99 |  | `comum::md::CDetalheCandidato::GetSuplente` | `comum::md::CDetalheCandidato::GetSuplente` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cdetalhecandidato.cpp | high |
| 1547 | 184 | ✓ | `comum::md::CCargo::GetDetalheConsulta@1547` | `comum::md::CCargo::GetNome` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | medium |
| 1651 | 160 | ✓ | `comum_f1651` | `std::filesystem::path::__relative_path` | library helper (libc++ path::__relative_path: PathParser + ConsumeRootDir -> string_view) | — | high |
| 1923 | 22 |  | `comum::md::CCargo::GetDetalheConsulta@1923` | `comum::md::CCargo::GetDetalheConsulta` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | high |
| 2256 | 1398 | ✓ | `comum::md::CVoto::CVoto` | `comum::md::CVoto::CVoto` | src/uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | uenux2/src/app/comum/dados/md/rdv/cvoto.cpp | high |
| 2257 | 78 | ✓ | `comum::md::CEleicaoPE::GetCargos` | `comum::md::CEleicaoPE::GetCargos` | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | high |
| 2258 | 906 | ✓ | `comum::md::CCargo::GetOrdinalEscolha` | `comum::md::CCargo::GetOrdinalEscolha` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | high |
| 2730 | 68 |  | `comum_f2730` | `comum::md::CComparecimentoMesario::CComparecimentoMesario (with idArquivo)` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | medium |
| 2731 | 68 |  | `comum_f2731` | `comum::md::CComparecimentoMesario::CComparecimentoMesario (idArquivo NULL)` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | medium |
| 2738 | 76 |  | `comum_f2738` | `comum::md::CComparecimentoMesarioPK::CComparecimentoMesarioPK` | src/uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.h | uenux2/src/app/comum/comparecimentomesario/md/ccomparecimentomesario.cpp | medium |
| 2796 | 221 |  | `comum::md::CCargo::GetDetalheConsulta@2796` | `comum::md::CCargo::GetNome(CSexo::ESexo)` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | medium |
| 2797 | 184 |  | `comum::md::CCargo::GetDetalheConsulta@2797` | `comum::md::CCargo::GetNomeAbreviado` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | medium |
| 3608 | 61 |  | `comum_f3608` | `comum::IPedeTituloMesario::IPedeTituloMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | high |
| 3611 | 18 |  | `comum_f3611` | `comum::CComparecimentoMesarioError factory thunk (CBaseError<EUeComumComparecimentoMesarioError,{7750,7800}>)` | library/inlined helper (merged error-constructor thunk: ecourna_f710 + vtable @1593524) | — | high |
| 3708 | 173 | ✓ | `comum_f3708` | `comum::md::(anonymous namespace)::BuscaEleicao` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | medium |
| 3709 | 32 |  | `comum_f3709` | `std::__tree<std::__value_type<TCargoID,int>>::destroy` | library helper (std::map node destructor for ConfereCedula's vote counter) | — | high |
| 3714 | 680 |  | `comum::md::CEleicaoPE::ValidaOrdem` | `comum::md::CEleicaoPE::ValidaOrdem` | src/uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp | high |
| 3715 | 574 |  | `comum::md::CCargo::GetDetalheCandidato@3715` | `comum::md::CCargo::GetNomeSuplente` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | medium |
| 3716 | 184 |  | `comum::md::CCargo::GetDetalheConsulta@3716` | `comum::md::CCargo::GetNomeMasculino` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | medium |
| 3750 | 135 |  | `comum_f3750` | `std::__tree<std::__value_type<TEleicaoID,CCedula>>::destroy` | library helper (std::map<TEleicaoID, CCedula> node destructor used by GravaVotos) | — | high |
| 3836 | 292 |  | `comum::CInfoMTLCD::MostrarImagemNoLCD` | `comum::CInfoMTLCD::MostrarImagemNoLCD` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | high |
| 3837 | 12 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)1>::vf0` | `api::BatteryIconDataSource<Vertical>::~BatteryIconDataSource` | library/inlined helper (template dtor, see note in src/uenux2/src/app/comum/cinfomtlcd.cpp) | uenux2/src/api/gui/cpowerinformation.cpp (template defined there, srcloc :119/:131) | high |
| 3839 | 57 |  | `comum_f3839` | `std::__tree<ESavdPacote,string>::destroy (CArquivosSavd::m_pacotes)` | library helper (map node destructor, emitted with carquivossavd.cpp) | — | high |
| 3840 | 57 |  | `comum_f3840` | `std::__tree<ESavdArquivoUE,string>::destroy (CArquivosSavd::m_arquivos)` | library helper (map node destructor, emitted with carquivossavd.cpp) | — | high |
| 3841 | 57 |  | `comum_f3841` | `std::__tree<ESavdAplicacao,string>::destroy (CArquivosSavd::m_aplicacoes)` | library helper (map node destructor, emitted with carquivossavd.cpp) | — | high |
| 3881 | 32 |  | `comum_f3881` | `std::unique_ptr<comum::IPedeTituloMesario>::reset` | library helper (unique_ptr reset with the devirtualised destructor 1378; used by the atexit stubs 10366/10372/10379) | — | high |
| 4454 | 5617 | ✓ | `comum::md::CVotosCargos::ConfereCedula` | `vota::CEleitorVotando::GravaVotos` | src/uenux2/src/app/vota/eleitor/celeitorvotando.u22.cpp | uenux2/src/app/vota/eleitor/celeitorvotando.cpp | medium |
| 4625 | 3584 | ✓ | `comum::CPacoteArquivos::ValidarChaveEAplicacaoValida` | `comum::CPacoteArquivos::ValidaAssinaturasArquivosTrabalho` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | medium |
| 5372 | 720 |  | `comum::CControladorReconhecimentoMesario::ComparaDigitais` | `comum::CControladorReconhecimentoMesario::ComparaDigitais` | src/uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.cpp | high |
| 5382 | 994 |  | `comum::CPedeDigitalMesario::GetControlador` | `comum::CPedeDigitalMesario::SalvaDigitalMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | low |
| 5385 | 371 |  | `comum::CImprimirIdentificacaoMesariosFinal::GetNomeMesario` | `comum::CImprimirIdentificacaoMesariosFinal::GetNomeMesario` | src/uenux2/src/app/comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.cpp | high |
| 5386 | 1465 |  | `comum::CLimiteMesariosRegistradosAtingido::GetControlador@5386` | `comum::CLimiteMesariosRegistradosAtingido::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | high |
| 5389 | 983 |  | `comum_f5389` | `comum::CConfirmaFimRegistroMesarios::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | high |
| 5390 | 472 |  | `comum::dao::CComparecimentoMesarioDAO::ConverteDedoHabilitacao` | `comum::dao::CComparecimentoMesarioDAO::ConverteDedoHabilitacao` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | high |
| 5391 | 475 |  | `comum::dao::CComparecimentoMesarioDAO::ConverteEstadoBiometria` | `comum::dao::CComparecimentoMesarioDAO::ConverteEstadoBiometria` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | high |
| 5510 | 12 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)0>::vf0` | `api::BatteryIconDataSource<Horizontal>::~BatteryIconDataSource` | library/inlined helper (template dtor, see note in src/uenux2/src/app/comum/cinfomtlcd.cpp) | uenux2/src/api/gui/cpowerinformation.cpp (template defined there, srcloc :119/:131) | high |
| 5639 | 45 |  | `comum::md::CVotosEleicoesVota::Comparecimento` | `comum::md::CVotosEleicoesVota::Comparecimento` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | high |
| 5640 | 1152 | ✓ | `comum::md::(anonymous namespace)::MontaMapa` | `comum::md::CVotosEleicoesVota::CVotosEleicoesVota` | src/uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoseleicoesvota.cpp | medium |
| 5642 | 291 |  | `comum_f5642` | `comum::md::CVotosCargos::QtdVotosPorCedula` | src/uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | low |
| 5643 | 665 |  | `comum::md::CVotosCargos::CVotosCargos` | `comum::md::CVotosCargos::CVotosCargos` | src/uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | uenux2/src/app/comum/dados/md/rdv/cvotoscargos.cpp | high |
| 5648 | 517 |  | `comum::md::CPleito::GetSituacoesEleicoes` | `comum::md::CPleito::GetSituacoesEleicoes` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | high |
| 5651 | 517 |  | `comum::md::CPleito::GetEleicao` | `comum::md::CPleito::GetEleicao` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | high |
| 5652 | 844 | ✓ | `comum::md::CPleito::CPleito` | `comum::md::CPleito::CPleito` | src/uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp | high |
| 5657 | 403 |  | `comum::md::CCargo::Valida` | `comum::md::CCargo::Valida` | src/uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | uenux2/src/app/comum/dados/md/processoeleitoral/ccargo.cpp | high |
| 5900 | 948 |  | `comum::CPacoteArquivos::RetornaNomeChave` | `comum::CPacoteArquivos::RetornaNomeChave` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | high |
| 5901 | 594 |  | `comum::CPacoteArquivos::TrataErroPacote` | `comum::CPacoteArquivos::TrataErroPacote` | src/uenux2/src/app/comum/cpacotearquivos.cpp | uenux2/src/app/comum/cpacotearquivos.cpp | high |
| 5902 | 188 |  | `comum::CInfoMTLCD::vf0` | `comum::CInfoMTLCD::~CInfoMTLCD` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | high |
| 5907 | 38 |  | `comum_f5907` | `comum::CArquivosSavd::~CArquivosSavd` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | high |
| 6029 | 68 |  | `comum_f6029` | `merged body: optional<T> engaged check or throw CDadosError (CCargo::GetDetalheConsulta / CLocal::GetContingencia)` | library/inlined helper (wasm-opt merge-similar-functions body: (this, offset, srcloc, msg, code)) | — | high |
| 6049 | 159 |  | `api_f6049` | `api::BatteryIconDataSource<O>::~BatteryIconDataSource (shared body)` | library/inlined helper (merged destructor body of both BatteryIconDataSource instantiations, vtable as parameter) | uenux2/src/api/gui/cpowerinformation.cpp (template defined there, srcloc :119/:131) | high |
| 10313 | 6002 |  | `comum::(anonymous namespace)::GetControlador` | `comum::IPedeTituloMesario::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | high |
| 10314 | 47 |  | `comum::IPedeTituloMesario::vf2` | `comum::IPedeTituloMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp | high |
| 10315 | 17 |  | `comum::CPedeDigitalMesario::FinishState` | `comum::CPedeDigitalMesario::FinishState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | high |
| 10316 | 3808 |  | `comum::CPedeDigitalMesario::ProcessTick` | `comum::CPedeDigitalMesario::ProcessTick` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | high |
| 10317 | 130 |  | `comum::CPedeDigitalMesario::vf7` | `comum::CPedeDigitalMesario::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | high |
| 10318 | 257 |  | `comum::CPedeDigitalMesario::StartState` | `comum::CPedeDigitalMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | high |
| 10319 | 14 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text@10319` | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpededigitalmesario.cpp | high |
| 10322 | 14 |  | `comum::CDigitalMesarioNaoReconhecida::vf7` | `comum::CDigitalMesarioNaoReconhecida::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | high |
| 10323 | 14 |  | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text@10323` | `comum::(anonymous namespace)::CNomeMesariosUrnaDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cdigitalmesarionaoreconhecida.cpp | high |
| 10327 | 403 |  | `comum::CGestorDadoMesario::StartState` | `comum::CGestorDadoMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp | high |
| 10337 | 1733 |  | `comum::IGestorDadoMesario::GetControlador` | `comum::IGestorDadoMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/igestordadomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/igestordadomesario.cpp | high |
| 10338 | 17 |  | `comum::CMesarioRegistrado::GetControlador@10338` | `comum::CMesarioRegistrado::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | high |
| 10339 | 254 |  | `comum::CMesarioRegistrado::GetControlador@10339` | `comum::CMesarioRegistrado::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cmesarioregistrado.cpp | high |
| 10342 | 14 |  | `comum::CTituloMesarioJaRegistrado::vf7` | `comum::CTituloMesarioJaRegistrado::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | high |
| 10343 | 149 |  | `comum::CTituloMesarioJaRegistrado::StartState` | `comum::CTituloMesarioJaRegistrado::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | high |
| 10344 | 14 |  | `comum::(anonymous namespace)::CTituloMesarioJaRegistradoDS::Text` | `comum::(anonymous namespace)::CTituloMesarioJaRegistradoDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariojaregistrado.cpp | high |
| 10347 | 14 |  | `comum::CTituloMesarioInvalido::vf7` | `comum::CTituloMesarioInvalido::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | high |
| 10348 | 12 |  | `comum::CTituloMesarioInvalido::GetControlador` | `comum::CTituloMesarioInvalido::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | high |
| 10349 | 14 |  | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text` | `comum::(anonymous namespace)::CTituloMesarioInvalidoDS::Text` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesarioinvalido.cpp | high |
| 10352 | 14 |  | `comum::CTituloMesarioVazio::vf7` | `comum::CTituloMesarioVazio::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | high |
| 10353 | 12 |  | `comum::CTituloMesarioVazio::GetControlador` | `comum::CTituloMesarioVazio::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | uenux2/src/app/comum/comparecimentomesario/estados/ctitulomesariovazio.cpp | high |
| 10357 | 130 |  | `comum::CLimiteMesariosRegistradosAtingido::vf7` | `comum::CLimiteMesariosRegistradosAtingido::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | high |
| 10358 | 105 |  | `comum::CLimiteMesariosRegistradosAtingido::GetControlador@10358` | `comum::CLimiteMesariosRegistradosAtingido::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | uenux2/src/app/comum/comparecimentomesario/estados/climitemesariosregistradosatingido.cpp | high |
| 10366 | 12 |  | `comum_f10366` | `atexit: CPedeTituloMesarioVotacao::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | high |
| 10372 | 12 |  | `comum_f10372` | `atexit: CPedeTituloMesarioFinal::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | high |
| 10379 | 12 |  | `comum_f10379` | `atexit: CPedeTituloMesarioInicial::GetInst()::s_inst.reset()` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | high |
| 10380 | 445 |  | `comum::CEncerraRegistroMesarios::GetControlador` | `comum::CEncerraRegistroMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp | high |
| 10383 | 194 |  | `comum::CConfirmaFimRegistroMesarios::GetControlador@10383` | `comum::CConfirmaFimRegistroMesarios::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | high |
| 10385 | 106 |  | `comum::CConfirmaFimRegistroMesarios::GetControlador@10385` | `comum::CConfirmaFimRegistroMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cconfirmafimregistromesarios.cpp | high |
| 10388 | 4789 |  | `comum::CPedeTituloMesario::StartState` | `comum::CPedeTituloMesario::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | high |
| 10391 | 17 |  | `comum::CRegistrarMesarios::GetControlador@10391` | `comum::CRegistrarMesarios::ProcessInput` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | high |
| 10392 | 209 |  | `comum::CRegistrarMesarios::GetControlador@10392` | `comum::CRegistrarMesarios::StartState` | src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp | high |
| 10399 | 12 |  | `comum::dao::CComparecimentoMesarioDAO::vf2` | `comum::dao::CComparecimentoMesarioDAO::Clone` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | medium |
| 10401 | 3016 |  | `comum::dao::CComparecimentoMesarioDAO::vf8` | `comum::dao::CComparecimentoMesarioDAO::RecuperarTodos` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | medium |
| 10402 | 2308 |  | `comum::dao::CComparecimentoMesarioDAO::vf7` | `comum::dao::CComparecimentoMesarioDAO::Recuperar` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | medium |
| 10403 | 558 |  | `comum::dao::CComparecimentoMesarioDAO::vf3` | `comum::dao::CComparecimentoMesarioDAO::Inserir` | src/uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.cpp | medium |
| 10990 | 10 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)0>::vf1` | `api::BatteryIconDataSource<Horizontal>::~BatteryIconDataSource (deleting)` | library/inlined helper (deleting destructor: 5510 + free) | uenux2/src/api/gui/cpowerinformation.cpp (template defined there, srcloc :119/:131) | high |
| 11369 | 871 |  | `comum::asn::CConversorNomesCargo::vf3` | `comum::asn::CConversorNomesCargo::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversornomescargo.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversornomescargo.cpp | high |
| 11459 | 535 |  | `comum::asn::CConversorPartido::vf3` | `comum::asn::CConversorPartido::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | high |
| 11652 | 10 |  | `api::BatteryIconDataSource<(api::CPowerInformation::IconOrientation)1>::vf1` | `api::BatteryIconDataSource<Vertical>::~BatteryIconDataSource (deleting)` | library/inlined helper (deleting destructor: 3837 + free) | uenux2/src/api/gui/cpowerinformation.cpp (template defined there, srcloc :119/:131) | high |
| 11653 | 257 | ✓ | `comum::CInfoMTLCD::Update` | `comum::CInfoMTLCD::Update` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | high |
| 11654 | 13 |  | `comum::CInfoMTLCD::vf1` | `comum::CInfoMTLCD::~CInfoMTLCD (deleting)` | src/uenux2/src/app/comum/cinfomtlcd.cpp | uenux2/src/app/comum/cinfomtlcd.cpp | high |
| 11658 | 37 |  | `comum_f11658` | `atexit: CArquivosSavd::GetInst()::s_instancia.reset()` | src/uenux2/src/app/comum/carquivossavd.u22.cpp | uenux2/src/app/comum/carquivossavd.cpp | high |
| 12587 | 467 | ✓ | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::vf2` | `api::CDataText<vota::(anonymous namespace)::DS_NomeCargoNeutroComEscolha>::GetText` | src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u22.cpp | uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp | medium |

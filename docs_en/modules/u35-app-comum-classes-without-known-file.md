# u35: `comum::` classes without a known source file (result writers, ASN.1 converters, voter-roll counters, report parts)

Unit u35 collects **98 wasm functions** of component `app:comum` (the `uenux2/src/app/comum` library shared by the
urna's applications) that the tools could not attach to an original file: 38 classes, most of them known only from
RTTI/vtables (no `std::source_location` record), plus two groups of free functions. This unit names them, rebuilds the code, and
infers where each class lived from the TSE naming convention (class `CFooBar` in `cfoobar.h/.cpp`, next to its
closest relatives). Paths marked *(path inferred)* are such guesses; files that ARE attested by srclocs (e.g.
`clocal.cpp`, `celeitores.cpp`, `crelutil.cpp`) receive a `.u35.cpp` fragment instead.

Only **17 of the 98 functions ran** during the recorded votes (`analysis/runtime/*.functions.tsv`): the start-up
readers of the election data (`CConversorCandidatura` 11448, `CConversorDadosCandidato` 11450,
`CConversorSecaoEleitoral` 11453, `CConversorDadoLocal` 11404, `CConversorDadoCorrespondencia` 11399/11400,
`CLocal::GetTodasSecoes` 5741, `CLocal::GetLocalID` 1933), `CCandidaturasDSSexo::operator()` (5806, the
confirmation screen), the signer `vota::CAssinadorVota` (1501, every state save), the application-context guard
(675), the singleton helper (1406) and a few string/library helpers (1080, 1221, 1374, 1880, 2769). Everything that
belongs to the **encerramento** (end of voting: BU, result files, copy to the result stick) never runs in the web
simulator, because the page never gets there.

The unit's contents, grouped by topic:

| topic | classes / functions | § |
|---|---|---|
| result-file writers of the encerramento | `IGravador` (slots 2..6), `IGravadorEnvelope`, `CGravadorEnvelopeArquivo`, `CGravadorRDV`, `CGravadorLog`, `CGravadorVersoesArquivos` (destructors) | 3 |
| copy of the results to the MR stick | `CCopiadorMR`, `CCopiadorWSQMR` | 4 |
| signing of state/result files | `vota::CAssinadorVota` | 5 |
| ASN.1 converters (23 functions, 19 classes) | `CConversorCarga`, `CConversorDadoCorrespondencia`, `CConversorCabecalhoPacote`, `CConversorDadosCandidato`, `CConversorSecaoEleitoral`, `CIndexadorFotos`, ... | 6 |
| voter roll, local, candidates | `CEleitores` counters + `Procura`, `CEleitorDetalhe::PodeVotar`, `CLocal` accessors, `CCandidaturasDSSexo`, `CRegraIdentidadeLivre`, `CRdvPosicionadorVota`, `CServicoEstadoGeral`, `EhFaseTreinamento`, `CPath::GetPathChaves` | 7 |
| reports | `CRelUtil` helpers, `CParteEleitores`, `CParteCargos`, `CParteCandidatosProporcionais`, `CGeradorRelPU`, `CGeradorRelVersaoPacoteDados`, `IEventosLog::LogaGeracaoRelatorio`, `CGeradorBUQRCodeVota::DadosEleicao` | 8 |
| infrastructure | `CAppStateContext`, `~CApplicationContextGuard`, merged singleton accessors, `CStringUtils::Trim/Split`, library instances | 9 |
| the BU (boletim de urna) pieces of this unit | step by step | 10 |

Glossary: *urna* voting machine; *eleitor* voter; *mesário* poll worker; *seção* polling section; *seção agregada*
a small section merged into this one (its voters vote here); *local de votação* polling place; *zona* electoral zone;
*município* municipality; *cargo* office being elected; *consulta* referendum/plebiscite question; *candidatura*
candidacy; *suplente/vice* running mate; *apto* able to vote/to run; *impedimento* impediment; *habilitação* the poll
worker enabling the voter (*biométrica* by fingerprint, *biográfica* by birth-year question); *TTE* transferência
temporária de eleitor (voter temporarily moved to this section); *carga* the media load that installed the election
data on this urna; *correspondência* the record tying urna and carga; *encerramento* end of voting; *BU* boletim de
urna, the per-urna result; *RDV* registro digital do voto, the shuffled table of cast votes; *zerésima* the zero
report printed before voting; *via* printed copy; *MI* memória interna (internal flash `/dsk/fi`); *MV* mídia de
votação (the removable flash card `/dsk/fe`); *MR* mídia de resultado (the USB stick `/dsk/mr` taken to the electoral
office); *SAVD* the urna's signature service; *fase* oficial / simulado / treinamento.

**Reconstructed sources** (new files unless marked *fragment*):

```
src/uenux2/src/app/comum/cappstate.h                                       CAppStateContext                 (path inferred)
src/uenux2/src/app/comum/gravadores/igravador.cpp                          IGravador slots 2..6             (path inferred)
src/uenux2/src/app/comum/gravadores/igravadorenvelope.{h,cpp}              IGravadorEnvelope                (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.{h,cpp}       CGravadorEnvelopeArquivo         (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorrdv.{h,cpp}                   CGravadorRDV                     (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorlog.{h,cpp}                   CGravadorLog                     (path inferred)
src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp       fragment (file of u33)
src/uenux2/src/app/comum/gravadores/ccopiadormr.{h,cpp}                    CCopiadorMR, CCopiadorWSQMR      (path inferred)
src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.{h,cpp}            CConversorCarga                  (path inferred)
src/uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.{h,cpp}                             (path inferred)
src/uenux2/src/app/vota/comum/cassinadorvota.{h,cpp}                       vota::CAssinadorVota             (path inferred)
src/uenux2/src/app/comum/asn/cconversorhasharquivo.u35.cpp                 fragment (class of u17 fragment)
src/uenux2/src/app/comum/asn/cconversorseguranca.{h,cpp}                   CConversorSeguranca              (path inferred)
src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.u35.cpp             fragment (file of u21)
src/uenux2/src/app/comum/asn/util.u35.cpp                                  fragment: Utils::ConverteDataHoraJE + 4 inlined Desconverte*
src/uenux2/src/app/comum/md/ccabecalhopacote.u35.cpp                       fragment: CCabecalhoPacote / CIDPacoteValidar / CIDEleitoral (inlined)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.{h,cpp}             (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.u35.cpp      fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.{h,cpp}                    (path inferred)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.{h,cpp}                      (path inferred)
src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.{h,cpp}                  (path inferred)
src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.u35.cpp     fragment (file of u03)
src/uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.{h,cpp}     CIndexadorFotos                  (path inferred)
src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp   DoDesconverte (header of u21)    (path inferred)
src/uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.{h,cpp}                             (path inferred)
src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.u35.cpp        fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.u35.cpp fragment (file of u21)
src/uenux2/src/app/comum/dados/asn/cconversorpartido.u35.cpp               fragment (file of u22)
src/uenux2/src/app/comum/dados/md/candidatura/cdadoscandidato.u35.cpp      fragment: 4 inlined constructors (cdadoscandidato.cpp:35/:65)
src/uenux2/src/app/comum/dados/md/chorarioverao.u35.cpp                    fragment: inlined constructor (:29/:32)
src/uenux2/src/app/comum/dados/md/csecaoeleitoral.u35.cpp                  fragment: inlined constructor (:34/:37/:41)
src/uenux2/src/app/comum/dados/md/processoeleitoral/cdetalheconsulta.u35.cpp  fragment (:29/:32)
src/uenux2/src/app/comum/dados/md/processoeleitoral/crespostaconsulta.u35.cpp fragment (:29/:32)
src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.{h,cpp}            CRegraIdentidadeLivre            (path inferred)
src/uenux2/src/app/comum/dados/crdvposicionadorvota.{h,cpp}                CRdvPosicionadorVota             (path inferred)
src/uenux2/src/app/comum/dados/clocal.u35.cpp                              fragment: GetTodasSecoes, GetLocalID, GetQtdAgregadas
src/uenux2/src/app/comum/dados/celeitores.u35.cpp                          fragment: Procura + 4 counters
src/uenux2/src/app/comum/dados/celeitordetalhe.u35.cpp                     fragment: PodeVotar
src/uenux2/src/app/comum/dados/ccandidaturas.u35.cpp                       fragment: CCandidaturasDSSexo::operator()
src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp          CServicoEstadoGeral::GetPathArquivo (path inferred)
src/uenux2/src/app/comum/appinfo/cappinfo.u35.cpp                          fragment: EhFaseTreinamento      (file inferred)
src/uenux2/src/app/comum/cpath.u35.cpp                                     fragment: GetPathChaves
src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u35.cpp fragment: CRegistradorMesario::Existe
src/uenux2/src/app/comum/log/ieventoslog.u35.cpp                           fragment: LogaGeracaoRelatorio
src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp                       fragment: IncluiSeparador, CompletaDireita, FormataQtdAptos
src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u35.cpp               fragment: CGeradorBUQRCodeVota::DadosEleicao
src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp               fragment: ~CParteCandidatosProporcionais
src/uenux2/src/app/comum/relatorios/cpartecargos.{h,cpp}                   CParteCargos                     (path inferred)
src/uenux2/src/app/comum/relatorios/cparteeleitores.{h,cpp}                CParteEleitores                  (path inferred)
src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp                      destructors (header shared with u37) (path inferred)
src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.{h,cpp}   CGeradorRelVersaoPacoteDados     (path inferred)
src/uenux2/src/api/gui/capplicationcontextstack.u35.cpp                    fragment: ~CApplicationContextGuard
src/ecourna/api/util/cstringutils.u35.cpp                                  fragment: CStringUtils::Trim(const&), Split
src/uenux2/src/app/comum/u35-foreign-fragments.cpp                         merged singleton body, error thunk, implicit dtors, library list
```

---

## 1. Where this code sits in the voting process

`comum` is the layer between the election data files (ASN.1/BER, see `docs/data-model/asn1-schemas.md`) and the
applications (`vota` is the voting application). The functions of this unit touch four moments of the urna's day:

1. **Start-up (votaInit in the web build).** The urna reads its static data: candidates (`-ca.dat`, decoded with
   `CConversorCandidatura`/`CConversorDadosCandidato`), the section (`-lo.dat`, `CConversorSecaoEleitoral`),
   package headers (`*.pid`, `CConversorCabecalhoPacote`), situations of the elections (`-ste.dat`), daylight-saving
   periods (`-cm.dat`), photos (`-fo.dat`, indexed by `CIndexadorFotos` without decoding the images). It also reads
   and writes its persistent state `eg.bin` / `vota.bin` (`CConversorDadoCorrespondencia`, `CConversorDadoLocal`,
   `CConversorLocalidadeEleitoral`, `CConversorNumViasImpressasRelatorios`) and signs it (`vota::CAssinadorVota`).
2. **Voting.** The confirmation screens read the current candidacy (`CCandidaturasDSSexo`: the office title agrees
   with the candidate's sex). Each cast vote is inserted into the RDV at a content-dependent position
   (`CRdvPosicionadorVota`) so that the RDV does not reveal the voting order. The mesário looks voters up
   (`CEleitores::Procura`) and every guarded step pushes an "application context" for the error screen
   (`~CApplicationContextGuard`).
3. **Reports** printed from the menus (voter list, package versions, urna parameters, zerésima, BU): report parts
   and layout helpers of §8.
4. **Encerramento.** `vota::CGravaResultado` builds one writer per result file and drives the `IGravador` pipeline
   of §3 (write in the MI work area, copy to the MI result area, sign, copy to the MV); then
   `vota::CCopiaResultadoParaMR` copies everything to the result stick with `CCopiadorMR` (§4).

---

## 2. Classes and hierarchy (RTTI)

```
comum::IResultado (u23) ─ comum::IGravador (slots 2..6 here: igravador.cpp)
   ├─ CGravadorRDV                     @1556856   rdv.dat      (dtors here; GravaResultado u18)
   ├─ CGravadorLog                     @1557196   log.jez      (dtors here; GravaResultado u12)
   ├─ CGravadorVersoesArquivos         @1557248   mr.ver       (dtors here; rest u33)
   ├─ CGravadorBU / CGravadorRCSecao / CGravadorHashes / CGravadorWSQ   (u23)
   └─ IGravadorEnvelope                @1554456   (dtor here; GravaResultado u18)
        └─ CGravadorEnvelopeArquivo    @1554512   imgbu.dat / imgze.dat (dtors here; ctor u07; LeConteudo u12)
comum::CCopiadorMR                     @1553348   ─ CCopiadorWSQMR @1553380
comum::CAssinador (u23)                @1553260   ─ vota::CAssinadorVota @1532312 (ctor here)
api::CStateContext<comum::CAppState>              ─ comum::CAppStateContext @1551312
api::IReportPart                                  ─ comum::CParteEleitores @1576596, CParteCargos @1576564,
                                                    CParteCandidatosProporcionais @1576452 (siblings: u25)
comum::CGeradorRelPU                   @1545288   (no base)
comum::CGeradorRelVersaoPacoteDados    @1576284   (no base, 6 slots)
comum::CGeradorBUQRCode (u25)                     ─ CGeradorBUQRCodeVota @1575984 (slot 3 here)
comum::md::IRegraIdentidade (u05)                 ─ CRegraTitulo, CRegraCPF (u05), CRegraIdentidadeLivre @1574652
comum::md::CRdvPosicionador                       ─ comum::CRdvPosicionadorVota @1560252
comum::IServicoEstado<CEstadoGeral, CConversorEstadoGeral>  ─ CServicoEstadoGeral @1558024
comum::asn::IConversorASN<ENTIDADE, DADO> (u21)   ─ 18 converters of §6
comum::asn::IConversorParcialASN<EntidadeFotosCandidatos, md::CIndicesFotos, CVisitanteFoto> ─ CIndexadorFotos @1562548
```

Converter vtables all have the shape `[0] 174 (trivial dtor) [1] 144 (operator delete) [2] DoConverte [3]
DoDesconverte` (the default slot throws 7655/7656 "Método DoXxx() não implementado para {}", so a read-only file
only overrides slot 3). The partial converter `CIndexadorFotos` is the exception: its vtable has 3 slots and slot 2
is its `DoDesconverte(entidade, visitante)`.

---

## 3. Result-file writers (`IGravador` pipeline)

`IGravador` (declared by u23 in `iresultado.h`) owns the name of ONE result file:
`"<fase o|s|t><pleito:05><uf><município:05><zona:04><seção:04>-<sufixo>"`, e.g. `t02410ac0000100010001-bu.dat`.
The five non-pure virtual methods are shared by every writer except `CGravadorWSQ` (same table slots 2187..2191 in
the vtables of CGravadorBU, CGravadorRDV, CGravadorRCSecao, CGravadorHashes, CGravadorLog, CGravadorVersoesArquivos,
IGravadorEnvelope and CGravadorEnvelopeArquivo). `CGravadorWSQ` overrides all five. Slot 2 (func 11579, with `resultado(MI)/<nome>`) and slots 3/6 (11577/11578 =
`comum_f6041(this, 0|1)`) all end in the large WSQ packaging body func 5821 (glob of `*.wsq`,
`CGravadorWSQ::GetCaminhoCorretoInternal/External`, cgravadorwsq.cpp:175/192, ecourna icompressor.cpp:61). Slots 4/5
(`Grava`/`GravaMV`) and 7 are the empty ICF bodies 218/425:

| slot | func | method (names from u23) | what it does |
|---:|---:|---|---|
| 2 | 11632 | `CopiaParaResultado()` | `CopyFile(trab(MI)/nome, resultado(MI)/nome)` |
| 3 | 11630 | `CopiaParaMV()` | `CopyFile(trab(MI)/nome, trab(MV)/nome)` then `CopyFile(resultado(MI)/nome, resultado(MV)/nome)` |
| 4 | 11634 | `Grava()` | `CFile f(trab(MI)/nome, "wb"); GravaResultado(f) /*slot 7*/; f.Close();` |
| 5 | 11633 | `GravaMV()` | the same on `trab(MV)` (no caller in CGravaResultado) |
| 6 | 11631 | `CopiaResultadoParaMV()` | `CopyFile(trab(MV)/nome, resultado(MV)/nome)` (no caller in CGravaResultado; CGravadorWSQ overrides it) |

`trab` = `dinamico/trab<turno>/`, `resultado` = `dinamico/res<turno>/` (`CPath::GetPathTrab/GetPathResult`, the
turno read from eg.bin). `CSystem::CopyFile(origem, destino, preservaAtributos=false)` (func 378).

The concrete writers of this unit only contribute destructors (their `GravaResultado` bodies are in other units),
which document their layouts:

| writer | file (sufixo, SAVD id) | layout (beyond IResultado +0..+39) |
|---|---|---|
| `CGravadorRDV` | `-rdv.dat` (6, 37) | +40 `CDateTime` data de geração, +52 `'1'`, +56 `CDadoCorrespondencia` (96 B), +152 optional motivo SA (flag +160), +164 `const CRdvVota*` |
| `IGravadorEnvelope` | — | +40 data de geração, +52 fase, +56 `'1'`, +60 optional (flag +68), +72 `CEnvelopeGenerico::Tipo`, +76 `CDadoCorrespondencia` |
| `CGravadorEnvelopeArquivo` | `-imgbu.dat` (9, 43) / `-imgze.dat` (11, 44) | +172 path of the wrapped file (`trab/bu.dat`, `trab/ze.dat`) |
| `CGravadorLog` | `-log.jez` (13, 60) | +40 `dinamico/log/arquivados/`, +52 `dinamico/log/logd.dat`, +64 `trab(MI)` |
| `CGravadorVersoesArquivos` | `-mr.ver` (19, 70) | +40 tag string, +52 `map<string,string>` module → version |

Two of these destructors are the merged body `comum_f6043(this, offset, vtable)` (IGravadorEnvelope with offset 76,
CGravadorRDV with 56): wasm-opt merged the two functions that differ only in the offset of the
`CDadoCorrespondencia` member and in the vtable constant.

---

## 4. Copy to the result stick (`CCopiadorMR`)

`CCopiadorMR {vptr, std::string m_nome}` (16 bytes) is built by `vota::CCopiaResultadoParaMR::CopiaResultado`
(u08) for each of `bu.dat, rdv.dat, jufa.dat, imgbu.dat, imgze.dat, hash.dat, log.jez, vota.vsc, mr.ver` and, on a
biometric urna, as `CCopiadorWSQMR` for `wsqbio/wsqman/wsqmes.jez`.

* slot 2 `Copia()` (func 5874): `CopyFile(resultado(MI)/nome, /dsk/mr/nome)`. The only slot used.
* slot 3 `CopiaDaMV()` (func 5873, name inferred): the same from `resultado(MV)`. Never called by VOTA.

`CCopiadorWSQMR`'s vtable points at the very same functions but through **different table slots** for slots 1..3
(2179..2181 instead of 2176..2178); slot 0 shares table slot 2175 with the base (the complete destructor was emitted
as an alias of `~CCopiadorMR`). A table slot is created per function at link time. A per-class deleting destructor
is always emitted, so the separate slot 1 proves nothing. The separate slots 2 and 3 do show that the WSQ copier had
its own `Copia` / `CopiaDaMV` overrides. Their compiled bodies became byte-identical to the base ones and wasm-opt
folded them. In this build the WSQ copier behaves exactly like the base class. Only the BU copy is compared with
its source afterwards (`CSystem::AreFilesEqual`, in the caller).

---

## 5. Signing (`vota::CAssinadorVota`, func 1501)

`CAssinadorVota(pacote)` = `comum::CAssinador(ESavdAplic 1 = VOTA, pacote)` with the base constructor inlined:

```
caminho     = CArquivosSavd::GetInst()[pacote]            // carquivossavd.cpp:49, e.g.
              //  122/123: /dsk/fi/dinamico/trab1|2/vota.vsu          (state files of the MI)
              //  158/159: /dsk/fi/dinamico/res1|2/<prefixo>-vota.vsc (result package)
m_diretorio = Diretorio(caminho) + "/"                   // func 5462: text before the last '/'
m_arquivo   = NomeArquivo(caminho)                       // func 2763: text after the last '/'
m_local     = ""                                         // filled only by CGravaResultado before AssinaArquivosResultado
```

In the recorded sessions it ran from `comum_f491` (the state save, "Gravando o estado da urna"; 88 profiler
samples, the heaviest function of this unit). Other callers: the vote synchronisation of the urna
(`vota::impl::CSincronismoVotoEleitor` 7174, replaced by the web policy in the simulator) and
`CSincronizaVota::SincronizaRelatorios`. The actual signing
(`CAssinador::Assina`, func 1277) asks the SAVD; in the web build `CWasmSavd` answers OK and the `.vsu` files
contain the literal `assinatura simulada para vota_web_wasm` (see `docs/00-provenance.md`).

---

## 6. ASN.1 converters

| class | ASN.1 type ⇄ md type | func (slot) | behaviour / checks |
|---|---|---|---|
| `CConversorHashArquivo` | `ArquivoAssinatura` ⇄ `api::hash::CHashArquivo` | 10267 (2) | `{nomeArquivo, assinatura}` from the two strings (hash.dat entries) |
| `CConversorHistoricoVotoImpresso` | `ModuloBoletimUrna::HistoricoVotoImpresso` ⇄ `md::CHistoricoVotoImpresso` | 10276 (2) | 2 ints + `DataHoraJE`; never filled by VOTA (BU field absent) |
| `CConversorCarga` | `ModuloTiposEcoUrna::Carga` ⇄ `md::CCarga` | 10291 (2), 10290 (3) | serial "8 hex" ⇄ 4-byte OCTET STRING (3605/3604), `DataHoraJE`, generator via the ecourna converter |
| `CConversorSituacoesEleicoes` | `EntidadeSituacoesEleicoes` → `vector<CSituacoesEleicoes>` | 11361 (3) | keeps only `tipo == ativa`; suspended/cancelled/excluded elections silently dropped |
| `CConversorDetalheConsulta` | `ModuloEleicao::DetalhePergunta` → `md::CDetalheConsulta` | 11377 (3) | question text split at '\|', each line trimmed, re-joined; answers: número < 100000 (8167), non-empty (8168); question non-empty (8146), ≥ 1 answer (8147) |
| `CConversorHorarioVerao` | `HorarioVerao` → `md::CHorarioVerao` | 11384 (3) | `fim > início` (8003 "Período inválido."), `diferença != 0` (8004) |
| `CConversorNumViasImpressasRelatorios` | `NumViasImpressasRelatorios` ⇄ 4 × uint8 | 11386 (2), 11387 (3) | ASN.1 range 0..999 but only the low byte is read |
| `CConversorLocalidadeEleitoral` | `DadoSecao` ⇄ `CLocalidadeEleitoral` | 11388 (2) | município/zona/seção (11389 = u05) |
| `CConversorDadoCorrespondencia` | `DadoCorrespondencia` ⇄ `CDadoCorrespondencia` | 11399 (2), 11400 (3) | carga + secaoCarga + assinatura; **no verification** of the assinatura |
| `CConversorDadoLocal` | `DadoLocal` → `CDadoLocal` | 11404 (3) | uf, secaoCarga, tipoLocalVotacao |
| `CConversorSeguranca` | `Seguranca` ⇄ `md::CSeguranca` | 11413 (2) | `idArquivoCD` never written (stays 0) |
| `CConversorCabecalhoPacote` | `CabecalhoPacote` → `md::CCabecalhoPacote` | 11437 (3) | inlines util.cpp:89/329/451, cideleitoral.cpp:21/29/37 (id < 100000), cidpacote.cpp:21 (fase not '0'/'4'), ccabecalhopacote.cpp:33/36/39 (nome non-empty, versão 12 chars, origem in 1..12, compiled as one unsigned compare `(unsigned)(origem - 13) <= 0xFFFFFFF3`); abrangência ignored |
| `CIndexadorFotos` | `EntidadeFotosCandidatos` (partial) → `md::CIndicesFotos` | 11441 (2) | copies the visitor's `{código, [início, fim)}` list; keeps only the município of the abrangência |
| `CConversorCandidatura` | `Candidatura` → `md::CCandidatura` | 11448 (3) | titular + suplentesVices (`at()` bounds-checked); `drap` ignored |
| `CConversorDadosCandidato` | `DadosCandidato` → `md::CDadosCandidato` | 11450 (3) | keeps código, nomeUrna, nomeFonético, sexo (util.cpp:483: 0/2/4 → 0/1/2), ordemSuplência (1..9, 7998/7999); ignores nome civil/social, nascimento, situação, cassação, partido, reeleição; "apto" = list it came from |
| `CConversorSecaoEleitoral` | `SecaoEleitoral` → `md::CSecaoEleitoral` | 11453 (3) | agregadas validated (iconversorasn.h:71), **sorted by número**, then csecaoeleitoral.cpp:34/37/41: 1..9999 (8053), ≠ principal (8054), unique (8055) |
| `CConversorIdentificacaoAgregada` | `IdentificacaoAgregada` ⇄ `CIdentificacaoAgregada` | 11456 (2) | número + tipo local |
| `CConversorPartido` | `Partido` ⇄ `md::CPartido` | 11460 (2) | nome, sigla, número |
| `CConversorCodigoCargoConsulta` | `CodigoCargoConsulta` (CHOICE) ⇄ `uebyte` | 11592 (2), 11591 (3) | ≤ 24 → `cargoConstitucional`, else `numeroCargoConsultaLivre`; back: the value byte of either alternative |

`Utils::ConverteDataHoraJE` (func 1080) produces every `DataHoraJE` written by the urna:
`CDate::Format("YYYYMMDD") + "T" + CTime::Format("hhmmss")`. The eg.bin written at votaInit
(`analysis/runtime/memfs-after-init/dsk/fi/dinamico/eg.bin`) contains `20201231T235958`, the fixture's dataHoraCarga,
written by 11399.

---

## 7. Voter roll, local, candidates and state helpers

* **`CEleitores` counters** (celeitores.u35.cpp). `GetQtdHabilitadosPorBiometria` (2821) and
  `GetQtdHabilitadosPorBiografia` (1935) are thunks to `QtdVotaramPorTipoHabilitacao(1|2)` (6034);
  `GetQtdCompareceramSemBiometria` (2822) is the same loop compiled with the constant 0. Only voters with dynamic
  data (the optional at `CEleitorDetalhe` +192, filled by `CEleitorDinamicoDAO`, whose table name `eleitor_dinamico`
  is in the binary) and `estado == VOTOU` are counted. The `uenux.db` files of the recorded web sessions have no such
  table (only `comparecimento_mesario` and `registro_justificativa`, `analysis/runtime/memfs-after-vote`). `GetQtdAptosSecao` (2823) counts
  voters without impediment in the current turno (`PodeVotar`, 2266), split by the TTE field (+48 of the voter;
  any non-zero type, including the accessibility transfer, counts as temporary).
* **`CEleitores::Procura`** (2264): builds a `CEleitorIdentidade` (validated, padded to 11/12 digits), maps it to
  the voter's principal identity (map at +112), finds the voter and makes it the roll's current voter.
* **`CEleitorDetalhe::PodeVotar`** (2266): impediment of turno 2 (+200) if eg.bin's turno is `'2'`, else of turno 1
  (+196); apto when 0.
* **`CLocal`** (clocal.u35.cpp): `GetTodasSecoes` (5741) = principal section + agregadas; `GetLocalID` (1933) =
  local de votação; `GetQtdAgregadas` (2816). Each first calls `VerificaLido/VerificaEhSecao("<own name>")`, which is
  how the names were recovered.
* **`CCandidaturasDSSexo::operator()`** (5806): sex of the current titular, or of the running mate of ordem
  `m_suplente`.
* **`CRegraIdentidadeLivre`** (11273/11274 + shared `return true`): the rule for free-form identifiers: GetTipo 3,
  Formata = identity, Verifica = true (the identifier must still be all digits: iregraidentidade.cpp:23).
* **`CRdvPosicionadorVota::Posiciona`** (11496): `std::upper_bound` over the cargo's votes by (tipo, digitado);
  equal votes are appended after the existing ones. This is what keeps the RDV **sorted by content** rather than by
  time (privacy of the vote). Not executed in the web build (its sync policy does not persist votes).
* **`CServicoEstadoGeral::GetPathArquivo`** (11569): `<flash>/dinamico/eg.bin` (no turno component).
* **`EhFaseTreinamento`** (1485): eg.bin fase == `'3'`. Always true in the simulator (all scenarios are training
  loads); in training the WSQ packages are not encrypted, the recognised fingerprint is not stored and the
  end-of-voting time lock is never armed.
* **`CPath::GetPathChaves`** (1948): `GetPathRoot("/dsk/fi/estatico/chave/")`, the key directory (bu.pk1, wsq.pk1,
  bio.sk1, cv.ber.pri...).
* **`CRegistradorMesario::Existe`** (2221): whether the mesário-attendance cache exists, without creating it.

---

## 8. Reports

* `CRelUtil::IncluiSeparador(b, alinhamento)` (1387): 38 dashes. `CRelUtil::CompletaDireita(texto, largura)` (1881):
  right-pad with spaces. `CRelUtil::FormataQtdAptos(std::function<SQtdeAptos()>)` (1921): the three BU lines
  `"Eleitores aptos                   {:04}\n"`, `"          Originais da seção      {:04}\n"`,
  `"          Temporários na seção    {:04}"` (total = originais + temporários, as uint16).
* `IEventosLog::LogaGeracaoRelatorio(relatorio, termino)` (1047): log record
  `"Gerando relatório [<BU|ZERESIMA|...>] [INÍCIO|TÉRMINO]"` around every printed report.
* `CParteEleitores::Imprime` (11206): one sub-part per voter of the roll (voter-list report), walking the roll with
  the singleton's shared cursor. `CParteCargos` / `CParteCandidatosProporcionais`: destructors only (their `Imprime`
  are in u04; layouts in the headers).
* `CGeradorRelVersaoPacoteDados::MontaDados` (11222, slot 4): after a blank line, one centred line per data package
  `"<nome><espaços><versão>"` (38 columns), then `"Dados:"` + the first 8 Base64 characters of eg.bin's
  `hashVersoesPacotes`, right-aligned.
* `CGeradorRelPU`: `{vptr, CPaperFormBuilder}`; only its destructors are in this unit (11900/11901).
* `CGeradorBUQRCodeVota::DadosEleicao` (11241): returns `""` (VOTA adds no per-election data to the BU QR codes).

---

## 9. Infrastructure

* `CAppStateContext(estadoInicial)` (3844): `{vptr @1551312, CAppState*}`; one per cooperative thread.
* `~CApplicationContextGuard` (675): removes the guard's context from the global context stack **only when no
  exception is in flight** (`std::uncaught_exceptions() == 0`); on an exception the context stays on top so that the
  monitor thread shows it on the fatal-error screen. Search from the top with `operator==` (5556), erase by
  move-assignment (5552).
* `GetInstCriada<T>` (1406): the merged body of nine `GetInst()` accessors (CConfiguracaoEleicao, CCargos,
  CEleitores, CTelasVota, CRdvVota, CPE, CValidadorIdentidade, CRespostas, CHV): `lock; if (!p) throw
  CPatternError(1303, "<X> - instancia nao criada"); return *p;`. Only the mutex unlock residue remains (no threads).
* `CCodecWsq::GetInst` (2742, name used by u24 with "?"): lazily created empty singleton (ICF body 2900); its calls
  were reduced to copies in this build.
* `CStringUtils::Trim(const std::string&)` (1374, copy + slot 6416) and `CStringUtils::Split(texto, sep)` (1880).
* Library instances: std::sort of consulta answers (248, 1920, 2790), map/set helpers (1221, 1405, 2806), vector
  helpers (1874, 2769), `std::function` copy (1922), `pair<const char, CLabelParametrizado>` (1552),
  `filesystem::path::string()` (1555), `ASN1::CHOICE::operator=` (2188). Implicit destructors:
  `~CDadoCorrespondencia` (857), `~CCabecalhoQRCode` (2885). Error thunk: `CUeComumMdError` constructor (591).

---

## 10. The BU (boletim de urna): what this unit contributes

The BU itself (`EntidadeBoletimUrna`, `CGravadorBU`, `CConversorEntidadeBU`, `CGeradorBU`) is documented in
`docs/10-boletim-de-urna.md`, `docs/bu/*.md` and units u23/u25. The pieces that live in this unit are:

1. **Counters of the BU** (`DetalhamentoComparecimento` of the BU file, QR fields `APTO/APTS/APTT` and
   `HBSB/HBBM/HBBG`, printed lines "Eleitores aptos / Originais da seção / Temporários na seção" and
   "Habilitação sem biometria / biométrica / biográfica"):
   * aptos of the QR header and of the printed BU = `CEleitores::GetQtdAptosSecao()` (2823): voters with
     impediment 0 in the current turno, split originais / TTE; printed by `CRelUtil::FormataQtdAptos` (1921) as
     `{:04}`. (The per-eleição `qtdEleitoresAptos/AptosSecao/AptosTTE` of `ResultadoVotacaoPorEleicao` in the BU file
     come from the per-abrangência map `CEleitores::GetQtdAptos()`, func 2824, unit u04, which applies the same
     `PodeVotar` rule.)
   * `qtdEleitoresCompareceramSemBiometria` = 2822, `qtdEleitoresHabilitadosPorBiometria` = 2821,
     `qtdEleitoresHabilitadosPorBiografia` = 1935: voters with `estado VOTOU` and the given `tipoHabilitacao`
     (0/1/2) of `eleitor_dinamico`.
   * `LOCA` / identification: `CLocal::GetLocalID` (1933); `AGRE`: `CLocal::GetQtdAgregadas`/`GetTodasSecoes`.
2. **Carga / correspondência in the "urna" block of every result file**: `CConversorCarga` writes
   `Carga {numeroInternoUrna INTEGER, numeroSerieFC OCTET STRING(4) from 8 hex digits, identificadorGeradorMidia
   {nome, serialCertificadoTPM, serialInstalacao}, dataHoraCarga "YYYYMMDDThhmmss", codigoCarga (24 digits)}`.
   The same data is kept in eg.bin as `DadoCorrespondencia` (+ `secaoCarga`, `assinatura`) by
   `CConversorDadoCorrespondencia`; in the simulator the fixture has `numeroInternoUrna 87654321`, `codigoCarga
   "123456789012345678901234"`, `dataHoraCarga 20201231T235958`, generator `nome_maquina` and a 139-byte test
   `assinatura` that is copied but never verified.
3. **`historicoVotoImpresso`** (printed-vote module history): converter 10276 exists, VOTA never fills it.
4. **BU QR codes**: `CGeradorBUQRCodeVota::DadosEleicao` returns `""`; the header `CCabecalhoQRCode` (33 strings) is
   destroyed by 2885.
5. **Log**: `"Gerando relatório [BU] [INÍCIO]"` / `"[TÉRMINO]"` (1047) around the BU printing (`vota::CGeraBU`).
6. **Printed vias**: the printed BU image `trab/bu.dat` becomes `<prefixo>-imgbu.dat` through
   `CGravadorEnvelopeArquivo` (EntidadeEnvelopeGenerico, tipo "BU impresso"); the zerésima image `trab/ze.dat` becomes
   `-imgze.dat`.
7. **Writing, copying, signing** (the order of `vota::CGravaResultado`, u07):
   1. `Grava()` of every writer: `trab(MI)/<prefixo>-bu.dat`, `-rdv.dat`, `-jufa.dat`, `-imgbu.dat`, `-imgze.dat`,
      `-hash.dat`, `-mr.ver`, `-log.jez` (file opened `"wb"`, slot 7 encodes, then closed). `CGravadorWSQ::Grava()` is
      a no-op.
   2. `CopiaParaResultado()`: each file copied to `resultado(MI)` = `dinamico/res<turno>/`. For the `wsq*.jez`
      writers this is the override func 11579, which runs the WSQ packaging body (func 5821) for
      `resultado(MI)/<nome>`. Nothing was written by their `Grava()`.
   3. `CAssinador::AssinaArquivosResultado` with the `CAssinadorVota(158|159)` object: the SAVD signs the list
      of files into `res<turno>/<prefixo>-vota.vsc` (simulated in the web build).
   4. If the MV is present, `CopiaParaMV()`: trab and resultado copies on `/dsk/fe`, then the `.vsc` is copied.
   5. `vota::CCopiaResultadoParaMR`: `CCopiadorMR::Copia()` for each file → `/dsk/mr/<prefixo>-<sufixo>`; only the
      BU copy is compared with the source (`AreFilesEqual`, "Nao copiou" on mismatch, code 9370).
8. **RDV** (the BU's source of truth for the counts): each vote is inserted at `CRdvPosicionadorVota::Posiciona`
   (sorted by vote type and digits), so `-rdv.dat` lists votes in content order.

---

## 11. Web-build specifics

* None of the encerramento code (§3, §4, §10.7) runs: the page never ends the election.
* `CAssinadorVota` runs, but the SAVD is `CWasmSavd` (always OK); the `.vsu` files are fake text.
* `EhFaseTreinamento()` is always true (training scenarios), so every "only in the official phase" branch is dead
  in the simulator.
* The RDV positioner is not exercised: the web policy `CSincronismoVotoEleitorWeb` does not store votes.
* `CCodecWsq` and the fingerprint paths are stubs.

## 12. Notable wasm / Emscripten observations

* **Folded overrides detectable from table slots**: `CCopiadorWSQMR` slots 1..3 point at the base functions via
  distinct table slots (§4), so they were distinct functions at link time.
* **Merged destructor bodies**: `~CGravadorRDV` and `~IGravadorEnvelope` are thunks into `comum_f6043(this,
  offset, vtable)` (merge-similar-functions turned the member offset 56/76 and the vtable into parameters);
  `~CCopiadorMR` (shared_f1722/1723), `~CParteEleitores` (api_f6026) and `~CGeradorRelPU` (shared_f6055 / unknown_f6054) share
  bodies with unrelated classes (RHVoice, api::CFormPart). Some deleting destructors call the complete one
  (`CGravadorLog` 11583 → 5823), others repeat its code inline (`CGravadorVersoesArquivos` 11580, `CGravadorRDV`
  11585).
* **Constructors inlined into converters**: the md classes `CDadosCandidato`, `CHorarioVerao`, `CSecaoEleitoral`,
  `CDetalheConsulta`, `CRespostaConsulta`, `CCabecalhoPacote`, `CIDPacote`, `CIDEleitoral` have no out-of-line code:
  their srcloc records (file:line of each throw) only appear inside the DoDesconverte functions of this unit. The
  fragments in `dados/md/**.u35.cpp` and `md/ccabecalhopacote.u35.cpp` write them out.
* **Vtable-slot names**: every converter slot is resolved from the `IConversorASN` layout; the tools' `vfN` names
  are replaced by `DoConverte` (slot 2) / `DoDesconverte` (slot 3).
* **No decompiler artefact in 11569 / 11274** (correction): `decompiled/app-comum/_by_index/11500.dcmp` and
  `11000.dcmp` show each call once, like the wasm text. A `q.py f` card lists the callees on its `calls` line and
  then prints the body, so a callee name appears twice in the card; that is not duplicated code.
* **Emscripten exceptions**: `~CApplicationContextGuard` calls `env.__cxa_uncaught_exceptions` (JS side), and
  `Split`/`CompletaDireita` call through `invoke_*` trampolines only to reach their cleanup landing pads (the
  string in `CompletaDireita`; the temporary string and the result `std::vector<std::string>`, func 2255, in `Split`).
* The functions attributed to `app:comum` include ecourna (`CStringUtils::Trim/Split`), api
  (`~CApplicationContextGuard`), vota (`CAssinadorVota`) and library code: the classifier followed the callers.

---

## 13. Weird code / risks (details in the StructuredOutput "suspicious" list)

* `CGeradorRelVersaoPacoteDados::MontaDados` (11222): `std::string(38 - (nome + versão), ' ')` throws
  `std::length_error` for a package name longer than 26 characters (the longest in the scenarios is 25).
* `CConversorNumViasImpressasRelatorios::DoDesconverte` (11387): 0..999 counters truncated to 8 bits.
* `CConversorSituacoesEleicoes` (11361): non-active elections disappear silently.
* `CConversorDadoCorrespondencia` (11400): the load's `assinatura` is carried, never checked here.
* `CRegraIdentidadeLivre` + the rule lookup of func 566: a free-identifier rule placed before the título/CPF rule
  disables their check digits. The lookup stops at the first rule whose `GetTipo()` is 3 or the requested type. The
  rules are created in the order of the configuration's list of identifier types (loop in func 7787: 1 → CRegraTitulo,
  2 → CRegraCPF, 3 → CRegraIdentidadeLivre, anything else throws "Tipo de identificador principal inválido {}").
  So the order depends on the configuration data. Nothing in the code prevents it.
* `CParteEleitores::Imprime` (11206) moves the roll's shared cursor to the end.
* `~CApplicationContextGuard` (675): a context whose scope was left by an exception that is later caught is never
  removed (stale context on the error stack).
* `CConversorCabecalhoPacote` (11437): the header validation is NOT weak (correction). `origem` must be in 1..12
  (one unsigned compare, `i32.le_u`) and `DesconverteIdSistema` can only return 1..12 anyway. `DesconverteFase`
  (2843, util.cpp:116) rejects everything outside 1..3 before `CIDPacoteValidar` repeats a sentinel test ('0'/'4').
  Only the optional abrangência is ignored, because `md::CCabecalhoPacote` has no member for it.

## 14. Open questions

* Real names of `IGravador` slots 5/6 and of `CCopiadorMR` slot 3 (no caller in VOTA); whether `CopiaDaMV` is used
  by another application (SA / recuperação).
* `md::CIndicesFotos` keeps only one int of the abrangência (+16 = município): is it really the município or a
  "código de abrangência"?
* The class that owns `Split` (ecourna `CStringUtils` vs uenux2 `api::CStringUtils` vs comum `util`), and
  `CompletaDireita` vs `PadRight` (1881).
* `CCodecWsq` (2742): real class name and file.
* Layout of the optional at +60 of `IGravadorEnvelope` / +152 of `CGravadorRDV` (value type 8 bytes?).

---

## 15. Complete mapping table (all 98 functions)

"ran" = observed executing in the recorded votes. "src file" = where the reconstruction is (or why none).

| func | size | ran | reconstructed symbol | src file | original file | conf. |
|---:|---:|:-:|---|---|---|---|
| 248 | 320 |  | `std::iter_swap<md::CRespostaConsulta*> (std::sort helper)` | library/inlined helper: std::sort of consulta answers (28-byte elements) | library | high |
| 591 | 18 |  | `comum::md::CUeComumMdError::CUeComumMdError (merged ctor thunk)` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comment) | uenux2/src/app/comum/md/cabrangencia.h | high |
| 675 | 211 | ✓ | `api::CApplicationContextGuard::~CApplicationContextGuard` | src/uenux2/src/api/gui/capplicationcontextstack.u35.cpp | uenux2/src/api/gui/capplicationcontextstack.cpp | high |
| 857 | 38 |  | `comum::md::estadoaplicacao::CDadoCorrespondencia::~CDadoCorrespondencia` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comment) | uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocorrespondencia.cpp | high |
| 948 | 15 |  | `comum::CLogComum::GetInst` | src/uenux2/src/app/comum/log/clogcomum.cpp (u24) | uenux2/src/app/comum/log/clogcomum.cpp | high |
| 1047 | 635 |  | `comum::IEventosLog::LogaGeracaoRelatorio` | src/uenux2/src/app/comum/log/ieventoslog.u35.cpp | uenux2/src/app/comum/log/ieventoslog.cpp | medium |
| 1080 | 609 | ✓ | `comum::asn::Utils::ConverteDataHoraJE` | src/uenux2/src/app/comum/asn/util.u35.cpp | uenux2/src/app/comum/asn/util.cpp | medium |
| 1221 | 268 | ✓ | `std::map<std::string,std::string>::__emplace_hint_unique` | library/inlined helper: map<string,string> insert with hint | library | high |
| 1374 | 12 | ✓ | `ecourna::api::util::CStringUtils::Trim(const std::string&)` | src/ecourna/api/util/cstringutils.u35.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | medium |
| 1387 | 169 |  | `comum::CRelUtil::IncluiSeparador` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | medium |
| 1391 | 84 |  | `comum::CJustificador::GetInst` | src/uenux2/src/app/comum/justificativa/cjustificador.cpp (u24) | uenux2/src/app/comum/justificativa/cjustificador.cpp | high |
| 1405 | 32 |  | `std::__tree<map<md::ETipoAbrangencia,SQtdeAptos>>::destroy` | library/inlined helper: recursive tree destroy (trivial values) | library | high |
| 1406 | 72 | ✓ | `comum::(anonymous)::GetInstCriada<T> (merged singleton GetInst body)` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp | (merged by wasm-opt; one copy per GetInst) | medium |
| 1485 | 14 |  | `comum::EhFaseTreinamento` | src/uenux2/src/app/comum/appinfo/cappinfo.u35.cpp | uenux2/src/app/comum/appinfo/cappinfo.cpp | medium |
| 1501 | 318 | ✓ | `vota::CAssinadorVota::CAssinadorVota` | src/uenux2/src/app/vota/comum/cassinadorvota.cpp | uenux2/src/app/vota/comum/cassinadorvota.cpp | high |
| 1552 | 254 |  | `std::pair<const char, ecourna::app::dados::CLabelParametrizado>::pair` | library/inlined helper: pair ctor (char + {int, 4 strings}) | library | high |
| 1555 | 49 |  | `std::filesystem::path::string` | library/inlined helper: returns a copy of the native string | library | high |
| 1874 | 82 |  | `std::vector<char>::vector(const vector&)` | library/inlined helper: vector<char> copy ctor | library | high |
| 1880 | 1037 | ✓ | `ecourna::api::util::CStringUtils::Split` | src/ecourna/api/util/cstringutils.u35.cpp | ecourna-lib/ecourna/api/util/cstringutils.cpp | medium |
| 1881 | 171 |  | `comum::CRelUtil::CompletaDireita` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | low |
| 1920 | 300 |  | `std::__sort4<..., md::CRespostaConsulta*>` | library/inlined helper: std::sort | library | high |
| 1921 | 1346 |  | `comum::CRelUtil::FormataQtdAptos` | src/uenux2/src/app/comum/relatorios/crelutil.u35.cpp | uenux2/src/app/comum/relatorios/crelutil.cpp | medium |
| 1922 | 83 |  | `std::function<comum::SQtdeAptos()>::function(const function&)` | library/inlined helper: std::function copy ctor | library | high |
| 1933 | 95 | ✓ | `comum::CLocal::GetLocalID` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | high |
| 1935 | 9 |  | `comum::CEleitores::GetQtdHabilitadosPorBiografia` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | medium |
| 1948 | 15 |  | `comum::CPath::GetPathChaves` | src/uenux2/src/app/comum/cpath.u35.cpp | uenux2/src/app/comum/cpath.cpp | medium |
| 2188 | 76 |  | `ASN1::CHOICE::operator=(const CHOICE&)` | library/inlined helper: CHOICE copy-assignment | library (III ASN.1 runtime) | medium |
| 2221 | 23 |  | `comum::CRegistradorMesario::Existe` | src/uenux2/src/app/comum/comparecimentomesario/cregistradormesario.u35.cpp | uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp | medium |
| 2264 | 203 |  | `comum::CEleitores::Procura` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | medium |
| 2266 | 28 |  | `comum::CEleitorDetalhe::PodeVotar` | src/uenux2/src/app/comum/dados/celeitordetalhe.u35.cpp | uenux2/src/app/comum/dados/celeitordetalhe.cpp | medium |
| 2729 | 22 |  | `comum::CPedeTituloMesario::GetInst` | src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp (u22) | uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp | high |
| 2742 | 15 |  | `comum::CCodecWsq::GetInst` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp | uenux2/src/app/comum/reconhecimentobiometrico/ccodecwsq.cpp (?) | low |
| 2769 | 45 | ✓ | `std::vector<char>::__vallocate` | library/inlined helper | library | high |
| 2790 | 1123 |  | `std::__insertion_sort_incomplete<..., md::CRespostaConsulta*>` | library/inlined helper: std::sort | library | high |
| 2806 | 57 |  | `std::__tree<std::string>::destroy` | library/inlined helper: set<string> node destroy | library | high |
| 2816 | 158 |  | `comum::CLocal::GetQtdAgregadas` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | high |
| 2821 | 9 |  | `comum::CEleitores::GetQtdHabilitadosPorBiometria` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | medium |
| 2822 | 132 |  | `comum::CEleitores::GetQtdCompareceramSemBiometria` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | medium |
| 2823 | 138 |  | `comum::CEleitores::GetQtdAptosSecao` | src/uenux2/src/app/comum/dados/celeitores.u35.cpp | uenux2/src/app/comum/dados/celeitores.cpp | medium |
| 2885 | 895 |  | `comum::CCabecalhoQRCode::~CCabecalhoQRCode` | src/uenux2/src/app/comum/u35-foreign-fragments.cpp (comment) | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | high |
| 3844 | 21 |  | `comum::CAppStateContext::CAppStateContext` | src/uenux2/src/app/comum/cappstate.h | uenux2/src/app/comum/cappstate.h (inferred) | high |
| 3867 | 403 |  | `comum::CParteCandidatosProporcionais::~CParteCandidatosProporcionais` | src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | high |
| 5741 | 1295 | ✓ | `comum::CLocal::GetTodasSecoes` | src/uenux2/src/app/comum/dados/clocal.u35.cpp | uenux2/src/app/comum/dados/clocal.cpp | high |
| 5806 | 154 | ✓ | `comum::CCandidaturasDSSexo::operator()` | src/uenux2/src/app/comum/dados/ccandidaturas.u35.cpp | uenux2/src/app/comum/dados/ccandidaturas.cpp | high |
| 5823 | 124 |  | `comum::CGravadorLog::~CGravadorLog` | src/uenux2/src/app/comum/gravadores/cgravadorlog.cpp | uenux2/src/app/comum/gravadores/cgravadorlog.cpp (inferred) | high |
| 5872 | 12 |  | `comum::CCopiadorMR::~CCopiadorMR (deleting)` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferred) | high |
| 5873 | 606 |  | `comum::CCopiadorMR::CopiaDaMV` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferred) | low |
| 5874 | 606 |  | `comum::CCopiadorMR::Copia` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferred) | medium |
| 10267 | 265 |  | `comum::asn::CConversorHashArquivo::DoConverte` | src/uenux2/src/app/comum/asn/cconversorhasharquivo.u35.cpp | uenux2/src/app/comum/asn/cconversorhasharquivo.cpp (inferred) | high |
| 10276 | 241 |  | `comum::asn::CConversorHistoricoVotoImpresso::DoConverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.cpp | uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.cpp (inferred) | high |
| 10290 | 321 |  | `comum::asn::CConversorCarga::DoDesconverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp | uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp (inferred) | high |
| 10291 | 477 |  | `comum::asn::CConversorCarga::DoConverte` | src/uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp | uenux2/src/app/comum/gravadores/asn/cconversorcarga.cpp (inferred) | high |
| 11204 | 12 |  | `comum::CParteEleitores::~CParteEleitores (deleting)` | src/uenux2/src/app/comum/relatorios/cparteeleitores.cpp | uenux2/src/app/comum/relatorios/cparteeleitores.cpp (inferred) | high |
| 11206 | 75 |  | `comum::CParteEleitores::Imprime` | src/uenux2/src/app/comum/relatorios/cparteeleitores.cpp | uenux2/src/app/comum/relatorios/cparteeleitores.cpp (inferred) | high |
| 11207 | 119 |  | `comum::CParteCargos::~CParteCargos (deleting)` | src/uenux2/src/app/comum/relatorios/cpartecargos.cpp | uenux2/src/app/comum/relatorios/cpartecargos.cpp (inferred) | high |
| 11208 | 116 |  | `comum::CParteCargos::~CParteCargos` | src/uenux2/src/app/comum/relatorios/cpartecargos.cpp | uenux2/src/app/comum/relatorios/cpartecargos.cpp (inferred) | high |
| 11210 | 10 |  | `comum::CParteCandidatosProporcionais::~CParteCandidatosProporcionais (deleting)` | src/uenux2/src/app/comum/relatorios/cpartecandidatos.u35.cpp | uenux2/src/app/comum/relatorios/cpartecandidatos.cpp | high |
| 11222 | 1130 |  | `comum::CGeradorRelVersaoPacoteDados::MontaDados` | src/uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp | uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (inferred) | low |
| 11241 | 16 |  | `comum::CGeradorBUQRCodeVota::DadosEleicao` | src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u35.cpp | uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp | medium |
| 11273 | 4 |  | `comum::md::CRegraIdentidadeLivre::GetTipo` | src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp | uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp (inferred) | high |
| 11274 | 49 |  | `comum::md::CRegraIdentidadeLivre::Formata` | src/uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp | uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp (inferred) | medium |
| 11361 | 173 |  | `comum::asn::CConversorSituacoesEleicoes::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorsituacoeseleicoes.cpp (inferred) | high |
| 11377 | 2542 |  | `comum::asn::CConversorDetalheConsulta::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalheconsulta.cpp (inferred) | high |
| 11384 | 218 |  | `comum::asn::CConversorHorarioVerao::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.u35.cpp | uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.cpp | high |
| 11386 | 86 |  | `comum::asn::CConversorNumViasImpressasRelatorios::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp (inferred) | high |
| 11387 | 44 |  | `comum::asn::CConversorNumViasImpressasRelatorios::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.cpp (inferred) | high |
| 11388 | 143 |  | `comum::asn::CConversorLocalidadeEleitoral::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.cpp (inferred) | high |
| 11399 | 594 | ✓ | `comum::asn::CConversorDadoCorrespondencia::DoConverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp (inferred) | high |
| 11400 | 1107 | ✓ | `comum::asn::CConversorDadoCorrespondencia::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.cpp (inferred) | high |
| 11404 | 274 | ✓ | `comum::asn::CConversorDadoLocal::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.u35.cpp | uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp | high |
| 11413 | 167 |  | `comum::asn::CConversorSeguranca::DoConverte` | src/uenux2/src/app/comum/asn/cconversorseguranca.cpp | uenux2/src/app/comum/asn/cconversorseguranca.cpp (inferred) | high |
| 11437 | 2893 |  | `comum::asn::CConversorCabecalhoPacote::DoDesconverte` | src/uenux2/src/app/comum/asn/cconversorcabecalhopacote.u35.cpp | uenux2/src/app/comum/asn/cconversorcabecalhopacote.cpp | high |
| 11441 | 922 |  | `comum::asn::CIndexadorFotos::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.cpp | uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.cpp (inferred) | high |
| 11448 | 1570 | ✓ | `comum::asn::CConversorCandidatura::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp | uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp | high |
| 11450 | 3280 | ✓ | `comum::asn::CConversorDadosCandidato::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.cpp | uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.cpp (inferred) | high |
| 11453 | 1694 | ✓ | `comum::asn::CConversorSecaoEleitoral::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.u35.cpp | uenux2/src/app/comum/dados/asn/cconversorsecaoeleitoral.cpp | high |
| 11456 | 116 |  | `comum::asn::CConversorIdentificacaoAgregada::DoConverte` | src/uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.u35.cpp | uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp | high |
| 11460 | 290 |  | `comum::asn::CConversorPartido::DoConverte` | src/uenux2/src/app/comum/dados/asn/cconversorpartido.u35.cpp | uenux2/src/app/comum/dados/asn/cconversorpartido.cpp | high |
| 11496 | 257 |  | `comum::CRdvPosicionadorVota::Posiciona` | src/uenux2/src/app/comum/dados/crdvposicionadorvota.cpp | uenux2/src/app/comum/dados/crdvposicionadorvota.cpp (inferred) | medium |
| 11569 | 176 |  | `comum::CServicoEstadoGeral::GetPathArquivo` | src/uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp | uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp (inferred) | high |
| 11580 | 90 |  | `comum::CGravadorVersoesArquivos::~CGravadorVersoesArquivos (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp | uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.cpp | high |
| 11581 | 87 |  | `comum::CGravadorVersoesArquivos::~CGravadorVersoesArquivos` | src/uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.u35.cpp | uenux2/src/app/comum/gravadores/cgravadorversoesarquivos.cpp | high |
| 11583 | 10 |  | `comum::CGravadorLog::~CGravadorLog (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorlog.cpp | uenux2/src/app/comum/gravadores/cgravadorlog.cpp (inferred) | high |
| 11585 | 61 |  | `comum::CGravadorRDV::~CGravadorRDV (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorrdv.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (inferred) | high |
| 11586 | 14 |  | `comum::CGravadorRDV::~CGravadorRDV` | src/uenux2/src/app/comum/gravadores/cgravadorrdv.cpp | uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (inferred) | high |
| 11591 | 10 |  | `comum::asn::CConversorCodigoCargoConsulta::DoDesconverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp (inferred) | high |
| 11592 | 84 |  | `comum::asn::CConversorCodigoCargoConsulta::DoConverte` | src/uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp | uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.cpp (inferred) | high |
| 11621 | 100 |  | `comum::CGravadorEnvelopeArquivo::~CGravadorEnvelopeArquivo (deleting)` | src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (inferred) | high |
| 11622 | 97 |  | `comum::CGravadorEnvelopeArquivo::~CGravadorEnvelopeArquivo` | src/uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp | uenux2/src/app/comum/gravadores/cgravadorenvelopearquivo.cpp (inferred) | high |
| 11625 | 15 |  | `comum::IGravadorEnvelope::~IGravadorEnvelope` | src/uenux2/src/app/comum/gravadores/igravadorenvelope.cpp | uenux2/src/app/comum/gravadores/igravadorenvelope.cpp (inferred) | high |
| 11630 | 1207 |  | `comum::IGravador::CopiaParaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferred) | medium |
| 11631 | 608 |  | `comum::IGravador::CopiaResultadoParaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferred) | medium |
| 11632 | 608 |  | `comum::IGravador::CopiaParaResultado` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferred) | medium |
| 11633 | 438 |  | `comum::IGravador::GravaMV` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferred) | medium |
| 11634 | 438 |  | `comum::IGravador::Grava` | src/uenux2/src/app/comum/gravadores/igravador.cpp | uenux2/src/app/comum/gravadores/igravador.cpp (inferred) | medium |
| 11638 | 12 |  | `comum::CCopiadorMR::~CCopiadorMR` | src/uenux2/src/app/comum/gravadores/ccopiadormr.cpp | uenux2/src/app/comum/gravadores/ccopiadormr.cpp (inferred) | high |
| 11900 | 12 |  | `comum::CGeradorRelPU::~CGeradorRelPU (deleting)` | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (inferred) | high |
| 11901 | 12 |  | `comum::CGeradorRelPU::~CGeradorRelPU` | src/uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp | uenux2/src/app/comum/relatorios/cgeradorrelpu.cpp (inferred) | high |

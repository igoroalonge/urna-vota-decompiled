# Investigation (October 2026): the reconstructed VOTA against real 2026 urna data and TSE's public documents

*[Versão em português](LEIAME.md)*

This report compares the reconstruction in this repository with three kinds of public evidence about
the 2026 general election (1st round, 4 October 2026): the files that real urnas wrote and TSE
publishes (BU, RDV, logs, signature files), TSE's open-data tables and their official field
documentation, and TSE's public documents about formats and procedures. It was written on
2026-10-07. Every new result and every disagreement was re-run independently by a second reviewer,
and a sample of the confirmations was spot-checked; where the second pass corrected a first-pass
figure, the corrected figure is the one given here.

Nothing here comes from TSE beyond what TSE publishes. No raw TSE file is included in the repository
(see §9).

## Short answer

* **The recovered data model is exact for every 2026 result-file variant TSE published.** 135/135
  real BU files (`bu.dat`/`busa.dat`) and 135/135 `rdv.dat` decode and re-encode byte for byte with
  `src/asn1`, including RED-recovered, SA (ballot-paper) and contingency files. TSE's own 2026
  `bu.asn1` equals `src/asn1` on 41/41 structured types (B1, B2, B22, H6, H7).
* **Every integrity link the code implies holds on real data.** The vote hash chain of 260 eleições
  (15,611 tuples) recomputes with the code's format strings; every BU vote line is a pure function of
  the RDV (635 cargo lists); 260/260 BU signatures and 405/405 file signatures verify over
  SHA-512(hash) with per-urna hardware certificates: Ed521 on UE2020/2022, ECDSA P-521 on
  UE2013/2015 (B4, B5, B8, B9, H2).
* **TSE's published 2026 hash lists are tied to code in this repository.** Every "HASH GERAL" (10 PC
  lists, 112 per-UF urna values, read from a third-party copy of TSE's page) is the chain computed by
  `CMontadorHash::CalculaHashGeral`, over the files in the directory order of `CriaHashesDiretorio`
  and the scope of `CGravadorHashes`. One VOTA binary serves all four urna models, and 42 RHVoice
  data files on the urna are byte-identical to the simulator's (A1, A2, A5).
* **15 findings contradict statements in TSE public documents**, by the code, by real 2026 files, or
  by a sibling TSE document: five on the 2022 log-format document, the stale enumerations of the 2026
  `rdv.asn1`, the 2024 signature-file spec, four on the open-data field documentation (one of them,
  D2, covers two leiames), the mesário manual's 15 s fingerprint timeout, and three provisions of
  Res.-TSE 23.751/2026 that the urna does not implement as written (§1). A sixteenth, the QR
  character cap, is pending (H5).
* **The 2026 QR code matches the code as far as public material allows.** The generator rebuilds the
  2026 QR manual's worked example byte for byte (from a third-party transcription), and the
  certificate-QR split gives 2 QRCE codes for every one of the 110 real urna certificates. The code's
  1,100 − 277 character reserve is too small for 2026 signatures: rebuilt with the code's rules,
  13/110 real BUs would print a last QR of 1,102–1,239 characters (H1, H3, H5, G10).
* **The real logs show where the reconstruction is incomplete.** 183 of 191 VOTA log templates are
  produced by `src/`; 8 are missing (printer driver, "Votação suspensa", a mesário-terminal display
  error); the key-off shutdown path is missing; two log calls carry the wrong severity (C1, C2, C3,
  C18).
* **Vote counting follows Res.-TSE 23.751/2026 on every point checked**: panel order,
  legenda/nulo/inapto, repeated senator, transit scope, suspension, hours. The mesário terminal
  enforces less than the written procedure: the release "by the mesário's fingerprint" accepts any
  finger (550 of 1,605 real releases could not be attributed to a registered mesário), suspension and
  closing accept any título with valid check digits, and mesário registration can be skipped (4/110
  sections opened with nobody registered) (F1–F3, F6, F10–F17).
* **The secret key files are per UF, not per urna.** All 17 key files of a UF are identical on the
  four urna models, and the key-encryption secret works across models within a UF (medium
  confidence) (A4, G4).
* **TSE's published urna logs carry poll workers' título numbers**: 440 distinct títulos in 110/110
  sampled sections. Failed keypad tests are logged as "Sucesso" (at least 27 times, in 21/110
  sections) (G1, G2).
* **Open-data pitfalls.** The numeric `CD_CARGA_URNA_*` columns lose leading zeros in 9.3% of AC
  codes, which breaks joins and hash-chain checks; the datasets mix municipal local time and Brasília
  time without saying so (D3, D5).
* **The media-preparation chain GEDAI-UE → urna → BU holds for 62 AC urnas.** The BU records the PC
  that made the load medium, not the voting medium, and the código de carga is created by the urna
  (E3, E8, E9).
* **RDV order leaks nothing** (votes are sorted by type and typed digits), but 3,410 votes in 108/110
  sections of S110 carry a (vote type, typed digits) pair that is unique in their section and office
  (B10, G12).

## Sources and provenance

All downloads and reads were made on 2026-10-07.

| kind | what | where | weight |
|---|---|---|---|
| Official TSE data, downloaded directly | Per-section urna files of the 2026 1st round (pleito 3220): `aux.json` index, `-bu.dat`/`-busa.dat`, `-rdv.dat`, `-log.jez`/`-logsa.jez`, `-vota.vsc`/`-sa.vsc`; per-UF section lists | `https://resultados.tse.jus.br/oficial/ele2026/arquivo-urna/3220/dados/<uf>/<mun>/<zona>/<seção>/…` and `…/3220/config/<uf>/<uf>-p003220-cs.json` | Highest. Primary data; the BU, RDV and log files carry urna signatures, and all of them verified (B5). |
| Official TSE open data, downloaded directly | BU na Web (`bweb_1t_<UF>_…`), correspondências efetivadas (`CEFT_1t_<UF>_…`), correspondências esperadas por seção and de contingência (`CESP_1t_<UF>_…`, files `csec_…`/`ccont_…`), GEDAI-UE logs (`log_gedai_1t_<UF>_…`); each zip with its `.sha512` and its official `leiame` PDF | `https://cdn.tse.jus.br/estatistica/sead/eleicoes/eleicoes2026/{buweb,correspefet,correspesp,logsgedai}/…`; the 2024 GEDAI logs under `…/eleicoes2024/logsgedai/`; located through the CKAN API of `https://dadosabertos.tse.jus.br` | Highest for the data. The leiames are TSE's official field documentation and are quoted verbatim (from `pdftotext`; line numbers refer to that text). |
| Upstream software projects | RHVoice 1.14.0 configuration, Brazilian-Portuguese language data, Letícia-F123 voice | `https://github.com/RHVoice/RHVoice`, `https://github.com/RHVoice/Brazilian-Portuguese`, `https://github.com/RHVoice/leticia-f123-pt-br` | High (byte comparison only). |
| TSE document, official copy on a TRE site | Manual do Mesário 2022 (TSE) | `https://static.tre-al.jus.br/portal/eleitor/mesarios/tre-al-manual-do-mesario-tse-versao-web-2022.pdf` | High for the 2022 wording, which is the only mesário manual text verified word for word. |
| TSE documents, third-party copies | 2026 "Formato dos arquivos de BU, RDV e assinatura digital" package: README dated 2026-09-10, `bu.asn1`, `rdv.asn1`, `assinatura.asn1`, diagrams, scripts, and a QR verifier with demonstration data | `https://github.com/fernandysson/resultados-eleicoes-2026` (commit 80d8f36) | Medium-high. Its specs decode the real files byte for byte (H6, H7); the QR verifier's authorship is unknown, but the results that use it rest on independent cryptographic checks. |
| | 2026 QR manual ("manual do QR code no boletim de urna", Aug. 2026, 36 pp.): a third party's notes and a transcription of its small worked example | `https://github.com/fabricioluna/apuracaojuntos` (`tests/fixtures/bu-2026.json`, commit 664a111) | Medium. The transcription's internal hash chain verifies, which rules out transcription errors in the hashed text; the notes are paraphrase. |
| | 2024 format package v2 (`bu`/`rdv`/`assinatura.asn1`, scripts, CA certificates in `mr_util.py`), 2024 QR manual, 2022 "Formato dos arquivos de log" | `https://github.com/doccaz/urnas-br` (commit a87fbee6) | Medium-high. The 2024 package matches the TSE zip kept in the same repository. |
| | "Resumos digitais (hashes) das Eleições 2026" listings: urna systems for UE2013/2015/2020/2022 (`path1u13/u15/u20/u22`), PC applications (`pc1*`), other systems (`itnm1*`), verification media (`hash_tse/oab/mp/confea`) | A third-party HTML and text copy of TSE's page; the address of the copy was not recorded. TSE's page is listed under "could not be read" below. | Medium-high. Every list is internally consistent (each HASH GERAL recomputes, A1), and its counts and names equal those the earlier analysis read from TSE directly (A16). The individual digests were not cross-checked against TSE. |
| | Res.-TSE 23.751/2026 (atos gerais do processo eleitoral, Eleições 2026), 283 articles | `https://jurishand.com/resolucao-tse-23751-de-26-fevereiro-2026`; the official page is `tse.jus.br/legislacao/compilada/res/2026/resolucao-no-23-751-de-26-de-fevereiro-de-2026` (found by search, not readable) | Medium. Two phrases were cross-checked word for word by exact-phrase search. |
| Search-engine extracts | 2026 Manual do Mesário, Guia Rápido and the "Mesários Brasil" course; statements attributed to the 2026 QR manual (1,100-character cap, EdDSA/ECDSA); TSE descriptions of AVPART and SAVP; Res. 23.759/2026 phrases; the 2022 log document's SipHash description | various | Low. Always marked "search extract (unverified wording)". No contradiction in §1 rests on a search extract alone. |
| Could not be read | `www.tse.jus.br` answers 403 (Akamai) to the analysis machine, and its tools report the host as blocked: the 2026 "Log do ecossistema" (`…/eleicoes/eleicoes-2026-content/arquivos/log-do-ecossistema/@@display-file/file/log-do-ecossistema.pdf`), the 2026 QR manual PDF itself (`…/eleicoes/eleicoes-2026-content/arquivos/manual-do-qr-code-no-boletim-de-urna/@@display-file/file/manual-do-qr-code-no-boletim-de-urna.pdf`), the 2026 hashes page (`…/eleicoes/urna-eletronica/seguranca-da-urna/hash/resumos-digitais-hashes-das-eleicoes-2026-1o-e-2o-turnos`), `qrcodenobu.tse.jus.br`, `justicaeleitoral.jus.br` and TRE pages with the 2026 mesário material; `web.archive.org` was rate-limited | — | None. |
| Code | The reconstruction in `src/`. The wasm binary itself was not on the analysis machine. | this repository | As good as the reconstruction (see README_EN.md, "What this is not"). |

**Samples.** The same labels are used throughout.

| label | content |
|---|---|
| S110 | 110 sections of the 1st round, about 4 per UF including ZZ (abroad): BU, RDV, log and signature file of each. 106 come from a seeded random draw of 4 sections in each of the 28 UFs (6 of the 112 draws have no published files, all aggregated sections); 4 were added by name (3 abroad, 1 in AC; see §9). Their 110 logs hold 622,567 lines. |
| S135 | S110 plus 25 targeted sections chosen from TSE's CEFT tables across UFs: 7 RED-recovered BUs (one of them on a contingency urna), 6 SA (ballot-paper) BUs abroad, 6 contingency urnas and 6 urnas replaced on election day. 135 BU files, 135 RDV files, 135 signature files. |
| S129 | S110 plus the 19 non-SA sections of the targeted set (used where `bu.dat` is required). |
| AC open data | All 2,270 AC sections in bweb, CEFT, CSEC, CCONT and `aux.json`; the 22 official AC GEDAI-UE logs (30,023 lines, 454 sessions); urna files of 62 AC urnas (5 from S110, 57 fetched) for the preparation chain, and of the 10 AC contingency sections. |
| other open data | CEFT for AM, DF, MT, PE, RR and ZZ (49,930 rows over 7 UFs with AC); bweb for ZZ (1,309 sections); urna files of 2 ZZ SA sections and 1 PE RED section; the RR 2026 and AC 2024 GEDAI-UE log bundles; 3 AC 2024 urnas. |

**How much weight each kind carries.** A statement backed by downloaded TSE data and by the code is
given as fact. A statement that needs a document read through a third-party copy says so; where
the copy decodes real files byte for byte (the 2026 format package) or verifies cryptographically
(the QR example), it is treated as reliable in substance, but its exact wording should be re-checked
against TSE when `www.tse.jus.br` is reachable. A statement backed by a search extract is marked
"search extract" and is used only as context. Inferences are labelled as such.

## How to read

* **Ids.** A1…H18 refer to the appendix. The letter is the line of work: A file system and keys
  (hash lists), B result files and ASN.1, C real logs, D open-data tables and their documentation,
  E GEDAI-UE media preparation, F election rules against the code, G code-level inferences tested on
  data, H TSE's public 2026 documents.
* **Kinds.** *confirmation* (a prediction of the code that the data confirm), *new_inference*
  (something learned that the code or the earlier documentation did not state), *mismatch* (two
  sources disagree), *open_question*.
* **Verdicts.** Each new_inference, mismatch and open_question was re-run independently.
  *confirmed*: reproduced as stated. *confirmed with corrections*: reproduced, with corrected numbers
  or wording; the corrected version is the one in this report. *refuted*: the claim does not hold.
  *unverifiable*: a prediction that no public artefact can test yet. Confirmations were spot-checked
  rather than re-run one by one: *spot-checked* (re-run, agrees), *spot-checked, corrected* (re-run,
  a count changed) or *not re-checked*. *Corrected by review* marks the one confirmation (A12) that
  the final cross-check of all findings found partly wrong.
* **Code references** are repository paths, `src/…:line`. `func N` is a wasm function index (see
  README_EN.md).
* **Privacy.** No título, CPF, staff number, person's name, computer name, TPM or installation
  serial, urna serial or media serial is printed. Certificate subject names are masked as
  `UEAO########`. Sections are named only where nobody's conduct is at issue; the procedural lapses
  of §5 and §6 are given as counts.

## 1. Public documents that disagree with the code or the real data

### 1.1 Summary table

| document and section | what it says (quoted) | what the code or the real 2026 data show | ids |
|---|---|---|---|
| TSE, "Formato dos arquivos de log", Sept. 2022, p. 1 (third-party copy) | "Ao ser extraído da urna, ele é empacotado num arquivo em formato 7z" | ZIP. The code packs the log with `CZip` (`src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp:120-121`). 110/110 urna `-log.jez`, 22/22 outer and 22/22 inner GEDAI `.jez` start with the ZIP signature `50 4b 03 04`. | H9 |
| same document, line layout | "(campos delimitados por TAB): [Data] [Hora] [Severidade do Evento] [ID_UE] [Aplicativo] [Mensagem] [MAC]", "Data – … AAAAMMDD", "Hora – … HHMMSS" | 622,567/622,567 real lines have 6 TAB fields; the first is `DD/MM/YYYY HH:MM:SS` with a space inside. The document's own regular expression matches every line and its examples print the same date-time form; only its prose describes seven fields. | H10 |
| same document, "Categorias de severidade do evento" | INFO, ALERTA, EXTERNO, ERRO | A fifth value, `WHAT`, in 3 GAP lines. VOTA itself emits only INFO, ALERTA and ERRO. | H11 |
| same document, desktop logs | "O log dos aplicativos deskotp possui o mesmo formato utilizado pela urna" | The 2026 GEDAI-UE logs have 4 TAB fields (no urna id, no application), CRLF line ends; 0 of 30,023 lines match the document's regular expression. | H12 |
| same document, result-media log file name | "[fase][ppppp]-[MMMMM][ZZZZ][SSSS].[ext]", extension `logjez` | The code and all 110 real files use `<fase><pleito:5><uf><mun:5><zona:4><seção:4>-log.jez`, e.g. `o03220ac0107400040042-log.jez` (`src/uenux2/src/app/comum/carquivossavd.u22.cpp:44`, `:191`). TSE's 2026 README agrees with the code. | H13 |
| TSE 2026 format package, `rdv.asn1` (third-party copy) | `urnaChegouAposInicioVotacao (5)`, `reservaSecao (4)`, `reservaEncerrandoSecao (6)` (file byte-identical to 2024) | The same package's 2026 `bu.asn1`, and `src/asn1/ModuloTiposResultadosEcoUrna.asn:98`, `:135-136`: `urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)`, `contingenciaEncerrandoSecao (6)`. Same numbers, different meaning for 5. | H8 |
| TSE 2024 format package v2, `assinatura.asn1` (third-party copy) | `EntidadeAssinaturaEcourna ::= SEQUENCE { modeloEquipamento ModeloEquipamento, assinaturaSW EntidadeAssinatura, assinaturaHW EntidadeAssinatura }`; key fields untagged | Fails on 110/110 real 2026 signature files. Real layout: `SEQUENCE { SW, HW, SEQUENCE { model, algorithm } }` with `[0]`/`[1]` tags. TSE's 2026 `assinatura.asn1` describes it and decodes 110/110. | B3, H6 |
| BU na Web leiame, l. 205-208 | `CD_CARGA_1_URNA_EFETIVADA` "O conjunto dos primeiros 19 caracteres…"; `CD_CARGA_2_URNA_EFETIVADA` "O conjunto dos últimos 6 caracteres…" | 24 and 7 characters in all 258,052 AC rows and all 12,035 ZZ rows: `GetIDCargaFormatado` split after character 24 (`src/uenux2/src/app/comum/relatorios/crelutil.cpp:191-198`). | D2 |
| correspondência efetivada leiame, l. 73-75 | `CD_CARGA_2_URNA_EFETIVADA` "…dos 06 últimos dígitos do código da carga original da urna esperada…" | Built from the efetivada code in 303/303 rows where the two codes differ (7 UFs). | D2 |
| leiames of CEFT, CSEC, CCONT (column definitions and the 18 + 6 digit split) | "Código da carga original da urna …, enviado pelo GEDAI" | Written as unquoted numbers, losing leading zeros: 210/2,270 AC codes (9.3%); MT and PE have codes with only 19 digits. The urna requires exactly 24 digits (`src/uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp:29-33`) and the BU hash chain uses all 24. | D3 |
| preamble of every 2026 leiame | "Os campos estão entre aspas e separados por ponto e vírgula, inclusive os campos numéricos"; "Campos preenchidos com #NULO … O correspondente para #NULO nos campos numéricos é -1" | Unquoted columns: bweb 19, CEFT 11, CSEC 7, CCONT 5. Null markers actually used: `#NULO#`, empty strings, and quoted `"-1"`. | D21 |
| GEDAI log leiame, l. 16 | "UF: unidade federativa onde ocorreu a eleição (composto por 02 dígitos)" | Two letters (`ac`) in all 22 file names. | D22 |
| Manual do Mesário 2022, l. 729-730 (search extract: same sentence in 2026) | "ELEITOR NÃO RECONHECIDO. Essa mensagem também é mostrada caso a pessoa demore mais de 15 segundos para posicionar o dedo no sensor." | 30 s on the first attempt, 15 s on later ones (`src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85`). Real timeouts: first attempt n=152 at 30–37 s; later attempts n=476 at 15–22 s. | F4 |
| Res.-TSE 23.751/2026 art. 127 VII | the relatório "Eleitores Não Reconhecidos Biometricamente", "contendo a quantidade e o título eleitoral das eleitoras e dos eleitores que não foram habilitados por biometria" | The urna emits "Eleitores com habilitação biográfica" (BEHB, `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp:130-150`). Under the literal reading it leaves out the voters without registered biometrics released by birth year (1,424 in the 106 biometric BUs of S110). | F5 |
| Res.-TSE 23.751/2026 art. 123 | "Emitida a Zerésima e, antes do início da votação, a presença das mesárias e dos mesários será registrada no Terminal do Mesário." | The code lets registration end with nobody registered (`src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp:45-47`, `:76-77`); 4/110 sections opened with none. | F6 |
| Res.-TSE 23.751/2026 art. 210 XII | the Boletins de Urna contain the number of voters "a) habilitados por identificação biométrica; b) habilitados por identificação biográfica; e c) sem biometria cadastrada" | Non-biometric urnas omit the counts (`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:179-181`): 4/110 BUs, all abroad. | F7 |
| TSE QR-code manual 2024 (format 1.5), verified; the 2026 manual per search extract only | "Cada QR Code está limitado a 1.100 caracteres, incluindo todas as três seções." | The code reserves 277 characters (`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:277`); the 2026 last-QR tail needs 422–436. Rebuilt with the code's rules, 13/110 real BUs give a last QR of 1,102–1,239 characters. **Pending**: a contradiction only if the 2026 manual keeps the cap. | H5 |

### 1.2 The evidence, item by item

**Log package format (H9).** The 2022 document (header "Tribunal Superior Eleitoral / Secretaria de
Tecnologia da Informação / Seção de Voto Informatizado") says the urna log "é empacotado num arquivo em
formato 7z", and the same for desktop applications. The code writes the urna log package with `CZip`
(minizip): `temp.jez` is written and then renamed to the `-log.jez` result name. The 7z writer is
linked into the binary but unused. All 110 urna `-log.jez` files of S110 are ZIP (108 hold only
`logd.dat`, 2 also an archived `.jez`), and so are the 22 outer and 22 inner GEDAI-UE packages. The
2022 edition was written when result logs were called `.logjez`; a 2026 edition ("Log do
ecossistema") is listed on TSE's 2026 page but could not be read.

**Line layout (H10).** The 2022 prose defines seven TAB-separated fields with `AAAAMMDD` and `HHMMSS`.
All 622,567 lines of S110 have six: `DD/MM/YYYY HH:MM:SS` (one field), severity, an 8-digit
zero-padded urna id, program, message, and a 16-character upper-case hexadecimal code. The
document's own regular expression joins date and time with `[\s]`, which accepts a space, so it
matches 100% of the lines; its examples print `DD/MM/YYYY HH:MM:SS` too, but also a 7-digit urna id
where the regular expression demands 8. VOTA cannot settle the layout: it passes only (application,
severity, message), and the daemon that adds the other fields (`logd`) is not in the binary. Lines
by program: VOTA 566,963, GAP 33,538, ATUE 10,260, SCUE 9,329, LOGD 2,214, INITJE 263.

**Severities (H11).** The 2022 document lists four categories. By severity, S110's lines are INFO
620,331, ALERTA 2,218, ERRO 13, EXTERNO 2 (LOGD, as documented) and WHAT 3. The three WHAT lines come
from GAP, each right after a GAP ERRO line, and print a C++ exception text with function signature and
source line (C15). In `src/`, VOTA and the shared code log only with severities 1, 2 and 3.

**Desktop logs (H12).** The 22 official AC GEDAI-UE logs (30,023 lines) have four TAB fields
(date-time, severity, message, 16-hex code), no urna id and no application field, CRLF line ends, and
0 lines that match the 2022 regular expression. Their package names differ from the 2022 rule, but
that part is documented by TSE's own 2026 GEDAI leiame
(`gedai-ue-UF-PLEITO-oficial-NM_COMPUTADOR-SERIAL_TPM-SERIAL_INSTALACAO-log.extensão`), so for naming
the 2022 text is superseded rather than contradicted.

**Result-media log name (H13).** The 2022 rule `[fase][ppppp]-[MMMMM][ZZZZ][SSSS].logjez` matched the
2022 files. The code's prefix `{:c}{:05}{}{:05}{:04}{:04}-` (fase, pleito, UF, município, zona,
seção) plus the suffix `log.jez` is what all 110 real 2026 names follow, and TSE's 2026 README
(`fpppppuuMMMMMZZZZSSSS-suf.ext`; "log.jez … Arquivo compactado de LOG em formato texto") agrees with
the code. Archived logs inside a package still follow the 2022 rule for archived files
(`<urna id><DDMMAAAAHHMMSS>-01.jez`). This is an outdated edition rather than an error.

**Stale `rdv.asn1` (H8).** The 2026 package's `rdv.asn1` is byte-identical to the 2024 v2 file
(SHA-256 `282dcde0c7ba1f1b…a465` for both) and keeps `urnaChegouAposInicioVotacao (5)`,
`reservaSecao (4)` and `reservaEncerrandoSecao (6)`. The 2026 `bu.asn1` in the same package has
`urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)` and `contingenciaEncerrandoSecao (6)`,
the names `src/asn1` recovered from the binary's type information. Both headers list
`tiposresultadosecourna.asn1` among the aggregated sources, and the 2026 RDV class diagram also keeps
the old names while the BU diagram was updated. Decoding is unaffected (same numbers), but
`MotivoApuracaoMistaComBU = 5` reads "arrived after voting started" in one TSE file and "closed with
voters still in line" in the other. The package was read from a third-party copy; the unchanged
diagram makes a stray file unlikely.

**2024 signature spec (B3, H6).** The published 2024 `assinatura.asn1` fails on all 110 S110
signature files at `EntidadeAssinaturaEcourna.modeloEquipamento`. The real 2026 files are
`SEQUENCE { SW signature, HW signature, SEQUENCE { model, x } }`; the SW signature ends in a `[0]`
field (the key-set tag) and the HW signature in a `[1]` field (the certificate), in 135/135 files.
The pairs (model, x) over S135 are (22, 4) 45, (20, 4) 55, (15, 2) 29 and (13, 2) 6. TSE's 2026
`assinatura.asn1` names the trailer `OrigemAssinaturaHardware { modeloEquipamento,
algoritmoAssinatura }` and the key field `InfoChave ::= CHOICE { tagChaves [0], certificadoDigital
[1] }`, and re-encodes 110/110 files. So the 2024 text is superseded, not wrong for its year; the
layout change itself was already noted in analysis chapter 12-real-urna-2026 §3. That x is the
hardware signature algorithm (2 ecdsa, 4 eddsa) fits every file, but in this sample x is also fully
determined by the model family, so the data cannot tell the two readings apart.

**CD_CARGA lengths and a copy-paste slip (D2).** The BU na Web leiame (shipped inside
`bweb_1t_<UF>_051020261403.zip`) gives 19 and 6 characters. Every AC and ZZ row has 24 (18 digits
and 6 dots, `ddd.` six times) and 7 (`ddd.ddd`). Together they are the urna's printed
`GetIDCargaFormatado` string (31 characters) cut after character 24; the second part is the urna's
printed "RESUMO DA CORRESPONDÊNCIA" (D1). The 19/6 figures fit neither characters (24/7) nor digits
(18/6). The CEFT leiame gets 18 and 6 digits right, but describes `CD_CARGA_2_URNA_EFETIVADA` as
coming from the "urna esperada"; in every row where the expected and actual codes differ (AC 10,
AM 51, DF 28, MT 35, PE 145, RR 4, ZZ 30) the column comes from the efetivada code.

**Codes that lose their leading zeros (D3).** The CEFT, CSEC and CCONT files write
`CD_CARGA_URNA_*` unquoted, as numbers. AC digit counts (22/23/24): CEFT efetivada 20/190/2,060,
CEFT esperada 20/191/2,059, CSEC 20/191/2,059, CCONT 11/55/510; so 210 of 2,270 efetivada codes
(9.3%) lost at least one zero. Some MT and PE codes have only 19 digits. The GEDAI logs print 24
digits in every correspondence line, bweb's formatted `CD_CARGA_1` keeps the zero, and the
urna rejects anything but 24 decimal digits (errors 8021/8022). The BU's hash-chain seed formats the
code as `{:24}` (`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:62`), so on the
two decoded AC BUs whose code starts with 0, the first tuple hash of both eleições (4/4)
recomputes from the CEFT value only after zero-padding it to 24 digits. Users of the open data must
pad with zeros before joining or verifying.

**Quoting and null conventions (D21).** The preamble promises quoted fields "inclusive os campos
numéricos" and `#NULO` / `-1` for missing data. The files have 19 unquoted columns in bweb (for
example `CD_MUNICIPIO`, `QT_*`, `NR_URNA_EFETIVADA`), 11 in CEFT (including `CD_CARGA_URNA_*`), 7 in
CSEC and 5 in CCONT. Missing values appear as `#NULO#` with a trailing `#` (for example
`DS_SECOES_AGREGADAS` in 243,808 AC rows), as empty strings (CEFT `TP_DIVERGENCIA` in 2,260 AC rows,
ZZ opening and closing times in 111 rows, ZZ expected-urna fields in 28 rows) and as the quoted
string `"-1"` (junta and turma). The bweb leiame itself uses `#NULO#` once. Low impact, but it is
the root cause of D3.

**UF "dígitos" (D22).** The GEDAI log leiame (whose PDF entry is dated 2024-10-05, i.e. reused) says
the UF part of the file name is "composto por 02 dígitos". It is two letters in all 22 names. Its
"PLEITO … composto por 05 dígitos" is right (`03220`), as in the code's `{:05}`.

**Fingerprint timeout (F4).** The code arms 30,000 ms for the first attempt and 15,000 ms for the
following ones (`src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85`,
comment at `:177`). In S110's logs, first-attempt timeouts (`Timeout de reconhecimento do dedo.
Tentativa [1]`) come 30–37 s after the request (median 32, n=152) and later ones 15–22 s after
(median 17, n=476). The verified 2022 manual says 15 s; a search extract attributes the same
sentence to the 2026 manual. Harmless for voters, but the published procedure does not describe the
first attempt.

**Name and scope of the biometric report (F5).** Art. 127 VII names the report "Eleitores Não
Reconhecidos Biometricamente" and defines it as the voters "que não foram habilitados por
biometria". The urna's report is titled "Eleitores com habilitação biográfica" (`behb.dat`, printed
"via única"), and the 2026 mesário material (search extract) also calls it BEHB. It is generated in
106/110 sections; the other 4 are the non-biometric urnas abroad. Its rows are the voters released by
birth year plus mesário fingerprint (habilitação type 2: 1,605 voters, equal to the BUs'
`qtdEleitoresHabilitadosPorBiografia`); that the rows are type 2 only rests on a reconstruction
comment (`src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp:192-200`). Under the
resolution's literal wording the 1,424 voters without registered biometrics released by birth year
(their BU count) would also belong in it. Under the "não reconhecidos" reading the scope matches and
only the name differs.

**Registration of the mesários (F6).** The prompt "Registrar mesário?" accepts CORRIGE ("Cancelar")
and moves on with nothing to stop the opening of the vote. Mesários registered between the
election-day zerésima and "Urna pronta para receber votos", per section of S110: 4 in 82 sections,
3 in 17, 2 in 5, 1 in 1, 5 in 1, none in 4. In all 4 sections with none, mesários were registered
later during voting; 27 sections registered someone during voting. A mesa member was necessarily
present to print the zerésima, so in these 4 sections the step of art. 123 was not done before
voting, which the code permits. The parágrafo único allows late arrivals to register during voting.

**Enablement counts abroad (F7).** `CGravadorBU` fills `detalhamentoComparecimento` only
`if (m_urnaBiometrica)`; the field is `[1] … OPTIONAL` in `src/asn1/ModuloBoletimUrna.asn:37`, and
the printed BU likewise prints the three counts only on biometric urnas
(`src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp:66-70`). 106 BUs of S110 have the field and 4
do not: exactly the 4 sections abroad, whose 1,036 voters were all released by birth year. A formal
gap only: for these urnas the values would be (0, 0, comparecimento).

**The 1,100-character QR cap (H5), pending.** The code reserves 277 characters for the non-content
part of each QR: `limite = tamanhoMaximo − 277` with `tamanhoMaximo = 1100`
(`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:277`;
`src/uenux2/src/app/comum/relatorios/cgeradorbu.cpp:135`). In format 6.0 the last QR's fixed part is
`QRBU:i:n VRQR:6.0 ` (18) + ` HASH:` and 128 hex (134) + ` ASSI:` and the hex signature (270 for a
132-byte Ed521 signature, 284 for a 139-byte ECDSA one), 422–436 characters. The last QR can
therefore reach 823 + 422 = 1,245 characters (1,259 with ECDSA) and passes 1,100 whenever its
content slice is longer than about 678 (664) characters. Even the 2024 format 1.5 needed 300 > 277,
although no 2024 QR passed 1,100 (the longest had 1,073). Rebuilding the 110 BUs of S110 with the
code's rules gives 13 last QRs over 1,100 (1,102–1,239). The count depends on the unknown width of
the `PROC` field (12 with 1–3 digits, 13 with 4–5, 14 with 6) and omits `AGRE`, which is not in
`bu.dat`; 11 of the 13 exceed by 30 or more characters. Nothing in the code rejects a long QR: the
image check only requires width + 4 ≤ 400 modules. The verified cap is in the 2024 manual, which
describes format 1.5. The 2026 manual's wording is known only from search extracts, so this becomes
a contradiction if, and only if, the 2026 manual keeps the cap. No 2026 printed BU was available to
look at.

### 1.3 Open-data documentation: undocumented conventions, not contradictions

These points do not contradict a sentence of the leiames, but a reader who follows the leiames alone
would misread the data.

| topic | what the leiame says | what the data and the code show | ids |
|---|---|---|---|
| `CD_TIPO_URNA` (bweb) | "1: Apurada; 2: Não apurada; 3: Anulada e apurada em separado; 4: Anulada; e 5: Não instalada" | An apuração status, unrelated to the BU's `TipoUrna {secao 1, contingencia 3, contingenciaSecao 4, contingenciaEncerrandoSecao 6}`. All 2,270 AC and 1,309 ZZ sections are 1, including 10 contingency and 28 SA sections. The number 3 means different things in the two. The urna's own type is visible only indirectly (CEFT `TP_DIVERGENCIA` 'C' plus `CD_ORIGEM_VOTO`). | D4 |
| time zones | a zone is given only for `HH_GERACAO` ("com base no horário de Brasília") | Local time of the município: `DT_ABERTURA`, `DT_ENCERRAMENTO`, `DT_EMISSAO_BU`, `DT_CARGA_*`, CEFT `DT_RECBTO_URNA_EFETIVADA`. Brasília time: bweb `DT_BU_RECEBIDO`, CEFT `DT_RECBTO_URNA_ESPERADA`. The two reception columns of one CEFT record differ by exactly +2 h in 2,260/2,260 AC rows, +1 h in MT and RR, 0 in DF, −1 h on Fernando de Noronha, +1 or +2 h in AM, and by each city's offset abroad (Tokyo −12 h, Wellington −15 h). In ZZ, 515/1,309 sections look "received before emission". | D5 |
| `QT_ELEI_BIOM_SEM_HABILITACAO` | "Quantidade de eleitoras e eleitores com biometria, mas que não foram habilitados por meio dela" | Equal to the BU's `qtdEleitoresHabilitadosPorBiografia` (15/15 AC BUs; never to `…SemBiometria`). Where the BU has no detalhamento (all abroad), bweb prints 0, so "no data" and "0 voters" look the same. | D10, B13 |
| `QT_APTOS`, `QT_COMPARECIMENTO` | "aptos a votar", "que compareceram às eleições" | Per eleição, as in the BU: in 57/2,270 AC sections the presidential eleição has 1–55 more aptos (out-of-state transit voters), and comparecimento differs in 48. | D18, B11 |
| `DS_SECOES_AGREGADAS` | a list separated by "/" | Not in `bu.dat` at all; the urna prints the list `{:04}` space-separated and puts it in the QR as `AGRE:` with dots. bweb uses a third format. | D19 |
| `CD_ORIGEM_VOTO`, `TP_DIVERGENCIA` | "Código da origem dos votos" (no values listed); D, C, O | U = `votacaoUE`; R ("BU gerado por RED em urna de seção") = `votacaoRED`; C ("Sistema de apuração") = `saManual`. Only '' and 'C' occur in 49,930 CEFT rows of 7 UFs; 'C' covers both contingency urnas and SA BUs. | D12 |
| "urna efetivada" for SA BUs | "a urna que foi utilizada na eleição, podendo ser a urna original ou uma outra urna de contingência" | For SA BUs it is the urna on which the apuração system ran: all 29 ZZ rows of origin C share one urna, one carga and one load medium. CEFT writes `NM_MUNICIPIO` "BUENOS AIRES" on all 29 whatever `CD_MUNICIPIO` says, and bweb lists 28 of the 29. | D13 |
| contingency correspondences | no section columns | As in `IdentificacaoContingencia`. Contingency urnas served sections outside the município (6/10) or zona (1/10) they were prepared for, which the leiames do not mention. | D14 |
| one expected and one actual urna per section (CEFT) | — | The model cannot represent intermediate contingency urnas; they appear only in the BU/RDV carga history (one AC section has 3 cargas from 3 urnas). | D6 |
| bweb "código da carga original da urna" | — | This is the efetivada urna's own carga, which is what the leiame says ("original" qualifies the carga, not the urna). The first-pass claim that it is misleading was **refuted**. | D7 |
| 2026 format README, fase | "f é a fase (s para simulado e o para oficial)" | The code also uses `t` (treinamento), which TSE's own 2022 log document lists. | H16 |
| 2024 QR manual, signatures | Ed25519, "a biblioteca OpenSSL", one key pair per UF | Superseded rather than contradicted: TSE's 2026 README says Ed521 (PEM certificate) for UE2020/2022 and ECDSA (ASN.1 certificate) for the other models; the data agree and show one certificate per urna. | H2 |

## 2. What the real 2026 data confirms about the reconstruction

The reconstruction was built from a development build of the simulator (`10.23.0.1 -
DESENVOLVIMENTO`). The 2026 urnas ran `10.23.0.0 - Praia da Barra do Cahy`. The checks below test
predictions of the reconstructed code against what those urnas wrote.

| what the code predicts | what the real data show | ids |
|---|---|---|
| The ASN.1 modules in `src/asn1` describe the result files exactly | 135/135 BU files (envelope and inner `EntidadeBoletimUrna`) and 135/135 RDV files of S135 decode and re-encode byte for byte, including RED, SA and contingency variants; every CHOICE alternative of `IdentificacaoUrna`, `DadosSecaoSA` and `Eleicoes` occurs in a real file that round-trips | B1 |
| `rdv.dat` is written without an envelope (`CGravadorRDV`, func 11587) | 0/135 RDV files decode as `EntidadeEnvelopeGenerico`; all 135 decode as a bare `EntidadeResultadoRDV` | B2 |
| TSE's 2026 specs and `src/asn1` agree | 2026 `bu.asn1` = `src/asn1` on 41/41 structured types, including the three enum changes after 2024 that the earlier analysis predicted from the binary (`envelopeZeresimaImpressa (6)`, `urnaEncerradaComEleitoresNaFila (5)`, `contingenciaSecao (4)`); 2026 `assinatura.asn1` has the binary's `ModeloEquipamento {tpm20, ue2013, ue2015, ue2020, ue2022}` | H7 |
| No coded value outside the code's enumerations | None found in S135. Unseen but defined: `TipoUrna` 6, `TipoArquivo` 3/4/6, RDV vote types 5/8/9, `cargoSemCandidato`, consultas, municipal offices, phases simulado/treinamento, `tpm20` | B22 |
| The vote hash chain of `src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:62-72` (three format strings) | Recomputed for all 260 eleições of S135 (15,611 tuples); `ultimoHashVotosVotavel` equals the last tuple hash in every one | B8 |
| BU lines are computed from the RDV with the type table at `@546352` and the rules of `CGravadorBU` (no zero lines; nominal, branco, nulo, then legenda; per-cargo numbering) | 635/635 cargo lists recomputed exactly (534 in S110, 101 in the targeted set); no zero line among 15,611; comparecimento = first-cargo votes / number of choices in every case; nominal party = first two digits in 11,007/11,007 lines | B9 |
| `aptos` keyed by abrangência, comparecimento per eleição | The per-eleição split is needed: in 2 of S110 and 2 of the targeted sections the presidential eleição has more aptos and voters than the state one | B11 |
| BU and RDV of a contingency urna identify different things (`src/uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp:20-22` and `src/uenux2/src/app/comum/u18-foreign-fragments.cpp:144-146`) | 10/10 contingency BUs carry `identificacaoContingencia` with the contingency load's município and zona while their RDV carries the section; the correspondência's local is the constant 1 in 119/119 BU and 129/129 RDV section identifications | B14, G9 |
| One `agora` for all result files (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:115`) | Envelope, BU and RDV headers are equal in 110/110; the BU generation time equals the log's `[bu.dat] + [Início]` line in 108/110 (2 are 1 s earlier) | B23 |
| Signed-file order is the writer order of `CGravaResultado`, with WSQ files only on biometric urnas | 106 signature files list 11 files in exactly that order, 4 (abroad) list 8 without WSQ files; `log.jez` is last although its SAVD id is lower | B6 |
| The log is packed before the result files are signed and copied | 110/110 published logs end with `Gerando arquivo de resultado [log.jez] + [Início]`; the signing-session lines (`Inicia uma sessão no MSE/MSD`) never appear | B7, G13 |
| `CGravadorLog` packs `logd.dat` plus every archived log in `std::map` order | 135/135 archives: archived members sorted by name, `logd.dat` last; all members deflate | B19, C16 |
| Signature verification as TSE's scripts describe | 135/135 hardware certificates chain to TSE's CA certificates; 260/260 BU eleição signatures and 405/405 file signatures verify over SHA-512 of the hash; 0/260 over the raw hash | B5 |
| The 2026 QR generator (VRQR 6.0, no VRCH, header order, hash chain, 823-character slices) | The 2026 manual's small worked example (third-party transcription) is rebuilt byte for byte; both HASH fields recompute | H1 |
| Certificate QR split `ceil(2·len/1082)` (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:256`) | Matches TSE's demonstration QR data exactly; gives 2 QRCE codes for all 110 real urna certificates | H3 |
| QR cargo order by `ordemImpressao`, `IDEL` on each election change; `TOTC` = total votes | `IDEL 6 7 5 3 IDEL 1` in 102/110 BUs (DF: `6 8 5 3`, abroad: `1`); `TOTC = 2 × COMP` for Senador in 106/106 | H14, H15 |
| The HASH GERAL chain and directory traversal of `CMontadorHash`/`CGravadorHashes` | Reproduces every HASH GERAL of the 2026 hash lists: 10/10 PC lists, 112/112 per-UF urna values | A1 |
| One build for four urna models, model decided at run time (`IUrna::GetModelo()`) | One `vota.of` digest in all four urna lists; MSD on UE2013/2015 and MSE on UE2020/2022 in the hardware self-test logs (19/19 and 91/91) | A2, A12 |
| Log texts of VOTA | 183 of 191 coarse VOTA templates are produced by `src/` (86 exact, 92 `std::format`, 5 composed); call-site severity (1 → INFO, 2 → ALERTA) equals the observed level for 136 templates | C1, C3 |
| Constants of the monitor, biometrics, inspection, hours and power code | Monitor every 1,800 s (2,200/2,200 intervals); voltage logged only on models ≥ 2020; inactivity alert at 45 s (1,074/1,074); 4 fingerprint attempts with 30/15 s timeouts; inspections every U(60, 90) minutes, never inside a voter session; opening strictly after the start time; battery shutdown at 1,800 s; power-event cap 20; mesário limit 6; fingerprint score threshold 20 (720 matches ≥ 20, 2,828 non-matches ≤ 19) | C6–C11, G5–G7 |
| Suspension logic (`src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:329-370`) | Per-cargo `Voto confirmado` lines equal the RDV's confirmed votes; null-after-suspension votes (11 Presidente, 10 Governador, 7 Senador) equal the log lines; the first office never gets one; 11/11 partial suspensions fill every remaining choice, including the second Senate seat | B12, C12, F15 |
| Habilitação kinds | Rebuilt from the log, they equal the BU's detalhamento in 101/106 domestic sections; the 5 differences are 3 voters suspended before confirming any vote (enabled, not counted as attendance) and 2 parser artefacts at a cross-thread log reversal (the raw line counts match there) | C13, B13 |
| Version string is a build constant | `10.23.0.0 - Praia da Barra do Cahy` in VOTA, RED and SA files and in the logs of SCUE, GAP, ATUE and VOTA | B20 |
| The voting rules (§5) | Panel order in 24,523 + 1,116 (DF) complete sessions; 736 repeated-senator nulls; transit voters outside their UF get only Presidente; no release before 08:00 Brasília; closing refused before 17:00 | F10–F12, F17 |

## 3. Where the reconstruction is incomplete or wrong

| # | issue | evidence | where to fix | ids |
|---|---|---|---|---|
| 1 | Two log calls have the wrong severity. `Título {} é inválido/válido para suspender a votação` is logged with the one-argument `Loga` (severity 1, INFO); line 123 contradicts its own comment (`api_f1398` is `LogaAviso`, `src/uenux2/src/app/vota/log/clogvota.h:51`). | 29/29 real lines (15 "inválido", 14 "válido", 13 sections) are ALERTA. | `src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp:123` and `:130`: use `LogaAviso` (severity 2). | C3 |
| 2 | The key-off shutdown continuation is missing. `SaiPorVotacaoSuspensa` logs the key line, stops the threads, starts `std::async`, and the comment says the rest is not in the binary. | Of the 250 VOTA key-off lines, 249 are followed by `Desligando a urna` → `Finalização de aplicativo`, always in that order (the other one by a printer-jam line while the BU was printing). `Votação suspensa` comes first in the 3 key-offs between the election-day zerésima and the closing, and in none of the 246 before the zerésima. | `src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp:194-207` (comment at `:203-205`); `MostraMensagemDesligamento` (`:287-291`) is empty. The opposite order at `src/uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp:53-54` is a different path (wrong clock) and is not an error. | C2 |
| 3 | 8 VOTA log templates (919 lines) have no literal anywhere in `src/`: `Início de impressão` (452), `Fim de impressão` (446), `Impressora atolada. Aguardando ação do operador` (5), `Operador confirmou que concluiu a manutenção na impressora` (4), `Detectada impressora com serial …` (4), `Impressora OK` (4), `Votação suspensa` (3) and one ERRO `Não foi possível exibir imagem no LCD do terminal do mesário: Erro (12) exibindo imagem no microterminal.` | The only printer in `src/` is the web null printer (`src/uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp`); `CInfoMTLCD::MostrarImagemNoLCD` has no error path. The comparison is with `src/`, not with the wasm; that the 6 new templates are absent from the wasm too is inferred. | Document them: README_EN.md "What this is not", the u22 chapter (`src/uenux2/src/app/comum/cinfomtlcd.cpp:40-49`) and the u08/u26 chapters for printing and shutdown. No reconstructed site exists to correct. | C1 |
| 4 | Method bodies were not reconstructed for some of the most frequent lines, so their severity and arguments cannot be read from `src/`: `Voto confirmado para [{}]` (154,959 lines), `Atribuido voto nulo por suspensão [{}]` (28), `Aguardando confirmação para emissão da zerésima` (111), `Áudio desativado pelo fim da votação` (67). | All 155,165 lines are INFO. `Gerando relatório [{}] [{}]` and `Gerando arquivo de resultado [{}] + [{}]` are reconstructed (`src/uenux2/src/app/comum/log/ieventoslog.u35.cpp:20`, `src/uenux2/src/app/comum/log/clogcomum.cpp:75-76`). | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:438`, `:440` (funcs 4537, 4541); `src/uenux2/src/app/vota/eleitor/iniciovotacao/estadosiniciovotacao.u34.h:46`; `src/uenux2/src/app/vota/operador/aguardaeleitor/cdesabilitaaudioeleitor.u34.cpp:39` and `src/uenux2/src/app/vota/operador/comum/ccancelahabilitacaoeleitor.cpp:20`. | C18 |
| 5 | The id in data-file names is called `pleito` but is the processo eleitoral (`idPE`). | GEDAI logs `Processo eleitoral registrado: 01219`, `Pleito corrente: 03220`; `-el`, `-imp`, `-se`, `-tte`, `-mz`, `-mme`, `-cp` packages carry 01219 (2024: 00439); `-cm`/`-ste` carry the pleito; 323 package names in three bundles fit the grammar. | `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h:36` (`uedword pleito`); comment in `src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp:120-132`. | E11 |
| 6 | `-pu` is described as "partidos". It is the urna parametrization (`EntidadeParametrizacaoUrna`); parties are `-pa`. | The repository's own decoding (`docs_en/data-model/asn1-schemas.md`); real media carry one national `o00000br-pu`. | `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h:7`, `src/uenux2/src/app/comum/nomearquivo/cnomearquivo.cpp:80`, `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:176`. | E12 |
| 7 | The comment says func 7787 reads a UF-level `-pu.dat`; no real one exists. | Only `o00000br-pu.jez` in AC 2026, RR 2026 and AC 2024; the 62 AC urnas verify only `o00000br-pu.vsc`. | `src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp:7`; `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:175-176`; `docs_en/modules/u11-ecourna-lib-ecourna-api-asn.md:354`. Open question: how 7787 handles the missing file. | E13 |
| 8 | The certificate variable is called `der`, but on UE2020/2022 the token returns a 1,034-byte PEM text. | Real UE2020/2022 certificates are PEM (91/91), UE2013/2015 DER (19/19); TSE's demo QR prints PEM halves. | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:252`. | H3 |
| 9 | The comment says the 277-character reserve covers the header, HASH and ASSI; in format 6.0 it does not (422–436 are needed). | §1.2, H5. | `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:276`; the chapter sentences listed in §7(a). | H5 |
| 10 | Comments tie the generator `simulador-votacao-ng` / 64 × '0' to `eg.bin`. The `eg.bin` correspondence (and so a simulator BU) carries `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` is only in the `infomidia-fv-*-t.dat` fixtures, from which VOTA reads only the serial. | `src/uenux2/mock/app/comum/cappinfobuilder.u30.cpp:76-82` (func 9952), `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:246-251`. | `src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h:17-21`; `docs_en/modules/u40-lib-ecourna-classes-without-known-file.md:335`. | E4 |
| 11 | The key name `wsq.pk1` is marked as inferred (`?`). | `/dsk/fi/estatico/chave/wsq.pk1` exists for every UF in all four urna lists (strong support; `bio.sk1` is the only other 7-character key name). | `src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:71-75`; `docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md:356`. | A3 |
| 12 | The SAVD package table has `dadoscarga.vsu` on `/dsk/fe`; every real GAP run checks `/dsk/fi/estatico/dadoscarga.vsu`. | 494/494 GAP runs. The table was read back from the live object, so this is what the wasm holds; the table lists the MV copy of `-lo.vsc` too, which GAP checks on both media. | Add a note at `src/uenux2/src/app/comum/carquivossavd.u22.cpp:103`; open whether it is a slip or names the MV copy. | A10 |
| 13 | `MDUE` is read from an offset marked `+44 (?)`; that it is the 4-digit model year (2020, not the enum 20) is an inference. | `EUrnaModelo` values are years (`src/uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.h:8`, `src/uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocarga.cpp:64-67`); no 2026 printed BU was available. | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp:266`. | H4 |
| 14 | That the BEHB report lists only type-2 releases rests on a reconstruction comment. | 106/110 sections generate it; its gate is `QtdHabilitados(2)`. | `src/uenux2/src/app/vota/eleitor/u38-foreign-fragments.cpp:192-200`. | F5 |
| 15 | Object layouts and sizes in the chapters (for example a 12-byte `std::string`) are those of the wasm32 libc++ build; the real urna runs 32-bit x86 glibc with GCC libstdc++ and links `libecourna.so` dynamically. | File names in the 2026 urna lists (A15). | A note in README_EN.md or `docs_en/modules/README.md`. | A15 |
| 16 | Branches for models 2009–2011 are dead in 2026: the urna lists exist only for UE2013/2015/2020/2022. | A2. | A note in the u23 chapter (its table item 14). | A2 |

## 4. New things learned about the real urna

### 4.1 System image and the official hash lists

TSE publishes, for each election, SHA-512 digests (Base64) of the installed files of every system,
each list ending in a "HASH GERAL". For 2026 there is one list per urna model (`path1u13`,
`path1u15`, `path1u20`, `path1u22`), with 135/136/140/140 base files plus 17 key files for each of
28 UFs (27 states and ZZ).

* **The HASH GERAL is the code's own hash chain (A1).** `CMontadorHash::CalculaHashGeral`
  (`src/uenux2/src/api/hash/cmontadorhash.cpp:88-101`) computes G1 = H1 and Gk =
  Base64(SHA-512(G(k−1) ‖ Hk)), concatenating the Base64 texts. This reproduces all 10 PC and other
  lists in their listed order and 112/112 per-UF urna values, when the chain runs over the base
  files in listed order and then over the UF's 17 key files. The base order is exactly
  `CriaHashesDiretorio`'s traversal (in each directory, files first in byte order, then
  subdirectories; `:144-145`, recursion at `:161`); plain sorting by path gives 0/112. The scope is
  that of `CGravadorHashes` (`src/uenux2/src/app/comum/gravadores/cgravadorhashes.cpp:62-73`): the
  root without `/dsk`, then `/dsk/fi/estatico/chave/`. The per-urna files `uenux.cfg` and
  `avbootcfg.vsu` are excluded, as `foraDoHashGeral` does
  (`src/uenux2/src/api/hash/cmontadorhash.cpp:148`). Caveat: in this build the accumulated value is
  never read and is not written to any file, so the urna's own `hash.dat` holds only the per-file
  entries; that its scope equals the published list is inferred.
* **One VOTA binary for all four models (A2).** The union of base paths is 173; 123 are identical in
  all four lists, including `/uenux/bin/vota.of`, every program in `/uenux/bin`, `libecourna.so`, the
  kernel and the C library. The 50 that differ are kernel modules (7/8/11/11 `.ko` files per model),
  the hardware layer `libapihwil*`, the MSD library (UE2013/2015) versus the mesário-terminal
  libraries `libapimtpos`/`libapimtss` (UE2020/2022), and the signature files covering them. The
  code's model-dependent branches are run-time branches, as in the reconstruction.
* **User space (A15).** By file name: `/lib/ld-linux.so.2` (the 32-bit x86 glibc loader; no 64-bit
  loader in any list), glibc, `libgcc_s` and GCC `libstdc++`, plus TSE shared libraries
  (`libecourna.so`, `libapigeneric`, `libapiio_resources`, `libapisdk`, `libapiui`, and the per-model
  `libapihwil*`). The wasm instead links libc++/musl and ecourna statically.
* **Graphics and resources (A14).** No `/uenux/app`, `/resource`, font file, Qt6Svg or SVG plugin is
  installed. Qt6 runs on `linuxfb` with only the GIF and JPEG image plugins, which are exactly the
  formats the code requests (12 GIFs, 23 JPEGs, one PNG, which QtGui reads natively, and one WAV).
  The simulator's SVG icons and its variable font are web-only. That the resources live in
  `libapiio_resources.so` is inferred from the name.
* **RHVoice (A5).** All 20 Brazilian-Portuguese language files on the urna equal upstream commit
  `910c889` (2023-10-29; no other commit matches all 20); all 21 Letícia-F123 voice files
  (`voice.info`, `voice.params`, `16000/*`) equal the voice's upstream; `/etc/RHVoice/RHVoice.conf`
  equals the configuration of RHVoice tag 1.14.0; the core library is named
  `libRHVoice_core.so.1.14.0`. The earlier analysis had matched the same upstream files to the
  simulator's package, so 42 urna data files are identical to the simulator's. The simulator's 8
  placeholder files are absent: they are web-only.
* **Boot-time signature checks (A18).** GAP verifies a fixed series of signature files at every
  start. Over 110 logs there are 494 runs with 32 (6 runs), 33 (119), 34 (354) or 35 (15) steps, all
  16,680 checks "SUCESSO". Steps 1–24 are fixed: `avusrbin.vst` twice, `avboot.vst`,
  `avbootcfg.vsu`, `avbin.vst`, `avetc.vst`, five RHVoice files, `avpart90/91/97/99.vst`,
  `avmod.vst`, `avmodue.vst`, `avlib.vst` and three Qt plugin files, `avusrlib`/`avusrlibue.vst`,
  and the UF key signature `avusrchave.vst`. Then come the section's `-lo.vsc` on the internal
  medium, the voting medium's `infomidia.vsc` and `-lo.vsc` (0–2 checks), `dadoscarga.vsu`, `-cp`,
  `-pu`, two `-ste` and one to three `-ce` packages. Step count = 30 + checks on the voting medium +
  number of `-ce` packages.
* **The four signing entities (A6).** `/uenux/bin/avpart90.vst`, `91`, `97` and `99` are on every
  model. The verification-media lists name one key each: `90.pub` (OAB), `91.pub` (MP), `97.pub`
  (CONFEA), `99.pub` (TSE), so 90/91/97/99 are those entities' codes. That each `avpart9N.vst` is
  entity N's signature over the urna software is inferred from the numbers. GAP checks the four in an
  order that is constant within each urna (110/110 logs with several runs) but varies between urnas
  (23 of the 24 permutations occur). The GEDAI-UE log names the corresponding result-media
  application "Sistema Externo de Auditoria e Verificação - AVPart"; other expansions of AVPART found
  by search are unverified wording.
* **An unnamed signature file (A7).** `/uenux/bin/avusrbinof.vst` is in all four lists, next to
  `avusrbin.vst`, but no line of any program names it: GAP's steps 1 and 2 both name `avusrbin.vst`
  (494/494). Either step 2 checks `avusrbinof.vst` under the wrong label or the file is not checked;
  the logs cannot tell. Its link to the official-phase binaries `vota.of`/`sa.of` is inferred from
  the name.
* **Installation packages (A8, A9, A17, A20).** A real official installation checks
  `jez/avgm.vsc`, `jez/vota_apl.vst`, `jez/vota_ofi.vst` and `jez/avpart_ofi.vst` on the load medium
  (110/110 logs; the step numbers vary: 10–12 in 102 logs, 12–14 in 4, 6–8 in 4), so `vota_apl` is
  installed together with the phase package `vota_ofi`, not as a phase variant. The `.jez` container
  is a plain ZIP (no member of the 110 result logs is encrypted); whether GEDAI's application
  packages are also encrypted cannot be told from digests. During installation `/dsk/fe` is the load
  medium, which has its own `chave/` directory; afterwards it is the voting medium. SCUE, the
  installer, runs from the load medium and is not in the urna lists. Installed programs match the
  code's application vocabulary (`vota.of`, `sa.of`, `red`, `vpp`, `adh`, `atue`, `ste`, `gap`,
  `savd`, `logd`); only the `.of` variants are installed for an official load.
* **Shared code with other urna programs (A13, C4).** SCUE's seven signature labels in the logs ("EG
  Geral MI", "EG GAP 1 MI", "EG VOTA MI", "EG SA MI", "EG RED MI", "Tabela de correspondência",
  "UENUX CFG") are entries of VOTA's SAVD name table
  (`src/uenux2/src/app/comum/iinterfacesavd.u36.cpp`). SCUE and ATUE share the start-up lines
  (`Iniciando aplicação - {}`, `Versão da aplicação: {}`) with the reconstructed code, and ATUE also
  two `IInterfaceInit` lines (`Tamanho da MR: {}`, `Urna desligada a pedido do aplicativo`). Their
  power messages use other wording ("Urna operando com rede elétrica" and "Bateria interna com carga
  plena" in SCUE and GAP, "Operando com a rede elétrica" in ATUE) than VOTA's `CLogComum`, so that
  class is VOTA-side (inferred from wording).
* **The Windows "Votação" application (A11).** The PC list `pc1vota` is a Qt6 desktop application
  (`votacao.exe`, WebView2 loader, no network or TLS plugin) and a sibling of GEDAI-UE and Sorteio.
  Search results describe a "SAVP-Votação" used in the integrity test (unverified wording). It is not
  a build of the web simulator. Its only files equal to urna files are `dependencias.properties` and
  `versoes.properties`, the ASN.1 contract catalogue. The 2026 Transportador ships contract jars
  versioned `20260601173148`, the `TAG_CONTRATOS` compiled into this VOTA build
  (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:86`). That the urna's own file
  carries this tag is inferred; only its digest is public.
* **What the lists do not cover (E19).** GEDAI-UE checks `gedai-ue.vst` and `app.ini.vsc` at every
  start (454/454 sessions) and downloads
  `https://config-integracao.tse.jus.br:443/eleitoral/producao/oficial-ele2026.uris.properties`
  (444 sessions). The GEDAI list contains `gedai-ue.vst`, but no list contains `app.ini.vsc` or the
  official URIs file. Both are probably configuration rather than software; they cannot be audited
  from the published lists.
* **The copy of the lists is consistent (A16).** The copy has the counts and names the earlier
  analysis read from TSE directly (HotSwapFlash 8 digests, GEDAI-UE 70, 75 distinct together, the
  same titles), and its HTML and text forms agree.

### 4.2 Key material

* **Key file names (A3).** The key directory `/dsk/fi/estatico/chave/` and the five key files VOTA
  reads (`cv.ber.pri`, `bu.pk1`, `jufa.pk1`, `wsq.pk1`, `bio.sk1`) are in every UF block of every
  list, byte for byte. This closes the open question on `wsq.pk1`, with one caveat: `bio.sk1` is the
  only other 7-character name the reconstructed lookup could hold.
* **Per-UF and national keys (A4).** Each UF has the same 17 key files, and each UF block is
  identical in all four model lists. Eleven files differ per UF: five private keys (`bio.sk1`,
  `cv.ber.pri`, `log.ber.pri`, `sc.ber.pri`, `ue.ber.pri`), three CEPESC encryption keys stored
  wrapped (`bu.pk1`, `jufa.pk1`, `wsq.pk1`), two public keys (`tse.ber.pub`, `ue.ber.pub`) and the
  signature file `avusrchave.vst`. Six public keys (`pu`, `sc`, `secad`, `secinp`, `seint`, `sevin`
  `.ber.pub`) are national and equal Holocron's `chaves\legal\o<name>.ber.pub`. The `.ber` names map
  onto the code's `ESavdChaveValidar` enum (`src/uenux2/src/app/comum/cpacotearquivos.h:24-26`);
  `sc` = SCUE is inferred. All 110 signature files of S110 carry the same 12-digit key-set tag,
  `202607021654`.
* **The key-encryption secret is shared across models within a UF (G4, medium confidence).** The
  code always deciphers `wsq.pk1`, `bio.sk1`, `bu.pk1` and `jufa.pk1` with AES-256-CBC, key =
  SHA-512(IKernelHSM secret)[16..48), IV = the last 16 bytes
  (`src/ecourna/api/security/csymmetriccipherfactory.cpp:26-37`); the `cifrado` flag is ignored;
  `cv.ber.pri` is deciphered only if marked. In the logs, the `wsq.pk1` path returned an image id on
  UE2013 (1 section), UE2015 (13), UE2020 (46) and UE2022 (39), and voters were enabled by
  fingerprint (which needs `bio.sk1`) on UE2013 2/5, UE2015 13/14, UE2020 47/47 and UE2022 44/44
  sections; 22 UFs (wsq) and 24 UFs (bio) have two or more models succeeding. The same ciphertext
  therefore deciphers on all four models, so the secret is bound neither to the device nor to the
  model; it is per UF medium or national. The `wsq.pk1` evidence assumes that a logged image id means
  the key was deciphered. Consequence: signing keys are per urna, but the Código Verificador key
  (`cv.ber.pri`) and the voter-biometrics key are one per UF. Deciphering `bio.sk1` yields the
  CEPESC secret key; voter templates also need a per-record session key and the CEPESC algorithm.
* **The log line code (C5, A19).** The sixth field of each log line is 16 upper-case hex digits. All
  622,567 are distinct, and 2,767 lines that repeat an earlier line in every other field (mostly
  "Tecla indevida pressionada" in the same second) carry different codes, so the code depends on
  hidden state (a chain, counter or nonce). 1,240 unkeyed or trivially keyed constructions (MD5,
  SHA-1/2/3, BLAKE2, CRC-64, xxHash64, SipHash with guessable keys…) never match on the first lines
  of 3 urna and 2 GEDAI logs (weak negative evidence only). Each UF has a `log.ber.pri`, which VOTA
  never uses; Holocron ships `<app>.log.ber.pri` keys for desktop logs. The reading "a SipHash chain
  keyed by `log.ber.pri`" fits TSE's 2022 description (search extract) and the only MAC primitive in
  the library, `CSiphashMac` (SipHash-4-6, 16-byte key,
  `src/ecourna/api/security/csiphashmac.cpp:26-27,38-39`), but is inference. Without the key, third
  parties cannot check single lines; the published log is protected as a whole by the signature file
  (B5).
* **The encrypted-BU path was off (G8).** `CGravadorBU` writes a CEPESC-encrypted envelope only if
  `m_permiteCifrar && GetCriptografarBU()`
  (`src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:268`). None of the 129 `bu.dat` of S129 has
  the `seguranca` field, and the published `bu.dat` is exactly the file the urna signed, so
  `criptografarBU` was false in the 1st round although `bu.pk1` is shipped to every urna.

### 4.3 Signatures, certificates and the 2026 QR code

* **The signature file (B3, H6, E16).** See §1.2 for the layout. The software signature (SW) is
  uniform: signer `SEVIN`, serial 20260708, key set `202607021654`, CEPESC 256 bits over SHA-512,
  4,765-byte signatures. The GEDAI-UE log signature files use the same SW key set, plus a hardware
  signature by `chaveAssinaturaDesktop` (RSA-2048) with a per-PC certificate issued by
  `CN=Coruscant_Assinatura` (3-year validity, starting 10–23 Sep 2026; its common names end in a NUL
  byte, which strict parsers reject) and the trailer `{tpm20, 1}`. Across 189 files (167 urna, 22
  GEDAI) the trailer's second value equals the hardware algorithm number (RSA 1, ECDSA 2, EdDSA 4),
  but only three combinations occur, each tied to one equipment generation.
* **Hardware signatures and certificates (B4, A12, H17).** S110 has 47 UE2020, 44 UE2022, 14 UE2015
  and 5 UE2013 urnas; the 4 sections abroad use UE2013 (3) and UE2015 (1). UE2020/2022 sign with
  Ed521 (132-byte signatures); their certificate is a 1,034-byte PEM (721–723 bytes as DER) with key
  OID `1.3.6.1.4.1.44588.2.1`, issued by "AC UE2020" or "AC UE2022", subject `UEAO########`.
  UE2013/2015 sign with ECDSA P-521 (139-byte DER signatures) and carry a 944–946-byte DER
  certificate (ecdsa-with-SHA512) issued by "AC URNA", valid 2013–2027 (UE2013) or 2016–2030
  (UE2015), subject `ueao########`. *Correction:* the first-pass statement that UE2013/2015
  signature files carry no certificate (A12) was wrong: its script looked only for PEM; 19/19 carry
  the DER certificate. In 5 of the 19, one or two `0x00` bytes follow the DER. The digits of the
  subject equal the BU's internal urna number in 110/110 of S110 and in 22 of the 25 targeted
  sections; the 3 exceptions are the sections where RED ran on another urna (B18): the certificate
  identifies the urna that signed, which is not always the urna the BU names.
* **What is signed (B5, H2).** The 135 certificates of S135 verify against the CA certificates in
  TSE's `mr_util.py` (AC URNA 35, AC UE2020 55, AC UE2022 45). All 260 eleição signatures
  (`assinaturaUltimoHashVotosVotavel`: UE2013 9, UE2015 51, UE2020 110, UE2022 90) verify over
  SHA-512 of the 64-byte last hash, and 0/260 over the raw hash; the 405 file signatures (bu 129,
  busa 6, rdv 135, log 129, logsa 6) and the 135 self-signatures verify the same way. The code
  passes the raw hash to `IPkcs11::Assina`
  (`src/uenux2/src/app/comum/gravadores/asn/cconversorentidadebu.cpp:83`,
  `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:317-323`), and no `IPkcs11`
  implementation is in the wasm, so the SHA-512 step most likely happens below that interface
  (inference: the data show what is signed, not which layer hashes). For ECDSA this is ordinary
  ecdsa-with-SHA512; only Ed521 has an extra pre-hash. The QR `ASSI` follows the same rule: in TSE's
  demonstration QR data, a 132-byte Ed521 signature (UE2020 certificate) and a 139-byte ECDSA
  signature (UE2015 certificate) verify over SHA-512 of the HASH bytes, not over the raw hash or its
  hex text.
* **The CA certificates.** AC URNA is issued by "AC RAIZ UE", and AC UE2020/AC UE2022 by "AC URNA
  v2"; the certificates in `mr_util.py` are intermediates, not self-signed roots. The AC URNA
  certificate shipped there has `notAfter` 2026-07-18, before the election, while the UE2013/2015
  certificates it issued are valid to 2027/2030: a validator that checks dates on the whole chain
  would reject them (an observation by the reviewer, not part of a finding).
* **The 2026 QR format (H1).** A third party's transcription of the 2026 manual's small worked
  example (`QRBU:1:2 VRQR:6.0 ORIG:VOTA …`) is rebuilt byte for byte by the code's generator: no
  `VRCH`, `AGRE` right after `SECA`, the same header order, both HASH fields recomputing, slices of
  815 + 102 characters, QR texts of 967 and 524. Split limits from 816 to 825 reproduce the published
  cut; with the earlier 2024 range (823–829) that gives 823–825, and the code says 823. The example's
  `VERS:10.17.1.0` shows it was made with an earlier 10.x build. The earlier analysis had VRQR 6.0 and
  "no VRCH" only as code predictions.
* **Certificate QR codes (H3, H4, G10).** `MontaQRCodesCertificado` splits the certificate's hex text
  into `ceil(2·len/1082)` equal parts, each printed as `QRCE:i:n IDUE:<urna> MDUE:<model> CERT:<hex>`
  after the BU codes, with its own numbering. For today's certificates this gives 2 codes (all 110 of
  S110). The text of each code is 37 + 1,034 = 1,071 characters for a PEM certificate (UE2020/2022)
  and 983 for a DER one (UE2013/2015). "Two" is a size consequence: a certificate over 1,082 bytes
  would give 3, and the 1,082 constant leaves no room for the 37-character prefix (a part could reach
  1,119 characters). The on-screen certificate QR is a single code with the whole certificate, about
  2,100 characters for a PEM certificate. `MDUE` is most likely the 4-digit model year (medium
  confidence). Predicted but not observed: the printed `ASSI` is 264 hex characters (Ed521) or 278
  (ECDSA; the 34 DER signatures use fixed-width 66-byte r and s, so the length is stable).
* **Cargo order and Senate totals (H14, H15).** Sorting the real BUs' totals by `ordemImpressao`
  gives the state election first and Presidente in its own `IDEL` block, as in the manual's example;
  for Senador `TOTC = 2 × COMP` (two seats).
* **Certificate names in the demonstration data (H17).** TSE's demonstration QR certificates have
  subjects `UEAD…`/`uead…`, the real 2026 ones `UEAO…`/`ueao…` (upper case on UE2020/2022, lower case
  on UE2013/2015). The demonstration data probably come from test urnas. Whether an official urna
  prints in its QRCE codes the same certificate as in its signature file can only be checked on a
  printed 2026 BU.
* **Still unseen (H18).** The printed labels the code writes
  (`============= BU DIGITAL =============`, `======== CERTIFICADO DIGITAL =========`,
  `ASSINATURA BU DIGITAL:`, `-------------- 01 / 02 ---------------`), the 2026 manual's full field
  table (SA/RED fields) and its large example could not be checked.

### 4.4 Result files: contingency, RED, SA and aggregated sections

* **Attendance per eleição (B11, D18).** In 106 BUs of S110 with both eleições, eleição 6257 holds
  Presidente and 6259 the state offices. `qtdEleitoresAptosSecao` is equal in both, and the
  difference in aptos equals the difference in transit voters: only the presidential eleição can have
  more, which fits the rule that voters in transit outside their UF vote only for Presidente
  (art. 31). Example AL 27855/0002/0235: 475 aptos (50 in transit), 379 voters for Presidente
  against 436 (11), 341. `qtdEleitoresCompareceram` is the presidential figure and equals the count of
  `O voto do eleitor foi computado` (110/110). Across AC, 57 of 2,270 sections have more aptos for
  Presidente.
* **Meaning of the attendance counters (B13).** On 5 AC sections compared with BU na Web,
  `QT_ELEI_BIOM_SEM_HABILITACAO` equals `qtdEleitoresHabilitadosPorBiografia` (values 7, 6, 9, 9, 9)
  and never `…SemBiometria` (2, 5, 13, 4, 36). In 106 biometric BUs the three counters add up to the
  number of voters. A voter enabled but then suspended without any vote is not counted. These
  meanings agree with the comments of TSE's own 2024 `bu.asn1`.
* **Contingency urnas (B14, B15, D6, G9).** For the 10 contingency BUs of the targeted set, and
  separately the 10 of AC, the BU's correspondência is `identificacaoContingencia` with the
  município and zona the contingency medium was prepared for, while the RDV names the section,
  exactly as the two converters do. The carga history `historicoCodigosCarga` starts with the carga
  of the original section urna (CEFT "esperada", 10/10) and ends with the contingency urna's own
  carga (CEFT "efetivada", 10/10); one AC section and one RJ section have 3 entries; in the AC one
  the middle entry is an intermediate contingency urna that CEFT never lists. Sections whose
  replacement urna was re-loaded as a section urna on 03/10 have a single entry. All 110 BUs of S110
  have one entry. So the history is that of the section's voting (gap state), not "every carga code
  this urna had".
* **Envelope rule for SA (B16).** The 6 SA envelopes use contingency identification (their
  `TipoUrna` is `contingencia`) although the correspondência is the counted section, which the
  shared envelope converter predicts
  (`src/uenux2/src/app/comum/gravadores/asn/cconversorenvelopegenerico.cpp:53-66`).
* **SA (ballot-paper) BUs abroad (B17, D13).** The 6 SA BUs in the targeted set have `tipoUrna
  contingencia`, `tipoArquivo saManual`, `motivoUtilizacaoSA` "totalmente manual / outros", `dadosSA
  {junta 1, turma 1}` without an origin urna, `qtdEleitoresCompareceram` 0 while each office has
  11–49 votes, no detalhamento, voting-medium serial `00000000` (the code's default in
  `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:226`) and an RDV with `eleicoesSA`. All 6
  were counted on one UE2015 urna. In CEFT, 31 rows nationwide have origin C ("Sistema de
  apuração"), 29 of them abroad.
* **RED-recovered BUs (B18).** All 7 have `tipoArquivo votacaoRED`, the same version string and file
  names as VOTA, and their own signed-file order (`bu, jufa, rdv, …` instead of VOTA's
  `bu, rdv, jufa, …`); the BU is generated 0.79–5.29 h after the original emission time. In 3 of 7,
  RED ran on another urna: the log identifies "uma urna de contingência", `logd.dat` carries another
  urna's id, and the hardware certificate, signer serial and all signatures are that urna's, while the
  BU and RDV keep the original correspondence's urna identity.
* **The results-site index (B21).** In `aux.json`, each transmitted package's `hash` is the hex of an
  ASCII text: the standard Base64 of SHA-256 of the package's `.vsc` file with `/` written as `-`
  (135/135; `+` and `=` are kept). TSE thus indexes each package by its signature file, which hashes
  every other file.
* **Aggregated sections (B24, D19).** The BU names only the principal section; the results site
  serves no files for aggregated ones (all 6 fetch failures of the random draw were aggregated
  sections). The BU has no list of aggregated sections; bweb's list comes from elsewhere.
* **Logs of replaced urnas (B19).** A replaced urna's log reaches TSE inside its successor's
  `log.jez`, as an archived member (for example an original urna's VOTA log ending on 04/10 inside a
  contingency urna's package).
* **Coded values and timing (B20, B22, B23).** `10.23.0.0 - Praia da Barra do Cahy` is a build-wide
  string (VOTA, RED and SA files; SCUE, GAP, ATUE and VOTA logs). The binary's name
  `contingenciaSecao (4)` is the one TSE adopted in 2026 (`bu.asn1`); the 2024 text had
  `reservaSecao`. Load, opening, closing, emission and generation times are in that order in 129/129
  BUs.
* **A late section (B25).** The only non-standard status in S110, a PA section received two days
  late with an extra `imgbu.dat` in its transmission list (which answers 404), has ordinary,
  verifying urna files. Neither the urna files nor the code explain the delay; it is presumably a
  transmission or TSE-side matter.
* **Single transmission (D24).** All 2,270 AC sections and all 135 of S135 have exactly one
  transmitted hash, so the meaning of multi-hash sections could not be studied.

### 4.5 Preparation chain: GEDAI-UE → urna → BU

* **The AC 2026 workflow (E1).** From the 22 official AC GEDAI-UE logs (30,023 lines, 454 sessions):
  every session starts with "Abertura do GEDAI-UE - Versão 8.23.0.0 - Praia da Barra do Cahy", the
  computer identity, phase OFICIAL, UF AC, the Windows user, Holocron 4.20.0.0 and a signature check
  of `gedai-ue.vst`, then `app.ini.vsc`. 104 packages were imported on 16–24 Sep; 2,850 voting media
  (FV) were made on 17–20 Sep on 13 PCs of zone 001; 88 load media (FC) on 18–20 Sep on only two of
  them, 44 each, with 10–32 sections each; the FCs list the 2,270 AC sections exactly once (2,141
  plain and 129 principal sections; the 141 aggregated sections are not listed). 133 result-media
  runs: VOTA 108; SA, RED, VPP and ATUE 13; ADH 9 (123 serials); AVPart 3. 5,924 correspondence
  lines (24 Sep–3 Oct) came 4.97–14.89 days after the FC was made. Two zone PCs only imported
  packages; the two central PCs also handled correspondences. Logs are sent to
  `https://gedai-produtor.tse.jus.br:443/gedai/rest/jms/enviarLogGedai/`.
* **The generator identity (E2, D9, E15).** The GEDAI line "Computador | Serial TPM | Serial
  instalação" is exactly `IdentificadorGeradorMidia {nome, serialCertificadoTPM, serialInstalacao}`
  and the CSV triple `*_GERACAO_MIDIA` (the leiame defines the TPM field as "Número de série do
  certificado EK do TPM"). For 62/62 AC urnas, the BU's triple equals SCUE's "Mídia de carga gerada
  pelo computador" triple and the file-name triple of the GEDAI PC that logged the BU's load medium.
  Formats: name `Z<UF><zona:3>STD<2–3 digits>`; TPM serial 20, 32 or 40 hex characters on the 22 AC
  PCs (nationally, 44/110 BUs of S110 carry an 8-hex TPM serial); installation serial 8 hex. The TPM
  serial is stable while names change: two PCs were renamed, and one TPM appears in both 2024 and 2026
  under different names, so a machine is identified by its TPM serial, not its name.
* **Which PC the BU records (E3).** The BU records the PC that made the load medium (FC). GAP's
  "Mídia de votação gerada pelo computador" triple is the PC that made the voting medium: it equals
  the BU's triple in 0/62 urnas. The voting media of these 62 urnas came from 11 PCs, their load
  media from 2.
* **The user field (E6).** The user GAP logs for the voting medium corresponds to the Windows logon
  of the GEDAI session that generated it (62/62 urnas), not to the Odin login (52/62; the 10 misses
  are exactly the sessions where the two differ). So `DadosGeracaoMidia.usuario` holds the Windows
  logon (the link from the GAP line to that field is inferred). No identifier is reproduced here.
* **Media serials (E7, D8).** All 3,061 media serials (2,850 FV, 88 FC, 123 ADH) are 8 upper-case
  hex digits, all distinct, matching the code's rules (`src/ecourna/app/dados/cserialmidia.cpp:25`,
  `src/uenux2/src/app/comum/dados/md/correspondencia/ccarga.cpp:25-28`). Nine "Sobrescrevendo mídia
  de votação" events show that a re-generated medium gets a new serial: the serial belongs to a
  generation of the content, not to the card. bweb's "flash card" is the load medium's serial
  (`Carga.numeroSerieFC`, "Serial da MC"), never the voting medium's, which is not published. 88 FCs
  carry the 2,270 AC sections, 10–32 sections each.
* **The código de carga (E8, D1, D23).** It appears in GEDAI only in correspondences read back from
  the load media, never when an FC is made; with SCUE's line "Código de carga … gravado na tabela de
  correspondência" this shows the urna creates it at load. All 5,924 are 24 digits; 284 of the 2,877
  distinct codes start with 0. The two bweb columns `CD_CARGA_1` + `CD_CARGA_2` are the urna's
  printed rendering (`GetIDCargaFormatado`), and `CD_CARGA_2` is its printed "RESUMO DA
  CORRESPONDÊNCIA" (4,540/4,540 values). The code is not uniformly random: over 2,846 AC codes there
  are 1,278 distinct 6-digit prefixes (2,842 expected if random), every shared prefix stays within
  one zona, digit 6 is determined by digits 1–5 in this sample (not a Luhn or weighted mod-10/11
  digit), and the structure extends to about 8–9 digits. VOTA only checks "24 decimal digits".
* **The whole chain (E9).** For 62 AC urnas (5 of S110 and 57 fetched), 33 checks per urna link the
  GEDAI logs, the urna's SCUE and GAP lines and the BU: serials, generator triples, carga, dates and
  order. 31 pass 62/62 and the 2 that involve the Odin user pass 52/62 (E6). The FC generation time
  logged by the urna lies 168–259 s after GEDAI's "Início da geração de mídia de carga"; urnas were
  loaded on 23–27 Sep, 3.9–9.1 days after their voting medium was made.
* **Versions (E10).** 2026: urna programs `10.23.0.0`, GEDAI-UE `8.23.0.0`, Holocron `4.20.0.0`, all
  "Praia da Barra do Cahy"; HotSwapFlash `8.12.0.0`. AC 2024: urna `9.30.0.0 - Tupiniquim` (in logs
  and BUs), GEDAI `7.29.0.0`, Holocron `3.20.0.0`. Components share the release name within a cycle,
  majors go up by one per cycle, and the shared `.23` of 2026 is not a rule. The 2024 example files
  with `9.29.0.1 - Kayapó` are from the simulated phase (June 2024), not the election.
* **Shared library (E14).** All 109 GEDAI ERRO lines have the ecourna `CError` shape
  (`function:line:code - message`, `src/ecourna/api/exception/cerror.cpp:31-33`), all with code 3400,
  and one returns `ecourna::app::dados::CCorrespondenciasUrna`, so GEDAI-UE links the same data-model
  namespace that holds `CInformacaoMidia`, `CDadosGeracaoMidia` and `CIdentificadorGeradorMidia`.
* **Who signs the voting medium (E17, inference).** GAP checks the voting medium's `infomidia.vsc`
  (the wasm never does: `src/uenux2/src/app/comum/validamidia/cvalidamidia.cpp:46-47`); one urna
  logged "Falha ao validar assinatura SEVIN | pacote: (/dsk/fe/estatico/infomidia.vsc)" twice. The
  urna's `sevin.ber.pub` is national and equals Holocron's `osevin.ber.pub`; Holocron also ships
  `osevin.ber.pri` and runs in every GEDAI session. So the voting medium is most likely signed on the
  GEDAI PC with the SEVIN key held by Holocron.
* **Result media (E18).** GEDAI's result-media names (VOTA, SA, RED, VPP, ATUE, ADH) map one to one
  onto the code's `TipoAplicativo`; AVPart has no enum value, which fits its medium having no
  `infomidia` (`src/uenux2/src/app/comum/validamidia/cfabricaconteudomidiastart.cpp`, case 20).
* **Simulator fixtures (E4, E5).** A simulator BU carries the generator `nome_maquina` / `12345678`
  / `99999999` (from `eg.bin`), not `simulador-votacao-ng`. Neither fixture name has the real
  `Z<UF><zona:3>STD<2–3 digits>` form; the 64-zero TPM serial of the `infomidia` fixture is longer
  than any real one and than RFC 5280's 20-octet limit for X.509 serials; and the fixture load time
  `20201231T235958` has seconds where all 62 real `dataHoraCarga` end in `00`. The
  installation-serial fixtures do fit the real 8-hex format. The converters check none of these.
* **Number widths (D25).** The urna and GEDAI zero-pad identifiers (carga 24, urna 8, município 5,
  zona 4, seção 4 digits); the CSVs drop the padding inconsistently (bweb `CD_MUNICIPIO` unpadded,
  CEFT padded), so joins need normalization.

### 4.6 Logs and recovered 2026 parameters

The 110 logs of S110 hold 622,567 Latin-1 lines (VOTA 566,963, GAP 33,538, ATUE 10,260, SCUE 9,329,
LOGD 2,214, INITJE 263). The 2026 parameter file is not published, so these values are recovered from
the code plus the logs; most equal the simulator's scenario values.

| parameter or behaviour | value on real urnas | code | ids |
|---|---|---|---|
| monitor period | exactly 1,800 s (2,200/2,200 intervals) | `INTERVALO_LOG_S = 1800`, `src/uenux2/src/app/vota/monitor/cthreadmonitor.h:65` | C6 |
| voltage in the monitor line | only on models ≥ 2020 (UE2020 1,105/1,105 cycles, UE2022 1,015/1,015; UE2013/2015 0/454) | `src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp:265` | C6 |
| inactivity alert | 45 s (1,074/1,074 lines) | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:359-360` | C7 |
| fingerprint attempts | 4 (37,444/37,444 lines "de [4]"); success at attempt 1/2/3/4: 17,142/3,552/1,352/628; manual identification only after 4/4 | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:134` | C8 |
| fingerprint timeouts | 30 s first, 15 s later | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85` | C8, F4 |
| birth-year retries | at most 2 (never 3 wrong entries) | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:446-470` | C8, F16 |
| mesário fingerprint match threshold | score ≥ 20 (720 matches 20–271; 2,828 non-matches 0–19) | `src/uenux2/src/app/comum/comparecimentomesario/ccontroladorreconhecimetomesario.h:29` | G5 |
| booth inspection | every U(60, 90) minutes (634 intervals: 60.0 to 93.9, median 74.5), only while the mesário terminal waits for an identifier; 0 of 747 inside a voter session | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:36` | C9 |
| opening | strictly after the start time (95 urnas opened at hh:00:01 or hh:00:02, never hh:00:00); start hour 06 in AC, 07 in AM/MS/MT/RO/RR, 08 elsewhere and abroad (local clocks) | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:107` | C10, F17 |
| closing | refused before the local end time (115/115 requests consistent) | `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:207-214` | C10 |
| automatic shutdown on battery | 1,800 s (8/8 events at 1,801–1,802 s, all on preparation days) | `src/uenux2/src/app/vota/eleitor/cestadocomdesligamentoautomatico.cpp:49-84`, `src/uenux2/src/app/vota/eleitor/iniciovotacao/cexibealertadesligamento.cpp:34-51` | C11, G7 |
| power-event cap | 20 (2/2 suspension lines, each after exactly 20 source changes) | `src/uenux2/src/app/comum/util/cmonitoraalimentacao.h:50` | C11, G6 |
| registered-mesário limit | 6 | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:256` | C11 |
| power monitor | a "Carga" line after each switch to battery (165/165), only for the battery in use (181/181) | `src/uenux2/src/app/comum/util/cmonitoraalimentacao.cpp:58-96` | G6 |
| hard cut-off after `terminoVotacao` | never triggered; 48 sections took 638 votes after 17:00 Brasília, the last 104 minutes after | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:170-185` | F8 |

Other log results:

* **Partial suspension (C12, B12).** In 11/11 partial suspensions the confirmed and null lines add up
  to the six choices; when only the first Senate seat was confirmed, one Senador null is logged. Three
  suspensions without any vote log "Eleitor foi suspenso e não confirmou nenhum voto" and are not
  counted as attendance.
* **Send-then-log order (C14).** "Eleitor foi habilitado" (voter thread) directly follows "Tipo de
  habilitação do eleitor [biométrica]" (operator thread) 22,611 times and precedes it twice, during
  monitor log bursts. A log-then-send order would make the reversal impossible, so the data support
  the reconstructed order in
  `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalreconhecida.cpp:66-72`.
* **GAP exceptions (C15).** The three WHAT lines are GAP exception texts in exactly the ecourna
  `CError` format; one reports error 9342, a number that VOTA uses for something else
  (`ERRO_TELA_BRANCO_CONSULTA`, inside VOTA's range 9300–9500), so error codes are not unique across
  applications.
* **Refused keys (C17).** 71,318 "Tecla indevida pressionada" lines: 2.67 per computed vote, 0
  outside voter sessions, 1.53–4.30 per vote in domestic sections and 0.03–0.24 abroad (Presidente
  only). 29.0% are followed by a confirmation within 1 s (baseline 16.5%). This fits an early key
  during the 1 s check screen, but equally other refusals that happen right before CONFIRMA; 1 s
  timestamps cannot tell them apart. None of 128,222 consecutive confirmation pairs falls within one
  second.
* **Never-seen statements (C19).** 43 of 163 logging statements in `src/` never occur in S110. They
  split into post-packing lines (14, never in a published log), one development-only line, two blocked
  by configuration, one training-only, about 10 rare operator choices and about 12 error paths.
* **The published log ends at packing (G13, B7).** In 110/110 logs the last record is `Gerando
  arquivo de resultado [log.jez] + [Início]`; the copy to the internal and external media and the
  signing of the result files
  (`src/uenux2/src/app/vota/eleitor/fimvotacao/cgravaresultado.cpp:320-365`) happen after it, so
  they exist only in the live log on the urna.

### 4.7 Randomness and vote secrecy

* **The urna's random generator is not constant (G3).** The web build registers `CPrng(0)`
  (`src/uenux2/wasm/vota_web/vota_web_wasm.u30.cpp:449-450`), which re-seeds on every call and
  always returns 209652397, so every fingerprint image id would be 652606 and a second image would
  loop forever
  (`src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:47-61`).
  The real logs print 274 distinct image ids (2,878–999,190) in 99 sections, never 652606, none
  shared between sections, with 2–5 distinct ids within one run in 84 sections and a decile
  chi-square of 6.58 (9 degrees of freedom). This rules out a constant generator and a seed shared
  across urnas; it does not measure cryptographic quality. That the same generator also feeds the
  CEPESC random bytes is assumed.
* **RDV order (B10).** In every cargo list (534 in S110, 101 in the targeted set, SA included) votes
  are sorted by (vote type, typed digits as a byte string), not numerically, with no empty or dummy
  slots: the number of votes equals attendance × choices. Lexicographic and numeric order differ in
  223 (cargo, type) groups of S110 (183 legenda, 40 nulo) and 38 of the targeted set, and the files
  always follow the lexicographic order
  (`src/uenux2/src/app/comum/dados/crdvposicionadorvota.cpp:13-22`). The voting order cannot be
  recovered and the two Senate choices of one voter cannot be paired.
* **Votes stored as typed (F13).** A party (legenda) vote keeps the digits typed. Of the legenda
  votes in S110, 869/1,317 for Deputado Estadual, 146/714 for Deputado Federal and 18/43 for
  Deputado Distrital have more than the 2 party digits but fewer than the full number; counting
  2-digit entries, 1,061/1,317 and 365/714 are shorter than a full number. There are 462 (Estadual),
  308 (Federal) and 27 (Distrital) distinct strings among legenda votes, up to 71 for one party. This
  matches art. 207 and art. 204 §1 ("cada voto, como digitado").
* **Unique typed strings (G12).** 3,410 votes in 108/110 sections have a (type, digits) pair that is
  unique in their section and office: 2,083 nulls, 1,163 legenda votes with three or more digits and
  164 repeated-senator nulls (for example 1,068 of 3,594 Senador nulls; 698 of 1,125 Deputado Estadual
  legenda votes). Counting the digit string alone across all types gives 3,246. See §6.
* **The RDV key on the urna's flash (G11).** `GetCifradorCryptoTable`
  (`src/uenux2/src/app/comum/dados/crdv.cpp:45-91`) derives the AES-256 key and IV of the stored RDV
  from SHA-512 of the sorted office codes (the salt) and 32 bytes taken from a 128-byte hardware table
  (`IUrna` slot 6, `src/uenux2/src/api/hwil/iurna.h:53-54`), through HKDF-SHA512. The 2026 1st round
  has 3 office lists: (1,3,5,6,7) in 102 sections (states), (1,3,5,6,8) in 4 (DF) and (1) in 4
  (abroad), so 3 salts, reading 28, 30 and 28 distinct table positions. On an urna whose table does
  not change, the key and IV are a pure function of the office list and repeat in every election with
  the same offices; a Presidente-only 2nd round would reuse the (1) salt. Whether the table is per
  device is unknown.

## 5. Election rules (Res.-TSE 23.751/2026 and the 2026 mesário procedures) against the code

The governing text is Res.-TSE 23.751 of 26 Feb 2026 (atos gerais do processo eleitoral para as
Eleições 2026), read from a third-party verbatim copy; Res. 23.736/2024 is the 2024 municipal
counterpart and Res. 23.760/2026 the calendar (F24). The 2022 Manual do Mesário is the only manual
text verified word for word; the 2026 manual and course are known from search extracts. Real-data
columns refer to S110 (26,740 voter sessions).

| rule | what it says | code | real 2026 data | result | ids |
|---|---|---|---|---|---|
| art. 135 §1 II; Manual 2022 l. 749-751 | after the birth year is accepted, the voter "será habilitada(o) a votar mediante a leitura da digital da mesária ou do mesário"; the presidente "posiciona o próprio dedo … para atestar o procedimento" | `src/uenux2/src/app/vota/operador/confirmaidentidade/cregistradigitaloperador.cpp:285-297`: the finger is matched only for the log; "the voter is released in every case" | 1,605 such releases; 1,055 attributed to a registered mesário, 321 matched only an earlier unregistered capture, 229 matched no stored print (registered mesários or the 6 newest unknown captures): 550 (34%) not attributable, 6.0% of all sessions | enforcement gap: the rule describes a human procedure, the urna records the finger but does not check whose it is | F1 |
| arts. 143-144; 2026 manual (search extract) "digite o número do título do(a) Presidente" | suspension is done by "a(o) Presidente da Mesa" | `src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp:120-121`: any título with valid check digits that differs from the voter's own identifier | 29 suspension títulos typed: 14 accepted (11 of a registered mesário, 3 not), 15 refused; only 3 of the 15 fail the check digits, and the cause of the other 12 refusals is open | enforcement gap | F2 |
| art. 127 caput and I; 2026 manual (search extract) | closing belongs to "à(ao) Presidente … ou a quem ela(e) designar, dentre os componentes da Mesa" | `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:835-848`: any all-digit entry valid as a título after zero-padding to 12 | 112 closing títulos: after the same zero-padding, 111 are a registered mesário's and 1 is not | enforcement gap, small; the resolution itself allows a designated member | F3 |
| Manual 2022 l. 729-730 | message after 15 s without a finger | `src/uenux2/src/app/vota/operador/confirmaidentidade/cpededigital.cpp:84-85` | 30 s on the first attempt, 15 s later | **mismatch** (§1) | F4 |
| art. 127 VII | relatório "Eleitores Não Reconhecidos Biometricamente" | `src/uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp:130-150` | "Eleitores com habilitação biográfica" in 106/110 | **mismatch** of name; scope depends on reading (§1) | F5 |
| art. 123 | mesários registered after the zerésima and before voting | `src/uenux2/src/app/comum/comparecimentomesario/estados/cregistrarmesarios.cpp:45-47`, `:76-77` | 4/110 sections opened with none registered | **mismatch** (procedure not followed, code permits it) | F6 |
| art. 210 XII | BUs give biometric, biographic and no-biometrics counts | `src/uenux2/src/app/comum/gravadores/cgravadorbu.cpp:179-181` | absent in the 4 BUs from non-biometric urnas (abroad) | **mismatch** (formal) | F7 |
| art. 160 §§2-3 | voters in line vote "até que a última eleitora ou o último eleitor vote"; no time limit | `src/uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp:170-185`: after `terminoVotacao`, identification is blocked once nobody has voted for 5 minutes | never triggered; 638 votes after 17:00 in 48 sections, up to 104 minutes after | a mechanism the rules do not mention; its 2026 value is not public | F8 |
| Manual 2022 codes (555555555555 etc.); 2026 menu (search extract) | 2026: CORRIGE → "outras opções" → 1 audio, 2 encerrar | `src/uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp:63-66`, `:170-205` | menu choices logged: audio 2, closing 115, register mesários 38, counters 6; no 12-digit code exists in `src/` | match with 2026; the 2022 codes are gone | F9 |
| art. 142 §1 | panels: Dep. Federal, Dep. Estadual/Distrital, Senador 1st and 2nd seat, Governador, Presidente | order taken from the election data (`src/uenux2/src/app/comum/u30-foreign-fragments.cpp:90-97`) | 24,523 complete sessions in that order, 1,116 with Distrital | match | F10 |
| art. 31 | voters in transit outside their UF vote only for Presidente | `src/uenux2/src/app/comum/dados/celeitores.cpp:37-44`, `src/uenux2/src/app/comum/dados/ccargos.cpp:142-170` | 38 and 13 Presidente-only sessions in the two sections with such voters; 1,036 abroad | match | F11 |
| art. 206 §2 | same senator for both seats: second vote null | `src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp:67-80` | 736 such nulls, each with the matching nominal vote | match | F12 |
| arts. 207, 204 §1 | legenda votes; the RDV keeps each vote "como digitado" | `src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp:370-384` | partial numbers stored as typed (§4.7) | match | F13 |
| arts. 94 V, 208 II, 206 | only proportional inaptos are loaded; their votes are null; unknown majoritarian numbers are null | `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp:240-250`, `:413-421`; `src/uenux2/src/app/vota/eleitor/votamajoritario/cpedemajoritario.cpp:52` | not testable (no candidate tables in the sample) | match by code reading | F14 |
| arts. 143, 144 §3 | unconfirmed votes of a voter who abandons are null; with nothing confirmed the voter may return | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:329-370` (parameterised) | nulls after suspension: Presidente 11, Governador 10, Senador 7; no blank-after-suspension; 3 suspensions without a vote discarded | match (2026 parameters "discard" and "nulo") | F15 |
| arts. 133 §2, 134, 135 | up to 4 biometric readings; then birth year, one more try; then the Caderno | `src/uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecida.cpp:89-93`; `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:446-476`; `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:321-331` | N = 4 in all lines; birth-year mismatches never 3 in a row; birth year also asked of voters without biometrics (new in 2026) | match, apart from F1 and F4 | F16 |
| arts. 129, 160 | voting from 08:00 to 17:00 Brasília time | `src/uenux2/src/app/vota/u26-foreign-fragments.cpp:90-112`; `src/uenux2/src/app/vota/operador/u27-foreign-fragments.cpp:206-215` | no release before 08:00 Brasília; one early closing request refused (13:27 local, "após as 16:00:00 horas"); voters in line after 17:00 | match (hours are local per município in the data) | F17 |
| arts. 121, 122 | at 07:00 Brasília the mesa checks the urna; the presidente then prints the Zerésima and its Resumo | `src/uenux2/src/app/vota/eleitor/iniciovotacao/cverificahorariozeresima.cpp:23-35`; `src/uenux2/src/app/vota/u20-foreign-fragments.cpp:208-216` | zerésima 0–62 minutes after 07:00 (median 30) in 110/110; a zerésima more than 3 h late requires the mesário to confirm the clock (code only) | match; the clock confirmation is extra | F18 |
| art. 121 III, art. 126 II | mandatory keyboard test | `src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/cprezeresima.cpp:28-33`, `:64-69` | 101 skipped tests, all on preparation days; 1 on election day, after a restart once the zerésima was printed | match | F19 |
| art. 162; 2026 course (search extract) | 5 mandatory and up to 5 additional BU copies; mandatory copies, remove the result medium, additional copies | `src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp:54-87`, `src/uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbuoutrasobrigatorias.cpp:62-90`, `src/uenux2/src/app/vota/eleitor/fimvotacao/cretirarmr.cpp:26-69` | "qualidade OK" for the first copy 110/110; the copy counts are not in the published log | match with 2026; differs from the 2022 manual | F20 |
| Manual 2022 l. 1134-1136; art. 147 | a vote interrupted by power loss is not recorded and the voter may return | ballot kept in memory until the last office; restart with CONFIRMA only (`src/uenux2/src/app/vota/eleitor/iniciovotacao/creiniciovotacao.u07.cpp:56`) | 3 restarts, none with a voter session open; 8 battery shutdowns, all on preparation days | match (mid-vote case is code only) | F21 |
| arts. 142 §§3-5, 133 III-IV, 168 §2 IV, 140 §4 III | no-candidate message; the mesário terminal shows the office; CPF or título and photo; justification at the urna; audio | `src/uenux2/src/app/vota/eleitor/celeitorvotando.cpp:165-166`; `src/uenux2/src/app/vota/operador/u17-foreign-fragments.cpp:145`; `src/uenux2/src/app/vota/operador/confirmaidentidade/cnomeeleitor.cpp:86` | CPF identifications 6,125; photo shown 23,801 times; 673 justifications | match | F22 |
| TSE key descriptions (search extract) | "BRANCO e depois CONFIRMA"; CORRIGE "apaga aquela escolha" | `src/uenux2/src/api/gui/iinputfield.u17.h:28-63` with the field flags of `src/uenux2/src/app/vota/eleitor/comum/ctelasvota.u02.cpp:70`, `:137` | consistent with every recorded key test of the earlier analysis | match (flag meanings inferred from member offsets) | F23 |

## 6. Security- and privacy-relevant observations

These are measured facts and labelled inferences. None of them shows a change of votes; several show
what the published files can and cannot prove.

**Observations**

* **Poll workers' título numbers are in TSE's published urna logs (G2).** Twelve VOTA log templates
  print a título number, for example "Mesário <N> registrado" (814 lines, 110 sections), "Mesário
  <N> habilitou o eleitor" (1,055 lines), "Biometria coletada não é do mesário <N> (<id>)" (2,828
  lines) and "Título digitado para encerramento: <N>" (112 lines). Every number in the mesário
  templates is a valid título under the urna's own rule (the refused suspension entries are the
  exception: 12 of 15 are). Across S110 there are 440 distinct mesário títulos, 2–7 per section, in
  110/110 sections, and the log links each to the id of a fingerprint image stored in the
  unpublished `wsqmes.jez`. GAP and SCUE lines also print the 12-digit number of the user who
  generated each medium (602 and 110 lines). The voter's identifier is logged only by type
  ("Identificador digitado pelo mesário foi: (Título de eleitor)"). One caveat: if the 12 refused
  suspension entries of F2 were refused because they equal the voter's own identifier (the code's
  only other refusal condition), those 12 published lines would carry voters' títulos; this was not
  established. This report gives counts only, and the analysis outputs hold no título.
* **Failed keypad tests are logged as successes (G1).** On a wrong key, `CTesteTeclado` logs "Fim do
  teste de Teclado do TE - Sucesso" and then shows "Teste Falhou"
  (`src/uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp:126-138`). In the
  real logs, "Sucesso" is followed by a new "Início do teste" 7–48 s later (the code waits at least
  5 s) 27 times, in 21 of 110 sections (19%): at least 27 failed tests appear as "Sucesso", with the
  expected and pressed keys lost. The urna still forces a retest; the defect affects only the audit
  record.
* **The mesário terminal enforces less than the procedure (F1, F2, F3, F6).** The release by
  "mesário fingerprint" accepts any finger: 550 of 1,605 releases could not be attributed to a
  registered mesário. Suspension accepts any título with valid check digits other than the voter's:
  3 of 14 accepted suspension títulos were not those of a mesário registered in that urna. Closing
  accepts any valid título: 1 of 112 was not. Registration of the mesários can be skipped: 4 of 110
  sections opened with none. The data cannot show that anyone other than a mesário acted (a mesário
  may have registered without a usable fingerprint, or a presidente may not have registered), and
  accountability exists after the fact through the stored fingerprint images.
* **A marking channel in the published RDV (G12).** The RDV keeps each vote's digits as typed, by
  rule (art. 204 §1) and by TSE's schema ("Digitação como feita pelo eleitor na urna"). A voter can
  therefore produce a string that is unlikely to occur elsewhere in the section, for example a null
  vote with an unusual number. Because RDVs are published, such a string could in principle be used
  to show a third party that a given ballot was cast, which matters for coercion or vote buying. The
  data show that such unique strings exist in 108/110 sections (3,410 votes); most are presumably
  ordinary typos and invalid numbers. This is a measurement of a property known by design, not a
  defect.
* **Key granularity (G4, A4).** Signing keys are per urna, but the Código Verificador key
  (`cv.ber.pri`), the voter-biometrics key (`bio.sk1`) and the log key (`log.ber.pri`, inferred) are
  one per UF, deciphered with a secret that is the same on all four urna models of the UF (medium
  confidence). Whoever extracted that secret from one urna could decipher that UF's key files; for
  biometric templates, further keys and the CEPESC algorithm would still be needed.
* **The stored RDV (G11).** The key and IV that protect the RDV on the urna's flash depend only on
  the office list and 28–30 bytes of a 128-byte hardware table; there were 3 office lists in the
  1st round, and the same key recurs in elections with the same offices. The earlier analysis
  (analysis chapter 11-weird-code-audit, U2) found that superseded encrypted RDV copies are not
  overwritten; together, the confidentiality of intermediate RDV snapshots rests on that table, whose
  granularity (per device or not) is unknown.
* **The certificate identifies the signing urna (B4, B18).** When RED runs on another urna, the BU
  keeps the original urna's identity but is signed by the RED urna's hardware key. Tools that tie a
  BU to an urna by its certificate must expect this.
* **A CA certificate past its date (H2, reviewer's observation).** The AC URNA certificate in TSE's
  verification script expired on 2026-07-18; the UE2013/2015 urna certificates it issued are valid to
  2027/2030. A validator that checks dates on the whole chain would reject 2026 signatures from
  UE2013/2015 urnas.
* **What the published log can show (G13, C5).** The published log stops when `log.jez` is packed,
  so the signing and copying of the result files never appear in it. Single lines carry a keyed code
  that third parties cannot check; the log is protected as a whole by the signature file, which
  verified for every published log (B5).
* **Gaps in what the hash lists cover (E19, A7).** `app.ini.vsc` and the official URIs file that
  GEDAI-UE loads at every start are in no published list, and `/uenux/bin/avusrbinof.vst` is listed
  but never named in any log.
* **Over-long QR codes (H5).** By the code's rules about 13/110 BUs print a last QR above 1,100
  characters; that breaks the 2026 manual only if it keeps the cap. Such a QR still fits the QR
  standard, printed denser.
* **Unpublished files.** `jufa.dat` holds every voter's attendance and enablement outcome
  (`src/uenux2/src/app/comum/gravadores/cgravadorrcsecao.cpp:125-198`); it is listed in the
  signature files but not published, and whether the `criptografarJUFA` parameter was on cannot be
  told. The encrypted-BU option was off (G8); BUs are public anyway.
* **Exception texts in published logs (H11).** GAP's WHAT lines print internal C++ function
  signatures and source line numbers. Minor.

**Not a problem**

* **RDV order leaks nothing (B10, G12).** Votes are sorted by content, so neither the voting order
  nor the pairing of one voter's two Senate choices can be recovered from a published RDV.
* **The urna's random generator is not the simulator's constant one (G3).**
* **Integrity checks pass everywhere they can be run (B1, B5, B8, B9).** All hash chains, all
  certificates and all signatures of S135 verify; every BU line follows from its RDV; no unknown
  code value occurs.
* **The BU/RDV identity difference of contingency urnas is by design (G9, B14).**
* **Suspension, hours and the mandatory keyboard test behave as the rules say (F15, F17, F19).**
* **The 2026 parameters recovered from the logs match the code and, where the simulator's scenarios
  set them, the scenario values (C7, C11, G7).**

## 7. Corrections to the existing documentation

### 7(a) Files in this repository

Every change to `docs_en/` needs the same change in its Portuguese translation under `docs/` (same
file names), and README_EN.md in README.md.

| doc | section | says | should say | ids |
|---|---|---|---|---|
| README_EN.md | "What this is not", l. 47-48 | "The result files and logs they wrote are consistent with this code, which suggests the same source tree but does not prove the binaries are the same." | Keep, and add: 8 real VOTA log templates (printer driver, a mesário-terminal display error, "Votação suspensa") and the key-off shutdown path are absent from the reconstruction; two log calls have the wrong severity; the published log stops when `log.jez` is packed. | C1, C2, C3, G13 |
| docs_en/glossary.md | l. 448, "versão" | "the real 2024 BUs say `9.29.0.1 - Kayapó`" | The 2024 example files are simulated-phase files (FASE:S, June 2024) from the pre-release `9.29.0.1 - Kayapó`; the October 2024 election urnas ran `9.30.0.0 - Tupiniquim`. | E10 |
| docs_en/data-model/asn1-schemas.md | §5, l. 420-431 | compares the binary only with the published v2 (2024) specs | Add the 2026 package: `bu.asn1` equals `src/asn1` on 41/41 types; `assinatura.asn1` 2026 has the binary's `ModeloEquipamento` and the new `OrigemAssinaturaHardware`/`InfoChave`; the 2026 `rdv.asn1` is byte-identical to 2024 and still says `reservaSecao`/`urnaChegouAposInicioVotacao`. | H6, H7, H8, B22 |
| docs_en/modules/u23-uenux2-src-app-comum-gravadores-uenux2-src-app-comum-iinterf.md | l. 114 and l. 196 | `CEstadoGeralGap` "(all loads of this urna)"; `historicoCodigosCarga` "every load code" | The carga history of the section's voting: for a contingency urna it starts with the original section urna's carga and can include intermediate contingency urnas; a re-load restarts it. | D6, B15 |
| docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md | l. 356 | open question: "whether it really is `wsq.pk1`" | `/dsk/fi/estatico/chave/wsq.pk1` exists for every UF in all four 2026 urna lists (strong support; `bio.sk1` is the only other 7-character key name). Same for the `?` at `src/uenux2/src/app/comum/reconhecimentobiometrico/ccontrolaarmazenamentodeimagens.cpp:71-75`. | A3 |
| docs_en/modules/u24-uenux2-src-app-comum-justificativa-uenux2-src-app-comum-log-.md and docs_en/modules/u11-ecourna-lib-ecourna-api-asn.md | u24 l. 175-176; u11 l. 354 | `t02400ac-pu.dat`, `t00000ac-pu.dat`, `t00000br-pu.dat` "(partidos of the pleito, of the UF, national)"; "`-pu.dat` (national + UF)" | `-pu` is the urna parametrization (parties are `-pa`), and 02400 is the simulator's processo eleitoral; real 2026 and 2024 media carry only the national `o00000br-pu`, and how func 7787 handles the missing UF file is open. | E12, E13 |
| docs_en/modules/u27-uenux2-src-app-vota-operador.md | l. 141 and l. 166 | `CPedeTituloEncerramento` "presidente's título"; "título typed by the presidente to close the vote" | Any título valid after zero-padding to 12; nothing identifies the presidente or a registered mesário (art. 127 also allows a designated mesa member); 111/112 real closing títulos were a registered mesário's. | F3 |
| docs_en/modules/u40-lib-ecourna-classes-without-known-file.md | l. 335 | "simulator data: 'simulador-votacao-ng', 64 x '0'; 9224 observed through `CConversorDadoCorrespondencia` (11400, eg.bin)" | The `eg.bin` correspondence carries `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` / 64 × '0' / `A1B2C3DA` is the `infomidia-fv-*-t.dat` fixture. Real values: a `Z<UF>###STD##` name, an EK-certificate serial of 8/20/32/40 hex and an 8-hex installation serial. Same for the comments at `src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h:17-21`. | E4, E5 |
| docs_en/modules/u16-uenux2-src-api-gui-cprogressbar-cpp-uenux2-src-api-gui-cqrco.md, docs_en/modules/u36-app-comum-classes-without-known-file.md, docs_en/modules/u19-uenux2-src-api-pattern-cpolysingletonlist-h.md | u16 l. 241; u36 l. 388; u19 l. 397 | every "BU DIGITAL" part "≤ 1100 characters"; "parts of ≤ 1100 characters; on-screen … ≤ 2500"; "at most 2500 characters each" | All parts but the last are bounded; the last can reach 1,245 (Ed521) or 1,259 (ECDSA) characters printed and about 2,659 on screen, because the 277-character reserve is smaller than the 6.0 tail. Same for the comment at `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp:276`. | H5 |
| src/uenux2/src/app/vota/operador/u19-foreign-fragments.cpp | l. 123 and l. 130 | `CLogVota::GetInst().Loga(std::format("Título {} é inválido/válido para suspender a votação", …))` | `LogaAviso` (severity 2, ALERTA), as the `api_f1398` comment already says. | C3 |
| src/uenux2/src/app/comum/nomearquivo/cnomearquivo.h, cnomearquivo.cpp, src/uenux2/src/app/comum/dados/cdadosestaticos.u02.cpp | cnomearquivo.h l. 36 and l. 7; cnomearquivo.cpp l. 80; cdadosestaticos.u02.cpp l. 120-132 | `uedword pleito; // +4 CEstadoGeral +4`; "t02400ac-pu.dat (pleito 2400, UF AC, "pu" = partidos)" | The field is the processo eleitoral id (`idPE`, for example 01219 in `-el`/`-tte`/`-imp` names), not the pleito (03220, used in result-file names); "pu" is the urna parametrization. | E11, E12 |
| src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp | l. 252 | `const std::vector<uebyte> der = …RecuperarCertificado();` | The token returns a 1,034-byte PEM text on UE2020/2022 and DER only on UE2013/2015; the name is misleading. | H3 |
| src/uenux2/src/app/vota/monitor/cthreadmonitor.cpp | l. 203-205 (comment) | "whatever followed here is not in the binary. ?" | Add what real urnas log next: "Votação suspensa" (only if the zerésima was printed that day), then "Desligando a urna" and "Finalização de aplicativo". | C2 |

### 7(b) Chapters of the original analysis project (not in this repository)

| doc | section | says | should say | ids |
|---|---|---|---|---|
| analysis chapter 00-provenance | §1 (HotSwapFlash and GEDAI lists) | "Even with them, no hash could ever match" | No binary digest can match, but data files can: 42 RHVoice files on the 2026 urna equal upstream and the simulator's package; and every published HASH GERAL is reproduced by `CMontadorHash::CalculaHashGeral`. | A5, A1 |
| analysis chapter 00-provenance | §1, GEDAI package list | `vota_apl.jez`, `vota_ofi.jez`, `vota_sim.jez`, `vota_tre.jez`: "the VOTA application in its oficial / simulado / treinamento variants" | `vota_apl` is installed together with the phase package; a real official load checks `vota_apl.vst`, `vota_ofi.vst` and `avpart_ofi.vst` (110/110 installations). Only `_ofi`/`_sim`/`_tre` are phase variants. | A8 |
| analysis chapter 00-provenance | §3, table row on `eg.bin` | the media "generator" recorded in the data is `simulador-votacao-ng`, with an all-zero TPM serial (funcs 10268/9952) | The generator copied into a simulator BU (func 9952) is `nome_maquina` / `12345678` / `99999999`; `simulador-votacao-ng` is only in the `infomidia-fv-*-t.dat` fixture. | E4 |
| analysis chapter 00-provenance | §5, GAP lines | "a series of 34 steps" | 32–35 steps (494 runs: 32 × 6, 33 × 119, 34 × 354, 35 × 15): 24 fixed system and key packages, then the section's packages; the count varies with the voting-medium checks and the number of `-ce` packages. | A18, A6, A10 |
| analysis chapter 00-provenance | §5, "The media generator" | the data behind "gerada pelo computador (nome - tpm - instalação)" is part of the load record | Only SCUE's "Mídia de carga gerada pelo computador" triple equals the BU's `Carga.identificadorGeradorMidia` (62/62); GAP's "Mídia de votação gerada pelo computador" triple is the voting-medium PC, which differs in 62/62 and is not recorded in the BU. | E3, E2, D9 |
| analysis chapter 00-provenance §2 and glossary; also asn1-schemas, bu/qrcode and 10-boletim-de-urna where they call the 2024 examples "real" | 2024 references | "the real 2024 BUs say `9.29.0.1 - Kayapó`" | The 2024 examples are simulated-phase files from a pre-release; the 2024 election ran `9.30.0.0 - Tupiniquim` (GEDAI `7.29.0.0`, Holocron `3.20.0.0`). | E10 |
| analysis chapter libraries/boost-fmt-and-small-libs | GEDAI bullet | the urna packages "in their encrypted `.jez` form" | `.jez` is a ZIP container (110/110 urna logs open without a password, and the 22 + 22 GEDAI log packages are ZIP too); whether GEDAI's application packages are encrypted is unknown. Keep 00-provenance's ".jez packages are ZIP files". | A9, H9 |
| analysis chapter libraries/rhvoice | §9 | "Nothing in the public files tells which RHVoice build or voice revision is on the machines." | The 2026 urna lists show `libRHVoice_core.so.1.14.0`; the language files (upstream `910c889`), the Letícia-F123 16 kHz voice and `RHVoice.conf` (tag 1.14.0) are byte-identical to upstream and to the simulator package; the 8 simulator placeholders are web-only. | A5 |
| analysis chapter data-model/asn1-schemas | §5 | compares only with the 2024 v2 specs | Same correction as `docs_en/data-model/asn1-schemas.md` §5 above. | H6, H7, H8 |
| analysis chapter 12-real-urna-2026 | §1, results | the 2 messages missing from the wasm are "Início de impressão" and "Fim de impressão" | Over 110 urnas, 8 VOTA templates (919 lines) are absent from `src/` (§3, row 3); 183/191 are produced by it. | C1 |
| analysis chapter 12-real-urna-2026 | §2, "Same code, probably" | "Every kind of event the real VOTA logged is something the simulator's code can log, with the same text." | Almost every kind. Exceptions: the 8 templates, the key-off shutdown sequence, and the severity of the two suspension-título lines (ALERTA on urnas, INFO in `src/`). | C1, C2, C3 |
| analysis chapter 12-real-urna-2026 | §2, "Tecla indevida pressionada" | "1.3 to 3.6 refused keys per computed vote" | Over 110 urnas: 2.67 overall, 1.53–4.30 per domestic section, 0.03–0.24 abroad; never outside a voter session. | C17 |
| analysis chapter 12-real-urna-2026 | §3, the 2026 signature file | a trailing block with `ue2022` (22) and a second value 4; the two optional fields carry `[0]`/`[1]` | The trailer is TSE's `OrigemAssinaturaHardware {modeloEquipamento, algoritmoAssinatura}`: 4 (EdDSA, Ed521) on UE2020/2022 and 2 (ECDSA P-521) on UE2013/2015. `[0] tagChaves` and `[1] certificadoDigital` are the two alternatives of one mandatory CHOICE `InfoChave`; the certificate is PEM on UE2020/2022 and DER on UE2013/2015. | B3, H6, E16, B4 |
| analysis chapter 10-boletim-de-urna | §3.3, table row 10 | `historicoCodigosCarga`: "every carga code this urna had" | The carga history of the section's voting (see the u23 correction above). | D6, B15 |
| analysis chapter 10-boletim-de-urna | §4.2 last paragraph and §7 item 2 | the PKCS#11 module must treat the file and QR signatures differently; the algorithm is not visible | On 2026 data both verify over SHA-512 of the raw last hash with the same per-urna hardware key (Ed521 on UE2020/2022, ECDSA P-521 on UE2013/2015); the hashing most likely happens below the `IPkcs11` interface (inferred). The 2024 raw-hash QR rule belonged to the per-UF Ed25519 scheme. | B5, H2 |
| analysis chapter 10-boletim-de-urna | §4.3 and §7 item 4 | the certificate QR suggests a per-urna certificate (inference); VRQR 6.0 / no VRCH seen only in the harness | Per-urna certificates are confirmed in 135/135 real signature files (CA AC UE2020, AC UE2022, AC URNA); the code rebuilds the 2026 manual's example; the printed 2026 BU is still unchecked. | B4, H1, H2, H3 |
| analysis chapter bu/qrcode | §8, signature row, MDUE sentence and closing paragraph | "IPkcs11 slot 6 over the raw last hash; the algorithm is not visible"; MDUE "probably the urna model"; a per-urna or per-module key is an inference | Ed521 (UE2020/2022) or ECDSA P-521 (UE2013/2015) over SHA-512 of the raw last hash; ASSI 264 or 278 hex characters; key in a per-urna certificate printed as QRCE codes (2 per BU for all 110 real certificates; 1,071 characters each for PEM, 983 for DER); MDUE most likely the 4-digit model year. §6.2–§6.3 (per-UF Ed25519 keys) describe 2024 only. | H2, H3, H4, G10 |
| analysis chapter bu/qrcode | §4, end | "Every QR in the files is ≤ 1,100 characters (the longest … has 1,073)" | True for the 2024 files. With 6.0 signatures the last QR can exceed 1,100 (13/110 rebuilt 2026 BUs: 1,102–1,239). | H5, G10 |
| analysis chapter bu/codigo-verificador | §4 | the key file "presumably comes from the key packages that GEDAI-UE installs" (inference) | `/dsk/fi/estatico/chave/cv.ber.pri` is in the official 2026 urna lists, one per UF (28 distinct digests), identical on the four models; the CV key is per UF (medium confidence). | A3, A4, G4 |
| analysis chapter 11-weird-code-audit | §2.2 U6 and §6 | "not verified: the TSE pages returned 403" | Confirmed: TSE's published logs carry 440 distinct mesário títulos in 110/110 sampled sections (12 templates), linked to fingerprint image ids; GAP and SCUE lines carry the 12-digit number of the user who generated each medium. | G2, E6 |
| analysis chapter 11-weird-code-audit | §2.2 U7, impact | a constant generator on the urna is "improbable" | Excluded by data (274 distinct, uniformly spread image ids; never 652606). | G3 |
| analysis chapter 11-weird-code-audit | §2.2 U1 and U9, impact | U1 and U9 stated as inferences | U1 observed: at least 27 failed keypad tests logged "Sucesso" in 21/110 sections. U9 measured: 869/1,317 Dep. Estadual and 146/714 Dep. Federal legenda votes have partial numbers; 3,410 votes in 108/110 RDVs carry a string unique in their section and office. | G1, F13, G12 |
| analysis chapter 11-weird-code-audit | §6, external claims | the repeated-Senate-vote rule came from press reports | Res.-TSE 23.751/2026 art. 206 §2; 736 such nulls in the real RDVs. | F12, F24 |
| analysis chapter 08-voting-flow | §11 item 3 | "The meaning of each flag was not worked out." | Inferred from member offsets (medium confidence): CORRIGE clears the whole field, BRANCO only on an empty field, CONFIRMA finishes only on the remaining-digit field; this explains every recorded run. | F23 |

## 8. Open questions and what could not be checked

**TSE documents not read directly**

* The 2026 QR manual, the 2026 "Log do ecossistema", the 2026 Manual do Mesário, Guia Rápido and
  course pages, and the 2026 hashes page were not readable from the analysis machine. Res.-TSE
  23.751/2026 was read from a third-party copy. Wording and numbers taken from these sources should be
  re-verified on `www.tse.jus.br`. In particular: whether the 2026 QR manual keeps the 1,100-character
  cap (H5), whether the 2026 log document still says 7z and seven fields (H9, H10), and whether the
  stale 2026 `rdv.asn1` is also in TSE's own package (H8).
* The hash lists were read from a third-party copy. They are internally consistent, but the key-file
  digests on which A4 and G4 rest were not compared with TSE's page.
* Whether `verifica_qrcode*.py` in the 2026 package copy are TSE's (the README does not list them).
  The conclusions drawn from them rest on independent cryptographic checks.

**Artefacts that are not public**

* A printed 2026 BU, or its image `imgbu.dat` (HTTP 404): needed to see the QR labels, `ASSI` and
  `CERT` on paper, the `MDUE` value, the certificate name in QRCE (`UEAO` as in the signature file, or
  `UEAD` as in TSE's demonstration data), the real split limit and the over-long last QR (G10, H4,
  H5, H17, H18).
* `jufa.dat`, `imgze.dat`, `hash.dat`, `mr.ver` and the `wsq*.jez` packages are listed in the
  signature files but not published. So `criptografarJUFA`, the `local = 1` of `hash.dat`, the
  contract tag in `mr.ver` (`20260601173148` is expected) and the per-vote SAVD signing of the stored
  RDV remain code-only. Whether the urna's `hash.dat` reaches the published per-UF HASH GERAL cannot be
  closed (A1).
* The 2026 values of parameters that the logs do not reveal: numbers of mandatory and additional BU
  copies, `numTentativasVerificacao` (bounded at 2 by the data), `terminoVotacao`, the battery warning
  time, `permitirHabManualAudio`, the set of transit justifications.

**Keys and secrets**

* The construction and key of the 16-hex log code (C5, A19): SipHash-2-4 or 4-6, chain layout, and
  whether the chain restarts at each `logd` start.
* Whether the IKernelHSM secret (G4) and the 128-byte RDV table (G11) are per device, per UF or
  national. Only "not per device and not per model for the KEK" is established, and that rests on the
  assumption that a logged image id means `wsq.pk1` was deciphered.
* Which key SAVD uses to sign the `.vsu` files (a per-UF `ue.ber.pri` or a hardware key), and whether
  the Holocron private keys are TPM-wrapped (E17).
* Whether GEDAI's application packages (`.jez`) are encrypted (A9).

**Code questions without a follow-up**

* F2: why 12 of the 15 refused suspension entries were refused although they pass the título check
  digits. The code's only other refusal condition is "equal to the voter's own identifier"; whether
  `CValidadorIdentidade::EhValida` applies more than the check digits was not settled. The answer
  matters for §6 (it would mean those published lines carry voters' títulos).
* A7: does GAP check `/uenux/bin/avusrbinof.vst` under the wrong label at step 2, or not at all?
* A10: is `dadoscarga.vsu` on `/dsk/fe` in VOTA's SAVD table a slip, or the voting-medium copy?
* E13: how func 7787 handles the absence of a UF-level `-pu.dat`.
* Expansions of `ste`, `vpe` and `avgm` (the GEDAI-UE log names `VPP` "Verificador Pré e
  Pós-Eleição" and `ADH` "Software de Ajuste de Data/Hora"); the official wording of the AVPART and
  SAVP definitions (A6, A11, E18).
* What `/etc/ld.so.preload` loads, what `ld.so.cache.sig` and `ld.so.preload.sig` protect, and which
  component verifies `/boot/boot/avboot.vst` before the kernel runs.
* `docs_en/modules/u14` (l. 359) calls the `infomidia-fv-{1,2}-t.dat` files "FC media"; in this
  report FC is the load medium and FV the voting medium. Possibly a slip; not tested.
* The 43 logging statements of `src/` never seen in S110 (C19), mostly error paths and rare operator
  choices.
* The wasm's 32-bit `size_t` truncation in `FormataTamanho`: no real value reaches 4 GiB.

**Coverage of the data**

* The GEDAI analysis covers AC only (plus the RR 2026 and AC 2024 preambles). The 8-hex TPM serials of
  44/110 BUs from other UFs were not traced to GEDAI logs; the correspondences of 5 AC sections are
  missing from the 22 published AC GEDAI logs (2,265 of 2,270 found), for reasons not established.
* CEFT was read for 7 UFs and bweb for AC and ZZ only. The AM time zones of D5 were not compared with
  an official per-municipality list.
* No multi-hash section exists in S135 or among the 2,270 AC sections, so the meaning of a
  retransmission is unknown. Why one PA section was received two days late with an extra `imgbu.dat`
  is unexplained (B25).
* RED and SA are not in the binary; their behaviour (B17, B18, D13) is described from data only. Values
  never seen in the 1st round: `TipoUrna` 6, `TipoArquivo` 3/4/6, RDV vote types 5/8/9, consultas,
  municipal offices.
* The 2nd round (late October 2026) had not happened yet. G11's prediction that a Presidente-only round
  reuses the `(1)` salt, and every 2nd-round file check, are untested.
* The H5 count rests on fixed-width placeholders for `PROC` and `VERC` and omits `AGRE`; it can move by
  one or two sections.
* No inapto or federation-legenda vote could be identified (no 2026 candidate tables in the sample),
  no mid-vote power loss occurred, and the size of the Libras window could not be compared.
* Whether the 3 suspension títulos and the 1 closing título that match no registered mesário belonged
  to an unregistered presidente: the logs cannot tell (F2, F3).
* The Portuguese tree `docs/` was not reviewed; every correction of §7(a) applies to it too.

## 9. Reproduce

The scripts are in `investigation/scripts/`; `investigation/scripts/README.md` explains each one, and
`investigation/scripts/MANIFEST.json` lists their inputs, outputs and the results they reproduce. No
raw TSE file is committed to the repository: the urna logs contain poll workers' identifiers in clear
text, and the other files are TSE's to publish. `fetch_sections.py` and `fetch_open_data.py` download
the urna files and the open data; the HTML pages (or zips) of TSE's 2026 "Resumos digitais (hashes)"
page must be saved by hand. Keep downloaded data outside the repository. The outputs hold counts and
templates only: `log_templates.py` never prints raw lines and masks runs of 10–12 digits and the
media-generation computer and user values, and `vsc2026.py` masks the digits of the hardware signer
names. Review any output before publishing it.

Requirements: Python 3.9 or later with `asn1tools` and `cryptography` (`hashlists.py`,
`log_templates.py` and the two fetch scripts need only the standard library); run every script with
`python3 -I`. Ed521 verification is implemented in pure Python in `sigtools.py`, so no other EdDSA
library is needed. Shared helpers are in `kitlib.py`; the repository root is found from the scripts'
location, or given with `--repo`.

| step | script | what it does | results it reproduces |
|---|---|---|---|
| 1 | `fetch_sections.py` | downloads the per-section 2026 urna files (`aux.json`, BU, RDV, log, signature file) from `resultados.tse.jus.br`: a seeded random sample per UF (`sample`), named sections (`sections`), or the number of sections per UF (`count`); sections without published files are reported and skipped | data for the steps below |
| 2 | `fetch_open_data.py` | searches, lists and downloads TSE open-data resources through the CKAN API of `dadosabertos.tse.jus.br`, checking each download against its `.sha512` | data for §1 and §4.5 (the kit does not analyse it) |
| 3 | `hashlists.py` | reads the HTML pages (or zips) of TSE's "Resumos digitais (hashes)", recomputes every HASH GERAL, checks the directory-walk order, prints the per-model inventory and the per-UF key variability | A1, A2, A4 |
| 4 | `roundtrip_asn1.py` | decodes and re-encodes `bu.dat` and `rdv.dat` with `src/asn1` and compares bytes | B1, B2 |
| 5 | `bu_vs_rdv.py` | recomputes the vote hash chain and every BU line from the RDV, with the code's count, order and no-zero-line rules | B8, B9 |
| 6 | `vsc2026.py` (with `sigtools.py` and `assinatura2026_inferred.asn`, the layout inferred by this investigation, not a TSE document) | decodes and re-encodes the 2026 signature files, checks the published files' digests and sizes, and verifies the hardware signatures (files, lists, BU eleições) with each urna's certificate; with `--roots` it also checks the certificate chains against the AC UE2020, AC UE2022 and AC URNA certificates, which are not shipped and must be taken from TSE's verification package. The CEPESC software signature cannot be verified (no public implementation). | B3, B4, B5 |
| 7 | `log_templates.py` | reads `logd.dat` in memory, builds message templates per program and finds VOTA's templates in `src/` | C1 (and the line counts of §4.6) |
| 8 | `qr_rebuild.py` | rebuilds the 2026 BU QR texts and the certificate QR codes from `bu.dat` and the signature file, with the code's rules (`PROC`, `VERC`, `DTPL` and `TURN` as fixed-width placeholders, `AGRE` left out), and measures them | H3, H4, H5, G10 (sizes) |

`PYTHON=python3 sh investigation/scripts/run_all.sh SECTIONS_DIR HASH_HTML_DIR [OUT_DIR]` re-runs steps
3–8 offline on downloaded data and prints a summary (set `REPO=…` if the scripts are not inside
`<repo>/investigation/scripts`; `OUT_DIR` defaults to `./investigation-out`).

**Sample sizes.** `fetch_sections.py … sample --per-uf 4 --seed 2026` over the 28 UFs selects 112
sections; 106 of them are S110's, and the other 6 have no published files. The 4 remaining S110
sections are listed in `MANIFEST.json` and are fetched with the `sections` mode. The 25 targeted
sections of S135 are not listed in the kit. On S110 the kit prints: 110/110 BU and RDV round trips
(step 4); 216 eleições, 13,088 tuples and 534 cargo lists (step 5; this report's 260, 15,611 and 635
include the targeted sections); 110/110 signature files, models 47/44/14/5, and hardware signatures
verified over SHA-512 for 1,001 Ed521 and 197 ECDSA listed files (every file in each list, published
or not), 110 lists and 216 BU eleições, with 0/216 over the raw hash (step 6; this report's 405 file
signatures count only the published files of S135, and its 260 BU signatures include the targeted
sections); 622,567 lines and 191 VOTA templates, 86/92/5/8 (step 7); 13/110 last QRs over 1,100
characters, the longest 1,239 (step 8). The other figures of this report (the targeted sections, the
open-data joins, the GEDAI chain, the rule checks and the log statistics of §4.6 beyond the line
counts) come from working scripts of the analysis that are not part of the kit; they can be
re-derived from the same public data.

## Appendix: index of findings

Verdicts: confirmed; confirmed w/ corr. (confirmed with corrections); refuted; unverifiable; for
confirmation-type findings: spot-checked; spot-checked, corrected; not re-checked; corrected by
review. The statement is the verified one, shortened.

| id | kind | verdict | statement |
|---|---|---|---|
| A1 | new_inference | confirmed | Every 2026 HASH GERAL (10 PC lists, 112 per-UF urna values) is `CMontadorHash::CalculaHashGeral`'s chain in `CriaHashesDiretorio`'s traversal order and `CGravadorHashes`' scope. |
| A2 | confirmation | spot-checked, corrected | One VOTA binary and system for the four urna models; differences only in kernel modules (7/8/11/11 `.ko`), the hardware layer and MSD/mesário-terminal libraries. |
| A3 | confirmation | spot-checked | The key directory and the five key files VOTA reads exist on the real urna; `wsq.pk1` strongly supported. |
| A4 | new_inference | confirmed | 17 key files per UF, identical on all four models; 11 differ per UF, 6 national public keys equal Holocron's legal keys; one key-set tag in all 110 signature files. |
| A5 | mismatch | confirmed | 42 RHVoice data files on the urna are byte-identical to upstream and to the simulator's package; corrects two earlier analysis statements. |
| A6 | new_inference | confirmed w/ corr. | `avpart90/91/97/99.vst` belong to OAB/MP/CONFEA/TSE; GAP checks them in a per-urna order; AVPart is "Sistema Externo de Auditoria e Verificação" in the GEDAI log. |
| A7 | open_question | confirmed | `avusrbinof.vst` is in all four lists but no log names it; GAP's steps 1 and 2 both name `avusrbin.vst`. |
| A8 | mismatch | confirmed w/ corr. | A real official load installs `vota_apl` together with `vota_ofi` and `avpart_ofi` (110/110); `vota_apl` is not a phase variant. |
| A9 | mismatch | confirmed | `.jez` is a plain ZIP container; the earlier analysis contradicted itself ("encrypted" vs ZIP). |
| A10 | open_question | confirmed | VOTA's SAVD table has `dadoscarga.vsu` on `/dsk/fe`; every real GAP run checks it on `/dsk/fi`. |
| A11 | new_inference | confirmed w/ corr. | The Windows "Votação" list is an integrity-test support app (likely SAVP-Votação); it shares only the ASN.1 contract catalogue with the urna; contract release `20260601173148` = the code's `TAG_CONTRATOS`. |
| A12 | confirmation | corrected by review | The MSD/MSE split at model 2020 matches the urna files and self-test logs. Correction: UE2013/2015 signature files do carry a DER P-521 certificate from AC URNA (19/19); algorithm values 2 vs 4 are right. |
| A13 | confirmation | spot-checked | SCUE's seven signature labels are entries of VOTA's SAVD name table. |
| A14 | new_inference | confirmed w/ corr. | No resource directory or SVG support on the urna; Qt6 `linuxfb` with GIF and JPEG plugins only, matching the code's 12 GIF and 23 JPEG resources. |
| A15 | new_inference | confirmed w/ corr. | Urna user space is 32-bit x86 glibc and libstdc++ with TSE shared libraries; the hardware-layer libraries differ per model. |
| A16 | confirmation | spot-checked | The copy of the hash lists has the counts and names the earlier analysis read from TSE. |
| A17 | confirmation | spot-checked | Media roots match the code: `/dsk/fi` internal, `/dsk/fe` load medium during installation and voting medium afterwards. |
| A18 | new_inference | confirmed w/ corr. | GAP runs 32–35 signature steps (494 runs); 24 fixed, then the section's packages; the count varies with voting-medium checks and `-ce` packages. |
| A19 | open_question | confirmed | `log.ber.pri` (per UF) is unused by VOTA; likely the log daemon's MAC key (low confidence). |
| A20 | confirmation | spot-checked | The code's application identifiers equal the installed programs; SCUE runs from the load medium. |
| B1 | confirmation | spot-checked | 135/135 BU and 135/135 RDV files decode and re-encode byte for byte with `src/asn1`, including RED, SA and contingency. |
| B2 | confirmation | spot-checked | `rdv.dat` is a bare `EntidadeResultadoRDV`, not enveloped. |
| B3 | new_inference | confirmed w/ corr. | 2026 signature-file layout rebuilt (135/135); the published 2024 spec fails; the trailer's second value matches the HW algorithm (2/4), confounded with model family. |
| B4 | new_inference | confirmed w/ corr. | Models UE2020 47, UE2022 44, UE2015 14, UE2013 5; one national SW key set; per-urna HW certificates (except that RED on another urna signs with that urna's key). |
| B5 | new_inference | confirmed w/ corr. | 135 certificates, 260 BU signatures and 405 file signatures verify over SHA-512 of the hash; which layer hashes is inferred. |
| B6 | confirmation | spot-checked | Signed-file order = `CGravaResultado` writer order; WSQ files only on biometric urnas; RED and SA orders differ. |
| B7 | confirmation | spot-checked | Every published log ends at `[log.jez] + [Início]` (110/110). |
| B8 | confirmation | spot-checked | Vote hash chain recomputed for 260 eleições (15,611 tuples). |
| B9 | confirmation | spot-checked | Every BU line recomputes from the RDV (635 cargo lists). |
| B10 | confirmation | spot-checked, corrected | RDV sorted by (type, digits as bytes), no empty slots; lexicographic ≠ numeric in 223 groups of S110 and 38 of the targeted set. |
| B11 | new_inference | confirmed | Aptos and attendance are per eleição; out-of-UF transit voters count only for Presidente. |
| B12 | confirmation | not re-checked | Suspension nulls and vote logging match the code; RDV types 5/8/9 never occur. |
| B13 | new_inference | confirmed w/ corr. | `QT_ELEI_BIOM_SEM_HABILITACAO` = `qtdEleitoresHabilitadosPorBiografia` (5 AC sections); meanings agree with TSE's 2024 spec comments. |
| B14 | confirmation | spot-checked | Contingency BUs carry `identificacaoContingencia` while their RDVs carry the section (10/10). |
| B15 | new_inference | confirmed | A contingency urna's carga history starts with the original urna's carga and ends with its own; a re-loaded section urna restarts it. |
| B16 | confirmation | not re-checked | SA envelopes use contingency identification (6/6). |
| B17 | new_inference | confirmed | SA BUs abroad: `saManual`, attendance 0, voting-medium serial `00000000`, six sections on one UE2015 urna. |
| B18 | new_inference | confirmed w/ corr. | RED BUs: `votacaoRED`, own file order; in 3 of 7 RED ran on another urna, which signed with its own key. |
| B19 | confirmation | spot-checked | `log.jez` member order matches `CGravadorLog`'s `std::map` packing (135/135). |
| B20 | confirmation | not re-checked | `10.23.0.0 - Praia da Barra do Cahy` is build-wide (VOTA, RED, SA and other programs). |
| B21 | new_inference | confirmed | `aux.json` hash = hex of Base64(SHA-256 of the `.vsc`), `/` written as `-` (135/135). |
| B22 | confirmation | not re-checked | No coded value unknown to the code; `contingenciaSecao (4)` is the binary's and TSE's 2026 name. |
| B23 | confirmation | not re-checked | One `agora` for all result files; timestamps agree with the log within 1 s. |
| B24 | confirmation | not re-checked | The BU names only the principal section; no files are served for aggregated sections. |
| B25 | open_question | confirmed | The one late PA section has ordinary, verifying urna files; the urna files and the code do not explain the delay. |
| C1 | mismatch | confirmed w/ corr. | 8 VOTA log templates (919 lines) have no literal in `src/`; 6 are new against the earlier analysis. |
| C2 | mismatch | confirmed w/ corr. | The key-off shutdown continuation is missing; "Votação suspensa" appears only after the election-day zerésima. |
| C3 | mismatch | confirmed | "Título … (in)válido para suspender a votação" is ALERTA on urnas (29/29), INFO in `src/`. |
| C4 | new_inference | confirmed w/ corr. | SCUE/ATUE share the start-up lines with the reconstruction (ATUE also two `IInterfaceInit` lines); their power messages are different code. |
| C5 | new_inference | confirmed w/ corr. | The 16-hex log field is a stateful keyed code (622,567 distinct; repeated lines differ); not verifiable without the key. |
| C6 | confirmation | spot-checked | Monitor every 1,800 s; voltage only on models ≥ 2020. |
| C7 | confirmation | spot-checked | Inactivity alert at 45 s (1,074/1,074). |
| C8 | confirmation | not re-checked | 4 fingerprint attempts, 30/15 s timeouts, manual only after 4/4, at most 2 birth-year tries. |
| C9 | confirmation | spot-checked | Booth inspection every U(60, 90) minutes, never inside a voter session. |
| C10 | confirmation | not re-checked | Opening strictly after the start time; closing refused before the local end time; hours local per município. |
| C11 | confirmation | spot-checked | Battery shutdown at 1,800 s, power-event cap 20, mesário limit 6. |
| C12 | confirmation | not re-checked | A partial suspension fills every remaining choice, including the second Senate seat. |
| C13 | confirmation | not re-checked | Habilitação kinds from the log equal the BU detalhamento (101/106; differences explained). |
| C14 | confirmation | spot-checked | Two cross-thread reversals support the send-then-log order in `CDigitalReconhecida`. |
| C15 | new_inference | confirmed | GAP's WHAT lines use ecourna's `CError::what()` format; error numbers overlap between applications. |
| C16 | confirmation | not re-checked | `-log.jez` container matches `CGravadorLog` (archived logs first, deflate). |
| C17 | new_inference | confirmed w/ corr. | 71,318 refused keys: 2.67 per vote, none outside sessions, fewer abroad, enriched before confirmations. |
| C18 | open_question | confirmed w/ corr. | Four frequent messages exist in `src/` only as comments (155,165 lines, all INFO). |
| C19 | confirmation | not re-checked | Logging statements never seen are explained by packing order, configuration or rare paths. |
| D1 | confirmation | spot-checked | `CD_CARGA_1`/`CD_CARGA_2` are the urna's printed renderings of the código de carga (4,540/4,540). |
| D2 | mismatch | confirmed | bweb leiame says 19/6 characters for `CD_CARGA_1/2` (24/7 in the data); CEFT leiame says "urna esperada" for an efetivada column. |
| D3 | mismatch | confirmed | Numeric `CD_CARGA_URNA_*` columns lose leading zeros (9.3% of AC codes); zero-padding to 24 is needed for joins and the hash chain. |
| D4 | mismatch | confirmed w/ corr. | bweb `CD_TIPO_URNA` is an apuração status, not the BU's `TipoUrna`; a name collision, not a contradiction. |
| D5 | new_inference | confirmed | The datasets mix municipal local time and Brasília time; the two CEFT reception columns differ by the UTC offset. |
| D6 | mismatch | confirmed | `historicoCodigosCarga` is the section's carga history, not "every carga code this urna had" (corrects earlier docs). |
| D7 | mismatch | **refuted** | Claimed misleading wording of bweb's "código da carga original da urna"; the leiame correctly describes the efetivada urna's carga. |
| D8 | confirmation | spot-checked, corrected | "Flash card" = the load medium's serial; the voting medium's is unpublished; 88 FCs carry 10–32 sections each (not 34). |
| D9 | confirmation | spot-checked | The BU's generator triple = the CSV `*_GERACAO_MIDIA` triple = the GEDAI log file-name triple (15/15). |
| D10 | mismatch | confirmed w/ corr. | `QT_ELEI_BIOM_SEM_HABILITACAO` = biographic enablements; a missing detalhamento is published as 0 (undocumented convention). |
| D11 | confirmation | not re-checked | The CHOICE `DadosSecaoSA` appears in bweb as dates or junta/turma. |
| D12 | confirmation | spot-checked | Undocumented `CD_ORIGEM_VOTO` values map to `TipoArquivo`; `TP_DIVERGENCIA` 'C' covers contingency and SA. |
| D13 | new_inference | confirmed w/ corr. | For SA BUs the "urna efetivada" is the apuração urna, shared by many sections; CEFT's municipality name is wrong on those rows. |
| D14 | confirmation | not re-checked | Contingency correspondences have no section; contingency urnas served sections outside their prepared município or zona. |
| D15 | confirmation | not re-checked | `CD_TIPO_ELEICAO` confirms the placeholder-named ASN.1 `TipoEleicao`. |
| D16 | confirmation | spot-checked, corrected | `CD_TIPO_VOTAVEL` uses the BU `TipoVoto` numbers; 95/96/97 are database conventions (121 BUs decoded, not 120). |
| D17 | confirmation | spot-checked, corrected | `CD_CARGO_PERGUNTA` = `CargoConstitucional` numbers. |
| D18 | new_inference | confirmed | `QT_APTOS`/`QT_COMPARECIMENTO` are per eleição (57/2,270 AC sections differ). |
| D19 | new_inference | confirmed w/ corr. | Aggregated sections are not in `bu.dat`; the urna and bweb render the list in three formats. |
| D20 | confirmation | spot-checked, corrected | Minute precision of `DT_CARGA` comes from the urna (seconds `00` in 121/121 BUs). |
| D21 | mismatch | confirmed | The leiame preamble's quoting and `#NULO`/`-1` conventions do not hold in the files. |
| D22 | mismatch | confirmed | GEDAI leiame says the UF has "02 dígitos"; it is two letters. |
| D23 | new_inference | confirmed w/ corr. | The código de carga has structure in its first 8–9 digits (shared prefixes per zona); VOTA treats it as opaque. |
| D24 | confirmation | spot-checked | No multi-hash sections in AC; `DT_BU_RECEBIDO` = results-site reception time. |
| D25 | confirmation | not re-checked | The urna and GEDAI zero-pad identifiers; the CSVs drop padding inconsistently. |
| E1 | new_inference | confirmed w/ corr. | AC 2026 GEDAI-UE workflow rebuilt from 22 official logs (454 sessions, 2,850 FV, 88 FC, 133 result-media runs). |
| E2 | confirmation | spot-checked | The GEDAI identity triple is `IdentificadorGeradorMidia`; the BU carries it byte for byte (62/62). |
| E3 | mismatch | confirmed | GAP's "mídia de votação" generator is not the BU's load-record generator (0/62); corrects an earlier analysis sentence. |
| E4 | mismatch | confirmed w/ corr. | A simulator BU's generator is `nome_maquina`, not `simulador-votacao-ng`; corrects earlier docs and u40. |
| E5 | mismatch | confirmed w/ corr. | Simulator identity fixtures do not have the real formats (installation serials excepted). |
| E6 | new_inference | confirmed | `DadosGeracaoMidia.usuario` corresponds to the Windows logon of the GEDAI session, not the Odin user (62/62 vs 52/62). |
| E7 | new_inference | confirmed | Media serials are assigned per generation; all 3,061 meet the code's 8-hex rules. |
| E8 | confirmation | spot-checked | The código de carga is created by the urna at load and only read back by GEDAI. |
| E9 | confirmation | spot-checked, corrected | The chain GEDAI → SCUE/GAP → BU holds for 62 AC urnas (33 checks: 31 pass 62/62, 2 Odin-related 52/62). |
| E10 | new_inference | confirmed | Release names are shared per cycle and majors go up by one; the 2024 election ran `9.30.0.0 - Tupiniquim`. |
| E11 | mismatch | confirmed | The reconstruction calls the processo eleitoral id `pleito` in data-file naming. |
| E12 | mismatch | confirmed | `-pu` is the urna parametrization, not "partidos". |
| E13 | open_question | confirmed | No real UF-level `-pu.dat` exists although the code comment says func 7787 reads one. |
| E14 | new_inference | confirmed w/ corr. | GEDAI-UE uses ecourna's error format and data-model namespace. |
| E15 | new_inference | confirmed | A GEDAI PC's TPM serial is stable while its name changes. |
| E16 | new_inference | confirmed w/ corr. | The signature trailer is {model, HW algorithm} in 189/189 files (only 3 combinations); GEDAI log signatures described. |
| E17 | new_inference | confirmed | The voting medium's `infomidia.vsc` is likely signed on the GEDAI PC with Holocron's SEVIN key. |
| E18 | confirmation | spot-checked | GEDAI's result-media applications map onto `TipoAplicativo`; AVPart needs no `infomidia`. |
| E19 | open_question | confirmed | No published hash list covers `app.ini.vsc` or the official URIs file that GEDAI-UE loads. |
| F1 | mismatch | confirmed w/ corr. | The release "by the mesário's fingerprint" accepts any finger; 550/1,605 real releases not attributable (enforcement gap). |
| F2 | mismatch | confirmed w/ corr. | Suspension accepts any check-digit-valid título other than the voter's; 3/14 accepted were not a registered mesário's; cause of 12/15 refusals open. |
| F3 | mismatch | confirmed w/ corr. | Closing accepts any valid título; 1/112 was not a registered mesário's (not 6, after zero-padding); u27's "presidente's título" is intent. |
| F4 | mismatch | confirmed | Manual: 15 s fingerprint timeout; code and data: 30 s on the first attempt. |
| F5 | mismatch | confirmed w/ corr. | Art. 127 VII's report name differs from the urna's BEHB; scope depends on reading. |
| F6 | mismatch | confirmed w/ corr. | Mesário registration can end with nobody registered; 4/110 sections opened with none. |
| F7 | mismatch | confirmed | BUs of non-biometric urnas (abroad) omit the art. 210 XII counts (4/110). |
| F8 | new_inference | confirmed w/ corr. | A `terminoVotacao` cut-off exists in code that no public rule mentions; not triggered in 2026. |
| F9 | confirmation | spot-checked | The code follows the 2026 mesário-terminal menu; the 2022 12-digit codes are gone. |
| F10 | confirmation | spot-checked | Panel order (art. 142 §1) comes from the election data and is the legal one. |
| F11 | confirmation | spot-checked | Out-of-UF transit voters get only Presidente (art. 31). |
| F12 | confirmation | spot-checked | Same senator twice → second vote null (art. 206 §2): 736 such votes. |
| F13 | new_inference | confirmed w/ corr. | Legenda votes are stored as typed: 869/1,317 Dep. Estadual legenda votes have a partial number. |
| F14 | new_inference | confirmed | Only proportional inaptos are loaded; the majoritarian side has no inapto screen (arts. 94 V, 208 II). |
| F15 | confirmation | spot-checked | 2026 suspension parameters: discard with no vote, null for the remaining offices (arts. 143-144). |
| F16 | confirmation | spot-checked | 4 biometric attempts, then birth year with one retry (arts. 133-135). |
| F17 | confirmation | spot-checked | No release before 08:00 Brasília; closing refused before 17:00; voters in line vote after 17:00. |
| F18 | confirmation | not re-checked | Zerésima never before 07:00 Brasília; a late zerésima requires confirming the clock. |
| F19 | confirmation | not re-checked | The keyboard test can be skipped only before the eve of the zerésima, or after a post-zerésima restart. |
| F20 | confirmation | not re-checked | End-of-day order matches the 2026 course, not the 2022 manual. |
| F21 | confirmation | not re-checked | A vote interrupted by power loss is not recorded; restart needs only CONFIRMA. |
| F22 | confirmation | not re-checked | No-candidate message, office on the mesário terminal, CPF and photo, justification and audio match the 2026 rules. |
| F23 | new_inference | confirmed | CORRIGE clears the whole number; BRANCO works only on an empty field (flags inferred from offsets). |
| F24 | open_question | confirmed | Res. 23.751/2026 governs 2026 (not 23.736/2024); 23.760 is the calendar. |
| G1 | confirmation | spot-checked | Failed keypad tests are logged "Sucesso": at least 27 in 21/110 sections. |
| G2 | confirmation | spot-checked | Poll workers' títulos are in TSE's published logs: 440 distinct in 110/110 sections. |
| G3 | new_inference | confirmed w/ corr. | The real urna's random generator is not constant (274 distinct, uniform image ids). |
| G4 | new_inference | confirmed w/ corr. | The key-encryption secret is the same on all four models within a UF; secret key files are per UF (medium confidence). |
| G5 | confirmation | spot-checked | Mesário fingerprint match threshold is score ≥ 20. |
| G6 | confirmation | not re-checked | The power monitor behaves as reconstructed (165/165, 181/181, cap 20). |
| G7 | new_inference | confirmed w/ corr. | 2026 automatic battery shutdown after 1,800 s (8 events, all on preparation days). |
| G8 | confirmation | spot-checked | The encrypted-BU option was off in the 1st round (129/129 BUs without `seguranca`). |
| G9 | confirmation | not re-checked | Constant local 1 in the correspondência; the contingency BU/RDV split is the code's. |
| G10 | new_inference | unverifiable | Predictions for the printed BU: ASSI 264/278 hex; 2 certificate QR codes of 1,071 (PEM) or 983 (DER) characters; a last QR up to 1,245/1,259 characters. |
| G11 | new_inference | confirmed | The stored-RDV key depends on the office list and 28–30 bytes of a hardware table; 3 salts in the 1st round. |
| G12 | new_inference | confirmed w/ corr. | RDV order leaks nothing; 3,410 votes in 108/110 sections carry a unique typed string. |
| G13 | confirmation | spot-checked | The published log always ends when `log.jez` is packed; signing and copying never appear in it. |
| H1 | confirmation | spot-checked | The 2026 QR manual's worked example (third-party transcription) is rebuilt byte for byte. |
| H2 | new_inference | confirmed w/ corr. | 2026 QR signatures: Ed521 or ECDSA P-521 over SHA-512 of the HASH, per-urna certificate (verified on TSE demonstration data). |
| H3 | confirmation | spot-checked | The QRCE split matches TSE's demonstration data; 2 codes for all 110 real certificates. |
| H4 | new_inference | confirmed w/ corr. | Two certificate QR codes is a size consequence; MDUE is probably the model year. |
| H5 | mismatch | confirmed w/ corr. | The 1,100-character cap is not kept in format 6.0 (13/110 rebuilt last QRs over it); a public-document contradiction only if the 2026 manual keeps the cap. |
| H6 | confirmation | spot-checked | TSE's 2026 `assinatura.asn1` re-encodes 110/110 real signature files. |
| H7 | confirmation | spot-checked | TSE's 2026 `bu.asn1` equals `src/asn1` on 41/41 types. |
| H8 | mismatch | confirmed w/ corr. | The 2026 `rdv.asn1` is unchanged from 2024 and contradicts the 2026 `bu.asn1` and the code on two enumerations. |
| H9 | mismatch | confirmed | Log packages are ZIP, not 7z as the 2022 log document says. |
| H10 | mismatch | confirmed w/ corr. | Real log lines have 6 TAB fields with `DD/MM/YYYY HH:MM:SS`, not the 7 fields of the 2022 prose. |
| H11 | mismatch | confirmed | Undocumented severity WHAT in real logs (GAP only). |
| H12 | mismatch | confirmed w/ corr. | GEDAI-UE logs are not in the urna log format; their naming is documented by the 2026 leiame. |
| H13 | mismatch | confirmed | The 2022 result-log name rule is obsolete; the code and the 2026 README agree. |
| H14 | confirmation | spot-checked | QR cargo order by `ordemImpressao` with `IDEL` per election matches the manual example and 102/110 BUs. |
| H15 | confirmation | spot-checked | `TOTC = 2 × COMP` for Senador in the manual example and 106/106 BUs. |
| H16 | confirmation | spot-checked | VOTA's result files and names match the 2026 README, which omits phase `t`. |
| H17 | open_question | confirmed w/ corr. | Demonstration certificates are `UEAD`/`uead`, real 2026 ones `UEAO`/`ueao`. |
| H18 | open_question | unverifiable | Printed labels and the rest of the 2026 manual could not be checked. |

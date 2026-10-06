# Glossary: Portuguese election terms and TSE acronyms

The strings, class names and file names in `vota_web_wasm.wasm` are in Brazilian Portuguese, and
many are TSE acronyms (TSE is the federal electoral court). This page gives each term a one-line
English meaning and, where the term appears in the binary, shows where. Every address, function
index and count on this page was produced by the commands shown here (mostly those of §0).

The other chapters already explain most of these things in depth. This page only defines the
words and links to those chapters:
00-provenance, MAP, [the ASN.1 schemas](data-model/asn1-schemas.md),
the BU chapters, the [module chapters](modules/) and the
library chapters.

**How to read the "where" column**

| notation | meaning | look it up with |
|---|---|---|
| `@123634` | a NUL-terminated string at linear-memory address 123634 (decimal), column 1 of `analysis/strings.tsv`. A few short strings (`"uf"` @168637, `"ee"` @187015, `"te"` @176379, `"o"` @142797, `"s"` @81614) are the **tail of a longer string** (the linker lets `"uf"` share the bytes of `"EC_POINT_point2buf"`), so they are not in column 1; `q.py s` still decodes them | `python3 tools/wasmmap/q.py s 123634` |
| `func 12074` | wasm function index, the `N` in `(func (;N;) …)` of the `.wat` | `q.py f 12074` (pseudo-code), `q.py wat 12074` (wasm text) |
| `vota::CGeraBU` | a C++ class name, from RTTI (`analysis/classes.tsv`) or from a `std::source_location` signature | `q.py cls 'vota::CGeraBU$'` |
| `ModuloX.Tipo` | an ASN.1 type rebuilt from the binary, in [`src/asn1/`](../src/asn1/) | open the `.asn` file |
| `uenux2/src/app/…` | an original source path (`analysis/srcloc.tsv`, `analysis/files.tsv`) | `q.py file 'fimvotacao'` |
| `t02400ac-pu.dat` | a file of the simulator's filesystem (`upstream/fs/<scenario>/dsk/…`) or one the program writes | `ls`, or `analysis/runtime/memfs-*` |

`q.py` is `python3 tools/wasmmap/q.py`, run from the project root.

---

## 0. Finding a word yourself

```sh
python3 tools/wasmmap/q.py grep 'zer.sima'          # regex over all strings, with the functions that use them
grep -i 'boletim' analysis/strings.tsv               # the same table, plain grep
cut -f3 analysis/classes.tsv | grep -i mesario       # class names (only classes that have RTTI)
cut -f2 analysis/files.tsv | grep -i justificativa   # original source files
node tools/run/headless.mjs --scenario municipal-t1 --keys "91001  C  12  C  "   # watch the words at run time
```

In the last command, the pause (two spaces) after `91001` matters: right after the fifth digit
the program is in a short transient state (`CConfereVotoEmCargo<…>`), where a CONFIRMA is
rejected as *tecla indevida* (§6). Typed as `"91001C  12C  "`, the first `C` is lost, the
`1`/`2` meant for Prefeito land on the Vereador confirmation screen, and the run stops at the
Prefeito prompt.

Three pitfalls:

1. **The application's strings are Latin-1, not UTF-8.** A UTF-8 pattern finds **0** hits:
   `LC_ALL=C grep -a -o 'zerésima' vota_web_wasm.wasm | wc -l`. The pattern `'zer.sima'` finds **23**
   (same command), because the `é` is the single byte `0xE9`. In the raw binary, a lower-case
   letter + a byte `0xE0–0xFA` + a lower-case letter occurs 1,340 times; the UTF-8 equivalent
   (lower-case letter + `0xC3` + byte `0xA0–0xBA` + lower-case letter) occurs once, in @367628
   `'Turno invÃ¡lido.'` (that is how `strings.tsv` shows a UTF-8 `á` decoded as Latin-1).
   Both counts come from
   `python3 -c "import re;d=open('vota_web_wasm.wasm','rb').read();print(len(re.findall(rb'[a-z][\xe0-\xfa][a-z]',d)),len(re.findall(rb'[a-z]\xc3[\xa0-\xba][a-z]',d)))"`
   (prints `1340 1`; non-overlapping matches).
   `strings.tsv` and `q.py` decode Latin-1 for you, so use them. The run-time log `logd.dat` is
   Latin-1 as well (runtime README).
2. **Small addresses give false cross-references.** `q.py s 1544`
   (`'Código Verificador: {}.{}.{}.{}'`) lists the real user, func 11182 `comum::CRelUtil::DSCodigoVerificador`. It also
   lists OpenSSL funcs 3197, 4413 and 7243 and SQLite's `unixOpen` (12912), because in those
   functions `1544` is just an integer (in the OpenSSL ones, the source line number passed to
   `ERR_raise`). `q.py s 8704` (`'Seção agregada: {:04}'`) lists seven functions. Six are false
   hits, five in SQLite (`sqlite3RunParser`, `resolveAlias`, …, where 8704 is a flag word) and
   OpenSSL's `do_i2b`. The seventh, func 12105 `vota::CGeraRelatorios::StartState`, is a real
   user. A real use of a format string usually looks like this: the code stores the start
   **and** the end of the text, as a `std::string_view` for `std::format`: `i32.const 8704` then
   `i32.const 8725` in func 12105 (8725 − 8704 = 21 characters), and `1544` then `1575` in func 11182
   (31 characters). When an address is below about 10,000, read the code before you trust the
   reference.
3. **func 7787 is not really `CHKDFSeed::GetSeed`.** Many strings (for example
   @129510 `'[1] para imprimir a zerésima e seu resumo'`) are referenced by func 7787, which the
   tools formerly showed as `ecourna::api::security::CHKDFSeed::GetSeed`. Func 7787 is the
   148,912-byte start-up function; its only caller is `votaInit` (7840). Its `q.py f 7787` card
   lists 113 `source_location` records from 27 different source files, because link-time
   optimisation inlined about twenty functions from other files into it (`CHKDFSeed::GetSeed` is
   one of them). The tools first named it after the `chkdfseed.cpp` records at the top of that
   list, with `name_confidence low`. The database now calls it `vota::CInformacaoEleitor::Inicializar`
   (name inferred, `medium`), so `q.py s 129510` prints `ref by 7787: vota::CInformacaoEleitor::Inicializar`.
   See [u02](modules/u02-ecourna-lib-ecourna-api-security.md).

### Worked example: from an acronym to wasm instructions (BEHB)

`BEHB` appears in file names (`behb.dat` @60700, `behb.vsu` @17985) and in the class
`vota::CImprimindoBEHB`. The binary never spells out the acronym. Search for the words you would
expect in it instead:

```
$ python3 tools/wasmmap/q.py grep 'habilitados biograficamente'
  174639 'Gerando relatório de eleitores habilitados biograficamente'  <- 12105:vota::CGeraRelatorios::StartState
  376538 'Ocorreu um erro durante a geração do relatório de eleitores habilitados biograficamente na MI.'  <- 12105:…
  485744 'boletim de eleitores\nhabilitados biograficamente'  <- 12074:vota::CImprimindoBEHB::StartState
```

So BEHB = **B**oletim de **E**leitores **H**abilitados **B**iograficamente, the printed list of
voters who were admitted by birth year instead of fingerprint. The report's title is the third
string. This is how func 12074 uses it (`q.py wat 12074`, lines 178–212):

```wasm
local.get 1
i32.const 56
call 137                          ;; operator new(56): heap buffer for a long std::string
local.tee 3
i32.store offset=12               ;; string.data = buffer
local.get 1
i64.const -9223371796336607184    ;; = 0x80000038_00000030: size 48; cap word 56 (bytes allocated) | long-mode bit
i64.store offset=16 align=4       ;; string.size and the cap word in one 8-byte store
local.get 3
i32.const 485784                  ;; copy the text 8 bytes at a time, last chunk first
i64.load
i64.store offset=40 align=1
…                                 ;; 485776, 485768, 485760, 485752
local.get 3
i32.const 485744                  ;; the address q.py printed
i64.load
i64.store align=1
local.get 3
i32.const 0
i32.store8 offset=48              ;; NUL terminator after the 48 characters
```

The string is not passed by pointer. The compiler inlined the `std::string` constructor, so a
48-character literal became six `i64.load`/`i64.store` pairs (the title is 20 + 1 + 27 = 48
characters, counting the `\n`). The size and cap words follow the libc++
layout described in libcxx-core §2: the cap word holds the
allocation size (56 = capacity 55 + 1 for the NUL) with the sign bit set to mean "long string". When you look for the users of a
long literal, also search for the addresses inside it (start + 8, + 16, …): the first chunk
is not always the one the code loads first.

---

## 1. Acronyms at a glance

"Not spelled out" means that neither the binary nor the project's TSE documents write out the
expansion. An expansion in *italics* is not in the binary either. It comes from the TSE's
published file-format document
([`bu-rdv-format-v2/README.md`](../upstream/tse-docs/bu-rdv-format-v2/README.md)), from another
chapter, or it is the standard name of an industry term. The others are written in the binary,
or pieced together from strings in it (the address is given).

| acronym | expansion | one line | § |
|---|---|---|---|
| ADH | not spelled out | tool used to fix the urna's date and time (@363656 "utilize o ADH para ajustar o horário desta urna") | 11 |
| ATUE | not spelled out | an urna application type (`TipoAplicativo atue(7)`); `EstadoGeralGap.identificadoATUE` | 11 |
| BEHB | Boletim de Eleitores Habilitados Biograficamente (title @485744) | printed list of voters admitted by birth year | 10 |
| BIM | Boletim de Identificação de Mesários (@66858) | printed list of the poll workers who registered | 10 |
| BU | Boletim de Urna (@137222 `'Boletim de Urna foi atingido'`) | the tally of one urna, printed and written to the result medium | 10 |
| BUJ, BJust | Boletim de Justificativa (@155797 "Boletim de Justificativa Eleitoral") | printed list of absence justifications | 10 |
| CEPESC | *Centro de Pesquisa e Desenvolvimento para a Segurança das Comunicações* ([u01](modules/u01-ecourna-lib-ecourna-api-security.md)) | the government crypto centre whose algorithm protects TSE files; `ecourna::api::cepesc` | 10 |
| CV | Código Verificador (@1544; class `comum::CCalculaCV`) | the check number printed under every BU block | 10 |
| EG | Estado Geral (`EstadoGeralUrna`; @328306 `'EG Geral MI'`) | the urna's persistent state files (`eg.bin`, `vota.bin`, `gap.bin`, `sa.bin`) | 11 |
| FC / FV | flash de carga (@230523 `serialFlashCarga`) / *flash de votação* | the load card and the voting card; `ModuloInformacaoMidia.TipoMidia { mr(1), fc(2), fv(3) }` | 8 |
| FI / FE | *flash interna / flash externa* (the code has `EFlashOrigem` and `LogaCopiandoArqResParaFI`) | internal and external flash memory, `/dsk/fi` and `/dsk/fe` | 8 |
| GAP | not spelled out | state of the urna's application selection (`gap.bin`, `ModuloEstadoGeralGap`) | 11 |
| GEDAI-UE | not spelled out (see §11) | the TSE's Windows program that prepares the urna media; `Sistema.gedai(6)` | 11 |
| HSF | Hot Swap Flash (the `<h2>` heading `HOT SWAP FLASH - 02/09/2026` of `pc1hsfg.html`, whose files sit in `D:\Aplic\SEVIN\HSF\`) | a TSE Windows tool; not in the binary | 11 |
| HSM | *Hardware Security Module* | the urna's crypto chip; `api::IKernelHSM`, @7762 | 8 |
| HV | horário de verão | daylight saving time; `comum::CHV` | 12 |
| JE | Justiça Eleitoral (@155716) | the electoral justice system; type names `DataHoraJE`, `DataJE` | 2 |
| MC | mídia de carga (@367431) | the load medium (@8376 "Código identificação MC", @367431) | 8 |
| MI / ME | mídia interna / mídia externa (@373900, @373852) | the same as FI / FE, in messages (@331065, @334922) | 8 |
| MR | mídia de resultado (@137869) | the removable medium that carries the results out of the urna | 8 |
| MV | mídia de votação (@362075) | the card that holds the votes; it moves to a contingency urna (@370017) | 8 |
| PADA-UE | not spelled out | TSE system that produces the election data packages (`Sistema.padaUE(14)`) | 11 |
| PE | processo eleitoral (@8457) | the whole election (e.g. 2026 general elections); `comum::CPE`, `"pe": 2400` | 4 |
| PU | parametrização da urna (inferred: the `-pu` package holds `EntidadeParametrizacaoUrna`) | urna parameters package (`t02400ac-pu.dat`); `vota::CImpressaoPU` | 10 |
| RDV | Registro Digital do Voto (module `ModuloRegistroDigitalVoto`) | the list of every vote cast, with no link to the voter: kept sorted by (vote type, digits typed), so the voting order is lost (`comum::CRdvPosicionadorVota::Posiciona`, func 11496) | 10 |
| RED | Recuperador de Dados (@7981 "Sistema Recuperador de Dados") | recovers the results of an urna that failed | 11 |
| RZE / ZE | resumo da zerésima / zerésima | `rze.dat`, `imgze.dat` (@328640 "Resumo da Zerésima MI") | 9 |
| SA | *Sistema de Apuração* (TSE document) | counting system for paper ballots or unreadable media | 11 |
| SAVD | not spelled out | the signing/validation service interface; `comum::IInterfaceSavd` | 11 |
| SCUE | not spelled out | a key name and the load config file `scueconf-t1.dat` | 11 |
| SEVIN, SEINT, SECAD, SECINP | TSE units; *Seção de Voto Informatizado* for SEVIN (00-provenance) | names of the keys/signers of the data packages (func 5900) | 2 |
| SIECO | not spelled out | `REG AUTENTICAÇÕES SIECO` @326941, `uesieco-…-aut.dat` @60410 | 11 |
| STE | not spelled out | an application type (`TipoAplicativo ste(5)`). The file suffix `-ste` is something else, see §10 | 11 |
| TPM | *Trusted Platform Module* | the security chip whose certificate identifies the media generator | 8 |
| TRE | *Tribunal Regional Eleitoral* | state electoral court (@475104) | 2 |
| TSE | *Tribunal Superior Eleitoral* | the federal electoral court; in lower case in the source paths `/home/rubio/tse/…`, in upper case only at the end of three package-type names (`pacoteCandidatosTSE`…) | 2 |
| TTE | Transferência Temporária *de Eleitores* (@228762 `'TTE: Transferência Temporária'`) | voters moved to another section for election day | 7 |
| UE | urna eletrônica (@369448) | the voting machine (@8309 "Código identificação UE") | 8 |
| UENUX | not spelled out (probably UE + Linux) | the urna's Linux system; source tree `uenux2` | 11 |
| UF | *unidade da federação* (TSE document) | a state (`ac`, `df`), `br` for national, `zz` for abroad | 3 |
| VOTA | Software de Votação (@123634) | the election-day voting application, i.e. this program | 11 |
| VPE | not spelled out | expected media file `NNNueNN.vpe` (@181199); `vpe99_*.jez` in GEDAI-UE | 11 |
| VPP | not spelled out | an application type (`TipoAplicativo vpp(4)`) | 11 |
| WSQ | *Wavelet Scalar Quantization* | FBI fingerprint-image format; `wsqbio.jez`, `wsqmes.jez`, `wsqman.jez` | 10 |

---

## 2. People and institutions

| term | English | where in this binary |
|---|---|---|
| **TSE** (Tribunal Superior Eleitoral) | federal electoral court; it writes the urna software | Never a string of its own. In lower case, 1,639 of the 1,939 `std::source_location` records point under `/home/rubio/tse/uenux2/…` (`grep -c '/home/rubio/tse/' analysis/srcloc.tsv`). In upper case it only ends three ASN.1 value names of `ModuloTiposPacotes.TipoPacote`: `pacoteCandidatosTSE(31)` @330535, `pacoteFotosCandidatosTSE(32)` @330510, `pacoteAlteracaoCandidaturaTSE(33)` @330555 (`grep -P '\t[^\t]*TSE' analysis/strings.tsv`) |
| **TRE** (Tribunal Regional Eleitoral) | one electoral court per state; its technicians service the urnas | @475104 `'Por favor, solicite ajuda dos técnicos do TRE'` (func 11812 `vota::testeteclado::CEnviarManutencao::ProcessInput`, keyboard-test flow) |
| **Justiça Eleitoral** (JE) | the electoral branch of the judiciary (TSE + TREs + zones) | @155716 `'eleitor convocado pela Justiça Eleitoral'`; ASN.1 `DataHoraJE` (`"YYYYMMDDThhmmss"`) in `ModuloTiposEleitorais` |
| **cartório eleitoral** | local electoral office of a zone | @155402 `'Eleitor deve procurar cartório eleitoral'` |
| **Junta Eleitoral**, **junta / turma apuradora** | electoral board; the counting board/team that runs the SA | @369448 `'…envie-a para a Junta Eleitoral.'`; @225832 `juntaApuradora`, @225847 `turmaApuradora` |
| **eleitor** | voter | `vota::CEleitorVotando` (the voter's session), `comum::md::CEleitor` (`…/dados/md/eleitor/celeitor.cpp`), `ModuloEleitores` |
| **mesário** | poll worker at the section | `uenux2/src/app/comum/comparecimentomesario/`: the binary names 18 `.cpp` files in `estados/` alone (`grep -o 'comparecimentomesario/estados/[a-z]*\.cpp' analysis/strings.tsv \| sort -u \| wc -l`; 15 of them own functions in `analysis/files.tsv`); SQLite table `comparecimento_mesario` in `uenux.db` |
| **Mesa Receptora** | the section's team of poll workers ("receiving table") | @225817 `'Mesa Receptora'` |
| **operador** | the person at the poll worker's terminal, in code terms | `uenux2/src/app/vota/operador/…`, `vota::CThreadOperador`; @123127 `'Operador cancelou o encerramento da votação'` |
| **candidato** | candidate | see §5 |
| **SEVIN, SEINT, SECAD, SECINP** | TSE units (*seções*) that sign the data packages. SEVIN is the Seção de Voto Informatizado (00-provenance); the binary does not expand the other three | Key names returned by `comum::CPacoteArquivos::RetornaNomeChave(ESavdChaveValidar)` (func 5900), next to `CLOGI`, `CLOGI_CERT`, `CLOGI_UPDATE` and the inline constants `'SCUE'`, `'UE'`, `'PU'`. The package signers are listed in [asn1-schemas §6](data-model/asn1-schemas.md) |
| **CEPESC** | government crypto R&D centre; the TSE's file encryption carries its name | `ecourna::api::cepesc::CCepescCipher` (typeinfo 1115020). **In this build it does not encrypt**, see [u01 §4.1](modules/u01-ecourna-lib-ecourna-api-security.md) |

## 3. Places: where a vote happens

The scenario files encode the place in their names: `t02400ac0000100010001-el.dat` is fase `t`,
processo eleitoral `02400`, UF `ac`, município `00001`, zona `0001`, seção `0001`, then the kind
of file (`-el` = voters). See [asn1-schemas §6](data-model/asn1-schemas.md).

| term | English | where in this binary |
|---|---|---|
| **UF** (unidade da federação) | a state; two letters | `"uf": "ac"` in `upstream/fs/municipal-t1/cenario.json`; `df` and `zz` (voting abroad, scenario `geral-zz-t1` "Votação no exterior") in the others; `br` in `t00000br-pu.dat` (national package). `votaInit` (func 7840) reads the JSON key `uf` @168637 with fallback `"ee"` @187015, which is also the placeholder the TSE spec uses for a UF |
| **município** | municipality | @5122 `'Município inválido: {}'`; `…/dados/md/municipiozona/cmunicipio.cpp` |
| **zona (eleitoral)** | electoral zone, a group of sections under one cartório | @6530 `'Zona inválida: {}'`; `ModuloTiposEleitorais::MunicipioZona` |
| **seção (eleitoral)** | polling section: one urna, one voter list, one BU | @6431 `'Seção inválida: {}'`; `ModuloLocal::SecaoEleitoral`; `comum::CGravadorRCSecao` |
| **seção agregada** | a section merged into another one's urna | @8704 `'Seção agregada: {:04}'`, used by func 12105 `vota::CGeraRelatorios::StartState`; its other six references are false (pitfall 2); `ModuloLocal.SecaoEleitoral.agregadas` |
| **local de votação** | polling place (a school, for example) holding several sections | @4776 `'Tipo de local de votação inválido: {}'`; `0000100010001-lo.dat` (`ModuloLocal.Local`, name built from @60623 `"{:05}{:04}{:04}-lo.dat"`) |
| **tipo de local** | kind of polling place | `ModuloTiposCadastro.TipoLocalVotacao { normal(1), emTransito(2), presoProvisorio(3), temporario(4) }` |
| **abrangência** | scope of an election or package: municipal, estadual (state) or federal | `ModuloTiposEleitorais.TipoAbrangencia { municipal(0), estadual(1), federal(2) }`; @5508 `'Tipo de abrangência inválido: {}'` |
| **exterior** | abroad (Brazilian voters outside the country) | UF `zz`; @85932 `EhExterior` ("is abroad") |
| **cabina** | voting booth | @226226 `'Inspecione cabina e urna'` ("inspect booth and urna") |

## 4. The election: processo, pleito, eleição, turno, fase

| term | English | where in this binary |
|---|---|---|
| **processo eleitoral** (PE) | the whole election event (e.g. the 2026 general elections), with a numeric id | `"pe": 2400` (municipal) or `2500` (general) in `cenario.json`; @8457 `'Processo eleitoral: {:05}'`; `comum::CPE`, `…/md/processoeleitoral/cprocessoeleitoral.cpp` |
| **pleito** | one voting day of a processo; the 1st and 2nd rounds are two pleitos | @8443 `'Pleito: {:05}'`; `…/md/processoeleitoral/cpleito.cpp`; in `municipal-t1` pleito = 2410 |
| **eleição** | one election inside a pleito (e.g. the municipal election of one UF) | 2411 in `municipal-t1` (`t02411ac-ce.dat` = `ModuloEleicao.EntidadeEleicao`); `ModuloTiposEleitorais.TipoEleicao { ordinaria(0), extraordinaria(1), consultaPopular(2) }` |
| **eleição ordinária / extraordinária** | regular election / special election (e.g. after an annulled one) | `TipoEleicao` above |
| **eleições gerais / municipais** | general elections (president, governor, congress) / municipal elections (mayor, council) | scenario labels `"Eleições Gerais - 1º turno"`, `"Eleições Municipais - 1º turno"` |
| **turno** | round. The 2nd round (*segundo turno*) is a runoff for majoritarian offices | `ModuloTiposEleitorais.Turno { semTurno(0), turno1(1), turno2(2) }`; @7554 `'Urna em 2º turno sem dados de pleito 2: {}'`; package `turno2.jez` @9375 |
| **fase** | "phase" of a whole set of data: `oficial` (real election), `simulado` (rehearsal), `treinamento` (training) | ASN.1 `Fase { simulado(1), oficial(2), treinamento(3) }`. File names start with `o`, `s` or `t`. Banner strings @337308 `'==============TREINAMENTO============='`, @337386 `'…SIMULADO…'`. The simulator only ships `t` data. See the note below the table |
| **origem da configuração: oficial / comunitária** | whether the configuration is for an official election or a *comunitária* (non-official) one | `ModuloProcessoEleitoral.OrigemConfiguracao { oficial(1), comunitaria(2) }`; printed as `ORLC:LEG` / `ORLC:COM` in the BU QR code (build-a-bu) |
| **consulta popular** | referendum / plebiscite. The code treats its question as a *cargo* | `TipoEleicao.consultaPopular(2)`, `TipoCargoConsulta.consulta(3)`; @222527 `telaVotoNuloConsulta` |
| **totalização** | adding up all BUs centrally (outside the urna) | @142002 `pacoteResultadoTotalizacao`; `Sistema.sistot(11)` is probably the totalisation system |

**Note: `fase` in the wasm.** Look at two places:

* `votaInit` (func 7840) reads the JSON key `fase` @177525, with `"te"` @176379 as the fallback.
  It compares the value with `"oficial"` @157370 or `"o"` @142797 (result 49 = `'1'`), then
  with `"simulado"` @139917 or `"s"` @81614 (result 50 = `'2'`). Anything else gives 51 = `'3'`.
  So the site's `"te"` means training by default, not because the program knows `"te"`.
* `FormataFase(EUrnaFase)` (func 2828, `nomearquivo/cnomearquivo.cpp:38`) turns `'1'..'3'` into
  the file-name letter with a small lookup table packed into one `i32` constant:

```wasm
local.get 1          ;; EUrnaFase, a char '1'..'3'
i32.const 49
i32.sub              ;; index = fase - '1'
local.tee 1
i32.const 3
i32.ge_u
if                   ;; index >= 3 -> throw "Fase inválida: {}" (@6512)
…
local.get 0          ;; the std::string being returned (through a pointer)
i32.const 7631727    ;; 0x0074736F: the bytes 'o','s','t' in little-endian order
local.get 1
i32.const 3
i32.shl              ;; index * 8
i32.const 16777208   ;; 0xFFFFF8
i32.and
i32.shr_u            ;; shift the wanted letter into the low byte
i32.store8           ;; keep that byte: 'o' (oficial), 's' (simulado) or 't' (treinamento)
```

The C++ `EUrnaFase` (`'1'` = oficial) is numbered differently from the ASN.1 `Fase` (`1` = simulado).
Two enums with the same value names do not have to share numbers, so check each one.

## 5. Offices, candidates, parties

| term | English | where in this binary |
|---|---|---|
| **cargo** | office being voted for (mayor, councillor…) | `ModuloTiposEleitorais.CargoConstitucional { presidente(1) … vereador(13) }`; `…/md/processoeleitoral/ccargo.cpp`; @1810 `'Cargo {} não encontrado para eleição {}'` |
| **presidente, governador, senador, prefeito** | president, state governor, senator, mayor | the *majoritário* cargos of the scenarios |
| **deputado federal / estadual / distrital, vereador** | federal deputy / state deputy / Federal District deputy / city councillor | the *proporcional* cargos |
| **vice** | running mate (vice-president, vice-governor, vice-mayor) | `CargoConstitucional.vicePresidente(2)`, `viceGovernador(4)`, `vicePrefeito(12)` |
| **suplente** | substitute; senators are elected with two | `primeiroSuplenteSenador(9)`, `segundoSuplenteSenador(10)`; @5873 `'Suplente inexistente: {}'`; @73414 `'Sem suporte a mais de 2 suplentes'`; @8210 `{candidato-com-suplentes}` |
| **majoritário / proporcional** | winner-takes-all office / office shared out in proportion to party votes | `ModuloTiposEleitorais.TipoCargoConsulta { majoritario(1), proporcional(2), consulta(3) }`; the states `vota::CPedeMajoritario`, `vota::CPedeProporcional` |
| **número de dígitos** | how many digits the voter types | Seen at run time: Presidente 2, Governador 2, Senador 3, Deputado Federal 4, Deputado Estadual 5 (`geral-t1`); Prefeito 2, Vereador 5 (`municipal-t1`). The state JSON reports them as `"digits"` |
| **candidato / candidatura** | candidate / a candidacy (a candidate for a cargo) | `ModuloCandidatos`; `…/md/candidatura/ccandidatura.cpp`; @1379 `'Candidato não encontrado: {}/{}'` |
| **apto / inapto** | (candidacy) valid / not valid; a vote typed for an *inapto* candidate is not stored as a nominal vote | `SituacaoCandidatura { …, inapto(3), …, apto(12), … }` (17 values); `"apt": true` in the state JSON (`--full`; every candidate of the shipped scenarios is apt). The number gets its own screen, @124633 `'Tela de Candidato Inapto'`, class `vota::CCandidatoInapto` (RTTI name `CConfereVotoEmCargo<CCandidatoInapto, (ETelaVotacao)14>`), and the RDV check @222091 `'…voto nominal ({}) para candidatura inapta'` (func 11514) treats a nominal vote for an inapto candidacy as an error |
| **partido** | political party; its number is the first two digits of its candidates' numbers | `ModuloPartidos`, @67438 `EntidadePartidos`. In `municipal-t1`, candidate `91001` "Golfe" has `"party": 91` (`node tools/run/headless.mjs --scenario municipal-t1 --full --keys "9"`) |
| **federação (partidária)** | federation of parties that act as one for the election | `ModuloFederacoes`; @235226 `'o partido {} fazia parte das federações [{}] e [{}]'` |
| **coligação** | electoral coalition | **0 hits** in `strings.tsv` and `classes.tsv` |
| **legenda** | a party's number/label as a vote target (see *voto de legenda*) | `"legendaDigits": 2` for Vereador (`--full` state JSON); @335222 `'VOTO DE LEGENDA'` |
| **votável** | anything that can receive a vote: a candidate number or a party number | @154709 `identificacaoVotavel`, a BU member of type `IdentificacaoVotavel { partido, codigo }`; @154695 `NumeroVotavel`, the ASN.1 type name the TSE spec gives to `codigo` (referenced from `__wasm_call_ctors`, which builds the ASN.1 tables) |

## 6. Casting a vote

You can watch every kind of vote in the real program:

```sh
node tools/run/headless.mjs --scenario municipal-t1 --keys "91C  C  B  C  "
```

This prints the following (shortened):

```
state: …"substate":"vota::CPedeProporcional"…"cargo":"Vereador","digits":5
key 1 -> …"vota::CPedeNominal"…"voteMode":"legendaOuNominal","legendaValida":true
key C -> …"vota::CConfereVotoEmCargo<vota::CConfirmaVotoLegenda, (vota::ETelaVotacao)10>"…"voteMode":"legenda"
pause -> …"vota::CConfirmaVotoLegenda"…
key C -> …"vota::CPedeMajoritario"…"cargo":"Prefeito","digits":2
key B -> …"vota::CConfereVotoEmCargo<vota::CMajoritarioBranco, (vota::ETelaVotacao)4>"…"voteMode":"branco"
pause -> …"vota::CMajoritarioBranco"…
key C -> {"state":"vota::CSincronismoEleitor",…"voteMode":"gravando"…}
pause -> {"state":"vota::CAguardaMensagem","done":true,…"voteMode":"fim"…}
```

After typing `91`, the program already knows that 91 is a valid party (`legendaValida: true`). It
waits for either three more digits (a nominal vote) or CONFIRMA (a party-only vote).

| term | English | where in this binary |
|---|---|---|
| **voto nominal** | vote for a candidate, by typing their full number | `vota::CPedeNominal`, `vota::CConfirmaVotoNominal`; @156611 `VotoNominal`; @441748 `'Voto nominal para o cargo '` |
| **voto de legenda** | party-only vote: type the 2-digit party number and confirm (proportional cargos only) | `vota::CConfirmaVotoLegenda`; @156594 `legendaOuNominal`; @230592 `'    Votos de legenda'` (BU line); the RDV check @156380 `'…voto de legenda ({}) com cargo que não é proporcional'` |
| **voto em branco** | blank vote (the BRANCO key) | `vota::CMajoritarioBranco`, `vota::CProporcionalBranco`; @140660 `'Tela de Voto Branco'` |
| **voto nulo** | null vote: confirming a number that belongs to no candidate or party. There is no NULO key | `vota::CPedeNulo` (`votaproporcional/cpedenulo.cpp`); in `geral-t1`, if the first two digits for Deputado Estadual match no party (`12`, `22` or `02`), the state switches to `CPedeNulo`, `voteMode "nulo"`, as soon as the second digit is typed: `--scenario geral-t1 --keys "91C  C  12  "` |
| **cargo sem candidato** | an office with no valid candidate; its vote is recorded as null | BU `TipoVoto.cargoSemCandidato(5)`; RDV `nuloCargoSemCandidato(8)`; @127526 |
| **tipo de voto** (BU / RDV) | the vote categories in the result files | BU `TipoVoto { nominal(1), branco(2), nulo(3), legenda(4), cargoSemCandidato(5) }`; RDV `TipoVoto` has 9 values, adding `brancoAposSuspensao`, `nuloAposSuspensao`, `nuloPorRepeticao` and the "cargo sem candidato" cases ([src/asn1/ModuloRegistroDigitalVoto.asn](../src/asn1/ModuloRegistroDigitalVoto.asn)) |
| **CONFIRMA / CORRIGE / BRANCO** | the urna's three command keys: confirm (green), correct (orange), blank (white); the page draws them as `.btn-con` `#4da94f`, `.btn-cor` `#fe5100`, `.btn-bra` `#f7f7f7` in `upstream/site/css/vota-wasm.css` | headless keys `C`, `D`, `B`; @91180 `'CONFIRMA: habilitar'`, @91157 `'CORRIGE: não habilitar'` |
| **tecla indevida** | wrong key (pressed when not accepted) | @232379 `'Tecla indevida pressionada'` (func 1455 `vota::CVotacaoStateAudio::PlayKey`); also written to `logd.dat` |
| **tela** | screen. `ETelaVotacao` numbers the voting screens | seen at run time in `CConfereVotoEmCargo<…, N>`: 2, 4, 7 and 10 are the *conferência* screens of a nominal, blank, null (`CProporcionalNulo` for Vereador `12345`, `CMajoritarioNulo` for Prefeito `12`) and party-only vote (`vota::NomeTela`, func 6718: "Tela de Conferência de Voto Nominal / Branco / Nulo / Legenda"); the screens that follow are 1, 3, 6 and 9 (08 §11 item 6) |
| **suspensão** | the poll worker interrupts a voter who left without finishing | @124993 `'Eleitor foi suspenso e não confirmou nenhum voto'`; `ModuloParametrizacaoUrna.FormaSuspenderComVoto { tornaOutrosBranco(1), tornaOutrosNulo(2), tornaNaoVotouParcial(3) }` |
| **voto confirmado** | vote confirmed (log line) | @235278 `'Voto confirmado para [{}]'` (func 4537) |
| **colinha (eleitoral)** | "cheat sheet": the voter's own paper list of candidate numbers | **only in the web page**, not in the wasm: `upstream/site/eleitor-escolhas.html` ("Sua Colinha Eleitoral"), stored in `localStorage` key `escolhas` by `upstream/site/js/eleitor-escolhas.js` |

After a vote, the only file that changes is the log `dsk/fi/dinamico/log/logd.dat`, with Latin-1
lines such as `1|1|Voto confirmado para [Vereador]` (runtime README).

## 7. Identifying the voter, attendance, justification

| term | English | where in this binary |
|---|---|---|
| **título de eleitor** | voter registration card; its number (*número de inscrição eleitoral*, 12 digits) identifies the voter | @82819 `'Título de eleitor'`; `ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp`; `numero_titulo BIGINT` in `uenux.db`; example `XXXXXXXXXXXX` from `geral-t1` ([u03](modules/u03-uenux2-src-app-comum-dados.md)) |
| **identificador do eleitor** | which number identifies a voter | `ModuloTiposEleitorais.TipoIdentificadorEleitor { numeroInscricaoEleitoral(1), numeroCPF(2), numeroLivre(3) }`; the `CHECK(tipo_identificador IN (1,2,3))` in `uenux.db` |
| **CPF** | the national taxpayer number, an alternative voter id | `ecourna-lib/ecourna/app/dados/cnumerocpf.cpp` |
| **habilitação / habilitar** | the poll worker "enables" the urna for one voter after identifying them | @138039 `'Eleitor foi habilitado'` (func 7481 `vota::CAguardaMensagem::ProcessMessage`, also in `logd.dat`); @123014 `'Eleitor não habilitado para votação'` |
| **habilitação biométrica** | enabled by fingerprint match | @6831 `'Habilitação biométrica: {}'`; QR field `HBBM` @440139 |
| **habilitação biográfica** | fingerprint not recognised, so the voter is enabled after giving their birth year | @6858 `'Habilitação biográfica: {}'`; @338785 `'Digite o ANO de nascimento:'`; QR fields `HBBG` @440211 and `HBSB` @440310 (qrcode) |
| **biometria / digital / dedo** | biometrics / fingerprint / finger | @3799 `'Mais de dez dedos na biometria do eleitor: {}'`; `ModuloEleitores.ComposicaoBiometria { foto(1), dedos(2), fotoDedos(3) }`; `TipoDedo` (10 fingers, plus `naoIdentificado(0)`). The reader is mocked by `simulador::CFingerPrepareSimulador` |
| **terminal do mesário / terminal do eleitor** | the poll worker's keypad unit / the voter's unit (screen and keypad) | @365472 `'Siga as instruções no terminal do mesário.'`; @82666 `'Instruções no terminal do eleitor'`; threads `vota::CThreadOperador` and `vota::CThreadEleitor` |
| **comparecimento** | turnout / attendance | `ModuloResultadoUrnaCadastro.SituacaoComparecimentoEleitor { faltou(1), naoVotou(2), votou(3), semCargoParaVotar(4) }` |
| **eleitores aptos** | registered voters of the section | @445790 `'Eleitores aptos                   {:04}\n'`; QR field `APTO` |
| **faltosos** | voters who did not show up | @66128 `'Eleitores faltosos'` (func 12110 `vota::CGeraBU::StartState`) |
| **impedido / impedimento** | voter barred from voting here, and the reason | `ModuloImpedidos.TipoImpedimento` (15 values, e.g. `suspenso`, `semIdadeMinima`, `militarEmServico`); @122762 `'está impedido de votar nesta seção'`; file `-imp.dat` |
| **justificativa (eleitoral)** | a voter away from home declares why they cannot vote, at any urna | @155797 `'Boletim de Justificativa Eleitoral'`; `comum::CJustificador::Justifica`; SQLite table `registro_justificativa (numero_titulo, ano_nascimento)` |
| **TTE** (transferência temporária de eleitores) | voter moved to another section for this election | @228762 `'TTE: Transferência Temporária'` (func 11908 `vota::CImpressaoListaEleitores::StartState`); `TipoTransferenciaTemporaria` (9 reasons); file `-tte.dat` |
| **voto em trânsito** | voting outside one's home section, requested in advance | @126224 `'Voto em Trânsito'`; `TipoTransferenciaTemporaria.votoEmTransito(1)` |
| **preso provisório** | detainee not yet convicted, who still votes | @130651 `'Preso provisório'`; `TipoLocalVotacao.presoProvisorio(3)` |
| **acessibilidade / áudio** | accessible voting; the urna reads the screens aloud through headphones | @192094 `'eleitor pediu acessibilidade'`; `ModuloEleitores.NecessidadeEspecial.necessitaAudio(2)`; the RHVoice voice Letícia-F123 (rhvoice) |
| **caderno de votação** | the printed voter roll that voters sign | @192272 `'Assinar o caderno de votação antes de'`; @122981 `'Não assinar o caderno de votação'` |
| **registro de mesários** | poll workers identify themselves (título + fingerprint) at opening and closing | EstadoVota `registromesarioinicial(6)` / `registromesariofinal(9)`; `comum::CEncerraRegistroMesarios` |

## 8. The machine, its memories and its media

| term | English | where in this binary |
|---|---|---|
| **urna (eletrônica)**, **UE** | the electronic voting machine | @8309 `'Código identificação UE       {:08}'` (funcs 1942, 5591); `ModuloTiposResultadosEcoUrna::Urna` |
| **modelo** | hardware model, by year | `ModuloTiposEcoUrna.ModeloEquipamento { tpm20(2), ue2013(13), ue2015(15), ue2020(20), ue2022(22) }` |
| **flash interna (FI) / mídia interna (MI)** | the urna's built-in flash memory | path `/dsk/fi/` @358629. `comum::CLogComum::LogaCopiandoArqResParaFI` (func 5878) logs @235731 `'Copiando arquivo de resultado para MI: [{}]'`, which shows MI = FI. @334922 `'FALHA NA MI: SUBSTITUA A URNA'` (you replace the urna, not the memory) |
| **flash externa (FE) / mídia externa (ME)** | the removable flash card in the urna | path `/dsk/fe/` @358702. `LogaResultadoCopiadoResFE` (func 3828) logs @235775 `'…para ME: [{}]'`. @331065 `'FALHA NA ME: SUBSTITUA A ME'` |
| **mídia de votação (MV)** | the card that holds the vote data. On a failure it is moved to a contingency urna | @362075 `'…substitua a mídia de votação.'`; @370017 `'Desligue a urna, insira uma mídia de votação usada de outra urna que apresentou problema…'`; `CUrna::ValidaCriacao` (func 5862) checks @319987 `'Serial da MV inválido ['`. The monitor thread reports MV next to MI (@235442/@235494, func 10226), so MV is most probably the external card seen from its role |
| **flash de votação (FV)** | the voting card, as the ASN.1 media type names it (messages say MV) | `TipoMidia.fv(3)`; the scenario's medium description is `infomidia-fv-1-t.dat` (`ModuloInformacaoMidia.InformacaoMidia`) |
| **flash de carga (FC) / mídia de carga (MC)** | the card used to load the election data into urnas | `TipoMidia.fc(2)`; @8376 `'Código identificação MC       {:.8}'`; @367431 `'Serial da mídia de carga inválido.'` (func 11423 `comum::asn::CConversorDadosDisponiveisCarga::DoDesconverte`); @320011 `'Serial da MC inválido ['` (func 5670 `comum::md::CCarga::ValidaCriacao`) |
| **mídia de resultado (MR)** | removable medium that takes the results to the transmission point | `TipoMidia.mr(1)`; path `/dsk/mr/` @358550; @137869 `'Retire a mídia de resultado'`; state `vota::CRetirarMR`; `mr.ver` @86764 |
| **mídia de aplicação** | medium with the application software | @362203 `'…substitua a mídia de aplicação.'` |
| **estático / dinâmico** | static data (loaded election packages) / dynamic data (state written during the day) | `/dsk/fi/estatico/…` (scenario files), `/dsk/fi/dinamico/…` (`eg.bin`, `log/logd.dat`); `/dsk/fi/estatico/chave/` @358678 holds keys |
| **trab1 / trab2** | two "work" directories for the state files | `analysis/runtime/memfs-after-init/dsk/fi/dinamico/trab1/` holds `gap.bin`, `sa.bin`, `vota.bin`, `rdv.dat`, `uenux.db` and their `.vsu`; after `votaInit`, `trab2/` holds copies of the three `.bin` files and the `.vsu` files but no `rdv.dat` or `uenux.db`. `eg.bin` sits one level up, in `dinamico/` |
| **serial** | serial number of a medium | `serialv.dat` @60380, read by `votaInit`; in the simulator it contains `ABCDDCBA` |
| **bateria interna / externa** | internal battery / external battery | @136713 `'Tempo limite de uso de bateria interna inválido'` (`vota::CAjusteInicial::ValidaTemposDesligamento`, inlined in func 7160 `vota::CAjusteInicial::StartState`); `comum::util::CMonitoraAlimentacao` (`comum/util/cmonitoraalimentacao.cpp`) |
| **urna de contingência** | spare urna that replaces a broken one in the same section | @229763 `'Urna de contingência'` (func 1543); `ModuloTiposResultadosEcoUrna.TipoUrna { secao(1), contingencia(3), contingenciaSecao(4), contingenciaEncerrandoSecao(6) }`; `EstadoGeralUrna.TipoUrnaOperacao` |
| **HSM** (hardware security module) | crypto chip that holds the urna's secrets | @7762 `'Falha ao enviar ação ao HSM. {}'` (func 5889 `comum::EnviarAcaoHSM`); interface `api::IKernelHSM`. The key of the código verificador is wrapped with an HSM secret (codigo-verificador) |
| **TPM** (trusted platform module) | security chip in the machine that generates the media | @327351 `serialCertificadoTPM`, a member of `ModuloTiposEcoUrna.IdentificadorGeradorMidia { nome, serialCertificadoTPM, serialInstalacao }`; in the simulator's `infomidia` the TPM serial is all zeros ([asn1-schemas §7](data-model/asn1-schemas.md)) |

## 9. The election day, state by state

The urna's own enumerations give the timeline ([src/asn1/](../src/asn1/)):

* `ModuloEstadoGeralUrna.EstadoUrna { carregando(1), carregada(2), testada(3) }`: being loaded,
  loaded, tested.
* `ModuloEstadoGeralVota.EstadoVota`: `inicial(0)`, `gerabasedinamica(1)`, `aguardahorazeresima(2)`,
  `gerarze(3)`, `zeresimagerada(4)`, `zeresimaimpressa(5)`, `registromesarioinicial(6)`, `votar(7)`,
  `fimaquisicaovotos(8)`, `registromesariofinal(9)`, `gerarbu(10)`, `gerarrelatorios(11)`,
  `imprimirbu(12)`, `gravarresultados(13)`, `copiaresultadosmr(14)`, `encerrada(15)`,
  `exibealertadesligamento(16)`.
* `ModuloEstadoGeralVota.EstadoEncerramento`: `inicial(0)`, `imprimirobrigatoriabu(1)`, `retirarmr(2)`,
  `fimdostrabalhos(3)`.

| term | English | where in this binary |
|---|---|---|
| **carga** | loading the election data into an urna, days before the election | `ModuloTiposEcoUrna::Carga`; `…/md/correspondencia/ccarga.cpp`; @8983 `'Data da carga               {:.10}'`; `dadoscarga.dat` @60735 |
| **código (de identificação) da carga** | id of one load. It is printed on the reports and is 24 digits long in the QR code (`IDCA:`) | @230358 `'Código de identificação da carga'`; @230329 `'Histórico de código de carga'` |
| **correspondência** | the link between an urna and its section after the load | `…/dados/md/correspondencia/`; `tabcorr.dat` @60601 |
| **inspeção** | a poll worker inspects the booth and the urna; the poll worker's terminal starts it and waits for it to end | @226226 `'Inspecione cabina e urna'` (func 10676 `vota::CPedeIdentidade::ProcessTick`); states `vota::CInspecionaUrna`, `vota::CAguardaInspecao`, `vota::CConfirmaInspecionada`; @232454 `'Inspeção da urna terminada'` |
| **zerésima** | report printed before voting starts, showing every candidate at zero votes | @336236 `'Quer imprimir a zerésima?'`; `vota::CImprimindoZeresima`, `vota::CGeraZeresima`; envelope `envelopeZeresimaImpressa(6)`; files `imgze.dat` @60670 and `rze.dat` (summary, *resumo*) @60662 |
| **abertura / início da votação** | opening of the voting | `…/vota/eleitor/iniciovotacao/` |
| **fim da aquisição de votos** | the point after which no more votes are taken | EstadoVota `fimaquisicaovotos(8)` |
| **encerramento (da votação)** | closing of the voting | @123171 `'Encerramento da votação'`; @77708 `'Encerramento só pode ser solicitado após as {} horas'`; `…/vota/eleitor/fimvotacao/` |
| **fim dos trabalhos** | end of the section's work (after the MR is removed) | `EstadoEncerramento.fimdostrabalhos(3)` |
| **emissão / reemissão** | printing / reprinting (of a report) | @227344 `'Emissão de zerésima'`, @227364 `'Reemissão da zerésima'` |
| **simulado / treinamento** | rehearsal / training modes. The urna application id encodes them | `ModuloEstadoGeralGap.UrnaAplicativo { apvotatreinaeleitor1(0), apvotatreinamesario1(2), apvotasimula1(4), apvotaoficial1(6), apapuracaotreina1(8), … }` |

## 10. Output: reports, result files and their protection

The TSE's own table of output files (suffix, content, which system writes it) is in
[`upstream/tse-docs/bu-rdv-format-v2/README.md`](../upstream/tse-docs/bu-rdv-format-v2/README.md).
Output file names follow `fpppppuuMMMMMZZZZSSSS-suf.ext`. The wasm builds them from
`'{:c}{:05}{}{:05}{:04}{:04}-'` @378043 (funcs 1164, 3798), and takes the suffixes (`rdv.dat`,
`imgbu.dat`, `rdvred.dat`, `imgze.dat`…) from the table in `comum::CArquivosResultado::operator[]`
(func 347).

| term | English | where in this binary |
|---|---|---|
| **BU** (boletim de urna) | the urna's tally for its section, printed several times and written to the MR | `ModuloBoletimUrna.EntidadeBoletimUrna`; `vota::CGeraBU`, `comum::CGravadorBU`; `bu.dat`, `imgbu.dat` @60400 (image of the printed BU); `busa.dat`, `imgbusa.dat` @60709 when the SA writes it. See build-a-bu |
| **via / cópia do BU** | printed copy; the parameters set how many are mandatory (*obrigatórios*) and how many extra (*adicionais*) may be printed | @226157 `'Emitir mais cópias do boletim de urna'`; `vota::CVerificaQtdBUsAdicionais`; @66511 `'BUs do RED obrigatórios'`, @72090 `'BUs do RED adicionais'` |
| **QR code do BU** | the BU's data as QR codes at the foot of the printout | @322396 `'Imprimir QR Code no BU'`; `comum::CGeradorBUQRCode`; see qrcode |
| **código verificador** (CV) | check number printed under each BU block; a keyed SipHash-4-6 MAC | @1544 `'Código Verificador: {}.{}.{}.{}'` (func 11182); MAC in func 9498 `CSiphashMac::DoMac`; see codigo-verificador |
| **RDV** (registro digital do voto) | every vote, kept sorted by (vote type, digits typed) so that the voting order is lost and no vote can be linked to a voter | `ModuloRegistroDigitalVoto`; `comum::CRdv`, `comum::CRdvPosicionadorVota` (*posicionador* = the part that picks the position: a binary search, `upper_bound`, func 11496); `rdv.dat` @60392 (80 bytes after `votaInit`), `rdvred.dat` @60689 when the RED writes it |
| **BIM** | boletim de identificação de mesários: list of registered poll workers | @66858 (func 12105); `vota::CImprimindoBim`; `bim.dat` @60646 |
| **BEHB** | boletim de eleitores habilitados biograficamente | @485744 (func 12074); `behb.dat` @60700 (worked example in §0) |
| **BUJ / BJust** | boletim de justificativa | `comum::CGeradorBUJ::GeraBUJ`, `vota::CImprimirBJust`; `buj.dat` @60654 |
| **PU** (parametrização da urna) | package of urna parameters (how many BU copies, when the zerésima may print…) | `t00000br-pu.dat`, `t02400ac-pu.dat` (`ModuloParametrizacaoUrna.EntidadeParametrizacaoUrna`); `vota::CImpressaoPU::StartState` (func 11902) prints parameters such as @66511 `'BUs do RED obrigatórios'` |
| **relatório** | report | `vota::CGeraRelatorios`, `uenux2/src/app/comum/relatorios/` |
| **log** | the urna's event log | `logd.dat` @60680 (Latin-1 text), packed as `log.jez` @9209; `comum::CLogComum`, `vota::CLogVota` |
| **jufa.dat** | TSE: "registro de comparecimento de eleitores e mesários" (attendance of voters and poll workers) | `jufa.pk1` @353405 is the public key loaded by `comum::CGravadorRCSecao::LeChavePublica` (inlined in func 11616, `CGravadorRCSecao::GravaResultado`); that class writes the CEPESC-wrapped attendance data ([u01](modules/u01-ecourna-lib-ecourna-api-security.md)) |
| **wsqbio / wsqman / wsqmes** | fingerprints of voters enabled by biometrics / enabled manually / of poll workers (WSQ images) | @9187, @9198, @9176; `comum::CGravadorWSQ`; see compression §2 |
| **hash / hashes** | file digests written with the results | `comum::CGravadorHashes`; @65529 `hashesArquivos`; `hash.dat` in the TSE table |
| **mr.ver** | versions of the ASN.1 packages on the MR | @86764 |
| **assinatura** | digital signature | `comum::CAssinador::AssinaArquivosResultado`; package signatures `*.vsc` (algorithm `cepesc`); result signatures `vota.vsc`, `red.vsc` @207089, `sa.vsc` @207097 |
| **.vsu** | signature of a state file inside the urna | `eg.vsu`, `vota.vsu`, `rdv.vsu`… (@17876–@18012). In this build each one holds `'assinatura simulada para vota_web_wasm'` @149594 (func 11733) |
| **.vst** | signature files next to the software packages in the GEDAI-UE list | `vota_apl.vst`… in `pc1geda.html`; 0-byte stub `upstream/fs/vota_web_wasm/etc/avetc.vst` |
| **.jez** | a ZIP archive | see compression §2 |
| **.pid / pacote** | package header / package: election data is delivered as signed packages | `ModuloTiposEleitorais.CabecalhoPacote`; `ModuloTiposPacotes.TipoPacote` (`pacoteEleitores(1)`, `pacoteCandidatosTRE(7)`…); decoded by hand in [asn1-schemas §6.1](data-model/asn1-schemas.md) |
| **-ste.dat** | *situações das eleições*: which elections are active, suspended or cancelled (not the STE application) | `t02410ac-ste.dat` = `ModuloSituacoesEleicoes.EntidadeSituacoesEleicoes`; `TipoSituacaoEleicao { ativa(0), suspensa(1), cancelada(2), excluida(3) }` |
| **envelope** | generic signed wrapper around a result file | `ModuloEnvelopeGenerico.TipoEnvelope { envelopeBoletimUrna(1), envelopeRegistroDigitalVoto(2), envelopeBoletimUrnaImpresso(4), envelopeImagemBiometria(5), envelopeZeresimaImpressa(6) }` |
| **CEPESC** | the TSE's encryption for result files | `ecourna::api::cepesc::{CCepescCipher, CPlainText, CCipheredOut, CInfoSalt}`; an identity function in this build ([u01 §4](modules/u01-ecourna-lib-ecourna-api-security.md)) |
| **chave** | key | `/dsk/fi/estatico/chave/` @358678; `ModuloEnvelopeChave.EntidadeChave`; `cv.ber.pri` (codigo-verificador) |

## 11. Software and systems

| term | English | where in this binary |
|---|---|---|
| **VOTA** (Software de Votação) | the voting application: what runs on election day in each urna | `main` (func 10307) calls `api::CApplication::InitApplication` (func 11159) with `"VOTA Web"` @221349, `"Software de Votação"` @123634 and `"10.23.0.1 - DESENVOLVIMENTO"` @326597. `'VOTA'` @517095 is the `ORIG:` value of the BU QR code. `q.py s 517095` shows no user, because func 11242 `comum::CGeradorBUQRCodeVota::PreencheCabecalho` passes it as one packed `std::string_view` constant, `17180386279` = `4 << 32 \| 517095` (length 4, address), next to `"RED"` @517100 (qrcode). `TipoAplicativo.vota(1)` |
| **versão** | software version; real releases carry a name | this build `10.23.0.1 - DESENVOLVIMENTO` ("development"); the real 2024 BUs say `9.29.0.1 - Kayapó` (00-provenance) |
| **SA** (Sistema de Apuração) | counting system used with paper ballots (*cédulas*) or when the MR cannot be read | `TipoAplicativo.sa(2)`; `sa.bin` = `ModuloEstadoGeralSA` (states such as `apurarmajoritaria`, `buproporcionalpedepartido`); `TipoApuracao { totalmenteManual(1), totalmenteEletronica(2), mistaBU(3), mistaMR(4) }`; @322654 `'BU SEÇÃO SA INT'` |
| **cédula** | paper ballot | @228360 `cedula`; `ModuloRegistroDigitalVoto.OrigemVotosSA { cedula(1), rdv(2), bu(3) }` |
| **apuração** | counting (of one section, by the SA) | `ModuloTiposResultadosEcoUrna.MotivoApuracao*` (reasons such as `urnaComDefeito`, `indisponibilidadeFlashContingencia`) |
| **RED** (Recuperador de Dados) | system that recovers the results from a failed urna's memory | @7981 `'Sistema Recuperador de Dados\n{}'` (func 12105); `TipoAplicativo.red(3)`; `ORIG:RED` in QR codes; `red.vsu` @17977 |
| **GAP** | (not spelled out) the urna's application-selection state: which app runs now and which ran before | `gap.bin` @147746 = `ModuloEstadoGeralGap.EstadoGeralGap { appId, appAnteriorId, executadoRED, identificadoATUE, audio, … }`; @122347 `'assinatura EG Gap'` |
| **EG** (estado geral) | general state of the urna | `eg.bin` @147766 = `ModuloEstadoGeralUrna`; `vota.bin`, `sa.bin`, `gap.bin` are the per-application parts; @328306 `'EG Geral MI'` |
| **VPP, ATUE, ADH, STE** | other urna applications. The binary only has their short names | `ModuloInformacaoMidia.TipoAplicativo { vota(1), sa(2), red(3), vpp(4), ste(5), adh(6), atue(7) }`; strings `'vpp'` @92414 and `'adh'` @161567 (func 1164), `'atue'` @172328. ADH is the tool that sets the clock (@363656) |
| **GEDAI-UE** | TSE Windows/Qt 6 program that prepares urna media (its full name is not in the binary) | `Sistema.gedai(6)`, string `'gedai'` @159427; hash list `pc1geda.html` (00-provenance) |
| **PADA-UE** | TSE system that exports the data packages the urna loads | `Sistema.padaUE(14)`, the `origem` of every `.pid`; the page's default scenario description reads "Base de treinamento gerada a partir dos arquivos Pada-UE." (`upstream/site/index.html`, line 46) |
| **Sistema** (ASN.1 enum) | the TSE systems that can produce a file | `ModuloTiposEleitorais.Sistema { configurador(1), intercad(2), candidaturas(3), gedai(6), urnaEletronica(7), simulador(8), parametrizador(9), geradorDeBases(10), sistot(11), seweb(12), simon(13), padaUE(14) }` |
| **UENUX / uenux2** | the urna's Linux system; `uenux2` is the source tree of its applications | `/home/rubio/tse/uenux2/src/app/{vota,comum}/…`, `src/api/…`; @320989 `versaoUENUX`; `uenux.db` @221385 (SQLite, sqlite) |
| **ecourna** | TSE C++ library shared by the urna applications (ASN.1 data, crypto, I/O) | built as a Conan package: 300 srcloc records under `/home/rubio/.conan2/…/ecourna/…`; namespaces `ecourna::api::…`, `ecourna::app::dados::…` |
| **SAVD** | (not spelled out) the service the application asks to sign and validate files and to send HSM actions | @8013 `'Falha ao validar assinatura UE … Erro SAVD: ({})'`; `comum::IInterfaceSavd`; the web build installs `(anonymous namespace)::CWasmSavd` (vtable 1526688, stored by `main`) |
| **SCUE** | (not spelled out) appears as a key name and in the load configuration | inline constant `'SCUE'` in func 5900; `scueconf-t1.dat` (lists `"pacoteunico.sh"`, [asn1-schemas §6](data-model/asn1-schemas.md)) |
| **SIECO** | (not spelled out) a registry of authentications and certifications on the media | @326941 `'REG AUTENTICAÇÕES SIECO'`, @326965 `'REG CERTIFICAÇÕES SIECO'`, `sieco-dados/` @358506 |
| **VPE** | (not spelled out) a file type the media validator expects | `'#regex#[0-9]{3}ue[0-9]{2}\.vpe'` @181199 in `comum::impl::(anonymous)::ValidaConteudo` (func 5572, `validamidia/cvalidamidia.cpp`); `vpe99_{ofi,sim,tre}.jez` in `pc1geda.html` |
| **HSF** (Hot Swap Flash) | TSE Windows tool, listed in `pc1hsfg.html` | not in the binary |
| **simulador** | the training simulator; the mock layer replaces hardware | `uenux2/mock/app/simulador/wasm/`; `simulador::CWasmThread`, `simulador::CFingerPrepareSimulador`; media generator `"simulador-votacao-ng"` |
| **manutenção** | maintenance (by TRE technicians) | `vota::testeteclado::CEnviarManutencao` |

## 12. Reading TSE identifiers

Class names are verb phrases or noun phrases in Portuguese, prefixed with `C` (class) or `I`
(interface). If you can read these words, you can read the names:

| word in a name | English | examples (`cut -f3 analysis/classes.tsv`) |
|---|---|---|
| `Pede…` | asks for (input) | `vota::CPedeNominal`, `CPedeProporcional`, `CPedeMajoritario`, `CPedeNulo`, `CPedeIdentidade`, `CPedeDigital` |
| `Confere…` | checks (a transient screen) | `vota::CConfereVotoEmCargo<CConfirmaVotoNominal, (ETelaVotacao)2>` |
| `Confirma…` | confirms | `vota::CConfirmaVotoLegenda`, `CConfirmaImpressaoZeresima` |
| `Gera… / Gerador…` | generates | `vota::CGeraBU`, `vota::CGeraRelatorios`, `comum::CGeradorBU` |
| `Grava… / Gravador…` | writes to storage | `comum::CGravadorBU`, `CGravadorHashes`, `CGravadorRCSecao`, `CGravadorWSQ` |
| `Imprime… / Imprimindo… / Impressao…` | prints / printing / printout | `vota::CImprimindoBU`, `CImprimindoBEHB`, `CImpressaoPU` |
| `Conversor…`, `Converte / Desconverte` | converter between the C++ object and its ASN.1 form, one direction each | 93 non-template `CConversor*` classes in `comum::`/`ecourna::`, e.g. `ecourna::app::dados::asn::CConversorMunicipioZona` |
| `Aguarda…` | waits for | `vota::CAguardaMensagem`, `CAguardaInspecao`, `CAguardaEleitoresVotarem` |
| `Quer…` | "do you want to…?" (a yes/no prompt) | `vota::CQuerImprimirBU`, `CQuerReimprimirZeresima` |
| `Mostra…` | shows | `vota::CMostraQRCodeBU`, `CMostraEleitorVotando` |
| `Verifica…` | verifies | `vota::CVerificaHorarioZeresima`, `CVerificaQtdBUsAdicionais` |
| `Sincronismo…` | synchronisation (between the voter and poll-worker threads) | `vota::CSincronismoEleitor`, `CSincronismoOperador` |
| `Retirar…` | remove | `vota::CRetirarMR` |
| `Encerra…` | closes | `comum::CEncerraRegistroMesarios` |
| `Tela`, `Telas…` | screen(s) | `vota::CTelasVota`, `CTelasCargo`; strings `telaVotoLegenda` @230613 |
| `Estado…` | state (both "state machine state" and "persistent state") | `comum::CAppState` is the base class of screen/state classes such as `vota::CImprimindoBEHB` and `vota::CRetirarMR` (`q.py cls`); `EstadoGeral*` |
| `dados`, `md` | data; the domain-object namespace (probably *modelo de dados*) | `uenux2/src/app/comum/dados/md/…`, `comum::md::CCargo` |
| `dao` | data access object (SQLite) | `comum::dao::CJustificadorDAO`, `ccomparecimentomesariodao.cpp` |
| `DS…` | data source (for a report field) | `comum::CRelUtil::DSCodigoVerificador`, `api::CDataText<comum::CCandidaturasDSNumero>` |
| `eleitor/`, `operador/`, `monitor/` | the voter's thread, the poll worker's thread, the monitor thread | `uenux2/src/app/vota/…`; threads `vota::CThreadEleitor`, `CThreadOperador`, `CThreadMonitor` |
| `iniciovotacao/`, `fimvotacao/` | start of voting (zerésima, tests) / end of voting (BU, reports, MR) | `…/vota/eleitor/iniciovotacao/`, `…/vota/eleitor/fimvotacao/` |
| `votamajoritario/`, `votaproporcional/` | voting for a majoritarian / proportional cargo | `cpedemajoritario.cpp`, `cpedeproporcional.cpp`, `cpedenulo.cpp` |
| `CHV`, `HorarioVerao` | daylight saving time per municipality | `comum::CHV::VerificaEntraEmHorarioVerao`; @1985 `'Arquivo de HV ({}) não contém munícipio {}'` |
| `CPE` | holder of the processo eleitoral data | `comum::CPE::CreateInst` |
| `Ue…`, `uebyte`, `ueword` | "urna eletrônica" prefix of TSE types | `ueword ecourna::api::util::CBitArray::GetValor(const ueqword, const uebyte) const` @26248 |

## 13. Words you will not find in the binary

| word | why not |
|---|---|
| **colinha** | it belongs to the web page (§6), not to the urna |
| **coligação** | 0 hits; the data model has **federação** instead |
| **TSE** as a string | only in source paths (`tse`, lower case) and as the suffix of three `TipoPacote` value names (§2) |
| **lacração** (the public *Cerimônia de Assinatura Digital e Lacração dos Sistemas*, where the TSE signs and seals the software) | an event, not code; 0 hits for `lacra` or `cerim` in `strings.tsv` |
| **FC / FV** in upper case | only the ASN.1 enum names `fc`/`fv` and the file name `infomidia-fv-1-t.dat`; messages use MC / MV / MI / ME / MR |
| **"Boletim de Eleitores Habilitados Biograficamente"** as one phrase | only the acronym BEHB and the lower-case title (§0) |

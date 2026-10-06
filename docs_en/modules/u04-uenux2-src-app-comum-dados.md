# u04: `uenux2/src/app/comum/dados`: cargos, election configuration, voters, local, DST, processo eleitoral

Unit u04 holds 83 wasm functions. The unit builder assigned them to eight original files of the directory
`uenux2/src/app/comum/dados/` (`comum` = code shared by the urna applications, `dados` = data):

| file | class | what it is in the voting process |
|---|---|---|
| `ccargos.cpp` | `comum::CCargos` | cursor over the **cargos** (offices: Presidente, Governador, Prefeito, Vereador… and referendum questions, *consultas*) of every **eleição** of the day. The voter screens walk it, and so do the BU, the RDV and the BU QR codes. |
| `cconfiguracaoeleicao.cpp` | `comum::CConfiguracaoEleicao` | configuration of the elections in this urna: *processo eleitoral*, *pleito* (the elections held on the same day), cargos, versions of the data packages, section identification. |
| `celeitordetalhe.cpp` | `comum::CEleitorDetalhe` | everything known about **one voter** (*eleitor*): static roll data, dynamic attendance data, impediments. |
| `celeitores.cpp` | `comum::CEleitores` (+ `CEleitorDado*` text sources) | the section's voter list (*cadastro de eleitores da seção*). It creates and loads the SQLite "dynamic" voter table, counts the voters who may vote (*aptos*), and records how a voter was enabled (*habilitado*). |
| `chv.cpp` | `comum::CHV` | *horário de verão* (daylight-saving time) of the município. |
| `cintegridadereferencial.cpp` | `comum::CIntegridadeReferencial` | turns a failed referential-integrity check into an exception. |
| `clocal.cpp` | `comum::CLocal` | where the urna is: a polling section (*seção*) or a contingency urna (*urna de contingência*). |
| `cpe.cpp` | `comum::CPE` | the *processo eleitoral* read from `<fase><pe>-cp.dat`, used by report headers. |

About a third of the functions (29 of 83) come from **other** original files. They ended up in u04 because they
inline or call these classes. The most important one is **func 5604, the BU QR-code generator** (22.8 KB,
`comum::CGeradorBUQRCode`). The database named it `CCargos::GetCurrentEleicaoVersaoPacote` because it inlines
that 6-line method. See §6. All of them are named in the mapping table (§10). Where the real file is owned by
another unit, the reconstruction sits in a fragment file.

Reconstructed sources:

* `src/uenux2/src/app/comum/dados/{ccargos,cconfiguracaoeleicao,celeitordetalhe,celeitores,chv,cintegridadereferencial,clocal,cpe}.{h,cpp}`
* `src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp`: BU QR codes (funcs 5603–5608, 5616, 5617, 6028)
* `src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp`: the other foreign functions, grouped by original file

Ten functions ran during the recorded votes (`analysis/runtime`): 187, 273, 332, 861, 1938, 5764, 11724,
12742, 12811 and 13550. All of them draw the voting screens (current cargo, candidate number, progress bar,
next state after typing a number) or run `CEleitores::StaticLoad` at start-up (5764).

---

## 1. Classes and relations

None of the eight classes of the unit has RTTI except `CEleitores`. They are plain singletons: a static pointer
plus a mutex, `GetInst()` and `CreateInst()`.

```
comum::CEleitores  [vmi, typeinfo @1559444, vtable @1559120: exactly ONE slot]
 ├─ api::CDataMap<md::CEleitorIdentidade, comum::CEleitorDetalhe>  (non-polymorphic, base #0) -> at +4
 │     std::map (+4 begin, +8 root, +12 size), current iterator (+16), record name "CEleitores" (+20)
 └─ comum::IDataSourceAptos            (polymorphic, base #1, 1 pure virtual, no virtual dtor) -> vptr at +0
       slot 0: std::map<md::ETipoAbrangencia, SQtdeAptos> GetQtdAptos() const   = wasm 2824
   (the vmi base list gives the declaration order, CDataMap first; the only dynamic base becomes the
    primary base at offset 0, so the layout order is the reverse of the declaration order)

std::function lambdas with RTTI (the loaders, not in u04):
   CEleitores::GetEleitoresEstaticos(const std::string&)::$_0            -> vector<md::CEleitor>
   CEleitores::GetEleitoresImpedidos(const std::vector<std::string>&)::$_0 -> vector<md::CImpedido>

non-polymorphic singletons (object size, address of the static pointer):
   CCargos 28 B @1838720   CConfiguracaoEleicao @1838752   CEleitores @1838792
   CHV 28 B @1838872       CPE 204 B @1839112              CLocal 4 B @1838900 (lazy, comum_f401)
error types: CBaseError<comum::EUeComumDadosError>  typeinfo @1528076, vtable @1528096 (thunk comum_f170)
             CBaseError<ecourna::api::pattern::EPatternErr> (singletons: 1303 "… - instancia nao criada",
                                                             1304 "… - instancia ja criada")
```

Object layouts recovered from u04's accesses (details in the headers):

| object | layout |
|---|---|
| `CCargos` | `+0 size_t m_indice`, `+4 vector<{TEleicaoID eleicao; TCargoID cargo}> m_todos`, `+16 vector<…> m_cargos` (filtered for the current voter) |
| `CConfiguracaoEleicao` | `+0 id processo` (QR `PROC`), `+20 origem` (2 = comunitária), `+28 md::CPleito` (`+28 id`, `+44 data`, `+52 vector<CEleicaoPE>` of 52 bytes, `+64 map<TEleicaoID,string>` package versions), `+140 uint8` limit of voter-datum checks, `+620…` section identification used in file names (fase, processo, UF, município, zona) |
| `CEleitorDetalhe` (204 B) | `+0 md::CEleitorDecorator` (108 B), `+108 optional<md::CEleitorDinamico>` (84 B, flag `+192`), `+196 impedimentoP1`, `+200 impedimentoP2` |
| `md::CEleitorDecorator` | `+0 sequencial`, `+4 uint16 seção`, `+8 vector<CEleitorIdentidade>`, `+24 nome`, `+36 nomeSocial`, `+48 tipo de transferência temporária` (0 = none), `+52 UF` and `+64 município` of the domicile, `+80 optional<{bytes, offset, tamanho}>` location of the encrypted biometrics (flag `+100`), `+104 tipo do identificador principal` |
| `md::CEleitorDinamico` (84 B, one row of `eleitor_dinamico`) | `+0 título`, `+16 identidade usada na habilitação`, `+32 estado de comparecimento`, `+36 tipo de habilitação`, `+40 dedo`, `+44 score`, `+46 tentativas`, `+48 tipo de ativação do áudio` (default 2), `+52 erro ao decifrar biometria`, `+56 optional título do mesário`, `+76/+80 apresentação da foto` |
| `CEleitores` | `+32 UF` and `+44 município` of the urna, `+48 bool` urna biométrica (StaticLoad throws 7840 for a roll record with biometrics when it is false), `+52 set<ETipoAbrangencia>` of the cargos voted here, `+64`/`+76 vector<string>` roll file names read by `GetEleitoresEstaticos`, `+88 vector<string>` impediment files, `+100 tipo do identificador principal`, `+104` 32-bit `qtd votaram` (i32 stores), `+108 bool dinâmicos carregados`, `+112 map<identidade, identidade principal>` |
| `CLocal` | `+0 unique_ptr<md::CLocal>`. `md::CLocal`: UF `+16`, `optional<CSecaoEleitoral>` at `+84` (zona `+96`, seção `+104`, flag `+120`), `optional<CIdentificacaoUrnaContingencia>` at `+124` (flag `+136`) |
| `CPE` | `+0 md::CProcessoEleitoral` (176 B: `pleito1 +36`, `optional pleito2 +96`), `+176 turno`, `+180 {fase, processo, string, const CPleito*}` |

Enumerations whose numbers come from the code. The ASN.1 files use 1-based numbers; the in-memory `md` enums
are 0-based:

* `md::ETipoAbrangencia`: `0 MUNICIPAL`, `1 ESTADUAL`, `2 FEDERAL`. This matches ASN.1 `TipoAbrangencia`; `DesconverteAbrangencia` (5827) is the identity.
* `md::ETipoCargo` (`CCargo +4`): `0` majoritário, `1` proporcional, `2` consulta (`CCargo::Valida`). The BU QR field `TIPO:` prints this value.
* `md::EEstadoComparecimento` (`CEleitorDinamico +32`): `0` faltou, `1` sem cargo para votar, `2` habilitado but not voted, `3` votou. The names are inferred; `VOTOU == 3` is certain, it is tested in five places.
* `md::ETipoHabilitacao` (`+36`): `0` without biometrics, `1` biometrics recognised, `2` enabled with the poll worker's code (*mesário*).

## 2. Cargos: `CCargos` and `CConfiguracaoEleicao`

`CCargos::CreateInst` (inlined into start-up function 7787) builds `m_todos` and `m_cargos`: one entry for
each cargo of each election of the pleito (`comum_f3774`). It sorts `m_cargos` in **acquisition order**
(the order in which the voter is asked): election `ordemAquisicao`, then `CCargo::ordemAquisicao` (+15), with
comparator 11559.

* Voting (`vota::CEleitorVotando::IniciaCiclo`, u06): `FiltraPorAbrangencia(voter's abrangência)` (§3.2), then
  `First()` (2269). After that the screens call `GetCurrent` (332), `GetCurrentCargoID` (1938), … and `Next`
  (1708) after each confirmed cargo.
* Results (BU, RDV, QR code): `CGeraBU` (12110) and `CGravaResultado` (12098) first restore the full list
  (comum_f3784: `m_cargos = m_todos`, undoing the last voter's filter), then `OrdenaPorOrdemImpressao`
  (comum_f3782, comparator 11558, `CCargo +16`), then `First/IsEnd/Next`.

Every accessor checks the index and throws `EUeComumDadosError` 7816–7820 `"Cargo não posicionado."`. `Next`
throws 7815 `"Operação inválida."` if it is called at the end.

`CConfiguracaoEleicao::GetCargo(id)` (861) searches the cargos of every election of the pleito linearly (140 B
each) and throws 7827 `"Cargo não encontrado {}"`. `GetEleicao(id)` (inlined, line 293) throws 7826
`"Eleição não encontrada {}"`.

## 3. Voters: `CEleitores` / `CEleitorDetalhe`

### 3.1 Life cycle

```
votaInit -> 7787 (start-up)
   CEleitores::StaticLoad(estatico/)          [lines 395/407 = errors 7840/7841, inlined in 7787]
       GetEleitoresEstaticos: *-el.dat + *-tte.dat  (ASN.1 ModuloEleitores::EntidadeEleitores)
       GetEleitoresImpedidos: *-imp.dat            (ModuloImpedidos::EntidadeImpedidos)
       LoadEleitorDetalhe(decorator, impedidos, idx, nullptr) per voter (5764, observed)
   -> if EstadoVota >= '3': CInformacaoEleitor::CarregarDadosDinamicos (6734), see 3.4
votaInit / vota::CGeraDadosDinamicos::StartState  ("gera base dinâmica", EstadoVota 1 -> 3)
   6737 (CInformacaoEleitor::GerarDadosDinamicos, u02's name): unless fase '3' + treinamento-do-eleitor:
       CEleitores::DynamicCreate(GetPathTrab(interna, turno)/uenux.db)   i.e. /dsk/fi/dinamico/trabN/  (5761)
       CEleitores::DynamicCreate(GetPathTrab(externa, turno)/uenux.db)   i.e. /dsk/fe/dinamico/trabN/
   6734: if NOT treinamento-do-eleitor: CompleteLoad(estatico/, GetPathTrab(interna, turno)/uenux.db)
poll worker enables the voter -> CEleitores::MarcaEleitorFoiHabilitado (2825)
voter finishes                -> CEleitores::MarcaVotou (lines 456/460, inlined in 10210)
end of day                    -> GetQtdAptos (2824), QtdVotaramPorTipoHabilitacao (6034) feed BU / QR / RDV
```

### 3.2 Abrangência of a voter (func 3721, body of `GetCurrentAbrangenciaEleitor`, line 497)

Voters with a *transferência temporária de eleitor* (TTE) vote in a section outside their domicile. Example:
*voto em trânsito*, voting in transit. The TTE type is at `decorator +48`. The rule:

* no TTE, or domicile município = the urna's município → `MUNICIPAL`: every cargo;
* same UF as the urna → `ESTADUAL`: state and federal cargos;
* otherwise → `FEDERAL`: federal cargos only (for example Presidente).

`CCargos::FiltraPorAbrangencia` applies this rule to the list of cargos for each voter.

### 3.3 `DynamicCreate` (5761): creating the SQLite voter-state table

1. It throws 7837 `"Não há informações carregadas"` if the roll is empty, and 7838 `"Informações dinâmicas já carregadas"` if the first voter already has dynamic data.
2. It computes which abrangências have cargos in this urna (linear `find` over the set at `+52`).
3. For every voter it builds `CEleitorDinamico(título, título, estado)` (2800). `estado` = **0 (faltou)** if the voter has at least one cargo he may vote for (§3.2), **1 (sem cargo para votar)** otherwise. The biometric fields are zeroed, audio = 2, erro = 0.
4. It opens `dao::CEleitorDinamicoDAO(uenux.db)`, which creates the table `eleitor_dinamico` (`titulo BIGINT PRIMARY KEY…`). If the table is empty, it inserts every row in one transaction. If it is not empty, it sorts both lists and requires them to be **equal** (field-by-field `operator==`; the biometric fields are compared only when `estado ∈ {2,3}` and `tipo == 1`). Otherwise it throws 7839 `"Já existem informações dinâmicas no banco {}"`.

### 3.4 `CompleteLoad` (6734) and its wrapper

Func 6734 carries the srclocs of `CEleitores::CompleteLoad(const std::string&, const std::string&)` (lines
205/219/238), but the function is a **wrapper**. Its `this` is the 12-byte singleton returned by func 509,
which u02/u08 name `vota::CInformacaoEleitor`; u02 calls this method `CarregarDadosDinamicos()` (byte `+1`
= "dynamic data loaded"). The wrapper does five things:

1. loads **`rdv.dat`** (encrypted, `api::CEncryptedFile`) into `CRdvVota` (`Desconverte`, slot 12);
2. **only if `CEstadoGeralVota.treinamentoEleitor` is false** (`+72`, the sixth ASN.1 component, confirmed in the converter 11390), runs `CEleitores::CompleteLoad`:
   * 7834 `"Não existe o arquivo {}"` if `uenux.db` is missing;
   * reads the impediments (`+88` file list), the static roll and every `eleitor_dinamico` row, and sorts impediments and rows by identity (function-pointer comparators 11532/11531);
   * requires the same number of static and dynamic records, pairwise equal identities, else 7835 `"Havia divergência entre os títulos dinâmicos e os estáticos"`;
   * builds a new map `identity → CEleitorDetalhe` with `ValidaIdentidadesEleitor` (5765: 7852 `"Identificador principal ({}) ausente para o eleitor ({})"`) and `LoadEleitorDetalhe` (5764), and throws 7836 `"Identificador principal {} duplicado"` on a duplicate key;
   * fills `map<any identifier → principal identifier>` (título, CPF or free number all lead to the voter), counts `VOTOU` rows in `m_qtdVotaram` and sets `m_dinamicosCarregados`;
3. loads the **justificativas** (SQLite `registro_justificativa`) into `CJustificador`, through its `comum::servico::CJustificadorServico` member (+28);
4. loads the **mesários' attendance** (SQLite `comparecimento_mesario`) into `CDAORepositorio<CComparecimentoMesario>`, through its `comum::servico::CComparecimentoMesarioServico` member (+40). Both `SelectAll` calls go through the same ICF-merged body 5723 (`m_dao->SelectAll()`, DAO slot 8);
5. runs the referential-integrity check (`comum_f2543` → `CIntegridadeReferencial::Lanca` 2263: 7871 `"Integridade referencial: {}"`), then sets the loaded flag.

### 3.5 `LoadEleitorDetalhe` (5764): merge-join with impediments

Voters and impediments are both walked in identity order. `idx` points to the next impediment:

* equal identity → take P1 (`+20`) and P2 (`+24`, if present, else 0), `++idx`;
* voter identity **greater** → the impediment matches no voter → 7851 `"CEleitores::LoadEleitorDetalhe - ({}-{} > {}-{}). {} com marcação de impedido não encontrado nas listas de eleitores e de tte"`;
* smaller → no impediment.

Then it builds `CEleitorDetalhe`. With dynamic data, the constructor (celeitordetalhe.cpp:49) checks that the
row's título equals the voter's principal identity, else 7829 `"CEleitorDetalhe - identidades divergentes"`.
`CEleitorDetalhe::PodeVotar` (comum_f2266) uses P2 when the CDadoCarga *turno* is `'2'`, else P1.

### 3.6 Other `CEleitores` / `CEleitorDetalhe` functions

* `GetQtdAptos` (2824): `map{MUNICIPAL, ESTADUAL, FEDERAL} → SQtdeAptos{uint16 seção, uint16 TTE}`. Every voter without an impediment in the current round is counted under his own abrangência **and every wider one**, in the `TTE` counter if `decorator +48 != 0`. The BU QR code (`APTA/APTS/APTT`), the BU "eleitores aptos" line (11968) and `CGravaResultado` use it.
* `MarcaEleitorFoiHabilitado` (2825): 7842 `"Item inexistente"`, 7843 `"Dados dinâmicos não carregados"`, 7844 `"Eleitor já votou"`. It then runs the inlined `CEleitorDetalhe::MarcaHabilitado` (a `br_table` on the enabling type): for types 0/1/2 it sets estado ← 2 and tipo, and dedo/score/tentativas/erro/título do mesário according to the type (type 2 keeps or clears the título do mesário depending on whether the enabling data carry one). **Any other value hits the `br_table` default, which skips all of that**: estado, tipo and the biometric fields keep their previous values (estado stays `0` faltou), and the voter is not recorded as enabled. In every case the audio type, the enabling identity and the photo result are copied.
* `MarcaVotou` (lines 456/460, inlined in 10210 `CSincronismoOperador::vf2`): 7845 `"Item inexistente"`, 7846 `"Dados dinâmicos não carregados"`; if the row is not already `VOTOU` it sets estado ← 3 and increments the 32-bit counter at `+104`, otherwise it does nothing. 10210 skips it entirely in fase `'3'` + treinamento-do-eleitor.
* `QtdVotaramPorTipoHabilitacao(tipo)` (6034, through 2821 = tipo 1, 1935 = tipo 2): voters with dynamic data, that type and `estado == VOTOU`. These are the QR fields `HBBM` / `HBBG`; `HBSB` = tipo 0 comes from the twin comum_f2822.
* `GetDinamico` (1271) / `ConfereDadosDinamicos` (3770): 7832 `"{}: Dados dinâmicos não carregados"`.
* `GetBiometria` (1937): the encrypted biometrics are **read lazily**. The roll file (`<fase><pe><uf><mun><zona><seção>-el.dat`, or `-tte.dat` when the voter is in TTE other than type 4 *acessibilidade*) is opened `"rb"`, positioned at the stored offset, and `tamanho` bytes are read. The bytes are BER-decoded as `ModuloEleitores::BiometriaEleitorCifrada` and decrypted by `asn::CConversorBiometriaEleitorCifrada` (vtable slot 3) with the voter's identity. Errors: 7830 `"Eleitor corrente não tem biometria."`, 7831 `"Arquivo {} não existe"`, 7669/7670 `"CLeitorASN::LeBiometriaEleitor(<id>) - não foi possível …"`, 5953/5954 decode errors, 7658 `"Entidade está inválida: {}"`.
* Text sources for the poll worker's screen: `CEleitorDadoNomeParaUrna::Text(fmt)` (nome social if set, else nome, cut to 40), `CEleitorDadoSequencial`, `CEleitorDadoSecao`, `CEleitorDadoTTE::Text()` → `"Transferência Temporária"` or `""`. Each one throws 785x `"Não posicionado no eleitor corretamente"` without a current voter. They use `std::vformat` because the format string comes from the screen layout.

## 4. `CLocal`, `CHV`, `CPE`, `CIntegridadeReferencial`

* `CLocal` (lazy singleton, never throws on `GetInst`). `VerificaLido(func)` (782) throws 7874 `"{}: o arquivo de locais ainda não foi carregado"`. `GetZonaID` (1003, passes `"GetZona"`) takes the zona from the section or from the contingency identification. `GetSecaoID` (1078) returns **0 for a contingency urna**. Otherwise both throw 7872/7873 `"Tipo de local inválido"`. `VerificaEhSecao` (5742) throws 7875 `"{}: o local não era de seção"`.
* `CHV::GetHorarioVerao` (5745, name inferred: it passes `"GetHorarioVerao"`) throws 7870 `"{} - não entra em horário de verão"`, then the inlined `CHorarioVeraoMunicipio::GetHorarioVerao` throws 8005. Only the printer self-test report calls it.
* `CPE::CreateInst` (2787) builds its path from the section data, e.g. `estatico/t02400-cp.dat` (`FormataFase + FormataNumero(pe,5) + "-cp.dat"`). It reads `ModuloProcessoEleitoral::EntidadeProcessoEleitoral` and keeps the pleito of the current round (turno `'1'` → pleito1, else `GetPleito2()`). It throws 7876 `"Instância já criada"`. Callers create it lazily (they test comum_f2788 first). The new object receives a **copy** of the processo eleitoral that was read (the temporary is destroyed with 2830 afterwards). 5598 is `~CPE`, 2830 is `~CProcessoEleitoral`, and 11226 releases the singleton (probably at exit; no code references its table slot).

## 5. Data read and written

| data | format | where |
|---|---|---|
| `estatico/<fase><pe><uf><mun5><zona4><sec4>-el.dat`, `-tte.dat` | ASN.1 `ModuloEleitores` (voter roll, encrypted biometrics blob per voter) | StaticLoad / CompleteLoad loaders, `GetBiometria` (partial read at offset) |
| `estatico/…-imp.dat` | ASN.1 `ModuloImpedidos` (`impedimentoP1/P2`) | `GetEleitoresImpedidos` |
| `estatico/<fase><pe>-cp.dat` | ASN.1 `ModuloProcessoEleitoral` | `CPE::CreateInst` |
| `dinamico/trab{1,2}/uenux.db` table `eleitor_dinamico` | SQLite: `titulo, tipo_identificador, identidade_habilitacao, tipo_identidade_habilitacao, estado_comparecimento, tipo_habilitacao, dedo_habilitacao, score_habilitacao, numero_tentativa, erro_decifrar_biometria, tipo_ativacao_audio, estado_apresentacao_foto, resultado_decifracao_foto, titulo_mesario, tipo_identificador_mesario` | `DynamicCreate` (INSERT in a transaction), `CompleteLoad` (SELECT) |
| `uenux.db` tables `registro_justificativa`, `comparecimento_mesario` | SQLite | the 6734 wrapper |
| `dinamico/…/rdv.dat` | encrypted RDV | the 6734 wrapper |
| `vota.bin` `treinamentoEleitor` | ASN.1 `ModuloEstadoGeralVota` | decides whether the dynamic voter data exists at all |

## 6. Boletim de Urna: the QR codes (func 5604 and helpers)

The printed **BU** (*Boletim de Urna*, the poll-tape with the section's results) ends with one or more QR codes.
`vota::CMostraQRCodeBU` also shows them on screen. Their text payloads are produced by
**`comum::CGeradorBUQRCode::GeraQRCodes(size_t tamanhoMaximo)`**. The method name is inferred; the class and
file are certain from the srcloc `cgeradorbuqrcode.cpp:405 RetornaBlocoAssinado`. Reconstruction:
`src/uenux2/src/app/comum/relatorios/cgeradorbuqrcode.u04-fragment.cpp`.

**Caller.** `vota::CGeraBU::StartState` (12110, BU generation at *encerramento*, closing time) does four things:

1. It builds the header with `CCabecalhoQRCodeBuilder` setters (ZONA 5618, SECA 5620, IDUE 5621, IDCA 5623, HICA/HIQT 5622).
2. It constructs `CGeradorBUQRCodeVota(cabecalho, comparecimento, dataEmissao)` (5603). The base constructor copies the header (5608, 33 strings), keeps `CRdvVota` as the vote source, and stores `CEleitores::GetQtdAptos()`. `comparecimento` = max over the elections of `CVotosEleicoesVota::Comparecimento`.
3. It calls `GeraQRCodes(1100)`. The result is `{vector<string> conteudos; string assinatura}`.
4. For each payload it adds a caption `i/n` and `CQRCodeImagePaper::MontaImagem(payload)` to the printed BU.

**Step 1: header.** It is built from a copy of the builder's 33 fields. Each field is a `"TAG:value "` string.

| tag | value | source |
|---|---|---|
| `ORIG` | `VOTA`, or `RED` when the generator's flag `+418` is set (other apps: `SA` = *sistema de apuração*; `RED` = *recuperador de dados*) | virtual `PreencheCabecalho` (11242, u35) |
| `ORLC` | `LEG` or `COM` (configuration origin oficial/comunitária) | `CConfiguracaoEleicao +20 == 2` |
| `PROC` | id of the processo eleitoral | config `+0` |
| `DTPL` | pleito date `YYYYMMDD` | `CPleito` date via `CDate::Format` |
| `PLEI` | pleito id | config `+28` |
| `TURN` | `1`/`2` (char) | `CEstadoGeral` dado de carga |
| `FASE` | `{:c}` upper-case fase char (O/S/T) | `CDadoCarga::GetFaseChar` |
| `UNFE` | `{:2s}` UF, upper-cased with accent map (comum_f3509) | `CLocal::GetUF` |
| `MUNI`, `ZONA`, `SECA`, `AGRE` | município, zona, seção, aggregated sections (`AGRE` only if not empty) | 11242 / builder |
| `IDUE`, `IDCA`, `HICA` (`HIQT`) | urna id, carga code (`{:.24s}`), carga history | builder |
| `VERS` | `GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO")` → `10.23.0.1` in this build | 5604 |
| `LOCA`, `APTO`, `APTS`, `APTT`, `COMP`, `FALT` | local de votação, aptos (total, section, TTE), comparecimento, faltosos (= aptos − comparecimento) | 11242 |
| `HBBM`, `HBBG`, `HBSB` | voters who voted, enabled by biometrics / by the mesário's code / without biometrics (written only when comum_f820 is true, probably "the election uses biometrics"; not mandatory) | 11242 via 6034 (§3.6) |
| `DTAB`, `HRAB`, `DTFC`, `HRFC` | opening and closing date/time (`CEstadoGeralVota` dhIni/dhFimAquisicao; `"Início/Fim da aquisição não marcado"` otherwise) | 11242 |
| `DTEM`, `HREM` | emission date/time: **only in RED/SA payloads** (and `JUNT`/`TURM` for SA) | 11242 |

Validation (`CCabecalhoQRCodeBuilder::preBuild` lambda 2791) checks every mandatory field. An empty one throws
`EUeComumRelatoriosError` 9050 `"Campo ({}) não informado."`. For VOTA the mandatory fields are Origem,
OrigemProcessoEleitoral, ProcessoEleitoral, DataPleito, Pleito, Turno, Fase, Uf, Municipio, Zona, Secao, IdUrna,
CodigoCarga, HistoricoCarga, VersaoSoftware, Local, QtdeAptos, QtdeAptosDaSecao, QtdeAptosTTE, QtdeCompareceram
and QtdeFaltosos. Concatenation order: `ORIG ORLC PROC DTPL PLEI TURN FASE UNFE MUNI ZONA SECA [AGRE] IDUE IDCA
HICA VERS LOCA APTO APTS APTT COMP FALT HBBM HBBG HBSB DTAB HRAB DTFC HRFC` (RED adds `DTEM HREM`; SA uses
`JUNT TURM DTEM HREM` instead of `LOCA…HRFC`).

**Step 2: body.** The cargos are walked in print order (`OrdenaPorOrdemImpressao`, `First/IsEnd/Next`):

* when the election changes: `IDEL:<id> ` + virtual slot 3 (empty for VOTA);
* per cargo: `CARG:<código> TIPO:<0 maj|1 prop|2 consulta> `, then `VERC:<versão do pacote>` (candidate cargos: `CCandidaturas::RecuperaVersaoPacote`; consultas: `CCargos::GetCurrentEleicaoVersaoPacote` → `CPleito::GetVersaoPacoteEleicao`, 8163 `"Versão de pacote não encontrada"`);
* candidate cargo **with no apt candidate**: `APTA:<n> [APTS APTT] CSEC:<total de votos do cargo> `;
* **proportional**: for each party with votes: `PART:<nº> ` + `<candidato>:<votos> ` for each apt candidate with votes + `LEGP:<legenda> TOTP:<total do partido> `. Then `APTA… NOMI:<nominais> LEGC:<legendas> BRAN:<brancos> NULO:<nulos> TOTC:<total>`;
* **majoritarian**: `<candidato>:<votos> ` for every candidacy of the cargo with votes, then `APTA… NOMI BRAN NULO TOTC` (5606);
* **consulta**: the answers sorted by number (5605 is the `std::sort` instantiation), `<resposta>:<votos> ` for those with votes, then the same totals.

`APTA:` (3696, u25) is the aptos count of the cargo's abrangência (`APTS`/`APTT` are added when there are TTE
voters). The vote counts come from `CRdvVota` slots 3–10 (`Candidato`, `Legenda`, `Partido`, `Nominais`,
`Legendas`, `Nulos`, `Brancos`, `Cargo`).

**Step 3: split.** The text `header + body` is cut into parts of at most `tamanhoMaximo − 277` = **823**
characters. A part that fills the limit and does not end in a space is cut at its last space. Every part is
trimmed.

**Step 4: hash chain and signature (`RetornaBlocoAssinado`, line 405).**

* `hash₀ = SHA-512(parte₀)`; `hashᵢ = SHA-512(join(bloco₀…bloco_{i−1}, " ") + " " + parteᵢ)`, where `blocoᵢ = "parteᵢ HASH:hashᵢ"`. The hashes are 128 upper-case hex characters (`ecourna::api::security::CSha("SHA2-512")`), fed in 512-byte slices.
* payload `i` = `QRBU:<i+1>:<n> VRQR:6.0 parteᵢ HASH:hashᵢ`. The version constant is the 3-character string `"6.0"`, decoded from the packed `string_view` argument.
* the **last** payload also gets ` ASSI:<hex>`: the raw bytes of the last hash are signed through the `api::pkcs11::IPkcs11` poly-singleton (vtable slots 23 → 6 → 24). The hex signature is also returned separately.

Illustrative VOTA payload (the structure follows the code; the values are made up):

```
QRBU:1:1 VRQR:6.0 ORIG:VOTA ORLC:LEG PROC:2400 DTPL:20261004 PLEI:2410 TURN:1 FASE:T UNFE:AC MUNI:1 ZONA:1
SECA:1 IDUE:... IDCA:... HICA:... VERS:10.23.0.1 LOCA:... APTO:1 APTS:1 APTT:0 COMP:0 FALT:1 HBBM:0 HBBG:0
HBSB:0 DTAB:... HRAB:... DTFC:... HRFC:... IDEL:... CARG:13 TIPO:1 VERC:... PART:91 91001:1 LEGP:0 TOTP:1
APTA:1 NOMI:1 LEGC:0 BRAN:0 NULO:0 TOTC:1 CARG:11 TIPO:0 VERC:... APTA:1 NOMI:0 BRAN:0 NULO:1 TOTC:1
HASH:<128 hex> ASSI:<hex>
```

**In the web build** nothing registers an `api::pkcs11::IPkcs11`: the class has no RTTI, and its mangled name
`N3api6pkcs117IPkcs11E` (@517073) is referenced only by the lookup `CPolySingleton<IPkcs11>::instance` (3704),
which also serves the BU file signature (10273) and `CEstadoGeral::RecuperarCertificado` (5635). Its first check
(cpolysingleton.h:78) throws `EPatternErr` 1301 `"PolySingleton - solicitada uma instancia nao criada
N3api6pkcs117IPkcs11E…"` (the later `"{}: solicitada uma instância não criada de {}"` path is not reached). The
QR payloads therefore cannot be finished by the unmodified module. The web page never reaches BU generation
anyway (docs/10-boletim-de-urna.md §4.4/§5.1); the BU harness substitutes a zero-byte signer.

## 7. Web-build specifics

* **"Treinamento do eleitor" mode.** The simulator's `vota.bin` has `treinamentoEleitor = TRUE`; `comum_f697` = `fase == '3' && treinamentoEleitor`. Two consequences:
  * `DynamicCreate` never runs. This is confirmed by `analysis/runtime/memfs-after-init`: `uenux.db` has no `eleitor_dinamico` table.
  * The `CompleteLoad` part of 6734 is skipped. Only the static roll is loaded (`StaticLoad` → `LoadEleitorDetalhe` ran, sampled once). No voter has dynamic data, so `MarcaEleitorFoiHabilitado`, `GetDinamico` and `QtdVotaramPorTipoHabilitacao` would throw or count 0.
* Every scenario's roll (`…0000100010001-el.dat`) has **one** voter record (a different one for 1st- and 2nd-round scenarios); the `-tte.dat` rolls are empty. In municipal-t1, `-imp.dat` marks that voter with P2 = `suspenso`.
* No PKCS#11 signer is registered (§6).
* `DynamicCreate` would write the table into the `uenux.db` of **both flashes** of the current round
  (`/dsk/fi/dinamico/trabN/` and `/dsk/fe/dinamico/trabN/`, via `CPath::GetPathTrab(interna|externa, turno)`).
  Both copies in `memfs-after-init` only have `comparecimento_mesario` and `registro_justificativa`.
* The singletons' mutexes are no-ops: the lock calls vanished, and only `std::mutex::unlock` residue (func 150) remains in every `GetInst`.

## 8. Wasm / Emscripten notes

* **Inlining misleads the naming.** The pipeline named 5604 after a 6-line method it inlines. 6734 carries CompleteLoad's srclocs but is a wrapper. 5745 carries `VerificaEntraEmHorarioVerao` but returns `this + 4` (`GetHorarioVerao`). 5764 carries both `CEleitorDetalhe::CEleitorDetalhe` and `CEleitores::LoadEleitorDetalhe`. 11556 carries `CCargos::GetInst` but is a lambda of the zerésima summary.
* **Dead-argument elimination.** `LoadEleitorDetalhe` lost `this`. `CPE::CreateInst` lost its `EFlashOrigem`. `CEleitores::GetInst` inlined into the `Text` functions reads the global directly.
* **merge-similar-functions.** 5616 (`"RED"`) and 5617 (`"SA"`) are 13-byte thunks passing a packed `string_view` constant (`len<<32 | ptr`) to the shared body 6028.
* **Format arguments.** libc++ `std::format` stores the argument kinds as 5-bit codes: 6 = unsigned, 13 = string_view, 15 = handle. Decoding them gave the argument types of the messages, e.g. `QRBU:{}:{} VRQR:{} {}` = (unsigned, unsigned, string_view, string_view).
* Wasm-decompile's `a[N]:T@k` means **byte offset N·k** (e.g. `c[10]:int@2` = offset 20). This was needed for `SQtdeAptos` (two `uint16` at +20/+22 of the map node).
* The `Text` functions call `std::vformat` because the format strings are runtime data (screen layout).

## 9. Suspicious or noteworthy code

See the `suspicious` list in the unit result. Summary:

1. The BU QR signing needs `IPkcs11`, which does not exist in the web build (§6). Reaching BU QR generation in the simulator would throw.
2. The QR split loop (5604) has no guard for a full 823-character window without a usable space. The last-space scan returns `npos` when there is none: the whole remainder becomes one part and `pos += npos` moves the cursor back one character (at `pos == 0`, `substr` throws `out_of_range` instead). It returns `0` when the only space is the window's first character, which is exactly what follows every cut: an empty part is pushed and `pos` does not move. Either way the loop never ends (the `npos` case walks back until it reaches the `0` case), and the parts vector grows until memory runs out. Triggering it needs a run of about 822 characters without a space after a cut; normal data cannot produce that, because the field values are short and truncated.
3. The impediment merge-join (5764) never checks the impediments left after the last voter: an impediment whose identity sorts after every voter is silently ignored. Neither caller (StaticLoad in 7787, CompleteLoad in 6734) compares the final index with the list size.
4. `CEleitorDetalhe::MarcaHabilitado` silently ignores unknown enabling types (≥ 3): the `br_table` default skips the state update, so estado and tipo keep their previous values (normally `0` faltou, i.e. the voter is not recorded as enabled) while the audio/identity/photo fields are still overwritten. It does not throw.
5. `CParteCargos::Imprime` (11209) starts its "previous election" from the **last** cargo's election. With a single election the election header is never printed; with two or more it is. This looks intentional.
6. `CCargos::FiltraPorAbrangencia` with an invalid abrangência throws only when the cargo list is not empty.
7. Privacy: the training roll shipped with the simulator contains a full name, título number and birth date (probably fictitious test data). This code loads it into memory at start-up.

## 10. Complete mapping table (83 functions)

"u04 file" = reconstructed in this unit's files. "fragment" = reconstructed in a fragment file for another unit's
file. "library" = template instantiation summarised in a comment.

| idx | reconstructed symbol | original file | where / note |
|---:|---|---|---|
| 187 | `comum::CConfiguracaoEleicao::GetInst` | comum/dados/cconfiguracaoeleicao.cpp:36 | u04 file |
| 273 | `comum::CCargos::GetInst` | comum/dados/ccargos.cpp:24 | u04 file |
| 282 | `std::swap<comum::md::CEleitorDinamico>` | (celeitores.cpp instantiation) | library (move-swap) |
| 326 | `comum::CEleitores::GetInst` | comum/dados/celeitores.cpp:44 | u04 file |
| 332 | `comum::CCargos::GetCurrent` | comum/dados/ccargos.cpp:59 | u04 file |
| 470 | `comum::md::CEleitorDinamico::operator=(CEleitorDinamico&&)` | comum/dados/md/eleitor/celeitordinamico.h | library/implicit (move assignment) |
| 782 | `comum::CLocal::VerificaLido` | comum/dados/clocal.cpp:345 | u04 file |
| 861 | `comum::CConfiguracaoEleicao::GetCargo` | comum/dados/cconfiguracaoeleicao.cpp:316 | u04 file |
| 1003 | `comum::CLocal::GetZonaID` | comum/dados/clocal.cpp:86 | u04 file |
| 1073 | `comum::CPE::GetInst` | comum/dados/cpe.cpp:21 | u04 file |
| 1078 | `comum::CLocal::GetSecaoID` | comum/dados/clocal.cpp:110 | u04 file |
| 1271 | `comum::CEleitorDetalhe::GetDinamico` | comum/dados/celeitordetalhe.cpp | u04 file |
| 1392 | `std::__sort3<…, comum::md::CEleitorDinamico*>` | (celeitores.cpp) | library (std::sort) |
| 1708 | `comum::CCargos::Next` | comum/dados/ccargos.cpp:46 | u04 file |
| 1937 | `comum::CEleitorDetalhe::GetBiometria` | comum/dados/celeitordetalhe.cpp:94/101 | u04 file |
| 1938 | `comum::CCargos::GetCurrentCargoID` | comum/dados/ccargos.cpp:68 | u04 file |
| 2263 | `comum::CIntegridadeReferencial::Lanca` | comum/dados/cintegridadereferencial.cpp:344 | u04 file |
| 2269 | `comum::CCargos::First` | comum/dados/ccargos.cpp | u04 file (name inferred) |
| 2272 | `comum::CCandidaturas::GetNumerosCandidatosAptos` | comum/dados/ccandidaturas.cpp (u03) | fragment (name inferred) |
| 2728 | `std::__tree<map<CComparecimentoMesarioPK,CComparecimentoMesario>>::destroy` | (celeitores.cpp) | library |
| 2787 | `comum::CPE::CreateInst` | comum/dados/cpe.cpp:26 | u04 file |
| 2800 | `comum::md::CEleitorDinamico::CEleitorDinamico(id, id, estado)` | comum/dados/md/eleitor/celeitordinamico.cpp (u05) | fragment |
| 2809 | `std::__tree<map<CNumeroInscricaoEleitoral,CJustificadorDetalhe>>::destroy` | (celeitores.cpp) | library |
| 2817 | `comum::CHV::GetInst` | comum/dados/chv.cpp:27 | u04 file |
| 2820 | `std::__sort4<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | library |
| 2824 | `comum::CEleitores::GetQtdAptos` (IDataSourceAptos slot 0) | comum/dados/celeitores.cpp | u04 file (name inferred) |
| 2825 | `comum::CEleitores::MarcaEleitorFoiHabilitado` | comum/dados/celeitores.cpp:435/439/447 | u04 file (+ inlined `CEleitorDetalhe::MarcaHabilitado`) |
| 2830 | `comum::md::CProcessoEleitoral::~CProcessoEleitoral` | comum/dados/md/processoeleitoral/cprocessoeleitoral.h | library/implicit (noted in cpe.cpp) |
| 2835 | `comum::CCargos::GetCurrentEleicaoID` | comum/dados/ccargos.cpp:85 | u04 file |
| 2836 | `comum::CCargos::GetCurrentEleicao` (+ inlined `CConfiguracaoEleicao::GetEleicao`) | comum/dados/ccargos.cpp:76 (+ cconfiguracaoeleicao.cpp:293) | u04 file |
| 3721 | `comum::(anon)::GetAbrangenciaEleitor` | comum/dados/celeitores.cpp | u04 file (name inferred; body of GetCurrentAbrangenciaEleitor:497) |
| 3757 | `std::__sort4<…, CEleitorDinamico*>` | (celeitores.cpp) | library |
| 3758 | `std::__split_buffer<CEleitorDinamico>::~__split_buffer` | (celeitores.cpp) | library |
| 3759 | `std::__uninitialized_allocator_relocate<CEleitorDinamico>` | (celeitores.cpp) | library |
| 3762 | `std::__introsort<…, CEleitorDinamico*>` | (celeitores.cpp) | library (sort in DynamicCreate) |
| 3763 | `std::vector<CEleitorDinamico>::__destroy_vector::operator()` | (celeitores.cpp) | library |
| 3770 | `comum::CEleitorDetalhe::ConfereDadosDinamicos` | comum/dados/celeitordetalhe.cpp:224 | u04 file |
| 5404 | `vota::CControlaReconhecimento::LimiteVerificacoesAtingido` | vota/operador/confirmaidentidade/ccontrolareconhecimento.cpp (u10) | fragment (name inferred) |
| 5598 | `comum::CPE::~CPE` | comum/dados/cpe.cpp | u04 file |
| 5603 | `comum::CGeradorBUQRCodeVota::CGeradorBUQRCodeVota` (+ base ctor) | comum/relatorios/cgeradorbuqrcodevota.cpp (path inferred) | QR fragment |
| 5604 | `comum::CGeradorBUQRCode::GeraQRCodes` | comum/relatorios/cgeradorbuqrcode.cpp:405 (u25) | QR fragment (name inferred; inlines CCargos::GetCurrentEleicaoVersaoPacote:93, CPleito::GetVersaoPacoteEleicao:158, RetornaBlocoAssinado:405) |
| 5605 | `std::__introsort<…, md::CRespostaConsulta*>` (sort by número) | (cgeradorbuqrcode.cpp) | library |
| 5606 | `comum::CGeradorBUQRCode::TotaisVotosCargo` | comum/relatorios/cgeradorbuqrcode.cpp | QR fragment (name inferred) |
| 5608 | `comum::CCabecalhoQRCode::CCabecalhoQRCode(const CCabecalhoQRCode&)` | comum/relatorios/ccabecalhoqrcodebuilder.h (path inferred) | library/implicit (33 strings) |
| 5616 | `comum::(anon)::EhOrigemRED` | comum/relatorios/cgeradorbuqrcode.cpp | QR fragment (name inferred) |
| 5617 | `comum::(anon)::EhOrigemSA` | comum/relatorios/cgeradorbuqrcode.cpp | QR fragment (name inferred) |
| 5723 | `m_dao->SelectAll()` (DAO slot 8): ICF-shared body of `comum::servico::CJustificadorServico::SelectAll` / `CComparecimentoMesarioServico::SelectAll` | comum/servico (path inferred) | noted in celeitores.cpp (called with CJustificador+28 and the mesários repository+40, both `*Servico` objects; not a CDAORepositorio method) |
| 5742 | `comum::CLocal::VerificaEhSecao` | comum/dados/clocal.cpp:354 | u04 file |
| 5745 | `comum::CHV::GetHorarioVerao` (+ inlined VerificaEntraEmHorarioVerao:117) | comum/dados/chv.cpp | u04 file (name inferred) |
| 5748 | `std::__insertion_sort_incomplete<…, CEleitorDinamico*>` | (celeitores.cpp) | library |
| 5749 | `std::__sort5<…, CEleitorDinamico*>` | (celeitores.cpp) | library |
| 5751 | `std::__insertion_sort_incomplete<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | library |
| 5761 | `comum::CEleitores::DynamicCreate` | comum/dados/celeitores.cpp:301/305/358 | u04 file |
| 5764 | `comum::CEleitores::LoadEleitorDetalhe` (+ `CEleitorDetalhe` ctor) | comum/dados/celeitores.cpp:719 (+ celeitordetalhe.cpp:49) | u04 file |
| 5765 | `comum::CEleitores::ValidaIdentidadesEleitor` | comum/dados/celeitores.cpp:738 | u04 file |
| 5766 | `std::__introsort<…, fn-ptr cmp, CEleitorDinamico*>` | (celeitores.cpp) | library (sort in CompleteLoad) |
| 6028 | `comum::(anon)::OrigemIgual` (shared body of 5616/5617) | comum/relatorios/cgeradorbuqrcode.cpp | QR fragment |
| 6034 | `comum::CEleitores::QtdVotaramPorTipoHabilitacao` | comum/dados/celeitores.cpp | u04 file (name inferred) |
| 6734 | `vota::CInformacaoEleitor::CarregarDadosDinamicos` (u02's name) with `comum::CEleitores::CompleteLoad` inlined | comum/dados/celeitores.cpp:205/219/238 (+ vota/eleitor/comum/cinformacaoeleitor.cpp) | u04 file (CompleteLoad) + fragment (wrapper) |
| 10447 | `vota::CDadoEleitorNaoConfere::ProcessInput` | vota/operador/confirmaidentidade/cdadoeleitornaoconfere.cpp (path inferred) | fragment |
| 10448 | `vota::CDadoEleitorNaoConfere::StartState` | same | fragment |
| 11203 | `comum::CParteRdv::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragment |
| 11209 | `comum::CParteCargos::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragment |
| 11213 | `comum::CParteCandidatosProporcionais::Imprime` | comum/relatorios/cpartecandidatos.cpp (u25) | fragment |
| 11226 | release of `CPE::s_inst` (`delete exchange(s_inst, nullptr)`, unused `void*` arg; at-exit role inferred, no code references its table slot 2984) | comum/dados/cpe.cpp | u04 file (name inferred) |
| 11259 | `comum::CDataSourcesRelatorio<CRdvVota,CEleitores>::HeaderDetalheSeHouverVotos` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragment (name inferred) |
| 11526 | `comum::CEleitorDadoTTE::Text` | comum/dados/celeitores.cpp:796 | u04 file |
| 11527 | `comum::CEleitorDadoSecao::Text` | comum/dados/celeitores.cpp:784 | u04 file |
| 11528 | `comum::CEleitorDadoSequencial::Text` | comum/dados/celeitores.cpp:772 | u04 file |
| 11529 | `comum::CEleitorDadoNomeParaUrna::Text` | comum/dados/celeitores.cpp:756 | u04 file |
| 11556 | lambda "nome da eleição corrente" (function pointer, slot 3049) | vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp (u09) | fragment (inlines CCargos::GetInst:24) |
| 11687 | `vota::CMajoritarioValido::GetMensagemAudio` | vota/eleitor/votamajoritario/cconfirmamajoritario.cpp (path inferred) | fragment (name inferred) |
| 11690 | `vota::CMajoritarioRepetido::GetMensagemAudio` | same | fragment (name inferred) |
| 11693 | `vota::CMajoritarioNulo::GetMensagemAudio` | same | fragment (name inferred) |
| 11696 | `vota::CMajoritarioBranco::GetMensagemAudio` | same | fragment (name inferred) |
| 11724 | `vota::CPedeNominal::GetProximoEstado` | vota/eleitor/votaproporcional/cpedenominal.cpp (path inferred) | fragment |
| 11968 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::GetLinhasEleitoresAptos()::lambda::operator()` | comum/relatorios/cdatasourcesrelatorio.h:212 (u25) | fragment |
| 11972 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::DetalheCandidato` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragment (name inferred) |
| 11975 | `CDataSourcesRelatorio<CRdvVota,CEleitores>::TrailerComparecimento` | comum/relatorios/cdatasourcesrelatorio.h (u25) | fragment (name inferred) |
| 11976 | `vota::PartidoTemCandidatos` (zerésima predicate, slot 1631) | vota/eleitor/iniciovotacao/cgerazeresimabase.cpp (path inferred) | fragment (name inferred) |
| 12742 | `api::CDataText<comum::CCandidaturasDSNumero>::GetText` (+ `CCandidaturasDSNumero::operator()`) | comum/dados/ccandidaturas.cpp (u03) / api CDataText | fragment |
| 12811 | `comum::NomeCargoComEscolha` (text source, slot 1088) | comum/dados/ccargods.cpp (u03, path inferred) | fragment (name inferred) |
| 13550 | `vota::CPreShowProgressBar::PreShow` | vota/eleitor/comum/cpreshowprogressbar.cpp (path inferred) | fragment (described) |

Error-code map (`EUeComumDadosError`) of this directory: 7815–7821 CCargos, 7826/7827 CConfiguracaoEleicao,
7829–7832 CEleitorDetalhe, 7834–7839 CompleteLoad/DynamicCreate, 7840/7841 StaticLoad, 7842–7844
MarcaEleitorFoiHabilitado, 7845/7846 MarcaVotou, 7847 GetCurrentAbrangenciaEleitor, 7851/7852 LoadEleitorDetalhe/ValidaIdentidadesEleitor, 7854–7857 Text
sources, 7866… CHV::CreateInst, 7870 CHV, 7871 CIntegridadeReferencial, 7872–7875 CLocal, 7876 CPE.

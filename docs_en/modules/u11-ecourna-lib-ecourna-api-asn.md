# u11 — `ecourna::api::asn`: the validating ASN.1 converter template (`IConversorASN`) and the ecourna converters that inline it

Unit u11 has **90 functions**; **16** ran during the recorded votes (`analysis/runtime/*.functions.tsv`).
Its only attested original file is the header **`ecourna-lib/ecourna/api/asn/iconversorasn.hpp`** of the TSE
library *ecourna* (Conan package `libecea1da310e5107`). The header defines one class template,
`ecourna::api::asn::IConversorASN<ENTIDADE, DADO>`: the base of every ecourna *converter* (conversor) between an
ASN.1 object decoded from / encoded to a data file (the *entidade*) and a plain C++ data object (the *dado*). Its two
non-virtual methods check the ASN.1 object with the III ASN.1 validators and throw a `CBaseError<EApiAsnError>`
(codes **1900** / **1902**) when it is invalid. A run with a deliberately corrupted `-cfm.dat` (§6.1) shows that the
file layer (`api::CFileASN`) performs the same check first, so for data files this template is a second line of defence;
for the BU it is the check that guards the `detalhamentoComparecimento` counts (§7.1).

Because the template is inlined everywhere, the tool's `std::source_location` names spread onto the callers: the unit
also received 13 converter methods, 5 `std::transform` instances, 5 constructors, 3 implicit destructors and 12
container helpers of the ecourna data classes. The table below sums it up; §12 maps every index.

| group | # | functions |
|---|---|---|
| out-of-line `Converte`/`Deconverte` instantiations (18–21-byte thunks + 2 CHOICE copies) | 43 | 27 `Converte` + 16 `Deconverte` |
| merged template bodies (wasm-opt *merge-similar-functions*) | 6 | 682, 1167, 6031, 6032, 6149, 6150 |
| helpers of the template | 2 | 858 `std::stringstream()`, 1074 `CBaseError<EApiAsnError>` ctor |
| converter `DoConverte`/`DoDeconverte` bodies (real names recovered from vtables) | 13 | 9057 9061 9064 9095 9102 9120 9127 9137 9142 9171 9200 9207 11440 |
| `std::transform` instances whose lambda is an inlined `Deconverte` | 5 | 9092 9094 9119 9176 9206 |
| data-class constructors | 5 | 9029 9034 9037 9040 9045 |
| implicit destructors of data classes | 3 | 1006 1876 5097 |
| inline helper OCTET STRING → `std::vector<uebyte>` | 1 | 5095 |
| libc++ / III ASN.1 container instantiations | 12 | 306 2660 2661 3488 5096 5099 5100 6145 6146 9099 9118 9174 |

Reconstructed sources (§13): `src/ecourna/api/asn/iconversorasn.hpp` (+ `iconversorasn.instances.cpp`, a map of the
instantiations), ten converter files under `src/ecourna/app/dados/asn/…` (paths inferred), three small data-class
files, two `.u11.cpp` fragments for files owned by u14, `src/ecourna/app/dados/u11-foreign-fragments.cpp`, and
`src/uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp`.

---

## 1. Glossary

| term | meaning |
|---|---|
| *entidade* (`TEntidade`, `ENTIDADE`) | a class generated from the TSE ASN.1 modules (`ModuloTiposEleitorais::CabecalhoEntidade`, `ModuloResultadoUrnaCadastro::DadosComparecimento`, …), built on the III ASN.1 runtime (`docs/libraries/asn1-runtime.md`). Encoded as BER in the files |
| *dado* (`TDado`, `DADO`) | the ecourna data class the application works with (`ecourna::app::dados::CDadosComparecimento`, `CFoto`, …) or a range-checked scalar (`CBaseType<T, MIN, MAX, ID>`) |
| **Converte** / **Deconverte** | dado → entidade (to write a file) / entidade → dado (after reading a file). ecourna spells it *Deconverte*; the uenux2 twin `comum::asn::IConversorASN` spells it *Desconverte* |
| *DoConverte* / *DoDeconverte* | the pure-virtual worker methods (vtable slots 2 and 3) that each concrete converter implements |
| *cabeçalho* (`CabecalhoEntidade`) | header of every data file: generation date-time (`DataHoraJE`, text `YYYYMMDDTHHMMSS`) + election id (`IDEleitoral` CHOICE) |
| *comparecimento* | attendance: who came to vote in the section (*votou*, *faltou*, *não votou*…) and how each voter was enabled |
| *habilitação* | enabling the voter at the urna: by fingerprint (*biometria*), by typing a code (*por código*, done by the *mesário*) or by audio |
| *mesário* | poll worker; `ComparecimentoMesario` records the poll workers identified at opening (*abertura*) and closing (*encerramento*) |
| *justificativa* | a voter away from home who justifies not voting, at this urna, instead of voting (título + birth year are recorded) |
| *parametrização da urna* (`-pu.dat`) | behaviour switches, report titles and labels of the voting application |
| *configuração de municípios* (`-cfm.dat`) | per-municipality schedule: zerésima printing, start and end of voting, end of the working day |
| *federação* (`-fe.dat`) | party federation (federação partidária): a group of parties that runs as one |
| *mídia* / *InformacaoMidia* | a flash card or pen drive prepared for the urna and its description file `infomidia-*.dat` |
| *JUFA* | the name of the public key file `jufa.pk1` used to encrypt the attendance data (see §7.2) |

---

## 2. Classes and how they relate

### 2.1 RTTI

```
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>        35 instantiations, typeinfo only (__class_type_info: abstract,
 │                                                       no vtable of its own), iconversorasn.hpp
 └─ ecourna::app::dados::asn::CConversorXxx             35 converters, one per instantiation, all "si" (single
                                                         inheritance), sizeof 4 (vptr only), built on the caller's stack
      vtable: [0] ~T() = ICF 174 "return this"   [1] ~T() deleting = 144 free
              [2] DoConverte(const TDado&) -> TEntidade     [3] DoDeconverte(const TEntidade&) -> TDado

users outside ecourna (they build ecourna converters on their stack): comum::asn::CConversorCarga,
CConversorDadoCorrespondencia, CConversorBiometriaEleitor, CConversorFotoCandidato, CConversorVotosCargo,
CConversorEleicoesVota, CConversorEntidadeBU (BU), comum::CGravadorRCSecao (attendance file), api::CFileASN

ecourna::api::exception::CBaseError<ecourna::api::asn::EApiAsnError, SErrorLimits{1900, 1910}> : CError
     typeinfo 1563144, vtable 1563164; constructor thunk = func 1074 (shared body 710)
```

Every converter vtable has four slots; the `DoConverte`/`DoDeconverte` pair is **pure virtual** in the ecourna
template (all 35 subclasses implement both). This differs from the uenux2 twin `comum::asn::IConversorASN`
(`uenux2/src/app/comum/asn/iconversorasn.h`, unit u21), whose base slots throw *"Método DoConverte() não implementado
para {}"*.

Instantiations present in the binary (ENTIDADE → DADO):

| ASN.1 module | entities converted through `IConversorASN` |
|---|---|
| `ModuloTiposEleitorais` | CabecalhoEntidade, Fase (→ `CFaseID`), Turno (→ `CBaseType<ushort,1,2,11>`), Foto, MunicipioZona, IdentificacaoSecaoEleitoral, IdentificadorEleitor (CHOICE), CodigoCargoConsulta (→ `CBaseType<ushort,1,99,3>`) |
| `ModuloTiposEcoUrna` | IdentificadorGeradorMidia, RegistroIdentificacaoEleitor |
| `ModuloResultadoUrnaCadastro` | EntidadeResultadoUrnaCadastro, DadosComparecimento, DadosComparecimentoCifrado, DadosCifracao, ComparecimentoSecao, ComparecimentoMesario, IdentificacaoJustificativa, EstadoComparecimento, EstadoHabilitacaoPorCodigo, HabilitacaoBiometrica, ApresentacaoFotoEleitor |
| `ModuloConfiguracaoMunicipios` | EntidadeConfiguracaoMunicipios, ConfiguracaoMunicipio, HorariosUrna |
| `ModuloParametrizacaoUrna` | EntidadeParametrizacaoUrna, ParametrosUrna, TituloRelatorio, LabelParametrizado |
| `ModuloInformacaoMidia` | InformacaoMidia, DadosGeracaoMidia, Aplicativo, Autenticacao |
| `ModuloFederacoes` | EntidadeFederacoes, Federacao |
| `ModuloBoletimUrna` | **DetalhamentoComparecimento** (the BU's attendance breakdown, §7.1) |

### 2.2 Converter classes whose methods are in this unit

| class (namespace `ecourna::app::dados::asn` unless noted) | vtable | slot 2 `DoConverte` | slot 3 `DoDeconverte` | file (all paths inferred) |
|---|---|---|---|---|
| `CConversorDadosComparecimento` | 1132868 | **9102** | **9095** | `asn/resultadournacadastro/cconversordadoscomparecimento.cpp` |
| `CConversorDadosComparecimentoCifrado` | 1137068 | 9060 (u40) | **9057** | `…/cconversordadoscomparecimentocifrado.cpp` |
| `CConversorDadosCifracao` | 1136812 | 9062 (u40) | **9061** | `…/cconversordadoscifracao.cpp` |
| `CConversorIdentificacaoJustificativa` | 1136564 | 9065 (u40) | **9064** | `…/cconversoridentificacaojustificativa.cpp` |
| `CConversorComparecimentoSecao` | 1131064 | 9126 (u40) | **9120** | `…/cconversorcomparecimentosecao.cpp` |
| `CConversorIdentificacaoSecaoEleitoral` | 1130788 | 9129 (u40) | **9127** | `asn/cconversoridentificacaosecaoeleitoral.cpp` |
| `CConversorConfiguracaoMunicipios` | 1129768 | 9141 (u40) | **9137** | `asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp` |
| `CConversorConfiguracaoMunicipio` | 1129456 | 9144 (u40) | **9142** | `asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp` |
| `CConversorParametrizacaoUrna` | 1127436 | 9173 (u40) | **9171** | `asn/parametrizacaourna/cconversorparametrizacaourna.cpp` |
| `CConversorAutenticacao` | 1125420 | 9201 (u40) | **9200** | `asn/midias/cconversorautenticacao.cpp` |
| `CConversorFederacoes` | 1124660 | 9212 (u40) | **9207** | `asn/federacoes/cconversorfederacoes.cpp` |
| `comum::asn::CConversorFotoCandidato` (uenux2) | 1562588 | 11439 (base, throws) | **11440** `DoDesconverte` | `uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp` |

Directory choice: the srcloc-attested converters of u14 live in `ecourna/app/dados/asn/` (ModuloTiposEleitorais types)
and in sub-directories that mirror the data classes (`asn/midias/`, `asn/parametrizacaourna/`,
`asn/resultadournacadastro/`). The new files follow that rule.

---

## 3. The template: contract and control flow

Reconstruction: `src/ecourna/api/asn/iconversorasn.hpp`. Line numbers 49 and 66 and column 19 of the two throws match
the source_location records exactly (12 spaces of indentation + `throw `).

```cpp
TEntidade Converte(const TDado& dado) const {                      // "write" direction
    TEntidade entidade = DoConverte(dado);                          // vtable slot 2
    if (!entidade.isValid() || !entidade.isStrictlyValid()) {       // III ValidChecker + StrictlyValidChecker
        const std::string nome = std::string(typeid(TEntidade).name()) + ": ";
        std::stringstream erro;
        ASN1::trace_invalid(erro, nome.c_str(), entidade);          // InvalidTracer, Portuguese messages
        throw CApiAsnError(EApiAsnError(1900), erro.str());         // iconversorasn.hpp:49
    }
    return entidade;
}
TDado Deconverte(const TEntidade& entidade) const {                // "read" direction: check FIRST
    if (!entidade.isValid() || !entidade.isStrictlyValid()) { … throw CApiAsnError(EApiAsnError(1902), …); }   // :66
    return DoDeconverte(entidade);                                  // vtable slot 3
}
```

* **What is checked.** `isValid` (func 221) and `isStrictlyValid` (func 231) walk the whole object tree through the
  info tables: INTEGER ranges, SIZE constraints of strings and SEQUENCE OF, the selected CHOICE alternative, and for
  ENUMERATED only the upper bound `value <= maxEnumValue` (`ValidChecker::do_visit(const ENUMERATED&)`, func 8984; there
  is no lower bound, see §10.4). A decoded file whose values break the schema is refused *before* any data object is built, and a
  data object that would produce an out-of-range field is refused *before* the file is encoded.
* **Error message.** `"<typeinfo name>: <tracer text>"`. The tracer prints the dotted path of the bad field and the
  reason, so a BU count out of range would read
  `N17ModuloBoletimUrna26DetalhamentoComparecimentoE: .qtdEleitoresHabilitadosPorBiometria Valor 10000 para campo
  INTEGER é maior que seu limite superior 9999\n` (format confirmed at run time with the same tracer, see §6.1;
  `trace_invalid`, func 208, writes prefix + tracer text + `endl`, so the message ends with a newline). The name
  is the raw typeinfo string (the constant pointer is passed by every thunk), not a demangled name. The InvalidTracer
  prints paths, sizes, limits and at most the one offending character of a string (`" O caractere 'x' não é
  válido"`), not whole values.
* **Error type.** `CBaseError<EApiAsnError, SErrorLimits{1900, 1910}>`, 40-byte exception object, constructed by
  func 1074 (a thunk that passes the vtable to the shared `CBaseError` constructor body 710) and thrown with
  `__cxa_throw(obj, typeinfo 1563144, dtor slot 69)`. Only 1900 and 1902 occur in the binary (1901 is unused).
* **No catch here.** The exception goes to whoever loads the file (e.g. the static-data loader, func 7787, or
  `api::CFileASN`) or writes it (the BU and RC writers).
* **Comparison with the uenux2 twin** (`comum::asn::IConversorASN`, u21/u03): same structure, but codes
  `EUeComumAsnError` 7653 (`Converte`, iconversorasn.h:56, `"Entidade deixada em estado inválido: {}"`) and 7654
  (`Desconverte`, :71, `"Entidade está inválida: {}"`), formatted with `std::format` instead of the raw type-name prefix.

---

## 4. How the template looks in the wasm

### 4.1 Thunks and merged bodies

wasm-opt merged all out-of-line instantiations that differ only in constants into **six shared bodies**. Each
instantiation is a 20/21-byte thunk `body(result, this, arg, &srcloc, typeid(TEntidade).name())`:

| body | direction | kind | callers |
|---|---|---|---|
| **682** | Converte | SEQUENCE entities, `invoke_*` + landing pads (on unwind destroys the string, the stringstream and the entity via `ASN1::SEQUENCE::~SEQUENCE`) | 22 thunks |
| **1167** | Deconverte | `invoke_*` + landing pads | 11 thunks |
| **6031** | Converte | the same source **without any landing pad** | 3734, 5365 |
| **6032** | Deconverte | the same source without landing pads | 3733, 5708 |
| **6149** | Deconverte | ENUMERATED entity: `isValid && isStrictlyValid` inlined into `value > info->maxEnumValue` (signed, `info + 16`); small `TDado` returned in a register | 5103 (Fase), 9178 (Turno) |
| **6150** | Converte | ENUMERATED result, same inlined test | 9187 (Turno), 9188 (Fase) |

The **CHOICE** instantiation (`IdentificadorEleitor`: título / CPF / identificação livre) is not merged: name lookup
finds `ASN1::CHOICE::isValid` (func 1068) and `ASN1::CHOICE::isStrictlyValid` (func 1067, still labelled
"comum_f1067") instead of the `AbstractData` versions, so 9111 (Deconverte) and 9113 (Converte) are full copies.

### 4.2 Two exception models for the same template

6031/6032 have the logic of 682/1167 but are compiled with **no `invoke_*`**: nothing is destroyed if
`trace_invalid` or the throw unwinds through them. The code generation differs in one more place: they call
`std::string::append(const char*)` (run-time `strlen` of `": "`) where 682/1167 call `append(": ", 2)` with the length
folded, so they come from a translation unit compiled differently. Their four instantiations are exactly the out-of-line
ones that uenux2 code also calls (`CConversorCarga`, `CConversorDadoCorrespondencia`, `CGravadorRCSecao`,
`CConversorBiometriaEleitor`, `CConversorFotoCandidato`); none of the 33 thunks of 682/1167 has a uenux2 caller.
Inference (not provable from the binary): the linker kept the uenux2 COMDAT copy, so ecourna callers
(`CConversorDadosGeracaoMidia::DoDeconverte` 9202 / `DoConverte` 9203, `CConversorResultadoUrnaCadastro::DoConverte` 9056)
use it too.
This matches a wider pattern: in the decompiled per-file output, **189 of 402 ecourna functions** use `invoke_*`,
against **14 of 634** in `app-comum` and **1 of 388** in `app-vota`. The uenux2 application is built with exception
catching disabled for almost all functions (the RHVoice chapter, `docs/libraries/rhvoice.md` §8.5, found the same for
that library), while the Conan-built ecourna library has full C++ exception support. `comum::asn::CConversorFotoCandidato
::DoDesconverte` (11440, in this unit) is an example of a uenux2 function without landing pads.

### 4.3 Naming trap: ten functions named after an inlined `Deconverte`

A source_location record belongs to the function that contains it after inlining. When `Deconverte` is inlined into
another function, the tool names that function `IConversorASN<X>::Deconverte`. The vtable slot or the call pattern
decides the real identity:

| func | tool name | real identity | evidence |
|---|---|---|---|
| 9057 | `IConversorASN<DadosCifracao,…>::Deconverte` | `CConversorDadosComparecimentoCifrado::DoDeconverte` | vtable slot 3 |
| 9120 | `IConversorASN<IdentificacaoSecaoEleitoral,…>::Deconverte` | `CConversorComparecimentoSecao::DoDeconverte` | vtable slot 3 |
| 9127 | `IConversorASN<MunicipioZona,…>::Deconverte` | `CConversorIdentificacaoSecaoEleitoral::DoDeconverte` | vtable slot 3 |
| 9142 | `IConversorASN<HorariosUrna,…>::Deconverte` | `CConversorConfiguracaoMunicipio::DoDeconverte` | vtable slot 3 |
| 9171 | `IConversorASN<ParametrosUrna,…>::Deconverte` | `CConversorParametrizacaoUrna::DoDeconverte` | vtable slot 3 |
| 9092 | `IConversorASN<ComparecimentoMesario,…>::Deconverte` | `std::transform<…, back_insert_iterator<vector<CComparecimentoMesario>>, λ>` | signature `(first, last, out, op) -> out`, loop over `AbstractData*` |
| 9094 | `IConversorASN<IdentificacaoJustificativa,…>::Deconverte` | `std::transform` (justificativas) | same |
| 9119 | `IConversorASN<EstadoComparecimento,…>::Deconverte` | `std::transform` (voters of the section) | same |
| 9176 | `IConversorASN<Aplicativo,…>::Deconverte` | `std::transform` (applications of a medium, u14's `CConversorInformacaoMidia`) | same |
| 9206 | `IConversorASN<Federacao,…>::Deconverte` | `std::transform` (federations) | same |

The `std::transform` instances have the lambda `[&conversor](const X& x) { return conversor.Deconverte(x); }` inlined:
`op` is passed as a single pointer (the captured converter), and the loop calls `conversor->vtable[3]` after the
inlined validity check. `std::transform` (not `std::ranges::transform`) because the result is one `i32` (the
`back_insert_iterator`), not an `in_out_result` returned through memory.

### 4.4 Other wasm notes

* **Annotation artefact.** The decompiler annotates the constructor call as
  `env_invoke_iiiii(671 /* &simulador::CWasmBeep::vf0 */4 /* "Chave desconhecida: {}" */, …)`. The real operand
  is table slot **6714** = func 1074 (`CBaseError<EApiAsnError>` ctor). The annotator matched slot 671 inside the
  number 6714 and then decoded the remaining `4` as a string address. The raw `q.py wat` output shows
  `i32.const 6714`.
* func 858, labelled `ecourna_f858`, is the libc++ **`std::basic_stringstream<char>` default constructor** (three
  vtables, `ios_base::init`, stringbuf mode 24 = `in|out`). Its 28 callers are all ASN.1 validation error paths: the
  six merged bodies and the twelve other functions of this unit that contain the template code (9057, 9092, 9094,
  9111, 9113, 9119, 9120, 9127, 9142, 9171, 9176, 9206), the u14 converters 9066 (`CConversorHabilitacaoBiometrica`) and 9196 (`CConversorAplicativo`), the
  BU converter 10273, the RDV converters 11349/11351/11352, `CFileASN` 3735/5366, the loader 7787 and the voter-file
  partial reader 11523 (`CEleitores::GetEleitoresEstaticos` lambda, `CPartialFileASN`). All but 11523 also call 1074.
* func 1006 (`~CEstadoComparecimento`) and 1876 (`~CRegistroIdentificacaoEleitor`) show the layout of the attendance
  records: a `CRegistroIdentificacaoEleitor` is `shared_ptr<IIdentificadorEleitor>` (+0) plus
  `optional<shared_ptr<…>>` (+8, flag +16), 20 bytes.
* Constructors return `this` and results larger than a register come back through a hidden result pointer (sret),
  which is how 9029/9034/9045 were identified as the constructors of their only caller's result.

---

## 5. The converter bodies in this unit (what they read and build)

All converters are stateless; a `DoDeconverte` builds its child converters on the stack (vptr store only) and calls
their `Deconverte`, which validates the child entity first. Field numbers refer to the SEQUENCE order of
`src/asn1/*.asn`.

### 5.1 Attendance data — `ModuloResultadoUrnaCadastro` (the file of `comum::CGravadorRCSecao`; "RC" ≈ *resultado para o cadastro*, expansion inferred)

```
EntidadeResultadoUrnaCadastro { cabecalho, fase, versaoVotacao, situacao, infoDadosComparecimento CHOICE {
    [0] dadosComparecimento        DadosComparecimento,
    [1] dadosComparecimentoCifrado DadosComparecimentoCifrado { dadosCifracao { chave, salt, informacaoAdicional }, conteudo } } }
DadosComparecimento { justificativas SEQUENCE OF IdentificacaoJustificativa, identificacaoComparecimento ComparecimentoSecao,
                      mesariosAbertura [1] SEQUENCE OF ComparecimentoMesario OPTIONAL, mesariosEncerramento [2] … OPTIONAL }
ComparecimentoSecao { identificacao IdentificacaoSecaoEleitoral, eleitores SEQUENCE OF EstadoComparecimento }
```

* **`CConversorDadosComparecimento::DoConverte` (9102)** — the direction the urna uses at the end of the day. Order:
  (1) `identificacaoComparecimento` ← `CConversorComparecimentoSecao::Converte(dado.secao)` (thunk 9101 → 682 → 9126),
  assigned with the SEQUENCE assignment helper (func 339); (2) the `justificativas` vector is **copied**, converted
  element by element by the list helper 1970 (thunk 9100, `Converte` = 9097) and assigned with the III copy-and-swap
  (`SEQUENCE_OF(first,last)` = 9099, swap, clear); (3) if the optional `mesariosAbertura` is engaged (byte +52) the
  vector returned by `GetMesariosAbertura()` (func 9024, cdadoscomparecimento.cpp:38, throws *"Comparecimento de
  mesários na abertura não definido para este objeto."* when empty) is copied, converted (9098 → 1970, `Converte` =
  9096), `includeOptionalField(0, 2)` (func 515) and every element cloned into field 2; otherwise
  `removeOptionalField(0)` (func 432); (4) same for `mesariosEncerramento` (byte +68, 9023, optional 1 / field 3). The
  result is checked by the caller's `Converte` (thunk 5365 → no-EH body 6031).
* **`CConversorDadosComparecimento::DoDeconverte` (9095)** — the reverse: copies the `justificativas` SEQUENCE OF
  (`auto`, func 2656), transforms it (9094), converts the section (9093 → 1167 → 9120), then for each present
  optional list copies it and transforms it (9092) into `std::optional<std::vector<CComparecimentoMesario>>`, and
  builds `CDadosComparecimento` (func 3485) from **copies** of the four parts (by-value parameters).
* **`CConversorComparecimentoSecao::DoDeconverte` (9120)** — section id (inlined `Deconverte` → 9127) + one
  `CEstadoComparecimento` (96 bytes) per voter via `std::transform` 9119 → `CComparecimentoSecao(id, eleitores)` (5092).
* **`CConversorIdentificacaoSecaoEleitoral::DoDeconverte` (9127)** — `CMunicipioZona` (inlined `Deconverte` → 9130),
  then `CBaseType<uint,0,9999,8>(local)` and `CBaseType<ushort,0,9999,9>(secao)` (range-checked; `cbasetype.hpp:39`
  throws), → func 5110. Layout `{+0 município (uint), +4 zona (ushort), +8 local (uint), +12 seção (ushort)}`.
* **`CConversorIdentificacaoJustificativa::DoDeconverte` (9064)** — `CRegistroIdentificacaoEleitor` (thunk 9105) +
  `CBaseType<ushort,0,9999,36>(anoNascimentoEleitor)` → `CIdentificacaoJustificativa` (func 1875, which copies the
  registro: two `shared_ptr` add-refs).
* **`CConversorDadosCifracao::DoDeconverte` (9061)** — three OCTET STRINGs → three `std::vector<uebyte>` (helper
  5095) → `CDadosCifracao(chave, salt, informacaoAdicional)` (func 5091, by value; `salt` and `informacaoAdicional`
  must have ≥ 16 bytes, *"O tamanho de salt não pode ser menor do que 16."*).
* **`CConversorDadosComparecimentoCifrado::DoDeconverte` (9057)** — `CDadosCifracao` (inlined `Deconverte` → 9061)
  + `conteudo` (5095) → `CDadosComparecimentoCifrado` (func 3484).

The `…::DoDeconverte` functions of this family were **not** observed at run time: the urna writes this structure,
reading it back is for other tools.

### 5.2 Urna schedule — `ModuloConfiguracaoMunicipios` (`-cfm.dat`)

* **`CConversorConfiguracaoMunicipios::DoDeconverte` (9137)**: cabeçalho (2665), then for each `ConfiguracaoMunicipio`
  a `CConfiguracaoMunicipio` (9136 → 1167 → 9142) pushed into a vector; the vector is turned into
  `std::map<município, CConfiguracaoMunicipio>` by func 9134, which builds and returns the map (sret) through
  `std::insert_iterator` semantics (each insert uses the successor of the previous position as hint; `insert` does not
  overwrite, so a repeated município keeps the **first** entry) and passed to `CConfiguracaoMunicipios(cabecalho, mapa)`
  (func 9028).
* **`CConversorConfiguracaoMunicipio::DoDeconverte` (9142)**: `CBaseType<uint,0,99999,6>(codigoMunicipio)` first,
  then `CHorariosUrna` (inlined `Deconverte` → 9132: four `DataHoraJE` texts parsed to 64-bit times by func 1877).
* Decoded example, scenario `municipal-t1`, `t02410ac-cfm.dat` (BER dump):
  cabeçalho `20260910T162219`, idEleitoral `[2] 2410`; municípios 1, 2, 3, each with
  zerésima `20261004T070000`, início `20261004T080000`, encerramento `20261004T170000`, término `20261005T040000`.

### 5.3 Urna parameters — `ModuloParametrizacaoUrna` (`-pu.dat`)

**`CConversorParametrizacaoUrna::DoDeconverte` (9171)**: cabeçalho (2665) and `ParametrosUrna` (inlined `Deconverte`
→ `CConversorParametrosUrna::DoDeconverte` 9156, u14), then `CParametrizacaoUrna(cabecalho, parametros)` (func 9029,
copies the ~400-byte `CParametrosUrna` with func 3777) and destroys the temporary (2267). The `TituloRelatorio` and
`LabelParametrizado` thunks (9153/9154) run inside 9156 (9153 through the list helper 5102). This is the most expensive function of the unit at run time
(46/54 samples), and more than half of it is the cabeçalho (func 9218 parses the date-time with boost).

### 5.4 Federations — `ModuloFederacoes` (`-fe.dat`)

**`CConversorFederacoes::DoDeconverte` (9207)**: cabeçalho, then — only if the OPTIONAL list is present — a
`std::transform` (9206) of each `Federacao` (inlined `Deconverte` → 9216) into `std::vector<CFederacao>`, moved into
the result list, and `CFederacoes(cabecalho, lista)` (func 9045). **All 12 `-fe.dat` files of the simulator
scenarios are 25 bytes: they contain only the cabeçalho** (checked by BER-dumping them), so the list is always empty
in the simulator.

### 5.5 Media description — `ModuloInformacaoMidia` (`infomidia-*.dat`)

* **`CConversorAutenticacao::DoDeconverte` (9200)**: reads `tamanhoSenha` (field 2), `numeroTentativas` (field 3),
  copies `hashSenha`, and **only if both** `dataInicial` and `dataFinal` are present parses them (func 1877) and uses
  the constructor with a validity window (9040); otherwise the constructor without dates (9037). The object stores
  `numeroTentativas` at +32 and `tamanhoSenha` at +36 (both conversion directions agree, §9).
* `CDadosGeracaoMidia` constructor (9034): serial of the medium, user, `IdentificadorGeradorMidia` (3 strings; in the
  simulator `"simulador-votacao-ng"` and an all-zero TPM serial, see `docs/00-provenance.md`) and the date.
* `std::transform` 9176 / slow path 9174: the `aplicativos` list of a result medium (u14's `CConversorInformacaoMidia`).

### 5.6 Candidate photos (uenux2 side)

**`comum::asn::CConversorFotoCandidato::DoDesconverte` (11440)**: copies `codigoCandidato`, converts the `Foto`
through the ecourna `CConversorFoto` (5708 → 6032 → 9226) and returns `md::CFotoCandidato{codigo, foto}`. Observed at
run time in both votes, called from `(anonymous)::LeEntidadeEm` (3755), which decodes one `FotoCandidato` at the
offset indexed by `CVisitanteFoto` (u03 §5.6). `DoConverte` is not overridden (slot 2 = base 11439 that throws
*"Método DoConverte() não implementado para {}"*): photos are never written.

---

## 6. Run-time behaviour (web simulator)

The 16 functions seen in the profiles run when the static data are loaded during `votaInit` (the big loader
func 7787, `vota::CInformacaoEleitor::Inicializar`, formerly shown by the tools as `CHKDFSeed::GetSeed`, calls 9137,
9171 and 9207), when the urna's general state is decoded or
encoded (`comum::asn::CConversorEstadoGeral::DesconverteEstadoUrna` 11398 / `ConverteEstadoUrna` 11397 →
`CConversorDadoCorrespondencia`), or when a candidate photo is decoded:

| file read | chain observed |
|---|---|
| `-pu.dat` (national + UF) | 7787 → **9171** → 2665 → **1167** → 9218; 9171 → **9029**; 9171 → 9156 → 5102 → **9153**; 9156 → **9154** |
| `-cfm.dat` | 7787 → **9137** → 2665; **9136** → 1167 → **9142**; 9134; 9028 |
| `-fe.dat` | 7787 → **9207** → 2665 (the list is absent) |
| general state, `DadoCorrespondencia` (`CConversorDadoCorrespondencia`, u21) | 11397 → 5691 → 1563 → 11399 → **3734** → **6031**; 11398 → 5692 → 1008 → 11400 → **3733** → **6032** → 9224 |
| `-fo.dat` photo | 3755 → **11440** → **5708** → 6032 → 9226 |

No `1900`/`1902` error is raised in the recorded sessions. The attendance converters (§5.1) did not run: they belong
to the end-of-day flow.

### 6.1 Experiment: an out-of-range value in a data file

A copy of scenario `municipal-t1` was made in a temporary directory with one byte of `t02410ac-cfm.dat` changed: the first
`codigoMunicipio` `02 01 01` → `02 01 00` (0 is outside `INTEGER (1..99999)`). Running
`node tools/run/headless.mjs --scenario <relative path to the copy> --events` gives:

```
[vota_web_wasm] static T api::CFileASN::DecodeObjectFunction(const std::vector<char> &, const std::string &)
  [T = ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios]:143:5954 - Conteúdo inválido para ReadFromFile de
  /dsk/fi/estatico/t02410ac-cfm.dat: .configuracoes[0].codigoMunicipio Valor 0 para campo INTEGER é menor que seu limite inferior 1
event vota:error {...same message...}
votaInit(...) -> 0
```

The file layer (`api::CFileASN::DecodeObjectFunction`, `cfileasn.h:143`, `EUeIoError` 5954, unit u18) runs the same
`isValid`/`isStrictlyValid` + `trace_invalid` right after BER decoding, so the bad file is refused **before** the
converter is called. For files read through `CFileASN`, the `Deconverte` check of this template (1902) is a second,
redundant validation, and nested converters validate the same sub-trees again at every level (the cabeçalho of
`-cfm.dat` is validated by the file layer, by `IConversorASN<EntidadeConfiguracaoMunicipios>::Deconverte` inlined in
7787 (srcloc :66) and by the thunk 2665 called from 9137; each `ConfiguracaoMunicipio` twice more by 9136 and the
inlined `HorariosUrna` check in 9142). Symmetrically, `CFileASN::CodeObjectFunction` (`cfileasn.h:161/171`, 5955/5956) re-checks what `Converte`
(1900) already checked before encoding. `votaInit` fails cleanly (returns 0, emits `vota:error`).

---

## 7. BOLETIM DE URNA (BU) and the end-of-day files

This unit does not generate the BU, but its template guards two things that are written at the end of the day
(*encerramento*).

### 7.1 BU field `detalhamentoComparecimento`

In `EntidadeBoletimUrna` (`ModuloBoletimUrna`), the 8th field (index 7) is
`detalhamentoComparecimento [1] DetalhamentoComparecimento OPTIONAL` with
`{ qtdEleitoresCompareceramSemBiometria, qtdEleitoresHabilitadosPorBiometria, qtdEleitoresHabilitadosPorBiografia }`,
each `INTEGER (0..9999)`. Step by step, inside `comum::asn::CConversorEntidadeBU::DoConverte` (func 10273,
`cconversorentidadebu.cpp:232`, unit u23):

1. the BU model (`comum::md::CEntidadeBU`) returns its `ecourna::app::dados::CDetalhamentoComparecimento` through
   `GetDetalhamentoComparecimento()` (`centidadebu.cpp:206`; the srcloc record implies it throws when the BU has none);
2. an `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento` is built on the stack (vtable 1123424) and its
   **`IConversorASN<DetalhamentoComparecimento, CDetalhamentoComparecimento>::Converte` is inlined** (srcloc
   `iconversorasn.hpp:49`): `DoConverte` (func 9222) creates the three `Constrained_INTEGER<0..9999>` fields;
3. the result is validated (`isValid` + `isStrictlyValid`); a count above 9999 makes the BU generation fail with
   `CBaseError<EApiAsnError>` **1900** and the message
   `"N17ModuloBoletimUrna26DetalhamentoComparecimentoE: <tracer>"` (the string constant @545375 is referenced by 10273);
4. otherwise the SEQUENCE becomes the optional field of the BU entity, which the BU code of other units (u23, u08/u09)
   then BER-encodes, hashes/signs, stores and prints.

So the attendance breakdown printed on and stored in the BU is range-checked by this template at encoding time, like
every other BU field that goes through an `IConversorASN` (the uenux2 twin guards the rest).

### 7.2 The attendance file ("RC") written with the BU

`comum::CGravadorRCSecao::GravaResultado` (inlined into func 11616, `cgravadorrcsecao.cpp:110`, unit u23) writes the
section's attendance (who voted, justified, mesários) for the voter-registry system. The parts in this unit:

1. `CDadosComparecimento` → `DadosComparecimento` with `IConversorASN<DadosComparecimento>::Converte` (thunk 5365 →
   no-landing-pad body 6031 → `CConversorDadosComparecimento::DoConverte` 9102, §5.1), then validated (1900 on error);
2. the BER of `DadosComparecimento` is either stored in clear (`infoDadosComparecimento [0]`) or encrypted with the
   public key file **`jufa.pk1`** (`LeChavePublica`, errors 8655 "O arquivo … " at cgravadorrcsecao.cpp:65 and 8656
   "O arquivo … está vazio" at :83) into `DadosComparecimentoCifrado { dadosCifracao { chave, salt (≥16),
   informacaoAdicional (≥16) }, conteudo }` (`[1]`). Before that, the `DadosComparecimento` is BER-encoded by an inlined
   `api::CFileASN::CodeObjectFunction<DadosComparecimento>` (cfileasn.h:161/171 in 11616). According to u14 the choice
   follows the `-pu.dat` parameter *criptografarJUFA*, which is **TRUE in every simulator scenario**. **In the web build
   CEPESC does not encrypt** (u01 §4.1), so an "encrypted" variant would carry the plaintext and an all-zero key; but
   since no scenario ships `jufa.pk1`, `LeChavePublica` would throw 8655 first, and the web page never reaches the
   end-of-day flow anyway (u14 §10);
3. the envelope `EntidadeResultadoUrnaCadastro { cabecalho, fase, versaoVotacao, situacao, infoDadosComparecimento }`
   is converted with the ecourna `IConversorASN<EntidadeResultadoUrnaCadastro>::Converte` inlined into
   `api::CFileASN::CodeObjectFunction` (func 5366, srcloc :49, code 1900) and BER-encoded
   (`cfileasn.h:161/171`: *"Objeto com conteúdo inválido para {}: {}"*). The string `"10.23.0.1 - DESENVOLVIMENTO"`
   referenced by 11616 is presumably `versaoVotacao`.

The converters that READ these structures back (9057, 9061, 9095, 9120, …) are compiled in but unused by VOTA.

---

## 8. Web-build specifics

* Nothing in this unit is mocked: the template, the converters and the data files are the real ones.
* Security that is simulated elsewhere shows through here: the `DadosComparecimentoCifrado` path would carry
  plaintext (CEPESC stub, u01). No scenario package ships a `jufa.pk1` file (searched `upstream/fs/`), and every
  scenario sets `criptografarJUFA = TRUE`, so writing the RC would fail in `LeChavePublica` (8655) before anything is
  encrypted; the page does not run the end-of-day flow in the first place (u14 §10).
* Exception model (§4.2): most uenux2 code of this build, including `CConversorFotoCandidato` and the no-EH copies
  6031/6032 of this template, has no landing pads. An ASN.1 validation error raised there propagates as a JS
  exception without running destructors (memory leak only; the first `invoke_*` up the stack still catches it).
* The simulator scenarios have no party federations (§5.4) and identical schedules for all municipalities (§5.2).

---

## 9. Notes for other units (cross-checks)

* **u14 `CAutenticacao` layout.** `src/ecourna/app/dados/midias/cinformacaomidia.h` currently puts `m_tamanhoSenha` at
  +32 and `m_numeroTentativas` at +36. Both directions of the converter say the opposite: `DoConverte` (9201) writes
  ASN.1 field 3 = `numeroTentativas` (names table @1627872: dataInicial, dataFinal, tamanhoSenha, numeroTentativas,
  hashSenha) from +32 and field 2 = `tamanhoSenha` from +36; `DoDeconverte` (9200) passes field 3 first, which the
  constructors 9037/9040 store at +32. Also `sizeof(CAutenticacao)` is 56 (vector ends at +52, 8-byte alignment).
* **u14 `CDadosComparecimento`.** RTTI names a separate class `ecourna::app::dados::CComparecimentoSecao`
  (`IConversorASN<ComparecimentoSecao, CComparecimentoSecao>`), and 9102 passes `&dado + 12` to its `Converte`: the
  bytes +12..+40 are one `CComparecimentoSecao` member (`CIdentificacaoSecaoEleitoral` 16 bytes + `vector
  <CEstadoComparecimento>`), not two flattened members. Its implicit destructor is func 5097.
* **u14 `CDadosGeracaoMidia` / `CAutenticacao` constructors**: funcs 9034, 9037, 9040 (this unit) should be added to
  the declarations in `cinformacaomidia.h`; `CConfiguracaoMunicipios` is built from a `TMapConfiguracaoMunicipio`
  produced by func 9134 (§5.2).
* **u02** called func 9029 a `std::pair<const K, CParametrosUrna>` constructor. Its only caller constructs its own
  result with it, and the result type is the RTTI-named class `CParametrizacaoUrna`, so it is that class's
  constructor.
* **Names to fix in the database**: `comum_f1067` = `ASN1::CHOICE::isStrictlyValid() const`; `ecourna_f858` =
  `std::basic_stringstream<char>::basic_stringstream()`; `ecourna_f1074` = the `CBaseError<EApiAsnError>`
  constructor; the ten functions of §4.3; the `vfN` names of 9061, 9102, 9137, 9200, 11440.

---

## 10. Weird or risky code

1. **Two exception models for one template (6031/6032 vs 682/1167), and no landing pads in uenux2 code (11440).**
   The copies used for `IdentificadorGeradorMidia`, `DadosComparecimento` (`Converte`) and `Foto` (`Deconverte`)
   destroy nothing when an ASN.1 validation error unwinds through them (string, stringstream and, for `Converte`, the
   already built entity leak). The linker's COMDAT choice also gives these copies to ecourna callers. This is a
   property of the Emscripten build flags; whether the native urna build has the same split is unknown. Impact: a
   leak on an error path that aborts the load anyway (and §6.1 shows the file layer usually rejects bad data first).
   Low.
2. **Validity window of a medium's password is all-or-nothing (9200, and 9201).** An `Autenticacao` with only
   `dataInicial` or only `dataFinal` is valid ASN.1, but the converter drops the lone date and builds an object
   without any window. This is a class invariant rather than a slip: `GetDataHoraInicial` (9036) itself tests
   **both** engaged flags (+8 and +24) before returning, and throws 3041 otherwise. What the loss means for the password
   check could not be established: in this binary the only caller of `GetDataHoraInicial/Final` (9036/9035) is the
   re-encoder `CConversorAutenticacao::DoConverte` (9201), so VOTA never evaluates the window (a lone date would only
   vanish if the medium description were re-written). Info.
3. **Duplicate municipalities in `-cfm.dat` are silently ignored (9137 + 9134).** The map insert keeps the first
   schedule for a município and drops later ones without a log or error. Data are produced by TSE tools, so only a
   malformed file is affected. Low.
4. **ENUMERATED validation is only an upper bound (6149/6150).** `value > maxEnumValue` (signed) is the whole
   check, so `Fase 0` or negative values pass `IConversorASN`. The inlining is exact: the III runtime itself only
   checks the upper bound (`ValidChecker::do_visit(const ENUMERATED&)`, func 8984, and `InvalidTracer` 8937), so the file
   layer has the same gap for every ENUMERATED. `CConversorFase::DoDeconverte` (9193) rejects them again (*"Fase
   inválida."*, 2250/2251) and `CConversorTurno::DoDeconverte` (9191) accepts only 1..2 (2253, which also rejects the
   schema's own `semTurno (0)`), so there is no effect today. Info.
5. **Error messages carry mangled type names** (`N27ModuloResultadoUrnaCadastro19DadosComparecimentoE: …`), which is
   what a user or log would see if an `IConversorASN` check fails, e.g. a BU attendance count above 9999 (§7.1). A
   rejected data file does not show it: the `CFileASN` message (file path, empty tracer prefix) comes first (§6.1,
   reproduced). Cosmetic. Info.
6. **Deep copies on every conversion.** `DoDeconverte` 9095 copies whole `SEQUENCE OF` trees with `auto`, and 9102 /
   9095 / 9057 / 9061 copy every vector again to call by-value constructors. Harmless for one section (a few hundred
   voters). Info.
7. **Plaintext "encrypted" attendance in the web build** (§7.2): the unit only transports `DadosCifracao`, but in
   this build its content would come from the CEPESC stub (zero key, plaintext). In practice unreachable: no scenario
   ships `jufa.pk1` (with `criptografarJUFA = TRUE` the writer throws 8655 first) and the page never runs the
   end-of-day flow. Simulated security, simulator only. Info.
8. **Redundant validation** (§6.1): every file is validated by `CFileASN` and again by each nested `Deconverte`; the
   1902 path is practically unreachable for files and the repeated walks cost start-up time (the profiles show the
   `-pu.dat` conversion as the heaviest function of this unit). Info.

---

## 11. Open questions

* The exact source form of func 9134 (vector → `std::map`: an out-of-line function that constructs the map it returns
  and fills it with `std::insert_iterator` semantics) and of the list helper 1970 (`ConverteLista` here) is not
  recoverable; the reconstruction uses equivalent code marked `?` (`ConverteParaMapa`, name inferred).
* 5095 is out of line and shared by two converters, which suggests an inline helper in a header (named
  `ConverteOctetString` here, name and location inferred) or a C++23 `std::ranges::to`.
* Code 1901 of `EApiAsnError` is never thrown in this binary; its meaning is unknown.
* Whether the web simulator can reach the end-of-day flow that writes the RC (§7.2) was not tested here.

---

## 12. Complete mapping table (all 90 functions)

"run" = seen executing during the recorded votes. `:49` / `:66` = line of `iconversorasn.hpp` whose srcloc names the
function. Reconstructed files are under `src/` with the same path (`ecourna-lib/ecourna/…` → `src/ecourna/…`).

| func | size | run | reconstructed symbol | kind | original file |
|---|---|---|---|---|---|
| 306 | 16 |  | `std::__exception_guard_exceptions<std::vector<T>::__destroy_vector>::~__exception_guard_exceptions` | ICF: every trivially destructible T (42 callers: byte vectors of ecourna, boost regex, sqlite blobs, …) | library (libc++) |
| 682 | 654 |  | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Converte (merged body)` | 22 thunks | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 858 | 304 |  | `std::basic_stringstream<char>::basic_stringstream` | default ctor (mode in\|out = 24) | library (libc++) |
| 1006 | 257 |  | `ecourna::app::dados::CEstadoComparecimento::~CEstadoComparecimento` | implicit dtor | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h (implicit; u11-foreign-fragments.cpp) |
| 1074 | 18 |  | `ecourna::api::exception::CBaseError<ecourna::api::asn::EApiAsnError, ecourna::api::exception::SErrorLimits{1900, 1910}>::CBaseError` | thunk -> 710 | CBaseError class template (header not attested); instantiated by ecourna-lib/ecourna/api/asn/iconversorasn.hpp |
| 1167 | 516 | yes | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Deconverte (merged body)` | 11 thunks | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 1876 | 114 |  | `ecourna::app::dados::CRegistroIdentificacaoEleitor::~CRegistroIdentificacaoEleitor` | implicit dtor | ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.h (implicit; u11-foreign-fragments.cpp) |
| 2660 | 15 |  | `std::vector<ecourna::app::dados::CComparecimentoMesario>::__destroy_vector::operator()` | -> 6145(40,36,28,24) | library (libc++) |
| 2661 | 15 |  | `std::vector<ecourna::app::dados::CComparecimentoMesario>::~vector` | -> 6146(40,...) | library (libc++) |
| 2665 | 20 | yes | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, ecourna::app::dados::CCabecalhoEntidade>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 3488 | 15 |  | `std::vector<ecourna::app::dados::CIdentificacaoJustificativa>::~vector` | -> 6146(24,...) | library (libc++) |
| 3733 | 20 | yes | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, ecourna::app::dados::CIdentificadorGeradorMidia>::Deconverte` | thunk -> 6032 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 3734 | 20 | yes | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, ecourna::app::dados::CIdentificadorGeradorMidia>::Converte` | thunk -> 6031 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 5095 | 221 |  | `ecourna::app::dados::asn::ConverteOctetString` | inline helper | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.h (name and path inferred) |
| 5096 | 87 |  | `std::__exception_guard_exceptions<std::vector<ecourna::app::dados::CEstadoComparecimento>::__destroy_vector>::~__exception_guard_exceptions` |  | library (libc++) |
| 5097 | 75 |  | `ecourna::app::dados::CComparecimentoSecao::~CComparecimentoSecao` | implicit dtor | ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.h (implicit; u11-foreign-fragments.cpp) |
| 5099 | 15 |  | `std::vector<ecourna::app::dados::CIdentificacaoJustificativa>::__destroy_vector::operator()` | -> 6145(24,20,12,8) | library (libc++) |
| 5100 | 75 |  | `std::vector<ecourna::app::dados::CEstadoComparecimento>::~vector` |  | library (libc++) |
| 5103 | 18 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Fase, ecourna::app::dados::CFaseID>::Deconverte` | thunk -> 6149 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 5365 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, ecourna::app::dados::CDadosComparecimento>::Converte` | thunk -> 6031 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 5708 | 20 | yes | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Foto, ecourna::app::dados::CFoto>::Deconverte` | thunk -> 6032 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6031 | 151 | yes | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Converte (merged body, no landing pads)` | thunks 3734, 5365 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 6032 | 151 | yes | `ecourna::api::asn::IConversorASN<ENTIDADE, DADO>::Deconverte (merged body, no landing pads)` | thunks 3733, 5708 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6145 | 192 |  | `std::vector<T>::__destroy_vector::operator() (merged body, T starts with CRegistroIdentificacaoEleitor)` | thunks 2660, 5099 | library (libc++) |
| 6146 | 188 |  | `std::vector<T>::~vector (merged body, T starts with CRegistroIdentificacaoEleitor)` | thunks 2661, 3488 | library (libc++) |
| 6149 | 513 |  | `ecourna::api::asn::IConversorASN<ENUMERATED, DADO>::Deconverte (merged body)` | thunks 5103, 9178 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 6150 | 515 |  | `ecourna::api::asn::IConversorASN<ENUMERATED, DADO>::Converte (merged body)` | thunks 9187, 9188 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9029 | 34 | yes | `ecourna::app::dados::CParametrizacaoUrna::CParametrizacaoUrna` | ctor | ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.cpp (path inferred) |
| 9034 | 474 |  | `ecourna::app::dados::CDadosGeracaoMidia::CDadosGeracaoMidia` | ctor | ecourna-lib/ecourna/app/dados/midias/cdadosgeracaomidia.cpp (path inferred) |
| 9037 | 275 |  | `ecourna::app::dados::CAutenticacao::CAutenticacao` | ctor (no dates) | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp (u14; fragment .u11.cpp) |
| 9040 | 291 |  | `ecourna::app::dados::CAutenticacao::CAutenticacao` | ctor (with dates) | ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp (u14; fragment .u11.cpp) |
| 9045 | 569 |  | `ecourna::app::dados::CFederacoes::CFederacoes` | ctor | ecourna-lib/ecourna/app/dados/federacoes/cfederacoes.cpp (path inferred) |
| 9048 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, ecourna::app::dados::CDadosComparecimentoCifrado>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9050 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, ecourna::app::dados::CDadosComparecimento>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9053 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, ecourna::app::dados::CDadosComparecimentoCifrado>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9057 | 1263 |  | `ecourna::app::dados::asn::CConversorDadosComparecimentoCifrado::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp (path inferred) |
| 9059 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::DadosCifracao, ecourna::app::dados::CDadosCifracao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9061 | 1204 |  | `ecourna::app::dados::asn::CConversorDadosCifracao::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp (path inferred) |
| 9064 | 343 |  | `ecourna::app::dados::asn::CConversorIdentificacaoJustificativa::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp (path inferred) |
| 9067 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo, ecourna::app::dados::CEstadoHabilitacaoPorCodigo>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9076 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, ecourna::app::dados::CApresentacaoFotoEleitor>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9078 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, ecourna::app::dados::CHabilitacaoBiometrica>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9082 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, ecourna::app::dados::CHabilitacaoBiometrica>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9084 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, ecourna::app::dados::CApresentacaoFotoEleitor>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9092 | 888 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::ComparecimentoMesario>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CComparecimentoMesario>>, CConversorDadosComparecimento::DoDeconverte::$lambda>` | lambda instance | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (path inferred) |
| 9093 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, ecourna::app::dados::CComparecimentoSecao>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9094 | 878 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CIdentificacaoJustificativa>>, CConversorDadosComparecimento::DoDeconverte::$lambda>` | lambda instance | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (path inferred) |
| 9095 | 3926 |  | `ecourna::app::dados::asn::CConversorDadosComparecimento::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (path inferred) |
| 9096 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoMesario, ecourna::app::dados::CComparecimentoMesario>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9097 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa, ecourna::app::dados::CIdentificacaoJustificativa>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9099 | 26 |  | `ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa>::SEQUENCE_OF` | (first, last) -> 2926 | library (III ASN.1 template) |
| 9101 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, ecourna::app::dados::CComparecimentoSecao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9102 | 3374 |  | `ecourna::app::dados::asn::CConversorDadosComparecimento::DoConverte` | vtable slot 2 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.cpp (path inferred) |
| 9105 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, ecourna::app::dados::CRegistroIdentificacaoEleitor>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9109 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, ecourna::app::dados::CRegistroIdentificacaoEleitor>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9111 | 521 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, ecourna::app::dados::CIdentificadorEleitor>::Deconverte` | full copy (CHOICE::isValid/isStrictlyValid) | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9113 | 659 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, ecourna::app::dados::CIdentificadorEleitor>::Converte` | full copy (CHOICE) | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9118 | 1106 |  | `std::vector<ecourna::app::dados::CEstadoComparecimento>::push_back` | push_back(T&&) fast + slow path | library (libc++) |
| 9119 | 881 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::EstadoComparecimento>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CEstadoComparecimento>>, CConversorComparecimentoSecao::DoDeconverte::$lambda>` | lambda instance | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (path inferred) |
| 9120 | 818 |  | `ecourna::app::dados::asn::CConversorComparecimentoSecao::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (path inferred) |
| 9121 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloResultadoUrnaCadastro::EstadoComparecimento, ecourna::app::dados::CEstadoComparecimento>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9124 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, ecourna::app::dados::CIdentificacaoSecaoEleitoral>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9127 | 631 |  | `ecourna::app::dados::asn::CConversorIdentificacaoSecaoEleitoral::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp (path inferred) |
| 9128 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::MunicipioZona, ecourna::app::dados::CMunicipioZona>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9136 | 21 | yes | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, ecourna::app::dados::CConfiguracaoMunicipio>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9137 | 833 | yes | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipios::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp (path inferred) |
| 9138 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, ecourna::app::dados::CConfiguracaoMunicipio>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9142 | 643 | yes | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipio::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp (path inferred) |
| 9143 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloConfiguracaoMunicipios::HorariosUrna, ecourna::app::dados::CHorariosUrna>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9153 | 21 | yes | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, ecourna::app::dados::CTituloRelatorio>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9154 | 21 | yes | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, ecourna::app::dados::CLabelParametrizado>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9157 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, ecourna::app::dados::CTituloRelatorio>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9159 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, ecourna::app::dados::CLabelParametrizado>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9171 | 677 | yes | `ecourna::app::dados::asn::CConversorParametrizacaoUrna::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp (path inferred) |
| 9172 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloParametrizacaoUrna::ParametrosUrna, ecourna::app::dados::CParametrosUrna>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9174 | 620 |  | `std::vector<ecourna::app::dados::CAplicativo>::__push_back_slow_path` | T&& slow path | library (libc++) |
| 9176 | 903 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloInformacaoMidia::Aplicativo>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CAplicativo>>, CConversorInformacaoMidia::DoDeconverte::$lambda>` | lambda instance | ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp (u14; fragment .u11.cpp) |
| 9177 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, ecourna::app::dados::CDadosGeracaoMidia>::Deconverte` | thunk -> 1167 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9178 | 18 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Turno, CBaseType<unsigned short, 1, 2, 11>>::Deconverte` | thunk -> 6149 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:66 |
| 9180 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::Aplicativo, ecourna::app::dados::CAplicativo>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9186 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, ecourna::app::dados::CDadosGeracaoMidia>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9187 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Turno, CBaseType<unsigned short, 1, 2, 11>>::Converte` | thunk -> 6150 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9188 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::Fase, ecourna::app::dados::CFaseID>::Converte` | thunk -> 6150 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9197 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloInformacaoMidia::Autenticacao, ecourna::app::dados::CAutenticacao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9200 | 657 |  | `ecourna::app::dados::asn::CConversorAutenticacao::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp (path inferred) |
| 9206 | 961 |  | `std::transform<ASN1::SEQUENCE_OF<ModuloFederacoes::Federacao>::const_iterator, std::back_insert_iterator<std::vector<ecourna::app::dados::CFederacao>>, CConversorFederacoes::DoDeconverte::$lambda>` | lambda instance | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (path inferred) |
| 9207 | 410 | yes | `ecourna::app::dados::asn::CConversorFederacoes::DoDeconverte` | vtable slot 3 | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (path inferred) |
| 9208 | 21 |  | `ecourna::api::asn::IConversorASN<ModuloFederacoes::Federacao, ecourna::app::dados::CFederacao>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 9211 | 20 |  | `ecourna::api::asn::IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, ecourna::app::dados::CCabecalhoEntidade>::Converte` | thunk -> 682 | ecourna-lib/ecourna/api/asn/iconversorasn.hpp:49 |
| 11440 | 346 | yes | `comum::asn::CConversorFotoCandidato::DoDesconverte` | vtable slot 3 | uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp (path inferred) |

---

## 13. Reconstructed files

| file | contents |
|---|---|
| `src/ecourna/api/asn/iconversorasn.hpp` | `IConversorASN<ENTIDADE, DADO>` (Converte line 49, Deconverte line 66), `EApiAsnError`, `CApiAsnError`, notes on the merged bodies |
| `src/ecourna/api/asn/iconversorasn.instances.cpp` | explicit-instantiation map: every thunk index, its body, srcloc record and callers |
| `src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimento.{h,cpp}` | 9095, 9102 (+ 9092, 9094, 9099, 6145/6146 family, 5097 notes) |
| `…/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp` | 9057 |
| `…/resultadournacadastro/cconversordadoscifracao.{h,cpp}` | 9061, helper 5095 |
| `…/resultadournacadastro/cconversoridentificacaojustificativa.{h,cpp}` | 9064 |
| `…/resultadournacadastro/cconversorcomparecimentosecao.{h,cpp}` | 9120 (+ 9119, 9118, 5100, 5096, 1006 notes) |
| `src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.{h,cpp}` | 9127 |
| `src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.{h,cpp}` | 9142 |
| `src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp` | 9137 |
| `src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp` | 9171 |
| `src/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp` | 9200 |
| `src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.u11.cpp` | fragment for u14: 9176, 9174 |
| `src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp` | 9207 (+ 9206) |
| `src/ecourna/app/dados/parametrizacaourna/cparametrizacaourna.{h,cpp}` | 9029 |
| `src/ecourna/app/dados/federacoes/cfederacoes.{h,cpp}` | 9045 |
| `src/ecourna/app/dados/midias/cdadosgeracaomidia.cpp` | 9034 (class declared by u14 in `cinformacaomidia.h`) |
| `src/ecourna/app/dados/midias/cautenticacao.u11.cpp` | fragment for u14: 9037, 9040 |
| `src/ecourna/app/dados/u11-foreign-fragments.cpp` | implicit destructors 1876, 1006, 5097 |
| `src/uenux2/src/app/comum/dados/asn/candidatura/cconversorfotocandidato.cpp` | 11440 |

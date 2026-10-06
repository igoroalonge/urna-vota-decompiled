# u40: `ecourna` classes without a known source file (errors, RNGs, cipher factory, CEPESC stub, ASN.1 converters)

Unit u40 holds **101 wasm functions** that the tools put in component `lib:ecourna` but could not attach to a
source file: the classes have no `std::source_location` record (they throw nothing of their own, or they are the
exception classes themselves). About 60 are real `ecourna` code; the rest are library template
instantiations, wasm-opt merged bodies, static-object destructors and a few pieces of `uenux2/src/app/comum` that
landed here because of their neighbours in the function table. This document names every one of them.

Only **10 of the 101 ran** in the recorded votes (`analysis/runtime/*.functions.tsv`): three converters/helpers of
the start-up file loading (`ConverteDataHoraJE` 1877, `CConversorHorariosUrna::DoDeconverte` 9132,
`CConversorIdentificadorGeradorMidia::DoDeconverte` 9224, the last one reached from
`comum::asn::CConversorDadoCorrespondencia::DoDesconverte` 11400 while `eg.bin` is read), the RDV cipher construction at `votaInit`
(`CSha512::CSha512` 2684, `CSymmetricCipherFactory::Create(CAesKey)` 9489), the candidate list of the voting
screen (`CCandidaturas::GetNumerosCandidatosAptos` 2840) and four libc++ helpers (986, 1108, 2898, 3340). Several
claims below were also checked by **calling the functions in the running simulator** (a copy of
`tools/run/headless.mjs` that calls table slots after the run; §4.3, §6). The fidelity review re-ran that probe
independently (slots 6735, 114 and the `CPrng` vtable) and got the same values.

Glossary: *urna* voting machine; *eleitor* voter; *mesário* poll worker; *seção* polling section; *zona* electoral
zone; *município* municipality; *título (de eleitor)* voter registration number; *comparecimento* attendance;
*justificativa* declaration of a voter who cannot vote in his section; *habilitação* releasing a voter to vote
(*por biometria* = by fingerprint, *por biografia* = by the mesário checking the voter's data, *por código* = with a
release code); *BU (boletim de urna)* the section's result; *zerésima* zero report printed before voting; *RDV
(registro digital do voto)* shuffled table of cast votes; *carga* preparation of the urna's flash card; *mídia*
medium (flash card, pen drive); *cabeçalho* header; *pleito* election day; *federação* party federation; *CEPESC*
the government crypto library used for result files; `-jufa.dat` the TSE's "registro de comparecimento de
eleitores e mesários" (attendance of voters and poll workers) sent back to the voter registry (*cadastro*).

## Reconstructed sources

All paths are *(path inferred)* unless marked "attested" (the file appears in `analysis/srcloc.tsv`). New files
are complete; `*.u40.cpp` files are fragments of files owned by other units (the owning unit is named inside).

```
src/ecourna/api/exception/cerror.hpp, cerror.cpp            CError (1143, 4636, 1940, 11562)
src/ecourna/api/exception/cbaseerror.hpp                    SErrorLimits, CBaseError<E,L> (710, 1011, 2379; thunk list incl. 9181)
src/ecourna/api/security/irng.hpp                           IRng (slot layout, return codes)
src/ecourna/api/security/ctrng.hpp, ctrng.cpp               CTrng (9477, 9476, 9475, 5173; static 9478)
src/ecourna/api/security/cprng.hpp, cprng.cpp               CPrng (9503, 9502, 9500, 9499, 5178; twist 5179)
src/ecourna/api/security/isymmetriccipherfactory.hpp        ISymmetricCipherFactory
src/ecourna/api/security/csymmetriccipherfactory.hpp/.cpp   CSymmetricCipherFactory (1884, 9489)
src/ecourna/api/security/cepesc/ccepesccipher.hpp/.cpp      IAsymmetricCipher<>, CCepescCipher (2681; 5171 repeated from u03)
src/ecourna/api/security/security.u40.cpp                   fragments: CSha512 ctor 2684, ~CAesKey 3517, ~CBlockCipher D0 9488,
                                                            s_cipherTable dtor 9496, vector::insert 9492, CInfoSalt copy 9470, unique_ptr 5170
src/ecourna/api/util/datahora.hpp, datahora.cpp             ConverteDataHoraJE (1877), FormataDataHoraJE (9220)
src/ecourna/api/util/cstringutils.u40.cpp                   CStringUtils::Replace (9398)          [cstringutils.cpp attested]
src/ecourna/api/compression/clzmacompress.u40.cpp           CLzmaCompress::OnProgress (9585), static mutex 8378 [attested]
src/ecourna/api/pattern/iobservableprogresswithdescription.u40.cpp   deleting dtor 9528, signals2 map insert 9551
src/ecourna/app/dados/ccabecalhoentidade.h/.cpp             CCabecalhoEntidade (5112)
src/ecourna/app/dados/cidentificacaosecaoeleitoral.h/.cpp   CMunicipioZona, CIdentificacaoSecaoEleitoral (5110)
src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp   (5087, 2657)  [attested]
src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.u40.cpp    (2658)        [attested]
src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp   (3483, 3482)  [attested]
src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp  (3484)
src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.h/.cpp      (9114, 9112)
src/ecourna/app/dados/asn/cconversormunicipiozona.h/.cpp                     (9131; 9130 for completeness)
src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.u40.cpp      (9129)
src/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.h/.cpp        (9222; 9221) - BU field
src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h/.cpp         (9225, 9224)
src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.u40.cpp (9060)
src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp              (9062, helper 9063)
src/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.u40.cpp (9065)
src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.u40.cpp        (9126, 9122 note)
src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.h/.cpp                    (9133, 9132)
src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.u40.cpp          (9144)
src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.u40.cpp         (9141, 9139 note)
src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.u40.cpp            (9173)
src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.u40.cpp                            (9212)
src/ecourna/app/dados/asn/federacoes/cconversorfederacao.u40.cpp                             (9216; 9214/5082/1671 notes)
src/ecourna/app/dados/asn/midias/cconversorautenticacao.u40.cpp                              (9201)
src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.h/.cpp                          (9203, 9202)
src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.u40.cpp         CEleitorIdentidade::ToString (1924)
src/uenux2/src/app/comum/dados/ccandidaturas.u40.cpp                         GetNumerosCandidatosAptos (2840) [attested]
src/uenux2/src/app/comum/dados/cpartidos.u40.cpp                             ~CPartidos (2815), statics 11502/11503
```

Directory evidence for the inferred paths: `ecourna/api/security/` (attested `cblockcipher.hpp`, `csha.cpp`,
`isymmetriccipher.cpp`), `ecourna/api/security/cepesc/` (attested `cplaintext.cpp`, `ccipheredout.cpp`,
`ccipheredin.cpp`, `cinfosalt.cpp`), `ecourna/api/util/` (`cstringutils.cpp`), `ecourna/app/dados/` (data classes of
shared types, e.g. `cregistroidentificacaoeleitor.cpp`), `ecourna/app/dados/asn/` and its sub-directories (attested
converters of the same ASN.1 modules). The `api/exception/` directory itself is not attested; it follows the
namespace `ecourna::api::exception`, as every other `api/<x>/` directory does.

---

## 1. Where this code sits in the voting process

`ecourna` is the TSE library shared by the urna applications. u40's part of it does four jobs:

1. **Errors.** Every exception of the TSE code is a `CError`. Its constructor builds the long `what()` text
   (function signature, line, code, message). When a thread dies, the "urna inoperante" screen shows the plain
   message and the code, not that text (§3.1).
2. **Randomness and ciphers.** The application-wide random generator (`IRng`), the factory of AES ciphers used for
   the RDV and for the key files, and the CEPESC hybrid cipher that protects result files (BU, attendance, images).
3. **ASN.1 converters** between the ecourna data classes and the III ASN.1 objects for the files the urna reads at
   start-up (schedules per municipality `-cfm.dat`, parametrização `-pu.dat`, federações `-fe.dat`, the medium
   description `infomidia`) and writes at the end (the attendance result `-jufa.dat` and one field of the BU).
4. **Small data classes** used by those converters (header, section id, attendance records).

In the web build, only start-up file reading (1, 3) and the RDV cipher construction run. The end-of-day code
(BU, attendance file) is linked but never reached by the page (docs/bu/codepath.md §1.3); it was exercised by the
separate BU harness (`tools/bu/operator_harness.mjs`), which confirms parts of this unit (§5, §10).

---

## 2. Classes and hierarchy (RTTI)

```
std::exception
 └─ ecourna::api::exception::CError                      vtable @1526468: [0] ~ 4636 [1] D0 1940 [2] what 11562
     └─ CBaseError<E, SErrorLimits{min,max}>             45 families (EIoError{1175,1275}, ESecurityError{1325,1725},
                                                          EDadosError{1935,2135}, vota::EUeVotaError, ...): same 3 slots
                                                          (42 vtables; CUePrinterError, CUeDesligandoError and
                                                          io::CIoError derive from three of them)
ecourna::api::security::IRng                              (no base)
 ├─ CTrng                                                 vtable @1114668: [2] 9477 [3] 9476 [4] 9475 [5] 5173
 └─ CPrng                                                 vtable @1113176: [2] 9502 [3] 9500 [4] 9499 [5] 5178
ecourna::api::pattern::NonCopyable
 ├─ ISymmetricCipherFactory
 │   └─ CSymmetricCipherFactory                           vtable @1113840: [2] Create(string) 1884 [3] Create(CAesKey) 9489
 └─ IAsymmetricCipher<CPlainText, CCipheredOut, CCipheredIn, std::vector<uebyte>>
     └─ ecourna::api::cepesc::CCepescCipher               vtable @1115004: [2] Encrypt 2681 [3] Decrypt 5171
ecourna::api::pattern::IObservableProgressWithDescription vtable @1111068: [1] D0 9528
ecourna::api::asn::IConversorASN<ENTIDADE, DADO>          [0] ~ (174) [1] D0 (144) [2] DoConverte [3] DoDeconverte
 ├─ CConversorRegistroIdentificacaoEleitor   <ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, CRegistroIdentificacaoEleitor>   9114 / 9112
 ├─ CConversorIdentificadorGeradorMidia      <ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>        9225 / 9224
 ├─ CConversorMunicipioZona                  <ModuloTiposEleitorais::MunicipioZona, CMunicipioZona>                            9131 / (9130)
 ├─ CConversorIdentificacaoSecaoEleitoral    <ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, ...>                          9129 / (9127)
 ├─ CConversorDetalhamentoComparecimento     <ModuloBoletimUrna::DetalhamentoComparecimento, CDetalhamentoComparecimento>       9222 / (9221)
 ├─ CConversorHorariosUrna                   <ModuloConfiguracaoMunicipios::HorariosUrna, CHorariosUrna>                       9133 / 9132
 ├─ CConversorConfiguracaoMunicipio(s)       <...ConfiguracaoMunicipio / EntidadeConfiguracaoMunicipios>                        9144, 9141 / (9142, 9137)
 ├─ CConversorParametrizacaoUrna             <ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna, CParametrizacaoUrna>       9173 / (9171)
 ├─ CConversorFederacao / CConversorFederacoes <ModuloFederacoes::Federacao / EntidadeFederacoes>                               (9217) 9216 / 9212 (9207)
 ├─ CConversorAutenticacao                   <ModuloInformacaoMidia::Autenticacao, CAutenticacao>                              9201 / (9200)
 ├─ CConversorDadosGeracaoMidia              <ModuloInformacaoMidia::DadosGeracaoMidia, CDadosGeracaoMidia>                    9203 / 9202
 ├─ CConversorComparecimentoSecao            <ModuloResultadoUrnaCadastro::ComparecimentoSecao, CComparecimentoSecao>          9126 / (9120)
 ├─ CConversorIdentificacaoJustificativa     <...::IdentificacaoJustificativa, CIdentificacaoJustificativa>                    9065 / (9064)
 ├─ CConversorDadosCifracao                  <...::DadosCifracao, CDadosCifracao>                                              9062 / (9061)
 └─ CConversorDadosComparecimentoCifrado     <...::DadosComparecimentoCifrado, CDadosComparecimentoCifrado>                    9060 / (9057)
```

Numbers in parentheses belong to other units. The unit.py listing named converter slots `vf2`/`vf3`; the
`IConversorASN` layout (u11) makes them `DoConverte`/`DoDeconverte`. Converters are stateless (4 bytes, vptr
only) and are always built on the caller's stack.

---

## 3. Exceptions: `CError` and `CBaseError<E, L>`

### 3.1 Layout and `what()`

`CError` (40 bytes, the size every throw site allocates):

| offset | member | set by the constructor (func 1143) |
|---|---|---|
| +0 | vptr | CError @1526468, then overwritten by the family vtable |
| +4 | `int m_codigo` | the error code |
| +8 | `std::string m_mensagem` | the message, **moved** from the by-value parameter |
| +20 | `std::source_location m_local` | pointer to the static `{file, function, line, column}` record of the throw site |
| +24 | ? | never written (padding or an unused member) |
| +28 | `std::string m_what` | first a copy of the message, then the long form below |

The long form is

```
m_what = function_name() + ":"s + to_string(line()) + ":"s + to_string(codigo) + " - "s + mensagem
```

inside a `try { } catch (...) { }`: every landing pad of the body goes to a catch-all that returns normally, so
a failure while formatting (only `std::bad_alloc` is possible) leaves `what()` = the plain message. The file name
of the location is not used. Real example captured by the BU harness
(`samples/bu-real/run-full/exceptions.json`):

```
static T &api::CPolySingleton<api::IGenericFactory<ecourna::api::security::IHash>>::instance(
  TPolySingletonsInfo &, const std::source_location &) [T = ...]:78:1301 - PolySingleton - solicitada uma instancia
  nao criada N3api15IGenericFactoryIN7ecourna3api8security5IHashEEE [/home/rubio/tse/uenux2/src/api/hash/cmontadorhash.cpp:49]
```

The screen does **not** show this text. `CThreadVota::TrataExcecao` (func 7710, u26 §4) builds the "urna
inoperante" page from the context title + `" ({})"` with `GetCodigo()` and from `m_mensagem` (+8) followed by the
context's actions. `what()` is only passed as the last argument of `CApplication::ShowExceptionMsg` (5568), whose
body in this build is just a `std::system_error` throw (u15 §2.4), so what it does with that text is not visible.
For a non-`CError` `std::exception`, 7709 even strips a TSE-style prefix with the regex
`.*?\).*?:\d+:-?\d+ - ((.|\n)*)` (@403127) before showing it. The developer signature therefore appears in logs
and harness output (as above), not on the voter's screen.

### 3.2 `CBaseError` and the three merged constructor bodies

`CBaseError<E, SErrorLimits{min, max}>` adds nothing to `CError`; the limits are part of the type name only (no
function compares the code with them). Its constructor is
`CBaseError(E codigo, std::string mensagem, std::source_location local = current()) : CError(int(codigo), std::move(mensagem), local) {}`.
wasm-opt's *merge-similar-functions* folded the 45 instantiations (RTTI count; three more RTTI entries are classes
derived from a `CBaseError<>`) into shared bodies that take the family vtable as an argument, each family keeping a
18-21 byte thunk:

| body | variant | families (thunk) |
|---|---|---|
| **1011** | CError ctor through `invoke` (slot 169) + landing pad that frees the moved message | ecourna: EDadosResultadoUrnaCadastro 9025, EDadosProcessoEleitoral 9026, EDadosMidias 9041, EDadosFederacoes 9046, EAsnResultadoUrnaCadastro 9107, EAsnParametrizacaoUrna 9158, **EAsnMidias 9181 (this unit)**, EAsn 9229, EDados 9266, EUtil 9410, ESql 9461, ESecurity 9505, ECompression 9584 |
| **710** | direct call (no landing pad) | uenux2 + one header family: vota::EUeVota 253, EUeComumGravadores 283, EUeUtil 346, EUeGui 406, EUeAssert 463, EUeComum 480, EUeComumRelatorios 580, EUeComumMd 591, EUeRdv 655, EApiAsn 1074, EUeHwil 1229, EUeComumAppInfo 1710, EUePersistencia 1715, EUeIpc 1889, EUeComumJustificativa 2260, ... |
| 2294 + **2379** | 2294 (rt:shared) calls a per-family function through a table slot and then stores the vptr; the five per-family functions were identical and ICF folded them into 2379 (slots 144/159/469/477/483) | EPatternError (331), EUePatternError (235), EUeComumDadosError (170), EUeComumAsnError (255), EUeIoError (210) |

The three copies differ only in exception handling of the call, which reflects how each translation unit was
compiled (see u11 on TUs built without `invoke`). The five families of the 2294/2379 variant are exactly the five
whose deleting destructor is 1940 rather than 454 (§12), which points to the translation units that define those
families rather than to the template.

---

## 4. Random generators: `IRng`, `CTrng`, `CPrng`

### 4.1 Interface

| slot | method (names inferred) | CTrng | CPrng |
|---|---|---|---|
| 2 | `uebyte GeraByte()` | 9477: `random_device("/dev/urandom")() & 0xFF` | 9502: first output of a fresh mt19937, **top** byte |
| 3 | `int Gera()` | 9476: uniform over [INT_MIN+1, INT_MAX-1] | 9500: same range, fresh mt19937 |
| 4 | `int Gera(vector<uebyte>& b)` | 9475: `Gera(b, b.size())` | 9499: same |
| 5 | `int Gera(vector<uebyte>& b, size_t n)` | 5173: 0 ok, 1 n==0, 2 b too small, **-1** on `std::exception` | 5178: 0 / 1 / 2 (no catch) |

The "uniform int" is the rejection loop `do r = rng(); while (r > 0xFFFFFFFD); return r - 0x7FFFFFFF`
(2^32 - 2 values starting at -2147483647). Callers take remainders of it (`CPoliticaExecucaoEleitor`: `% 4 + 3`
passes and `% 100 + 50` ms, signed remainders; `CControlaArmazenamentoDeImagens`: unsigned `% 999999`).

### 4.2 `CTrng`

Stateless. Every call builds a `std::random_device` from a static `std::string` "/dev/urandom" (@1911784, built
by `__wasm_call_ctors`, freed by 9478). This libc++ accepts no other token and draws with `getentropy` =
WASI `random_get` = `crypto.getRandomValues` in the browser. Its only user is `CBlockCipher<CAesCipher, CTrng>`
(u01). `CAesCipher::Encrypt` (9493) draws a 16-byte IV with `Gera(iv, 16)` **only when its `CAesKey` has no IV**.
The RDV key comes with a fixed IV (`CHKDFSeed` seed[32..48), u01 §3.1), so the recorded votes never drew from
`CTrng` (5173 was not observed executing). Only a cipher from `Create(segredo)` (empty IV) would draw. Every
caller of that factory in the binary reads (deciphers) a key file (§10, u01 §2.6), and the review found only
3-argument slot-3 (`Decrypt`) calls on those ciphers.

### 4.3 `CPrng`: deterministic by construction

`CPrng` holds only a 32-bit seed (+4). Every method builds a new **Boost.Random `mt19937`** on the stack (the
seeding code contains boost's `normalize_state()`: `x[0]` fixed from `x[396] ^ x[623]` with `0x321161BF`, then the
all-zero check), draws, and throws the engine away. Bytes come from `boost::random::uniform_int_distribution`
(output `>> 24`, not `& 0xFF` as std:: would give). `main` (func 10307) registers `CPrng(0)` as the application's
`IRng`. Consequences, **verified by calling the functions in the running simulator**:

| call (seed 0) | result, every time |
|---|---|
| `GeraByte()` | 140 |
| `Gera()` | 209652397 |
| `Gera(buf, 32)` | `8c97b7d89adb8bd86c9fa562704ce40ef645627acacf877a9164ecd6125616a5` |
| `Gera(buf, 0)` / `Gera(buf(32), 33)` | 1 / 2 |

A second quirk: `GeraByte` and `Gera(buf, n)` load the seed with `i32.load8_u` (only its low byte), `Gera()` with
`i32.load`. The probe confirmed it: seed 256 gives the same bytes as seed 0 (`140`, `8c97b7d8...`) but a different
`Gera()` (-1950843743). Users of the `IRng` singleton: `comum::CGravadorBU` (the 32-byte CEPESC "número aleatório
da UE" of the BU, cgravadorbu.cpp:485), `comum::CGravadorRCSecao` (32 bytes, cgravadorrcsecao.cpp:110),
`CControlaArmazenamentoDeImagens` (file names of fingerprint images), `CPoliticaExecucaoEleitor` (not used by the web
page, which registers `CPoliticaExecucaoEleitorWeb`).

The constant is visible in a real output: the attendance file written by the BU harness
(`samples/bu-real/run-full/analysis/jufa.decoded.txt`) has `DadosCifracao.salt` = `8c97b7d89adb8bd86c9fa562704ce40e`
and `informacaoAdicional` = `f645627acacf877a9164ecd6125616a5`, i.e. the two halves of `Gera(buf, 32)` above, and
`chave` = 32 zero bytes (the CEPESC stub, §5.2). Every simulator run produces the same salt.

---

## 5. Symmetric ciphers and CEPESC

### 5.1 `CSymmetricCipherFactory`

* **`Create(const CAesKey&)`** (9489, observed: builds the RDV cipher at `votaInit`, key/IV from `CHKDFSeed`, u02):
  `new CBlockCipher<CAesCipher, CTrng>(key, CBC)` (36 bytes, `ISymmetricCipher` ctor 9474 checks the key size),
  wrapped in a `std::shared_ptr` created from the raw pointer (not `make_shared`).
* **`Create(const std::string& segredo)`** (1884): key derivation for the key files.
  1. `SHA-512(segredo)` with `CSha512` (2684: OpenSSL digest "SHA2-512", 64 bytes);
  2. **AES-256 key = digest bytes 16..47** (the wasm copies 4 x 8 bytes from offsets 16, 24, 32, 40 of the digest);
  3. empty IV, key size 256, then the virtual `Create(CAesKey)`.
  u01's doc, `openssl.md` and u13's `csha.hpp` comment say "first 32 bytes" / `[0..32)`; that is wrong. The BU harness already relies on `[16:48]`
  (`tools/bu/bu_real_seeds.py` enciphers `jufa.pk1` with `SHA-512(32 zero bytes)[16:48]` and the real code
  deciphers it), which confirms the reading.

  Callers and what `segredo` is: `vota::CGeraBU` (key of the *código verificador* `cv.ber.pri`), `comum::CGravadorBU`
  (`bu.pk1`), `comum::CGravadorRCSecao` (`jufa.pk1`), `CControlaArmazenamentoDeImagens`,
  `CConversorBiometriaEleitorCifrada`. In all of them the secret comes from `api::IKernelHSM`, which has no
  implementation in the web build (u01 §5).

### 5.2 `CCepescCipher`: an identity "cipher" in this build

`Encrypt(const CPlainText&)` (2681) returns `CCipheredOut(32 zero bytes, plain.conteudo[, *plain.infoSalt])`; the
file type, crypto id, zone/section, the 1024-byte crypto table of the urna, the random number and the public key
are ignored. `Decrypt` (5171, u03) returns the content unchanged. So the "encrypted" BU envelope, attendance file
and fingerprint images of the simulator are plaintext with a key of 32 zero bytes (`Seguranca.idArquivoChave` of
the envelope, `DadosCifracao.chave` of the attendance file; the latter and its clear BER `conteudo` are visible in
`samples/bu-real/run-full/analysis/jufa.decoded.txt`).

---

## 6. DataHoraJE helpers

`DataHoraJE` (ModuloTiposEleitorais) is a GeneralString `"YYYYMMDDThhmmss"`.

* `FormataDataHoraJE(ptime)` (9220) = `std::format("{:04}{:02}{:02}T{:02}{:02}{:02}", ...)`, a thunk into the
  merged body 6155, which it shares with `api::FormataAAAAMMDDhhmmss` (thunk 2685). In that body the year goes through
  `std::stoi(std::to_string(year))`. Month and day are the plain 16-bit fields.
* `ConverteDataHoraJE(string)` (1877): if the text has >= 9 characters and ends in `'Z'` it is rebuilt as
  `s.substr(0,8) + "T" + s.substr(8,6)` (ASN.1 GeneralizedTime `YYYYMMDDhhmmssZ`; the zone is dropped, no UTC
  conversion), then `boost::date_time::parse_iso_time(s, 'T')`. Checked in the running module:

| input | result |
|---|---|
| `20240101T120000` | 212570913600000000 µs |
| `20240101120000Z` | the same |
| `20240101T120000Z` | throws `boost::wrapexcept<boost::bad_lexical_cast>` |
| `20241301T120000` | throws `boost::wrapexcept<boost::gregorian::bad_month>` |

Malformed dates in a data file therefore raise Boost exceptions, not `CError`.

---

## 7. ASN.1 converters and data classes

### 7.1 Pattern

Every converter implements `DoConverte` (data class -> ASN.1 object) and `DoDeconverte` (the reverse); the public
`Converte`/`Deconverte` of `IConversorASN` (u11) validate the ASN.1 side (`isValid() && isStrictlyValid()`, errors
1900/1902). None of u40's converter bodies validates anything itself: range checks happen in the `CBaseType`
constructors of the data side (CPatternError 1300, `cbasetype.hpp:39`) and in the III runtime. Composite converters
build the sub-converters on the stack and call their `Converte`; lists go through `ConverteLista` (u14, body 1970)
and are assigned with the runtime's copy-and-swap (`SEQUENCE_OF(first,last)` thunks 9122/9139 into body 2926).
Optional fields use the generated `includeOptionalField` (func 515) / `removeOptionalField` (shared_f432).

### 7.2 What each converter maps

| converter | ASN.1 type (module) | data class (layout) | notes |
|---|---|---|---|
| RegistroIdentificacaoEleitor 9114/9112 | `{identificacaoUtilizada, identificacaoPrincipal OPTIONAL}` (TiposEcoUrna) | `CRegistroIdentificacaoEleitor` (20 B) | principal present iff the optional is engaged; used for voters, mesários and justificativas of the attendance file |
| IdentificadorGeradorMidia 9225/9224 | `{nome, serialCertificadoTPM, serialInstalacao}` | 3 x std::string (36 B) | simulator data: "simulador-votacao-ng", 64 x '0'; 9224 observed through `CConversorDadoCorrespondencia` (11400, eg.bin) |
| MunicipioZona 9131 | `{municipio 1..99999, zona 1..9999}` | `CMunicipioZona` (8 B) | |
| IdentificacaoSecaoEleitoral 9129 | `{municipioZona, local, secao}` | `CIdentificacaoSecaoEleitoral` (16 B; ctor 5110) | |
| DetalhamentoComparecimento 9222 | BU `[1] {semBiometria, porBiometria, porBiografia}` | 3 x u16 | **BU**, §10 |
| HorariosUrna 9133/9132 | 4 x DataHoraJE | `CHorariosUrna` (4 x ptime) | observed at start-up (`-cfm.dat`) |
| ConfiguracaoMunicipio 9144 | `{codigoMunicipio, horariosUrna}` | 40 B POD | |
| ConfiguracaoMunicipios 9141 | `{cabecalho, SEQUENCE OF ConfiguracaoMunicipio}` | `CConfiguracaoMunicipios` (header + map) | map flattened in key order |
| ParametrizacaoUrna 9173 | `{cabecalho, parametros}` | `CParametrizacaoUrna` | writer side only; the urna reads `-pu.dat` |
| Federacao 9216 | `{identificador 100..999, sigla, nome, partidos SEQUENCE OF 0..99}` | `CFederacao` (40 B) | inlined `CBaseType<u16,100,999,39>` "FederacaoID" (BASIC_TYPE from the srcloc signature @1124484) |
| Federacoes 9212 | `{cabecalho, federacoes OPTIONAL}` | `CFederacoes` | empty list -> field absent |
| Autenticacao 9201 | `{[1] dataInicial OPT, [2] dataFinal OPT, tamanhoSenha, numeroTentativas, hashSenha}` | `CAutenticacao` (56 B) | dates written only if both present |
| DadosGeracaoMidia 9203/9202 | `{serialMidia, usuario, identificadorGeradorMidia, data}` | `CDadosGeracaoMidia` (72 B) | serial as hex text (9257) / `CSerialMidia` (9259) |
| ComparecimentoSecao 9126 | `{identificacao, SEQUENCE OF EstadoComparecimento}` | `CComparecimentoSecao` | one entry per voter of the roll |
| IdentificacaoJustificativa 9065 | `{identificacaoEleitor, anoNascimentoEleitor 0..9999}` | `CIdentificacaoJustificativa` (24 B) | |
| DadosCifracao 9062 | 3 x OCTET STRING (chave, salt, informacaoAdicional) | `CDadosCifracao` (36 B) | salt/info >= 16 bytes checked by the data class; each vector goes through the by-value helper 9063 |
| DadosComparecimentoCifrado 9060 | `{dadosCifracao, conteudo}` | `CDadosComparecimentoCifrado` (48 B; ctor 3484) | used when parameter `criptografarJUFA` is set |

### 7.3 Data-class constructors

`CCabecalhoEntidade(ptime, int id, ETipoId)` (5112; ETipoId = CHOICE index: processo eleitoral / pleito / eleição),
`CIdentificacaoSecaoEleitoral` (5110), `CHabilitacaoBiometrica` without (5087) and with (2657) a
`CEstadoHabilitacaoPorCodigo`, `CEstadoComparecimento(registro, situação)` (2658), `CResultadoUrnaCadastro` clear
(3483) and encrypted (3482), `CDadosComparecimentoCifrado` (3484). All are member-wise (copies or moves of by-value
parameters) without checks. Their layouts agree with the headers of u14.

### 7.4 The attendance result file (`-jufa.dat`) flow

At the end of voting `comum::CGravadorRCSecao` (func 11616, u23) builds a `CResultadoUrnaCadastro`:
header (5112), fase, voting version, situação (final/partial) and either `CDadosComparecimento` in clear (3483) or,
with `criptografarJUFA`, a `CDadosComparecimentoCifrado` (3484, 3482) made of the CEPESC output (2681) of the BER of
the attendance data plus `CDadosCifracao{chave, salt, info}`. The attendance records are built with 2658/5087/2657.
`CConversorResultadoUrnaCadastro` (u14) then calls, through `CConversorDadosComparecimento::DoConverte` (9102,
u14), this unit's converters: 9126 (section: 9129 + 9131, and `CConversorEstadoComparecimento` per voter), 9065
(justificativas), 9114 (every identification of voters and mesários), and 9060/9062 for the encrypted variant.
The 32 random bytes of the encrypted variant come from the deterministic `CPrng` (§4.3): they end up as the
`salt` and `informacaoAdicional` of `DadosCifracao`, constant in the simulator; the stub cipher (§5.2) ignores them.

---

## 8. `uenux2` code that the tools filed here

| func | real symbol | where it is used |
|---|---|---|
| 1924 | `comum::md::CEleitorIdentidade::ToString() const` | display of an identifier: título `"xxxx xxxx xxxx"` (spaces inserted at 8 then 4), CPF `"xxx.xxx.xxx-xx"` (`-` at 9, `.` at 6 and 3), free id as is, unknown type `""`. Operator texts (`TipoToStr` host 10586), voter-list report (11233), `CGeraRelatorios` (12105) |
| 2840 | `comum::CCandidaturas::GetNumerosCandidatosAptos(TCargoID) const` | numbers of the candidacies of a cargo whose situação word is 0, in map order. Observed: voting screens, the web page's `"candidates"` JSON (u29), BU QR codes (5604) and BU candidate printing (11214) |
| 2815 | `comum::CPartidos::~CPartidos()` | party table singleton (map + name "CPartidos") |
| 5615 | `comum::CCalculaCV::Reinicia(const std::string&)` | BU código verificador, already reconstructed in `ccalculacv.cpp` (u25) |
| 3692 | `comum::md::CNomesCargo::operator=` (implicit, 4 strings) | inside `CCargo::operator=` (unknown_f5594) |
| 5593 | copy loop of `md::CRespostaConsulta` (28-byte items) | same |
| 11502 / 11503 | atexit: `CPartidos::GetInst` static mutex / `unique_ptr<CPartidos>` | cpartidos.cpp |
| 11506 | atexit: `CLocal::GetInst` static mutex (@1838876) | clocal.cpp |
| 11509 / 11510 | atexit: `CHV::s_mutex` / `CHV` instance pointer (@1838848 / @1838872) | chv.cpp |
| 11512 | atexit: `CFotos::GetInst` static mutex (@1838820) | cfotos.cpp |

---

## 9. Library code in this unit

* **libc++**: `string::assign(const char*, n)` 986, `vector<string>::__push_back_slow_path` 1108,
  `vector<T>::__construct_at_end` merged body 2898, `locale::operator==` 3340, `__throw_bad_any_cast` 3861,
  filesystem `ErrorHandler<void>::report` 4792, `vector<uint16_t>::push_back` 9214, `vector<uebyte>::insert` 9492,
  the `CRespostaConsulta` copy loop 5593.
* **III ASN.1 runtime** (inline/template code instantiated in ecourna TUs): `AbstractString` copy ctor 1671,
  `INTEGER` copy ctor 5082, `SEQUENCE_OF(first,last)` thunks 9122/9139.
* **ecourna helper 9063**: builds an `ASN1::OCTET_STRING` from a `std::vector<uebyte>` and returns it by value. It is
  **not** a constructor: its type is `(i32,i32)->void` and both callers (9060, 9062) reach it through `invoke_vii`.
  Every constructor in this build returns `this` and would be invoked as `invoke_iii`. It is reconstructed as
  `ConverteOctetString` (name inferred) in `cconversordadoscifracao.u40.cpp`.
* **Boost**: `mt19937::twist` 5179; the `std::map` range insert of `signals2::detail::grouped_list` 9551
  (group-key comparator: meta group, then the optional int only for `grouped_slots`).
* **7-Zip** (linked for `CLzmaCompress`, which is dead code, u12): `CBuffer<Byte>::Alloc` 2614,
  `CArcErrorInfo::operator=` 3437, `SetProperties` 4993.
* **SQLite**: the shared body of `sqlite3_column_text`/`_text16` 6077.
* **wasm-opt merged enum helpers** of the converters (the source functions are reconstructed by u14):
  6144 (ASN `v` -> `v-1`, range 3, throws `CAsnResultadoUrnaCadastroError`), 6148 (0-based -> `v+1`, also range 3, message
  copied from a static string, `CAsnParametrizacaoUrnaError`), 6151 (identity with range `[1,k]` into an ENUMERATED).
* **Static destructors** (atexit): 8378 (CLzmaCompress mutex), 9431 (CSqlStatement mutex), 9478 (CTrng token),
  9496 (CAesCipher `s_cipherTable`), 9150/9151 (titulo-relatório messages), 9170 (label message), 9166
  (`"Forma de validar título inválido."`, a static no code uses).

---

## 10. Boletim de Urna (BU): what this unit contributes

The BU is generated by `vota::CGeraBU` / `comum::CGeradorBU*`, written by `comum::CGravadorBU` and converted by
`comum::asn::CConversorEntidadeBU` (units u06-u09, u21, u23; docs/10-boletim-de-urna.md, docs/bu/codepath.md).
u40 supplies these pieces, in the order the encerramento uses them:

1. **The CV key file** (`vota::CGeraBU::StartState`, 12110). `/dsk/fi/estatico/chave/cv.ber.pri` is an
   `EntidadeChave`; if its key is enciphered, the cipher is `CSymmetricCipherFactory::Create(segredo)` (1884) with
   `segredo` from `api::IKernelHSM`: **AES-256-CBC, key = SHA-512(segredo)[16..48)**, IV = last 16 bytes of the
   input. The first 16 bytes of the resulting key are the SipHash key of `comum::CCalculaCV`.
2. **The código verificador chain.** Each time a new CV is started, `CCalculaCV::Reinicia` (5615) resets the MAC
   input to `tipo ('F' or 'A') + identificação + cv` and remembers `cv` (docs/bu/codigo-verificador.md).
3. **Candidate lists.** `CCandidaturas::GetNumerosCandidatosAptos` (2840) gives, per cargo, the apt candidate
   numbers used by the BU QR code generator (`CGeradorBUQRCode`, 5604: fields `CARG`/`TIPO`/`VERC` ...) and by the
   zerésima's candidate part (11214). The QR payload hash chain uses `CSha512` (2684).
4. **`detalhamentoComparecimento`.** `vota::CGravaResultado` (12098) builds
   `CDetalhamentoComparecimento(semBiometria, porBiometria, porBiografia)` (ctor 5094) from three `CEleitores`
   counters. `CGravadorBU::MontaEntidadeBU` keeps it only for a biometric urna (u23 §3.4);
   `CConversorEntidadeBU` then sets `EntidadeBoletimUrna.[1] detalhamentoComparecimento` with
   `CConversorDetalhamentoComparecimento::Converte` (9222 + the validity check of `IConversorASN`), or omits the
   field. Exact ASN.1: `DetalhamentoComparecimento ::= SEQUENCE { qtdEleitoresCompareceramSemBiometria INTEGER
   (0..9999), qtdEleitoresHabilitadosPorBiometria INTEGER (0..9999), qtdEleitoresHabilitadosPorBiografia INTEGER
   (0..9999) }`. The BU harness produced `{1, 0, 0}` for its single voter (docs/bu/codepath.md §4).
5. **The file envelope** (`CGravadorBU::GravaResultado`, 11629). The BU public key `bu.pk1` is deciphered with the
   same 1884 cipher; then `CPlainText(tipoArquivo 0, idCriptografia 1, zona, seção, tabela[1024], número aleatório,
   chave pública, BER(EntidadeBoletimUrna))` goes through `CCepescCipher::Encrypt` (2681) and the result becomes
   `Seguranca{idTipoArquivo 0, idCriptografia 1, idArquivoChave = out.chave}` + `conteudo` of the
   `EntidadeEnvelopeGenerico` written as `…-bu.dat`. In this build: `idArquivoChave` = 32 zero bytes, `conteudo` =
   the unencrypted BU, and the "número aleatório" (32 bytes from `IRng::Gera(buf)`) is always
   `8c97b7d8…16a5` (§4.3), ignored anyway by the stub.
6. **Errors.** Any failure of this path is a `CBaseError<…>` whose `what()` has the §3.1 form.

The attendance file (`-jufa.dat`) written next to the BU by `CGravadorRCSecao` is described in §7.4.

---

## 11. What is specific to the web build

* `main` registers `CPrng(0)` as the only `IRng`: every "random" value of the application is a constant (§4.3).
* `CCepescCipher` does not encrypt (§5.2); `api::IKernelHSM` has no implementation, so `Create(segredo)` (1884) is
  never reached by the page (the BU harness supplies a fake HSM returning 32 zero bytes).
* `CTrng` would draw from `crypto.getRandomValues` (WASI `random_get`), but the page never reaches it: the RDV
  cipher's IV is fixed by `CHKDFSeed` (u01 §3.1), so the RDV file is byte-identical from run to run.
* The end-of-day converters (attendance, BU detail) are linked but only reachable through the BU harness.
* Nothing in this unit reads URL parameters, touches the network or the DOM.

---

## 12. Notable wasm / Emscripten observations

* **merge-similar-functions everywhere.** Three bodies for the 45 exception constructors (§3.2); three for the enum
  helpers of the converters (6144/6148/6151, which receive the message, source location, code, constructor slot
  and typeinfo as parameters); one for `vector::__construct_at_end` (2898, element copier passed as a table slot);
  one for the two SQLite column-text functions (6077); `FormataDataHoraJE` shares the body of an `api` formatter.
* **ICF.** Five per-family "CError part" functions folded into 2379 (five table slots). The deleting destructor of
  the error families exists twice. 1940 calls `~CError` (4636) and serves CError plus the 5 families of the
  2294/2379 variant (EPatternError, EUePatternError, EUeComumDadosError, EUeComumAsnError, EUeIoError).
  `icf_shared_vf1@454` has the two string destructors inlined and serves 39 vtables: 37 `CBaseError<>` families plus
  the derived `io::CIoError` and `api::CUeDesligandoError`. `api::CUePrinterError` has its own (4910). These are
  different inlining decisions in different TUs, each then ICF-folded.
* **Constructors return `this`** (wasm C++ ABI), which is why 5110/5112/2657/2658/3482-3484 end with `return a`.
* **By-value `std::string`/`std::optional`/vectors are moved** with an i64 + i32 copy of the 12-byte object followed
  by zeroing the source (`c[0]:long@4 = 0; c[2] = 0`), e.g. in 1143, 3482-3484.
* **Boost vs std** can be told apart in the inlined RNG code: boost's `normalize_state` constant `0x321161BF` and
  the `>> 24` byte extraction identify Boost.Random in `CPrng`; `CTrng` uses libc++'s distribution and random device.
* **Static objects** (`std::string` messages, the `/dev/urandom` token, `std::map`s, singleton `unique_ptr`s and the
  pthread-stubbed `std::mutex`es) each get a tiny atexit function (9478, 9496, 9150/9151/9166/9170, 8378, 9431,
  11502-11512); a mutex destructor is only the ICF'd noexcept residue (func 150) applied to its address.
* **An unused static.** 9166 destroys `"Forma de validar título inválido."` (@1912244): built at start-up, read by
  nothing - the message of a converter that is not linked.

---

## 13. Suspicious or noteworthy code

| # | func | what | impact | severity |
|---|---|---|---|---|
| 1 | 9502/9500/9499/5178 (+ main 10307) | `CPrng` re-seeds a fresh mt19937 with the same seed on every call, and the web `main` seeds it with 0: `GeraByte()` = 140, `Gera()` = 209652397, `Gera(buf,32)` = `8c97b7d8…` forever (verified at run time) | simulator: the CEPESC "número aleatório da UE" of the BU, the `salt`/`informacaoAdicional` written into every attendance file (seen in `samples/bu-real/run-full`), and the ids of stored fingerprint images (`Gera() % 999999` = 652606) are constants; `GerarCaminhosUnicos` retries `while (file exists)` with that constant, so a second WSQ image in the same directory would hang the (single-threaded) page (u24 #1; latent, the web page stores no image). On a real urna, even with a random seed, this class would return one value per instance | medium |
| 2 | 9502, 5178 | the byte-producing methods seed with the **low 8 bits** of the seed (`i32.load8_u`), `Gera()` with all 32 bits (verified: seed 256 == seed 0 for bytes) | at most 256 distinct byte streams whatever the seed | low |
| 3 | 2681 | `CCepescCipher::Encrypt` returns the plaintext with a 32-byte zero "session key"; table, random number and public key ignored | simulator: every "ciphered" result file is plaintext (security simulated); if the same stub were linked in an urna, BU/JUFA/biometric data would not be protected | medium |
| 4 | 5173 (+ CAesCipher 9493) | `CTrng::Gera(buf,n)` catches `std::exception` and returns -1; `CAesCipher::Encrypt` ignores the status | only an encryption whose key has **no** IV draws from `CTrng`. If the entropy source failed, that IV would silently be the 16 zero bytes 9493 pre-fills instead of an error. The RDV is not affected (its IV is fixed by `CHKDFSeed`), and the IV-less ciphers of this binary are only used to decipher key files, so the flaw is latent | low |
| 5 | 1884 | key = plain SHA-512 of the HSM secret, bytes 16..47 (no salt/KDF); earlier project docs state bytes 0..31 | documentation error corrected here (confirmed by the BU harness); the derivation itself is only as strong as the HSM secret | info |
| 6 | 1877 | `ConverteDataHoraJE`: a trailing `Z` means "GeneralizedTime without T": `YYYYMMDDThhmmssZ` is mangled and rejected; the UTC marker is dropped without conversion; parse errors are Boost exceptions (`bad_lexical_cast`, `bad_month`), not `CError` (verified) | a malformed date in `-cfm.dat`/infomidia/attendance data escapes the `CError` handlers and ends in the generic "unknown exception" path of the thread | low |
| 7 | 1143 | `what()` embeds the full C++ signature and line of the throwing function; the formatting is wrapped in `catch (...) {}` | the text reaches logs and `ShowExceptionMsg`, but the "urna inoperante" screen uses `GetMensagem()` + code (7710), and 7709 strips the prefix with a regex, so it is **not** shown to voters. Only the silent fallback to the plain message on `bad_alloc` remains | info |
| 8 | 9201 (and 9200, u11) | Autenticacao validity window written/read only when BOTH dates exist; a single date is silently dropped | a medium with only `dataFinal` would lose its expiry on round trip | low |
| 9 | 9166 | static message "Forma de validar título inválido." with no user | dead code hint (converter removed or not linked) | info |
| 10 | 9585 | `CLzmaCompress::OnProgress` truncates the 64-bit progress to 32 bits; the class is never constructed | dead code | info |

---

## 14. Complete mapping table (all 101 functions of u40)

`run` = observed executing during the recorded votes. "library" rows are libc++/Boost/7-Zip/SQLite/ASN.1-runtime
code, wasm-opt merged bodies of functions reconstructed elsewhere, or compiler-generated static destructors; they are
described in comments of the listed file or in §8-§9, and no source was written for them. *(path inferred)* = no
`std::source_location` for the file.

| func | size | run | reconstructed symbol | original file | src | conf. |
|---:|---:|:-:|---|---|---|---|
| 710 | 114 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (merged body, no landing pad)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (path inferred) | src/ecourna/api/exception/cbaseerror.hpp | high |
| 986 | 164 | ✔ | `std::string::assign(const char*, size_t) (__assign_external)` | libcxx/include/string | library: rt:libcxx (no file written) | high |
| 1011 | 181 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (merged body, EH variant)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (path inferred) | src/ecourna/api/exception/cbaseerror.hpp | high |
| 1108 | 268 | ✔ | `std::vector<std::string>::__push_back_slow_path(const std::string&)` | libcxx/include/vector | library: rt:libcxx (no file written) | high |
| 1143 | 1778 |  | `ecourna::api::exception::CError::CError` | ecourna-lib/ecourna/api/exception/cerror.cpp (path inferred) | src/ecourna/api/exception/cerror.cpp | high |
| 1671 | 44 |  | `ASN1::AbstractString::AbstractString(const AbstractString&)` | III ASN.1 runtime asn1.h (inline) | library: lib:asn1-runtime (no file written) | high |
| 1877 | 897 | ✔ | `ecourna::api::util::ConverteDataHoraJE` | ecourna-lib/ecourna/api/util/datahora.cpp (path inferred) | src/ecourna/api/util/datahora.cpp | medium |
| 1884 | 1167 |  | `ecourna::api::security::CSymmetricCipherFactory::Create(const std::string&)` | ecourna-lib/ecourna/api/security/csymmetriccipherfactory.cpp (path inferred) | src/ecourna/api/security/csymmetriccipherfactory.cpp | high |
| 1924 | 334 |  | `comum::md::CEleitorIdentidade::ToString` | uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (path inferred) | src/uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.u40.cpp | medium |
| 1940 | 10 |  | `ecourna::api::exception::CError::~CError (deleting)` | ecourna-lib/ecourna/api/exception/cerror.cpp (path inferred) | src/ecourna/api/exception/cerror.cpp | high |
| 2379 | 123 |  | `ecourna::api::exception::CBaseError<E,L>::CBaseError (CError part, 5 instantiations ICF)` | ecourna-lib/ecourna/api/exception/cbaseerror.hpp (path inferred) | src/ecourna/api/exception/cbaseerror.hpp | medium |
| 2614 | 72 |  | `CBuffer<Byte>::Alloc (7-Zip CByteBuffer)` | CPP/Common/MyBuffer.h | library: lib:7zip (no file written) | high |
| 2657 | 197 |  | `ecourna::app::dados::CHabilitacaoBiometrica::CHabilitacaoBiometrica (with CEstadoHabilitacaoPorCodigo)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp | high |
| 2658 | 167 |  | `ecourna::app::dados::CEstadoComparecimento::CEstadoComparecimento(const CRegistroIdentificacaoEleitor&, ESituacaoComparecimento)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp | src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.u40.cpp | high |
| 2681 | 345 |  | `ecourna::api::cepesc::CCepescCipher::Encrypt` | ecourna-lib/ecourna/api/security/cepesc/ccepesccipher.cpp (path inferred) | src/ecourna/api/security/cepesc/ccepesccipher.cpp | high |
| 2684 | 276 | ✔ | `ecourna::api::security::CSha512::CSha512` | ecourna-lib/ecourna/api/security/csha.hpp (path inferred) | src/ecourna/api/security/security.u40.cpp | high |
| 2815 | 37 |  | `comum::CPartidos::~CPartidos` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | medium |
| 2840 | 150 | ✔ | `comum::CCandidaturas::GetNumerosCandidatosAptos(TCargoID)` | uenux2/src/app/comum/dados/ccandidaturas.cpp | src/uenux2/src/app/comum/dados/ccandidaturas.u40.cpp | medium |
| 2898 | 77 | ✔ | `std::vector<T>::__construct_at_end(first, last, n) (merged body)` | libcxx/include/vector | library: rt:libcxx (no file written) | high |
| 3340 | 63 | ✔ | `std::locale::operator==` | libcxx/src/locale.cpp | library: rt:libcxx (no file written) | high |
| 3437 | 58 |  | `CArcErrorInfo::operator=(const CArcErrorInfo&) (7-Zip, implicit)` | CPP/7zip/UI/Common/OpenArchive.h | library: lib:7zip (no file written) | medium |
| 3482 | 365 |  | `ecourna::app::dados::CResultadoUrnaCadastro::CResultadoUrnaCadastro (encrypted data)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp | high |
| 3483 | 445 |  | `ecourna::app::dados::CResultadoUrnaCadastro::CResultadoUrnaCadastro (clear data)` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.cpp | src/ecourna/app/dados/resultadournacadastro/cresultadournacadastro.u40.cpp | high |
| 3484 | 236 |  | `ecourna::app::dados::CDadosComparecimentoCifrado::CDadosComparecimentoCifrado` | ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp (path inferred) | src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimentocifrado.cpp | high |
| 3517 | 60 |  | `ecourna::api::security::CAesKey::~CAesKey (implicit)` | ecourna-lib/ecourna/api/security/isymmetriccipher.hpp (path inferred) | src/ecourna/api/security/security.u40.cpp | high |
| 3692 | 449 |  | `comum::md::CNomesCargo::operator=(const CNomesCargo&) (implicit)` | uenux2/src/app/comum/dados/md/processoeleitoral/cnomescargo.h (path inferred) | library: app:comum (no file written) | medium |
| 3861 | 44 |  | `std::__throw_bad_any_cast` | libcxx/include/any | library: rt:libcxx (no file written) | high |
| 4636 | 32 |  | `ecourna::api::exception::CError::~CError` | ecourna-lib/ecourna/api/exception/cerror.cpp (path inferred) | src/ecourna/api/exception/cerror.cpp | high |
| 4792 | 181 |  | `std::filesystem::detail::ErrorHandler<void>::report(const error_code&, const char*, ...)` | libcxx/src/filesystem/error.h | library: rt:libcxx (no file written) | medium |
| 4993 | 1359 |  | `SetProperties(IUnknown*, const CObjectVector<CProperty>&) (7-Zip)` | CPP/7zip/UI/Common/SetProperties.cpp | library: lib:7zip (no file written) | medium |
| 5082 | 40 |  | `ASN1::INTEGER::INTEGER(const INTEGER&)` | III ASN.1 runtime asn1.h (inline) | library: lib:asn1-runtime (no file written) | high |
| 5087 | 46 |  | `ecourna::app::dados::CHabilitacaoBiometrica::CHabilitacaoBiometrica` | ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp | src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.u40.cpp | high |
| 5110 | 34 |  | `ecourna::app::dados::CIdentificacaoSecaoEleitoral::CIdentificacaoSecaoEleitoral` | ecourna-lib/ecourna/app/dados/cidentificacaosecaoeleitoral.cpp (path inferred) | src/ecourna/app/dados/cidentificacaosecaoeleitoral.cpp | high |
| 5112 | 34 |  | `ecourna::app::dados::CCabecalhoEntidade::CCabecalhoEntidade` | ecourna-lib/ecourna/app/dados/ccabecalhoentidade.cpp (path inferred) | src/ecourna/app/dados/ccabecalhoentidade.cpp | high |
| 5170 | 84 |  | `std::unique_ptr<ecourna::api::cepesc::CInfoSalt>::reset` | ecourna-lib/ecourna/api/security/cepesc/cinfosalt.hpp (path inferred) | src/ecourna/api/security/security.u40.cpp | medium |
| 5173 | 233 |  | `ecourna::api::security::CTrng::Gera(std::vector<uebyte>&, std::size_t)` | ecourna-lib/ecourna/api/security/ctrng.cpp (path inferred) | src/ecourna/api/security/ctrng.cpp | high |
| 5178 | 1059 |  | `ecourna::api::security::CPrng::Gera(std::vector<uebyte>&, std::size_t)` | ecourna-lib/ecourna/api/security/cprng.cpp (path inferred) | src/ecourna/api/security/cprng.cpp | high |
| 5179 | 680 |  | `boost::random::mersenne_twister_engine<uint32_t,32,624,397,31,...>::twist` | boost/random/mersenne_twister.hpp | library: lib:boost (no file written) | high |
| 5593 | 293 |  | `std::__copy_loop for comum::md::CRespostaConsulta (copy-assign range)` | libcxx/include/__algorithm/copy.h | library: rt:libcxx (no file written) | medium |
| 5615 | 249 |  | `comum::CCalculaCV::Reinicia` | uenux2/src/app/comum/relatorios/ccalculacv.cpp | src/uenux2/src/app/comum/relatorios/ccalculacv.cpp (unit u25) | high |
| 6077 | 246 |  | `sqlite3_column_text/_text16 shared body (columnMem + sqlite3ValueText + columnMallocFailure)` | sqlite3.c | library: lib:sqlite (no file written) | high |
| 6144 | 258 |  | `enum Deconverte helper (ASN v -> v-1, range 3) merged body` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/ (wasm-opt merge-similar) | library: lib:ecourna (no file written) | high |
| 6148 | 295 |  | `enum Converte helper (0-based -> ASN v+1, message from static string) merged body` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/ (wasm-opt merge-similar) | library: lib:ecourna (no file written) | high |
| 6151 | 268 |  | `enum Converte helper (identity, range [1,k]) merged body` | ecourna-lib/ecourna/app/dados/asn/(wasm-opt merge-similar) | library: lib:ecourna (no file written) | high |
| 8378 | 10 |  | `atexit: ~std::mutex of CLzmaCompress s_mutex (@1923244)` | ecourna-lib/ecourna/api/compression/clzmacompress.cpp | src/ecourna/api/compression/clzmacompress.u40.cpp | high |
| 9060 | 394 |  | `ecourna::app::dados::asn::CConversorDadosComparecimentoCifrado::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.cpp (path inferred) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscomparecimentocifrado.u40.cpp | high |
| 9062 | 634 |  | `ecourna::app::dados::asn::CConversorDadosCifracao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.cpp (path inferred) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp | high |
| 9063 | 141 |  | `ecourna::app::dados::asn::ConverteOctetString(const std::vector<uebyte>&)` (by-value helper returning `ASN1::OCTET_STRING`; not a constructor: `(i32,i32)->void`, called via `invoke_vii`) | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/ (path inferred) | src/ecourna/app/dados/asn/resultadournacadastro/cconversordadoscifracao.u40.cpp | low |
| 9065 | 268 |  | `ecourna::app::dados::asn::CConversorIdentificacaoJustificativa::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.cpp (path inferred) | src/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.u40.cpp | high |
| 9112 | 613 |  | `ecourna::app::dados::asn::CConversorRegistroIdentificacaoEleitor::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp (path inferred) | src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp | high |
| 9114 | 843 |  | `ecourna::app::dados::asn::CConversorRegistroIdentificacaoEleitor::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp (path inferred) | src/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.cpp | high |
| 9122 | 26 |  | `ASN1::SEQUENCE_OF<ModuloResultadoUrnaCadastro::EstadoComparecimento>::SEQUENCE_OF(first, last)` | III ASN.1 runtime (template instantiation) | library: lib:asn1-runtime (no file written) | high |
| 9126 | 592 |  | `ecourna::app::dados::asn::CConversorComparecimentoSecao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.cpp (path inferred) | src/ecourna/app/dados/asn/resultadournacadastro/cconversorcomparecimentosecao.u40.cpp | high |
| 9129 | 251 |  | `ecourna::app::dados::asn::CConversorIdentificacaoSecaoEleitoral::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp (path inferred) | src/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.u40.cpp | high |
| 9131 | 54 |  | `ecourna::app::dados::asn::CConversorMunicipioZona::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversormunicipiozona.cpp (path inferred) | src/ecourna/app/dados/asn/cconversormunicipiozona.cpp | high |
| 9132 | 134 | ✔ | `ecourna::app::dados::asn::CConversorHorariosUrna::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp (path inferred) | src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp | high |
| 9133 | 1852 |  | `ecourna::app::dados::asn::CConversorHorariosUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp (path inferred) | src/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp | high |
| 9139 | 26 |  | `ASN1::SEQUENCE_OF<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio>::SEQUENCE_OF(first, last)` | III ASN.1 runtime (template instantiation) | library: lib:asn1-runtime (no file written) | high |
| 9141 | 1272 |  | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipios::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp (path inferred) | src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.u40.cpp | high |
| 9144 | 297 |  | `ecourna::app::dados::asn::CConversorConfiguracaoMunicipio::DoConverte` | ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp (path inferred) | src/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.u40.cpp | high |
| 9150 | 36 |  | `atexit: ~std::string MSG_ALINHAMENTO_INVALIDO (@1912268)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp (u14, static declared there) | high |
| 9151 | 36 |  | `atexit: ~std::string MSG_ESTILO_INVALIDO (@1912256)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversortitulorelatorio.cpp (u14, static declared there) | high |
| 9166 | 36 |  | `atexit: ~std::string "Forma de validar título inválido." (@1912244, unused static)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrosurna.cpp | library: lib:ecourna (no file written) | medium |
| 9170 | 36 |  | `atexit: ~std::string MSG_GENERO_INVALIDO (@1912232)` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp | src/ecourna/app/dados/asn/parametrizacaourna/cconversorlabelparametrizado.cpp (u14, static declared there) | high |
| 9173 | 349 |  | `ecourna::app::dados::asn::CConversorParametrizacaoUrna::DoConverte` | ecourna-lib/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.cpp (path inferred) | src/ecourna/app/dados/asn/parametrizacaourna/cconversorparametrizacaourna.u40.cpp | high |
| 9181 | 18 |  | `ecourna::api::exception::CBaseError<ecourna::app::dados::asn::EAsnMidiasError, SErrorLimits{2485,2510}>::CBaseError` | ecourna-lib/ecourna/app/dados/dadoserros.h (path inferred) | src/ecourna/api/exception/cbaseerror.hpp (thunk list) | high |
| 9201 | 1371 |  | `ecourna::app::dados::asn::CConversorAutenticacao::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversorautenticacao.cpp (path inferred) | src/ecourna/app/dados/asn/midias/cconversorautenticacao.u40.cpp | high |
| 9202 | 621 |  | `ecourna::app::dados::asn::CConversorDadosGeracaoMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp (path inferred) | src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp | high |
| 9203 | 1234 |  | `ecourna::app::dados::asn::CConversorDadosGeracaoMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp (path inferred) | src/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.cpp | high |
| 9212 | 492 |  | `ecourna::app::dados::asn::CConversorFederacoes::DoConverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacoes.cpp (path inferred) | src/ecourna/app/dados/asn/federacoes/cconversorfederacoes.u40.cpp | high |
| 9214 | 252 |  | `std::vector<unsigned short>::push_back(const unsigned short&)` | libcxx/include/vector | library: rt:libcxx (no file written) | high |
| 9216 | 1629 |  | `ecourna::app::dados::asn::CConversorFederacao::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/federacoes/cconversorfederacao.cpp (path inferred) | src/ecourna/app/dados/asn/federacoes/cconversorfederacao.u40.cpp | high |
| 9220 | 17 |  | `ecourna::api::util::FormataDataHoraJE` | ecourna-lib/ecourna/api/util/datahora.cpp (path inferred) | src/ecourna/api/util/datahora.cpp | medium |
| 9222 | 307 |  | `ecourna::app::dados::asn::CConversorDetalhamentoComparecimento::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.cpp (path inferred) | src/ecourna/app/dados/asn/cconversordetalhamentocomparecimento.cpp | high |
| 9224 | 423 | ✔ | `ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia::DoDeconverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp (path inferred) | src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp | high |
| 9225 | 666 |  | `ecourna::app::dados::asn::CConversorIdentificadorGeradorMidia::DoConverte` | ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp (path inferred) | src/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.cpp | high |
| 9398 | 203 |  | `ecourna::api::util::CStringUtils::Replace(std::string&, char, const std::string&)` | ecourna-lib/ecourna/api/util/cstringutils.cpp | src/ecourna/api/util/cstringutils.u40.cpp | medium |
| 9431 | 10 |  | `atexit: ~std::mutex of CSqlStatement (@1911796)` | ecourna-lib/ecourna/api/sql/sqlite/csqlstatement.cpp | src/ecourna/api/sql/sqlite/csqlstatement.cpp (u13 comment) | high |
| 9470 | 491 |  | `ecourna::api::cepesc::CInfoSalt::CInfoSalt(const CInfoSalt&) (implicit)` | ecourna-lib/ecourna/api/security/cepesc/cinfosalt.cpp | src/ecourna/api/security/security.u40.cpp | high |
| 9475 | 20 |  | `ecourna::api::security::CTrng::Gera(std::vector<uebyte>&)` | ecourna-lib/ecourna/api/security/ctrng.cpp (path inferred) | src/ecourna/api/security/ctrng.cpp | high |
| 9476 | 105 |  | `ecourna::api::security::CTrng::Gera` | ecourna-lib/ecourna/api/security/ctrng.cpp (path inferred) | src/ecourna/api/security/ctrng.cpp | high |
| 9477 | 90 |  | `ecourna::api::security::CTrng::GeraByte` | ecourna-lib/ecourna/api/security/ctrng.cpp (path inferred) | src/ecourna/api/security/ctrng.cpp | high |
| 9478 | 36 |  | `atexit: ~std::string TOKEN "/dev/urandom" of CTrng (@1911784)` | ecourna-lib/ecourna/api/security/ctrng.cpp (path inferred) | src/ecourna/api/security/ctrng.cpp | high |
| 9488 | 75 |  | `ecourna::api::security::CBlockCipher<CAesCipher, CTrng>::~CBlockCipher (deleting)` | ecourna-lib/ecourna/api/security/cblockcipher.hpp | src/ecourna/api/security/security.u40.cpp | high |
| 9489 | 282 | ✔ | `ecourna::api::security::CSymmetricCipherFactory::Create(const CAesKey&)` | ecourna-lib/ecourna/api/security/csymmetriccipherfactory.cpp (path inferred) | src/ecourna/api/security/csymmetriccipherfactory.cpp | high |
| 9492 | 1177 |  | `std::vector<uebyte>::insert(const_iterator, first, last)` | libcxx/include/vector | library: rt:libcxx (no file written) | high |
| 9496 | 18 |  | `atexit: ~std::map s_cipherTable of CAesCipher (@1911772)` | ecourna-lib/ecourna/api/security/caescipher.cpp | src/ecourna/api/security/security.u40.cpp | high |
| 9499 | 20 |  | `ecourna::api::security::CPrng::Gera(std::vector<uebyte>&)` | ecourna-lib/ecourna/api/security/cprng.cpp (path inferred) | src/ecourna/api/security/cprng.cpp | high |
| 9500 | 370 |  | `ecourna::api::security::CPrng::Gera` | ecourna-lib/ecourna/api/security/cprng.cpp (path inferred) | src/ecourna/api/security/cprng.cpp | high |
| 9502 | 332 |  | `ecourna::api::security::CPrng::GeraByte` | ecourna-lib/ecourna/api/security/cprng.cpp (path inferred) | src/ecourna/api/security/cprng.cpp | high |
| 9503 | 21 |  | `ecourna::api::security::CPrng::CPrng` | ecourna-lib/ecourna/api/security/cprng.cpp (path inferred) | src/ecourna/api/security/cprng.cpp | high |
| 9528 | 185 |  | `ecourna::api::pattern::IObservableProgressWithDescription::~IObservableProgressWithDescription (deleting)` | ecourna-lib/ecourna/api/pattern/iobservableprogresswithdescription.hpp (path inferred) | src/ecourna/api/pattern/iobservableprogresswithdescription.u40.cpp | high |
| 9551 | 1095 |  | `std::map<group_key_type, list_iterator, group_key_less<int>>::insert(first, last) (boost::signals2 grouped_list)` | boost/signals2/detail/slot_groups.hpp | library: lib:boost (no file written) | medium |
| 9585 | 769 |  | `ecourna::api::compression::CLzmaCompress::OnProgress` | ecourna-lib/ecourna/api/compression/clzmacompress.cpp | src/ecourna/api/compression/clzmacompress.u40.cpp | medium |
| 11502 | 10 |  | `atexit: ~std::mutex of CPartidos::GetInst (@1838904)` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | high |
| 11503 | 37 |  | `atexit: ~std::unique_ptr<comum::CPartidos> (@1838928)` | uenux2/src/app/comum/dados/cpartidos.cpp (path inferred) | src/uenux2/src/app/comum/dados/cpartidos.u40.cpp | high |
| 11506 | 10 |  | `atexit: ~std::mutex of comum::CLocal::GetInst (@1838876)` | uenux2/src/app/comum/dados/clocal.cpp | src/uenux2/src/app/comum/dados/clocal.u02.cpp (u02 comment) | high |
| 11509 | 10 |  | `atexit: ~std::mutex comum::CHV::s_mutex (@1838848)` | uenux2/src/app/comum/dados/chv.cpp | src/uenux2/src/app/comum/dados/chv.h (static declared there) | high |
| 11510 | 12 |  | `atexit: ~std::unique_ptr<comum::CHV> s_inst (@1838872)` | uenux2/src/app/comum/dados/chv.cpp | src/uenux2/src/app/comum/dados/chv.h (static declared there) | high |
| 11512 | 10 |  | `atexit: ~std::mutex of comum::CFotos::GetInst (@1838820)` | uenux2/src/app/comum/dados/cfotos.cpp | src/uenux2/src/app/comum/dados/cfotos.u21.cpp (u21 comment) | high |
| 11562 | 21 |  | `ecourna::api::exception::CError::what` | ecourna-lib/ecourna/api/exception/cerror.cpp (path inferred) | src/ecourna/api/exception/cerror.cpp | high |

---

## 15. Open questions

* `CError` +24: four bytes the constructor never writes (padding or a member set elsewhere).
* The exact source form of the `CPrng` seed truncation (`static_cast<uebyte>`, a templated helper, or a
  `uebyte`-typed engine argument), and whether the real urna registers `CPrng`, `CTrng` or a hardware RNG as `IRng`
  (the registering `main` of this build is the web one).
* Whether `CCepescCipher` is a web-only stub or the only CEPESC implementation in this source tree.
* 2379: why five CBaseError families call their CError part through a per-family function (a templated forwarding
  constructor would explain five identical bodies). The same five families are the only users of the out-of-line
  deleting destructor 1940, so the answer is probably in how the TUs that define them were built.
* (resolved) The "FederacaoID" type is `CBaseType<unsigned short, 100, 999, 39>`. The srcloc record @1124484 that the
  inlined check passes contains the full signature.
* 9063's real name and home: it is a by-value helper (not an `OCTET_STRING` constructor) shared by 9060 and 9062.
* File names: all u40 classes lack a srcloc; the directory choices (e.g. `asn/` vs `asn/resultadournacadastro/` for
  `CConversorRegistroIdentificacaoEleitor`, `asn/` vs `asn/boletimurna/` for `CConversorDetalhamentoComparecimento`,
  `api/util/datahora.*` for the date helpers) are inferences. Other units include some of these headers under
  different directories.
* Layout disagreement to resolve in u14's `cinformacaomidia.h`: 9201 reads `numeroTentativas` from +32 and
  `tamanhoSenha` from +36 (u11 agrees; u14's comment has them swapped).

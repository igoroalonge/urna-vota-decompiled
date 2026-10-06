// Functions of unit u35 ("app:comum: classes without known file") whose original file belongs to another unit, or
// that are compiler/library instances. Reconstructed from vota_web_wasm.wasm (unit u35).
// Every function index of the unit is mapped in docs/modules/u35-app-comum-classes-without-known-file.md.
#include <memory>
#include <mutex>
#include <source_location>
#include <string>

namespace comum {

// =================================================================================================================
// Singleton helpers produced by wasm-opt's merge-similar-functions / identical-code folding
// =================================================================================================================

// wasm func 1406 - observed executing. Merged body of the "checked" singleton
// accessors: comum::CConfiguracaoEleicao::GetInst (187), CCargos::GetInst (273), CEleitores::GetInst (326),
// vota::CTelasVota::GetInst (407), CRdvVota::GetInst (555), CPE::GetInst (1073), md::CValidadorIdentidade::GetInst
// (1926), CRespostas::GetInst (2812), CHV::GetInst (2817). Each caller passes its mutex, its srcloc record, its
// message ("<Classe> - instancia nao criada") and the address of its static pointer. The lock itself was compiled
// away (no pthreads in this build): only the unlock residue (func 150) remains.                     name inferred
template <typename T>
T& GetInstCriada(std::mutex& mutex, const std::source_location& local, const char* mensagem, T* const& instancia)
{
    std::lock_guard<std::mutex> trava(mutex);
    if (instancia == nullptr)
        throw ecourna::api::pattern::CPatternError(ecourna::api::pattern::EPatternErr(1303), mensagem, local);  // 331
    return *instancia;
}
// e.g.  CConfiguracaoEleicao& CConfiguracaoEleicao::GetInst()
//       { return GetInstCriada(s_mutex /*@1838756*/, std::source_location::current() /*:36*/,
//                              "CConfiguracaoEleicao - instancia nao criada", s_instancia /*@1838752*/); }

// wasm func 2742 - name as used by u24/u10 (CCodecWsq, "?"): lazily created empty (1-byte) singleton @1908636 under
// the mutex @1908612 (ICF body func 2900, shared with 948 CLogComum::GetInst, 348 and 3603). Its method calls were
// compiled into plain copies/empty results in this build (the WSQ fingerprint codec is not in the web build).
// Callers: CControlaArmazenamentoDeImagens::LeChavePublica (2725), CPedeDigitalMesario::ProcessTick (10316),
// vota::CRegistraDigitalOperador::ProcessTick (10451), vota::CPedeDigital::ProcessTick (10465).
class CCodecWsq;                                                                  // name inferred  ?
CCodecWsq& CCodecWsq_GetInst();   // = GetInstLazy<CCodecWsq>(mutex @1908612, unique_ptr @1908636)   (func 2742)

// Already reconstructed by other units (functions of this unit):
//   wasm func 948  CLogComum::GetInst()           src/uenux2/src/app/comum/log/clogcomum.cpp (u24)
//   wasm func 1391 CJustificador::GetInst()       src/uenux2/src/app/comum/justificativa/cjustificador.cpp (u24)
//   wasm func 2729 CPedeTituloMesario::GetInst()  src/uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp

// =================================================================================================================
// Error-constructor thunk
// =================================================================================================================

// wasm func 591 - thunk: ecourna_f710(exception, code, message, srcloc, vtable @1552744) = constructor of
// CUeComumMdError = CBaseError<comum::EUeComumMdError, {8900, 8950}> (typeinfo @1552724). Called by the inlined md
// constructors (CCabecalhoEntidade, CAbrangencia, CSeguranca, CIdentificacaoUrna, CIdentificacaoSecao, CIDEleitoral,
// CIDPacote, CCabecalhoPacote in CConversorCabecalhoPacote 11437/11438). Merge-similar-functions artefact of
//     md::CUeComumMdError::CUeComumMdError(EUeComumMdError, std::string, const std::source_location&)

// =================================================================================================================
// Implicit destructors of application structs
// =================================================================================================================

// wasm func 857 - md::estadoaplicacao::CDadoCorrespondencia::~CDadoCorrespondencia() (implicit; class of u29).
// Destroys +60 CIdentificadorGeradorMidia (shared_f2240: three strings), +48 std::vector<uebyte> assinatura
// (func 520), +28 codigoCarga, +4 numeroSerieFC. Callers: ~CGravadorBU, ~CGravadorRDV (11585), ~IGravadorEnvelope
// (via 6043), ~CGravadorEnvelopeArquivo (11621/11622), vota::CInformacaoEleitor::Inicializar (7787), mock/state
// copies...

// wasm func 2885 - CCabecalhoQRCode::~CCabecalhoQRCode() (implicit; struct of 33 std::string fields = the
// "CHAVE:valor " prefixes of the BU QR-code header, declared in cgeradorbuqrcode.u04-fragment.cpp). Destroys the 33
// strings from the last (+384) to the first (+0). Callers: ~CGeradorBUQRCode (2251), CPolySingletonList::instance
// (1956), CCargos::GetCurrentEleicaoVersaoPacote (5604 = CGeradorBUQRCode::GeraQRCodes), vota::CGeraBU (12110).

} // namespace comum

// =================================================================================================================
// Library template instances attributed to this unit (not reconstructed: they are the standard library)
// =================================================================================================================
//   248   std::swap / iter_swap of md::CRespostaConsulta {int, std::string, std::string} (28 bytes)  - std::sort helper
//   1920  std::__sort4<..., md::CRespostaConsulta*> (compares the int at +0 = número)              - std::sort helper
//   2790  std::__insertion_sort_incomplete<..., md::CRespostaConsulta*>                            - std::sort helper
//         (the introsort bodies are func 5605, called by CGeradorBUQRCode::GeraQRCodes 5604, and 5610, called by
//          CGeradorBUBase::ImprimeConsulta: consulta answers sorted by número)
//   1221  std::map<std::string, std::string>::__emplace_hint_unique(hint, const value_type&)  (40-byte node)
//         - users: CSubstituidorTitulo::GetInst, CPacoteArquivos::ValidarChaveEAplicacaoValida,
//           md::CVersoesArquivos::ValidaCriacao, vota::CGravaResultado (map copies)          observed executing
//   1405  std::__tree<...>::destroy(node*) of std::map<md::ETipoAbrangencia, SQtdeAptos> (trivial node values)
//   1552  std::pair<const char, ecourna::app::dados::CLabelParametrizado>::pair(const char&, const CLabelParametrizado&)
//         (CLabelParametrizado = {EGeneroLabel; 4 x std::string}; map of parametrised labels of CTradutorFrase)
//   1555  std::filesystem::path::string() const (returns a copy of the native string)
//   1874  std::vector<char>::vector(const vector&) (OCTET_STRING / SEQUENCE copies of the III ASN.1 runtime)
//   1922  std::function<SQtdeAptos()>::function(const function&)
//   2188  ASN1::CHOICE::operator=(const CHOICE&): clone the selected alternative (vtable slot 3), delete the old one
//         (slot 1), copy the choice index (III ASN.1 runtime, table slot 6895)
//   2769  std::vector<char>::__vallocate(size_t) (throws vector::__throw_length_error for n < 0)  observed executing
//   2806  std::__tree<std::string>::destroy(node*) (std::set<std::string>: CVisitanteEleitor, CConversorEntidadeEleitores)

// Reconstructed from vota_web_wasm.wasm by unit u11 - FRAGMENTS of other original files.
//
// The unit builder attached these implicit special members of ecourna data classes to u11 because their
// callers are the converters reconstructed here. Their real homes are the class headers (unit u14 owns the
// .cpp files). They are compiler-generated (`= default`); the layouts they reveal are recorded below.
// See docs/modules/u11-ecourna-lib-ecourna-api-asn.md.

namespace ecourna::app::dados {

// =================================================================================================
// ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.h  (path inferred; .cpp is srcloc-attested)
// =================================================================================================
// Declared (unit u14) in src/ecourna/app/dados/cregistroidentificacaoeleitor.h; member names as there:
// class CRegistroIdentificacaoEleitor {                          // 20 bytes
//     TSharedIdentificadorEleitor m_identificadorHabilitacao;                 // +0 (+4 control block)
//     std::optional<TSharedIdentificadorEleitor> m_identificadorPrincipal;    // +8 (+12), flag +16
// };   (TSharedIdentificadorEleitor = std::shared_ptr<IIdentificadorEleitor>)
// IIdentificadorEleitor is implemented by CNumeroInscricaoEleitoral / CNumeroCPF / CNumeroIdentificacaoLivre
// (the three alternatives of the ModuloTiposEleitorais::IdentificadorEleitor CHOICE).
//
// wasm func 1876 - CRegistroIdentificacaoEleitor::~CRegistroIdentificacaoEleitor() (implicit): releases the
// optional shared_ptr (if engaged) and then the first one. Used by CConversorIdentificacaoJustificativa (9064),
// CConversorEstadoHabilitacaoPorCodigo (9073), CConversorEstadoComparecimento (9081),
// CConversorComparecimentoMesario (9106) and the std::transform instances 9092/9094.
CRegistroIdentificacaoEleitor::~CRegistroIdentificacaoEleitor() = default;

// =================================================================================================
// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h  (path inferred)
// =================================================================================================
// wasm func 1006 - CEstadoComparecimento::~CEstadoComparecimento() (implicit, 96-byte object):
//   if (+92 habilitacaoBiometrica engaged && +88 habilitacaoPorCodigo engaged && +84 mesário engaged)
//       ~CRegistroIdentificacaoEleitor at +64 (inner optional flag +80)
//   ~CRegistroIdentificacaoEleitor at +0 (the voter)
// Used by the vector<CEstadoComparecimento> code of CConversorComparecimentoSecao (9118, 9119, 5100, 5096),
// CConversorDadosComparecimento (9095), comum_f2845 (relocation), ~CComparecimentoSecao (5097), func 1162
// ("ecourna_f1162") and comum::CGravadorRCSecao (11616).
CEstadoComparecimento::~CEstadoComparecimento() = default;

// =================================================================================================
// ecourna-lib/ecourna/app/dados/resultadournacadastro/ccomparecimentosecao.h  (path inferred)
// =================================================================================================
// wasm func 5097 - CComparecimentoSecao::~CComparecimentoSecao() (implicit): destroys the
// std::vector<CEstadoComparecimento> at +16 (the CIdentificacaoSecaoEleitoral at +0 is trivial). Only
// caller: the unwinding path of CConversorDadosComparecimento::DoDeconverte (9095).
CComparecimentoSecao::~CComparecimentoSecao() = default;

}  // namespace ecourna::app::dados

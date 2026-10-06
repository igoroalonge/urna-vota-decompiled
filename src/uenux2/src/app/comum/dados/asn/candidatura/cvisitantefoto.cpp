// uenux2/src/app/comum/dados/asn/candidatura/cvisitantefoto.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
//   EntidadeFotosCandidatos ::= SEQUENCE { cabecalho, abrangencia, fotos SEQUENCE OF FotoCandidato }
//   FotoCandidato ::= SEQUENCE { codigoCandidato GeneralString (SIZE(0..18)), foto Foto }
// The partial BER decoder calls visit() at the start of every element whose dotted path was registered.
#include "comum/dados/asn/candidatura/cvisitantefoto.h"

namespace comum::asn {

namespace {
constexpr auto CAMINHO_FOTO   = "EntidadeFotosCandidatos.fotos.OF";                // @329707 (32 chars)
constexpr auto CAMINHO_CODIGO = "EntidadeFotosCandidatos.fotos.codigoCandidato";   // @127254 (45 chars)
} // namespace

// wasm func 11442 (vtable slot 2; srcloc lines 40, 48, 56, 63)
ASN1::IAbstractVisitor::EVisitorResult
CVisitanteFoto::visit(int /*nivel*/, const std::string& caminho, ASN1::AbstractData& dado,
                      const std::size_t inicio, const std::size_t fim)
{
    if (caminho == CAMINHO_FOTO) {
        if (m_sequenciador != 0) {
            throw CDadosError(7883, "Sequenciador com valor errado para o carregamento de fotos do candidato.");    // line 40
        }
        m_sequenciador = 1;
        m_posicao = {inicio, fim};
        return EVisitorResult{1};   // release the decoded element afterwards (ASN1::DataFree): images are not kept
    }

    if (caminho == CAMINHO_CODIGO) {
        if (m_sequenciador != 1) {
            throw CDadosError(7884, "Sequenciador com valor errado para o carregamento de codigo do candidato.");   // line 48
        }
        m_sequenciador = 0;
        const auto* codigo = dynamic_cast<const ASN1::AbstractString*>(&dado);
        if (codigo == nullptr) {
            throw CDadosError(7885, "Erro ao carregar codigo do candidato.");   // line 56
        }
        // Built from c_str(): the code is cut at the first NUL byte. The wasm builds the string, then a pair with
        // func 5659 (both members COPIED, not moved), destroys the string, and copy-constructs the pair into the
        // vector (no move anywhere). That matches a CONST temporary string (TSE functions often return
        // `const std::string`, e.g. CCandidaturas::RecuperaVersaoPacote) and an lvalue pair pushed with push_back.
        const TIndice indice(static_cast<const std::string>(codigo->getValue().c_str()), m_posicao);   // func 5659
        m_indices.push_back(indice);
        return EVisitorResult{1};   // enumerator name unknown
    }

    throw CDadosError(7886, "Entidade visitada invalida " + caminho);   // line 63
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU: wasm func 5659 — TIndice (std::pair<std::string, TPosicao>) constructor from
// (const std::string&, const TPosicao&); also used by CIndexadorFotos::vf2 (func 11441). Library/inline code.

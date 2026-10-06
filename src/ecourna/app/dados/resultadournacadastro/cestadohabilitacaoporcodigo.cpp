// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
// On a throw the constructors destroy the optional member (func 5089 = ~optional<CRegistroIdentificacaoEleitor>).
#include "ecourna/app/dados/resultadournacadastro/cestadohabilitacaoporcodigo.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 5090 (srcloc line 23): without the mesário's identification the situation must be NaoReconhecido.
CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacaoReconhecimentoMesario situacao)
    : m_situacao(situacao)
    , m_identificacaoMesario(std::nullopt)
{
    if (m_situacao != NaoReconhecido) {
        throw CDadosResultadoUrnaCadastroError(
            3270, "Para situação diferente de NaoReconhecido é preciso informar o título de eleitor do mesário.");   // line 23
    }
}

// wasm func 5088 (srcloc line 37): with the identification the situation must not be NaoReconhecido.
CEstadoHabilitacaoPorCodigo::CEstadoHabilitacaoPorCodigo(ESituacaoReconhecimentoMesario situacao,
                                                         const CRegistroIdentificacaoEleitor& identificacaoMesario)
    : m_situacao(situacao)
    , m_identificacaoMesario(identificacaoMesario)
{
    if (m_situacao == NaoReconhecido) {
        throw CDadosResultadoUrnaCadastroError(
            3271, "Para situação igual a NaoReconhecido não deve ser informado o título de eleitor do mesário.");     // line 37
    }
}

// wasm func 9017 (srcloc line 46). ("habiliação" [sic] in the original message.)
const CRegistroIdentificacaoEleitor& CEstadoHabilitacaoPorCodigo::GetIdentificacaoMesario() const
{
    if (!m_identificacaoMesario.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3272, "Não existe título de eleitor associado a esta habiliação.");  // line 46
    }
    return *m_identificacaoMesario;
}

} // namespace ecourna::app::dados

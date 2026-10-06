// ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9024 (srcloc line 38)
const TVectorComparecimentoMesario& CDadosComparecimento::GetMesariosAbertura() const
{
    if (!m_mesariosAbertura.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3278, "Comparecimento de mesários na abertura não definido para este objeto.");      // line 38
    }
    return *m_mesariosAbertura;
}

// wasm func 9023 (srcloc line 49)
const TVectorComparecimentoMesario& CDadosComparecimento::GetMesariosEncerramento() const
{
    if (!m_mesariosEncerramento.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3279, "Comparecimento de mesários no encerramento não definido para este objeto.");  // line 49
    }
    return *m_mesariosEncerramento;
}

// Implicit special members instantiated in this unit:
//   wasm func 5846  CDadosComparecimento(const CDadosComparecimento&) = default
//                   (justificativas: 24-byte elements with two shared_ptr copies; seção 16 bytes;
//                    eleitores via func 2278 (uninitialized copy, unit u23); the two optionals via 5832)
//   wasm func 5832  std::optional<TVectorComparecimentoMesario>(const optional&) (40-byte elements)
//   wasm func 1162  ~CDadosComparecimento() = default
//   wasm func 1943  ~std::optional<TVectorComparecimentoMesario>()

} // namespace ecourna::app::dados

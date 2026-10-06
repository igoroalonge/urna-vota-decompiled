// ecourna-lib/ecourna/app/dados/cnumeroidentificacaolivre.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/cnumeroidentificacaolivre.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
// The constructor body is the merged func 3939 (see cnumeroinscricaoeleitoral.cpp).
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"

#include <format>

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 5106 -> 3939 (srcloc line 22)
CNumeroIdentificacaoLivre::CNumeroIdentificacaoLivre(const std::string& numero)
    : IIdentificadorEleitor(TTipoIdentificadorEleitor(TipoIDNumeroLivre), numero, TAMANHO)
{
    if (numero.size() > m_tamanho) {
        throw CDadosError(2026, std::format("O tamanho do número de identificação livre não pode ser maior do que {} ({}).",
                                            m_tamanho, numero));                                 // line 22
    }
}

// wasm func 9263 (vtable slot 2): a 9-byte thunk to the GetNumero body (func 1139).
std::string CNumeroIdentificacaoLivre::GetNumeroFormatado() const
{
    return GetNumero();
}

} // namespace ecourna::app::dados

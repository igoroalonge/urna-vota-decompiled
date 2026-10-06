// ecourna-lib/ecourna/app/dados/cnumerocpf.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/cnumerocpf.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
// The constructor body is the merged func 3939 (see cnumeroinscricaoeleitoral.cpp).
// Note: only length and digits are checked; the two CPF check digits are NOT validated here
// (they are checked by the uenux2 input rule comum::md::CRegraCPF, func 11270, before this class is built).
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"

#include <format>

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 5107 -> 3939 (srcloc line 20)
CNumeroCPF::CNumeroCPF(const std::string& numero)
    : IIdentificadorEleitor(TTipoIdentificadorEleitor(TipoIDCPF), numero, TAMANHO)
{
    if (numero.size() > m_tamanho) {
        throw CDadosError(2025, std::format("O tamanho do número de CPF não pode ser maior do que {} ({}).",
                                            m_tamanho, numero));                                 // line 20
    }
}

// wasm func 9264 (vtable slot 2). "12345678901" -> "123.456.789-01".
std::string CNumeroCPF::GetNumeroFormatado() const
{
    std::string numero = GetNumero();
    return numero.insert(9, "-").insert(6, ".").insert(3, ".");
}

} // namespace ecourna::app::dados

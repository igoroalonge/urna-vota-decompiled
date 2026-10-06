// ecourna-lib/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/cnumeroinscricaoeleitoral.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// The constructors of the three identifier classes differ only in constants, so wasm-opt
// (merge-similar-functions) turned them into 33-byte thunks over one shared body, wasm func 3939:
//     ecourna_f3939(this, numero, &srcloc, code, fmt{ptr,len}, derived vtable, tamanho, tipo)
//   CNumeroInscricaoEleitoral : srcloc cnumeroinscricaoeleitoral.cpp:21, code 2024, tamanho 12, tipo 1
//   CNumeroCPF                : srcloc cnumerocpf.cpp:20,                code 2025, tamanho 11, tipo 2
//   CNumeroIdentificacaoLivre : srcloc cnumeroidentificacaolivre.cpp:22, code 2026, tamanho 12, tipo 3
// The shared body: TTipoIdentificadorEleitor(tipo) (func 2666), IIdentificadorEleitor(...) (func 9256),
// store the derived vptr, then the length check below; on a throw the base is destroyed (func 778).
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"

#include <format>

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 1241 -> 3939 (srcloc line 21)
CNumeroInscricaoEleitoral::CNumeroInscricaoEleitoral(const std::string& numero)
    : IIdentificadorEleitor(TTipoIdentificadorEleitor(TipoIDNumeroInscricao), numero, TAMANHO)
{
    if (numero.size() > m_tamanho) {
        throw CDadosError(2024, std::format("O tamanho do número de inscrição eleitoral não pode ser maior do que {} ({}).",
                                            m_tamanho, numero));                                 // line 21
    }
}

// wasm func 9262 (vtable slot 2). "123456789012" -> "1234 5678 9012".
std::string CNumeroInscricaoEleitoral::GetNumeroFormatado() const
{
    std::string numero = GetNumero();
    return numero.insert(8, " ").insert(4, " ");
}

} // namespace ecourna::app::dados

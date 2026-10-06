// ecourna-lib/ecourna/app/dados/iidentificadoreleitor.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/iidentificadoreleitor.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/iidentificadoreleitor.h"

#include <algorithm>
#include <format>

#include "ecourna/api/util/cstringutils.h"   // ? PadLeft helper (func 753, unit u05)
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9256 (srcloc line 28).
// The number is stored left-padded with '0' up to `tamanho` characters (func 753: copy the string,
// then insert(0, tamanho - size, '0') when it is shorter; a longer number is kept as is and is
// rejected by the derived constructors). It must then be numeric and must not be all zeros.
IIdentificadorEleitor::IIdentificadorEleitor(TTipoIdentificadorEleitor tipo, const std::string& numero,
                                             uebyte tamanho)
    : m_tipo(tipo)
    , m_numero(api::util::CStringUtils::PadLeft(numero, '0', tamanho))   // ? helper name
    , m_tamanho(tamanho)
{
    const bool todosZeros = (m_numero == std::string(m_tamanho, '0'));
    const bool numerico = std::ranges::all_of(m_numero, [](char c) { return c >= '0' && c <= '9'; });
    if (todosZeros || !numerico) {
        // The message prints the number as received (before padding).
        throw CDadosError(2023, std::format("O número de identificação de eleitor deve ser numérico ({}).",
                                            numero));                                         // line 28
    }
}

// wasm func 778 (vtable slot 0). Restores the vptr and frees m_numero (shared helper func 1727).
IIdentificadorEleitor::~IIdentificadorEleitor() = default;

} // namespace ecourna::app::dados

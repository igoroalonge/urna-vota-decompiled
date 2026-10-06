// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original (path inferred): uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.h
//
// CEleitorIdentidade = one identifier of a voter: the digits of a título de eleitor (voter card
// number, 12 digits), of a CPF (taxpayer id, 11 digits) or of a "free" identifier, plus its type.
// sizeof == 16. Compared with the defaulted operator<=> (string, then type) - see func 456.
#pragma once

#include <compare>
#include <cstdint>
#include <string>

namespace ecourna::app::dados {
enum class ETipoIdentificadorEleitor : std::uint8_t {   // stored as int, formatted as (tipo & 255)
    TITULO = 1,   // título de eleitor, 12 digits
    CPF    = 2,   // 11 digits
    LIVRE  = 3,   // ? free identifier (rule CRegraIdentidadeLivre), padded to 12
};
}

namespace comum::md {

using ecourna::app::dados::ETipoIdentificadorEleitor;

class CEleitorIdentidade {
public:
    CEleitorIdentidade(std::string identidade, ETipoIdentificadorEleitor tipo);   // func 566

    // func 2798  name inferred: left-pad/trim leading zeros to 11 (CPF) or 12 digits
    static std::string Formata(std::string identidade, ETipoIdentificadorEleitor tipo);

    const std::string& GetIdentidade() const { return m_identidade; }
    ETipoIdentificadorEleitor GetTipo() const { return m_tipo; }

    auto operator<=>(const CEleitorIdentidade&) const = default;
    bool operator==(const CEleitorIdentidade&) const = default;

private:
    std::string m_identidade;              // +0
    ETipoIdentificadorEleitor m_tipo;      // +12 (int-sized)
};

}  // namespace comum::md

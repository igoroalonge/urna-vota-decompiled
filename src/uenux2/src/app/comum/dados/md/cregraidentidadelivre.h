// uenux2/src/app/comum/dados/md/cregraidentidadelivre.h   (path inferred: RTTI only; siblings cregratitulo.cpp /
// cregracpf.cpp next to the attested iregraidentidade.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// Identity rules (IRegraIdentidade, unit u05) validate and format the identifier the poll worker types to find a
// voter: título de eleitor (12 digits + check digits), CPF (11 digits + check digits) or a FREE identifier
// ("identificação livre", ETipoIdentificadorEleitor 3, e.g. elections of other organisations that use their own ids).
// CValidadorIdentidade::GetInst() is built at start-up with one rule per identifier type enabled in the election
// configuration (1 -> CRegraTitulo, 2 -> CRegraCPF, 3 -> CRegraIdentidadeLivre).
//
// RTTI: comum::md::CRegraIdentidadeLivre : comum::md::IRegraIdentidade, vtable @1574652
//   [0] 174 [1] 144 [2] 11273 GetTipo [3] 11274 Formata [4] icf_ret_1_vf0 (371) Verifica. sizeof 4.
#pragma once

#include <string>

#include "comum/dados/md/iregraidentidade.h"

namespace comum::md {

class CRegraIdentidadeLivre final : public IRegraIdentidade
{
public:
    ETipoIdentificadorEleitor GetTipo() const override;                       // wasm func 11273
    std::string Formata(const std::string& identidade) const override;       // wasm func 11274
    bool Verifica(const std::string&) const override { return true; }        // ICF body 371 "return 1"
};

} // namespace comum::md

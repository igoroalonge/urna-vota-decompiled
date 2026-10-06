// uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21: constructor only; DoDesconverte 11448 belongs to another unit).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/candidatura/ccandidatura.h"
#include "ModuloCandidatos.h"

namespace comum::asn {

// Converts one ModuloCandidatos::Candidatura. The ASN.1 record does not carry the office, the party or the
// "apto" status (they come from the enclosing CandidatosPorCargos / CandidatoPorPartido), so the converter is built
// with them.
// RTTI: CConversorCandidatura : IConversorASN<ModuloCandidatos::Candidatura, md::CCandidatura>
// vtable @1562292: [0] 174 [1] 144 [2] 11446 (base DoConverte, throws 7655) [3] 11448 DoDesconverte. sizeof 12.
class CConversorCandidatura : public IConversorASN<ModuloCandidatos::Candidatura, md::CCandidatura>
{
public:
    // wasm func 5712 (only caller: CConversorCandidaturas::DoDesconverte 11445)   // name inferred
    CConversorCandidatura(TCargoID cargo, TPartidoID partido, bool apto)
        : m_cargo(cargo), m_partido(partido), m_apto(apto) {}

protected:
    TDado DoDesconverte(const TEntidade& candidatura) const override;   // wasm func 11448 (other unit)

private:
    TCargoID m_cargo;       // +4 (uint8)
    TPartidoID m_partido;   // +6 (uint16)
    bool m_apto;            // +8 true = list candidatosAptos, false = candidatosInaptos
};

} // namespace comum::asn

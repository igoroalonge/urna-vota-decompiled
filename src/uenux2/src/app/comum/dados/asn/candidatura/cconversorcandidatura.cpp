// uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidatura.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: DoDesconverte). The class declaration (with the inline constructor,
// func 5712) was written by unit u21 in cconversorcandidatura.h.
//
//   Candidatura ::= SEQUENCE { numero INTEGER (0..99999), titular DadosCandidato, drap GeneralString (SIZE(0..18)),
//                              suplentesVices SEQUENCE OF DadosCandidato OPTIONAL }
// "suplentesVices" = the running mates: vice-presidente / vice-governador / vice-prefeito, or the two suplentes of
// a senador. "drap" (the registration request id) is not used by the urna.
#include "comum/dados/asn/candidatura/cconversorcandidatura.h"

#include <vector>

#include "comum/dados/asn/candidatura/cconversordadoscandidato.h"

namespace comum::asn {

// wasm func 11448 - vtable slot 3. Observed executing (candidate files read at votaInit).
md::CCandidatura CConversorCandidatura::DoDesconverte(const ModuloCandidatos::Candidatura& candidatura) const
{
    const CConversorDadosCandidato conversor(m_apto);                               // {vtable @1562232, apto}
    const auto numero = static_cast<md::TCandidatoID>(candidatura.get_numero());
    const md::CDadosCandidato titular = conversor.Desconverte(candidatura.get_titular());      // func 5711

    std::vector<md::CDadosCandidato> suplentes;
    if (candidatura.hasOptionalField(0)) {
        const auto& lista = candidatura.get_suplentesVices();
        for (std::size_t i = 0; i < lista.size(); ++i)
            suplentes.push_back(conversor.Desconverte(lista.at(i)));                // at(): vector::__throw_out_of_range
    }
    // md::CCandidatura's constructor (func 5660) validates (ccandidatura.cpp:37..55).
    return md::CCandidatura(m_cargo, m_partido, numero, titular, suplentes);
}

} // namespace comum::asn

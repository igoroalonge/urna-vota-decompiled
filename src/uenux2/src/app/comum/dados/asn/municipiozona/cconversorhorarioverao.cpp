// uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u03 holds only DoConverte; DoDesconverte, func 11384, is in u35).
// No srcloc record names this file: the class comum::asn::CConversorHorarioVerao is known from RTTI only, and
// TSE puts every converter in a file named after the class next to its siblings (municipiozona/).
//
//   HorarioVerao ::= SEQUENCE { inicioHV DataJE, fimHV DataJE, diferencaHV INTEGER (-180..180) }
#include "comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.h"   // class declared there (see note)

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11385 (vtable slot 2)
ModuloComplementosMunicipios::HorarioVerao CConversorHorarioVerao::DoConverte(const TDado& horario) const
{
    ModuloComplementosMunicipios::HorarioVerao entidade;
    entidade.set_inicioHV(Utils::ConverteDataJE(horario.GetInicio()));   // func 3802: CDate -> "YYYYMMDD"
    entidade.set_fimHV(Utils::ConverteDataJE(horario.GetFim()));
    entidade.set_diferencaHV(horario.GetDiferenca());
    return entidade;
}

} // namespace comum::asn

// FRAGMENT of uenux2/src/app/comum/dados/asn/municipiozona/cconversorhorarioverao.cpp (path inferred; file of unit
// u03, which wrote DoConverte = func 11385; the class is declared in cconversorcomplementosmunicipios.h).
// Reconstructed from vota_web_wasm.wasm (unit u35: DoDesconverte = func 11384).
//
//   HorarioVerao ::= SEQUENCE { inicioHV DataJE, fimHV DataJE, diferencaHV INTEGER (-180..180) }
// Daylight-saving period of a município (complementos de municípios, "-cm.dat"): the urna shifts its clock by
// diferencaHV minutes between the two dates.
#include "comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.h"

#include "comum/asn/util.h"

// md::CHorarioVerao's constructor (chorarioverao.cpp:29/:32), inlined here: src/uenux2/src/app/comum/dados/md/chorarioverao.u35.cpp

namespace comum::asn {

// wasm func 11384 - vtable slot 3. Not observed executing (only reached for a município whose complemento has the
// optional horário de verão).
md::CHorarioVerao CConversorHorarioVerao::DoDesconverte(const ModuloComplementosMunicipios::HorarioVerao& horario) const
{
    const api::CDate inicio = Utils::DesconverteDataJE(horario.get_inicioHV());   // func 2276
    const api::CDate fim = Utils::DesconverteDataJE(horario.get_fimHV());
    return md::CHorarioVerao(inicio, fim, horario.get_diferencaHV());
}

} // namespace comum::asn

// FRAGMENT of ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp (path
// inferred; class + DoDeconverte by unit u11). Reconstructed by unit u40 from vota_web_wasm.wasm.
#include "ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.h"

namespace ecourna::app::dados::asn {

// wasm func 9144 (vtable @1129456 slot 2)
//   ConfiguracaoMunicipio ::= SEQUENCE { codigoMunicipio INTEGER (1..99999), horariosUrna HorariosUrna }
ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio
CConversorConfiguracaoMunicipio::DoConverte(const CConfiguracaoMunicipio& dado) const
{
    const CConversorHorariosUrna conversorHorarios;                                        // vptr @1130328
    ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio entidade;
    // Built through a Constrained_INTEGER<1, 99999> temporary (vtable @1567692); no range check here.
    entidade.set_codigoMunicipio(dado.GetCodigoMunicipio());
    entidade.set_horariosUrna(conversorHorarios.Converte(dado.GetHorariosUrna()));        // 9143 -> 9133
    return entidade;
}

}  // namespace ecourna::app::dados::asn

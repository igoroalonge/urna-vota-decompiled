// ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.cpp   (path inferred:
//   the data class is ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp (srcloc), converters
//   mirror the data directories under asn/)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
//
// "Configuração de municípios" = the per-municipality urna schedule of file <..>-cfm.dat, e.g. from the
// municipal-t1 scenario (t02410ac-cfm.dat, municipalities 1, 2, 3 all alike):
//   emissaoZeresima 20261004T070000, inicioVotacao 20261004T080000,
//   encerramentoVotacao 20261004T170000, terminoVotacao 20261005T040000
#include "ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.h"

namespace ecourna::app::dados::asn {

// wasm func 9142 (vtable slot 3). The tool named it "IConversorASN<HorariosUrna, CHorariosUrna>::Deconverte"
// (inlined call, srcloc :66 record @1129744). Observed at run time (votaInit loads the -cfm.dat file).
CConfiguracaoMunicipio CConversorConfiguracaoMunicipio::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorHorariosUrna conversorHorarios;   // vptr @1130328

    // Left-to-right: the code is range-checked BEFORE the schedule is converted.
    return CConfiguracaoMunicipio(TCodigoMunicipio(entidade.get_codigoMunicipio()),
                                  conversorHorarios.Deconverte(entidade.get_horariosUrna()));   // -> 9132
}

}  // namespace ecourna::app::dados::asn

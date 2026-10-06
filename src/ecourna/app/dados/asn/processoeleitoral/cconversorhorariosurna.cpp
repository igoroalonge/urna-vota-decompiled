// ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40. No srcloc (nothing thrown here; a malformed date makes the
// Boost parser throw from ConverteDataHoraJE).
#include "ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.h"

#include "ecourna/api/util/datahora.hpp"

namespace ecourna::app::dados::asn {

using api::util::ConverteDataHoraJE;
using api::util::FormataDataHoraJE;

// wasm func 9133 (vtable slot 2). Each date: FormataDataHoraJE (func 9220) -> DataHoraJE temporary
// (AbstractString(info @1913292, text), vptr @1556172) -> string assignment into the field.
ModuloConfiguracaoMunicipios::HorariosUrna CConversorHorariosUrna::DoConverte(const CHorariosUrna& horarios) const
{
    ModuloConfiguracaoMunicipios::HorariosUrna entidade;
    entidade.set_emissaoZeresima(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(horarios.m_emissaoZeresima)));
    entidade.set_inicioVotacao(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(horarios.m_inicioVotacao)));
    entidade.set_encerramentoVotacao(
        ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(horarios.m_encerramentoVotacao)));
    entidade.set_terminoVotacao(ModuloTiposEleitorais::DataHoraJE(FormataDataHoraJE(horarios.m_terminoVotacao)));
    return entidade;
}

// wasm func 9132 (vtable slot 3). Observed executing (votaInit loads the -cfm.dat of the scenario). The four
// conversions run in field order (zerésima first), then the 32-byte POD is filled.
CHorariosUrna CConversorHorariosUrna::DoDeconverte(const ModuloConfiguracaoMunicipios::HorariosUrna& entidade) const
{
    return CHorariosUrna{
        ConverteDataHoraJE(entidade.get_emissaoZeresima()),       // +0
        ConverteDataHoraJE(entidade.get_inicioVotacao()),         // +8
        ConverteDataHoraJE(entidade.get_encerramentoVotacao()),   // +16
        ConverteDataHoraJE(entidade.get_terminoVotacao()),        // +24
    };
}

}  // namespace ecourna::app::dados::asn

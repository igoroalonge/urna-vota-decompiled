// ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorhorariosurna.h   (path inferred; already included
//   by u11's cconversorconfiguracaomunicipio.h)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.h"   // CHorariosUrna (u14)
#include "ModuloConfiguracaoMunicipios.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorHorariosUrna : IConversorASN<ModuloConfiguracaoMunicipios::HorariosUrna, CHorariosUrna>
// vtable @1130328: [0] 174  [1] 144  [2] 9133 DoConverte  [3] 9132 DoDeconverte (observed executing)
//   HorariosUrna ::= SEQUENCE { emissaoZeresima DataHoraJE, inicioVotacao DataHoraJE,
//                               encerramentoVotacao DataHoraJE, terminoVotacao DataHoraJE }
// The per-municipality schedule of the urna: when the zerésima (zero report) may be printed, when voting
// opens and closes, and the "término" deadline. Read from <fase><pleito><uf>-cfm.dat at start-up.
class CConversorHorariosUrna
    : public api::asn::IConversorASN<ModuloConfiguracaoMunicipios::HorariosUrna, CHorariosUrna>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9133
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9132
};

}  // namespace ecourna::app::dados::asn

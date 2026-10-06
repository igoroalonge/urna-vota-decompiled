// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorcodigocargoconsulta.h   (path inferred: RTTI only;
// included under this name by cconversorcargo.cpp, unit u03)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloTiposEleitorais:
//   CodigoCargoConsulta ::= CHOICE { cargoConstitucional [1] CargoConstitucional (ENUMERATED 1..24),
//                                    numeroCargoConsultaLivre [2] INTEGER (25..99) }
// The application keeps only the number (uebyte): 1..24 are the constitutional offices (Presidente, Governador,
// Senador, Deputado..., Prefeito, Vereador...), 25..99 are "consultas" (referenda) and other free offices.
//
// RTTI: comum::asn::CConversorCodigoCargoConsulta : IConversorASN<ModuloTiposEleitorais::CodigoCargoConsulta,
//       unsigned char>, vtable @1555736: [2] 11592 DoConverte [3] 11591 DoDesconverte. sizeof 4.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

class CConversorCodigoCargoConsulta : public IConversorASN<ModuloTiposEleitorais::CodigoCargoConsulta, uebyte>
{
protected:
    TEntidade DoConverte(const TDado& codigo) const override;      // wasm func 11592
    TDado DoDesconverte(const TEntidade& codigo) const override;   // wasm func 11591
};

} // namespace comum::asn

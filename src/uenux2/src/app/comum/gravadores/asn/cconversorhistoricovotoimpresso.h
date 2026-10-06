// uenux2/src/app/comum/gravadores/asn/cconversorhistoricovotoimpresso.h   (path inferred: RTTI only; the only
// user is the attested gravadores/asn/cconversorentidadebu.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// BU field historicoVotoImpresso (ModuloBoletimUrna):
//   HistoricoVotoImpresso ::= SEQUENCE { idImpressoraVotos INTEGER (0..99999999),
//                                        idRepositorioVotos INTEGER (0..99999999), dataHoraLigamento DataHoraJE }
// One entry per "voto impresso" printer session (the printed-vote module of some urna models: printer id, ballot
// box id, time it was switched on). The VOTA of this binary never fills the list (CEntidadeBU keeps it empty and
// CConversorEntidadeBU omits the optional field).
//
// RTTI: comum::asn::CConversorHistoricoVotoImpresso
//         : IConversorASN<ModuloBoletimUrna::HistoricoVotoImpresso, comum::md::CHistoricoVotoImpresso>
//       vtable @1597396: [0] 174 [1] 144 [2] 10276 DoConverte [3] 10275 default DoDesconverte (throws 7656,
//       "Método DoDesconverte() não implementado para ..." - the BU is write-only). sizeof 4.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/gravadores/md/chistoricovotoimpresso.h"   // md::CHistoricoVotoImpresso (20 bytes)   (name ?)
#include "ModuloBoletimUrna.h"

namespace comum::asn {

class CConversorHistoricoVotoImpresso
    : public IConversorASN<ModuloBoletimUrna::HistoricoVotoImpresso, md::CHistoricoVotoImpresso>
{
protected:
    TEntidade DoConverte(const TDado& historico) const override;   // wasm func 10276
};

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversornumviasimpressasrelatorios.h   (path inferred: RTTI only;
// included under this name by cconversorestadogeralvota.cpp / cconversorestadogeralgap.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloEstadoGeralDefs:
//   NumViasImpressasRelatorios ::= SEQUENCE { numViasEstadoUrna INTEGER (0..999), numViasEleitores INTEGER (0..999),
//                                             numViasVersoesDados INTEGER (0..999), numViasPU INTEGER (0..999) }
// Number of copies ("vias") already printed of four reports of the "Mais informações" menu (estado da urna,
// lista de eleitores, versões dos pacotes de dados, parâmetros de urna), kept in vota.bin / gap.bin so that the
// next print says "2ª via", "3ª via"...
// md::estadoaplicacao::CNumViasImpressasRelatorios: 4 x uint8 (+0..+3), constructor func 5629 (unit u29).
//
// RTTI: comum::asn::CConversorNumViasImpressasRelatorios : IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios,
//       md::estadoaplicacao::CNumViasImpressasRelatorios>, vtable @1569296: [2] 11386 DoConverte [3] 11387 DoDesconverte.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cnumviasimpressasrelatorios.h"
#include "ModuloEstadoGeralDefs.h"

namespace comum::asn {

class CConversorNumViasImpressasRelatorios
    : public IConversorASN<ModuloEstadoGeralDefs::NumViasImpressasRelatorios,
                           md::estadoaplicacao::CNumViasImpressasRelatorios>
{
protected:
    TEntidade DoConverte(const TDado& vias) const override;       // wasm func 11386
    TDado DoDesconverte(const TEntidade& vias) const override;    // wasm func 11387
};

} // namespace comum::asn

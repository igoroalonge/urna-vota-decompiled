// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.h   (path inferred: RTTI only; the
// header is included under this name by cconversordadolocal.cpp, unit u21)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloEstadoGeralUrna: DadoSecao ::= SEQUENCE { municipio INTEGER (1..99999), zona INTEGER (1..9999),
//                                                 secao INTEGER (0..9999) }
// md::estadoaplicacao::CLocalidadeEleitoral (8 bytes): +0 uint32 município, +4 uint16 zona, +6 uint16 seção.
// Used inside eg.bin (DadoLocal.secaoCarga, DadoCorrespondencia.secaoCarga) and gap.bin.
//
// RTTI: comum::asn::CConversorLocalidadeEleitoral
//         : IConversorASN<ModuloEstadoGeralUrna::DadoSecao, md::estadoaplicacao::CLocalidadeEleitoral>
//       vtable @1569252: [0] 174 [1] 144 [2] 11388 DoConverte [3] 11389 DoDesconverte (unit u05). sizeof 4.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/clocalidadeeleitoral.h"
#include "ModuloEstadoGeralUrna.h"

namespace comum::asn {

class CConversorLocalidadeEleitoral
    : public IConversorASN<ModuloEstadoGeralUrna::DadoSecao, md::estadoaplicacao::CLocalidadeEleitoral>
{
protected:
    TEntidade DoConverte(const TDado& localidade) const override;   // wasm func 11388
    TDado DoDesconverte(const TEntidade& secao) const override;     // wasm func 11389 (u05-foreign-fragments.cpp)
};

} // namespace comum::asn

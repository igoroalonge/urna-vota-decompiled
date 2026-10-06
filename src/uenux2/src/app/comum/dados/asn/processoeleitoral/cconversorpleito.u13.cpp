// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversorpleito.cpp  --  FRAGMENT written by unit u13
// (path inferred: TSE naming convention + its only user, CConversorProcessoEleitoral, lives in this directory)
//
// RTTI: comum::asn::CConversorPleito : IConversorASN<ModuloProcessoEleitoral::Pleito, md::CPleitoDTO>
// vtable @1570340: [0] 174 trivial dtor [1] 144 operator delete [2] 11364 (base DoConverte) [3] 11365 DoDesconverte
#include "comum/dados/asn/processoeleitoral/cconversorpleito.h"

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11365 (vtable slot 3) - name from the IConversorASN convention (slot 3 = DoDesconverte).
// Observed executing (the scenario's processo eleitoral is decoded at start-up).
// Pleito ::= SEQUENCE { id INTEGER (0..99999), nome GeneralString (SIZE(1..38)), data DataJE }
md::CPleitoDTO CConversorPleito::DoDesconverte(const ModuloProcessoEleitoral::Pleito& pleito) const
{
    return md::CPleitoDTO(pleito.get_id(), pleito.get_nome(), Utils::DesconverteDataJE(pleito.get_data()));   // func 2276
}

} // namespace comum::asn

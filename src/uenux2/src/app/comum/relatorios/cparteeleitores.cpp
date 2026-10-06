// uenux2/src/app/comum/relatorios/cparteeleitores.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35: deleting destructor and Imprime).
#include "comum/relatorios/cparteeleitores.h"

#include "comum/dados/celeitores.h"

namespace comum {

// wasm func 11204 - vtable slot 1 (deleting destructor): api_f6026(this, vtable @1576596) = vptr, release the
// shared_ptr at +4/+8, operator delete(this). (api_f6026 is a merged body shared with api::CFormPart.)
// The complete destructor (slot 0, func 11205) is "= default" in u24-foreign-fragments.cpp.

// wasm func 11206 - vtable slot 2. Not observed executing.
// NOTE: it walks the roll with the SHARED cursor of the CEleitores singleton (First/Next = CDataMap +16); after the
// loop the "current voter" is end(): whatever voter was current before printing is lost.
void CParteEleitores::Imprime() const
{
    CEleitores& eleitores = CEleitores::GetInst();                // func 326
    for (eleitores.First(); !eleitores.IsEnd(); eleitores.Next())  // CDataMap<CEleitorIdentidade, CEleitorDetalhe>::Next (5601)
        m_parteEleitor->Imprime();                                 // IReportPart slot 2
}

} // namespace comum

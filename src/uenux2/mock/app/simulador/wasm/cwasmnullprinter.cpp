// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmnullprinter.cpp
//
// simulador::CWasmNullPrinter : api::IImpressoraRelatorios : api::IImpressora - the report printer
// ("impressora de relatórios", the urna's thermal printer) of the web build: it prints nothing.
// RTTI typeinfo @1530712; vtable @1530652 (15 slots; the destructor is slot 10/11 in this interface):
//   [0] ret 0 (ICF 340)  [1] nop(x) (ICF 425)  [2] nop (218)  [3] nop (218)
//   [4] IImpressoraRelatorios::SetStyle (10881)   [5] GetColumns (10880)
//   [6] IImpressora::DisableSensors (8360, throws "Not supported")  [7] IImpressora::EnableSensors (8341)
//   [8] nop  [9] nop  [10] ~ (ICF 174)  [11] deleting (ICF 144)  [12] DoSetStyle (8365)  [13] ret 0  [14] nop(x,y,z)
// Registered as api::IImpressoraRelatorios by func 8302 (8-byte object: vptr + m_estilo = 0).
#include "api/hwil/iimpressorarelatorios.h"

namespace simulador {

class CWasmNullPrinter : public api::IImpressoraRelatorios {
public:
    int GetColumns() const override;                     // slot 5  (10880)
protected:
    void DoSetStyle(api::ELPStyle estilo) override;      // slot 12 (8365)
};

// wasm func 10880 - slot 5 (name from u17): style 3 (double width) -> 19 columns, otherwise 38.
int CWasmNullPrinter::GetColumns() const
{
    return m_estilo == static_cast<api::ELPStyle>(3) ? 19 : 38;
}

// wasm func 8365 - slot 12 (name from u17): only remembers the style (m_estilo, +4).
void CWasmNullPrinter::DoSetStyle(api::ELPStyle estilo)
{
    m_estilo = estilo;
}

}  // namespace simulador

// Reconstructed from vota_web_wasm.wasm (unit u17) - partial.
// Original: uenux2/src/api/hwil/iimpressorarelatorios.cpp (srcloc iimpressorarelatorios.cpp:22).
//
// IImpressoraRelatorios = the report printer (IImpressora + text styles). Used through
// simulador::CWasmNullPrinter in the web build (see iimpressora.h for the vtable).
#include "api/hwil/iimpressora.h"

namespace api {

enum class ELPStyle : int { NENHUM = 0 /* 1, 2, 3 = double width (19 columns) ... */ };   // names inferred

class IImpressoraRelatorios : public IImpressora {
public:
    virtual void SetStyle(ELPStyle estilo);            // slot 4
    virtual int GetColumns() const = 0;                // slot 5
protected:
    virtual void DoSetStyle(ELPStyle estilo) = 0;      // slot 12
    ELPStyle m_estilo{};                               // +4
};

// wasm func 10881 - iimpressorarelatorios.cpp:22
void IImpressoraRelatorios::SetStyle(ELPStyle estilo)
{
    if (estilo == ELPStyle::NENHUM)
        throw CUeCodedPrinterError(EUePrinterError::ESTILO_INVALIDO, "Invalid style", "no style",
                                   /*codigoImpressora*/ 2);
    if (m_estilo != estilo) {
        DoSetStyle(estilo);
        m_estilo = estilo;
    }
}

}  // namespace api

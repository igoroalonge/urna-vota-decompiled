// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasminit.cpp
//
// simulador::CWasmInit : comum::IInterfaceInit - the web stand-in for the urna's "init" service, the process
// that owns the hardware (power, boot device, result-media (MR, "mídia de resultado") USB port, the internal
// / external flash, demonstration mode). The application sends numbered commands and reads a vector<int>
// answer whose first element is the status (see comum/iinterfaceinit.h, unit u23). CWasmInit answers from
// a fixed table and logs every command as "CWasmInit::<cmd> <answer>" into the simulator's web log.
//
// RTTI typeinfo @1528804; vtable @1528772:
//   [0] IInterfaceInit::~ (5898)  [1] deleting (9388)  [2] CurrentBootDevice -> 1 (ICF 434)  [3] -> true (ICF 434)
//   [4] -> m_parametro (9418)     [5] EnviarMensagem (9405)  [6] -> false (ICF 340)  [7] no-op (ICF 425)
// Built by func 8302 as CWasmInit(10) (28 bytes: IInterfaceInit members +4..+23, then +24 int).
#include <cstdint>
#include <string>
#include <vector>

#include "comum/iinterfaceinit.h"
#include "simulador/wasm/cwasmutil.h"   // LogComando (func 1524, u29): CWasmLogBus channel INIT, "CWasmInit::{} {}"

namespace simulador {

class CWasmInit : public comum::IInterfaceInit {
public:
    explicit CWasmInit(int parametro) : m_parametro(parametro) {}   // inlined into 8302 (parametro = 10)
    ~CWasmInit() override = default;                                 // slot 1: func 9388

    comum::EBootDeviceId CurrentBootDevice() override { return comum::EBootDeviceId{1}; }   // slot 2 (ICF 434)
    bool Slot3() override { return true; }                                                  // slot 3 (ICF 434) ?
    int Slot4() override;                                                                   // slot 4 (9418)  ?
    std::vector<int> EnviarMensagem(ueint32 comando) override;                              // slot 5 (9405)
    bool Slot6() override { return false; }                                                 // slot 6 (ICF 340) ?
    void Slot7(int) override {}                                                             // slot 7 (ICF 425) ?

private:
    int m_parametro;   // +24
};

// wasm func 9418 - slot 4 (?): returns the constructor argument (10).
int CWasmInit::Slot4()
{
    return m_parametro;
}

// wasm func 9405 - slot 5. Observed executing. Command numbers from comum::IInterfaceInit (u23).
std::vector<int> CWasmInit::EnviarMensagem(ueint32 comando)
{
    const int cmd = static_cast<int>(comando & 0xFFFF);
    int resposta = 0;
    switch (cmd) {
    case 6:                                   // ?  (external flash / memory card query)
        LogComando(6, "= FE_NOT_PRESENT");
        resposta = 1;
        break;
    case 7:
        LogComando(7, "");
        break;
    case 10:                                  // CMD_HABILITA_MR             -> OK
    case 11:
    case 34:                                  // CMD_MONTA_MR                -> OK
    case 36:
    case 47:
    case 48:                                  // CMD_DESLIGA (power off)     -> OK, nothing happens
    case 58:                                  // CMD_DESMONTA_MR             -> OK
    case 68:
        LogComando(cmd, "");
        break;
    case 15:
    case 16:
        LogComando(cmd, "= no");
        resposta = 1;
        break;
    case 17:                                  // CMD_MR_MONTADO: 1 = no (ConsultaFlag tests == 0)
    case 18:                                  // CMD_MR_PRESENTE: 1 = no MR in the port
        LogComando(cmd, "= no");
        resposta = 1;
        break;
    case 39:                                  // ?  a size / free space
        LogComando(39, "= 512000");
        resposta = 512000;
        break;
    case 67:                                  // CMD_MODO_DEMONSTRACAO: 0 = not a demonstration urna
        LogComando(67, "= 0");
        break;
    default:                                  // includes 12 (MR device index -> 0) and 37 (boot device -> 0)
        LogComando(cmd, "= default");
        break;
    }
    return {resposta};
}

}  // namespace simulador

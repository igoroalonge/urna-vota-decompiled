// uenux2/src/app/comum/iinterfaceinit.h
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// IInterfaceInit = the application's client of the urna's "init" service (the process that owns the hardware:
// power, boot device, result-media (MR) USB port, demonstration flag). Every request is one numeric command
// sent through the pure virtual EnviarMensagem (slot 5); the answer is a vector of ints whose first element is
// the status (0 = OK, -1 = failure, or a value).
//
// RTTI: comum::IInterfaceInit (vtable @1552160)
//         └ simulador::CWasmInit (vtable @1528772, web build; commands answered by func 9405 and logged
//           as "CWasmInit::{} {}")
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "comum/comumdefs.h"

namespace comum {

enum class EBootDeviceId : int { DESCONHECIDO = 0, DISPOSITIVO_1 = 1, DISPOSITIVO_2 = 2 };   // names inferred

// Commands seen in this unit (numbers from the calls; names inferred from the error texts):
enum ECmdInit : ueint32 {
    CMD_HABILITA_MR       = 10,   // EnviarMensagemThrowVoid(10, "habilitando MR")
    CMD_DISPOSITIVO_MR    = 12,   // -> index of the MR block device (1 = sda, ...), -1 = failure
    CMD_MONTA_MR          = 34,   // "montar MR sem habilitar"
    CMD_BOOT_DEVICE       = 37,
    CMD_MR_MONTADO        = 17,   // IsMRMontadoSemHabilitar
    CMD_MR_PRESENTE       = 18,   // IsMRPresenteSemHabilitar
    CMD_DESLIGA           = 48,
    CMD_DESMONTA_MR       = 58,
    CMD_MODO_DEMONSTRACAO = 67,
};

class IInterfaceInit {
public:
    virtual ~IInterfaceInit();                                     // slot 0 func 5898 (other unit), 1 deleting
    virtual EBootDeviceId CurrentBootDevice();                     // slot 2 func 11646 (srcloc :47)
    virtual bool Slot3() = 0;                                      // slot 3 (CWasmInit: return 1)   ?
    virtual int Slot4() = 0;                                       // slot 4 (CWasmInit: return +24) ?
    virtual std::vector<int> EnviarMensagem(ueint32 comando) = 0;  // slot 5 (CWasmInit func 9405)
    virtual bool Slot6() = 0;                                      // slot 6 (CWasmInit: return 0)   ?
    virtual void Slot7() = 0;                                      // slot 7 (CWasmInit: no-op)      ?

    bool IsMRPresenteSemHabilitar();                               // func 2862 (:137)
    bool IsMRMontadoSemHabilitar();                                // func 5897 (:148)
    void MontarMRSemHabilitar();                                   // func 5896 (:159, :163, :172)
    void DesmontarMRSemDesabilitar();                              // func 3832 (:184, :188)
    void DesligarUrna();                                           // func 5894 (:522, :527, :531)
    int DispositivoMR();                                           // inlined into 5896 (:632, :636)
    void EnviarMensagemThrowVoid(int comando, const std::string& descricao);       // func 3833 (:731, :737); comando
                                                                                   // is formatted as int (fmt arg type 3)
    bool GetDemoMode();                                            // func 729 (:753, :757)
    std::string GetSerialNumberMR(int dispositivo);               // inlined into 5896 (:855)

private:
    bool ConsultaFlag(const std::source_location& local, int codigoErro, ueint32 comando);   // func 6046 (name inferred)

    std::shared_ptr<bool> m_demoMode;    // +4 / +8   cached answer of command 67
    std::string m_serialMR;              // +12       serial of the last MR mounted
};

}  // namespace comum

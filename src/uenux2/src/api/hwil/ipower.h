// uenux2/src/api/hwil/ipower.h
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::IPower: the urna's power-supply / battery interface (HWIL = hardware interface layer). Obtained
// with CPolySingletonList::instance<IPower>() by the status bar (CFormBuilder / CPowerInformation battery
// icon), the automatic shut-down states (CEstadoComDesligamentoAutomatico, CExibeAlertaDesligamento) and
// comum::util::CMonitoraAlimentacao. In the web build the only implementation is api::teste::CPowerMock
// (typeinfo @1531020, vtable @1530952), which never reports "running on the internal battery".
//
// Vtable order (from CPowerMock, the only vtable): slots 0..7 are overridden by the mock, 8..12 are the
// IPower default bodies below (the mock does not override them), then the destructor (13/14) and two more
// methods (15, 16). Slot names 0..7, 15, 16 are inferred from other units (u06, u09, u15).
//
// The five default bodies (the only IPower code in the binary) are for the internal battery ("BatInt")
// features of newer urna models: maximum state of charge ("SOC") and the battery storage mode. The base
// class refuses them: api::EUeHwilError (CBaseError limits {5150, 5950}), message "Not supported".
// None is called in the recorded sessions.
#pragma once

#include <tuple>

#include "api/hwil/euhwilerror.h"      // api::EUeHwilError, api::CUeHwilError   (paths inferred)

namespace api {

class IPower {
public:
    // Slots 0..7 (pure here; api::teste::CPowerMock 8128/8120/8116/8113/8111/8109/8108/8107). Names inferred:
    virtual bool   EhAlimentacaoExterna() = 0;        // slot 0: refresh, then !(status byte +5 & 0x20)   ?
    virtual bool   vf1() = 0;                         // slot 1 (8120)                                     ?
    virtual int    GetCorrenteBateria() = 0;          // slot 2: signed 16-bit value at +16                ?
    virtual int    GetTensaoBateria() = 0;            // slot 3: 16-bit value at +12                       ?
    virtual int    GetTensaoCarregador() = 0;         // slot 4: 16-bit value at +14                       ?
    virtual int    GetCargaEstimada() = 0;            // slot 5: % from a voltage table                    ?
    virtual int    GetTemperatura() = 0;              // slot 6: byte at +18                               ?
    virtual int    GetPercentualBateria() = 0;        // slot 7: clamped to 100 (u15: "{: >3}%")

    // slot 8 - wasm func 8105 (srcloc line 354)
    virtual float BatIntMaxSocValue()
    {
        throw CUeHwilError(EUeHwilError(5172), "Not supported");                     // line 354
    }

    // slot 9 - wasm func 8098 (srcloc line 361)
    virtual void SetBatIntMaxSocValue(float /*valor*/)
    {
        throw CUeHwilError(EUeHwilError(5173), "Not supported");                     // line 361
    }

    // slot 10 - wasm func 8091 (srcloc line 409): {storage mode active?, SOC target}
    virtual std::tuple<bool, float> BatIntStorageMode()
    {
        throw CUeHwilError(EUeHwilError(5174), "Not supported");                     // line 409
    }

    // slot 11 - wasm func 8083 (srcloc line 417)
    virtual void EnableBatIntStorageMode(float /*soc*/)
    {
        throw CUeHwilError(EUeHwilError(5175), "Not supported");                     // line 417
    }

    // slot 12 - wasm func 8074 (srcloc line 424)
    virtual void DisableBatIntStorageMode()
    {
        throw CUeHwilError(EUeHwilError(5176), "Not supported");                     // line 424
    }

    virtual ~IPower() = default;                        // slots 13/14 (174 / 144: trivial)

    // slot 15: refreshes the cached status word at +4 (CPowerMock 8071; u06/u09: `(status & 6) == 2` means
    // "running on the internal battery"). slot 16: CPowerMock returns 1 (ICF 434).        names inferred
    virtual void AtualizaStatus(struct SStatusEnergia& status) = 0;
    virtual bool vf16() = 0;
};

} // namespace api

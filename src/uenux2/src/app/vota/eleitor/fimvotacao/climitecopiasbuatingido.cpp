// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/climitecopiasbuatingido.cpp
//
// "Limite de cópias do BU atingido" = shown at the end of the day when the maximum number of BU
// copies (mandatory + additional) was already printed; CONFIRMA goes to the on-screen QR code of the BU.
//
// srcloc evidence:
//   :37  StartState  Assert (comum::CAppInfo::GetInst().GetVota().GetEstadoVota() == EAVENCERRADA) (3473)
//   api/gui/cinteractiveform.h:57  Read(), inlined in ProcessInput
//
// RTTI: comum::CAppState <- vota::CLimiteCopiasBUAtingido (typeinfo @1542888, vtable @1542836)
//   [0] 244 dtor (ICF) [1] 387 deleting (ICF) [2] StartState 12027 [7] ProcessInput 12026

#include "vota/eleitor/fimvotacao/climitecopiasbuatingido.h"

#include "comum/cappinfo.h"
#include "vota/eleitor/fimvotacao/cmostraqrcodebu.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

// wasm func 12027 — vtable slot 2
void CLimiteCopiasBUAtingido::StartState()
{
    UE_ASSERT(comum::CAppInfo::GetInst().GetVota().GetEstadoVota() == EEstadoVota::EAVENCERRADA);   // :37
    CLogVota::GetInst().LogaQtdViasExcedeMaximo();   // func 4556 "Quantidade de vias adicionais excede o máximo permitido"
    m_tela->Exibe();
    m_proximoEstado = this;
}

// wasm func 12026 — vtable slot 7 (analyzer name vota::CLimiteCopiasBUAtingido::vf7)
void CLimiteCopiasBUAtingido::ProcessInput()
{
    if (m_tela->Read() == api::EInputResult::Confirma)                    // cinteractiveform.h:57
        m_proximoEstado = &CMostraQRCodeBU::GetInst();                    // func 2888
}

}  // namespace vota

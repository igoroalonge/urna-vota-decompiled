// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u27): uenux2/src/app/vota/operador/justificativa/cjustificativaefetuada.cpp
// Constructor/GetInst (2747): operador/u27-foreign-fragments.cpp. ProcessInput (10593): u19-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/justificativa/cjustificativaefetuada.h"

#include "vota/log/clogvota.h"

namespace vota {

// wasm func 10594 - vtable slot 2. The texts are Latin-1 literals ("AUSÊNCIA JUSTIFICADA" @335297,
// "JÁ JUSTIFICOU" @322214); the log records go through api::CLoga::loga inline (not the api_f233 copy).
void CJustificativaEfetuada::StartState()
{
    if (m_novaJustificativa) {
        *m_texto = "AUSÊNCIA JUSTIFICADA";                                            // ecourna_f276 = string::assign
        CLogVota::GetInst().Loga(api::ESeveridade{1}, "Justificativa recebida");
    } else {
        *m_texto = "JÁ JUSTIFICOU";
        CLogVota::GetInst().Loga(api::ESeveridade{2}, "Eleitor já justificou");       // warning
    }
    m_form->Show();                                                                   // +20
    m_proximoEstado = this;
}

}  // namespace vota

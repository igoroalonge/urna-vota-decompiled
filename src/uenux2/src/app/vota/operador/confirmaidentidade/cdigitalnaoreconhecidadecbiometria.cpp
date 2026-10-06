// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred by u10): uenux2/src/app/vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.cpp
// Constructor: operador/u10-foreign-fragments.cpp. ProcessInput (10477): operador/u19-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/confirmaidentidade/cdigitalnaoreconhecidadecbiometria.h"

#include <format>

#include "comum/appinfo/cappinfo.h"          // comum::EhTreinamentoSemTreinamentoEleitor (func 1823)
#include "comum/dados/celeitores.h"
#include "vota/log/clogvota.h"

namespace vota {

// wasm func 10478 - vtable slot 2
void CDigitalNaoReconhecidaDecBiometria::StartState()
{
    m_proximoEstado = this;
    m_form->Show();

    // In a training urna without "treinamento de eleitor" the roll has no real biometrics: nothing to report.
    if (comum::EhTreinamentoSemTreinamentoEleitor())                                            // func 1823
        return;

    // vota_f1393 = CEleitores::GetCurrent().GetBiometria(), returned BY VALUE: the whole CBiometriaEleitor
    // (optional vector + optional map of fingers) is copied twice just to read the int at +36.
    const auto& eleitores = comum::CEleitores::GetInst();                                       // func 326
    if (eleitores.GetCurrent().GetBiometria().GetEstadoDecifracao() <= 0)
        return;

    // The code is an enum formatted through a TSE std::formatter specialisation (format-arg type 15 =
    // handle, formatter body = shared libc++ lambda func 536 via table slot 4091: printed as an integer).
    CLogVota::GetInst().LogaErro(std::format("Erro ao decifrar a biometria do eleitor - Código ({})",   // func 2282 (severity 3)
                                             eleitores.GetCurrent().GetBiometria().GetEstadoDecifracao()));
}

}  // namespace vota

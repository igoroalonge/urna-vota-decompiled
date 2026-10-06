// uenux2/src/app/comum/comparecimentomesario/estados/cencerraregistromesarios.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :62
// (GetControlador).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/pattern/cpolysingletonlist.h"

namespace comum {

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :62
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
} // namespace

// wasm func 5388 (other unit; tools: api_f5388). 28 bytes, flags 0, both form pointers stay empty.
CEncerraRegistroMesarios& CEncerraRegistroMesarios::GetInst()
{
    static std::mutex mutex;                                        // @1909488
    static std::unique_ptr<CEncerraRegistroMesarios> s_inst;        // @1909512
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CEncerraRegistroMesarios());
    return *s_inst;
}

// wasm func 10380 - vtable slot 2 (tools: "CEncerraRegistroMesarios::GetControlador").
// Leaves the registration and tells VOTA where to go next.
void CEncerraRegistroMesarios::StartState()
{
    m_proximoEstado = this;
    switch (GetControlador().GetPeriodoRegistro()) {                              // slot 6
    case EPeriodoRegistro::NENHUM:
    case EPeriodoRegistro::INICIAL:
        m_proximoEstado = GetControlador().GetEstadoAposRegistroInicial();        // slot 9 (VOTA: CAguardaInicio)
        GetControlador().LogaEncerrouRegistro();                                  // slot 29
        GetControlador().NotificaFimRegistroInicial();                            // slot 10 (VOTA: msg 11 to the voter thread)
        break;
    case EPeriodoRegistro::VOTACAO:
        m_proximoEstado = GetControlador().GetEstadoAposRegistroVotacao();        // slot 11 (VOTA: CRegistroMesarioEncerrado)
        GetControlador().LogaEncerrouRegistro();
        break;
    case EPeriodoRegistro::FINAL:
        m_proximoEstado = GetControlador().GetEstadoAposRegistroFinal();          // slot 12 (VOTA: CFinalizaOperador)
        GetControlador().LogaEncerrouRegistro();
        GetControlador().NotificaFimRegistroFinal();                              // slot 15 (VOTA: msg 7 -> BU generation)
        break;
    default:
        break;
    }
}

} // namespace comum

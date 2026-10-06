// uenux2/src/app/comum/comparecimentomesario/estados/cgestordadomesario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :35
// (StartState). The three IGestorDadoMesario singletons are created here (constructors inlined).
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/pattern/cpolysingletonlist.h"

namespace comum {

// wasm func 5383 (other unit): vota_f764(mutex @1909824, &s_inst @1909848, vtable @1595176, flags 0)
CGestorDadoMesario& CGestorDadoMesario::GetInst()
{
    static std::mutex mutex;
    static std::unique_ptr<CGestorDadoMesario> s_inst;
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CGestorDadoMesario());
    return *s_inst;
}

// The three per-period singletons (12 bytes, flags 0; inlined into StartState below).
CGestorDadoMesarioInicial& CGestorDadoMesarioInicial::GetInst()
{
    static std::mutex mutex;                                        // @1909740
    static std::unique_ptr<CGestorDadoMesarioInicial> s_inst;       // @1909764
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CGestorDadoMesarioInicial());
    return *s_inst;
}
CGestorDadoMesarioVotacao& CGestorDadoMesarioVotacao::GetInst()
{
    static std::mutex mutex;                                        // @1909768
    static std::unique_ptr<CGestorDadoMesarioVotacao> s_inst;       // @1909792
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CGestorDadoMesarioVotacao());
    return *s_inst;
}
CGestorDadoMesarioFinal& CGestorDadoMesarioFinal::GetInst()
{
    static std::mutex mutex;                                        // @1909796
    static std::unique_ptr<CGestorDadoMesarioFinal> s_inst;         // @1909820
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CGestorDadoMesarioFinal());
    return *s_inst;
}

// wasm func 10327 - vtable slot 2 (srcloc :35)
void CGestorDadoMesario::StartState()
{
    m_proximoEstado = this;
    switch (api::CPolySingletonList::instance<IControladorRegistraMesarios>().GetPeriodoRegistro()) {   // :35, slot 6
    case EPeriodoRegistro::INICIAL:
        m_proximoEstado = &CGestorDadoMesarioInicial::GetInst();
        break;
    case EPeriodoRegistro::VOTACAO:
        m_proximoEstado = &CGestorDadoMesarioVotacao::GetInst();
        break;
    case EPeriodoRegistro::FINAL:
        m_proximoEstado = &CGestorDadoMesarioFinal::GetInst();
        break;
    default:
        break;   // NENHUM: stays here (nothing drawn)
    }
}

} // namespace comum

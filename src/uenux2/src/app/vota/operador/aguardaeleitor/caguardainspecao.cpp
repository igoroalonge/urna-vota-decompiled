// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/operador/aguardaeleitor/caguardainspecao.cpp
//
// Also here: the constructor and GetInst of CConfirmaInspecionada, which exist only inlined into
// CAguardaInspecao::ProcessMessage (func 10530). They belong to cconfirmainspecionada.cpp (path inferred;
// its ProcessInput, func 10527, and its declaration are in unit u34's fragments cconfirmainspecionada.u34.cpp /
// estadosoperador.u34.h).
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/aguardaeleitor/caguardainspecao.h"

#include <memory>
#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "vota/operador/estadosoperador.u34.h"          // CConfirmaInspecionada (declared by unit u34)

namespace vota {

using api::SPoint;

namespace {
constexpr auto DIREITA = api::ETextAlignment(1);
constexpr auto CENTRO  = api::ETextAlignment(2);

// Message 11 of the operator queue: posted by CInspecionaUrna::ProcessInput (func 11797) after
// "Inspeção da urna confirmada" on the voter keypad.                                  name inferred
constexpr uebyte MSG_INSPECAO_CONFIRMADA = 11;
}  // namespace

// ---------------------------------------------------------------------------------------------------------
// wasm func 10530 - vtable slot 6
void CAguardaInspecao::ProcessMessage(uebyte mensagem)
{
    if (mensagem == MSG_INSPECAO_CONFIRMADA)
        m_proximoEstado = &CConfirmaInspecionada::GetInst();    // GetInst + constructor inlined (below)
}

// =========================================================================================================
// cconfirmainspecionada.cpp (path inferred) - inlined into func 10530.
// 20 bytes, vtable @1590788: CAppState(2 = keys), +12 interactive MT form.
// The positions are packed SPoint constants (0x00010021 = {33,1}, 0x00020014 = {20,2}, 0x00040028 = {40,4})
// that the annotator shows as bogus string pointers.
CConfirmaInspecionada::CConfirmaInspecionada()
    : comum::CAppState(2)                                                           // shared_f224
{
    api::CFormBuilderMT campos;                                                     // string() + vector
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});                                  // func 728
    campos.Add<api::CTextFieldMT>(SPoint{20, 2},
                                  std::make_shared<api::CFixedText>(CENTRO, "Inspeção completa"));              // func 180
    campos.Add<api::CTextFieldMT>(SPoint{40, 4},
                                  std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: continuar a votação"));
    campos.AddInputControl();                                                       // func 395
    m_form = campos.CriaFormInterativo("", true);                                   // shared_f301 -> +12
}

// Lazy singleton. The two statics have no guard variable, so they are static data members (or
// namespace-scope objects) of cconfirmainspecionada.cpp, not function-local statics (see the note at the top
// of src/uenux2/src/app/vota/u38-foreign-fragments.cpp); only the no-op unlock stub (func 150) of the mutex
// survives in the code. (Add `static std::mutex s_mutex; static std::unique_ptr<CConfirmaInspecionada>
// s_instancia;` and the private constructor to u34's declaration when merging.)
std::mutex CConfirmaInspecionada::s_mutex;                                  // @1908556
std::unique_ptr<CConfirmaInspecionada> CConfirmaInspecionada::s_instancia;  // @1908580

CConfirmaInspecionada& CConfirmaInspecionada::GetInst()
{
    std::lock_guard trava(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CConfirmaInspecionada());
    return *s_instancia;
}

}  // namespace vota

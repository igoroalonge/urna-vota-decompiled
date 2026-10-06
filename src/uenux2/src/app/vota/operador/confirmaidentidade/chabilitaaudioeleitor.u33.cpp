// uenux2/src/app/vota/operador/confirmaidentidade/chabilitaaudioeleitor.cpp  -- FRAGMENT written by unit u33
// (path inferred by unit u19, which reconstructs ProcessInput = func 10523 in operador/u19-foreign-fragments.cpp).
//
// vota::CHabilitaAudioEleitor : comum::CAppState   typeinfo @1590896, vtable @1590860, 20 bytes
//   [2] StartState 10524   [7] ProcessInput 10523   (others: CAppState defaults)
// Microterminal state shown to the poll worker (mesário) when the voter being released needs audio
// (registration flag "necessidade especial" or audio switched on by hand): the mesário is asked to plug the
// headphones in, and CONFIRMA releases the voter with audio on (messages 8/9 then 1 to the voter thread).
// Dead code in the web build: the operator thread never runs there (unit u10 §2).
#include <memory>
#include <mutex>
#include <string>

#include "api/gui/cformbuildermt.h"
#include "comum/cappstate.h"
#include "vota/operador/confirmaidentidade/chabilitaaudioeleitor.h"

namespace vota {

// Constructor - inlined into GetInst (func 2743). Form of the 4 x 40 MT LCD (SPoint{coluna, linha}, 1-based;
// alignment 0 left, 1 right). The texts are Latin-1 literals @131505, @226277, @90975.
CHabilitaAudioEleitor::CHabilitaAudioEleitor()
    : comum::CAppState(2)                                       // shared_f224: accepts keys
{
    api::CFormBuilderMT campos;                                 // string() + vector
    campos.Add<api::CBuzzFieldMT>(51, 10);                      // func 941
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});              // func 728: hh:mm:ss top right
    campos.Add<api::CTextFieldMT>(SPoint{1, 2},
        std::make_shared<api::CFixedText>(ESQUERDA, "Este eleitor necessita de áudio"));    // func 180
    campos.Add<api::CTextFieldMT>(SPoint{1, 3},
        std::make_shared<api::CFixedText>(ESQUERDA, "Coloque o fone de ouvido na urna"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 4},                 // {1,4} (0x00040001) with alignment 1, as compiled.
        std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA: continuar"));   // Right alignment ignores x on the
                                                                // MT (CWasmScreenMT::Write, func 8799: column 40 - len),
                                                                // so this ends at column 40 like the other "CONFIRMA:" lines
    campos.AddInputControl();                                   // func 395
    m_form = campos.CriaFormInterativo("", true);               // shared_f301 -> +12
}

// wasm func 2743                                                                    // name inferred
// Lazy singleton (static unique_ptr @1908608; the static mutex @1908584 left only its unlock stub).
// Callers: CNomeEleitor::HabilitaEleitorSemBiometria (5400), CDigitalReconhecida::ProcessInput (10481),
// CControlaReconhecimento::ProcessInput (10511) and the "release the voter" body 3883, always right after
// IInformacaoThreadOperador::GetInst().DeveHabilitarAudio() (func 2746) returned true.
CHabilitaAudioEleitor& CHabilitaAudioEleitor::GetInst()
{
    static std::mutex mutex;                                    // @1908584
    static std::unique_ptr<CHabilitaAudioEleitor> s_inst;       // @1908608
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CHabilitaAudioEleitor());              // 20-byte object
    return *s_inst;
}

} // namespace vota

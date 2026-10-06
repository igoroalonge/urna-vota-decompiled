// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cledfieldmt.cpp (srcloc cledfieldmt.cpp:46). Merge into it.
//
// "MT" = microterminal, the poll worker's (mesário's) keypad with a small LCD and LEDs. CLedFieldMT is a
// form field of the microterminal forms that only drives an LED.
#include <format>

#include "api/gui/gui-common.u15.h"

namespace api {

class IScreenMT;   // slots 8..11: the four LED operations

// Operation codes 0..3 -> IScreenMT vtable slots 8..11. In the web mock (simulador::CWasmScreenMT)
// slots 8 and 11 set the LED byte (+52) to 0 and slots 9 and 10 set it to 1, then repaint (slot 14).
enum class ELedOperacao : int { Op0 = 0, Op1 = 1, Op2 = 2, Op3 = 3 };   // real names unknown   ?

// CLedFieldMT (typeinfo @1579740, vtable @1579692): +24 ELedOperacao m_operacao.
class CLedFieldMT : public IFormField<IScreenMT> {
public:
    void Draw(IScreenMT& tela) const override;
private:
    ELedOperacao m_operacao;   // +24
};

// wasm func 11050 (not observed) - srcloc cledfieldmt.cpp:46, slot 2
void CLedFieldMT::Draw(IScreenMT& tela) const
{
    const int op = static_cast<int>(m_operacao);
    if (op >= 4)   // unsigned comparison in the binary: negative values also throw
        throw CUeGuiError(static_cast<EUeGuiError>(4940), std::format("Operação desconhecida {}", op));
    switch (op) {                          // compiled as call_indirect(vtable[8 + op])
    case 0: tela.LedOperacao0(); break;    // slot 8
    case 1: tela.LedOperacao1(); break;    // slot 9
    case 2: tela.LedOperacao2(); break;    // slot 10
    case 3: tela.LedOperacao3(); break;    // slot 11
    }
}

} // namespace api

// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/gui/iscreen.h (srcloc iscreen.h:69, IScreen::GetFontMetrics).
//
// api::IScreen is the voter-screen device (implemented in the web build by simulador::CWasmScreen,
// which draws on an HTML canvas through js_* imports); api::IScreenMT is the poll worker's
// microterminal LCD (simulador::CWasmScreenMT). Both are obtained through
// CPolySingletonList::instance<IScreen / IScreenMT>() (see IForm::GetRenderForm).
//
// Slot names come from their callers in the api/gui widgets (units u07, u15, u17); "?" = guessed.
#pragma once

#include <string>

#include "api/gui/gui-common.u15.h"
#include "api/gui/iform.h"

namespace api {

class IText;
class CMovie;

// vtable @1529100 (39 slots). Implementations: simulador::CWasmScreen (vtable @1528824).
class IScreen {
public:
    // slot 0 - wasm func 5071 (not in unit): the destructor removes every voter form,
    //          IForm<IScreen>::RemoveAll() (func 5069). slot 1: deleting dtor (ICF 325).
    virtual ~IScreen() { IForm<IScreen>::RemoveAll(); }

    virtual void GetMaxCharSize(uebyte& largura, uebyte& altura, const SFont& fonte) = 0;   // slot 2 (?)

    // slot 3 - wasm func 8938. Default implementation: not implemented.   srcloc iscreen.h:69
    // (CWasmScreen overrides it: slot 3 -> func 9261.)
    virtual void GetFontMetrics(TPosition& largura, TPosition& altura, const SFont& fonte,
                                const std::string& texto)
    {
        (void)largura; (void)altura; (void)fonte; (void)texto;
        throw CUeGuiError(static_cast<EUeGuiError>(4982), "Método não implementado");
    }

    virtual void Clear(TColor cor) = 0;                                        // slot 4
    virtual void ClearRect(const SRect& area, TColor cor) = 0;                 // slot 5
    virtual void FillRect(const SRect& area, TColor cor) = 0;                  // slot 6
    // slots 7, 8 ?
    virtual void DrawRect(const SRect& area, TColor cor, int espessura) = 0;   // slot 9
    // slots 10..12 ?; slot 13 default no-op (ICF 218)
    // slots 14..18 ?
    virtual SRect WriteText(const SPoint& pos, const IText& texto, const SFont& fonte,
                            TColor corTexto, TColor corFundo) = 0;             // slot 19
    // slot 20 ?; slot 21 text clipped to a rectangle
    // slot 22 DrawImage(pos, image, anchor); slot 24 DrawImage(pos, image, clip, anchor)
    // slot 26 DrawMovie(pos, movie, anchor)
    virtual void Refresh() = 0;                                                // slot 27
    // slot 29 SetColors(texto, fundo) (?); slot 32 GetTextWidth(text, font)
    // slots 37, 38: default no-ops (ICF 218)
};

// vtable @1529856 (20 slots). Implementation: simulador::CWasmScreenMT.
class IScreenMT {
public:
    // slot 0 - wasm func 8741: the destructor removes every microterminal form
    //          (IForm<IScreenMT>::RemoveAll, func 3461). slot 1: deleting dtor (ICF 325).
    virtual ~IScreenMT() { IForm<IScreenMT>::RemoveAll(); }

    // slot 6: beep(freq?, times?) - called as (50, 3) by CSuspensaoAutomaticaEleitor (unit u10)
    virtual void Refresh() {}                                                  // slot 14, default no-op
};

}  // namespace api

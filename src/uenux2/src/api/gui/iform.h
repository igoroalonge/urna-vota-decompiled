// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/gui/iform.h (srclocs iform.h:142 FindFieldAs, iform.h:178 GetRenderForm).
//
// api::IForm<MEDIA> is the base of every screen ("formulário") of the urna: the voter screen
// (MEDIA = IScreen), the poll worker's microterminal (MEDIA = IScreenMT, 4 x 40 LCD) and printed
// reports (MEDIA = IPaper). A form is a list of fields (IFormField<MEDIA>) plus a "pre-show" hook.
// Forms are stacked: a static per-MEDIA stack holds the forms that are on the device, the top one is
// "active" (drawn, its timers running). Every change of the stack is published to a static
// observable (a snapshot of {form, lifetime token} pairs).
//
// Three instantiations exist, with identical code (only the static addresses and the MEDIA slot of
// Refresh differ):
//
//                       vtable     stack vector   stack mutex   observable  once_flag   Refresh slot
//   IForm<IScreen>      @1578040   @1832632       @1832604      @1529260    @1832852    IScreen 27
//   IForm<IScreenMT>    @1579944   @1832892       @1832864      @1529940    @1832860    IScreenMT 14
//   IForm<IPaper>       @1581100   @1833528       @1833500      @1581136    @1839228    (none)
//
// Derived: CInteractiveForm<IScreen, IInputKbd> (@1579176), CInteractiveForm<IScreenMT, IInputMT>
// (@1580288) - see cinteractiveform.u15.h - and IFormImpressao<IPaperRelatorios> (shares the IPaper
// slots). Constructor: shared body vota_f6024 (not in this unit), called through the thunks 5548
// (IScreen), 5522 (IScreenMT).
//
// Build notes (wasm, no threads): std::mutex::lock() compiles to nothing and unlock()/the condition
// variables' notify to the no-op stub func 150; the shared_mutex of FormControlBlock survives as
// funcs 3328/3327 (libc++ __shared_mutex_base::lock/unlock).
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <vector>

#include "api/gui/gui-common.u15.h"         // SPoint, SRect, CUeGuiError, IFormFieldBase protocol
#include "api/pattern/cpolysingletonlist.h"  // CPolySingletonList::instance<T>()

namespace api {

class IScreen;
class IScreenMT;
class IPaper;
template <class MEDIA> class IFormField;
template <class MEDIA> class IPreShow;       // slot 2: void PreShow(MEDIA&) - e.g. CPreShowClearMT
class CTextField;

// Lifetime token shared by a form and the snapshots published to observers (RTTI:
// std::__shared_ptr_emplace<api::FormControlBlock>, 128 bytes). An observer that holds a raw
// IForm* takes the shared lock and checks m_vivo before touching the form.
struct FormControlBlock {
    bool m_vivo = true;                 // +0   set to false by ~IForm under the exclusive lock
    std::shared_mutex m_mutex;          // +4   mutex(24) + gate1(48) + gate2(48) + state(4)
};

// One entry of the published stack snapshot (12 bytes).                                name inferred
template <class MEDIA>
struct SFormInfo {
    class IForm<MEDIA>* form;                       // +0
    std::shared_ptr<FormControlBlock> controle;     // +4/+8
};

// Minimal observable used for the stack snapshot (layout at @1529260 / @1529940 / @1581136).
// Only NotifyStackChanged touches it; nothing in this build registers an observer, so the
// notification loop never runs (see the doc).                                      name inferred
template <class T>
class CObservableValue {
public:
    struct IObserver { virtual ~IObserver() = default; virtual void Update(CObservableValue&) = 0; };  // slot 2
    std::vector<T> m_valor;               // +0
    std::vector<IObserver*> m_observers;  // +12
    std::mutex m_mutex;                   // +24
};

template <class MEDIA>
class IForm {
public:
    using TCampos = std::vector<std::shared_ptr<IFormField<MEDIA>>>;

    // Shared body vota_f6024 (not in this unit): copies the fields and the pre-show hook, creates
    // the FormControlBlock (m_vivo = true), leaves m_nome empty and calls SetForm(this) (field
    // slot 6) on every field.
    IForm(const TCampos& campos, const std::shared_ptr<IPreShow<MEDIA>>& preShow);

    // wasm func 2781 (IScreen) / 2776 (IScreenMT) / 5514 (IPaper) - slot 0
    // wasm func 11136 / 11045 / 5513 - slot 1, deleting destructor (dtor + operator delete)
    virtual ~IForm()
    {
        {
            std::unique_lock lock(m_controle->m_mutex);   // func 3328 (shared_mutex::lock)
            m_controle->m_vivo = false;
        }                                                  // func 3327 (shared_mutex::unlock)
        Remove(this);            // IScreen: out-of-line func 5987; IScreenMT/IPaper: inlined here
        for (auto& campo : m_campos)
            campo->SetForm(nullptr);                       // field slot 6
    }

    // slot 2 - wasm func 11135 (IScreen, observed executing) -> body 5541 (not in this unit)
    //                   func 11044 (IScreenMT)                 -> body 5520
    //                   func 11009 (IPaper, body inlined)
    // "Show": the form replaces the whole stack.                                    name inferred
    virtual void Show() { DoShow(this); }

    // slot 3 - wasm func 11134 (IScreen) -> 5540 (not in unit), 11043 (IScreenMT) -> 5519,
    //          11008 (IPaper, inlined). The form goes (or moves) to the top of the stack. name inferred
    virtual void ShowOnTop() { DoShowOnTop(this); }

    // slot 4 - wasm func 11133 (IScreen, observed executing) / 11042 (IScreenMT) / 11007 (IPaper)
    // Redraws only the fields that asked for it (field slot 5), then refreshes the device.
    // Fields call this from their Invalidate() idiom (see gui-common.u15.h).        name inferred
    virtual void Redraw()
    {
        std::lock_guard lock(m_mutex);
        if (m_ativo) {
            MEDIA& media = GetRenderForm();
            for (auto& campo : m_campos)
                campo->RedrawIfDirty(media);               // field slot 5
            GetRenderForm().Refresh();                     // IScreen slot 27 / IScreenMT slot 14;
        }                                                  // IPaper: GetRenderForm() only (no-op Refresh)
    }

    // slot 5 - wasm func 11132 (IScreen, observed) / 11041 (IScreenMT) / 11004 (IPaper: the whole
    // CPolySingletonList lookup is inlined there, see the doc).  srcloc iform.h:178
    virtual MEDIA& GetRenderForm() const
    {
        return CPolySingletonList::instance<MEDIA>();
    }

    // slot 6 - no-op in IForm (ICF body 425); CInteractiveForm moves the input focus.
    virtual void SetFocus(class IInputField<MEDIA>* /*campo*/) {}

    // slot 7 - wasm func 5539 (IScreen, observed) / 5518 (IScreenMT) / 11005 (IPaper)
    // Called when the form becomes the top of the stack: draw everything, then start the fields'
    // timers/animations (field slot 3).                                             name inferred
    virtual void OnActivate()
    {
        m_ativo = true;
        {
            std::lock_guard lock(m_mutex);
            if (m_ativo) {
                MEDIA& media = GetRenderForm();
                m_preShow->PreShow(media);                 // IPreShow slot 2
                for (auto& campo : m_campos)
                    campo->Draw(media);                    // field slot 2
                GetRenderForm().Refresh();                 // IScreen 27 / IScreenMT 14 / IPaper none
            }
        }
        for (auto& campo : m_campos)
            campo->Start();                                // field slot 3
    }

    // wasm func 1721 - srcloc iform.h:142, instantiation [MEDIA = IScreen, T = CTextField]
    // (caller: vota::CMostraQRCodeBU::AjustaTela, func 5982). Fields are matched by the unique name
    // given by CFormBuilder::Add (IFormFieldBase +12).
    template <class T>
    std::shared_ptr<T> FindFieldAs(const std::string& nome) const
    {
        for (const auto& campo : m_campos) {
            if (campo->GetNome() != nome)
                continue;
            if (auto tipado = std::dynamic_pointer_cast<T>(campo))
                return tipado;
            throw CUeGuiError(static_cast<EUeGuiError>(4978),
                              "Campo encontrado, mas não é do tipo solicitado: " + nome);   // iform.h:142
        }
        return nullptr;
    }

    // Removes every form (called by ~IScreen / ~IScreenMT, i.e. when the device goes away).
    // wasm func 5069 (IScreen, not in unit) / 3461 (IScreenMT)                       name inferred
    static void RemoveAll()
    {
        std::lock_guard lock(ms_mutex);
        while (!ms_pilha.empty()) {
            ms_pilha.back()->Deactivate();
            ms_pilha.pop_back();
        }
        NotifyStackChanged();
    }

protected:
    // Inlined everywhere (5520, 5519, 3461, 5987 ...).                                name inferred
    void Deactivate()
    {
        {
            std::lock_guard lock(m_mutex);
            m_ativo = false;
        }
        for (auto& campo : m_campos)
            campo->Stop();                                 // field slot 4
    }

    // wasm func 5541 (IScreen, not in unit) / 5520 (IScreenMT) / inlined in 11009 (IPaper)
    static void DoShow(IForm* form)                                                  // name inferred
    {
        std::lock_guard lock(ms_mutex);
        if (!ms_pilha.empty())
            ms_pilha.back()->Deactivate();
        ms_pilha.clear();
        ms_pilha.push_back(form);
        form->OnActivate();                                // slot 7
        NotifyStackChanged();                              // (the stack mutex is released first)
    }

    // wasm func 5540 (IScreen, not in unit) / 5519 (IScreenMT) / inlined in 11008 (IPaper)
    static void DoShowOnTop(IForm* form)                                             // name inferred
    {
        std::unique_lock lock(ms_mutex);
        if (!ms_pilha.empty()) {
            if (ms_pilha.back() == form)
                return;                                    // already on top: nothing to do, no notify
            ms_pilha.back()->Deactivate();
            if (auto it = std::find(ms_pilha.begin(), ms_pilha.end(), form); it != ms_pilha.end())
                ms_pilha.erase(it);                        // shared_f736 = std::find
        }
        ms_pilha.push_back(form);
        form->OnActivate();
        lock.unlock();
        NotifyStackChanged();
    }

    // wasm func 5987 (IScreen, not in unit; also called by vota::CEmitirMaisBU::ProcessInput);
    // inlined into ~IForm<IScreenMT> (2776) and ~IForm<IPaper> (5514).                 name inferred
    // If the form was the active one it is deactivated; every remaining form is redrawn from the
    // bottom up and the new top is re-activated.
    static void Remove(IForm* form)
    {
        std::unique_lock lock(ms_mutex);
        auto it = std::find(ms_pilha.begin(), ms_pilha.end(), form);
        if (ms_pilha.empty() || it == ms_pilha.end())
            return;
        if (form == ms_pilha.back())
            form->Deactivate();
        ms_pilha.erase(it);
        for (IForm* restante : ms_pilha) {
            if (restante == ms_pilha.back()) {
                restante->OnActivate();                    // slot 7 (draw + refresh + start)
            } else {
                MEDIA& media = restante->GetRenderForm();  // slot 5
                restante->m_preShow->PreShow(media);
                for (auto& campo : restante->m_campos)
                    campo->Draw(media);                    // no Refresh, no lock of the form
            }
        }
        lock.unlock();
        NotifyStackChanged();
    }

    // wasm func 3941 (shared body, MEDIA-independent; called through the thunks 5056 (IScreen,
    // not in unit), 5022 (IScreenMT) and 5512 (IPaper), which pass the static addresses).
    // Copies the stack as {form, lifetime token} pairs, under the stack mutex.         name inferred
    static std::vector<SFormInfo<MEDIA>> GetFormStack()
    {
        std::vector<SFormInfo<MEDIA>> pilha;
        std::lock_guard lock(ms_mutex);
        pilha.reserve(ms_pilha.size());                    // func 8879
        for (IForm* form : ms_pilha)
            pilha.push_back({form, form->m_controle});     // func 8872 on growth (never: reserved)
        return pilha;
    }

    // wasm func 2651 (IScreen; the tools file it under iscreen.h) / 2631 (IScreenMT) / 3670 (IPaper)
    // Publishes the new stack. The observable is filled once (std::call_once with the lambdas
    // 8902 / 8716 / 11002, which do the same assignment without notifying), then every call
    // assigns the snapshot (func 1872 = vector::assign) and notifies each observer (slot 2).
    static void NotifyStackChanged()                                                 // name inferred
    {
        static std::once_flag s_once;                      // @1832852 / @1832860 / @1839228
        std::call_once(s_once, [] {                        // 8902 / 8716 / 11002
            auto pilha = GetFormStack();
            std::lock_guard lock(ms_observavel.m_mutex);
            ms_observavel.m_valor = pilha;
        });
        auto pilha = GetFormStack();
        std::lock_guard lock(ms_observavel.m_mutex);
        ms_observavel.m_valor = pilha;                     // func 1872
        for (auto* observador : ms_observavel.m_observers)
            observador->Update(ms_observavel);             // observer slot 2
    }

    // ---- layout (72 bytes) -----------------------------------------------------------------
    std::atomic<bool> m_ativo{false};                      // +4   ? atomic: re-read after the lock
    TCampos m_campos;                                      // +8
    std::shared_ptr<IPreShow<MEDIA>> m_preShow;            // +20
    mutable std::mutex m_mutex;                            // +28  (24 bytes, zero-initialised)
    std::shared_ptr<FormControlBlock> m_controle;          // +52
    std::string m_nome;                                    // +60  (empty after the constructor)

    static inline std::mutex ms_mutex;                     // @1832604 / @1832864 / @1833500
    static inline std::vector<IForm*> ms_pilha;            // @1832632 / @1832892 / @1833528
    static inline CObservableValue<SFormInfo<MEDIA>> ms_observavel;   // @1529260 / @1529940 / @1581136
};

// Template instantiations of this header that are plain library code (unit u17 mapping table):
//   func 1872  std::vector<SFormInfo<MEDIA>>::assign(first, last, n)  (__assign_with_size; one body
//              for the three MEDIA - identical layouts, merged by wasm-opt)
//   func 8872  std::vector<SFormInfo<MEDIA>>::__emplace_back_slow_path
//   func 8879  std::vector<SFormInfo<MEDIA>>::reserve
//   func 3328 / 3327  std::__shared_mutex_base::lock() / unlock()  (FormControlBlock::m_mutex)

}  // namespace api

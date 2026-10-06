// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/comum/cinformacaothreadoperador.cpp
//   (srcloc :554, "static IInformacaoThreadOperador &vota::impl::IInformacaoThreadOperador::GetInst()").
// The class itself (96 bytes, 31 virtual slots) is reconstructed in cinformacaothreadoperador.cpp (u10).
//
// IInformacaoThreadOperador is the operator (mesário) thread's blackboard. Only the operator side and the
// voter-audio states use it, so in the web build it is created lazily (not in the start-up trace).
#include "vota/operador/comum/cinformacaothreadoperador.h"

#include <memory>
#include <mutex>
#include <source_location>
#include <string>

#include "api/pattern/cpolysingleton.h"

namespace vota::impl {

// wasm func 599 (tools name "api::CPolySingletonList::instance@599"); the CInformacaoThreadOperador
// constructor (vtable @1590116, next-inspection time from func 5412) and push<> are inlined.
IInformacaoThreadOperador& IInformacaoThreadOperador::GetInst()
{
    static std::mutex s_mutex;                                              // @1908504 (unlock residue)
    std::lock_guard trava(s_mutex);
    if (!api::CPolySingleton<IInformacaoThreadOperador>::exists())         // func 3622
        api::CPolySingletonList::push<IInformacaoThreadOperador>(std::make_unique<CInformacaoThreadOperador>(),
                                                                 api::GetPolySingletonsInfo());
        // (push itself has no "exists -> erase" prefix; only CPolySingletonList::replace has one)
    return api::CPolySingleton<IInformacaoThreadOperador>::instance(api::GetPolySingletonsInfo(),
                                                                    std::source_location::current());  // :554
}

}  // namespace vota::impl

// ---------------------------------------------------------------------------------------------------------
// Outlined one-line calls "IInformacaoThreadOperador::GetInst().slotN(...)" that the tools put in u19
// (the same pattern as funcs 1535, 1687, 2226, 2745, 2746, 3619-3621, 5414, 5416, 10583 in u10).
// They are separate functions in the binary; in the source they are ordinary expressions or small
// data-source functions bound into std::function<std::string()> text fields of MT screens.
namespace vota {

// wasm func 3623: returns the typed identity (slot 23 GetIdentidadeDigitada, by value).
// Callers: CPerguntaCodigoSuspensao::ProcessInput (10420), CValidaIdentidade::vf2 (10627), func 10670.
inline std::string IdentidadeDigitada()                                              // name inferred
{
    return impl::IInformacaoThreadOperador::GetInst().GetIdentidadeDigitada();
}

// wasm func 10670 (table slot 3860): data source of an MT text field, forwards to 3623.   // name inferred
std::string DS_IdentidadeDigitada() { return IdentidadeDigitada(); }

// wasm func 5415: slot 7 SetAudioHabilitadoManualmente(bool).
// Callers: CHabilitaAudioEleitor::ProcessInput (10523, with false), CHabilitaAudioManualmente::vf2 (10751).
inline void SetAudioHabilitadoManualmente(bool habilitado)                          // name inferred
{
    impl::IInformacaoThreadOperador::GetInst().SetAudioHabilitadoManualmente(habilitado);
}

// wasm func 10584 (table slot 4054): data source "ÁUDIO ATIVADO" / " " (slot 8 GetTextoAudio).  // name inferred
std::string DS_TextoAudio() { return impl::IInformacaoThreadOperador::GetInst().GetTextoAudio(); }

}  // namespace vota

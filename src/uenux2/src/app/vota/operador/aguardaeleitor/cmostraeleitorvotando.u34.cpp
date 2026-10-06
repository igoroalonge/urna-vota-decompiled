// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/operador/aguardaeleitor/cmostraeleitorvotando.cpp (attested by srcloc;
// the class is reconstructed by units u10/u17). This is the text source of line 3 of the MT screen
// "eleitor votando" (CDataText<std::string (*)()>, table slot 4057), see u17-foreign-fragments.cpp.
//
// WEB BUILD: dead code (operator thread not run).
#include <string>

#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"

namespace vota {
namespace {

// wasm func 10539 (table slot 4057)                                                    name inferred
// "ÁUDIO ATIVADO" while the voter uses the headphones (audio switched on from the registration data or by
// the mesário through "Ativar áudio"), otherwise a single blank (so the field is overwritten).
std::string TextoAudioEleitor()
{
    // CThreadEleitor's static unique_ptr (@1833212) is read inside a lock_guard on its singleton mutex
    // @1833188 (the lock is compiled away in this build, the mutex::unlock residue right after the read
    // remains): the voter-thread state is only read if that thread object already exists. The audio mode
    // (+4, an int) is then read after CInformacaoEleitor::GetInst() has released its own mutex. name inferred
    const bool audioEleitor = CThreadEleitor::Existe()
                              && CInformacaoEleitor::GetInst().m_modoAudio != 2;       // func 509, +4
    if (audioEleitor || impl::IInformacaoThreadOperador::GetInst().GetAudioHabilitadoManualmente())   // func 1687
        return "ÁUDIO ATIVADO";                                                        // @326872 (13 chars)
    return " ";
}

}  // namespace
}  // namespace vota

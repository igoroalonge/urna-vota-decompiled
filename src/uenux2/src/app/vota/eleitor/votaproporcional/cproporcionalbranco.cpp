// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalbranco.cpp
#include "vota/eleitor/votaproporcional/cproporcionalbranco.h"

namespace vota {

// wasm func 11704 - vtable slot 15. Latin-1 literal @371701
// (75 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CProporcionalBranco::GetMensagemAudio() const
{
    return "Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige.";
}

}  // namespace vota

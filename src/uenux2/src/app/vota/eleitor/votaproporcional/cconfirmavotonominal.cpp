// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotonominal.cpp
#include "vota/eleitor/votaproporcional/cconfirmavotonominal.h"

namespace vota {

// wasm func 11728 - vtable slot 15. Latin-1 literal @371466
// (106 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CConfirmaVotoNominal::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Aperte confirma ou corrige.";
}

}  // namespace vota

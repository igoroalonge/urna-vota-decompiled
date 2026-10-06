// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/cproporcionalnulo.cpp
#include "vota/eleitor/votaproporcional/cproporcionalnulo.h"

namespace vota {

// wasm func 11701 - vtable slot 15. Latin-1 literal @371979
// (142 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CProporcionalNulo::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} no candidato {voto}. Número errado. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige.";
}

}  // namespace vota

// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinapto.cpp
#include "vota/eleitor/votaproporcional/ccandidatoinapto.h"

namespace vota {

// wasm func 11748 - vtable slot 15. Latin-1 literal @372122
// (163 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CCandidatoInapto::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}. Candidat{o/a} não concorre. Se apertar confirma, este voto será nulo. Aperte confirma ou corrige.";
}

}  // namespace vota

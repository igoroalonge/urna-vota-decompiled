// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/ccandidatoinexistente.cpp
#include "vota/eleitor/votaproporcional/ccandidatoinexistente.h"

namespace vota {

// wasm func 11731 - vtable slot 15. Latin-1 literal @362831
// (179 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CCandidatoInexistente::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} no número {voto}. Candidato inexistente. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar este voto.";
}

}  // namespace vota

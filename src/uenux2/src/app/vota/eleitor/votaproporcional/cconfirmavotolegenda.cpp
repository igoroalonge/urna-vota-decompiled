// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/cconfirmavotolegenda.cpp
#include "vota/eleitor/votaproporcional/cconfirmavotolegenda.h"

namespace vota {

// wasm func 11735 - vtable slot 15. Latin-1 literal @362653
// (157 bytes); the {tags} are expanded by CVotacaoStateAudio::FormataMensagem (slot 14, func 6999)
// before the text goes to the speech synthesiser.
std::string CConfirmaVotoLegenda::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} no número {voto}. Aperte confirma para votar na legenda {legenda}, {nome-partido}, ou corrige para reiniciar o seu voto.";
}

}  // namespace vota

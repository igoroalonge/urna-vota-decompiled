// Reconstructed from vota_web_wasm.wasm (unit u39).
// Original (path inferred): uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp
// GetProximoEstado (11724): src/uenux2/src/app/comum/dados/u04-foreign-fragments.cpp.
#include "vota/eleitor/votaproporcional/cpedenominal.h"

namespace vota {

// wasm func 11725 - vtable slot 15. Latin-1 literal @363011 (153 bytes), tags expanded by FormataMensagem.
std::string CPedeNominal::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual}. Digite os demais dígitos do seu candidato, "
           "ou aperte confirma para prosseguir, ou corrige para reiniciar este voto.";
}

}  // namespace vota

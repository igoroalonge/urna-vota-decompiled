// uenux2/src/app/vota/eleitor/votaproporcional/cpedenulo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location record: :44 (GetProximoEstado).
#include "vota/eleitor/votaproporcional/cpedenulo.h"

#include "comum/dados/ccandidaturas.h"
#include "comum/dados/ccargos.h"
#include "ecourna/api/util/cstringutils.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/celeitorvotando.h"               // g_votoDigitado
#include "vota/eleitor/cconferevotoemcargo.h"           // CConfereVotoEmCargo<CCandidatoInapto|CProporcionalNulo, ...>

namespace vota {

// wasm func 11745 (vtable slot 15)
std::string CPedeNulo::GetMensagemAudio() const
{
    return "Você está votando para {cargo-atual} no candidato {voto}. Número errado. "
           "Aperte confirma para prosseguir, ou corrige para reiniciar este voto.";
}

// wasm func 11744 (vtable slot 16, srcloc :44)
comum::CAppState* CPedeNulo::GetProximoEstado(const std::string& numero) const
{
    auto& cargos = comum::CCargos::GetInst();
    if (cargos.IsEnd())
        throw CUeVotaError(9385, "O cargo atual nao esta posicionado");                         // :44
    const auto& cargo = cargos.GetCurrent();
    g_votoDigitado = numero;                                     // (skipped when numero is g_votoDigitado itself)

    if (cargo.GetNumeroDigitos() == numero.size()) {             // CCargo +12
        const auto* candidatura = comum::CCandidaturas::GetInst().Busca(
            cargo.GetCodigo(), ecourna::api::util::CStringUtils::ToDWord(numero));             // func 521 + 1273
        if (candidatura != nullptr && candidatura->EhInapto())                                  // +52
            return &CConfereVotoEmCargo<CCandidatoInapto, ETelaVotacao(14)>::GetInst();         // @1837820
    }
    return &CConfereVotoEmCargo<CProporcionalNulo, ETelaVotacao(7)>::GetInst();                 // @1837856
}

} // namespace vota

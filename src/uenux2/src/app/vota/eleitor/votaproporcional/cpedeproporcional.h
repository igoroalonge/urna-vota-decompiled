// uenux2/src/app/vota/eleitor/votaproporcional/cpedeproporcional.h   (path inferred from cpedeproporcional.cpp,
// attested by std::source_location records :42 and :84)
// Reconstructed from vota_web_wasm.wasm (unit u26).
//
// vota::CPedeProporcional ("ask for a proportional vote", e.g. Vereador, Deputado): first step of a
// proportional vote. The screen asks for the two digits of the party ("legenda"); when they are complete
// the vote continues in CPedeNominal (the party exists and has candidates for this office: type the rest of
// the candidate number) or in CPedeNulo (wrong party number). BRANCO goes to the blank-vote conferência.
//
// RTTI: CVotacaoStateAudio <- vota::CPedeProporcional (typeinfo @1549536, vtable @1549440), 28 bytes;
// singleton = func 3849 (merged body 6051, CVotacaoStateAudio flags 2).
//   [9] ProcessInputAudio 11711  [10] StartStateAudio 11712  [15] GetMensagemAudio 11710
#pragma once

#include <string>

#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

/// wasm func 5922 (cpedeproporcional.cpp; name inferred). True when `partido` exists in comum::CPartidos and it
/// (or its federação) has an apt candidacy for `cargo`: the first two digits typed are a valid voto de legenda.
/// Callers: CPedeProporcional::ProcessInputAudio (11711) and CVotaWebEngine::BuildStateJson (5500, web build).
bool LegendaValida(comum::TCargoID cargo, comum::TPartidoID partido);

class CPedeProporcional : public CVotacaoStateAudio {
public:
    static CPedeProporcional& GetInst();                         // func 3849 (unit u06)

    void ProcessInputAudio() override;                           // [9]  func 11711 (srcloc :84)
    void StartStateAudio() override;                             // [10] func 11712
    std::string GetMensagemAudio() const override;               // [15] func 11710

    /// cpedeproporcional.cpp:42 - thunk func 5923 over the merged body func 6050 (error 9386).
    CFormInterativoTelaVota GetTelaCargoAtual();

private:
    CPedeProporcional() : CVotacaoStateAudio(2) {}
};

} // namespace vota

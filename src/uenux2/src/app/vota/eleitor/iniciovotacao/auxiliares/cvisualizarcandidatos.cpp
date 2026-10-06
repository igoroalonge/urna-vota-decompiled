// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.cpp
//
// srcloc evidence:
//   :132  StartState  "Nenhuma candidatura a ser exibida"   (CUeVotaError 9378)
//   :140  StartState  "Número de inputs inválido"           (CUeVotaError 9379)
//   api/gui/cinteractiveform.h:62  CInteractiveForm<IScreen, IInputKbd>::WaitAndRead(), inlined
//
// WARNING (web build): WaitAndRead() busy-waits for a key with CWait::Sleep(10) = emscripten_sleep(10)
// while the byte @1584624 == 1, which aborts the wasm (no Asyncify). The state is not reachable from the
// voter terminal driven by the web page.

#include "vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h"

#include <source_location>
#include <vector>

#include "vota/comum/votadefs.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/auxiliares/cmenuvisualizarcandidatos.h"

namespace vota {

// wasm func 11876 — vtable slot 2
void CVisualizarCandidatos::StartState()
{
    m_proximoEstado = &CMenuVisualizarCandidatos::GetInst();       // api_f2288: when this returns, back to the menu

    std::vector<const comum::md::CCandidatura*> selecionadas;
    for (const auto& c : m_candidaturas) {
        if (m_numero && *m_numero != 0 && c.GetNumero() != *m_numero)       continue;
        if (m_cargo && *m_cargo != 0 && c.GetCargo() != *m_cargo)           continue;
        if (m_partido && *m_partido != 0 && c.GetPartido() != *m_partido)   continue;
        selecionadas.push_back(&c);
    }
    if (selecionadas.empty())
        throw CUeVotaError(9378, "Nenhuma candidatura a ser exibida", std::source_location::current());  // :132

    const std::size_t total = selecionadas.size();
    auto atual = selecionadas.begin();
    auto tela = CTelasVota::GetInst().CriaTelaVisualizacaoCandidato(**atual, 1, total);   // func 6569

    // Modal loop inside StartState (the thread does not return to its event loop while browsing).
    while (tela->GetInputs().size() == 1) {
        tela->Exibe();
        const auto& campo = *tela->GetInputs().at(0);
        switch (tela->WaitAndRead()) {                                       // cinteractiveform.h:62
        case api::EInputResult::Corrige:                                     // 5: "Retornar"
            return;
        case api::EInputResult::Tecla:                                       // 13
            if (campo.GetUltimaTecla() == '4')                               // previous, wraps to the last
                atual = (atual == selecionadas.begin()) ? selecionadas.end() - 1 : atual - 1;
            else if (campo.GetUltimaTecla() == '6')                          // next, wraps to the first
                atual = (atual == selecionadas.end() - 1) ? selecionadas.begin() : atual + 1;
            break;
        default:
            break;
        }
        tela = CTelasVota::GetInst().CriaTelaVisualizacaoCandidato(
            **atual, static_cast<std::size_t>(atual - selecionadas.begin()) + 1, total);
    }
    throw CUeVotaError(9379, "Número de inputs inválido", std::source_location::current());     // :140
}

// wasm func 2871 — slot 0: destroys m_candidaturas (72-byte elements: the optional string at +32 when
// its flag +44 is set, the strings at +20 and +8, and a sub-object at +60 via shared_f1719).
CVisualizarCandidatos::~CVisualizarCandidatos() = default;

// wasm func 11877 — slot 1: deleting destructor (~CVisualizarCandidatos(); operator delete).
// wasm func 11880 — at-exit destructor of the singleton pointer @1834660 (func 1279 registers it).

}  // namespace vota

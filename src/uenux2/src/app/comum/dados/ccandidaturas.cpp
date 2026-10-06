// uenux2/src/app/comum/dados/ccandidaturas.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03) — the four functions of this file that are in the unit.
//
// CCandidaturas is the in-memory list of candidacies (candidaturas) of the loaded election, positioned on the
// "current" one by the voting screens (CDataMap cursor). Data sources such as CCandidaturasDSNome / DSNumero /
// DSSexo / DSFoto read the current candidacy through GetCandidaturaAtual().
#include "comum/dados/ccandidaturas.h"

#include <format>

namespace comum {

// wasm func 5810 (srcloc line 89)
const std::string CCandidaturas::RecuperaVersaoPacote(const TCargoID cargo) const
{
    const auto it = m_versoesPacote.find(cargo);
    if (it == m_versoesPacote.end()) {
        throw CDadosError(7801, std::format("Versão de pacote não encontrada para o cargo {}", cargo));   // line 89
    }
    return it->second;
}

// wasm func 2838 (srcloc line 261)
const md::CCandidatura& GetCandidaturaAtual(const std::string& contexto)
{
    const md::CCandidatura* atual = CCandidaturas::GetInst().GetCurrent();   // api::CDataMap<>::GetCurrent (null at end)
    if (atual == nullptr) {
        throw CDadosError(7805, std::format("{} - não posicionado no candidato corretamente", contexto));   // line 261
    }
    return *atual;
}

// CCandidaturas::GetInst (inlined here, out-of-line copy = func 521):
//   static std::unique_ptr<CCandidaturas> s_instancia;  static std::mutex s_mutex;
//   std::lock_guard lock(s_mutex);
//   if (!s_instancia) s_instancia.reset(new CCandidaturas());
//   return *s_instancia;

} // namespace comum

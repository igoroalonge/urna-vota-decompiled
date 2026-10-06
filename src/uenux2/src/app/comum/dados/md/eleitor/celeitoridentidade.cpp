// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original (path inferred): uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp
//
// The two functions below were attributed by the analysis tools to cvalidadoridentidade.cpp (566:
// its body contains the inlined CValidadorIdentidade::Valida, srcloc cvalidadoridentidade.cpp:61)
// and to cbiometriaeleitor.cpp (2798: by caller). Both are really CEleitorIdentidade code:
// 566 returns `this` (wasm constructor ABI) and fills {string, tipo}; 2798 is the formatter it uses.
#include "celeitoridentidade.h"

#include "../cvalidadoridentidade.h"

namespace comum::md {

namespace {
// comum_f3510: s.erase(0, s.find_first_not_of('0'))  (clears the string when it is all zeros)
void RemoveZerosEsquerda(std::string& s)
{
    const auto pos = s.find_first_not_of('0');
    if (pos == std::string::npos) s.clear(); else s.erase(0, pos);
}
// comum_f3512: s.insert(0, largura - s.size(), '0') when shorter
void PreencheZerosEsquerda(std::string& s, std::size_t largura)
{
    if (s.size() < largura) s.insert(0, largura - s.size(), '0');
}
std::size_t Largura(ETipoIdentificadorEleitor tipo)
{
    return tipo == ETipoIdentificadorEleitor::CPF ? 11 : 12;
}
}  // namespace

// wasm func 2798  name inferred
std::string CEleitorIdentidade::Formata(std::string identidade, ETipoIdentificadorEleitor tipo)
{
    const std::size_t largura = Largura(tipo);
    if (identidade.size() > largura)
        RemoveZerosEsquerda(identidade);
    PreencheZerosEsquerda(identidade, largura);
    return identidade;
}

// wasm func 566  name inferred (tools: "CValidadorIdentidade::Valida")
// Every voter/poll-worker identity built from data files (CConversorEntidadeEleitores,
// CConversorImpedido), from the DAOs and from what the operator types goes through here, so an
// invalid título/CPF raises CUeComumDadosError 8108/8109 at construction time.
CEleitorIdentidade::CEleitorIdentidade(std::string identidade, ETipoIdentificadorEleitor tipo)
    : m_identidade(Formata(std::move(identidade), tipo))
    , m_tipo(tipo)
{
    CValidadorIdentidade::GetInst().Valida(m_tipo, m_identidade);   // inlined (cvalidadoridentidade.cpp:61)

    // normalised again after validation (same code as Formata, applied in place)
    const std::size_t largura = Largura(m_tipo);
    if (m_identidade.size() > largura)
        RemoveZerosEsquerda(m_identidade);
    PreencheZerosEsquerda(m_identidade, largura);
}

}  // namespace comum::md

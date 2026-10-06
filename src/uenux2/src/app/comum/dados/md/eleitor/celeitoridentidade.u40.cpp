// FRAGMENT of uenux2/src/app/comum/dados/md/eleitor/celeitoridentidade.cpp (path inferred by unit u05, which
// reconstructed the class). Reconstructed by unit u40 from vota_web_wasm.wasm.
// Declaration to add to celeitoridentidade.h:   std::string ToString() const;   // wasm func 1924
#include "celeitoridentidade.h"

namespace comum::md {

// wasm func 1924 (not observed executing). Callers: vota::TipoToStr host 10586 (operator microterminal texts
// "Título: xxxx xxxx xxxx"), comum::(anon)::SituacaoEleitor 11233 (voter-list report) and
// vota::CGeraRelatorios::StartState 12105.                                     // name inferred (u25: ToString)
// Display form of the identifier: título "1234 5678 9012", CPF "123.456.789-01", free id unchanged, any other
// type -> "" (the default of the switch). The positions are fixed: the constructor already normalised the
// digits to 12 (título/livre) or 11 (CPF) characters, so insert() cannot go past the end.
std::string CEleitorIdentidade::ToString() const
{
    std::string texto = m_identidade;
    switch (m_tipo) {
    case ETipoIdentificadorEleitor::TITULO:
        return texto.insert(8, " ").insert(4, " ");                 // literal @445549
    case ETipoIdentificadorEleitor::CPF:
        return texto.insert(9, "-").insert(6, ".").insert(3, ".");  // literals @378254 "-", @378041 "."
    case ETipoIdentificadorEleitor::LIVRE:
        return texto;
    }
    return {};
}

} // namespace comum::md

// uenux2/src/app/comum/dados/md/cregraidentidadelivre.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35). Not observed executing (the training scenarios identify voters by
// título).
#include "comum/dados/md/cregraidentidadelivre.h"

namespace comum::md {

// wasm func 11273 - vtable slot 2.
ETipoIdentificadorEleitor CRegraIdentidadeLivre::GetTipo() const
{
    return ETipoIdentificadorEleitor(3);                 // LIVRE
}

// wasm func 11274 - vtable slot 3. No normalisation: the identifier is kept exactly as typed.
std::string CRegraIdentidadeLivre::Formata(const std::string& identidade) const
{
    return identidade;
}

// Slot 4 (Verifica) is the shared "return true" body: a free identifier has no check digits. It must still be a
// non-empty string of digits: IRegraIdentidade::Valida (iregraidentidade.cpp:23, inlined in func 566) tests that
// before calling Verifica ("Identidade inválida: ...", code 8109).
// NOTE (func 566, md::CEleitorIdentidade's constructor, unit u05): the rule lookup
// (cvalidadoridentidade.cpp:61) stops at the first rule whose GetTipo() equals the requested type OR equals 3.
// When this rule is registered before the título/CPF rule, it is the one used for every identifier type, so the
// check digits of a título/CPF are then not verified.

} // namespace comum::md

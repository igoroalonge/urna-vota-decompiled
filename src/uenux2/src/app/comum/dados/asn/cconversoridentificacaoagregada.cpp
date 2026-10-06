// uenux2/src/app/comum/dados/asn/cconversoridentificacaoagregada.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/cconversoridentificacaoagregada.h"

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11455 (vtable slot 3). Not observed executing (the section in the scenarios has no aggregated sections).
// The ENUMERATED is copied (ENUMERATED copy ctor, func 752) because Utils::DesconverteTipoLocalVotacao takes it by value.
md::CIdentificacaoAgregada CConversorIdentificacaoAgregada::DoDesconverte(const ModuloLocal::IdentificacaoAgregada& agregada) const
{
    const auto numero = static_cast<TSecaoID>(agregada.get_numero());
    return md::CIdentificacaoAgregada(numero, Utils::DesconverteTipoLocalVotacao(agregada.get_tipoLocalOrigem()));
}

} // namespace comum::asn

// ---------------------------------------------------------------------------------------------------------------
// wasm func 5672 - md::CIdentificacaoAgregada::CIdentificacaoAgregada(TSecaoID numero, ETipoLocalVotacao tipo)
// (inline constructor, name inferred; also called by comum::CLocal::GetTodasSecoes, func 5741):
//     m_tipo = tipo;      // +0
//     m_numero = numero;  // +4 (uint16)

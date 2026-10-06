// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/eleitor/celeitordecorator.cpp
//
// CEleitorDecorator (sizeof 108) = a CEleitor (+0..+103) plus the identifier type that is "principal"
// for this election (+104, copied from the configuration by CEleitores::GetEleitoresEstaticos, func
// 5772). CEleitores keeps its static roll as a vector<CEleitorDecorator> sorted by that principal
// identity (std::sort instantiation funcs 945/3761/5755/5756/5769, comparator = func 456).
#include "celeitordecorator.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 708 (srcloc line 39)
CEleitorIdentidade CEleitorDecorator::GetIdentidadePorTipo(ETipoIdentificadorEleitor tipo) const
{
    const auto& identidades = m_eleitor.GetIdentidades();
    const auto it = std::find_if(identidades.begin(), identidades.end(),
                                 [tipo](const CEleitorIdentidade& i) { return i.GetTipo() == tipo; });
    if (it != identidades.end())
        return *it;
    throw CUeComumDadosError(7833, std::format("Tipo de identificador inválido: {}",
                                               static_cast<unsigned>(tipo) & 0xFF));
}

// wasm func 456  name inferred (tools: comum_f456). Three-way comparison by principal identity,
// i.e. the defaulted CEleitorIdentidade::operator<=> (memcmp of the digit strings, then length,
// then type) applied to GetIdentidadePorTipo(m_tipoPrincipal) of both sides.
std::strong_ordering CEleitorDecorator::operator<=>(const CEleitorDecorator& outro) const
{
    return GetIdentidadePorTipo(m_tipoPrincipal) <=> outro.GetIdentidadePorTipo(outro.m_tipoPrincipal);
}

}  // namespace comum::md

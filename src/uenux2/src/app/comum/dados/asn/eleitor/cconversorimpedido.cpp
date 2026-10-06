// uenux2/src/app/comum/dados/asn/eleitor/cconversorimpedido.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Voters impeded from voting in this section (impedidos), file <...>-imp.dat:
//   EntidadeImpedidos ::= SEQUENCE { cabecalho, identificacao IdentificacaoSecaoEleitoral,
//                                    eleitores SEQUENCE OF EleitorImpedido OPTIONAL }
//   EleitorImpedido ::= SEQUENCE { identificadorEleitor IdentificadorEleitor, impedimentoP1 TipoImpedimento,
//                                  impedimentoP2 TipoImpedimento OPTIONAL }          (P1/P2 = 1st/2nd pleito)
//   TipoImpedimento ::= ENUMERATED { semImpedimento(1) ... tteSituacaoRua(15) }
// Example (scenario geral-t1): {numeroInscricao "XXXXXXXXXXXX", P1 semImpedimento, P2 suspenso}.
#include "comum/dados/asn/eleitor/cconversorimpedido.h"

#include <format>
#include <string>

#include "comum/dados/md/cvalidadoridentidade.h"

namespace comum::asn {

// wasm func 5699 (srcloc line 109). The parameter is taken by value (the caller copies the ENUMERATED).
md::ETipoImpedimento CConversorImpedido::DesconverteTipoImpedimento(const ModuloImpedidos::TipoImpedimento tipo) const
{
    const int valor = tipo.asInt();
    if (valor < 1 || valor > 15) {
        throw CDadosError(7904, std::format("Tipo de impedimento inválido: {}", valor));   // line 109
    }
    return static_cast<md::ETipoImpedimento>(valor - 1);   // semImpedimento -> 0 ... tteSituacaoRua -> 14
}

// Inlined into wasm func 11410 (srcloc line 40)
md::CEleitorIdentidade CConversorImpedido::MontaEleitorIdentidade(const ModuloTiposEleitorais::IdentificadorEleitor& id) const
{
    const std::string numero = id.getSelection<ASN1::AbstractString>().getValue();
    switch (id.currentSelection()) {
    case 0: return md::CValidadorIdentidade::Valida(numero, 1);   // numeroInscricao
    case 1: return md::CValidadorIdentidade::Valida(numero, 2);   // numeroCPF
    case 2: return md::CValidadorIdentidade::Valida(numero, 3);   // identificacaoLivre
    }
    throw CDadosError(7903, "Tipo desconhecido de identidade");   // line 40
}

// wasm func 11410 (vtable slot 3; the tool named it after the inlined MontaEleitorIdentidade)
std::vector<md::CImpedido> CConversorImpedido::DoDesconverte(const TEntidade& entidade) const
{
    const auto secao = static_cast<TSecaoID>(entidade.get_identificacao().get_secao());
    std::vector<md::CImpedido> impedidos;
    if (!entidade.eleitores_isPresent()) {
        return impedidos;
    }
    for (const auto* eleitor : entidade.get_eleitores()) {
        const auto impedimentoP1 = DesconverteTipoImpedimento(eleitor->get_impedimentoP1());
        const auto identidade = MontaEleitorIdentidade(eleitor->get_identificadorEleitor());
        if (eleitor->impedimentoP2_isPresent()) {
            const auto impedimentoP2 = DesconverteTipoImpedimento(eleitor->get_impedimentoP2());
            impedidos.emplace_back(secao, identidade, impedimentoP1, impedimentoP2);   // func 5661
        } else {
            impedidos.emplace_back(secao, identidade, impedimentoP1);                  // func 5662
        }
    }
    return impedidos;
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Emitted in this TU (inline constructors of md::CImpedido, real home md/eleitor/cimpedido.h):
//  * wasm func 5661 — CImpedido(TSecaoID, const CEleitorIdentidade&, ETipoImpedimento p1, ETipoImpedimento p2):
//    stores p2 and temImpedimentoP2 = true.
//  * wasm func 5662 — CImpedido(TSecaoID, const CEleitorIdentidade&, ETipoImpedimento p1):
//    stores p2 = ETipoImpedimento(15) (one past the last valid value) and temImpedimentoP2 = false.

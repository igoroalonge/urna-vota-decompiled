// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversordetalhecandidato.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
//   DetalheCargo ::= SEQUENCE { qtdeSuplentes INTEGER (0..2), nomes NomesCargo, temFoto BOOLEAN,
//                               suplencias SEQUENCE OF Suplencias OPTIONAL }
// Suplentes (vice-prefeito, vice-governador, 1º/2º suplente de senador...) are shown next to the
// candidate on the confirmation screen; this converter enforces qtdeSuplentes == size(suplencias).
#include "comum/dados/asn/processoeleitoral/cconversordetalhecandidato.h"

#include <vector>

#include "comum/dados/asn/processoeleitoral/cconversornomescargo.h"
#include "comum/dados/asn/processoeleitoral/cconversorsuplencia.h"

namespace comum::asn {

// wasm func 11375 (vtable slot 3; srcloc lines 29, 34, 39)
md::CDetalheCandidato CConversorDetalheCandidato::DoDesconverte(const TEntidade& detalhe) const
{
    const CConversorNomesCargo conversorNomes;
    const auto qtdeSuplentes = static_cast<std::size_t>(detalhe.get_qtdeSuplentes());

    if (qtdeSuplentes == 0) {
        if (detalhe.suplencias_isPresent() && !detalhe.get_suplencias().empty()) {
            throw CDadosError(7950, "Quantidade de suplências incompatível com sequência de suplências.");   // line 29
        }
    } else if (!detalhe.suplencias_isPresent()) {
        throw CDadosError(7951, "Quantidade não nula e sem suplências.");                                    // line 34
    }
    if (detalhe.suplencias_isPresent() && qtdeSuplentes != detalhe.get_suplencias().size()) {
        throw CDadosError(7952, "Quantidade de suplências incompatível.");                                   // line 39
    }

    const bool temFoto = detalhe.get_temFoto();
    const md::CNomesCargo nomes = conversorNomes.Desconverte(detalhe.get_nomes());

    std::vector<md::CSuplencia> suplencias;
    for (std::size_t i = 0; i < qtdeSuplentes; ++i) {
        const CConversorSuplencia conversorSuplencia;   // constructed inside the loop
        suplencias.push_back(conversorSuplencia.Desconverte(detalhe.get_suplencias().at(i)));
    }
    return md::CDetalheCandidato(temFoto, nomes, suplencias);   // inline ctor: copies the vector (func 2341 per element)
}

} // namespace comum::asn

// ----------------------------------------------------------------------------------------------------------
// Code emitted in this TU (listed in the unit):
//  * wasm func 2341 — md::CSuplencia::CSuplencia(const CSuplencia&), the implicit copy constructor
//    (2-byte header + 4 std::string). Also used by comum_f365 (CDetalheCandidato copy) and unknown_f5594.
//  * wasm func 1135 — std::vector<md::CSuplencia>::__base_destruct_at_end(new_last): destroys 52-byte elements
//    (4 strings each) from the end. Library helper (also called by comum_f242, comum_f761, comum_f5582...).

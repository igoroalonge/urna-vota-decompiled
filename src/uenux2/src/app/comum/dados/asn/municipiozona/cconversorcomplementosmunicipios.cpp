// uenux2/src/app/comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Municipality complements (time zone, daylight saving time, biometrics switch), file <...>-cm.dat:
//   EntidadeComplementosMunicipios ::= SEQUENCE { cabecalho CabecalhoEntidade,
//                                                 municipiosUf SEQUENCE OF ComplementosMunicipiosUF }
//   ComplementosMunicipiosUF ::= SEQUENCE { uf SiglaUF, fusoTRE INTEGER (-720..720),
//                                           complementosMunicipios SEQUENCE OF ComplementoMunicipio }
// The urna accepts exactly ONE UF per file. cabecalho and fusoTRE are not read.
#include "comum/dados/asn/municipiozona/cconversorcomplementosmunicipios.h"

#include <format>
#include <string>
#include <vector>

#include "comum/dados/asn/municipiozona/cconversorcomplementomunicipio.h"

namespace comum::asn {

// wasm func 11381 (vtable slot 3; srcloc line 29, plus the inlined constructor
//                  md::CComplementosMunicipiosUF::CComplementosMunicipiosUF, ccomplementosmunicipiosuf.cpp:26/31)
md::CComplementosMunicipiosUF CConversorComplementosMunicipios::DoDesconverte(const TEntidade& entidade) const
{
    const auto& municipiosUf = entidade.get_municipiosUf();
    if (municipiosUf.size() != 1) {
        throw CDadosError(7944, std::format("Quantidade inválida de UFs [{}]", municipiosUf.size()));   // line 29
    }

    md::CComplementosMunicipiosUF::TVetorCM complementos;   // std::vector<md::CComplementoMunicipio> (32-byte elements)
    const CConversorComplementoMunicipio conversor;
    for (const auto* complemento : municipiosUf[0].get_complementosMunicipios()) {
        complementos.push_back(conversor.Desconverte(*complemento));
    }
    // Inlined md constructor (ccomplementosmunicipiosuf.cpp):
    //   if (uf.size() != 2)       throw CDadosError(8115, std::format("UF inválida: [{}]", uf));        // :26
    //   if (complementos.empty()) throw CDadosError(8116, "Lista de complementos de municípios vazia");  // :31
    return md::CComplementosMunicipiosUF(municipiosUf.at(0).get_uf(), complementos);
}

} // namespace comum::asn

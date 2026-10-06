// ecourna-lib/ecourna/app/dados/asn/cconversorcargoid.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversorcargoid.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   CodigoCargoConsulta ::= CHOICE { cargoConstitucional [1] CargoConstitucional (1..13),
//                                    numeroCargoConsultaLivre [2] INTEGER (25..99) }
//   <-> TCargoID = CBaseType<unsigned short, 1, 99, 3>
// A cargo code 1..13 is a constitutional office (presidente ... vereador); 25..99 is a free
// cargo/consulta number (referendums etc.); 14..24 and 0 are invalid.
// Used by the RDV vote converters (comum::asn::CConversorVotosCargo, CConversorEleicoesVota).
// DoConverte was observed executing during the recorded votes.
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9230 (vtable slot 2; srcloc line 66)
// The original is a switch with one case per constitutional office (13 identical blocks that each
// create the ENUMERATED through its info table's create function and select CHOICE alternative 0).
ModuloTiposEleitorais::CodigoCargoConsulta CConversorCargoID::DoConverte(const TCargoID& cargo) const
{
    ModuloTiposEleitorais::CodigoCargoConsulta codigo;
    const unsigned short valor = cargo;
    if (valor >= 25) {
        codigo.select_numeroCargoConsultaLivre() = valor;                   // CHOICE id 1, INTEGER
        return codigo;
    }
    switch (valor) {
    case ModuloTiposEleitorais::CargoConstitucional::presidente:            // 1
    case ModuloTiposEleitorais::CargoConstitucional::vicePresidente:        // 2
    case ModuloTiposEleitorais::CargoConstitucional::governador:            // 3
    case ModuloTiposEleitorais::CargoConstitucional::viceGovernador:        // 4
    case ModuloTiposEleitorais::CargoConstitucional::senador:               // 5
    case ModuloTiposEleitorais::CargoConstitucional::deputadoFederal:       // 6
    case ModuloTiposEleitorais::CargoConstitucional::deputadoEstadual:      // 7
    case ModuloTiposEleitorais::CargoConstitucional::deputadoDistrital:     // 8
    case ModuloTiposEleitorais::CargoConstitucional::primeiroSuplenteSenador:  // 9
    case ModuloTiposEleitorais::CargoConstitucional::segundoSuplenteSenador:   // 10
    case ModuloTiposEleitorais::CargoConstitucional::prefeito:              // 11
    case ModuloTiposEleitorais::CargoConstitucional::vicePrefeito:          // 12
    case ModuloTiposEleitorais::CargoConstitucional::vereador:              // 13
        codigo.select_cargoConstitucional() =                                // CHOICE id 0, ENUMERATED
            static_cast<ModuloTiposEleitorais::CargoConstitucional::NamedNumber>(valor);
        return codigo;
    }
    throw CAsnError(2245, "Código de cargo inválido.");                    // line 66
}

// wasm func 9228 (vtable slot 3; srcloc line 80)
// Both alternatives keep their value at the same offset, so the value is read without looking at
// which one is selected; the CBaseType constructor (func 5682) range-checks it (1..99).
CConversorCargoID::TDado CConversorCargoID::DoDeconverte(const TEntidade& codigo) const
{
    if (static_cast<unsigned>(codigo.currentSelection()) >= 2) {           // also catches "unselected" (-1)
        throw CAsnError(2246, "Estrutura do código de cargo inválida.");  // line 80
    }
    return TCargoID(static_cast<unsigned short>(codigo.getSelection().asInt()));
}

} // namespace ecourna::app::dados::asn

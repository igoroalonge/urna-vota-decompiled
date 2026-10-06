// ecourna-lib/ecourna/app/dados/asn/cconversorfase.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversorfase.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   Fase ::= ENUMERATED { simulado(1), oficial(2), treinamento(3) }  <->  CFaseID (same numbers)
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9195 (vtable slot 2; srcloc lines 27, 29)
CConversorFase::TEntidade CConversorFase::DoConverte(const TDado& fase) const
{
    switch (static_cast<int>(fase)) {
    case CFaseID::Simulado:
    case CFaseID::Oficial:
    case CFaseID::Treinamento:
        return ModuloTiposEleitorais::Fase(static_cast<ModuloTiposEleitorais::Fase::NamedNumber>(static_cast<int>(fase)));
    case 0:   // ? an explicit "undefined phase" enumerator of CFaseID
        throw CAsnError(2248, "Fase inválida.");   // line 27
    }
    throw CAsnError(2249, "Fase inválida.");       // line 29
}

// wasm func 9193 (vtable slot 3; srcloc lines 42, 44)
CConversorFase::TDado CConversorFase::DoDeconverte(const TEntidade& fase) const
{
    switch (fase.asInt()) {
    case ModuloTiposEleitorais::Fase::simulado:
    case ModuloTiposEleitorais::Fase::oficial:
    case ModuloTiposEleitorais::Fase::treinamento:
        return CFaseID(fase.asInt());
    case -1:
        // ? Like the other enum converters of the TSE code (see unit u03), the switch has an explicit
        //   case for the value -1, probably an extra "invalid" enumerator of the generated NamedNumber.
        throw CAsnError(2250, "Fase inválida.");   // line 42
    }
    throw CAsnError(2251, "Fase inválida.");       // line 44
}

} // namespace ecourna::app::dados::asn

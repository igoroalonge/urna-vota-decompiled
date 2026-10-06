// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocarga.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// "Dado de carga" (how this urna was loaded) inside eg.bin:
//   DadoCarga ::= SEQUENCE { turno Turno, tipoUrnaT1 TipoUrnaOperacao, tipoUrnaT2 TipoUrnaOperacao,
//                            modelo ModeloEquipamento, fase Fase }
//   TipoUrnaOperacao ::= ENUMERATED { semtipo(0), vota(1), contingencia(2), contingenciavota(3),
//                                     contingenciavotarecupera(4) }
//   ModeloEquipamento ::= ENUMERATED { tpm20(2), ue2013(13), ue2015(15), ue2020(20), ue2022(22) }
// The md side stores the type as a character '0'..'4' and the model as the model year.
#include "comum/dados/asn/estadoaplicacao/cconversordadocarga.h"

#include <format>

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 5693 (srcloc line 61)
ModuloEstadoGeralUrna::TipoUrnaOperacao CConversorDadoCarga::ConverteTipoUrna(const EUrnaTipo tipo) const
{
    const int valor = static_cast<char>(tipo) - '0';
    if (valor < 0 || valor > 4) {
        // Unlike the other md->ASN switches of this unit, the argument is a plain int (packed arg type 3, the raw
        // character code), not the enum through the TSE std::formatter handle.
        throw CDadosError(7909, std::format("Valor inválido para EUrnaTipo: {}", static_cast<int>(tipo)));   // line 61
    }
    return ModuloEstadoGeralUrna::TipoUrnaOperacao(valor);
}

// wasm func 5694 (srcloc lines 84, 88)
EUrnaTipo CConversorDadoCarga::DesconverteTipoUrna(const ModuloEstadoGeralUrna::TipoUrnaOperacao& tipo) const
{
    switch (tipo.asInt()) {
    case ModuloEstadoGeralUrna::TipoUrnaOperacao::semtipo:                  return EUrnaTipo{'0'};
    case ModuloEstadoGeralUrna::TipoUrnaOperacao::vota:                     return EUrnaTipo{'1'};
    case ModuloEstadoGeralUrna::TipoUrnaOperacao::contingencia:             return EUrnaTipo{'2'};
    case ModuloEstadoGeralUrna::TipoUrnaOperacao::contingenciavota:         return EUrnaTipo{'3'};
    case ModuloEstadoGeralUrna::TipoUrnaOperacao::contingenciavotarecupera: return EUrnaTipo{'4'};
    case -1:   // ? generated "invalid" enumerator
        throw CDadosError(7911, std::format("Valor inválido para TipoUrnaOperacao: {}", tipo));   // line 84
    }
    throw CDadosError(7912, std::format("Valor inválido para TipoUrnaOperacao: {}", tipo));       // line 88
}

// Inlined into wasm func 11401 (srcloc line 105)
ModuloTiposEcoUrna::ModeloEquipamento CConversorDadoCarga::ConverteModelo(const EUrnaModelo modelo) const
{
    // Compiled as a bit test (645 = bits 0,2,7,9 of modelo-2013) + lookup table @509460 = {13,0,15,0,0,0,0,20,0,22}.
    switch (static_cast<int>(modelo)) {
    case 2013: return ModuloTiposEcoUrna::ModeloEquipamento::ue2013;
    case 2015: return ModuloTiposEcoUrna::ModeloEquipamento::ue2015;
    case 2020: return ModuloTiposEcoUrna::ModeloEquipamento::ue2020;
    case 2022: return ModuloTiposEcoUrna::ModeloEquipamento::ue2022;
    default:
        throw CDadosError(7913, std::format("Valor não suportado para EUrnaModelo: {}", modelo));   // line 105
    }
}

// Inlined into wasm func 11402 (srcloc lines 127, 131)
EUrnaModelo CConversorDadoCarga::DesconverteModelo(const ModuloTiposEcoUrna::ModeloEquipamento& modelo) const
{
    switch (modelo.asInt()) {
    case ModuloTiposEcoUrna::ModeloEquipamento::ue2013: return EUrnaModelo{2013};
    case ModuloTiposEcoUrna::ModeloEquipamento::ue2015: return EUrnaModelo{2015};
    case ModuloTiposEcoUrna::ModeloEquipamento::ue2020: return EUrnaModelo{2020};
    case ModuloTiposEcoUrna::ModeloEquipamento::ue2022: return EUrnaModelo{2022};
    case -1:   // ? generated "invalid" enumerator
    case ModuloTiposEcoUrna::ModeloEquipamento::tpm20:   // a TPM 2.0 media, not an urna model
        // The ENUMERATED object itself is formatted (handle, table slot 2655 -> func 1699), not asInt().
        throw CDadosError(7915, std::format("Valor não suportado para ModeloUrna: {}", modelo));   // line 127
    }
    throw CDadosError(7916, std::format("Valor inválido para ModeloUrna: {}", modelo));           // line 131
}

// wasm func 11401 (vtable slot 2; srclocs: util.cpp:566 Utils::ConverteTurno inlined, line 105)
ModuloEstadoGeralUrna::DadoCarga CConversorDadoCarga::DoConverte(const TDado& carga) const
{
    ModuloEstadoGeralUrna::DadoCarga entidade;
    // Utils::ConverteTurno(EUrnaTurno): (turno - '0') >= 3 -> CBaseError<EUeComumAsnError>(7689, "Turno inválido: {}").
    entidade.set_turno(Utils::ConverteTurno(carga.GetTurno()));
    entidade.set_tipoUrnaT1(ConverteTipoUrna(carga.GetTipoUrnaT1()));
    entidade.set_tipoUrnaT2(ConverteTipoUrna(carga.GetTipoUrnaT2()));
    entidade.set_fase(Utils::ConverteFase(carga.GetFase()));
    entidade.set_modelo(ConverteModelo(carga.GetModelo()));
    return entidade;
}

// wasm func 11402 (vtable slot 3; srclocs: util.cpp:581 Utils::DesconverteTurno inlined, lines 127/131)
md::estadoaplicacao::CDadoCarga CConversorDadoCarga::DoDesconverte(const TEntidade& carga) const
{
    // Utils::DesconverteTurno: value >= 3 -> CBaseError<EUeComumAsnError>(7690, "Turno inválido: {}"),
    // else EUrnaTurno('0' | value).
    const EUrnaTurno turno = Utils::DesconverteTurno(carga.get_turno());
    const EUrnaTipo tipoT1 = DesconverteTipoUrna(carga.get_tipoUrnaT1());
    const EUrnaTipo tipoT2 = DesconverteTipoUrna(carga.get_tipoUrnaT2());
    const EUrnaFase fase = Utils::DesconverteFase(carga.get_fase());
    const EUrnaModelo modelo = DesconverteModelo(carga.get_modelo());
    // Out-of-line md constructor (func 5633, named "CDadoCarga::ValidaModelo" after its inlined srcloc):
    // it stores the five fields and validates the model.
    return md::estadoaplicacao::CDadoCarga(turno, tipoT1, tipoT2, modelo, fase);
}

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/rdv/cconversorvoto.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// One RDV vote: ModuloRegistroDigitalVoto::Voto { tipoVoto TipoVoto, digitacao VotoDigitado OPTIONAL }
//            <-> md::CVoto { +0 ETipo tipo; +4 std::string digitacao }   (16 bytes)
// md::CVoto::ETipo uses the same numbers as TipoVoto (1 legenda ... 9 nuloAposSuspensaoCargoSemCandidato):
// both directions only range-check and copy the integer.
#include "comum/dados/asn/rdv/cconversorvoto.h"

#include <format>
#include <string>

namespace comum::asn {

// Inlined into wasm func 11355 (srcloc line 44).
ModuloRegistroDigitalVoto::TipoVoto CConversorVoto::ConverteTipo(const md::CVoto::ETipo tipo)
{
    const auto valor = static_cast<int>(tipo);
    if (valor < 1 || valor > 9) {   // (valor - 1) >= 9 as unsigned
        // The ETipo argument goes through a std::formatter<enum> (format handle, table slot 2776 -> func 536).
        throw CDadosError(7973, std::format("Valor inválido para tipo voto eleitor: {}", tipo));   // line 44
    }
    return ModuloRegistroDigitalVoto::TipoVoto(static_cast<ModuloRegistroDigitalVoto::TipoVoto::NamedNumber>(valor));
}

// Inlined into wasm func 11354 (srcloc lines 71, 75).
md::CVoto::ETipo CConversorVoto::DesconverteTipo(const ModuloRegistroDigitalVoto::TipoVoto tipo)
{
    switch (tipo.asInt()) {
    case ModuloRegistroDigitalVoto::TipoVoto::legenda:
    case ModuloRegistroDigitalVoto::TipoVoto::nominal:
    case ModuloRegistroDigitalVoto::TipoVoto::branco:
    case ModuloRegistroDigitalVoto::TipoVoto::nulo:
    case ModuloRegistroDigitalVoto::TipoVoto::brancoAposSuspensao:
    case ModuloRegistroDigitalVoto::TipoVoto::nuloAposSuspensao:
    case ModuloRegistroDigitalVoto::TipoVoto::nuloPorRepeticao:
    case ModuloRegistroDigitalVoto::TipoVoto::nuloCargoSemCandidato:
    case ModuloRegistroDigitalVoto::TipoVoto::nuloAposSuspensaoCargoSemCandidato:
        return static_cast<md::CVoto::ETipo>(tipo.asInt());   // identity mapping (1..9)
    case -1:
        // ? Every enum "Desconverte" of this unit has an explicit case for the value -1, thrown from a line
        //   inside the switch. It is probably an extra "invalid/unknown" enumerator that TSE's ASN.1 code
        //   generator adds to each NamedNumber; its name is not in the binary.
        throw CDadosError(7974, std::format("Valor inválido para tipo voto eleitor: {}", tipo.asInt()));   // line 71
    }
    throw CDadosError(7975, std::format("Valor inválido para tipo voto eleitor: {}", tipo.asInt()));       // line 75
}

// wasm func 11354 (vtable slot 3; the tool named it after the inlined DesconverteTipo, srcloc lines 71/75)
md::CVoto CConversorVoto::DoDesconverte(const TEntidade& voto) const
{
    const auto tipo = DesconverteTipo(voto.get_tipoVoto());   // ENUMERATED copied to a temporary first
    // An absent digitacao becomes an empty VotoDigitado (NumericString) and therefore an empty string.
    const ModuloRegistroDigitalVoto::VotoDigitado digitacao =
        voto.digitacao_isPresent() ? voto.get_digitacao() : ModuloRegistroDigitalVoto::VotoDigitado("");
    return md::CVoto(tipo, std::string(digitacao.getValue()));   // md::CVoto::CVoto(ETipo, const std::string&)
}

// wasm func 11355 (vtable slot 2; the tool named it after the inlined ConverteTipo, srcloc line 44)
ModuloRegistroDigitalVoto::Voto CConversorVoto::DoConverte(const TDado& voto) const
{
    ModuloRegistroDigitalVoto::Voto entidade;
    entidade.set_tipoVoto(ConverteTipo(voto.GetTipo()));
    if (voto.GetDigitacao().empty()) {
        entidade.omit_digitacao();                              // removeOptionalField(0)
    } else {
        entidade.set_digitacao(voto.GetDigitacao());            // includeOptionalField(0, 1) + string assign
    }
    return entidade;
}

} // namespace comum::asn

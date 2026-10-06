// uenux2/src/app/comum/asn/cconversorcabecalhoentidade.cpp
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/asn/cconversorcabecalhoentidade.h"

#include <format>
#include <utility>

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11589 (vtable slot 2; srcloc line 35).
// dataGeracao is formatted by Utils::ConverteDataHoraJE (wasm 1080: CDate::Format("YYYYMMDD") + "T" +
// CTime::Format("hhmmss")). The id type selects the IDEleitoral alternative (index = ETipoCabecalho).
ModuloTiposEleitorais::CabecalhoEntidade CConversorCabecalhoEntidade::DoConverte(const md::CCabecalhoEntidade& cabecalho) const
{
    ModuloTiposEleitorais::CabecalhoEntidade entidade;
    entidade.set_dataGeracao(Utils::ConverteDataHoraJE(cabecalho.GetDataGeracao()));
    const auto tipo = cabecalho.GetTipo();                                    // +16
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CUeComumAsnError(EUeComumAsnError(7651),
                               std::format("Tipo inválido [{}]", std::to_underlying(tipo)));                // line 35
    }
    // CHOICE::select(index, INTEGER::create(info @1556824 = INTEGER (0..99999))) then set the value.
    entidade.ref_idEleitoral().select(static_cast<int>(tipo), ASN1::INTEGER::create()).setValue(cabecalho.GetId());
    return entidade;
}

// wasm func 11588 (vtable slot 3; srcloc line 56). Observed executing (every data file read at votaInit).
// A CHOICE with no alternative selected has choiceID -1, which the unsigned compare also rejects.
md::CCabecalhoEntidade CConversorCabecalhoEntidade::DoDesconverte(const ModuloTiposEleitorais::CabecalhoEntidade& cabecalho) const
{
    const auto& id = cabecalho.get_idEleitoral();
    const int tipo = id.currentSelection();                                   // CHOICE +12
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CUeComumAsnError(EUeComumAsnError(7652), "Tipo de id de cabeçalho inválido");            // line 56
    }
    const uedword valor = static_cast<const ASN1::INTEGER&>(id.getSelection()).getValue();
    return md::CCabecalhoEntidade(Utils::DesconverteDataHoraJE(cabecalho.get_dataGeracao()), valor,
                                  static_cast<md::ETipoCabecalho>(tipo));   // func 1945 (ccabecalhoentidade.cpp:24)
}

} // namespace comum::asn

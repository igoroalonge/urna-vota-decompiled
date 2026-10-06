// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorajustedatahora.cpp
// Reconstructed from vota_web_wasm.wasm (unit u03).
//
// Clock-adjustment record kept in eg.bin (EstadoGeralUrna.ajusteDataHora):
//   AjusteDataHora ::= SEQUENCE { tipoAjusteDataHora TipoAjusteDataHora, valorAjusteDataHora INTEGER }
//   TipoAjusteDataHora ::= ENUMERATED { semAjusteDataHora(1), alterarDataEleicao(2), alterarDataSistema(3) }
// md ETipoDeltaT is the same list numbered 0..2.
#include "comum/dados/asn/estadoaplicacao/cconversorajustedatahora.h"

#include <format>

namespace comum::asn {

using md::estadoaplicacao::CAjusteDataHora;

// Inlined into wasm func 11405 (srcloc line 69)
ModuloEstadoGeralUrna::TipoAjusteDataHora
CConversorAjusteDataHora::ConverteTipoAjusteDataHora(const CAjusteDataHora::ETipoDeltaT& tipo) const
{
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CDadosError(7908, std::format("Valor inválido para ETipoDeltaT: {}", tipo));   // line 69
    }
    return ModuloEstadoGeralUrna::TipoAjusteDataHora(static_cast<int>(tipo) + 1);
}

// Inlined into wasm func 11406 (srcloc lines 49, 53)
CAjusteDataHora::ETipoDeltaT
CConversorAjusteDataHora::DesconverteTipoAjusteDataHora(const ModuloEstadoGeralUrna::TipoAjusteDataHora& tipo) const
{
    switch (tipo.asInt()) {
    case ModuloEstadoGeralUrna::TipoAjusteDataHora::semAjusteDataHora:  return CAjusteDataHora::ETipoDeltaT{0};  // names of the
    case ModuloEstadoGeralUrna::TipoAjusteDataHora::alterarDataEleicao: return CAjusteDataHora::ETipoDeltaT{1};  // md enumerators
    case ModuloEstadoGeralUrna::TipoAjusteDataHora::alterarDataSistema: return CAjusteDataHora::ETipoDeltaT{2};  // are unknown
    case -1:   // ? generated "invalid" enumerator, see cconversorvoto.cpp
        throw CDadosError(7906, std::format("Valor inválido para TipoAjusteDataHora: {}", tipo.asInt()));   // line 49
    }
    throw CDadosError(7907, std::format("Valor inválido para TipoAjusteDataHora: {}", tipo.asInt()));       // line 53
}

// wasm func 11405 (vtable slot 2; named after the inlined ConverteTipoAjusteDataHora)
ModuloEstadoGeralUrna::AjusteDataHora CConversorAjusteDataHora::DoConverte(const TDado& ajuste) const
{
    ModuloEstadoGeralUrna::AjusteDataHora entidade;
    entidade.set_valorAjusteDataHora(ajuste.GetValor());                          // +4, stored first
    entidade.set_tipoAjusteDataHora(ConverteTipoAjusteDataHora(ajuste.GetTipo()));  // +0
    return entidade;
}

// wasm func 11406 (vtable slot 3; named after the inlined DesconverteTipoAjusteDataHora)
CAjusteDataHora CConversorAjusteDataHora::DoDesconverte(const TEntidade& ajuste) const
{
    return CAjusteDataHora(DesconverteTipoAjusteDataHora(ajuste.get_tipoAjusteDataHora()),
                           ajuste.get_valorAjusteDataHora());   // md ctor = func 1081
}

} // namespace comum::asn

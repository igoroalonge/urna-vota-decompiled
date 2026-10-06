// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/correspondencia/ccorrespondenciaresultado.cpp
//
// CCorrespondenciaResultado = "which urna/section produced this result file":
// municipality, zone, section, the CCarga and the urna type. It is the md side of
// ModuloTiposResultadosEcoUrna::CorrespondenciaResultado and is written into the envelope of every
// result file: BU (CGravadorBU::vf7), RDV (CGravadorRDV::vf7), generic envelopes
// (IGravadorEnvelope::vf7), and read back by CConversorCorrespResultado::DoDesconverte.
// Layout: +0 TMunicipioID (int), +4 TZonaID (uint16), +6 TSecaoID (uint16), +8 CCarga (76 bytes),
//         +84 ETipoUrna (int; valid values '1'..'4', i.e. ASCII codes 49..52).
#include "ccorrespondenciaresultado.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 2801: the constructor with ValidaCriacao() inlined (it returns `this`; the analysis tools
// named it after the inlined ValidaCriacao srclocs, lines 42, 47, 52, 57).
CCorrespondenciaResultado::CCorrespondenciaResultado(TMunicipioID municipio, TZonaID zona, TSecaoID secao,
                                                     const CCarga& carga, ETipoUrna tipoUrna)
    : m_municipio(municipio), m_zona(zona), m_secao(secao), m_carga(carga), m_tipoUrna(tipoUrna)
{
    ValidaCriacao();
}

void CCorrespondenciaResultado::ValidaCriacao() const
{
    if (m_municipio >= 100000)
        throw CUeComumDadosError(8027, std::format("Município inválido: {}", m_municipio));   // line 42
    if (m_zona >= 10000)
        throw CUeComumDadosError(8028, std::format("Zona inválida: {}", m_zona));             // line 47
    if (m_secao >= 10000)
        throw CUeComumDadosError(8029, std::format("Seção inválida: {}", m_secao));           // line 52
    const int tipo = static_cast<int>(m_tipoUrna);
    if (tipo < '1' || tipo > '4')                        // compiled as (tipo - 53) <=u -5
        throw CUeComumDadosError(8030, std::format("Tipo de urna inválido: {}", tipo));       // line 57
}

}  // namespace comum::md

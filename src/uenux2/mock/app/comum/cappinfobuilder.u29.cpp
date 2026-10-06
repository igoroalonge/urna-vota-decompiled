// FRAGMENT of uenux2/mock/app/comum/cappinfobuilder.cpp (+ cappinfobuilder.h) reconstructed by unit u29 from
// vota_web_wasm.wasm. See cappinfobuilder.u28.h / .u28.cpp (unit u28) for the class, SalvaGeral and SalvaApps;
// the constructor and the fluent setters are unit u30's.
//
// comum::teste::CAppInfoBuilder is a TEST FIXTURE of the urna code base used in production by the web build:
// votaInit (func 7840) fills one from the page's options and saves eg.bin, gap.bin, sa.bin and vota.bin on both
// flash memories.

#include <format>
#include <source_location>
#include <stdexcept>
#include <vector>

#include "mock/app/comum/cappinfobuilder.u28.h"

namespace comum::teste {

// ------------------------------------------------------------------------------------------------
// wasm func 5344. EMidia -> EFlashOrigem (same numeric values). The source_location is the CALLER's (default
// argument), so the error names the caller's line: cappinfobuilder.cpp:112 (SalvaGeral) or :277 (SalvaApps).
// The enum is formatted with a custom std::formatter (handle, table slot 468 -> func 9935).
// ------------------------------------------------------------------------------------------------
EFlashOrigem Converte(EMidia midia, const std::source_location& local)
{
    if (static_cast<unsigned>(midia) >= 2)
        throw std::logic_error(std::format("Converte - midia invalida [{}] da linha [{}]",   // @235349
                                           midia, local.line()));
    return static_cast<EFlashOrigem>(midia);
}

// ------------------------------------------------------------------------------------------------
// wasm func 6004: body shared (wasm-opt merge-similar-functions) by two thunks of unit u30 that differ only in
// the turno constant:  func 10242 -> 6004(..., '1')  and  func 10239 -> 6004(..., '2').
// Both are called by func 10256 (the builder's "save everything" of votaInit) right after SalvaGeral.
// Both vectors are taken BY VALUE (10256 copies `midias` with func 2720 before each call, as it does for
// SalvaGeral(bool, std::vector<EMidia>), whose by-value signature is attested by srcloc cappinfobuilder.cpp:112;
// u30 declares the pair the same way). The body copies them once more (func 2720 -> merged body 6005 for
// std::vector<EMidia>, func 10221 -> 6005 for std::vector<EApp>) for the by-value parameters of
// SalvaApps(bool, std::vector<EMidia>, EUrnaTurno, std::vector<EApp>) (srcloc :277): the lvalues are copied,
// not moved.
// Source shape (two small members, names inferred):
// ------------------------------------------------------------------------------------------------
CAppInfoBuilder& CAppInfoBuilder::SalvaAppsPrimeiroTurno(bool assina, std::vector<EMidia> midias,
                                                         std::vector<EApp> apps)              // func 10242 -> 6004
{
    return SalvaApps(assina, midias, EUrnaTurno::Primeiro, apps);                              // func 10212
}

CAppInfoBuilder& CAppInfoBuilder::SalvaAppsSegundoTurno(bool assina, std::vector<EMidia> midias,
                                                        std::vector<EApp> apps)               // func 10239 -> 6004
{
    return SalvaApps(assina, midias, EUrnaTurno::Segundo, apps);
}

// ------------------------------------------------------------------------------------------------
// Copy helpers. Before saving, the builder rebuilds every state through the public field-wise constructors of
// the md classes (the CriaEstado family of u28/u30: 10205, 10123, 10101, 10067, and 10155 for
// CDadoCorrespondencia). Two of the leaf helpers are in this unit; their only callers are those functions,
// hence the placement in this file (the tools also attribute them to cappinfobuilder.cpp).     names inferred
// ------------------------------------------------------------------------------------------------
namespace {

// wasm func 5324 (table slot 452). Callers: 10067 (copy of CEstadoGeralVota) and 10123 (CEstadoGeralGap).
md::estadoaplicacao::CNumViasImpressasRelatorios Copia(const md::estadoaplicacao::CNumViasImpressasRelatorios& o)
{
    return md::estadoaplicacao::CNumViasImpressasRelatorios(o.GetNumViasEstadoUrna(), o.GetNumViasEleitores(),
                                                            o.GetNumViasVersoesDados(), o.GetNumViasPU());  // 5629
}

// wasm func 9862 (table slot 449). Caller: 10155 (copy of CDadoCorrespondencia).
ecourna::app::dados::CIdentificadorGeradorMidia Copia(const ecourna::app::dados::CIdentificadorGeradorMidia& o)
{
    return ecourna::app::dados::CIdentificadorGeradorMidia(o.GetNome(), o.GetSerialCertificadoTPM(),
                                                           o.GetSerialInstalacao());                        // 5109
}

}  // namespace

// ------------------------------------------------------------------------------------------------
// wasm func 5644: the implicit destructor. Only three members have non-trivial destructors:
//   m_gap[1] (+224) and m_gap[0] (+180): their std::vector<CDadoCorrespondencia> (func 1698 each),
//   m_geral (+0): ~CEstadoGeral (func 2793).
// m_sa[2] (+268) and m_vota[2] (+300) are trivially destructible.
// Called by votaInit when the builder goes out of scope (and on its exception paths).
// ------------------------------------------------------------------------------------------------
CAppInfoBuilder::~CAppInfoBuilder() = default;

}  // namespace comum::teste

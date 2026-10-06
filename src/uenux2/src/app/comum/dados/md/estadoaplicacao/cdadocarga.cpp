// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/estadoaplicacao/cdadocarga.cpp
#include "cdadocarga.h"

#include <format>

namespace comum::md::estadoaplicacao {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5633: constructor with ValidaModelo inlined (tools named it after the srcloc, line 84).
// Callers: comum::asn::CConversorDadoCarga::DesconverteModelo (11402) and the simulator's
// hard-coded eg.bin fixture (mock_f10167).
CDadoCarga::CDadoCarga(int turno, int tipoUrnaT1, int tipoUrnaT2, EUrnaModelo modelo, EFase fase)
    : m_turno(turno), m_tipoUrnaT1(tipoUrnaT1), m_tipoUrnaT2(tipoUrnaT2), m_modelo(modelo), m_fase(fase)
{
    ValidaModelo(m_modelo);
}

void CDadoCarga::ValidaModelo(const EUrnaModelo modelo)
{
    const int m = static_cast<int>(modelo);
    if (m < 2013 || m > 2022)                        // compiled as (m - 2023) <=u -11
        // the enum itself is the format argument: arg type 15 (__handle) with the shared enum formatter
        // lambda (func 536, prints the underlying integer), not an int argument
        throw CUeComumDadosError(8075, std::format("Modelo inválido: {}", modelo));
}

// wasm func 2253 (srcloc line 108). The phase letter used in result-file names and package names:
// '1' -> 'o' (oficial), '2' -> 's' (simulado), '3' -> 't' (treinamento). The wasm uses the packed
// constant 0x74736F ("ost") shifted by 8*(fase - '1').
char CDadoCarga::GetFaseChar() const
{
    switch (m_fase) {
    case EFase::OFICIAL:     return 'o';
    case EFase::SIMULADO:    return 's';
    case EFase::TREINAMENTO: return 't';
    }
    throw CUeComumDadosError(8077, std::format("Fase inválida: {}", m_fase));
}

}  // namespace comum::md::estadoaplicacao

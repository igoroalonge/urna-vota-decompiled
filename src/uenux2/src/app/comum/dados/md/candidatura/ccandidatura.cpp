// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/candidatura/ccandidatura.cpp
//
// CCandidatura = one candidacy on the ballot (ModuloCandidatos::Candidatura in the *-ca.dat files):
// office, party, ballot number, the titular's data and the ordered list of substitutes
// (suplentes / vices). Built by comum::asn::CConversorCandidatura::DoDesconverte (func 11448).
//
// Layout (sizeof 72):
//   +0 TCargoID cargo (uint8)   +2 TPartidoID partido (uint16)   +4 TCandidatoID numero (uint32)
//   +8 CDadosCandidato titular (52 bytes)   +60 std::vector<CDadosCandidato> suplentes
// CDadosCandidato (52 bytes): +0 nome, +12 nomeUrna, +24 optional<string> nomeSocial (+36 flag),
//   +40 8 bytes (sexo / situação ?), +48 int8 ordemSuplencia (0 = titular, 1..9 = suplente/vice).
#include "ccandidatura.h"

#include <format>

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;
}

// wasm func 5660 (srclocs lines 37, 40, 43, 46, 49, 55). Observed executing (data load).
CCandidatura::CCandidatura(const TCargoID cargo, const TPartidoID partido, const TCandidatoID numero,
                           const CDadosCandidato& titular, const std::vector<CDadosCandidato>& suplentes)
    : m_cargo(cargo), m_partido(partido), m_numero(numero), m_titular(titular), m_suplentes(suplentes)
{
    if (m_cargo >= 100)
        throw CUeComumDadosError(7991, "Número do cargo inválido");          // line 37
    if (m_partido >= 100)
        throw CUeComumDadosError(7992, "Partido inválido");                  // line 40
    if (m_numero >= 100000)
        throw CUeComumDadosError(7993, "Número do candidato inválido");      // line 43
    if (m_titular.GetOrdemSuplencia() != 0)
        throw CUeComumDadosError(7994, "Titular é suplente");                // line 46
    if (m_suplentes.size() > 9)
        throw CUeComumDadosError(7995, "Quantidade de suplentes inválida");  // line 49
    for (std::size_t i = 0; i < m_suplentes.size(); ++i) {
        const auto ordem = m_suplentes.at(i).GetOrdemSuplencia();            // int8
        if (ordem != static_cast<std::int8_t>(i + 1))
            // format args: {ordem (int), nomeUrna (string_view)} - packed type word 419 = 3 | 13<<5,
            // values[0] = ordem, values[1] = nomeUrna (+12 of the substitute), e.g. "(2 [FULANO])"
            throw CUeComumDadosError(7996, std::format("Ordem de suplente errada ({} [{}])",
                                                       ordem, m_suplentes.at(i).GetNomeUrna()));   // line 55
    }
}

// wasm func 1389 (srcloc line 65). ordem = 1..N (1 = first substitute / vice).
const CDadosCandidato& CCandidatura::GetSuplente(uebyte ordem) const
{
    if (ordem == 0 || ordem > m_suplentes.size())
        throw CUeComumDadosError(7997, std::format("Suplente inexistente: {}", ordem));
    return m_suplentes[ordem - 1];
}

}  // namespace comum::md

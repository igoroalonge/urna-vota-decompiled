// uenux2/src/app/comum/dados/md/processoeleitoral/cpleito.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by std::source_location records
// :64, :67, :70 (constructor), :139 (GetEleicao), :175 (GetSituacoesEleicoes). GetVersaoPacoteEleicao
// (:158, func 5604) is listed in another unit.
//
// comum::md::CPleito - one round ("pleito": 1st or 2nd turno) of the processo eleitoral. 60 bytes:
//   +0  TPleitoID m_id                    +4  std::string m_nome        +16 api::CDate m_data (8 bytes)
//   +24 TVectorEleicaoPE m_eleicoes       (std::vector<CEleicaoPE>, 52-byte elements)
//   +36 TMapVersaoPacoteEleicao m_versoes (std::map<TEleicaoID, std::string>)
//   +48 std::vector<CSituacoesEleicoes>   (8-byte trivially-copyable elements with 7 bytes of data:
//        {idEleicao; tipo; ordemAquisicao; ordemImpressao; ...} - the copy is a memmove of n*8-1 bytes,
//        libc++ 21's __datasizeof optimisation, not a bug)
#include "comum/dados/md/processoeleitoral/cpleito.h"

#include <algorithm>
#include <format>
#include <set>

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum::md {

// wasm func 5652 (srcloc :64, :67, :70) - observed executing (built from t02400-cp.dat / -ste.dat)
CPleito::CPleito(const TPleitoID id, const std::string& nome, const api::CDate& data,
                 const TVectorEleicaoPE& eleicoes, const TMapVersaoPacoteEleicao& versoes,
                 const std::vector<CSituacoesEleicoes>& situacoes)
    : m_id(id), m_nome(nome), m_data(data), m_eleicoes(eleicoes), m_versoes(versoes), m_situacoes(situacoes)
{
    std::set<TEleicaoID> ids;
    for (const CEleicaoPE& eleicao : m_eleicoes)
        ids.insert(eleicao.GetId());
    if (ids.size() != m_eleicoes.size())
        throw CDadosError(EUeComumDadosError{8159}, "Há eleições com identificadores repetidos.");   // :64
    if (ids.size() > m_versoes.size())
        throw CDadosError(EUeComumDadosError{8160}, "Há eleições sem versão de pacotes.");           // :67
    if (ids.size() != m_situacoes.size())
        throw CDadosError(EUeComumDadosError{8161}, "Há eleições com situações inválidas.");         // :70
    // (func 780, named after RHVoice by the tools, is the std::set<unsigned> node destructor here)
}
// NOTE: only the COUNTS are compared: a version-map or situations entry for an unknown eleição is not
// detected, as long as the sizes match.

// wasm func 5651 (srcloc :139) - linear search
const CEleicaoPE& CPleito::GetEleicao(const TEleicaoID id) const
{
    const auto it = std::ranges::find(m_eleicoes, id, &CEleicaoPE::GetId);
    if (it == m_eleicoes.end())
        throw CDadosError(EUeComumDadosError{8162}, std::format("Eleição não encontrada: {}", id));   // :139
    return *it;
}

// wasm func 5648 (srcloc :175) - linear search
const CSituacoesEleicoes& CPleito::GetSituacoesEleicoes(const TEleicaoID id) const
{
    const auto it = std::ranges::find(m_situacoes, id, &CSituacoesEleicoes::GetIdEleicao);
    if (it == m_situacoes.end())
        throw CDadosError(EUeComumDadosError{8164}, std::format("Situação eleição não encontrada: {}", id));   // :175
    return *it;
}

} // namespace comum::md

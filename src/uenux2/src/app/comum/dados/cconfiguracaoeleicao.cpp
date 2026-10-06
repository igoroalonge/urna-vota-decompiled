// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp
//
// See ccargos.cpp for the error types (CErroDados / CErroPattern).
// CreateInst (lines 103, 117, 130) and the constructor (line 239) are inlined into the start-up
// function 7787 (unit u02) and are not reconstructed here.
#include "cconfiguracaoeleicao.h"

#include <algorithm>
#include <format>

#include "md/processoeleitoral/ccargo.h"
#include "md/processoeleitoral/celeicaope.h"

namespace comum {

using CErroDados   = ecourna::api::exception::CBaseError<EUeComumDadosError>;
using CErroPattern = ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternErr>;

std::mutex             CConfiguracaoEleicao::s_mutex;
CConfiguracaoEleicao*  CConfiguracaoEleicao::s_inst = nullptr;

// wasm func 187 (srcloc line 36). Observed executing during the recorded votes.
CConfiguracaoEleicao& CConfiguracaoEleicao::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (s_inst == nullptr)
        throw CErroPattern(ecourna::api::pattern::EPatternErr{1303}, "CConfiguracaoEleicao - instancia nao criada");
    return *s_inst;
}

// cconfiguracaoeleicao.cpp:293 - exists only inlined in CCargos::GetCurrentEleicao (wasm 2836).
// Linear search in the pleito's elections (52-byte md::CEleicaoPE, id at +0).
const md::CEleicaoPE& CConfiguracaoEleicao::GetEleicao(const TEleicaoID id) const
{
    const auto& eleicoes = m_pleito.GetEleicoes();
    const auto it = std::ranges::find_if(eleicoes, [id](const md::CEleicaoPE& e) { return e.GetId() == id; });
    if (it == eleicoes.end())
        throw CErroDados(EUeComumDadosError{7826}, std::format("Eleição não encontrada {}", id));
    return *it;
}

// wasm func 861 (srcloc line 316). Observed executing during the recorded votes.
// Searches every election of the pleito; CEleicaoPE::GetCargos() itself throws 8154
// "Eleição não tem cargos definidos." when an election has no cargo.
const md::CCargo& CConfiguracaoEleicao::GetCargo(const TCargoID id) const
{
    for (const md::CEleicaoPE& eleicao : m_pleito.GetEleicoes()) {
        for (const md::CCargo& cargo : eleicao.GetCargos())     // 140-byte md::CCargo, codigo at +0
            if (cargo.GetCodigo() == id)
                return cargo;
    }
    throw CErroDados(EUeComumDadosError{7827}, std::format("Cargo não encontrado {}", id));
}

}  // namespace comum

// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/chv.cpp
//
// See ccargos.cpp for the error types. CreateInst (lines 32, 47, 54, 68: "instância já criada",
// missing -cm.dat file...) is inlined into the start-up function 7787 (unit u02).
#include "chv.h"

#include <format>

namespace comum {

using CErroDados   = ecourna::api::exception::CBaseError<EUeComumDadosError>;
using CErroPattern = ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternErr>;

std::mutex CHV::s_mutex;
CHV*       CHV::s_inst = nullptr;

// wasm func 2817 (srcloc line 27)
CHV& CHV::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (s_inst == nullptr)
        throw CErroPattern(ecourna::api::pattern::EPatternErr{1303}, "CHV - instancia nao criada");
    return *s_inst;
}

// chv.cpp:117 - only exists inlined into GetHorarioVerao below.
void CHV::VerificaEntraEmHorarioVerao(const std::string& funcao) const
{
    if (!m_hv.TemHorarioVerao())                                          // flag +24
        throw CErroDados(EUeComumDadosError{7870}, std::format("{} - não entra em horário de verão", funcao));
}

// wasm func 5745 - name inferred (the srcloc record at line 117 belongs to the inlined check; the
// function returns `this + 4` and passes its own name "GetHorarioVerao" to the check).
// md::CHorarioVeraoMunicipio::GetHorarioVerao (chorarioveraomunicipio.cpp:37, code 8005
// "Não há informação de horário de verão.") is inlined too - it re-tests the same flag.
const md::CHorarioVerao& CHV::GetHorarioVerao() const
{
    VerificaEntraEmHorarioVerao("GetHorarioVerao");
    return m_hv.GetHorarioVerao();
}

}  // namespace comum

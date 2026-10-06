// uenux2/src/app/comum/relatorios/cgeradorrellistaeleitores.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25) — only the function this unit holds.
// srcloc :100  std::string comum::(anonymous namespace)::SituacaoEleitor()          (wasm 11233)
// The "lista de eleitores" report itself is printed by vota::CImpressaoListaEleitores (wasm 11908, other
// unit), whose form uses this function as a data source (table slot 2971) for each voter of the
// CEleitores CDataMap. Not executed in the recorded votes.

#include <string>
#include <vector>

#include "api/util/cstringutils.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/md/eleitor/celeitordecorator.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

namespace {

// wasm func 11233 (srcloc :100). One line of the voters list: the voter's identity followed, right-aligned in
// 38 columns, by the flags of the voter joined by '-':
//     BIO  voter with biometrics            (CEleitorDetalhe +100)
//     IMP  voter impedido in the current turno (field +196 in the 1st, +200 in the 2nd turno, func 2266)
//     AUD  field +20 == 1 (deficiência auditiva ?)
//     TTE  transferência temporária de eleitor (field +48 != 0)
std::string SituacaoEleitor()
{
    const CEleitorDetalhe* eleitor = CEleitores::GetInst().GetEleitores().GetCurrent();   // CDataMap at +4, func 656
    if (eleitor == nullptr)
        throw CRelatoriosError(EUeComumRelatoriosError{9068}, "Eleitor não posicionado");  // :100

    std::vector<std::string> situacoes;
    if (eleitor->PossuiBiometria())                     // +100
        situacoes.push_back("BIO");
    if (!eleitor->PodeVotarNoTurno())                   // func 2266: impedimento of the turno == 0
        situacoes.push_back("IMP");
    if (eleitor->GetNecessidadeEspecial() == 1)         // +20 ?
        situacoes.push_back("AUD");
    if (eleitor->EhTransferenciaTemporaria())           // +48
        situacoes.push_back("TTE");

    const std::string identidade =
        md::CEleitorDecorator::GetIdentidadePorTipo(*eleitor, eleitor->GetTipoIdentidade()).ToString();   // 708, 1924
    const std::string flags = api::CStringUtils::Join(situacoes, "-");                                  // shared_f1696
    // NOTE: no guard: if identity + flags exceed 38 characters, 38 - size wraps and std::string(n, ' ')
    // throws std::length_error.
    return identidade + std::string(38 - (identidade.size() + flags.size()), ' ') + flags;
}

} // namespace

} // namespace comum

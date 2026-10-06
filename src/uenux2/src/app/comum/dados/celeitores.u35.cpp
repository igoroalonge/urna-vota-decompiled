// FRAGMENT of uenux2/src/app/comum/dados/celeitores.cpp (attested; file of unit u04, declarations in celeitores.h).
// Reconstructed from vota_web_wasm.wasm (unit u35). CEleitores = the voter roll of the section; the map
// (CDataMap base at +4: root +8, begin +4, current iterator +16) holds one CEleitorDetalhe per voter (node value +32).
//
// Counters used by the BU (boletim de urna) and its QR code. They only count voters WITH dynamic data
// (CEleitorDetalhe +192 engaged, i.e. rows of the SQLite table eleitor_dinamico) whose estado is VOTOU (3):
//   BU DetalhamentoComparecimento.qtdEleitoresCompareceramSemBiometria  <- GetQtdCompareceramSemBiometria (2822)
//                                .qtdEleitoresHabilitadosPorBiometria   <- GetQtdHabilitadosPorBiometria   (2821)
//                                .qtdEleitoresHabilitadosPorBiografia   <- GetQtdHabilitadosPorBiografia   (1935)
//   QR code fields HBSB / HBBM / HBBG; printed BU lines "Habilitação sem biometria / biométrica / biográfica".
// md::ETipoHabilitacao (CEleitorDinamico +36): 0 sem biometria, 1 biometria (fingerprint recognised), 2 biografia
// (enabled by the poll worker after the voter answered the birth year).
#include "comum/dados/celeitores.h"

#include <string>

namespace comum {

// wasm func 2821 - thunk: QtdVotaramPorTipoHabilitacao(1) (func 6034, unit u04).                name inferred
std::uint16_t CEleitores::GetQtdHabilitadosPorBiometria() const
{
    return QtdVotaramPorTipoHabilitacao(md::ETipoHabilitacao(1));
}

// wasm func 1935 - thunk: QtdVotaramPorTipoHabilitacao(2).                                      name inferred
std::uint16_t CEleitores::GetQtdHabilitadosPorBiografia() const
{
    return QtdVotaramPorTipoHabilitacao(md::ETipoHabilitacao(2));
}

// wasm func 2822 - the same loop as func 6034 with the constant 0 (compiled separately, not a thunk). name inferred
std::uint16_t CEleitores::GetQtdCompareceramSemBiometria() const
{
    std::uint16_t qtd = 0;
    for (const auto& [id, detalhe] : GetContainer()) {
        if (!detalhe.TemDinamico())
            continue;
        if (detalhe.GetDinamico().GetTipoHabilitacao() != md::ETipoHabilitacao(0))
            continue;
        qtd += detalhe.GetDinamico().GetEstadoComparecimento() == md::EEstadoComparecimento::VOTOU;   // func 1271
    }
    return qtd;
}

// wasm func 2823 - name inferred (u04 calls it GetQtdAptosSecao). Not observed executing.
// "Eleitores aptos": voters of the roll WITHOUT impediment in the current turno (CEleitorDetalhe::PodeVotar,
// func 2266), split into the section's own voters and voters in temporary transfer (any non-zero
// CEleitorDecorator +48 = tipo de transferência temporária, the accessibility transfer included).
// Used for the BU header / QR "APTS:" "APTT:" "APTO:" and the printed lines of CRelUtil::FormataQtdAptos (1921).
SQtdeAptos CEleitores::GetQtdAptosSecao() const
{
    SQtdeAptos aptos{};                                                          // {secao, tte} = {0, 0}
    for (const auto& [id, detalhe] : GetContainer()) {
        if (!detalhe.PodeVotar())
            continue;
        if (detalhe.GetEleitor().GetTipoTransferenciaTemporaria() == 0)
            ++aptos.qtdAptosSecao;
        else
            ++aptos.qtdAptosTTE;
    }
    return aptos;
}

// wasm func 2264 - name as used by u27 (CProcuraEleitor). Not observed executing in the recorded votes.
// Finds a voter by any of its identities (título, CPF or free identifier), makes it the CURRENT voter of the roll
// (cursor +16, used by the poll worker's screens and CEleitorDadoNomeParaUrna...) and returns it; nullptr when the
// identity is unknown (the cursor is then left where it was).
// Callers: CProcuraEleitor::StartState (10631), CControladorRegistraMesariosVota slots 13/14 (10789/10790),
// CImprimirIdentificacaoMesariosFinal::GetNomeMesario (5385), CBiometriaEleitor::GetFoto (3616).
const CEleitorDetalhe* CEleitores::Procura(const std::string& identidade,
                                           ecourna::app::dados::ETipoIdentificadorEleitor tipo)
{
    // md::CEleitorIdentidade's constructor (func 566) pads the digits (11 for CPF, 12 otherwise) and validates them
    // with CValidadorIdentidade.
    const md::CEleitorIdentidade id(identidade, tipo);
    const auto principal = m_identidadePrincipal.find(id);                      // +112: any identity -> principal
    if (principal == m_identidadePrincipal.end())
        return nullptr;
    const auto it = GetContainer().find(principal->second);                     // shared_f1703
    if (it == GetContainer().end())
        return nullptr;
    SetCurrent(it);                                                              // +16
    return &it->second;
}

} // namespace comum

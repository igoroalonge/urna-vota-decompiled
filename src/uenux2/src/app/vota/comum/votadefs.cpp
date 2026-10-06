// uenux2/src/app/vota/comum/votadefs.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records: :103, :110, :135.
#include "vota/comum/votadefs.h"

#include <filesystem>

#include "api/pattern/cpolysingletonlist.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/iinterfacesavd.h"
#include "comum/informacao/cinformacaoeleicao.h"

namespace vota {

// votadefs.cpp:103 - wasm func 3290. Caller: vota::impl::CSincronismoVotoEleitor::SincronizaVoto (7174),
// after each vote is written (vota.vsu, rdv.vsu, uenux.vsu).
// In the web build IInterfaceSavd is (anonymous)::CWasmSavd: the check always succeeds.
void VerificaAssinaturaMI(const std::string& pacote)
{
    auto& savd = api::CPolySingletonList::instance<comum::IInterfaceSavd>();              // :103 (func 1822)
    const auto turno = comum::CAppInfo::GetInst().GetGeral().GetTurno();                  // CEstadoGeral +32
    savd.ValidarUE(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA, turno) / pacote);   // func 358, comum_ValidarUE
}

// votadefs.cpp:110 - wasm func 3288 (same body, MV = external flash).
void VerificaAssinaturaMV(const std::string& pacote)
{
    auto& savd = api::CPolySingletonList::instance<comum::IInterfaceSavd>();              // :110
    const auto turno = comum::CAppInfo::GetInst().GetGeral().GetTurno();
    savd.ValidarUE(comum::CPath::GetPathTrab(comum::EFlashOrigem::EXTERNA, turno) / pacote);
}

// votadefs.cpp:135 - wasm func 4582. Callers: vota::CEmitirMaisBU::ProcessInput (unit u08) and func 12974.
//   voter-training urna (fase '3' + EstadoGeralVota.treinamentoEleitor): always 1;
//   otherwise  obrigatórias + adicionais - já impressas, where
//     obrigatórias = CInformacaoEleicao::GetNumBUVotaObrigatorios()  (func 3847: 1 in demo mode, else parâmetro +12)
//     adicionais   = CInformacaoEleicao::GetNumBUVotaAdicionais()    (func 5914: 1 in demo mode, else parâmetro +16)
//     impressas    = EstadoGeralVota.qtdBU                            (CEstadoGeralVota +8)
//   and the count of printed copies may never be below the mandatory ones (assert).
comum::TQtdImpresso GetQuantidadeMaximaBUsAdicionais()
{
    auto& appInfo = comum::CAppInfo::GetInst();
    if (appInfo.GetGeral().GetFase() == comum::EFase::Treinamento                     // +48 == '3'
        && appInfo.GetVota(comum::EUrnaTurno::Atual).EhTreinamentoEleitor())           // +72
        return 1;

    const auto& vota = appInfo.GetVota(comum::EUrnaTurno::Atual);
    const comum::CInformacaoEleicao informacao(comum::CConfiguracaoEleicao::GetInst());  // vota_f603 (cfg +88)
    const comum::TQtdImpresso numObrigatorias = informacao.GetNumBUVotaObrigatorios();   // func 3847
    const comum::TQtdImpresso numAdicionais = informacao.GetNumBUVotaAdicionais();       // func 5914
    const comum::TQtdImpresso numImpresso = vota.GetQtdBU();
    UE_ASSERT(numImpresso >= numObrigatorias);                                         // :135 (code 3449)
    return numObrigatorias - numImpresso + numAdicionais;
}

} // namespace vota

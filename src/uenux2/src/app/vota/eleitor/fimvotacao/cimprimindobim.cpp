// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobim.cpp
//
// srcloc evidence:
//   :50  StartState        Assert (vota.GetEstadoVota() == EAVENCERRADA)                     (3463)
//   :51  StartState        Assert (vota.GetEstadoEncerramento() == EAEIMPRIMIROBRIGATORIABU) (3464)
//   :66  ImprimeRelatorio  api::IPaperRelatorios lookup
// Not executed in the recorded sessions.

#include "vota/eleitor/fimvotacao/cimprimindobim.h"

#include <filesystem>
#include <memory>
#include <mutex>

#include "api/hwil/ipaperrelatorios.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/clocal.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "comum/relatorios/csigverifier.h"
#include "vota/comum/crelvotautil.h"
#include "vota/eleitor/fimvotacao/cimprimindobehb.h"
#include "vota/eleitor/fimvotacao/cretirarmr.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12071 — vtable slot 2
void CImprimindoBim::StartState()
{
    auto& vota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVENCERRADA);                            // :50
    UE_ASSERT(vota.GetEstadoEncerramento() == EEstadoEncerramento::EAEIMPRIMIROBRIGATORIABU); // :51

    ImprimeRelatorio();

    const comum::CInformacaoEleicao info(comum::CConfiguracaoEleicao::GetInst());   // built but unused
    if (!comum::CInformacaoEleicao::EhModoDemonstracao() &&                           // func 2286
        comum::CLocal::GetInst().UrnaBiometrica())                                    // func 820
        m_proximoEstado = &CImprimindoBEHB::GetInst();
    else
        m_proximoEstado = &CRetirarMR::GetInst();                                     // func 2291
}

// srcloc :66 — inlined into func 12071
void CImprimindoBim::ImprimeRelatorio()
{
    auto& papel = api::IPaperRelatorios::GetInst();                                   // :66
    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);
    const std::filesystem::path arquivo = trab / "bim.dat";
    CRelVotaUtil::MarcaImpressaoEmAndamento();                                          // func 1488
    CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BIM, 1);           // func 1127
    const comum::CSigVerifier verificador(trab, "bim.dat", "bim.vsu");                 // func 1540
    papel.ImprimeArquivo(arquivo, verificador, "boletim de mes\xE1rios", "via \xFAnica");   // slot 7
    papel.AguardaFimImpressao();                                                         // slot 10
    CRelVotaUtil::RemoveMarcaImpressaoEmAndamento();                                     // func 1487
}

// Singleton of the next state, inlined here (object @1833680, mutex residue @1833656).
CImprimindoBEHB& CImprimindoBEHB::GetInst()
{
    static std::unique_ptr<CImprimindoBEHB> s_inst;
    if (!s_inst)
        s_inst.reset(new CImprimindoBEHB());          // 12 bytes, CAppState(0)
    return *s_inst;
}

}  // namespace vota

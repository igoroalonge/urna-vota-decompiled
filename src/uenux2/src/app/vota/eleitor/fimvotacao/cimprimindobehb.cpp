// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobehb.cpp
//
// srcloc evidence:
//   :45  StartState        Assert (vota.GetEstadoVota() == EAVENCERRADA)                     (3484)
//   :46  StartState        Assert (vota.GetEstadoEncerramento() == EAEIMPRIMIROBRIGATORIABU) (3485)
//   :55  ImprimeRelatorio  api::IPaperRelatorios lookup
// Not executed in the recorded sessions.

#include "vota/eleitor/fimvotacao/cimprimindobehb.h"

#include <filesystem>

#include "api/hwil/ipaperrelatorios.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/relatorios/csigverifier.h"
#include "vota/comum/crelvotautil.h"
#include "vota/eleitor/fimvotacao/cretirarmr.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12074 — vtable slot 2
void CImprimindoBEHB::StartState()
{
    auto& vota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVENCERRADA);                            // :45
    UE_ASSERT(vota.GetEstadoEncerramento() == EEstadoEncerramento::EAEIMPRIMIROBRIGATORIABU); // :46
    ImprimeRelatorio();
    m_proximoEstado = &CRetirarMR::GetInst();                                                 // func 2291
}

// srcloc :55 — inlined into func 12074
void CImprimindoBEHB::ImprimeRelatorio()
{
    auto& papel = api::IPaperRelatorios::GetInst();                                   // :55
    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);
    const std::filesystem::path arquivo = trab / "behb.dat";
    CRelVotaUtil::MarcaImpressaoEmAndamento();                                          // func 1488
    CLogVota::GetInst().LogaImpressaoRelatorio(
        comum::ERelatoriosUE::ELEITORES_HABILITADOS_BIOGRAFICAMENTE, 1);                // 12, func 1127
    const comum::CSigVerifier verificador(trab, "behb.dat", "behb.vsu");               // func 1540
    papel.ImprimeArquivo(arquivo, verificador,
                         "boletim de eleitores\nhabilitados biograficamente", "via \xFAnica");  // slot 7
    papel.AguardaFimImpressao();                                                         // slot 10
    CRelVotaUtil::RemoveMarcaImpressaoEmAndamento();                                     // func 1487
}

}  // namespace vota

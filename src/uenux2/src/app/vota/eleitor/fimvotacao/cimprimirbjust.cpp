// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimirbjust.cpp
//
// srcloc evidence:
//   :51  StartState    Assert (vota.GetEstadoVota() == EAVENCERRADA)                     (3468)
//   :52  StartState    Assert (vota.GetEstadoEncerramento() == EAEIMPRIMIROBRIGATORIABU) (3469)
//   :69  imprimeBJust  api::IPaperRelatorios lookup
// Not executed in the recorded sessions (web printer = simulador::CWasmNullPaper, slot 7 is a no-op).

#include "vota/eleitor/fimvotacao/cimprimirbjust.h"

#include <filesystem>

#include "api/hwil/ipaperrelatorios.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "comum/relatorios/csigverifier.h"
#include "vota/comum/crelvotautil.h"
#include "vota/eleitor/fimvotacao/cimprimindobim.h"
#include "vota/eleitor/fimvotacao/cretirarmr.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12068 — vtable slot 2
void CImprimirBJust::StartState()
{
    auto& vota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVENCERRADA);                          // :51
    UE_ASSERT(vota.GetEstadoEncerramento() == EEstadoEncerramento::EAEIMPRIMIROBRIGATORIABU); // :52

    CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BUJ, 1);   // func 1127
    imprimeBJust();

    if (comum::CInformacaoEleicao(comum::CConfiguracaoEleicao::GetInst()).IdentificaMesarios())  // func 1950
        m_proximoEstado = &CImprimindoBim::GetInst();                           // func 5985
    else
        m_proximoEstado = &CRetirarMR::GetInst();                               // func 2291
}

// srcloc :69 — inlined into func 12068
void CImprimirBJust::imprimeBJust()
{
    auto& papel = api::IPaperRelatorios::GetInst();                                   // :69
    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);
    const std::filesystem::path arquivo = trab / "buj.dat";
    CRelVotaUtil::MarcaImpressaoEmAndamento();                                          // func 1488
    const comum::CSigVerifier verificador(trab, "buj.dat", "buj.vsu");                 // func 1540
    papel.ImprimeArquivo(arquivo, verificador, "boletim de justificativa eleitoral", "via \xFAnica");  // slot 7
    papel.AguardaFimImpressao();                                                         // slot 10
    CRelVotaUtil::RemoveMarcaImpressaoEmAndamento();                                     // func 1487
}

}  // namespace vota

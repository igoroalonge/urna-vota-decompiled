// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindoresumozeresima.cpp
//
// srcloc evidence:
//   :60  void ImprimeResumoZeresima()   api::IPaperRelatorios lookup (inlined into func 11855)

#include "vota/eleitor/iniciovotacao/creimprimindoresumozeresima.h"

#include "comum/cappinfo.h"
#include "vota/eleitor/cdefinerotaposreinicio.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

void ImprimeViasRelatorio(const char* arquivo, const char* assinatura, comum::ERelatoriosUE relatorio,
                          const char* mensagem);      // see cimprimindozeresima.cpp

// wasm func 11855 — vtable slot 2 (analyzer name ...::ImprimeResumoZeresima)
void CReimprimindoResumoZeresima::StartState()
{
    const auto tela = CriaTelaAguarde();                   // func 6587 "Por favor, aguarde..."
    tela->Exibe();
    ImprimeResumoZeresima();
    comum::CAppInfo::GetInst().GetVota().SetEstadoVota(EEstadoVota::EAVZERESIMAIMPRESSA);   // 54
    comum::SalvaEstado();                                                                   // func 491
    m_proximoEstado = &CDefineRotaPosReinicio::GetInst();                                   // func 5943
}

void CReimprimindoResumoZeresima::ImprimeResumoZeresima()                                   // :60
{
    ImprimeViasRelatorio("rze.dat", "rze.vsu", comum::ERelatoriosUE::RESUMO_ZERESIMA /*11*/,
                         "resumo da zer\xE9sima");
}

}  // namespace vota

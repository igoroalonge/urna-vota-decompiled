// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/creimprimindozeresima.cpp (+ .h, declared here)
//
// "Reimprimindo zerésima" = reprint of trab/ze.dat after a restart (chosen with '1' in
// CQuerReimprimirZeresima), followed by the reprint of the summary.
//
// srcloc evidence:
//   :51  void ImprimeZeresima()   api::IPaperRelatorios lookup (inlined into StartState = func 11852)
//
// RTTI: comum::CAppState <- vota::CReimprimindoZeresima (typeinfo @1546328, vtable @1546276)
//   [0] 174 [1] 144 [2] StartState 11852 [3..8] defaults

#include "comum/cappstate.h"
#include "vota/eleitor/iniciovotacao/creimprimindoresumozeresima.h"
#include "vota/log/clogvota.h"

namespace vota {

void ImprimeViasRelatorio(const char* arquivo, const char* assinatura, comum::ERelatoriosUE relatorio,
                          const char* mensagem);      // see cimprimindozeresima.cpp (same inlined loop)

class CReimprimindoZeresima final : public comum::CAppState {
public:
    static CReimprimindoZeresima& GetInst();   // func 5941 (cquerreimprimirzeresima.cpp)
    void StartState() override;                // wasm func 11852
private:
    static void ImprimeZeresima();             // :51, inlined
};

// wasm func 11852 — vtable slot 2 (analyzer name vota::CReimprimindoZeresima::ImprimeZeresima)
void CReimprimindoZeresima::StartState()
{
    ImprimeZeresima();
    m_proximoEstado = &CReimprimindoResumoZeresima::GetInst();       // api_f5942
}

void CReimprimindoZeresima::ImprimeZeresima()                        // :51
{
    // identical to CImprimindoZeresima::ImprimeZeresima: trab/ze.dat, signature ze.vsu,
    // "zerésima", "via {} de {}" for via = 1..CConfiguracaoEleicao +96, ERelatoriosUE 0
    ImprimeViasRelatorio("ze.dat", "ze.vsu", comum::ERelatoriosUE::ZERESIMA, "zer\xE9sima");
}

}  // namespace vota

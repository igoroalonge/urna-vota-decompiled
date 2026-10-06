// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cimprimindozeresima.cpp (+ .h, declared here)
//
// "Imprimindo zerésima" = prints the zerésima (report proving that the RDV holds no vote before voting
// starts, generated earlier as trab/ze.dat by CGeraZeresimaBase) and its summary (trab/rze.dat, by
// CGeraResumoZeresimaBase), each in N copies (CConfiguracaoEleicao +96), then EstadoVota = zerésima
// impressa and routing to the pre-voting states (CDefineRotaPreVotacao).
//
// srcloc evidence:
//   :77   void ImprimeZeresima()        api::IPaperRelatorios lookup
//   :104  void ImprimeResumoZeresima()  api::IPaperRelatorios lookup
//   (both inlined into StartState = func 11991, which the tools named after the first one)
//
// RTTI: comum::CAppState <- vota::CImprimindoZeresima (typeinfo @1543580, vtable @1543512)
//   [0] 174 [1] 144 [2] StartState 11991 [3..8] defaults

#include <filesystem>
#include <format>
#include <memory>
#include <mutex>
#include <string>

#include "api/hwil/ipaperrelatorios.h"
#include "comum/cappinfo.h"
#include "comum/cappstate.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/relatorios/csigverifier.h"
#include "vota/comum/crelvotautil.h"
#include "vota/eleitor/iniciovotacao/cdefinerotaprevotacao.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

class CImprimindoZeresima final : public comum::CAppState {
public:
    static CImprimindoZeresima& GetInst();   // func 5973 (unit u06): vota_f764(@1834076, @1834100, vtable, 0)
    void StartState() override;              // wasm func 11991

private:
    static void ImprimeZeresima();           // :77, inlined
    static void ImprimeResumoZeresima();     // :104, inlined
};

// Shared shape of the zerésima printing loops (inlined 3 times in this unit: here, in
// CReimprimindoZeresima and in CReimprimindoResumoZeresima).                         name inferred
void ImprimeViasRelatorio(const char* arquivo, const char* assinatura, comum::ERelatoriosUE relatorio,
                          const char* mensagem)
{
    auto& papel = api::IPaperRelatorios::GetInst();                                    // func 905
    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);        // func 436
    const std::filesystem::path caminho = trab / arquivo;
    const int vias = comum::CConfiguracaoEleicao::GetInst().GetQtdViasZeresima();     // cfg +96  name inferred
    for (int via = 1; via <= vias; ++via) {
        CRelVotaUtil::MarcaImpressaoEmAndamento();                                     // func 1488
        CLogVota::GetInst().LogaImpressaoRelatorio(relatorio, static_cast<uebyte>(via));   // func 1127
        const comum::CSigVerifier verificador(trab, arquivo, assinatura);             // func 1540
        papel.ImprimeArquivo(caminho, verificador, mensagem,
                             std::format("via {} de {}", via, vias));                  // slot 7
        CRelVotaUtil::RemoveMarcaImpressaoEmAndamento();                               // func 1487
    }
    papel.AguardaFimImpressao();                                                       // slot 10
}

// wasm func 11991 — vtable slot 2 (analyzer name vota::CImprimindoZeresima::ImprimeZeresima)
void CImprimindoZeresima::StartState()
{
    const auto tela = CriaTelaAguarde();          // func 6587: "Por favor, aguarde..." (api_f2380, font 35)
    tela->Exibe();
    CRelVotaUtil::CortaPapel();                   // func 2882
    if (!comum::UrnaDesligando()) {               // @1832936
        ImprimeZeresima();
        if (!comum::UrnaDesligando()) {
            ImprimeResumoZeresima();
            if (!comum::UrnaDesligando()) {
                comum::CAppInfo::GetInst().GetVota().SetEstadoVota(EEstadoVota::EAVZERESIMAIMPRESSA);   // 54
                comum::SalvaEstado();                                                                   // func 491
                // singleton created inline: @1834072 (mutex @1834048), 16 bytes, CAppState(7), +12 = 0
                m_proximoEstado = &CDefineRotaPreVotacao::GetInst();
            }
        }
    }
}

void CImprimindoZeresima::ImprimeZeresima()                                            // :77
{
    ImprimeViasRelatorio("ze.dat", "ze.vsu", comum::ERelatoriosUE::ZERESIMA /*0*/, "zer\xE9sima");
}

void CImprimindoZeresima::ImprimeResumoZeresima()                                      // :104
{
    ImprimeViasRelatorio("rze.dat", "rze.vsu", comum::ERelatoriosUE::RESUMO_ZERESIMA /*11*/,
                         "resumo da zer\xE9sima");
}

}  // namespace vota

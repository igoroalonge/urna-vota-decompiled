// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cimprimindobu.cpp
//
// srcloc evidence:
//   :70   virtual void StartState()   Assert (vota.GetEstadoVota() == EAVIMPRIMIRBU)          (3465)
//   :71   virtual void StartState()   Assert (vota.GetQtdBU() == 0)                           (3466)
//   :111  static void ImprimeBU(uebyte, PrintMessageMode)   api::IPaperRelatorios lookup
//   :114  static void ImprimeBU(uebyte, PrintMessageMode)   Assert (estVota.GetQtdBU() == numVia - 1) (3467)
//   api/gui/cinteractiveform.h:57  CInteractiveForm<IScreen, IInputKbd>::Read(), inlined in ProcessInput
//
// Not executed in the recorded sessions. In the web build the printer is simulador::CWasmNullPaper:
// IPaperRelatorios slot 8 only releases the header shared_ptr, slot 10 does nothing.

#include "vota/eleitor/fimvotacao/cimprimindobu.h"

#include <filesystem>
#include <format>
#include <string>

#include "api/gui/cpaperformbuilder.h"
#include "api/hwil/ipaperrelatorios.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/relatorios/csigverifier.h"
#include "vota/comum/crelvotautil.h"
#include "vota/eleitor/fimvotacao/cemitirmaisbu.h"
#include "vota/eleitor/fimvotacao/cgravaresultado.h"
#include "vota/log/clogvota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;
using comum::md::estadoaplicacao::EEstadoEncerramento;

// wasm func 12078 — vtable slot 2
void CImprimindoBU::StartState()
{
    auto& vota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVIMPRIMIRBU);          // :70 (61)
    UE_ASSERT(vota.GetQtdBU() == 0);                                         // :71 (EstadoGeralVota +8)

    if (!comum::UrnaDesligando()) {                                          // global flag @1832936
        CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BU, 1);   // func 1127
        ImprimeBU(1, PrintMessageMode::ComMensagem);
        CLogVota::GetInst().LogaMesarioIndagadoQualidadeBU();                // func 4551
        if (!comum::UrnaDesligando())
            m_tela->Exibe();                                                 // form slot 2
    }
    m_proximoEstado = this;
}

// wasm func 12077 — vtable slot 7 (analyzer name vota::CImprimindoBU::vf7)
void CImprimindoBU::ProcessInput()
{
    switch (m_tela->Read()) {                                                // cinteractiveform.h:57
    case api::EInputResult::Corrige:                                         // 5: print again
        CLogVota::GetInst().Loga("Mesário indicou reimprimir Boletim de Urna");   // api_f233 (level 1)
        if (!comum::UrnaDesligando()) {
            CLogVota::GetInst().LogaImpressaoRelatorio(comum::ERelatoriosUE::BU, 1);
            ImprimeBU(1, PrintMessageMode::ComMensagem);                     // still via 1: QtdBU is 0
            CLogVota::GetInst().LogaMesarioIndagadoQualidadeBU();
            if (!comum::UrnaDesligando())
                m_tela->Exibe();
        }
        m_proximoEstado = this;
        break;

    case api::EInputResult::Confirma: {                                      // 9: print is OK
        CLogVota::GetInst().Loga("Mesário indicou qualidade OK para Boletim de Urna");
        auto& vota = comum::CAppInfo::GetInst().GetVota();
        vota.IncrementaQtdBU();                                              // func 3701 (+8 += 1)
        if (comum::EhTreinamentoEleitor()) {                                 // func 697
            // voter-training urna: no result files, straight to the end of the day
            vota.SetEstadoVota(EEstadoVota::EAVENCERRADA);                   // 64
            vota.SetEstadoEncerramento(EEstadoEncerramento::EAEFIMDOSTRABALHOS);   // 52
            comum::SalvaEstado();                                            // func 491
            m_proximoEstado = &CEmitirMaisBU::GetInst();                     // func 3879
        } else {
            vota.SetEstadoVota(EEstadoVota::EAVGRAVARRESULTADOS);            // 62
            comum::SalvaEstado();
            m_proximoEstado = &CGravaResultado::GetInst();                   // func 6037
        }
        break;
    }

    default:                                                                 // other keys: ignored
        break;
    }
}

// wasm func 2890 — srcloc :111, :114
void CImprimindoBU::ImprimeBU(uebyte numVia, PrintMessageMode modo)
{
    auto& papel = api::IPaperRelatorios::GetInst();                          // :111 (func 905)
    const auto& estVota = comum::CAppInfo::GetInst().GetVota();
    UE_ASSERT(estVota.GetQtdBU() == numVia - 1);                             // :114 (3467)

    // header printed before the stored image: "<n>a. VIA", centred, then 3 blank lines
    api::CPaperFormBuilder cabecalho;
    cabecalho.AddText(std::format("{}a. VIA", numVia), 2, 2);                // shared_f193
    cabecalho.AddNewLine(3);                                                 // func 198
    const api::SharedPaperForm formCabecalho = cabecalho.Build();            // shared_f357

    const auto trab = comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA);   // func 436
    const std::filesystem::path arquivo = trab / "bu.dat";
    CRelVotaUtil::MarcaImpressaoEmAndamento();       // func 1488: creates <root>dinamico/imprimindo  name inferred

    // the printer service verifies trab/bu.dat against the signature trab/bu.vsu before printing
    const comum::CSigVerifier verificador(trab, "bu.dat", "bu.vsu");        // func 1540

    std::string mensagem;
    std::string via;
    if (modo == PrintMessageMode::ComMensagem) {
        mensagem = "boletim de urna";
        via = std::format("{}\xAA via", numVia);                             // "{}ª via" (Latin-1)
    }
    papel.ImprimeArquivo(arquivo, verificador, formCabecalho, mensagem, via);   // IPaperRelatorios slot 8
    papel.AguardaFimImpressao();                                             // slot 10  name inferred
    CRelVotaUtil::RemoveMarcaImpressaoEmAndamento();                         // func 1487  name inferred
}

}  // namespace vota

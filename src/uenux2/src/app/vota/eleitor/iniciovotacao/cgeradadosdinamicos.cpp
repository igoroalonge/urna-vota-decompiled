// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cgeradadosdinamicos.cpp (+ .h, declared here)
//
// "Gera dados dinâmicos" = first state of a fresh urna on election day (EstadoVota inicial /
// gerabasedinamica): creates the dynamic voter database (uenux.db, table eleitor_dinamico) and the empty
// RDV, loads them, then waits for the zerésima time (CVerificaHorarioZeresima).
//
// srcloc evidence:
//   :47  Assert (vota.GetEstadoVota() == EAVINICIAL || vota.GetEstadoVota() == EAVGERADADOSDINAMICOS) (3478)
//
// RTTI: comum::CAppState <- vota::CGeraDadosDinamicos (typeinfo @1546024, vtable @1545972)
//   [0] 174 [1] 144 [2] StartState 11865 [3..8] defaults

#include "comum/cappinfo.h"
#include "comum/cappstate.h"
#include "comum/dados/celeitores.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/iniciovotacao/cverificahorariozeresima.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

class CGeraDadosDinamicos final : public comum::CAppState {
public:
    static CGeraDadosDinamicos& GetInst();   // not in this unit; 12 bytes
    void StartState() override;              // wasm func 11865 (srcloc 47)
};

// wasm func 11865 — vtable slot 2
void CGeraDadosDinamicos::StartState()
{
    auto& info = CInformacaoEleitor::GetInst();       // func 509; passed along, unused by the callees (?)
    auto& appInfo = comum::CAppInfo::GetInst();
    GeraDadosDinamicos(info);        // func 6737 (other unit): throws CUeDesligandoError if the urna is
                                     // shutting down; unless voter training: CEleitores::DynamicCreate on
                                     // trab1/trab2 uenux.db; writes the empty encrypted rdv.dat (MI and MV)
    comum::CEleitores::CompleteLoad(info);            // func 6734 (celeitores.cpp:205..238)

    auto& vota = appInfo.GetVota();
    UE_ASSERT(vota.GetEstadoVota() == EEstadoVota::EAVINICIAL ||
              vota.GetEstadoVota() == EEstadoVota::EAVGERADADOSDINAMICOS);                  // :47 (49/50)
    vota.SetEstadoVota(EEstadoVota::EAVAGUARDAHORAZERESIMA);                                // 51
    comum::SalvaEstado();                                                                   // func 491
    m_proximoEstado = &CVerificaHorarioZeresima::GetInst();                                 // func 5947
}

// func 5947 (not in this unit) = CVerificaHorarioZeresima::GetInst(): 52 bytes,
//   CEstadoComDesligamentoAutomatico(6), m_tela = CTelasVota +20/+24, cfg +544..+555 (hora da zerésima),
//   a 2000 ms tick on the thread's message queue (func 5452).

}  // namespace vota

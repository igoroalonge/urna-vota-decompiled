// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cinformacaozeresimatardia.cpp (+ .h, declared here)
//
// "Informação zerésima tardia" = reached from CImpressaoZeresimaTardia when the mesário answers that the
// urna clock is WRONG ("Mesário confirma que o horário da urna está errado"): an information screen
// whose CONFIRMA powers the urna off ("Finalização de aplicativo" / "Desligando a urna").
//
// srcloc evidence:
//   :48  ProcessInput  api::IBeep lookup
//   :51  ProcessInput  comum::IInterfaceInit lookup (DesligarUrna)
//
// RTTI: comum::CAppState <- vota::CInformacaoZeresimaTardia (typeinfo @1543724, vtable @1543656)
//   [0] ~dtor 2879 [1] deleting 11984 [2] StartState 11983 [7] ProcessInput 11982

#include <memory>

#include "api/hwil/ibeep.h"
#include "api/util/cwait.h"
#include "comum/cappstate.h"
#include "comum/iinterfaceinit.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/log/clogvota.h"

namespace vota {

class CInformacaoZeresimaTardia final : public comum::CAppState {
public:
    /// Created inline by CImpressaoZeresimaTardia::ProcessInput (func 11931): @1834156 (mutex @1834132),
    /// 48 bytes: CAppState(2), m_espera = api::CWait(1000) (func 5447), m_tela = CTelasVota +68/+72.
    static CInformacaoZeresimaTardia& GetInst();

    ~CInformacaoZeresimaTardia() override = default;   // wasm func 2879 (releases m_tela); deleting 11984
    void StartState() override;                         // wasm func 11983
    void ProcessInput() override;                       // wasm func 11982 (srcloc 48, 51)

private:
    api::CWait m_espera;                                // +16 (24 bytes): deadline = construction + 1 s
    CFormInterativoTelaVota m_tela;                     // +40 (+44)
};

// wasm func 11983 — vtable slot 2
void CInformacaoZeresimaTardia::StartState()
{
    m_tela->Exibe();
    m_proximoEstado = this;
}

// wasm func 11982 — vtable slot 7
void CInformacaoZeresimaTardia::ProcessInput()
{
    if (m_tela->Read() != api::EInputResult::Confirma)                    // cinteractiveform.h:57
        return;
    CLogVota::GetInst().Loga("Finalização de aplicativo");
    CLogVota::GetInst().Loga("Desligando a urna");
    api::IBeep::GetInst().BeepDesligamento();                             // :48, IBeep slot 4  name inferred
    m_espera.Aguarda();                                                   // func 5446 (see api/util/cwait.u09.cpp)
    comum::IInterfaceInit::GetInst().DesligarUrna();                      // :51 (func 5894)
    m_proximoEstado = nullptr;                                            // no next state
}

// wasm func 2879  — slot 0: ~CInformacaoZeresimaTardia() (releases m_tela at +40/+44)
// wasm func 11984 — slot 1: deleting destructor
// wasm func 11986 — at-exit destructor of the singleton pointer @1834156:
//     auto* p = s_inst; s_inst = nullptr; if (p) { p->~CInformacaoZeresimaTardia(); operator delete(p); }

}  // namespace vota

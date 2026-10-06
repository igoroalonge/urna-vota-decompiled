// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cconferevotoemcargo.h (path inferred)
//
// "Confere voto em cargo" = the transient *conferência* screen shown right after the voter typed the
// last digit (or BRANCO / an invalid number). It shows the ETelaVotacao given as template argument
// (e.g. "Tela de Conferência de Voto Nominal"), says "Confira o seu voto." when audio is on, ignores
// every key ("Tecla indevida pressionada") and, when its tick fires (or, when CConfiguracaoEleicao
// +176 <= 0 so that no tick was created, as soon as the current audio ends, without showing the
// screen), switches to the confirmation state TConfirma (CONFIRMA/CORRIGE screen).
//
// RTTI: CVotacaoStateAudio <- IConfereVotoEmCargo (typeinfo @1547684, vtable @1547584)
//         <- CConfereVotoEmCargo<CConfirmaVotoNominal, 2>, <CConfirmaVotoLegenda, 10>,
//            <CCandidatoInexistente, 12>, <CCandidatoInapto, 14>, <CProporcionalBranco, 4>,
//            <CProporcionalNulo, 7>, <CMajoritarioValido, 2>, <CMajoritarioRepetido, 16>,
//            <CMajoritarioNulo, 7>, <CMajoritarioBranco, 4>
#pragma once

#include <functional>
#include <memory>
#include <mutex>

#include "vota/eleitor/cvotacaostateaudio.h"       // unit u08
#include "vota/eleitor/comum/ctelascargo.h"        // ETelaVotacao

namespace api { class IEsperaAudio; }   // implemented by api::CEsperaAudio and, in the web build,
                                       // simulador::(anon)::CEsperaAudioWasm (both RTTI)

namespace vota {

class IConfereVotoEmCargo : public CVotacaoStateAudio {
public:
    // slot 0 func 1717 (cancels and releases m_espera); own slot 1 = func 325 (unreachable: abstract),
    // the template instances' slot 1 = func 1278 (deleting destructor)
    ~IConfereVotoEmCargo() override;

    void ProcessInputAudio() override;                      // slot 9  func 11791 (srcloc :80)
    void StartStateAudio() override;                        // slot 10 func 11793 (GetCargoID inlined)
    void FinishStateAudio() override;                       // slot 11 func 11792 (cancels m_espera)
    void ProcessTickAudio(uebyte tick) override;            // slot 12 func 11790
    std::string GetMensagemAudio() const override;          // slot 15 func 11789 "Confira o seu voto."

    /// Confirmation state that follows the conferência (template-provided).
    virtual comum::CAppState* GetProximoEstado() = 0;       // slot 16 name inferred

protected:
    /// wasm func 1165, defined in cconferevotoemcargo.u37.cpp (inlined GetInst of every instance in
    /// CPedeMajoritario/CPedeProporcional/CPedeNominal/CPedeNulo, unit u26):
    ///   CVotacaoStateAudio(6 = keys|ticks), m_espera = {}, m_tela = tela;
    ///   const int ms = ParametrosUrna.tempoConfirmacaoVoto;   // CConfiguracaoEleicao +176 (ms)
    ///   m_comTick = ms > 0; m_tick = ms > 0 ? CThreadEleitor::GetInst().CriaTick(ms) : 0;
    /// i.e. whether the conferência screen is shown at all is decided by the configuration.
    explicit IConfereVotoEmCargo(ETelaVotacao tela);

    /// Runs `acao` when the audio that is currently playing ends (immediately if none is playing).
    void ExecutarAposAudioAtual(std::function<void()> acao);   // func 3851 (srcloc :95)

    uebyte m_tick;                                          // +28 (CThreadEleitor tick, cfg +176 ms)
    bool m_comTick;                                         // +29 cfg +176 > 0: show the screen, wait for the tick
    ETelaVotacao m_tela;                                    // +30
    std::shared_ptr<api::IEsperaAudio> m_espera;            // +32 (+36); size 40 (returned by ISound slot 9)
};

template <typename TConfirma, ETelaVotacao TELA>
class CConfereVotoEmCargo final : public IConfereVotoEmCargo {
public:
    CConfereVotoEmCargo() : IConfereVotoEmCargo(TELA) {}   // func 1165 + vptr store (e.g. new(40), tela 10
                                                            // in CPedeNominal::GetProximoEstado, func 11724)
    ~CConfereVotoEmCargo() override = default;              // slot 1 = func 1278 for all instances

    /// Lazy singleton per instantiation, inlined into the CPede* states (unit u26). Its mutex and
    /// unique_ptr are template static data members (defined in u38-foreign-fragments.cpp).
    static CConfereVotoEmCargo& GetInst()
    {
        std::lock_guard<std::mutex> lock(s_mutex);          // only the unlock residue remains in the
        if (!s_instancia)                                   // single-threaded build
            s_instancia.reset(new CConfereVotoEmCargo);
        return *s_instancia;
    }

    /// slot 16: funcs 11670-11673, 11707, 11715-11717, 11738, 11739
    comum::CAppState* GetProximoEstado() override { return &TConfirma::GetInst(); }

private:
    static std::mutex s_mutex;                              // name inferred
    static std::unique_ptr<CConfereVotoEmCargo> s_instancia; // name inferred
};

}  // namespace vota

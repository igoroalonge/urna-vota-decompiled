// Reconstructed from vota_web_wasm.wasm (unit u10).
// Original (path inferred): uenux2/src/app/vota/operador/confirmaidentidade/ccontrolareconhecimento.h
//
// "Controla reconhecimento": entry state of the biometric identification of a voter on the mesário's
// terminal, and holder (static members) of the state of that identification: attempt counters, which
// finger of the 1x4 comparison is next, the captured fingerprint images and the matcher score.
// Those statics are read later by CMostraEleitorVotando::SalvaHabilitacaoEleitor.
//
// RTTI: comum::CAppState <- vota::CControlaReconhecimento (typeinfo @1591076, vtable @1590992)
//   slot 0 icf 244 (dtor)  1 icf 387 (deleting)  2 StartState (10512)  3..6 CAppState defaults
//   7 ProcessInput (10511)  8 no-op
#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "comum/cappstate.h"
#include "api/gui/cinteractiveform.h"

namespace vota {

using uebyte = std::uint8_t;

class CControlaReconhecimento final : public comum::CAppState {
public:
    /// wasm func 1256 (unit u37). Lazy singleton (@1908756, 20 bytes): CAppState(2 = keys),
    /// m_form built by func 5410 (unit u27): Beep(1), "ELEITOR(A) PODE VOTAR", "Assinar o caderno de
    /// votação antes de" / "votar", "CONFIRMA: prosseguir"; then touches vota_f1903 (matcher parameters).
    static CControlaReconhecimento& GetInst();

    void StartState() override;                              // slot 2 (func 10512)
    void ProcessInput() override;                            // slot 7 (func 10511)

    static void AvancaTentativa();                           // func 5406 (srcloc line 167)
    static void AvancaProximoDedo1x4();                      // srcloc line 187 (inlined into func 10465)
    static bool EhPrimeiraTentativa() { return s_tentativa == 1; }                 // func 5403
    static bool EhUltimaTentativa();                         // func 5405 (name inferred)
    static uebyte GetTentativa() { return static_cast<uebyte>(s_tentativa); }       // load8 of @1590924
    static uebyte GetTentativaDigitalSalva() { return static_cast<uebyte>(s_tentativaDigitalSalva); }
    /// ParametrosUrna.numTentativasHabilitacao (cfg +136, byte load). Only seen inlined into
    /// CDigitalNaoReconhecida::StartState, where it follows a second GetInst() call.   name inferred
    uebyte GetNumTentativas() const;

    // Finger order of the 1x4 verification (ANSI/NIST finger positions): right thumb, left thumb,
    // right index, left index. @1590944 = {1, 6, 2, 7}.
    static constexpr std::array<int, 4> DEDOS_1X4{1, 6, 2, 7};

    // ---- statics (addresses in linear memory; names inferred) ------------------------------
    static int                 s_tentativa;               // @1590924  1-based fingerprint attempt
    static int                 s_verificacoesDadoEleitor; // @1590928  birth-year checks (vs cfg numTentativasVerificacao, func 5404)
    static int                 s_tentativaDigitalSalva;   // @1590932  attempt whose image is in s_digitalCapturada
    static uebyte              s_indiceDedo;              // @1908668  index into DEDOS_1X4 (0..4)
    static std::vector<uebyte> s_digitalCapturada;        // @1908672  voter's last capture (WSQ)
    static std::vector<uebyte> s_digitalMesario;          // @1908684  mesário's capture (CRegistraDigitalOperador)
    static std::string         s_tituloMesario;           // @1908696  título of the mesário who released the voter
    static std::uint16_t       s_score;                   // @1908708  matcher score of the recognised finger
    // Training simulation (EhTreinamentoSemTreinamentoEleitor): the roll is cut into sixths by the
    // voter's sequential number; see CPedeDigital::VerificaDigitalTreinamento.
    static int                 s_qtdEleitores;            // @1908712
    static int                 s_faixaTentativa[4];       // @1908716..@1908728, each = qtdEleitores / 6

private:
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +12/+16
};

}  // namespace vota

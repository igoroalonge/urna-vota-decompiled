// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cconfirmavotoemcargo.h (path inferred)
//
// "Confirma voto em cargo": the screen with the vote summary and the CONFIRMA / CORRIGE instructions.
// CONFIRMA stores the vote (tipo = m_tipoVoto, number = g_votoDigitado) and ends the cargo;
// CORRIGE goes back to the "pede" state of the cargo (GetEstadoCorrige()).
//
// RTTI: CVotacaoStateAudio <- CConfirmaVotoEmCargo (typeinfo @1548328, vtable @1548240, 18 slots)
//         <- CConfirmaProporcional (@1548420) <- CConfirmaVotoNominal, CConfirmaVotoLegenda,
//                                               CCandidatoInexistente, CCandidatoInapto,
//                                               CProporcionalBranco, CProporcionalNulo
//         <- CConfirmaMajoritario  (@1549900) <- CMajoritarioValido, CMajoritarioRepetido,
//                                               CMajoritarioNulo, CMajoritarioBranco
#pragma once

#include <string>

#include "vota/eleitor/cvotacaostateaudio.h"
#include "vota/eleitor/comum/ctelascargo.h"
#include "comum/dados/md/rdv/cvoto.h"

namespace vota {

class CConfirmaVotoEmCargo : public CVotacaoStateAudio {
public:
    void ProcessInputAudio() override;          // slot 9  func 11754 (EmiteEcoCorrigeConfirma inlined; unit u08)
    void StartStateAudio() override;            // slot 10 func 11755
    /// slot 12 (pure in CVotacaoStateAudio): no-op override (func 425, own table slot 1986, inherited
    /// by all ten confirmation states); without it the leaf classes would stay abstract.
    void ProcessTickAudio(uebyte) override {}
    // slot 15 GetMensagemAudio stays pure here (each leaf: e.g. CConfirmaVotoNominal func 11728)

    /// State to return to on CORRIGE (the "pede" state of the cargo).
    virtual comum::CAppState* GetEstadoCorrige() = 0;                  // slot 16 name inferred
    /// Hook called with the input result before leaving (CORRIGE=5 / CONFIRMA=9).
    virtual void TrataResultado(api::EInputResult resultado) {}       // slot 17 name inferred

protected:
    // wasm func 5929
    CConfirmaVotoEmCargo(uebyte flags, ETelaVotacao tela, comum::md::CVoto::ETipo tipo);

    CFormInterativoTelaVota GetTelaCargoAtual(const std::string& funcao);   // func 5928 (srcloc :71)

    ETelaVotacao m_tela;                        // +28
    comum::md::CVoto::ETipo m_tipoVoto;         // +32 ; size 36
};

class CConfirmaProporcional : public CConfirmaVotoEmCargo {
public:
    comum::CAppState* GetEstadoCorrige() override;            // func 11751 -> CPedeProporcional
protected:
    CConfirmaProporcional(ETelaVotacao tela, comum::md::CVoto::ETipo tipo);   // func 11752
};

class CConfirmaMajoritario : public CConfirmaVotoEmCargo {
public:
    comum::CAppState* GetEstadoCorrige() override;            // func 11699 -> CPedeMajoritario
protected:
    CConfirmaMajoritario(uebyte flags, ETelaVotacao tela, comum::md::CVoto::ETipo tipo);   // func 11700
};

}  // namespace vota

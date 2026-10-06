// Reconstructed from vota_web_wasm.wasm (unit u39 = 26 of the 37 slots; the others by u02, u18, u20, u26, u27).
// Original (path inferred, as in u20/u27): uenux2/src/app/vota/comum/ccontroladorregistramesariosvota.h
// (u18 proposed operador/comparecimentomesario/; both are guesses: no srcloc, no string names the file).
//
// VOTA's implementation of comum::IControladorRegistraMesarios, the host interface of the application-
// independent "registro de comparecimento de mesários" states (comum/comparecimentomesario/estados, u22):
// before, during and after voting the mesários (poll workers) identify themselves on the MT with their
// título and, when the section is biometric, their fingerprint; each registration becomes a row of the SQLite
// table comparecimento_mesario (uenux.db) and ends up in the BIM report ("boletim de identificação de
// mesários") printed at the closing. This class supplies the operator-thread ticks, the current period
// (derived from EstadoGeralVota.estadoVota), the 6-mesário limit, the states to return to and the log texts.
//
// Pushed into CPolySingletonList by vota::CAjusteInicial (func 7160, 4-byte object: vptr only) - never in the
// web build, where the comum states cannot even look it up (u22).
//
// RTTI: comum::IControladorRegistraMesarios (typeinfo @1534552) <- vota::CControladorRegistraMesariosVota
// (typeinfo @1586756, vtable @1586608, 37 slots). Slot names: interface reconstruction of u22
// (src/uenux2/src/app/comum/comparecimentomesario/icontroladorregistramesarios.h); aliases used by other units
// in brackets.
#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "comum/comparecimentomesario/icontroladorregistramesarios.h"

namespace vota {

class CControladorRegistraMesariosVota final : public comum::IControladorRegistraMesarios {
public:
    // [0] 174 trivial dtor, [1] 144 operator delete
    uebyte CriaTick(std::uint32_t ms) override;                           // [2]  wasm func 10800
    void StopTick(uebyte tick) override;                                  // [3]  wasm func 10799
    void StartTick(uebyte tick) override;                                 // [4]  wasm func 10798
    void AtualizaEstadoRegistro() override;                               // [5]  wasm func 10797 (u20 [IniciaRegistro])
    comum::EPeriodoRegistro GetPeriodoRegistro() const override;          // [6]  wasm func 10796
    bool LimiteMesariosAtingido() const override;                         // [7]  wasm func 10795
    bool Slot8() const override { return true; }                          // [8]  ICF 434 "return 1"   name unknown
    comum::CAppState* GetEstadoAposRegistroInicial() override;            // [9]  wasm func 10794 (u27)
    void NotificaFimRegistroInicial() override;                           // [10] wasm func 10792 (u18 [LiberaTerminalEleitor])
    comum::CAppState* GetEstadoAposRegistroVotacao() override;            // [11] wasm func 10793
    comum::CAppState* GetEstadoAposRegistroFinal() override;              // [12] wasm func 10791
    bool MesarioEhEleitorDaSecao() const override;                        // [13] wasm func 10790
    const comum::CEleitorDetalhe* GetEleitorMesario() const override;     // [14] wasm func 10789
    void NotificaFimRegistroFinal() override;                             // [15] wasm func 10787 (u18 [IniciaEncerramento])
    void SincronizaBancoDados() override;                                 // [16] wasm func 10786 (u02)
    const std::string& GetTituloMesario() const override;                 // [17] wasm func 10788 (u18 [GetTituloDigitado])
    void SetTituloMesario(const std::string& titulo) override;            // [18] wasm func 10785 (u18 [SetTituloDigitado])
    void LogaMesarioRegistrado(const std::string& titulo) override;       // [19] wasm func 10784
    void LogaRegistroAntesVotacao() override;                             // [20] wasm func 10783
    void LogaRegistroDuranteVotacao() override;                           // [21] wasm func 10782
    void LogaRegistroAposVotacao() override;                              // [22] wasm func 10781
    void LogaIndagadoRegistro() override;                                 // [23] wasm func 10779
    void LogaConfirmouRegistro() override;                                // [24] wasm func 10778
    void LogaCancelouRegistro() override;                                 // [25] wasm func 10777
    void LogaTituloInvalido() override;                                   // [26] wasm func 10776
    void LogaMesarioJaRegistrado(const std::string& titulo) override;     // [27] wasm func 10775
    void LogaLimiteMesariosAtingido() override;                           // [28] wasm func 10774 (u26)
    void LogaEncerrouRegistro() override;                                 // [29] wasm func 10773
    void LogaDigitalNaoCorresponde(const std::vector<int>& scores) override;   // [30] wasm func 10772
    void LogaPedidoLeituraBiometria() override;                           // [31] wasm func 10771
    void LogaMesarioEhEleitor() override;                                 // [32] wasm func 10770
    void LogaMesarioNaoEhEleitor() override;                              // [33] wasm func 10769
    void LogaConferenciaBiometria() override;                             // [34] wasm func 10768
    void LogaIndagadoContinuarRegistro() override;                        // [35] wasm func 10767
    void LogaIndagadoFinalizarRegistro() override;                        // [36] wasm func 10766

    // sizeof 4: no data members (operator new(4) in func 7160).
};

}  // namespace vota

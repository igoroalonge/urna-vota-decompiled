// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h   (path inferred from ctesteteclado.cpp,
// attested by std::source_location records :38, :42, :50, :60, :81, :121)
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner; siblings CTesteFalhou / CEnviarManutencao /
// CEsperaRetestar / CErroTesteTecladoFim in unit u02's fragment ctesteteclado.u02.cpp).
//
// "Teste do teclado do TE" (TE = terminal do eleitor, the voter's keypad): before the zerésima (CPreZeresima)
// and after a restart (CRetomada) the mesário may be asked to press the 13 keys of the voter keypad
// (0-9, BRANCO, CORRIGE, CONFIRMA) in a random order; each key to press is highlighted on the drawing of
// the keypad. A wrong key leads to CTesteFalhou ("Repetir teste" / "Prosseguir" -> manutenção).
//
// RTTI:
//   comum::CAppState <- vota::CEstadoComDesligamentoAutomatico <- vota::testeteclado::CTesteTeclado
//     (typeinfo @1547276, vtable @1547140): [0] dtor 2868 [1] deleting dtor 11806 [2] StartState 11805
//     [3] 7480 [4] 1661 [5] nop 218 [6] nop 425 [7] ProcessInput 11804 [8] 12061 [9] nop 425
//   vota::testeteclado::impl::IGeradorTeclas (typeinfo @1547320) <- impl::CGeradorTeclasAleatorio
//     (typeinfo @1547308, vtable @1547296: [0] 174 [1] 144 [2] GeraSequencia = func 11803, other unit)
#pragma once

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/gui/ctextbox.h"                          // api::CTextBox
#include "vota/eleitor/cestadocomdesligamentoautomatico.h"
#include "vota/eleitor/comum/ctelasvota.h"             // CFormInterativoTelaVota

namespace vota::testeteclado {

namespace impl {

/// Produces the order in which the keys must be pressed. Registered on first use (default registration
/// pattern, ctesteteclado.cpp:121) as impl::CGeradorTeclasAleatorio (random order).
class IGeradorTeclas {
public:
    static IGeradorTeclas& GetInst();                  // :121 - inlined into CTesteTeclado::StartState
    virtual ~IGeradorTeclas() = default;               // [0] 174 / [1] 144
    virtual std::vector<std::string> GeraSequencia(std::vector<std::string> teclas) = 0;   // [2]  name inferred
};

class CGeradorTeclasAleatorio : public IGeradorTeclas {  // 4 bytes (vptr)
public:
    std::vector<std::string> GeraSequencia(std::vector<std::string> teclas) override;      // func 11803 (u02/other)
};

} // namespace impl

class CTesteTeclado : public CEstadoComDesligamentoAutomatico {
public:
    /// wasm func 3855 (name inferred). Lazy singleton @1835148 (mutex residue @1835124), 68 bytes,
    /// CEstadoComDesligamentoAutomatico(flags 2 = keyboard). Exit-time reset: func 11808.
    static CTesteTeclado& GetInst();

    ~CTesteTeclado() override;                         // func 2868 (slot 0), 11806 = deleting (slot 1)

    void StartState() override;                        // func 11805 (srcloc :38, :42)
    void ProcessInput() override;                      // func 11804 (srcloc :50, :60, :81)

    /// Set by testeteclado::CBase::ProcessInput (func 11874) before switching here: where to go when the
    /// test passes (CBase slot 11 GetEstadoPassouNoTeste).                           name inferred
    void SetEstadoAposTeste(comum::CAppState* estado) { m_estadoAposTeste = estado; }

    struct STecla {                                    // map node value (key string at node +16)
        char tecla;                                    // node +28: character reported by IInputKbd
        std::shared_ptr<api::CTextBox> caixa;          // node +32: drawing of the key (status 2 normal / 1 destacada)
    };

private:
    CTesteTeclado() : CEstadoComDesligamentoAutomatico(2) {}

    // CEstadoComDesligamentoAutomatico: +0..+27
    CFormInterativoTelaVota m_tela;                    // +28 / +32  "telaTesteTeclado"
    std::map<std::string, STecla> m_teclas;            // +36 (begin +36, root +40, size +44), by key name
    std::vector<std::string> m_sequencia;              // +48 order to press (from IGeradorTeclas)
    std::size_t m_indice = 0;                          // +60 current position in m_sequencia
    comum::CAppState* m_estadoAposTeste = nullptr;     // +64
};

} // namespace vota::testeteclado

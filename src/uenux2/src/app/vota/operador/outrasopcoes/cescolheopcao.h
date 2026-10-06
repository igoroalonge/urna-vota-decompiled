// Reconstructed from vota_web_wasm.wasm (unit u27).
// Original (path inferred from cescolheopcao.cpp, attested by srclocs :150-:152):
// uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.h
//
// "Escolhe opção" = the "outras opções" menu of the mesário's terminal, reached with CORRIGE from the
// idle identification screen (CPedeIdentidade):
//
//     Selecione a opção: _             hh:mm
//     1-Ativar áudio       3-Registrar mesários
//     2-Encerrar votação   4-Exibir contadores
//     CORRIGE: retornar    CONFIRMA: prosseguir
//
// Options 3 and 4 only exist when their preconditions hold (see the constructor). The number shown is
// the menu key; each entry also carries an "action" id that selects what CONFIRMA does.
//
// RTTI: comum::CAppState <- vota::CEscolheOpcao (vtable @1588004: slot 0 dtor 2752, 1 deleting dtor 10691,
//   2 StartState 10690, 7 ProcessInput 10689). vota::CEscolheOpcao::COpcaoDS is the text source type
//   (RTTI of api::CDataText<vota::CEscolheOpcao::COpcaoDS>, vtable @1588108).
#pragma once

#include <format>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"
#include "comum/cappstate.h"

namespace vota {

class CEscolheOpcao final : public comum::CAppState {
public:
    /// Text source of one menu line: "<n>-<texto()>" (the text is computed at every redraw, e.g.
    /// "Ativar áudio" / "Desativar áudio"). 32 bytes: +0 numero, +8 std::function.
    struct COpcaoDS {
        int numero;                                      // +0
        std::function<std::string()> texto;              // +8 (buffer) / +24 (__f_)
        COpcaoDS(int n, std::function<std::string()>&& f) : numero(n), texto(std::move(f)) {}   // wasm func 2751
        // Used by api::CDataText<COpcaoDS>::GetText (vtable slot 2, func 10685): format "{}-{}" @1632
        // -> "1-Ativar áudio". An empty std::function throws std::bad_function_call (func 648).
        std::string Text() const { return std::format("{}-{}", numero, texto()); }
    };

    /// Action executed by CONFIRMA (value of the map, name inferred).
    enum class EAcao : int { AUDIO = 1, ENCERRAR_VOTACAO = 2, REGISTRAR_MESARIOS = 3, CONTADORES_BIOMETRIA = 4 };

    /// Map value: {action, text source}. 40 bytes (+0 acao, +8 COpcaoDS).   name inferred
    struct SOpcao {
        EAcao acao;
        COpcaoDS ds;
        SOpcao(EAcao a, COpcaoDS&& d) : acao(a), ds(std::move(d)) {}           // wasm func 2750
    };

    /// wasm func 2753 (tools: api_f2753). Lazy singleton @1905308 (mutex @1905284), 32 bytes.
    static CEscolheOpcao& GetInst();

    ~CEscolheOpcao() override;                          // slot 0 (func 2752), slot 1 (func 10691)
    void StartState() override;                         // slot 2 (func 10690, srclocs :150-:152)
    void ProcessInput() override;                       // slot 7 (func 10689)

private:
    CEscolheOpcao();
    comum::CAppState* ExecutaOpcao();                   // inlined into ProcessInput, name inferred

    std::map<int, SOpcao> m_opcoes;                     // +12 (begin +12, root +16, size +20); key = menu number
    std::shared_ptr<api::CInteractiveForm<api::IScreenMT, api::IInputMT>> m_form;   // +24/+28
};

}  // namespace vota

// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/capplicationcontextstack.cpp (srcloc line 45 = the guard constructor).
// Not here (other units): CApplicationContext copy constructor (func 1841), ~CApplicationContextGuard
// (func 675), CApplicationContextStack::Top (func 1695, fallback "Pilha de contexto vazia").
#include "api/gui/capplicationcontextstack.u15.h"

#include <format>

#include "api/gui/gui-common.u15.h"   // CUeGuiError

namespace api {

namespace {
constexpr auto kTexto = "Tire uma foto dessa tela em que apareça o QR Code ao lado e envie para a equipe "
                        "de suporte para receber auxílio para resolução do problema.";            // @374109
constexpr auto kSePersistir = "Desligue e ligue a urna.";                                           // @373788
} // namespace

// wasm func 5557 (tools: vota_f5557)                                       // name inferred
// Moves every part in; the vector is stolen (its three words are copied and the source zeroed).
CApplicationContext::CApplicationContext(std::string detalhe, std::string titulo, std::string mensagem,
                                         std::vector<std::string> acoes, bool generico)
    : m_detalhe(std::move(detalhe)),
      m_titulo(std::move(titulo)),
      m_mensagem(std::move(mensagem)),
      m_acoes(std::move(acoes)),
      m_generico(generico)
{
}

// wasm func 3682 (tools: vota_f3682; observed executing)                   // name inferred
// Callers: CApplicationContextGuard (676) and funcs 2740, 3229.
CApplicationContext::CApplicationContext(Actions acao, std::string detalhe, std::string titulo,
                                         std::string mensagem)
    : CApplicationContext(std::move(detalhe), std::move(titulo), std::move(mensagem),
                          ActionsToText(acao), /*generico*/ false)
{
}

// wasm func 5558 (tools: vota_f5558; observed executing)                   // name inferred
// The fallback context: CApplication::InitApplication pushes
//   ("Não é possível continuar a execução", "Erro inesperado"),
// and CApplicationContextStack::Top (func 1695) returns ("Pilha de contexto vazia", "Erro inesperado")
// when the stack is empty. Both get the QR-code hint as message and Actions::ReinicieUrna.
CApplicationContext::CApplicationContext(std::string detalhe, std::string titulo)
    : CApplicationContext(std::move(detalhe), std::move(titulo), kTexto,
                          ActionsToText(Actions::ReinicieUrna), /*generico*/ true)
{
}

// wasm func 5555 (tools: vota_f5555; observed executing, 3.2 KB)          // name inferred
// One or two lines of instructions for the operator, chosen by the Actions code.
std::vector<std::string> ActionsToText(Actions acao)
{
    constexpr auto religue = kSePersistir;   // "Desligue e ligue a urna."
    switch (acao) {
    case Actions::DesligueUrna:
        return {"Desligue a urna."};
    case Actions::ReinicieUrna:
        return {religue};
    case Actions::ReinicieOuSubstituaUrna:
        return {religue, "Se o erro persistir, substitua a urna."};
    case Actions::ConfiraImpressora:
        return {"Desligue a urna, confira o encaixe da impressora e ligue novamente.",
                "Se o erro persistir, substitua o módulo impressor."};
    case Actions::ReinicieOuSubstituaMidiaVotacao:
        return {religue, "Se o erro persistir, substitua a mídia de votação."};
    case Actions::ReinicieOuSubstituaMidiaExterna:
        return {religue, "Se o erro persistir, substitua a mídia externa."};
    case Actions::ReinicieOuSubstituaMidiaAplicacao:
        return {religue, "Se o erro persistir, substitua a mídia de aplicação."};
    case Actions::ReinicieOuSubstituaMidiaInterna:
        return {religue, "Se o erro persistir, substitua a mídia interna."};
    case Actions::ReinicieOuSubstituaMidiaResultado:
        return {religue, "Se o erro persistir, substitua a mídia de resultados."};
    case Actions::ReinicieOuApureEmOutraUrna:
        return {religue, "Se o erro persistir, faça a apuração em outra urna."};
    case Actions::EnvieParaJunta:
        return {"Desligue a urna eletrônica e envie-a para a Junta Eleitoral."};
    case Actions::ReinicieOuFotografeQRCode:
        return {religue, "Se o erro persistir, tire uma foto dessa tela em que apareça o QR Code ao lado e "
                         "envie para a equipe de suporte para receber auxílio para resolução do problema."};
    case Actions::NovaCarga:
        return {"Desligue a urna e faça uma nova carga."};
    case Actions::InsiraMidiaVotacaoDeOutraUrna:
        return {"Desligue a urna, insira uma mídia de votação usada de outra urna que apresentou problema "
                "e tente novamente."};
    case Actions::InsiraMidiaVotacaoValida:
        return {"Desligue a urna, insira uma mídia de votação válida e tente novamente."};
    case Actions::SubstituaUrna:
        return {"Desligue a urna e substitua-a."};
    case Actions::AjusteHorarioComADH:
        return {"Desligue a urna, retire a mídia de votação e a mídia de resultado e utilize o ADH para "
                "ajustar o horário desta urna, ou leve a mídia de votação e a mídia de resultado para "
                "outra urna de contingência com horário correto."};
    }
    return {std::format("Actions({}) - não reconhecida", static_cast<int>(acao))};
}

// wasm func 3684 (tools: api_f3684; observed executing)                    // name inferred
// `this` was constant-propagated by Binaryen (always the global stack @1839212) and the bool result
// dropped because no direct caller uses it (InitApplication, CEleitorVotando::IniciaCiclo,
// CPedeDigital::StartState). The guard constructor inlines the same code and uses the result.
// Rule: a generic context may only go on top of an empty stack or of another generic context.
bool CApplicationContextStack::Push(CApplicationContext contexto)
{
    if (contexto.m_generico && !m_pilha.empty() && !m_pilha.back().m_generico)
        return false;
    m_pilha.push_back(std::move(contexto));    // slow path: func 5554
    return true;
}

// Inlined in InitApplication (loop of ~CApplicationContext = func 1468 from the back).
void CApplicationContextStack::Clear()
{
    m_pilha.clear();
}

// wasm func 676 (observed executing) - srcloc capplicationcontextstack.cpp:45
// Signature of the srcloc: CApplicationContextGuard(CApplicationContextStack&, CApplicationContext).
// The wasm function is the (Actions, detalhe, titulo, mensagem) overload with that constructor inlined.
// Callers include vota::CGeraBU::vf2, CGravaResultado::vf2, CGeraRelatorios::StartState,
// CCopiaResultadoParaMR::CopiaResultado, comum::AssinarUE and CPedeAnoNascimento::vf7.
CApplicationContextGuard::CApplicationContextGuard(CApplicationContextStack& pilha,
                                                   CApplicationContext contexto)
    : m_pilha(pilha), m_contexto(std::move(contexto))
{
    if (!m_pilha.Push(m_contexto))            // copy: func 1841
        throw CUeGuiError(static_cast<EUeGuiError>(5013), "O contexto não pôde ser criado");
}

// wasm func 5554 (tools: api_f5554):                                        library instantiation
// std::vector<CApplicationContext>::__push_back_slow_path(CApplicationContext&&) - growth x2, element
// size 52, max_size 82595524; elements moved one by one, then the old ones destroyed with func 1468.

} // namespace api

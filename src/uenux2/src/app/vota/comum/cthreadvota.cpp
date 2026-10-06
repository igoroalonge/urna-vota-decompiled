// uenux2/src/app/vota/comum/cthreadvota.cpp   (path inferred, see cthreadvota.h)
// Reconstructed from vota_web_wasm.wasm (unit u26). The tools filed these functions under
// vota/monitor/cthreadmonitor.cpp because CThreadMonitor inherits them.
//
// What happens when a VOTA thread dies of an exception (urna only; unreachable in the web build):
//   1. a CUeDesligandoError (the urna is being switched off) is not an error: return;
//      the same if another thread already started the shutdown (CSincronizaVota flag @1832936);
//   2. FinalizaExecucao(): stop the other threads (policy-specific, slot 5);
//   3. the mesário terminal (microterminal, MT) shows "URNA ELETRÔNICA INOPERANTE / Siga as instruções na
//      tela do eleitor", LED off (func 4633);
//   4. mark the urna as shutting down (func 3336);
//   5. the voter screen shows the fatal-error page built from the top of the application-context stack
//      (api::CApplicationContextStack, unit u15): título "<título> (<código>)", the context message, the
//      recommended actions ("Desligue e ligue a urna." ...), plus the exception text when the context is
//      the generic fallback one; api::CApplication::ShowExceptionMsg (func 5568) also starts an SOS beep;
//   6. park forever in api::CApplication::EnterLoopDoingNothing (func 5566).
// In the web build step 5 throws std::system_error("thread constructor failed") (std::async without
// pthreads) and step 6 would abort on emscripten_sleep; errors of votaTick are reported by vota_web_wasm.cpp
// instead ("erro desconhecido em votaTick").
#include "vota/comum/cthreadvota.h"

#include <format>
#include <regex>
#include <string>
#include <vector>

#include "api/gui/capplication.h"                  // api::CApplication (unit u15)
#include "api/gui/capplicationcontextstack.h"      // api::CApplicationContextStack / CApplicationContext
#include "api/gui/cformbuildermt.h"                // api::CFormBuilderMT (path inferred)
#include "api/hwil/cuedesligandoerror.h"           // api::CUeDesligandoError (path inferred)
#include "ecourna/api/util/cstringutils.h"         // Join
#include "vota/comum/csincronizavota.h"

namespace vota {

namespace {

// wasm func 4633 (name inferred). Message on the mesário's microterminal (2-line LCD, IScreenMT).
void MostraUrnaInoperanteNoMicroterminal()
{
    api::CFormBuilderMT campos;
    campos.Add<api::CLedFieldMT>(false);                                                   // func 435 (LED off)
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 1},
        std::make_shared<api::CFixedText>(api::ETextAlignment(2), "URNA ELETRÔNICA INOPERANTE"));            // func 180
    campos.Add<api::CTextFieldMT>(api::SPoint{1, 2},
        std::make_shared<api::CFixedText>(api::ETextAlignment(2), "Siga as instruções na tela do eleitor"));  // func 180
    campos.CriaForm("")->Show();                                                           // func 1694, IForm slot 2
}

// wasm func 5569 (name inferred). Kind of error page: 2 for the generic fallback context, 1 otherwise.
// First argument of api::CApplication::ShowExceptionMsg.
int TipoContexto()
{
    return api::CApplicationContextStack::GetInst().Top().m_generico ? 2 : 1;             // func 1695
}

// Inlined in 7709/7710: the recommended actions, one per line (shared_f1696 = join).
std::string Acoes(const api::CApplicationContext& contexto)
{
    return ecourna::api::util::CStringUtils::Join(contexto.m_acoes, "\n");
}

// Inlined in 7710 (name inferred).
std::string MensagemErro(const ecourna::api::exception::CError& erro)
{
    const auto contexto = api::CApplicationContextStack::GetInst().Top();
    if (!contexto.m_generico)
        return contexto.m_mensagem + "\n\n" + Acoes(contexto);
    return erro.GetMensagem() /* CError +8 */ + "\n\n" + contexto.m_mensagem + "\n\n" + Acoes(contexto);
}

// Inlined in 7709 (name inferred). what() of the TSE errors is "<função> <arquivo>:<linha>:<coluna> - <texto>":
// the regex keeps only <texto>. A failure of the regex falls back to the full what().
std::string TextoExcecao(const std::exception& erro)
{
    const std::string texto = erro.what();
    try {
        const std::regex padrao(R"(.*?\).*?:\d+:-?\d+ - ((.|\n)*))");     // vota_f2416 (basic_regex ctor)
        std::smatch resultado;
        if (std::regex_search(texto, resultado, padrao))                 // api_f6155
            return resultado[1].str();                                    // "" when group 1 did not match
    } catch (...) {
    }
    return texto;
}

std::string MensagemExcecao(const std::exception& erro)
{
    const auto contexto = api::CApplicationContextStack::GetInst().Top();
    if (!contexto.m_generico)
        return contexto.m_mensagem + "\n\n" + Acoes(contexto);
    return TextoExcecao(erro) + "\n\n" + contexto.m_mensagem + "\n\n" + Acoes(contexto);
}

} // namespace

// wasm func 2126 (slot 0 of CThreadVota and of CThreadMonitor). Members, then api::CThread::~CThread (2721).
CThreadVota::~CThreadVota() = default;       // m_pContexto.reset() (virtual dtor, slot 1), m_ticks tree (func 2125)

// wasm func 7710 (vtable slot 3)                                                   name as in unit u07
void CThreadVota::TrataExcecao(const ecourna::api::exception::CError& erro)
{
    if (dynamic_cast<const api::CUeDesligandoError*>(&erro))           // __dynamic_cast CError -> CUeDesligandoError
        return;
    if (CSincronizaVota::UrnaDesligando())                             // byte @1832936
        return;

    FinalizaExecucao();                                                 // slot 5
    MostraUrnaInoperanteNoMicroterminal();                              // func 4633
    CSincronizaVota::MarcaUrnaDesligando();                             // func 3336

    const int tipo = TipoContexto();                                    // func 5569
    const std::string detalhe = api::CApplicationContextStack::GetInst().Top().m_detalhe;
    const std::string titulo = std::format("{} ({})", api::CApplicationContextStack::GetInst().Top().m_titulo,
                                           erro.GetCodigo() /* CError +4 */);
    const std::string mensagem = MensagemErro(erro);
    api::CApplication::ShowExceptionMsg(tipo, detalhe, titulo, mensagem, erro.what());   // func 5568
    api::CApplication::EnterLoopDoingNothing();                                           // func 5566 (never returns)
}

// wasm func 7709 (vtable slot 4)                                                   name as in unit u07
void CThreadVota::TrataExcecaoDesconhecida(const std::exception& erro)
{
    if (CSincronizaVota::UrnaDesligando())
        return;

    FinalizaExecucao();
    MostraUrnaInoperanteNoMicroterminal();
    CSincronizaVota::MarcaUrnaDesligando();

    const int tipo = TipoContexto();
    const std::string detalhe = api::CApplicationContextStack::GetInst().Top().m_detalhe;
    const std::string titulo = api::CApplicationContextStack::GetInst().Top().m_titulo;   // no error code here
    const std::string mensagem = MensagemExcecao(erro);
    api::CApplication::ShowExceptionMsg(tipo, detalhe, titulo, mensagem, erro.what());
    api::CApplication::EnterLoopDoingNothing();
}

} // namespace vota

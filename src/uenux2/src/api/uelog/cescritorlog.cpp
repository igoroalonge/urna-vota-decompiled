// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/api/uelog/cescritorlog.cpp (srclocs cescritorlog.cpp:28, :38, :44).
//
// api::CEscritorLog ("log writer") is the abstract front-end of the urna's log daemon (logd). Every
// application log line goes  CLoga::loga -> CPolySingletonList::instance<CEscritorLog>() -> loga()
// -> Escreve() (pure virtual). The real urna implements Escreve() by talking to the logd process;
// the web build registers simulador::CWasmLogd (vtable @1530808), whose Escreve (func 8327, other unit)
// appends Latin-1 records "<app>|<severity>|<text>" to /dsk/fi/dinamico/log/logd.dat in MEMFS.
//
// CEscritorLog is abstract, so it has no vtable of its own in the binary: its two concrete methods
// appear in CWasmLogd's vtable (the analysis tools therefore named func 10260 "CWasmLogd::vf3").
//
// vtable (CWasmLogd): [0] ~ (8318)  [1] deleting (8316)  [2] fazOperacao (10262)  [3] loga (10260)
//                     [4] Escreve (8327, pure in CEscritorLog; name inferred)
#include "api/uelog/cescritorlog.h"

#include <format>
#include <string>

namespace api {

// ecourna::api::exception::CBaseError<api::EUeUeLogError, ...> (typeinfo @1599588, vtable @1599648).
// wasm func 3601 (tools: api_f3601) is its constructor thunk: ecourna_f710(exc, codigo, msg, srcloc, vtable).

namespace {

constexpr unsigned QTD_APLICATIVOS = 61;          // codes 0..60 are applications
constexpr unsigned PRIMEIRA_OPERACAO = 230;       // codes 230..240 are logd "operations"
constexpr unsigned ULTIMA_OPERACAO = 240;

// inlined into func 10260                                                 srcloc cescritorlog.cpp:28
void confereAppValida(const std::string& funcao, ELogAplicativos aplicativo)
{
    if (static_cast<unsigned>(aplicativo) >= QTD_APLICATIVOS)
        throw CUeUeLogError(EUeUeLogError{6950},
                            std::format("{} - O código da aplicação deve ser menor que {} [{}]",
                                        funcao, QTD_APLICATIVOS, aplicativo));   // ELogAplicativos formatter
}

} // namespace

// wasm func 10262 (vtable slot 2)                                srcloc cescritorlog.cpp:38 / :44
void CEscritorLog::fazOperacao(ELogOperacoesLogD operacao)
{
    const unsigned codigo = static_cast<unsigned>(operacao);
    if (codigo < PRIMEIRA_OPERACAO)
        throw CUeUeLogError(EUeUeLogError{6951},
            std::format("Não existe nenhuma operação com número menor que {} [{}]", PRIMEIRA_OPERACAO, operacao));
    if (codigo > ULTIMA_OPERACAO)
        throw CUeUeLogError(EUeUeLogError{6952},
            std::format("Não existe nenhuma operação com número maior que {} [{}]", ULTIMA_OPERACAO, operacao));

    // An operation is sent as a record whose "application" byte is the operation code,
    // severity 1 and an empty text.
    Escreve(static_cast<uebyte>(codigo), ESeveridade{1}, std::string());       // slot 4
}

// wasm func 10260 (vtable slot 3; tools: simulador::CWasmLogd::vf3)                name inferred
void CEscritorLog::loga(ELogAplicativos aplicativo, ESeveridade severidade, const std::string& mensagem)
{
    confereAppValida("loga", aplicativo);
    Escreve(static_cast<uebyte>(aplicativo), severidade, mensagem);            // slot 4
}

} // namespace api

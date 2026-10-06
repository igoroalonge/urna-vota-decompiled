// uenux2/src/app/vota/log/clogvota.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner; see also the fragment clogvota.u09.cpp).
// std::source_location records of this file: :37 (GetInst, func 184) and :432 (LogaEleitorImpedido, func
// 10635, other unit).
#include "vota/log/clogvota.h"

#include <format>
#include <memory>
#include <source_location>
#include <typeinfo>

#include "api/pattern/cpolysingletonlist.h"
#include "comum/util/formatatamanho.h"       // comum::util::FormataTamanho (func 1947, path inferred)

namespace vota {

// clogvota.cpp:37 - wasm func 184 (observed executing). The "default registration" pattern of VOTA:
// a platform may register its own IEventosLog first; otherwise the urna's CLogVota is created. The push is
// inlined (cpolysingletonlist.h:129 throws EUePatternError 6756 "{}: instância já criada de {}"; with the
// DEBUG_UENUX trace enabled it prints "push" and "sz[{}] ptr[{}]").
CLogVota& CLogVota::GetInst()
{
    auto& info = api::GetPolySingletonsInfo();
    if (!api::CPolySingletonList::contains<comum::IEventosLog>(info))                       // api_f2509
        api::CPolySingletonList::push<comum::IEventosLog>(std::make_unique<CLogVota>(), info);
    return dynamic_cast<CLogVota&>(api::CPolySingletonList::instance<comum::IEventosLog>(info));   // :37 (func 837)
}

// wasm func 3268 (name inferred). Callers: vota::CThreadMonitor::Run (LogaMemoria, inlined).
// Format argument types (packed 429 = string_view, string_view).
void CLogVota::LogaQuantidade(const std::pair<std::string, std::uint64_t>& quantidade)
{
    Loga(std::format("Quantidade de {} [{}]", quantidade.first,
                     comum::util::FormataTamanho(static_cast<std::size_t>(quantidade.second))));   // func 1947
}

// wasm func 4511 (name inferred). Caller: vota::testeteclado::CTesteTeclado::ProcessInput (11804), on the
// last key of the test AND on a wrong key.
void CLogVota::LogaFimTesteTecladoSucesso()
{
    Loga("Fim do teste de Teclado do TE - Sucesso");
}

// wasm func 3902 (merged body; name inferred). merge-similar-functions turned the five 8-byte loads of the
// literal into five pointer parameters (c, d, e, f, g = texto+0, +8, +16, +24, +30).
void CLogVota::Loga38(api::ESeveridade severidade, const char (&texto)[39]) const
{
    api::CLoga::loga(m_aplicativo, severidade, std::string(texto, 38));
}

// wasm func 5880 (name inferred). Callers: vota::CVerificaHorarioZeresima::ProcessInput (11869),
// vota::CImpressaoEstadoUrna::StartState (11911).
void CLogVota::LogaImprimindoRelatorioEstadoUrna()
{
    Loga38(api::ESeveridade{1}, "Imprimindo relatório de estado da urna");
}

} // namespace vota

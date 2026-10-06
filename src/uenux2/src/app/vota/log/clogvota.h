// uenux2/src/app/vota/log/clogvota.h   (path inferred from clogvota.cpp, attested by std::source_location
// records :37 GetInst and :432 LogaEleitorImpedido)
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner of clogvota.cpp; more methods in unit u09's
// fragment clogvota.u09.cpp and in the units of their only callers).
//
// vota::CLogVota = the VOTA implementation of comum::IEventosLog: one method per fixed log record of the
// application. Records go to the urna log /dsk/fi/dinamico/log/logd.dat as Latin-1 text
// "<aplicativo>|<severidade>|<texto>" through api::CLoga::loga (e.g. "1|1|Voto confirmado para [Vereador]").
//
// RTTI: comum::IEventosLog <- vota::CLogVota (typeinfo @1532824, vtable @1532792 = {174, 144}: only the
// virtual destructor). 8 bytes: +0 vptr, +4 api::ELogAplicativos m_aplicativo = 1 (VOTA).
//
// Most methods are one-liners inlined into their callers ("CLogVota::GetInst().Loga(<literal>)"); the
// out-of-line ones are merged by wasm-opt: e.g. func 3902 is the common body of every method whose message is a
// 38-byte literal (the five 8-byte chunks of the literal became parameters).
#pragma once

#include <cstdint>
#include <string>
#include <utility>

#include "comum/log/ieventoslog.h"

namespace vota {

class CLogVota : public comum::IEventosLog {
public:
    /// clogvota.cpp:37 - wasm func 184. Registers a CLogVota as the IEventosLog poly-singleton on first use
    /// and returns it (dynamic_cast from IEventosLog; std::bad_cast if another implementation is registered).
    /// Observed executing (every log record of the recorded votes).
    static CLogVota& GetInst();

    CLogVota() : comum::IEventosLog(api::ELogAplicativos(1)) {}

    // ---- methods reconstructed in this unit ----------------------------------------------------------
    /// wasm func 3268 (name inferred): "Quantidade de <nome> [<tamanho formatado>]" (memory statistics of
    /// CThreadMonitor). The 64-bit value is narrowed to size_t (32 bits on wasm32) by FormataTamanho.
    void LogaQuantidade(const std::pair<std::string, std::uint64_t>& quantidade);

    /// wasm func 4511 (name inferred): "Fim do teste de Teclado do TE - Sucesso". NOTE: its only caller,
    /// CTesteTeclado::ProcessInput, also calls it when the wrong key was pressed (test failed).
    void LogaFimTesteTecladoSucesso();

    /// wasm func 5880 (name inferred): "Imprimindo relatório de estado da urna" (severity 1, body 3902).
    void LogaImprimindoRelatorioEstadoUrna();

    // ---- other methods seen in this unit (bodies elsewhere) ------------------------------------------
    //   func 5881 "Erro estado do aplicativo não esperado" (severity 3, body 3902)  - CPreZeresima
    //   func 5885 "Mídia de resultado não estava presente" (severity 3, body 3902)  - CCopiaResultadoParaMR
    //   func 4550 "Erro partido não encontrado"                                      - CPedeMajoritario
    //   Loga(texto) = IEventosLog::Loga (api_f233, severity 1); LogaAviso(texto) = api_f1398 (severity 2)
    //   see also clogvota.u09.cpp: LogaImpressaoRelatorio (1127), LogaMesarioIndagadoQualidadeBU (4551),
    //   LogaMesarioIndagadoImprimirZeresima (5882).

private:
    /// wasm func 3902 (merged body, name inferred): writes a 38-byte literal with the given severity.
    void Loga38(api::ESeveridade severidade, const char (&texto)[39]) const;
};

} // namespace vota

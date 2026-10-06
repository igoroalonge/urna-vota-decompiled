// FRAGMENT reconstructed by unit u09 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/log/clogvota.cpp (attested by other std::source_location records).
// vota::CLogVota methods whose only callers are in unit u09 (the tools attributed them to the callers'
// files). CLogVota +4 is the api::CLoga*; Loga(msg) = api_f233 = CLoga::loga(level 1, msg);
// LogaErro(msg) = func 2282 = CLoga::loga(level 3, msg). Messages are Latin-1 in the binary.

#include "vota/log/clogvota.h"

#include <format>
#include <string>

#include "comum/log/ieventoslog.h"

namespace vota {

// wasm func 1127 — "Imprimindo relatório [<nome>] via nº [<via>]" (<nome> from
// comum::IEventosLog::ConverteERelatoriosUE, func 5879: 0 ZERESIMA, 1 ZERÉSIMA DE APURAÇÃO,
// 2 ZERÉSIMA DE SEÇÃO, 3 BU, 4 BUJ, 5 RDV, 6 AUTOTESTE, 7 COMPROVANTE DE CARGA, 8 HASHES ARQUIVOS,
// 9 BIM, 10 ARQUIVOS DE ELEITORES E CANDIDATOS, 11 RESUMO DA ZERÉSIMA,
// 12 ELEITORES HABILITADOS BIOGRAFICAMENTE; other values throw EUeComumLogError 8852 "Valor inválido").
// Callers: every printing state of u09 and CEmitirMaisBU.                         name as in unit u08
void CLogVota::LogaImpressaoRelatorio(comum::ERelatoriosUE relatorio, uebyte via)
{
    Loga(std::format("Imprimindo relatório [{}] via nº [{}]",
                     comum::IEventosLog::ConverteERelatoriosUE(relatorio), via));
}

// wasm func 4551 — callers: CImprimindoBU::StartState / ProcessInput.             name inferred
void CLogVota::LogaMesarioIndagadoQualidadeBU()
{
    Loga("Mesário indagado sobre qualidade do Boletim de Urna");
}

// wasm func 5882 — callers: CQuerReimprimirZeresima::StartState, CQuerImprimirZeresima::StartState.
// (Calls CLoga::loga(level 1) directly instead of through api_f233.)             name inferred
void CLogVota::LogaMesarioIndagadoImprimirZeresima()
{
    Loga("Mesário indagado se quer imprimir a zerésima");
}

}  // namespace vota

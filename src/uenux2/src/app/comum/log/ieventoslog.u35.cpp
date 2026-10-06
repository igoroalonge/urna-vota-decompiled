// FRAGMENT of uenux2/src/app/comum/log/ieventoslog.cpp (attested; owner u24, declarations in ieventoslog.h).
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/log/ieventoslog.h"

#include <format>
#include <string>

namespace comum {

// wasm func 1047 - name inferred (other units call it LogaGeracaoRelatorio). Not observed executing (reports are
// printed at the zerésima / encerramento, which the recorded sessions do not reach).
// Writes an informational record of the urna log (logd.dat), e.g.
//   "Gerando relatório [BU] [INÍCIO]"  ...  "Gerando relatório [BU] [TÉRMINO]"
// Callers: vota::CGeraResumoZeresimaBase (11943), vota::CGeraZeresimaBase (11946), vota::CGeraRelatorios (12105),
// vota::CGeraBU (12110). The strings are Latin-1 in the binary.
void IEventosLog::LogaGeracaoRelatorio(const ERelatoriosUE relatorio, const bool termino) const
{
    const std::string nome = ConverteERelatoriosUE(relatorio);                  // func 5879 (line 426)
    const std::string momento = termino ? "TÉRMINO" : "INÍCIO";
    Loga(std::format("Gerando relatório [{}] [{}]", nome, momento));             // api_f233
}

} // namespace comum

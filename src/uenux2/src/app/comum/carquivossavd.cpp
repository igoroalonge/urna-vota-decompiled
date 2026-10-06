// uenux2/src/app/comum/carquivossavd.cpp
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/carquivossavd.h"

#include <format>

#include "comum/comumdefs.h"      // CUeComumError = CBaseError<comum::EUeComumError, SErrorLimits{7200, 7600}>

namespace comum {

// wasm func 275 (srcloc line 49). Callers: SincronizaRelatorios, CSincronismoVotoEleitor (vote synchronisation),
// CGravaResultado, ... Returns a copy of the path. The id is formatted with the enum formatter (handle, slot 2142).
std::string CArquivosSavd::operator[](ESavdPacote pacote) const
{
    const auto it = m_pacotes.find(pacote);
    if (it == m_pacotes.end()) {
        throw CUeComumError(EUeComumError(7201),
            std::format("Arquivo de assinatura associado ao identificador {} não encontrado", pacote));   // line 49
    }
    return it->second;
}

// wasm func 680 (srcloc line 73). Callers: SincronizaRelatorios, CSincronismoVotoEleitor::vf2 (7174), func 4657.
std::string CArquivosSavd::operator[](ESavdArquivoUE arquivo) const
{
    const auto it = m_arquivos.find(arquivo);
    if (it == m_arquivos.end()) {
        throw CUeComumError(EUeComumError(7203),
            std::format("Arquivo assinado associado ao identificador {} não encontrado", arquivo));       // line 73
    }
    return it->second;
}

} // namespace comum

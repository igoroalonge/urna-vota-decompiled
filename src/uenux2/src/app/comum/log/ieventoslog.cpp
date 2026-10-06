// uenux2/src/app/comum/log/ieventoslog.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24; LogaInicioAplicacao / LogaVersaoAplicacao: unit u20).
//
// srclocs: :426 ConverteERelatoriosUE (wasm 5879), :440 ConverteFonteAlim and :456 ConverteStatusBateria
// (both only inlined, into CLogComum::LogaNivelBateria = wasm 5875). All texts are Latin-1 in the binary
// and are built in place (SSO) or with operator new + unaligned 8-byte copies from .rodata.
#include "comum/log/ieventoslog.h"

namespace comum {

// wasm func 2861 - "CUeComumLogError(code, msg, where)" constructor thunk: shared CBaseError body
// (ecourna func 710) with vtable @1552936. Curated name in the database is correct.

// wasm func 5879 (srcloc line 426). Callers: comum_f1047 and vota::CLogVota::LogaImpressaoRelatorio (1127):
// "Imprimindo relatório [<this text>] via nº [<n>]".
std::string IEventosLog::ConverteERelatoriosUE(const ERelatoriosUE relatorio)
{
    switch (relatorio) {
    case ERelatoriosUE::ZERESIMA:                              return "ZERÉSIMA";   // SSO: 8 Latin-1 bytes 5A 45 52 C9 53 49 4D 41
    case ERelatoriosUE::ZERESIMA_APURACAO:                     return "ZERÉSIMA DE APURAÇÃO";
    case ERelatoriosUE::ZERESIMA_SECAO:                        return "ZERÉSIMA DE SEÇÃO";
    case ERelatoriosUE::BU:                                    return "BU";
    case ERelatoriosUE::BUJ:                                   return "BUJ";
    case ERelatoriosUE::RDV:                                   return "RDV";
    case ERelatoriosUE::AUTOTESTE:                             return "AUTOTESTE";
    case ERelatoriosUE::COMPROVANTE_CARGA:                     return "COMPROVANTE DE CARGA";
    case ERelatoriosUE::HASHES_ARQUIVOS:                       return "HASHES ARQUIVOS";
    case ERelatoriosUE::BIM:                                   return "BIM";
    case ERelatoriosUE::ARQUIVOS_ELEITORES_CANDIDATOS:         return "ARQUIVOS DE ELEITORES E CANDIDATOS";
    case ERelatoriosUE::RESUMO_ZERESIMA:                       return "RESUMO DA ZERÉSIMA";
    case ERelatoriosUE::ELEITORES_HABILITADOS_BIOGRAFICAMENTE: return "ELEITORES HABILITADOS BIOGRAFICAMENTE";
    }
    throw CUeComumLogError(EUeComumLogError{8852}, "Valor inválido");                  // line 426
}

// Line 440 - inlined into wasm 5875. Power source ("fonte de alimentação") of the urna.
std::string IEventosLog::ConverteFonteAlim(const uebyte fonte)
{
    switch (fonte) {
    case 0: return "ALIMENTAÇÃO AC";                     // mains
    case 1: return "ALIMENTAÇÃO BATERIA INTERNA";
    case 2: return "ALIMENTAÇÃO BATERIA EXTERNA";
    }
    throw CUeComumLogError(EUeComumLogError{8853}, "Valor inválido");                  // line 440
}

// Line 456 - inlined into wasm 5875. Charge state of a battery (2-bit field of the IPower status).
std::string IEventosLog::ConverteStatusBateria(const uebyte status)
{
    switch (status) {
    case 0: return "PLENA";                              // full
    case 1: return "PARCIAL";                            // partial
    case 2: return "CRÍTICA";                            // critical (also raises the log severity to 2)
    case 3: return "AUSENTE";                            // absent
    }
    throw CUeComumLogError(EUeComumLogError{8854}, "Valor inválido");                  // line 456
}

} // namespace comum

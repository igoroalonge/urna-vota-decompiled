// uenux2/src/app/comum/log/clogcomum.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// Out-of-line functions of this file in the binary: 5875 (LogaNivelBateria, the tools call it
// "IEventosLog::ConverteFonteAlim" after an inlined callee), 5876, 5878, 3828 and the merged body 6045.
// The other methods exist only inlined (srclocs listed in the header) and are reconstructed from their
// inlined copies in vota::CGravaResultado (12098), vota::CThreadMonitor (10226) and
// comum::CControladorReconhecimentoMesario::ComparaDigitais (5372).
// Nothing of this file ran in the recorded sessions (no encerramento, no battery events).
#include "comum/log/clogcomum.h"

#include <format>
#include <string>

#include "api/hwil/ipower.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/log/ieventoslog.h"

namespace comum {

namespace {
IEventosLog& Log()      // every method calls this at its own line (the srcloc is that call site)
{
    return api::CPolySingletonList::instance<IEventosLog>();                   // func 837
}
} // namespace

std::mutex                 CLogComum::s_mutex;
std::unique_ptr<CLogComum> CLogComum::s_pInstancia;

// wasm 948 (unit u35): lazy singleton; body shared with other empty singletons (func 2900).
CLogComum& CLogComum::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_pInstancia)
        s_pInstancia.reset(new CLogComum());
    return *s_pInstancia;
}

// MSD / MSE: the urna's security module (MSD in the older models, MSE in UE2020 and later; the caller,
// CAssinador::AssinaArquivosResultado, picks MSE when IUrna::GetModelo() >= 2020).
void CLogComum::LogaIniciaSessaoMSD()   { Log().Loga("Inicia uma sessão no MSD"); }     // line 44
void CLogComum::LogaFinalizaSessaoMSD() { Log().Loga("Finaliza a sessão no MSD"); }     // line 50
void CLogComum::LogaIniciaSessaoMSE()   { Log().Loga("Inicia uma sessão no MSE"); }     // line 56
void CLogComum::LogaFinalizaSessaoMSE() { Log().Loga("Finaliza a sessão no MSE"); }     // line 62

// wasm func 6045 (tools: comum_f6045) - merge-similar-functions body of the two methods below; it
// receives the format string as a (begin, end) pair and the source_location record as parameters:
//   (extensao, fmt_end, fmt_begin, srcloc)
// Body: log = instance<IEventosLog>(srcloc); nome = CArquivosResultado::GetInst()[extensao];
//       log.Loga(std::vformat(fmt, nome)).

// wasm func 5878 (srcloc line 76). "FI" = flash interna, printed as "MI" (mídia interna).
void CLogComum::LogaCopiandoArqResParaFI(EExtensaoArquivoResultado extensao)
{
    auto& log = Log();                                                                   // line 76
    log.Loga(std::format("Copiando arquivo de resultado para MI: [{}]",
                         CArquivosResultado::GetInst()[extensao]));                      // funcs 348, 347
}

// wasm func 3828 (srcloc line 84). "FE" = flash externa, printed as "ME" (mídia externa).
void CLogComum::LogaResultadoCopiadoResFE(EExtensaoArquivoResultado extensao)
{
    auto& log = Log();                                                                   // line 84
    log.Loga(std::format("Copiando arquivo de resultado para ME: [{}]",
                         CArquivosResultado::GetInst()[extensao]));
}

// wasm func 5876 (srcloc line 92). Called twice per result file by CGravacaoResultados::Executa:
//   "Gerando arquivo de resultado [bu.dat] + [Início]" ... "[Término]"
void CLogComum::LogaGerandoResultados(EExtensaoArquivoResultado extensao, EStatusOperacaoRelatorio status)
{
    auto& log = Log();                                                                   // line 92
    const std::string estado = (status == EStatusOperacaoRelatorio::INICIO) ? "Início" : "Término";
    log.Loga(std::format("Gerando arquivo de resultado [{}] + [{}]",
                         CArquivosResultado::GetInst()[extensao], estado));
}

// wasm func 5875 (srcloc line 106; ConverteFonteAlim :440 and ConverteStatusBateria :456 inlined).
// Called by comum::util::CMonitoraAlimentacao::VerificaAlimentacao (inlined in CThreadMonitor, 10226)
// when the charge state of a battery changes: LogaNivelBateria(1, interna) / LogaNivelBateria(2, externa).
// A critical battery is logged with severity 2, everything else with severity 1.
void CLogComum::LogaNivelBateria(uebyte fonteAlimentacao, uebyte statusBateria)
{
    auto& log = Log();                                                                   // line 106
    const std::string mensagem = std::format("Carga da [{}]: [{}]",
                                             log.ConverteFonteAlim(fonteAlimentacao),     // throws 8853
                                             log.ConverteStatusBateria(statusBateria));   // throws 8854
    log.Loga(api::ESeveridade{statusBateria == 2 /*CRÍTICA*/ ? 2 : 1}, mensagem);
}

// Line 136 - inlined into 10226 after 20 power-source changes (counter +7 of the monitor reaches 20).
void CLogComum::LogaSuspensoMonitoramenteRepeticao(uebyte eventos)
{
    auto& log = Log();                                                                   // line 136
    log.Loga(std::format(
        "Suspenso monitoramento de alimentação devido repetição de eventos. [ {} ] eventos", eventos));
}

// Lines 144 / 145 / 158 - inlined into 10226. Power source = bits 1..2 of the IPower status byte (+4),
// refreshed by IPower vtable slot 15.
void CLogComum::LogaTipoBateria()
{
    auto& log = Log();                                                                   // line 144
    auto& power = api::CPolySingletonList::instance<api::IPower>();                      // line 145 (func 862)
    power.AtualizaStatus();                                                              // slot 15   name inferred
    switch (power.GetFonteAlimentacao()) {                                               // (byte +4 >> 1) & 3
    case 1:  log.Loga("Urna operando na bateria interna"); break;
    case 2:  log.Loga("Urna operando na bateria externa"); break;
    case 3:  throw CUeComumLogError(EUeComumLogError{8850},
                                    "Tipo de alimentação não implementado");             // line 158
    default: log.Loga("Urna operando na rede elétrica"); break;                          // 0
    }
}

void CLogComum::LogaInicioProcedimentoAssinatura()                                       // line 166
{
    Log().Loga("Início do procedimento de assinatura dos arquivos de resultados");
}

void CLogComum::LogaTerminoProcedimentoAssinatura()                                      // line 172
{
    Log().Loga("Término do procedimento de assinatura dos arquivos de resultados");
}

void CLogComum::LogaPreparandoAssinaturaArquivosResultado()                              // line 178
{
    Log().Loga("Preparando para assinatura dos arquivos de resultados");
}

// Line 214 - inlined into CControladorReconhecimentoMesario::ComparaDigitais (5372).
void CLogComum::LogaScoreReconhecimentoMesario(const ueint32 score)
{
    auto& log = Log();                                                                   // line 214
    log.Loga(std::format("Batimento de digitais retornou o score {}", score));
}

} // namespace comum

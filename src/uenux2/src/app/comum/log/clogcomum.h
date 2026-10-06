// uenux2/src/app/comum/log/clogcomum.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// comum::CLogComum = fixed log records shared by the applications of the urna (VOTA, SA, ...): result
// generation at the end of the day (encerramento), signature sessions with the security module, power
// supply / battery monitoring and poll-worker fingerprint matching. Every method writes one record through
// api::CPolySingletonList::instance<IEventosLog>() (the srcloc line of each method is the line of that call).
//
// No RTTI, no data member. Instance = lazy singleton of an empty (1-byte) class: wasm 948 (unit u35,
// unique_ptr @1838636, mutex @1838612, atexit reset 11640); callers do CLogComum::GetInst().LogaXxx(...).
// None of the methods uses `this`, so wasm-opt dead-argument elimination removed it from the three
// out-of-line bodies (5875, 5876, 6045).
#pragma once

#include <mutex>
#include <memory>

#include "comum/carquivosresultado.h"     // EExtensaoArquivoResultado
#include "comum/comumtypes.h"             // uebyte, ueint32   (header name ?)

namespace comum {

// Second argument of LogaGerandoResultados: 0 -> "Início", anything else -> "Término".
enum class EStatusOperacaoRelatorio : int { INICIO = 0, TERMINO = 1 };        // enumerator names inferred

class CLogComum {
public:
    static CLogComum& GetInst();                                               // wasm 948 (unit u35)

    void LogaIniciaSessaoMSD();                                                // line 44  (inlined in 12098)
    void LogaFinalizaSessaoMSD();                                              // line 50  (inlined in 12098)
    void LogaIniciaSessaoMSE();                                                // line 56  (inlined in 12098)
    void LogaFinalizaSessaoMSE();                                              // line 62  (inlined in 12098)
    void LogaCopiandoArqResParaFI(EExtensaoArquivoResultado extensao);         // line 76  wasm 5878
    void LogaResultadoCopiadoResFE(EExtensaoArquivoResultado extensao);        // line 84  wasm 3828
    void LogaGerandoResultados(EExtensaoArquivoResultado extensao,
                               EStatusOperacaoRelatorio status);               // line 92  wasm 5876
    void LogaNivelBateria(uebyte fonteAlimentacao, uebyte statusBateria);      // line 106 wasm 5875
    void LogaSuspensoMonitoramenteRepeticao(uebyte eventos);                   // line 136 (inlined in 10226)
    void LogaTipoBateria();                                                    // line 144..158 (inlined in 10226)
    void LogaInicioProcedimentoAssinatura();                                   // line 166 (inlined in 12098)
    void LogaTerminoProcedimentoAssinatura();                                  // line 172 (inlined in 12098)
    void LogaPreparandoAssinaturaArquivosResultado();                          // line 178 (inlined in 12098)
    void LogaScoreReconhecimentoMesario(const ueint32 score);                  // line 214 (inlined in 5372)

private:
    static std::mutex                 s_mutex;        // @1838612
    static std::unique_ptr<CLogComum> s_pInstancia;   // @1838636
};

} // namespace comum

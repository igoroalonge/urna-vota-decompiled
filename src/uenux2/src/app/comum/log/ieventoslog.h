// uenux2/src/app/comum/log/ieventoslog.h
// Reconstructed from vota_web_wasm.wasm (unit u24; other methods in units u09, u12, u20).
//
// comum::IEventosLog = the application's event-log interface (the urna log "logd.dat", Latin-1 text
// records "<app>|<severity>|<text>"). RTTI: class without base, vtable @1552860 = {174 (D1), 144 (D0)}:
// only a virtual destructor. The single implementation is vota::CLogVota (vtable @1532792), created and
// pushed into api::CPolySingletonList by CLogVota::GetInst() (wasm 184); code of comum/ reaches it as
// api::CPolySingletonList::instance<IEventosLog>().
//
// sizeof 8: +0 vptr, +4 api::ELogAplicativos m_aplicativo (CLogVota passes 1).
#pragma once

#include <string>

#include "api/uelog/cloga.h"                       // api::CLoga::loga(ELogAplicativos, ESeveridade, msg)
#include "comum/comumtypes.h"                      // uebyte, EUrnaTurno   (header name ?)
#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

// CBaseError<EUeComumLogError, SErrorLimits{8850, 8900}> (typeinfo @1552868, vtable @1552936).
// Constructor thunk = wasm 2861. Codes: 8850 LogaTipoBateria, 8852 ConverteERelatoriosUE,
// 8853 ConverteFonteAlim, 8854 ConverteStatusBateria. 8851 is not used as an error code anywhere (the
// two i32.const 8851 of the module, in api_f4683 and CMontadorHash::CalculaHashGeral, are the function-table
// slot of api_f1055 passed to invoke_viii).
enum class EUeComumLogError : int {};
using CUeComumLogError =
    ecourna::api::exception::CBaseError<EUeComumLogError, ecourna::api::exception::SErrorLimits{8850, 8900}>;

// Reports printed by the urna (names inferred from the texts of ConverteERelatoriosUE).
enum class ERelatoriosUE : int {
    ZERESIMA = 0,                               // "ZERÉSIMA"
    ZERESIMA_APURACAO = 1,                      // "ZERÉSIMA DE APURAÇÃO"
    ZERESIMA_SECAO = 2,                         // "ZERÉSIMA DE SEÇÃO"
    BU = 3,                                     // "BU"   Boletim de Urna
    BUJ = 4,                                    // "BUJ"  Boletim de Justificativa
    RDV = 5,                                    // "RDV"  Registro Digital do Voto
    AUTOTESTE = 6,                              // "AUTOTESTE"
    COMPROVANTE_CARGA = 7,                      // "COMPROVANTE DE CARGA"
    HASHES_ARQUIVOS = 8,                        // "HASHES ARQUIVOS"
    BIM = 9,                                    // "BIM"
    ARQUIVOS_ELEITORES_CANDIDATOS = 10,         // "ARQUIVOS DE ELEITORES E CANDIDATOS"
    RESUMO_ZERESIMA = 11,                       // "RESUMO DA ZERÉSIMA"
    ELEITORES_HABILITADOS_BIOGRAFICAMENTE = 12, // "ELEITORES HABILITADOS BIOGRAFICAMENTE"
};

class IEventosLog {
public:
    virtual ~IEventosLog() = default;                                   // vtable slot 0/1 (ICF 174 / 144)

    // Informational record: CLoga::loga(m_aplicativo, 1, msg). Out-of-line copy = api_f233 (other unit).
    void Loga(const std::string& mensagem) const { api::CLoga::loga(m_aplicativo, api::ESeveridade{1}, mensagem); }
    // With explicit severity (1 info, 2 warning, 3 error: api_f1398 / vota_f2282 are the fixed-level copies).
    void Loga(api::ESeveridade severidade, const std::string& mensagem) const
    {
        api::CLoga::loga(m_aplicativo, severidade, mensagem);
    }

    // Text of enum values used in log records. Non-static in the source (the srcloc pretty names have no
    // "static"); the unused `this` was removed by wasm-opt dead-argument elimination.
    std::string ConverteERelatoriosUE(const ERelatoriosUE relatorio);   // wasm 5879 (line 426)
    std::string ConverteFonteAlim(const uebyte fonte);                  // line 440, inlined in 5875
    std::string ConverteStatusBateria(const uebyte status);             // line 456, inlined in 5875

    void LogaInicioAplicacao(EUrnaTurno turno);                         // wasm 11641 (unit u20)
    void LogaVersaoAplicacao();                                         // wasm 11642 (unit u20)

protected:
    explicit IEventosLog(api::ELogAplicativos aplicativo) : m_aplicativo(aplicativo) {}

    api::ELogAplicativos m_aplicativo;                                  // +4
};

} // namespace comum

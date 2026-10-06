// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cpe.h
//
// CPE = "Processo Eleitoral": the md::CProcessoEleitoral read from <fase><pe>-cp.dat
// (ASN.1 ModuloProcessoEleitoral::EntidadeProcessoEleitoral, e.g. t02400-cp.dat) plus the pleito of
// the current round. Singleton (204-byte object). Used by report headers (CRelUtil::
// IncluiCabecalhoEleicoesMZS, CGeradorRelVersaoPacoteDados, CRelatorioTesteImpressora,
// vota::CImpressaoPU): CreateInst is called lazily by those report generators, not at start-up.
#pragma once

#include <mutex>
#include <string>

#include "md/processoeleitoral/cprocessoeleitoral.h"

namespace comum {

class CPE {
public:
    static CPE& GetInst();                                                   // line 21 (wasm 1073)
    static void CreateInst(const md::estadoaplicacao::CEstadoGeral& eg,
                           const EFlashOrigem origem);                       // line 26 (wasm 2787)
    ~CPE();                                                                  // wasm 5598

private:
    CPE(const md::CProcessoEleitoral& pe, const md::estadoaplicacao::CEstadoGeral& eg);

    // Identification of the processo/pleito used by the reports.                      // name inferred
    struct SIdentificacaoPE {
        int                 fase;         // +180  CEstadoGeral +48 (fase: '1'..'3'?)
        std::uint32_t       processo;     // +184  CProcessoEleitoral +0 (id)
        std::string         texto;        // +188  copied from CEstadoGeral +8 by comum_f1879   // ?
        const md::CPleito*  pleito;       // +200  pleito of the current round
    };

    md::CProcessoEleitoral m_pe;           // +0   (176 bytes: id +0, nome +4, +16 string, +28 date?,
                                           //       pleito1 +36, optional<CPleito> pleito2 +96 (flag +156),
                                           //       +160 int, +164 vector)
    int                    m_turno;        // +176 first 4 bytes of CEstadoGeral +32 (CDadoCarga, turno char)
    SIdentificacaoPE       m_identificacao; // +180

    static std::mutex s_mutex;             // @1839088
    static CPE*       s_inst;              // @1839112 (released by wasm 11226, at exit?)
};

}  // namespace comum

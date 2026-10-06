// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cconfiguracaoeleicao.h
//
// CConfiguracaoEleicao = the configuration of the elections held in this urna: the processo
// eleitoral, the pleito (all elections of the day, with their cargos and the version of each
// election's data package), the parâmetros da urna, the section identification, the voter
// identifier types... Singleton built at start-up (CreateInst, cconfiguracaoeleicao.cpp:103/117/130,
// and the constructor lambda at :239, all inlined into 7787 / unit u02).
//
// Only GetInst and GetCargo exist as functions in u04; GetEleicao (line 293) is inlined in
// CCargos::GetCurrentEleicao. Layout offsets below come from the accesses made by u04 code.
#pragma once

#include <cstdint>
#include <mutex>
#include <string>

#include "md/processoeleitoral/cpleito.h"

namespace comum {

using TEleicaoID = std::uint32_t;
using TCargoID   = std::uint8_t;

namespace md {
class CCargo;
class CEleicaoPE;
enum class EOrigemConfiguracao : int { OFICIAL = 1, COMUNITARIA = 2 };   // ASN.1 OrigemConfiguracao
}  // namespace md

class CConfiguracaoEleicao {
public:
    static CConfiguracaoEleicao& GetInst();                          // line 36  (wasm 187)
    static void CreateInst(const md::estadoaplicacao::CEstadoGeral& eg,
                           const md::TVectorIdentificacaoAgregada& agregadas,
                           const EFlashOrigem origem);               // lines 103/117/130 (7787)

    const md::CEleicaoPE& GetEleicao(const TEleicaoID id) const;     // line 293 (inlined)
    const md::CCargo& GetCargo(const TCargoID id) const;             // line 316 (wasm 861)

    std::uint32_t GetIdProcessoEleitoral() const { return m_idProcesso; }      // QR "PROC:"
    md::EOrigemConfiguracao GetOrigem() const { return m_origem; }             // QR "ORLC:"
    const md::CPleito& GetPleito() const { return m_pleito; }                  // QR "PLEI:", "DTPL:"
    std::uint8_t GetLimiteVerificacoesDadoEleitor() const { return m_limiteVerificacoes; } // name inferred

private:
    std::uint32_t            m_idProcesso;          // +0    ?  printed as "PROC:{}"
    std::string              m_nome;                // +4    ?
    // +16 ?
    md::EOrigemConfiguracao  m_origem;              // +20   2 = comunitária ("COM"), else "LEG"
    // +24 ?
    md::CPleito              m_pleito;              // +28   id +28, nome +32, data +44 (CDate),
                                                    //       eleições vector<CEleicaoPE> +52..+64,
                                                    //       map<TEleicaoID,string> versões de pacote +64
    // ...
    std::uint8_t             m_limiteVerificacoes;  // +140  ? compared with the counter of
                                                    //       vota::CControlaReconhecimento (wasm 5404)
    // ...
    // +620  identificação da seção/urna used to build file names: fase (+620), processo (+624),
    //       UF (+628, std::string), município (+644), zona (+648, uint16)
    // +652.. ? (tipo identificador principal, tipos permitidos, parâmetros da urna, ...)

    static std::mutex             s_mutex;   // @1838756
    static CConfiguracaoEleicao*  s_inst;    // @1838752
};

}  // namespace comum

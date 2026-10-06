// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/ccargos.h
//
// CCargos = cursor over the offices ("cargos": Presidente, Governador, Senador, Prefeito, Vereador,
// referendum questions...) of every election ("eleição") of the pleito loaded in the urna.
// It is a process-wide singleton (CreateInst at ccargos.cpp:25 is inlined into the start-up function
// 7787). The voter state machine walks it cargo by cargo (First/Next/GetCurrent...) while the voter
// votes, and the BU / RDV / QR-code generators walk it again at the end of the day.
//
// Not polymorphic (no RTTI, no vtable). Size 28 bytes (operator new(28) in func 7787).
#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

namespace comum {

using TEleicaoID = std::uint32_t;   // "IDEL" in the QR code, CEleicaoPE +0
using TCargoID   = std::uint8_t;    // CCargo +0 (codigo)

namespace md {
class CCargo;        // md/processoeleitoral/ccargo.h  (140 bytes)
class CEleicaoPE;    // md/processoeleitoral/celeicaope.h (52 bytes)
enum class ETipoAbrangencia : int { MUNICIPAL = 0, ESTADUAL = 1, FEDERAL = 2 };  // names inferred
}  // namespace md

class CCargos {
public:
    // One entry per (election, office) pair, built by comum_f3774 from
    // CConfiguracaoEleicao::GetPleito().GetEleicoes()[i].GetCargos().
    struct SCargoEleicao {          // 8 bytes                                   // name inferred
        TEleicaoID eleicao;         // +0
        TCargoID   cargo;           // +4
    };

    static CCargos& GetInst();      // ccargos.cpp:24  (wasm 273; also inlined in 11556)
    static void CreateInst();       // ccargos.cpp:25  (inlined into 7787, unit u02)

    void First();                                        // wasm 2269  name inferred
    bool IsEnd() const;                                  // shared_f602 (not in u04) name inferred
    void Next();                                         // ccargos.cpp:46
    const md::CCargo& GetCurrent() const;                // ccargos.cpp:59
    TCargoID GetCurrentCargoID() const;                  // ccargos.cpp:68
    const md::CEleicaoPE& GetCurrentEleicao() const;     // ccargos.cpp:76
    TEleicaoID GetCurrentEleicaoID() const;              // ccargos.cpp:85
    std::string GetCurrentEleicaoVersaoPacote() const;   // ccargos.cpp:93 (only inlined, in 5604)
    void FiltraPorAbrangencia(md::ETipoAbrangencia abr); // ccargos.cpp:249 (inlined in 7377, u06)
    void OrdenaPorOrdemImpressao();                      // comum_f3782 (not in u04) name inferred
    void RestauraTodos();                                // comum_f3784 (not in u04) name inferred:
                                                         // m_cargos = m_todos (undoes the per-voter filter);
                                                         // called by CGeraBU (12110) and CGravaResultado (12098)

private:
    std::size_t                m_indice = 0;   // +0   current position in m_cargos
    std::vector<SCargoEleicao> m_todos;        // +4   every cargo of the pleito (unfiltered)
    std::vector<SCargoEleicao> m_cargos;       // +16  cargos the current voter can vote for

    static std::mutex s_mutex;                 // @1838696 (lock is a no-op in this build)
    static CCargos*   s_inst;                  // @1838720
};

}  // namespace comum

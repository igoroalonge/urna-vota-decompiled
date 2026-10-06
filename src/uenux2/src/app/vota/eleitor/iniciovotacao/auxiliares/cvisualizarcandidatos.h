// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cvisualizarcandidatos.h (path inferred from
// the .cpp, attested by srcloc lines 132, 140).
//
// "Visualizar candidatos" = operator menu "Mais informações" -> "Visualização de candidatos": pages
// through the candidacies (one screen per candidate: cargo, party, number, name, gender, photo,
// situation) that pass the filters chosen in the filter menus (cargo / partido / número).
//
// RTTI: comum::CAppState <- vota::CVisualizarCandidatos (typeinfo @1545740, vtable @1545672)
//   [0] ~dtor 2871 [1] deleting 11877 [2] StartState 11876 [3..8] defaults
#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "comum/cappstate.h"
#include "comum/dados/md/candidatura/ccandidatura.h"

namespace vota {

// Elements of m_candidaturas are copies of comum::md::CCandidatura (72 bytes; the RTTI of the lambdas of
// CTelasVota::CriaTelaVisualizacaoCandidato(const comum::md::CCandidatura&, unsigned long, unsigned long)
// names the type). Fields used here:
//   +0 uint8_t cargo (TCargoID), +2 uint16_t partido, +4 int32_t numero, +8 std::string,
//   +20 std::string nome, +32 std::optional<std::string> (flag +44),
//   +48 int gênero (1 masculino, 2 feminino, else "não informado"; resposta id for consultas),
//   +52 int inapto (!= 0 -> "INAPTO", no photo), +56 uint8_t, +60 member (shared_f1719)

class CVisualizarCandidatos final : public comum::CAppState {
public:
    /// Lazy singleton, wasm func 1279 (unit u02): @1834660; at-exit reset = wasm func 11880.
    static CVisualizarCandidatos& GetInst();

    ~CVisualizarCandidatos() override;           // wasm func 2871; deleting dtor 11877
    void StartState() override;                  // wasm func 11876 (srcloc 132, 140)

    void CarregaCandidaturas();                  // func 5953 (unit u02 fragment)

    std::vector<comum::md::CCandidatura> m_candidaturas;   // +12
    std::optional<uint8_t>  m_cargo;                         // +24 (engaged flag +25); 0 = "Todos"
    std::optional<int32_t>  m_numero;                        // +28 (flag +32)
    std::optional<uint16_t> m_partido;                       // +36 (flag +38); 0 = "Todos"
};

}  // namespace vota

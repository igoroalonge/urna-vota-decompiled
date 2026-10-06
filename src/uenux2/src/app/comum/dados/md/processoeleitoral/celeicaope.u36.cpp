// uenux2/src/app/comum/dados/md/processoeleitoral/celeicaope.cpp  --  FRAGMENT written by unit u36.
// Reconstructed from vota_web_wasm.wasm.
//
// md::CEleicaoPE (52 bytes): +0 TEleicaoID id, ..., +24 abrangência, +28 std::vector<CCargo> m_cargos
// (140-byte elements). CCargo +20 is std::optional<CDetalheCandidato> (engaged flag +84), present for
// elective offices and absent for referendum questions ("consultas", which use CCargo +88
// std::optional<CDetalheConsulta>).
#include "comum/dados/md/processoeleitoral/celeicaope.h"

#include <algorithm>

namespace comum::md {

// wasm func 5653                                                              // name inferred (u02)
// True when at least one cargo of the eleição is an elective office (has candidate details).
// Callers: CNomeArquivo::MontaNomesEleicao (3744: a pure referendum has no candidate file) and the
// start-up function 7787. Executed: 6 (municipal) to 12 (general) calls in votaInit (entry counter).
// The binary walks the vector by index ((end - begin) / 140 iterations), which any_of compiles to as well.
bool CEleicaoPE::PossuiCargoEletivo() const
{
    return std::ranges::any_of(m_cargos, [](const CCargo& cargo) {
        return cargo.GetDetalheCandidato().has_value();                     // byte +84
    });
}

}  // namespace comum::md

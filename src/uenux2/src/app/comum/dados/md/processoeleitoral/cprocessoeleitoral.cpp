// uenux2/src/app/comum/dados/md/processoeleitoral/cprocessoeleitoral.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :61.
// comum::md::CProcessoEleitoral: ... +96 std::optional<CPleito> m_pleito2 (60 bytes, engaged flag +156).
#include "comum/dados/md/processoeleitoral/cprocessoeleitoral.h"

#include "comum/dados/dadosdefs.h"   // CDadosError

namespace comum::md {

// wasm func 1267 (srcloc :61) - the second round exists only in elections that may have one
const CPleito& CProcessoEleitoral::GetPleito2() const
{
    if (!m_pleito2.has_value())
        throw CDadosError(EUeComumDadosError{8165}, "Não há pleito 2.");                     // :61
    return *m_pleito2;
}

} // namespace comum::md

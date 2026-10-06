// uenux2/src/app/comum/md/cabrangencia.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
#include "comum/md/cabrangencia.h"

#include <format>
#include <utility>

namespace comum::md {

// wasm func 3739 (srclocs lines 31, 34, 39, 42, 47, 51). Only caller: CConversorAbrangencia::DoDesconverte
// (11461), i.e. when parties/candidates/federations files are read. The members are stored first, then
// validated in this order (the wasm switch on uf.size() computes the "UF consistent with federal" flag used
// by the line-47 test in advance; the observable order of the exceptions is the one below).
CAbrangencia::CAbrangencia(ETipoAbrangencia tipo, const std::string& uf, TMunicipioID municipio)
    : m_tipo(tipo), m_uf(uf), m_municipio(municipio)
{
    if (!uf.empty() && uf.size() != 2)
        throw CUeComumMdError(EUeComumMdError{8900}, "UF inválida [" + uf + "]");                        // line 31

    if (uf.empty() && tipo != ETipoAbrangencia::Federal)
        throw CUeComumMdError(EUeComumMdError{8901}, "UF não informada para abrangência não federal");   // line 34

    if (tipo == ETipoAbrangencia::Municipal && municipio == 0)
        throw CUeComumMdError(EUeComumMdError{8902}, "Município zerado para abrangência municipal");     // line 39

    if (tipo != ETipoAbrangencia::Municipal && municipio != 0)
        throw CUeComumMdError(EUeComumMdError{8903}, "Município informado para abrangência não municipal"); // line 42

    if (tipo == ETipoAbrangencia::Federal && !uf.empty())
        throw CUeComumMdError(EUeComumMdError{8904}, "UF informada para abrangência federal");           // line 47

    if (static_cast<unsigned>(tipo) >= 3)
        throw CUeComumMdError(EUeComumMdError{8905},
                              std::format("Tipo de abrangência inválido: {}", std::to_underlying(tipo)));  // line 51
}

} // namespace comum::md

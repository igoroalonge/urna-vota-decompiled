// ecourna-lib/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14 (only the getter; the constructors are in unit u40).
#include "ecourna/app/dados/resultadournacadastro/chabilitacaobiometrica.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9016 (srcloc line 44)
const CEstadoHabilitacaoPorCodigo& CHabilitacaoBiometrica::GetEstadoHabilitacaoPorCodigo() const
{
    if (!m_porCodigo.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3275, "Estado de habilitação por código não definido para este objeto.");   // line 44
    }
    return *m_porCodigo;
}

} // namespace ecourna::app::dados

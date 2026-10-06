// ecourna-lib/ecourna/app/dados/midias/cautenticacao.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/midias/cautenticacao.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/dadoserros.h"
#include "ecourna/app/dados/midias/cinformacaomidia.h"

namespace ecourna::app::dados {

// wasm func 9036 (srcloc line 72).
// Both getters test BOTH optionals (bytes +8 and +24): a validity period is either complete or
// absent. Asking for the start date of an object that has only a start date throws.
const boost::posix_time::ptime& CAutenticacao::GetDataHoraInicial() const
{
    if (!(m_dataHoraInicial.has_value() && m_dataHoraFinal.has_value())) {   // ? written as a helper in the original
        throw CDadosMidiasError(3041, "Data e hora de início não definidos para este objeto.");   // line 72
    }
    return *m_dataHoraInicial;
}

// wasm func 9035 (srcloc line 82)
const boost::posix_time::ptime& CAutenticacao::GetDataHoraFinal() const
{
    if (!(m_dataHoraInicial.has_value() && m_dataHoraFinal.has_value())) {
        throw CDadosMidiasError(3042, "Data e hora de fim não definidos para este objeto.");      // line 82
    }
    return *m_dataHoraFinal;
}

} // namespace ecourna::app::dados

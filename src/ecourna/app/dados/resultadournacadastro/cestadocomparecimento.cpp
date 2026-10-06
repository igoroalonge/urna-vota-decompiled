// ecourna-lib/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/cestadocomparecimento.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// BUG (diagnostic only): the error codes/messages of GetApresentacaoFoto (line 55) and
// GetSituacaoHabilitacaoAudio (line 65) are swapped. Each getter tests the right optional, but the
// photo getter reports "Habilitação de áudio não definido..." (code 3269) and the audio getter
// reports "Dados de apresentação de foto de eleitor não definidos..." (code 3268).
#include "ecourna/app/dados/resultadournacadastro/cestadocomparecimento.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9021 (srcloc line 55): tests the flag at +32, returns this + 24.
const CApresentacaoFotoEleitor& CEstadoComparecimento::GetApresentacaoFoto() const
{
    if (!m_apresentacaoFoto.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3269, "Habilitação de áudio não definido para este objeto.");   // line 55 [sic]
    }
    return *m_apresentacaoFoto;
}

// wasm func 9020 (srcloc line 65): tests the flag at +40, returns the int at +36.
CEstadoComparecimento::ESituacaoHabilitacaoAudio CEstadoComparecimento::GetSituacaoHabilitacaoAudio() const
{
    if (!m_habilitacaoAudio.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3268,
                                               "Dados de apresentação de foto de eleitor não definidos para este objeto.");   // line 65 [sic]
    }
    return *m_habilitacaoAudio;
}

// wasm func 9019 (srcloc line 75): tests the flag at +92, returns this + 44.
const CHabilitacaoBiometrica& CEstadoComparecimento::GetHabilitacaoBiometrica() const
{
    if (!m_habilitacaoBiometrica.has_value()) {
        throw CDadosResultadoUrnaCadastroError(3267, "Habilitação biométrica não definida para este objeto.");   // line 75
    }
    return *m_habilitacaoBiometrica;
}

} // namespace ecourna::app::dados

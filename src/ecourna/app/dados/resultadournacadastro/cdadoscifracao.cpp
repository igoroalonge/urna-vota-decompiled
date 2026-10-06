// ecourna-lib/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/resultadournacadastro/cdadoscifracao.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/resultadournacadastro/cdadoscifracao.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 5091 (srcloc lines 27, 32). The three vectors are moved in. Only salt and
// informacaoAdicional are length-checked; `chave` may be empty.
CDadosCifracao::CDadosCifracao(std::vector<uebyte> chave, std::vector<uebyte> salt,
                               std::vector<uebyte> informacaoAdicional)
    : m_chave(std::move(chave))
    , m_salt(std::move(salt))
    , m_informacaoAdicional(std::move(informacaoAdicional))
{
    if (m_salt.size() < 16) {
        throw CDadosResultadoUrnaCadastroError(3282, "O tamanho de salt não pode ser menor do que 16.");                  // line 27
    }
    if (m_informacaoAdicional.size() < 16) {
        throw CDadosResultadoUrnaCadastroError(3283, "O tamanho de informacaoAdicional não pode ser menor do que 16.");   // line 32
    }
}

// wasm func 9054: implicit copy constructor (three vector copies; guard func 306).
// wasm func 2659: implicit destructor.
// wasm func 3487: CDadosComparecimentoCifrado's implicit destructor (4 vectors).

} // namespace ecourna::app::dados

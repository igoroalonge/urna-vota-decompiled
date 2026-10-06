// ecourna-lib/ecourna/app/dados/federacoes/cfederacao.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/federacoes/cfederacao.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/federacoes/cfederacao.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9047 (srcloc line 29). The members are copied first; the empty-list check follows.
// (func 2671, the vector's exception guard, is the libc++ __exception_guard<__destroy_vector>.)
CFederacao::CFederacao(TFederacaoID id, const std::string& sigla, const std::string& nome,
                       const TVectorNumeroPartido& partidos)
    : m_id(id)
    , m_sigla(sigla)
    , m_nome(nome)
    , m_partidos(partidos)
{
    if (m_partidos.empty()) {
        throw CDadosFederacoesError(2940, "A lista de partidos não pode ser vazia.");   // line 29
    }
}

// wasm func 9044: the implicit copy constructor (id, two strings, vector memcpy), address-taken
// (table slot 7043) and called by func 9045 (container code, unit u40).

} // namespace ecourna::app::dados

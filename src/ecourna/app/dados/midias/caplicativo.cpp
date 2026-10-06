// ecourna-lib/ecourna/app/dados/midias/caplicativo.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/midias/caplicativo.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
#include "ecourna/app/dados/dadoserros.h"
#include "ecourna/app/dados/midias/cinformacaomidia.h"

namespace ecourna::app::dados {

// wasm func 9043 (no srcloc; the tool attributed it to cconversoraplicativo.cpp, its only caller).
// Copies the 40 scalar bytes of CAutenticacao (two optional<ptime>, tamanhoSenha, numeroTentativas)
// and the hashSenha vector, then marks the optional engaged (+64 = 1). ? may be inline in caplicativo.h
CAplicativo::CAplicativo(ETipoAplicativo tipo, const CAutenticacao& autenticacao)
    : m_tipo(tipo)
    , m_autenticacao(autenticacao)
{
}

// wasm func 9042 (srcloc line 28)
const CAutenticacao& CAplicativo::GetAutenticacao() const
{
    if (!m_autenticacao.has_value()) {
        throw CDadosMidiasError(3040, "Autenticação não definida para este aplicativo.");   // line 28
    }
    return *m_autenticacao;
}

} // namespace ecourna::app::dados

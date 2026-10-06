// ecourna-lib/ecourna/app/dados/cregistroidentificacaoeleitor.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/cregistroidentificacaoeleitor.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// libc++ ABI v2 makes std::shared_ptr trivial_abi, so the by-value shared_ptr parameters are
// destroyed by the callee: each constructor copies them (use_count + 1) and releases the parameter
// at the end (use_count - 1). On a throw the members and the parameters are released
// (func 5105 = ~optional<shared_ptr>).
#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 1675 (srcloc line 23)
CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor habilitacao)
    : m_identificadorHabilitacao(habilitacao)
    , m_identificadorPrincipal(std::nullopt)
{
    if (!m_identificadorHabilitacao) {
        throw CDadosError(2029, "O identificador de habilitação do eleitor precisa estar definido.");   // line 23
    }
}

// wasm func 1878 (srcloc line 37). The optional is engaged even when `principal` is empty.
// Latent only in this binary: both callers pass a non-null principal (CConversorRegistroIdentificacaoEleitor
// ::DoDeconverte, func 9112, passes the result of CConversorIdentificadorEleitor::Deconverte, which is a
// make_shared or a throw; comum::CGravadorRCSecao, func 11616, passes CriaIdentidadeEleitor's result, func
// 5845, which is a make_shared or a throw).
CRegistroIdentificacaoEleitor::CRegistroIdentificacaoEleitor(TSharedIdentificadorEleitor habilitacao,
                                                             TSharedIdentificadorEleitor principal)
    : m_identificadorHabilitacao(habilitacao)
    , m_identificadorPrincipal(principal)
{
    if (!m_identificadorHabilitacao) {
        throw CDadosError(2030, "O identificador de habilitação do eleitor precisa estar definido.");   // line 37
    }
}

// wasm func 2669 (srcloc line 46)
TSharedIdentificadorEleitor CRegistroIdentificacaoEleitor::GetIdentificadorHabilitacao() const
{
    if (!m_identificadorHabilitacao) {
        throw CDadosError(2028, "O identificador de habilitação do eleitor não está definido.");        // line 46
    }
    return m_identificadorHabilitacao;
}

// wasm func 9260 (srcloc line 56)
TSharedIdentificadorEleitor CRegistroIdentificacaoEleitor::GetIdentificadorPrincipal() const
{
    if (!m_identificadorPrincipal.has_value()) {
        throw CDadosError(2027, "O identificador principal do eleitor não está definido.");             // line 56
    }
    return *m_identificadorPrincipal;
}

} // namespace ecourna::app::dados

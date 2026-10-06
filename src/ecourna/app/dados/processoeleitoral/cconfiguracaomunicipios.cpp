// ecourna-lib/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Library code compiled into this file: the map copy is std::map::insert(first, last) (func 9027,
// node = operator new(64), key at +16, CConfiguracaoMunicipio at +24) using
// __tree::__find_equal(hint, ...) (func 3489) and __tree_balance_after_insert.
// Observed executing during the recorded votes (VOTA start-up loads the -cfm.dat file).
#include "ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9028 (srcloc lines 25 and 31)
CConfiguracaoMunicipios::CConfiguracaoMunicipios(const CCabecalhoEntidade& cabecalho,
                                                 const TMapConfiguracaoMunicipio& configuracoes)
    : m_cabecalho(cabecalho)
    , m_configuracoes(configuracoes)
{
    if (m_configuracoes.empty()) {
        throw CDadosProcessoEleitoralError(3145, "A lista de configurações de municípios não pode ser vazia.");   // line 25
    }
    for (const auto& [codigo, configuracao] : m_configuracoes) {
        if (codigo != configuracao.GetCodigoMunicipio()) {
            throw CDadosProcessoEleitoralError(
                3146, "Cada índice no mapa deve ser igual ao código do município da configuração correspondente.");   // line 31
        }
    }
}

} // namespace ecourna::app::dados

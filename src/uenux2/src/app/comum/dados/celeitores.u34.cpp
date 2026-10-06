// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/dados/celeitores.cpp (attested by srcloc; owner unit u05).
//
// CEleitores::GetEleitoresImpedidos: reads the "impedidos" files of the voter roll (voters who are impeded
// from voting, e.g. suspended political rights, with the reason codes shown to the mesário as
// "Eleitor impedido - <motivo>"). One file per aggregated section (*-imp.dat, ModuloImpedidos, BER).
// Observed executing: votaInit loads the roll (start-up routine 7787 and CEleitores::CompleteLoad 6734).
#include <functional>
#include <string>
#include <vector>

#include "api/io/asn/cfileasn.h"
#include "comum/dados/asn/eleitor/cconversorimpedido.h"
#include "comum/dados/celeitores.h"

namespace comum {

// wasm func 5767 (tools: api_f5767). Name from the RTTI of its lambda
// std::__function::__func<CEleitores::GetEleitoresImpedidos(const std::vector<std::string>&)::$_0,
//                         std::vector<md::CImpedido>(const std::string&)>   (vtable @1559572).
// The wasm function takes no `this` (static member, or `this` removed by wasm-opt as unused).        ?
// A small generic helper "apply a loader to every file and concatenate" is inlined here: the lambda is
// wrapped in a std::function (two copies exist at run time, both destroyed at the end) and every partial
// vector is appended with vector::insert(end, first, last) (32-byte md::CImpedido elements).
std::vector<md::CImpedido> CEleitores::GetEleitoresImpedidos(const std::vector<std::string>& arquivos)
{
    const std::function<std::vector<md::CImpedido>(const std::string&)> carrega =
        [](const std::string& arquivo) {
            // $_0 = wasm func 11517: api::CFileASN::ReadFromFile<ModuloImpedidos::EntidadeImpedidos>(arquivo)
            // (cfileasn.h:78 "O arquivo [{}] não existe", :48/:60 size/open checks, :135/:143 BER decode and
            // validity) then CConversorImpedido().Desconverte(entidade) (iconversorasn.h:71).
            return asn::CConversorImpedido().Desconverte(
                api::CFileASN::ReadFromFile<ModuloImpedidos::EntidadeImpedidos>(arquivo));
        };

    std::vector<md::CImpedido> impedidos;
    for (const std::string& arquivo : arquivos) {
        const std::vector<md::CImpedido> parte = carrega(arquivo);   // std::bad_function_call if empty (never)
        impedidos.insert(impedidos.end(), parte.begin(), parte.end());
    }
    return impedidos;
}

// Library instantiations filed in unit u34 whose callers are in this file:
//   wasm func 5379  std::map<std::string, V>::insert(value_type&&) = __tree::__emplace_unique_key_args
//                   (84-byte node: std::string key at +16, 56-byte mapped value moved in; find via shared_f860).
//                   One body shared (identical code folding) by CEleitores::CompleteLoad (6734) and
//                   CDAORepositorio<CComparecimentoMesario>::IndexaIdentificacao / CDataMap::Update (5378).
//   wasm func 6733  std::set<int>::set(first, last) / insert(first, last) with end() hint (20-byte node,
//                   int key at +16). Callers: CEleitorEncontrado::StartState (10635: copy of the std::set<int>
//                   at CConfiguracaoEleicao +604, see celeitorencontrado.cpp) and the start-up routine (7787).

}  // namespace comum

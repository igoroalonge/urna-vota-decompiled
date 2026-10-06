// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadolocal.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/dados/asn/estadoaplicacao/cconversordadolocal.h"

#include "comum/asn/util.h"
#include "comum/dados/asn/estadoaplicacao/cconversorlocalidadeeleitoral.h"   // vtable @1569252

namespace comum::asn {

// wasm func 11403 (vtable slot 2). Observed executing: CConversorEstadoGeral::DoConverte writes eg.bin during
// votaInit (the web build's CAppInfoBuilder mock creates the state files).
ModuloEstadoGeralUrna::DadoLocal CConversorDadoLocal::DoConverte(const md::estadoaplicacao::CDadoLocal& local) const
{
    ModuloEstadoGeralUrna::DadoLocal entidade;
    entidade.set_tipoLocalVotacao(Utils::ConverteTipoLocalVotacao(local.GetTipoLocalVotacao()));   // func 3801
    entidade.set_uf(ModuloTiposEleitorais::SiglaUF(local.GetUF()));      // AbstractString(info @1913072, std::string)
    entidade.set_secaoCarga(CConversorLocalidadeEleitoral().Converte(local.GetLocalidade()));   // thunk 5696 (:56)
    return entidade;
}

} // namespace comum::asn

// uenux2/src/app/comum/gravadores/asn/cconversorcorrespresultado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// comum::asn::CConversorCorrespResultado : IConversorASN<ModuloTiposResultadosEcoUrna::CorrespondenciaResultado,
// md::CCorrespondenciaResultado> (vtable @1596224): { identificacao IdentificacaoUrna, carga Carga }.
// [2] DoConverte = func 10289, [3] DoDesconverte = func 10288. md::CCorrespondenciaResultado: see u05
// (src/uenux2/src/app/comum/dados/md/correspondencia/ccorrespondenciaresultado.h).
#include "comum/gravadores/asn/cconversorcorrespresultado.h"

#include <format>

#include "comum/gravadores/asn/cconversorcarga.h"
#include "comum/gravadores/iresultado.h"

namespace comum::asn {

// wasm func 10289 (vtable slot 2; srcloc :53)
CConversorCorrespResultado::TEntidade CConversorCorrespResultado::DoConverte(const TDado& dado) const
{
    TEntidade entidade;
    if (dado.GetSecao() == 0) {
        entidade.identificacao().set_identificacaoContingencia({dado.GetMunicipio(), dado.GetZona()});
    } else {
        switch (static_cast<int>(dado.GetTipoUrna())) {                               // +84
        case '1': case '3': case '4':
            // md::CCorrespondenciaResultado has no local: the ASN.1 IdentificacaoSecaoEleitoral gets the CONSTANT
            // local 1 (`((a[2])[1])[2] = 1` in the wasm; the official 2024 BUs show local 1 here too, while the
            // envelope carries the real local; docs/bu/build-a-bu.md).
            entidade.identificacao().set_identificacaoSecao(
                {{dado.GetMunicipio(), dado.GetZona()}, /*local*/ 1, dado.GetSecao()});
            break;
        case '2':
            entidade.identificacao().set_identificacaoContingencia({dado.GetMunicipio(), dado.GetZona()});
            break;
        case '0':
            throw CUeComumGravadoresError(8601, std::format("Tipo inválido de urna: {}",
                                                            static_cast<int>(dado.GetTipoUrna())));   // :53
        default:
            break;                                                                     // left unset (?)
        }
    }
    entidade.set_carga(CConversorCarga().Converte(dado.GetCarga()));                  // iconversorasn.h:56
    return entidade;
}

// wasm func 10288 (vtable slot 3; srcloc :85)
md::CCorrespondenciaResultado CConversorCorrespResultado::DoDesconverte(const TEntidade& entidade) const
{
    TMunicipioID municipio; TZonaID zona; TSecaoID secao = 0; md::ETipoUrna tipo;
    const auto& id = entidade.get_identificacao();
    switch (id.choiceIndex()) {
    case 0:  municipio = id.secao().municipioZona().municipio(); zona = id.secao().municipioZona().zona();
             secao = id.secao().secao(); tipo = md::ETipoUrna::SECAO; break;          // '1'
    case 1:  municipio = id.contingencia().municipio(); zona = id.contingencia().zona();
             tipo = md::ETipoUrna::CONTINGENCIA; break;                                 // '2'
    default: throw CUeComumGravadoresError(8602, "Entidade em formato incorreto");   // :85
    }
    const md::CCarga carga = CConversorCarga().Desconverte(entidade.get_carga());     // iconversorasn.h:71
    return md::CCorrespondenciaResultado(municipio, zona, secao, carga, tipo);         // func 2801
}

}  // namespace comum::asn

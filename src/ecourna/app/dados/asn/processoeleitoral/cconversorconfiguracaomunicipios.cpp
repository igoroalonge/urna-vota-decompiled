// ecourna-lib/ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipios.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/asn/cconversorcabecalhoentidade.h"
#include "ecourna/app/dados/asn/processoeleitoral/cconversorconfiguracaomunicipio.h"
#include "ecourna/app/dados/processoeleitoral/cconfiguracaomunicipios.h"
#include "ModuloConfiguracaoMunicipios.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorConfiguracaoMunicipios
//         : IConversorASN<ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios, CConfiguracaoMunicipios>
// vtable @1129768: [0] 174  [1] 144  [2] 9141 DoConverte (unit u40)  [3] 9137 DoDeconverte
//
//   EntidadeConfiguracaoMunicipios ::= SEQUENCE { cabecalho CabecalhoEntidade,
//                                                 configuracoes SEQUENCE OF ConfiguracaoMunicipio }
//   CConfiguracaoMunicipios: constructor func 9028 (const CCabecalhoEntidade&, const TMapConfiguracaoMunicipio&)
//   with TMapConfiguracaoMunicipio = std::map<TMunicipioID, CConfiguracaoMunicipio> (u14 header)
class CConversorConfiguracaoMunicipios
    : public api::asn::IConversorASN<ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios,
                                     CConfiguracaoMunicipios>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9141 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9137

private:
    // ? name and placement inferred (static member or file-local helper): wasm func 9134
    static TMapConfiguracaoMunicipio ConverteParaMapa(const std::vector<CConfiguracaoMunicipio>& configuracoes);
};

// wasm func 9137 (vtable slot 3; was "CConversorConfiguracaoMunicipios::vf3"). Observed at run time.
CConfiguracaoMunicipios CConversorConfiguracaoMunicipios::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorCabecalhoEntidade conversorCabecalho;           // vptr @1123740
    const CConversorConfiguracaoMunicipio conversorMunicipio;       // vptr @1129456

    const CCabecalhoEntidade cabecalho = conversorCabecalho.Deconverte(entidade.get_cabecalho());   // func 2665

    std::vector<CConfiguracaoMunicipio> configuracoes;               // push_back of a 40-byte POD, inlined
    for (const auto& configuracao : entidade.get_configuracoes()) {
        configuracoes.push_back(conversorMunicipio.Deconverte(configuracao));   // func 9136 -> 1167 -> 9142
    }

    // func 9134 ("libcxx_f9134"): out-of-line, signature (TMapConfiguracaoMunicipio* sret, const vector*):
    // it constructs the map itself and fills it keyed by the municipality code (first word of each
    // 40-byte record; node size 64). Each element goes through __find_equal(hint, ...) and the hint then
    // becomes the SUCCESSOR of the inserted/found node - the `iter = c.insert(iter, v); ++iter;` of a
    // std::insert_iterator, not a fixed end() hint. So: std::transform/std::copy into
    // std::inserter(mapa, mapa.end()) inside a helper that returns the map. ? Helper name and exact form
    // are not recoverable. Duplicate codes: insert() does not overwrite, so the FIRST record wins.
    const TMapConfiguracaoMunicipio porMunicipio = ConverteParaMapa(configuracoes);   // func 9134
    return CConfiguracaoMunicipios(cabecalho, porMunicipio);          // func 9028
}

// ? name inferred; wasm func 9134 (see above)
TMapConfiguracaoMunicipio CConversorConfiguracaoMunicipios::ConverteParaMapa(
    const std::vector<CConfiguracaoMunicipio>& configuracoes)
{
    TMapConfiguracaoMunicipio mapa;
    std::transform(configuracoes.begin(), configuracoes.end(), std::inserter(mapa, mapa.end()),
                   [](const CConfiguracaoMunicipio& configuracao) {
                       return TMapConfiguracaoMunicipio::value_type(configuracao.GetCodigoMunicipio(), configuracao);
                   });
    return mapa;
}

}  // namespace ecourna::app::dados::asn

// FRAGMENT reconstructed by unit u17 from vota_web_wasm.wasm.
// Original file (path inferred): uenux2/src/app/comum/asn/cconversorhasharquivo.cpp
// RTTI: comum::asn::CConversorHashArquivo : IConversorASN<ModuloTiposEleitorais::ArquivoAssinatura,
//       api::hash::CHashArquivo> (typeinfo @1599016, vtable @1599000):
//   [0] 174 (trivial dtor)  [1] 144  [2] 10267 DoConverte (other unit)  [3] 10269 DoDesconverte (u17)
// Conventions: see comum/dados/asn/cconversorlocal.h.
#include "api/hash/chasharquivo.h"
#include "comum/asn/iconversorasn.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

class CConversorHashArquivo
    : public IConversorASN<ModuloTiposEleitorais::ArquivoAssinatura, api::hash::CHashArquivo> {
protected:
    // TDado = api::hash::CHashArquivo (application object), TEntidade = the ASN.1 type.
    TEntidade DoConverte(const TDado& hash) const override;           // wasm func 10267
    TDado DoDesconverte(const TEntidade& arquivo) const override;     // wasm func 10269
};

// wasm func 10267 (other unit, for context): ArquivoAssinatura{nomeArquivo = hash.GetNome(),
// assinatura = hash.GetHash()} - both ASN.1 GeneralString members are assigned from the two std::strings.

// wasm func 10269
// ASN.1 {nome, hash} -> api::hash::CHashArquivo (validated by its constructor, func 3600).
api::hash::CHashArquivo CConversorHashArquivo::DoDesconverte(const TEntidade& arquivo) const
{
    return api::hash::CHashArquivo(arquivo.nomeArquivo, arquivo.assinatura);   // GeneralStrings
}

}  // namespace comum::asn

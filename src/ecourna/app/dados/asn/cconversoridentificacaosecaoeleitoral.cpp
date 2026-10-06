// ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11). Only DoDeconverte is in this unit.
#include "ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.h"

#include "ecourna/app/dados/asn/cconversormunicipiozona.h"   // CConversorMunicipioZona (9130/9131)

namespace ecourna::app::dados::asn {

// wasm func 9127 (vtable slot 3). Named "IConversorASN<MunicipioZona, CMunicipioZona>::Deconverte" by the
// tool because that call is inlined here (srcloc iconversorasn.hpp:66, record @1131040).
CIdentificacaoSecaoEleitoral CConversorIdentificacaoSecaoEleitoral::DoDeconverte(const TEntidade& entidade) const
{
    const CConversorMunicipioZona conversorMunicipioZona;   // vptr @1130584
    // validates field 0 (1902 on failure), then CConversorMunicipioZona::DoDeconverte (func 9130)
    const CMunicipioZona municipioZona = conversorMunicipioZona.Deconverte(entidade.get_municipioZona());

    // Arguments evaluated left to right: local (field 1, 32-bit) then secao (field 2, read as 16 bits).
    // The CBaseType constructors (cbasetype.hpp:39) throw when the value is outside 0..9999.
    return CIdentificacaoSecaoEleitoral(municipioZona, TNumeroLocal(entidade.get_local()),
                                        TNumeroSecao(entidade.get_secao()));   // func 5110
}

}  // namespace ecourna::app::dados::asn

// ecourna-lib/ecourna/app/dados/asn/cconversoridentificacaosecaoeleitoral.h   (path inferred: the types
//   are ModuloTiposEleitorais ones, whose converters (cconversoridentificadoreleitor.cpp, cconversorfase.cpp,
//   cconversorturno.cpp) sit directly in ecourna/app/dados/asn/)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/api/pattern/cbasetype.hpp"
#include "ecourna/app/dados/cidentificacaosecaoeleitoral.h"
#include "ModuloTiposEleitorais.h"

namespace ecourna::app::dados::asn {

using TNumeroLocal = CBaseType<unsigned int, 0, 9999, 8>;     // func 5849; alias name inferred
using TNumeroSecao = CBaseType<unsigned short, 0, 9999, 9>;   // func 5848; alias name inferred

// RTTI: CConversorIdentificacaoSecaoEleitoral
//         : IConversorASN<ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, CIdentificacaoSecaoEleitoral>
// vtable @1130788: [0] 174  [1] 144  [2] 9129 DoConverte (unit u40)  [3] 9127 DoDeconverte
//
//   IdentificacaoSecaoEleitoral ::= SEQUENCE { municipioZona MunicipioZona,
//                                              local INTEGER (1..9999), secao INTEGER (1..9999) }
//   CIdentificacaoSecaoEleitoral (16 bytes): +0 CMunicipioZona (8), +8 local (uint), +12 secao (ushort)
//   constructor func 5110 (CMunicipioZona, TNumeroLocal, TNumeroSecao)
class CConversorIdentificacaoSecaoEleitoral
    : public api::asn::IConversorASN<ModuloTiposEleitorais::IdentificacaoSecaoEleitoral,
                                     CIdentificacaoSecaoEleitoral>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9129 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9127
};

}  // namespace ecourna::app::dados::asn

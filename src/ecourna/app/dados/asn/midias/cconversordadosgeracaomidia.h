// ecourna-lib/ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.h   (path inferred: next to the srcloc-attested
//   cconversorinformacaomidia.cpp, which includes this name)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/midias/cinformacaomidia.h"   // CDadosGeracaoMidia (u14)
#include "ModuloInformacaoMidia.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorDadosGeracaoMidia : IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, CDadosGeracaoMidia>
// vtable @1125156: [0] 174  [1] 144  [2] 9203 DoConverte  [3] 9202 DoDeconverte
//   DadosGeracaoMidia ::= SEQUENCE { serialMidia GeneralString, usuario GeneralString,
//                                    identificadorGeradorMidia IdentificadorGeradorMidia, data DataHoraJE }
// "Dados de geração da mídia": which medium (serial), which operator, which generator and when the urna's flash
// card was prepared. Part of InformacaoMidia (infomidia.dat).
class CConversorDadosGeracaoMidia
    : public api::asn::IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, CDadosGeracaoMidia>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9203
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9202
};

}  // namespace ecourna::app::dados::asn

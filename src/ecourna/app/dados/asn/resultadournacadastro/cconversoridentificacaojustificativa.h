// ecourna-lib/ecourna/app/dados/asn/resultadournacadastro/cconversoridentificacaojustificativa.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u11).
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/api/pattern/cbasetype.hpp"
#include "ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.h"
#include "ecourna/app/dados/resultadournacadastro/cidentificacaojustificativa.h"
#include "ModuloResultadoUrnaCadastro.h"

namespace ecourna::app::dados::asn {

using TAnoNascimento = CBaseType<unsigned short, 0, 9999, 36>;   // alias name inferred (func 2277 = its ctor)

class CConversorIdentificacaoJustificativa
    : public api::asn::IConversorASN<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa,
                                     CIdentificacaoJustificativa>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9065 (unit u40)
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9064
};

}  // namespace ecourna::app::dados::asn

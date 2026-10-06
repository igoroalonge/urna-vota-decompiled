// ecourna-lib/ecourna/app/dados/asn/cconversorregistroidentificacaoeleitor.h   (path inferred: the data class
//   cregistroidentificacaoeleitor.cpp is srcloc-attested directly in ecourna/app/dados/, and the converters of
//   the shared types (cconversoridentificadoreleitor.cpp, cconversorcabecalhoentidade.cpp ...) sit directly in
//   ecourna/app/dados/asn/. Three u14 files include it as asn/resultadournacadastro/...; one u11 file as asn/.)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"
#include "ModuloTiposEcoUrna.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorRegistroIdentificacaoEleitor
//         : IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, CRegistroIdentificacaoEleitor>
// vtable @1131924: [0] 174  [1] 144  [2] 9114 DoConverte  [3] 9112 DoDeconverte
//
//   RegistroIdentificacaoEleitor ::= SEQUENCE { identificacaoUtilizada IdentificadorEleitor,
//                                              identificacaoPrincipal IdentificadorEleitor OPTIONAL }
// Used for voters (EstadoComparecimento), mesários (ComparecimentoMesario, EstadoHabilitacaoPorCodigo) and
// justifications (IdentificacaoJustificativa) of the attendance result file.
class CConversorRegistroIdentificacaoEleitor
    : public api::asn::IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, CRegistroIdentificacaoEleitor>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9114
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9112
};

}  // namespace ecourna::app::dados::asn

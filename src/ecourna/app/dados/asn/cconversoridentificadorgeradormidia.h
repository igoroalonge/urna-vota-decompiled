// ecourna-lib/ecourna/app/dados/asn/cconversoridentificadorgeradormidia.h   (path inferred: a ModuloTiposEcoUrna
//   type like RegistroIdentificacaoEleitor; asn/midias/ - where its only ecourna user, the DadosGeracaoMidia
//   converter, lives - is the other candidate)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40.
#pragma once

#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/midias/cinformacaomidia.h"   // CIdentificadorGeradorMidia (u14/u29)
#include "ModuloTiposEcoUrna.h"

namespace ecourna::app::dados::asn {

// RTTI: CConversorIdentificadorGeradorMidia
//         : IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>
// vtable @1123140 (stored by 6 functions: this class is also used by uenux2 code, e.g. CConversorCarga and
// comum::asn::CConversorDadoCorrespondencia 11400, the caller seen at run time).
//   IdentificadorGeradorMidia ::= SEQUENCE { nome GeneralString, serialCertificadoTPM GeneralString,
//                                           serialInstalacao GeneralString }
// "Gerador de mídia" = the machine/program that generated the urna's flash card. In the simulator data:
// nome "simulador-votacao-ng", serialCertificadoTPM = 64 '0'.
class CConversorIdentificadorGeradorMidia
    : public api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9225
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9224 (observed executing)
};

}  // namespace ecourna::app::dados::asn

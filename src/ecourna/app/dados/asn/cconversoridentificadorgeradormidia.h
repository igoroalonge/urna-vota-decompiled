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
// "Gerador de mídia" = the computer that generated the medium (on real urnas a GEDAI-UE PC; the BU's Carga holds
// the one that made the load medium). In the simulator, the eg.bin correspondence (so the BU) carries nome
// "nome_maquina", serialCertificadoTPM "12345678", serialInstalacao "99999999" (func 9952); "simulador-votacao-ng",
// 64 '0' and "A1B2C3DA" are the infomidia-fv-*-t.dat fixture's. Real values: nome Z<UF><zona:3>STD<2-3 digits>,
// the TPM's EK-certificate serial in 8/20/32/40 hex, an 8-hex installation serial (2026 urna data:
// investigation/README.md, findings E2, E3, E4, E5).
class CConversorIdentificadorGeradorMidia
    : public api::asn::IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>
{
protected:
    TEntidade DoConverte(const TDado& dado) const override;          // wasm func 9225
    TDado DoDeconverte(const TEntidade& entidade) const override;    // wasm func 9224 (observed executing)
};

}  // namespace ecourna::app::dados::asn

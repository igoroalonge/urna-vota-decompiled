// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocorrespondencia.h   (path inferred: RTTI only;
// included under this name by cconversorestadogeral.cpp / cconversorestadogeralgap.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloEstadoGeralUrna:
//   DadoCorrespondencia ::= SEQUENCE { carga Carga, secaoCarga DadoSecao, assinatura OCTET STRING }
// The identity of the media load ("carga") of this urna plus the section it was prepared for, and a signature
// blob. eg.bin keeps the current one (EstadoGeralUrna), gap.bin a history of up to 10 (printed on the BU as
// "histórico de cargas"). md::estadoaplicacao::CDadoCorrespondencia (96 bytes, unit u29) flattens the Carga:
//   +0 numeroInternoUrna  +4 numeroSerieFC (hex text)  +16 api::CDateTime dataHoraCarga  +28 codigoCarga
//   +40 CLocalidadeEleitoral secaoCarga  +48 std::vector<uebyte> assinatura  +60 CIdentificadorGeradorMidia
//
// RTTI: comum::asn::CConversorDadoCorrespondencia : IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia,
//       md::estadoaplicacao::CDadoCorrespondencia>, vtable @1568024: [2] 11399 DoConverte [3] 11400 DoDesconverte.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "ModuloEstadoGeralUrna.h"

namespace comum::asn {

class CConversorDadoCorrespondencia
    : public IConversorASN<ModuloEstadoGeralUrna::DadoCorrespondencia, md::estadoaplicacao::CDadoCorrespondencia>
{
protected:
    TEntidade DoConverte(const TDado& correspondencia) const override;      // wasm func 11399
    TDado DoDesconverte(const TEntidade& correspondencia) const override;   // wasm func 11400
};

} // namespace comum::asn

// uenux2/src/app/comum/gravadores/asn/cconversorcarga.h   (path inferred: RTTI only; included under this name by
// the attested gravadores/asn/cconversorcorrespresultado.cpp, unit u23)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// ModuloTiposEcoUrna:
//   Carga ::= SEQUENCE { numeroInternoUrna INTEGER (0..99999999), numeroSerieFC OCTET STRING (SIZE(4)),
//                        identificadorGeradorMidia IdentificadorGeradorMidia, dataHoraCarga DataHoraJE,
//                        codigoCarga GeneralString }
//   IdentificadorGeradorMidia ::= SEQUENCE { nome, serialCertificadoTPM, serialInstalacao }   (GeneralStrings)
// "Carga" = the media preparation session that loaded this urna (the "correspondência" between urna and data):
// internal urna number, serial of the flash card used (FC = "flash de carga"), the station that generated the media,
// date/time and the 24-digit load code. Part of the "urna" block of EVERY result file (BU, RDV, jufa, envelopes,
// hashes) through CConversorCorrespResultado.
//
// md::CCarga (76 bytes, unit u05): +0 numeroInternoUrna, +4 std::string numeroSerieFC (8 hex digits),
//   +16 api::CDateTime dataHoraCarga, +28 std::string codigoCarga,
//   +40 ecourna::app::dados::CIdentificadorGeradorMidia gerador (3 strings).
//
// RTTI: comum::asn::CConversorCarga : IConversorASN<ModuloTiposEcoUrna::Carga, comum::md::CCarga>
//       vtable @1596148: [0] 174 [1] 144 [2] 10291 DoConverte [3] 10290 DoDesconverte. sizeof 4.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/correspondencia/ccarga.h"
#include "ModuloTiposEcoUrna.h"

namespace comum::asn {

class CConversorCarga : public IConversorASN<ModuloTiposEcoUrna::Carga, md::CCarga>
{
protected:
    TEntidade DoConverte(const TDado& carga) const override;       // wasm func 10291
    TDado DoDesconverte(const TEntidade& carga) const override;    // wasm func 10290
};

} // namespace comum::asn

// FRAGMENT reconstructed by unit u34 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp (attested by the srcloc :284 of
// the preBuild lambda; reconstructed by unit u25). Field layout of CCabecalhoQRCode: see
// cgeradorbuqrcode.u04-fragment.cpp (33 std::string "TAG:valor " fields, 396 bytes, zero-filled by the
// constructor func 5624).
//
// The four setters below fill four fields of the HEADER of the BU QR codes ("BU digital" printed at the end of
// the Boletim de Urna and shown on the voter screen by CMostraQRCodeBU). Each stores the complete
// "TAG:valor " text, with the trailing blank that separates the fields of the payload:
//     ZONA:<zona eleitoral> SECA:<seção> IDUE:<nº interno da urna> IDCA:<código de carga, 24 chars max>
// Callers: vota::CGeraBU::StartState (func 12110, printed BU, inlined comum::CriaQRCode, cgeradorbu.cpp:124)
// and vota::(anonymous)::GetQRDSInst (func 1956, on-screen BU QR codes), always in the order
// SetZona, SetSecao, SetIdUrna, SetCodigoCarga, SetHistoricoCargas (func 5622, other unit).
// Their emptiness is checked later by CCabecalhoQRCodeBuilder::preBuild ("Campo (Zona) não informado.", 9050).
//
// The wasm signatures return void (the builder object is reused by the caller); the TSE names are unknown,
// the setter names follow the u19 reconstruction of GetQRDSInst.                           names inferred
//
// WEB BUILD: never executed (the simulator never reaches the encerramento / BU generation).
#include <format>
#include <string>

#include "comum/relatorios/ccabecalhoqrcodebuilder.h"

namespace comum {

// wasm func 5618. Format argument type 6 (unsigned int); the caller passes the 16-bit CEstadoGeral zona (+24).
//                                                                        "ZONA:{} " @440346 -> field +108
void CCabecalhoQRCodeBuilder::SetZona(unsigned zona)
{
    m_cabecalho.zona = std::format("ZONA:{} ", zona);
}

// wasm func 5620. Format argument type 6 (unsigned int); the caller passes the 16-bit CEstadoGeral seção (+26).
//                                                                        "SECA:{} " @440364 -> field +120
void CCabecalhoQRCodeBuilder::SetSecao(unsigned secao)
{
    m_cabecalho.seca = std::format("SECA:{} ", secao);
}

// wasm func 5621. Format argument type 6 (unsigned int): CEstadoGeral.carga.numeroInternoUrna (eg.bin +60).
//                                                                        "IDUE:{} " @440220 -> field +144
void CCabecalhoQRCodeBuilder::SetIdUrna(unsigned idUrna)
{
    m_cabecalho.idue = std::format("IDUE:{} ", idUrna);
}

// wasm func 5623. The parameter is a std::string (the body tests its SSO flag byte +11; the caller 12110
// passes &eg.bin+88 = CEstadoGeral.carga.codigoCarga, a std::string member); std::format stores it as
// format argument type 13 (string_view).
// "{:.24s}" = precision 24: a longer code is silently TRUNCATED to its first 24 characters.
//                                                                        "IDCA:{:.24s} " @440443 -> field +156
void CCabecalhoQRCodeBuilder::SetCodigoCarga(const std::string& codigoCarga)
{
    m_cabecalho.idca = std::format("IDCA:{:.24s} ", codigoCarga);
}

}  // namespace comum

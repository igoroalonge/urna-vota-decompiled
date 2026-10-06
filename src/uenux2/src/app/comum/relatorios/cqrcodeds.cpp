// uenux2/src/app/comum/relatorios/cqrcodeds.cpp   (path inferred: next to CQRCodeDS, whose operator() is
// wasm 5782 and whose declaration is in crelatoriotesteimpressora.h)
// Reconstructed from vota_web_wasm.wasm (unit u36).
//
// The "QR code do estado da urna": a QR code that identifies the urna and its carga (not the BU). It is shown
// on the zerésima-time screens (vota::adicionaBlocoMensagem, wasm 3059, rotating QR image refreshed every
// 15 s) and printed at the bottom of the "ESTADO DA URNA" report (CRelatorioTesteImpressora, 11200).
// The two functions below produce the list of (tag, value) pairs; CQRCodeDS::operator() (5782) then appends
// the volatile fields (VERS, date of the pleito, ...) and joins everything as "TAG:value TAG:value ...".
//
// Observed executing: 5636 runs in votaInit (the start-up builds the zerésima screens, 3059). An entry counter
// (review run) gives 5 calls of 5636 and 5 of 5783 during votaInit in every scenario.
#include <cctype>
#include <string>
#include <utility>
#include <vector>

#include "api/util/cdate.h"
#include "api/util/ctime.h"
#include "comum/dados/md/estadoaplicacao/cestadogeral.h"
#include "ecourna/api/util/cstringutils.h"            // ParaHex = wasm 1243 (upper-case hex)

namespace comum {

using TCamposQRCode = std::vector<std::pair<std::string, std::string>>;   // 24-byte elements

// wasm func 5636 - observed executing                                                  // name inferred
// CEstadoGeral (eg.bin, ModuloEstadoGeralUrna::EstadoGeralUrna) offsets used:
//   +4 idPE | +8 UF | +20 município | +24 zona | +26 seção |
//   +32 CDadoCarga { +32 turno, +36 tipo de urna no 1º turno, +40 no 2º turno, +44 modelo, +48 fase } |
//   +60 CDadoCorrespondencia { +60 número interno da urna, +64 número de série da flash de carga (8 hex),
//        +76 data da carga, +84 hora da carga, +88 código da carga, +108 assinatura,
//        +120/+132/+144 gerador de mídia: nome, serial do certificado TPM, serial da instalação }
// Each pair is built as {std::string(tag), value} and moved in with emplace_back (wasm 625).
TCamposQRCode MontaCamposQRCodeEstadoUrna(const md::estadoaplicacao::CEstadoGeral& eg)
{
    TCamposQRCode campos;
    const auto& carga = eg.GetDadoCarga();
    const auto& correspondencia = eg.GetCorrespondencia();

    campos.emplace_back("PROC", std::to_string(eg.GetIdPE()));                         // unsigned

    // Tipo de urna of the current turno (EUrnaTipoOperacao: '0' sem tipo, '1' vota, '2' contingência,
    // '3' contingência-vota, '4' contingência-vota-recupera).
    const char tipo = carga.GetTurno() == '1' ? carga.GetTipoUrnaT1() : carga.GetTipoUrnaT2();
    if (tipo != '2' && tipo != '4')                                                    // br_table on tipo - '2'
        campos.emplace_back("TURN", std::to_string(carga.GetTurno() == '1' ? 1 : 2));

    campos.emplace_back("FASE", std::string(1, static_cast<char>(std::toupper(carga.GetFaseChar()))));   // O/S/T
    campos.emplace_back("UNFE", eg.GetUF());                                          // copied as stored
    campos.emplace_back("MUNI", std::to_string(eg.GetMunicipio()));                   // unsigned
    campos.emplace_back("ZONA", std::to_string(eg.GetZona()));                        // int(uint16)
    if (tipo == '1' || tipo == '3')                                                    // (tipo & ~2) == '1'
        campos.emplace_back("SECA", std::to_string(eg.GetSecao()));
    campos.emplace_back("IDUE", std::to_string(correspondencia.GetIdUrna()));        // unsigned
    campos.emplace_back("MDUE", std::to_string(static_cast<int>(carga.GetModelo())));    // e.g. 2015
    campos.emplace_back("IDFL", correspondencia.GetNumeroSerieFC());
    campos.emplace_back("IDCA", correspondencia.GetCodigoCarga());
    campos.emplace_back("DTCA", correspondencia.GetDataHoraCarga().GetDate().Format("YYYYMMDD"));   // wasm 706
    campos.emplace_back("HRCA", correspondencia.GetDataHoraCarga().GetTime().Format("hhmmss"));     // wasm 779
    campos.emplace_back("NOME", correspondencia.GetGerador().GetNome());
    campos.emplace_back("SERT", correspondencia.GetGerador().GetSerialCertificadoTPM());
    campos.emplace_back("SERI", correspondencia.GetGerador().GetSerialInstalacao());
    campos.emplace_back("ASSI", ecourna::api::util::CStringUtils::ParaHex(correspondencia.GetAssinatura()));
    return campos;
}

// wasm func 5783                                                                       // name inferred
// Marks where the QR code was produced. variante 0 -> "ORIG:T" (the "antes do horário" zerésima screen,
// CriaTelaAntesHorarioZeresima), 1 -> "ORIG:I" (the printed ESTADO DA URNA report), any other value (2 =
// CriaTelaConfirmaImpressaoZeresima) -> no ORIG field. The meaning of T / I is inferred (tela / impresso).  ?
void AdicionaOrigemQRCode(TCamposQRCode& campos, const int variante)
{
    switch (variante) {
    case 0: campos.emplace_back("ORIG", "T"); break;
    case 1: campos.emplace_back("ORIG", "I"); break;
    default: break;
    }
}

}  // namespace comum

// uenux2/src/app/comum/asn/util.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// comum::asn::Utils: static conversions between the application enums / value classes (comum, comum::md) and the
// ASN.1 types of ModuloTiposEleitorais, ModuloTiposCadastro and ModuloTiposPacotes. Every method is static (the
// srcloc signatures say "static ..."), so Utils is used like a namespace. Line numbers are the throw lines.
#pragma once

#include "api/util/cdate.h"
#include "api/util/cdatetime.h"
#include "comum/comumdefs.h"                         // EUrnaFase ('1' oficial, '2' simulado, '3' treinamento),
                                                     // ETipoLocalVotacao, ESistemaJE, EUrnaTurno
#include "comum/md/cabrangencia.h"                   // md::ETipoAbrangencia
#include "comum/md/ccabecalhopacote.h"               // md::ETipoPacote
#include "comum/md/cideleitoral.h"
#include "comum/md/cidpacote.h"
#include "comum/dados/md/eleitor/csexo.h"            // md::CSexo::ESexo
#include "ModuloTiposCadastro.h"
#include "ModuloTiposEleitorais.h"
#include "ModuloTiposPacotes.h"

namespace comum::asn {

class Utils
{
public:
    // ---- date / time ("DataHoraJE" = "YYYYMMDDTHHMMSS", "DataJE" = "YYYYMMDD", "HoraJE" = "HHMMSS") --------------
    static ModuloTiposEleitorais::DataHoraJE ConverteDataHoraJE(const api::CDateTime& dataHora);        // wasm 1080 (other unit)   // name inferred
    static api::CDateTime DesconverteDataHoraJE(const ModuloTiposEleitorais::DataHoraJE& dataHora);    // wasm 1713  :41
    static api::CDate DesconverteDataJE(const ModuloTiposEleitorais::DataJE& data);                    // wasm 2276 (util.u13.cpp)

    // ---- ids / phase -----------------------------------------------------------------------------------------
    static md::CIDEleitoral DesconverteIdEleitoral(const ModuloTiposEleitorais::IDEleitoral& id);       // :89  inlined in 11437 (other unit)
    static ModuloTiposEleitorais::Fase ConverteFase(const EUrnaFase& fase);                            // wasm 1712  :102
    static EUrnaFase DesconverteFase(const ModuloTiposEleitorais::Fase& fase);                         // wasm 2843  :116

    // ---- packages (CabecalhoPacote) ---------------------------------------------------------------------------
    static ModuloTiposPacotes::TipoPacote ConverteTipoPacote(const md::ETipoPacote& tipo);              // :231 inlined in 11438
    static md::ETipoPacote DesconverteTipoPacote(const ModuloTiposPacotes::TipoPacote& tipo);           // :329 inlined in 11437
    static ModuloTiposEleitorais::IDPacote ConverteIdPacote(const md::CIDPacote& id);                   // :344 inlined in 11438
    static ModuloTiposEleitorais::Sistema ConverteIdSistema(const ESistemaJE& sistema);                 // :419 inlined in 11438
    static ESistemaJE DesconverteIdSistema(const ModuloTiposEleitorais::Sistema& sistema);              // :451 inlined in 11437

    // ---- misc enums -------------------------------------------------------------------------------------------
    static md::CSexo::ESexo DesconverteSexo(ModuloTiposEleitorais::CodigoSexo::NamedNumber sexo);       // :483 inlined in 11450
    static md::ETipoAbrangencia DesconverteAbrangencia(ModuloTiposEleitorais::TipoAbrangencia::NamedNumber tipo); // wasm 5827 :498
    static ModuloTiposEleitorais::TipoAbrangencia::NamedNumber ConverteAbrangencia(md::ETipoAbrangencia tipo);    // :513 inlined in 11462
    static ModuloTiposCadastro::TipoLocalVotacao ConverteTipoLocalVotacao(ETipoLocalVotacao tipo);      // wasm 3801  :530
    static ETipoLocalVotacao DesconverteTipoLocalVotacao(ModuloTiposCadastro::TipoLocalVotacao tipo);   // wasm 3800  :549
    static ModuloTiposEleitorais::Turno ConverteTurno(EUrnaTurno turno);                                // :566 inlined in 11401
    static EUrnaTurno DesconverteTurno(ModuloTiposEleitorais::Turno turno);                             // :581 inlined in 11402
};

} // namespace comum::asn

// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/comum/ctelasvota.h
// (path inferred from ctelasvota.cpp, attested by 33 std::source_location records, lines 294..3618).
//
// CTelasVota ("telas de votação" = voting screens) is the factory and cache of every screen shown on the
// voter terminal. The constructor (ctelasvota.cpp:3502, inlined into the start-up function wasm func 7787,
// unit u02) builds all screens once, per cargo (CTelasCargo, one CFormInterativoTelaVota per ETelaVotacao)
// and the fixed ones (FIM, gravando, zerésima questions...). States keep shared_ptr copies of the
// screens (e.g. CReinicioVotacao copies m_telaReinicioVotacao in its constructor).
//
// Screens are built with api::CFormBuilder (a vector of shared_ptr<IFormField>) and turned into
// api::CInteractiveForm<IScreen, IInputKbd> (func 554) with a vota::CPreShowFormVota /
// vota::CPreShowProgressBar "pre-show" hook. Coordinates are in the logical 640x480 screen of the urna
// (simulador::CWasmScreen scales them to the canvas). api::TPosition is a 16-bit coordinate,
// api::SPoint = {x, y}, api::SRect = {left, top, right, bottom}, api::SFont = {size, style}.
//
// Not polymorphic (no RTTI). 252 bytes (operator new(252) in func 7787), singleton pointer @1833396
// with mutex @1833372 ("CTelasVota - instancia nao criada" / "... ja criada").
#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <string>

#include "api/gui/cinteractiveform.h"     // api::CInteractiveForm<api::IScreen, api::IInputKbd>
#include "api/gui/cformbuilder.h"         // api::CFormBuilder, api::SPoint, api::SRect, api::SFont, api::TPosition
#include "api/gui/cprogressbar.h"         // api::CProgressBar
#include "vota/eleitor/comum/ctelascargo.h"   // vota::CTelasCargo, vota::ETelaVotacao (unit u06)

namespace comum::md { class CCargo; class CCandidatura; }

namespace vota {

using CFormInterativoTelaVota = std::shared_ptr<api::CInteractiveForm<api::IScreen, api::IInputKbd>>;  // attested name

/// Error codes of this file: ecourna::api::exception::CBaseError<vota::EUeVotaError, SErrorLimits{9300, 9500}>
/// (typeinfo @1532388; thrown through the thunk wasm func 253). Enumerator names inferred.
enum EUeVotaErroTelas : int {
    ERRO_DS_CANDIDATO_NAO_POSICIONADO           = 9333,   // DS_CandidatoNaoConcorre (:294)
    ERRO_TELA_COMPLETA_COM0_NAO_CANDIDATO       = 9334,   // :822
    ERRO_TELA_COMPLETA_COM0_SUPLENTES           = 9335,   // :827
    ERRO_TELA_COMPLETA_COM1_NAO_CANDIDATO       = 9336,   // :853
    ERRO_TELA_COMPLETA_COM1_SUPLENTES           = 9337,   // :858
    ERRO_TELA_COMPLETA_COM2_NAO_CANDIDATO       = 9338,   // :890
    ERRO_TELA_COMPLETA_COM2_SUPLENTES           = 9339,   // :895
    ERRO_TELA_COMPLETA_CONSULTA                 = 9340,   // :942
    ERRO_TELA_BRANCO_CANDIDATO                  = 9341,   // :964
    ERRO_TELA_BRANCO_CONSULTA                   = 9342,   // :993
    // 9343 is never used in the binary: adicionaBaseTelaVotoNuloConsulta (:1014) throws 9344 as well.
    ERRO_TELA_NULO_CANDIDATO                    = 9344,   // :1049 (and :1014, see above)
    ERRO_TELA_CANDIDATO_INEXISTENTE             = 9345,   // :1075
    ERRO_TELA_CANDIDATO_INAPTO                  = 9346,   // :1101
    ERRO_TELA_VOTO_LEGENDA                      = 9347,   // :1126
    ERRO_TELA_INPUT_VAZIO_PROPORCIONAL          = 9348,   // :1392 (func 7787)
    ERRO_TELA_INPUT_VAZIO_PROPORCIONAL_2        = 9349,   // :1397 (func 7787)
    ERRO_TELA_INPUT_VAZIO_MAJORITARIO_COM0      = 9350,   // :1413 (func 7787)
    ERRO_TELA_INPUT_VAZIO_MAJORITARIO_COM1      = 9351,   // :1428 (func 7787)
    ERRO_TELA_INPUT_VAZIO_MAJORITARIO_COM2      = 9352,   // :1444 (func 7787)
    ERRO_TELA_INPUT_VAZIO_CONSULTA              = 9353,   // :1460 (func 7787)
    ERRO_TELA_CARGO_SEM_CANDIDATO               = 9354,   // :1585
    ERRO_TELA_PARTIDO                           = 9355,   // :1741 (func 7787)
    ERRO_TELA_VISUALIZACAO_SUPLENTES            = 9356,   // :3366
    ERRO_TIPO_CARGO_NAO_IDENTIFICADO            = 9357,   // :3502 (func 7787) "Tipo de cargo nao identificado"
};

class CTelasVota {
public:
    static CTelasVota& GetInst();                                                 // :3434  wasm func 407
    static void CreateInst();                                                     // :3435  inlined in func 7787

    /// wasm func 2369 (name inferred). One step of the "Gravando" progress bar (m_barraProgresso, 4 steps).
    /// Called 4x by CSincronismoVotoEleitor::SincronizaVoto and, in the web build, 4x by votaTick.
    void AvancaBarraProgresso();

    CFormInterativoTelaVota GetTelaCargo(comum::TCargoID cargo, ETelaVotacao tela) const;   // :3534, inlined in func 4135

    // Static screen factories (all static, all return CFormInterativoTelaVota)
    static CFormInterativoTelaVota CriaTelaInputVazioProporcional(const comum::md::CCargo&);    // :1392 (func 7787)
    static CFormInterativoTelaVota CriaTelaInputVazioMajoritarioCom0(const comum::md::CCargo&); // :1413 (func 7787)
    static CFormInterativoTelaVota CriaTelaInputVazioMajoritarioCom1(const comum::md::CCargo&); // :1428 (func 7787)
    static CFormInterativoTelaVota CriaTelaInputVazioMajoritarioCom2(const comum::md::CCargo&); // :1444 (func 7787)
    static CFormInterativoTelaVota CriaTelaInputVazioConsulta(const comum::md::CCargo&);        // :1460 (func 7787)
    static CFormInterativoTelaVota CriaTelaVotoCargoSemCandidato(const comum::md::CCargo&,
                                                                 api::TPosition);               // :1585  wasm func 3064
    static CFormInterativoTelaVota CriaTelaPartido(const comum::md::CCargo&);                   // :1741 (func 7787)
    static CFormInterativoTelaVota CriaTelaZeresimaTardia();                                    // :2382 (func 7787)
    CFormInterativoTelaVota CriaTelaVisualizacaoCandidato(const comum::md::CCandidatura&,
                                                          size_t, size_t);                      // :3323..3366 (func 6569, unit u09)

private:
    CTelasVota();                                                                 // :3502 (inlined in func 7787)

    // ---- layout (252 bytes). Offsets from the constructor in func 7787 and from the readers. ------------
    std::map<comum::TCargoID, CTelasCargo> m_telasCargo;       // +0   (begin +0, root +4, size +8)
    // +12, +16: zero-initialised                                        ?
    // +20 (func 6595), +28 (func 6599): 8-byte members                  ?
    // +36, +44, +60, +68, +92, +100, +108, +196: CFormInterativoTelaVota built with func 576
    // +52, +84: built with func 6592;  +76, +204: built with func 886;  +132, +140: func 2380;
    // +156, +164: func 6601 (+164 = "FIM / NÃO VOTOU", see celeitorvotando.cpp)
public:
    CFormInterativoTelaVota m_telaConfirmaRegerarZeresima;     // +84   used by CConfirmaRegerarZeresima
    CFormInterativoTelaVota m_telaInstrucoesAcessibilidade;    // +124  (unit u06)
    CFormInterativoTelaVota m_telaReinicioVotacao;             // +148  used by CReinicioVotacao (func 5938)
    api::SharedForm m_telaProgressoRegistroVoto;               // +172  "telaProgressoRegistroVoto": "Gravando" + bar
    std::shared_ptr<api::CProgressBar> m_barraProgresso;       // +180  CProgressBar(max 4, {70,225}-{570,255})
    // +188: ?
    api::SharedForm m_telaGeraZeresima;                        // +212  used by CGeraZeresimaBase (func 5965)
    api::SharedForm m_telaGeraResumoZeresima;                  // +220  used by CGeraResumoZeresimaBase (func 11945)
    CFormInterativoTelaVota m_telaQuerImprimirZeresima;        // +228  (func 2376) used by CQuerImprimirZeresima (func 5957)
    // +236: func 2376 question screen                                   ?
    CFormInterativoTelaVota m_telaQuerReimprimirZeresima;      // +244  used by CQuerReimprimirZeresima (func 5940)
};

} // namespace vota

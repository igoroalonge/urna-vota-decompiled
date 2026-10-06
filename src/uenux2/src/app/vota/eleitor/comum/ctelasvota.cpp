// Reconstructed from vota_web_wasm.wasm (unit u07). Original: uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp
// (attested by std::source_location records; line numbers below are the original ones).
//
// Only the out-of-line functions of this file are here. The constructor (:3502), CreateInst (:3435),
// CriaTelaInputVazio* (:1392..1460), CriaTelaPartido (:1741), CriaTelaZeresimaTardia (:2382) and
// CInfoEleitorAudioHabilitado::LarguraMaxima (:3618) are inlined into the start-up function wasm func 7787
// (unit u02); CriaTelaVisualizacaoCandidato's body is wasm func 6569 (unit u09); GetTelaCargo (:3534) is
// inlined into CTelasCargo::GetTela (wasm func 4135, unit u06).
//
// Screen vocabulary (Portuguese): cargo = office being voted (Prefeito, Vereador...), candidato/candidata,
// suplente/vice = running mates (Senate suplentes, vice-prefeito...), legenda = party-only vote (proportional
// offices), voto nulo / voto em branco = null / blank vote, consulta = referendum-style question (the "cargo"
// is a question and the "candidates" are answers), "tecla CONFIRMA" = the green key.
//
// Constants seen in the code (names inferred):
//   fonts (static api::SFont objects): @474880 {40,0}  @474896 {20,0}  @474992 {25,0}  @475016 {35,0}
//                                      @475328 {30,1 bold}  @476600 {13,0}
//   ETextAlignment: 0 left, 1 right, 2 center (CFixedText/CDataText first member)
//   colours: api::CTextField(..., fg = 2, bg = 1); CTextFieldBlinking uses 2/3/1 and a 500 ms timer
//   EAnchorPoint 1 = the given point is the TOP-RIGHT corner of the image (CWasmImageSurfaceOps::CalcRect)
#include "vota/eleitor/comum/ctelasvota.h"

#include <algorithm>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "api/gui/cdataimage.h"            // api::CDataImage<SRC>
#include "api/gui/cdatatext.h"             // api::CDataText<SRC>, api::CDataTextFmt<SRC>
#include "api/gui/cfixedtext.h"            // api::CFixedText
#include "api/gui/cframedtext.h"           // api::CFramedText, api::CGrayedFramedText
#include "api/gui/cimagefield.h"           // api::CImageField
#include "api/gui/cmaskedtextfield.h"      // api::CMaskedTextField<FRAME>
#include "api/gui/crectfield.h"            // api::CRectField
#include "api/gui/ctextfield.h"            // api::CTextField
#include "api/gui/ctextfieldblinking.h"    // api::CTextFieldBlinking
#include "api/gui/ctextfielddoubleline.h"  // api::CTextFieldDoubleLine
#include "api/gui/ctextfieldmultiline.h"   // api::CTextFieldMultiLine
#include "api/gui/iscreen.h"               // api::IScreen, api::IImageSurfaceOps
#include "vota/eleitor/comum/cpreshowformvota.h"   // vota::CPreShowFormVota, vota::CPreShowProgressBar
#include "comum/cconfiguracaoeleicao.h"
#include "comum/dados/ccandidaturas.h"     // comum::CCandidaturas + data sources CCandidaturasDS*
#include "comum/dados/ccargods.h"          // comum::CCargoDSNomeSexoCandidato
#include "comum/dados/crespostas.h"        // comum::CRespostasDSNumero
#include "comum/dados/md/processoeleitoral/ccargo.h"
#include "vota/comum/votadefs.h"       // vota::CUeVotaError (= CBaseError<EUeVotaError, {9300, 9500}>)

namespace vota {

using api::SPoint;
using api::SRect;
using api::SFont;
using api::TPosition;

namespace {

const SFont FONTE_40{40, 0};           // @474880
const SFont FONTE_20{20, 0};           // @474896
const SFont FONTE_25{25, 0};           // @474992
const SFont FONTE_35{35, 0};           // @475016
const SFont FONTE_30_NEGRITO{30, 1};   // @475328
const SFont FONTE_13{13, 0};           // @476600

constexpr api::ETextAlignment ESQUERDA = api::ETextAlignment(0);
constexpr api::ETextAlignment DIREITA  = api::ETextAlignment(1);
constexpr api::ETextAlignment CENTRO   = api::ETextAlignment(2);
constexpr api::EAnchorPoint   CANTO_SUPERIOR_DIREITO = api::EAnchorPoint(1);

// Data sources passed as plain function pointers (function-table slots; defined in other files):
std::string DS_NomeCargoGeneroCandidato();     // slot 1088 -> func 12811 (comum): CCargoDSNomeSexoCandidato{0,0} of the current cargo
std::string DS_Partido(const std::string& fmt);// slot 1089 -> func 11501 (comum): std::vformat(fmt, current party text)
std::string DS_NomeSuplente1();                // slots 1090/1092 -> func 6669 (comum_f6076(.., 257, 1))   ?
std::string DS_CargoSuplente1();               // slots 1091/1095 -> func 6664 (comum_f6075(.., 257))       ?
std::string DS_NomeSuplente2();                // slot 1093 -> func 13301 (comum_f6076(.., 258, 2))        ?
std::string DS_CargoSuplente2();               // slot 1094 -> func 13298 (comum_f6075(.., 258))           ?
std::string DS_RespostaAtual();                // slot 1096 -> func 11479 (comum::CRespostas current answer text)
const std::string& VotoDigitado();             // slot 1098 -> func 13176: returns g_votoDigitado (@1833288)

// ---------------------------------------------------------------------------------------------------
// wasm func 690                                                                     // name inferred
// Message of the "wrong kind of cargo" errors: "<tela> - cargo <codigo> (<nome>) - <motivo>".
std::string MensagemErroCargo(const std::string& nomeTela, const comum::md::CCargo& cargo, const std::string& motivo)
{
    return nomeTela + " - cargo " + std::to_string(cargo.GetCodigo()) + " (" + cargo.GetNomeCargo() + ") - " + motivo;
    //                                        func 296                     func 3716 (candidate: +36, consulta: +88)
}

// ---------------------------------------------------------------------------------------------------
// wasm func 1192                                                                    // name inferred
// Top line of every voting screen: the cargo name ("Prefeito", "Senador - 1ª vaga"...), from a data
// source that keeps a copy of the cargo (DS_NomeCargoNeutroComEscolha, built by func 6627; its operator()
// is func 12587: GetNomeNeutro() + (qtdEscolhas > 1 ? " - " + GetOrdinalEscolha(g_numeroEscolha) : "")).
void adicionaNomeCargo(const comum::md::CCargo& cargo, api::CFormBuilder& form)
{
    auto texto = std::make_shared<api::CDataText<DS_NomeCargoNeutroComEscolha>>(DS_NomeCargoNeutroComEscolha{cargo});
    if (cargo.PossuiFoto()) {                                                                // func 1546
        form.Add<api::CTextFieldDoubleLine>(SPoint{10, 60}, SPoint{10, 95}, TPosition{639}, texto,
                                            FONTE_30_NEGRITO, api::TColor{1});               // ? one TColor constant-folded
    } else {
        form.Add<api::CTextField>(SPoint{10, 60}, texto, FONTE_30_NEGRITO, api::TColor{2}, api::TColor{1});
    }
}

// ---------------------------------------------------------------------------------------------------
// wasm func 3060 (instantiation of CFormBuilder::Add<CMaskedTextField<CGrayedFramedText>>)  // name inferred
// The grey digit boxes that echo what the voter typed.
std::shared_ptr<api::CMaskedTextField<api::CGrayedFramedText>>
adicionaNumeroDigitado(api::CFormBuilder& form, uebyte digitos, const SPoint& pos)
{
    return form.Add<api::CMaskedTextField<api::CGrayedFramedText>>(
        api::CGrayedFramedText(digitos, pos),                                                 // func 5535
        std::make_shared<api::CDataText<const std::string& (*)()>>(ESQUERDA, &VotoDigitado));
}

// ---------------------------------------------------------------------------------------------------
// wasm func 2028                                                                    // name inferred
// Party line ("{}" formatted with the party text of the current candidate), only when the election
// configuration says so (CConfiguracaoEleicao +484; e.g. elections without parties hide it).
void adicionaPartido(api::CFormBuilder& form)
{
    if (!comum::CConfiguracaoEleicao::GetInst().ExibePartido())                              // +484 == 1  ?
        return;
    form.Add<api::CTextField>(SPoint{10, 250},
                              std::make_shared<api::CDataTextFmt<std::string (*)(const std::string&)>>(ESQUERDA, &DS_Partido, "{}"),
                              FONTE_25, api::TColor{2}, api::TColor{1});
}

// ---------------------------------------------------------------------------------------------------
// wasm func 3074                                                                    // name inferred
// Question of a consulta ("|" in the text means a line break), centred in {10,50}-{629,255}.
// Returns the y coordinate where the answer area starts (bottom of the text + 23).
TPosition adicionaPerguntaConsulta(api::CFormBuilder& form, const comum::md::CCargo& cargo)
{
    std::string pergunta = cargo.GetDetalheConsulta().GetPergunta();                        // func 1923, +12
    std::replace(pergunta.begin(), pergunta.end(), '|', '\n');
    auto campo = form.Add<api::CTextFieldMultiLine>(SRect{10, 50, 629, 255},
                                                    std::make_shared<api::CFixedText>(CENTRO, pergunta), FONTE_35);   // func 2782
    const SRect r = campo->Rect();
    return static_cast<TPosition>(r.bottom + 23);
}

// ---------------------------------------------------------------------------------------------------
// Framed candidate photo. Template; its only out-of-line instance is wasm func 6561
// (SRCFT = std::__bind<(lambda at ctelasvota.cpp:3323:29)&, const comum::md::CDadosCandidato&>).
// The SRCFT = comum::CCandidaturasDSFoto instance is inlined into funcs 4181 and 4184 (constant-specialised).
// Places the image with its top-right corner at (x+1, y+1) and returns the w x h rectangle it occupies.
template <typename SRCFT>
SRect adicionaFotoEmoldurada(api::CFormBuilder& form, const TPosition x, const TPosition y,
                             const TPosition largura, const TPosition altura, SRCFT fonte)
{
    const SPoint pos{static_cast<TPosition>(x + 1), static_cast<TPosition>(y + 1)};
    form.Add<api::CImageField>(pos, std::make_shared<api::CDataImage<SRCFT>>(std::move(fonte)),
                               CANTO_SUPERIOR_DIREITO);                                         // func 4129 for CCandidaturasDSFoto
    return api::IScreen::GetInst()                                                               // ctelasvota.cpp:487
        .GetImageSurfaceOps()                                                                    // IScreen slot 34 (CWasmScreen +44)
        .CalcRect(largura, altura, pos, CANTO_SUPERIOR_DIREITO);                                 // IImageSurfaceOps slot 14
}

// ---------------------------------------------------------------------------------------------------
// wasm func 4184 (contains adicionaFotoEmoldurada<CCandidaturasDSFoto> inlined; srcloc :487) // name inferred
// Big photo of the candidate (161 x 225, top-right corner at (640,36)); optional caption with the cargo name
// in the candidate's gender ("Prefeita", abbreviated when >= 19 characters).
void adicionaFotoCandidato(api::CFormBuilder& form, bool comLegenda)
{
    const SRect r = adicionaFotoEmoldurada(form, 639, 35, 161, 225, comum::CCandidaturasDSFoto{0});
    if (!comLegenda)
        return;

    SPoint pos;
    const TPosition largura = static_cast<TPosition>(r.right - r.left + 1);
    if (largura <= 1)
        pos = SPoint{719, 261};              // ? fallback = x + w/2, y + h + 1 - but the photo is anchored on its
                                             //   RIGHT edge, so this is 160 px right of the photo (off screen).
                                             //   Unreachable: CalcRect always returns a 161-wide rectangle.
    else
        pos = SPoint{static_cast<TPosition>(r.left + largura / 2), static_cast<TPosition>(r.bottom + 1)};

    form.Add<api::CTextField>(pos,                                                           // func 6528
                              std::make_shared<api::CDataText<comum::CCargoDSNomeSexoCandidato>>(
                                  CENTRO, comum::CCargoDSNomeSexoCandidato{/*indice*/ 0, /*abrevia*/ true}),
                              FONTE_20, api::TColor{2}, api::TColor{1});
}

// ---------------------------------------------------------------------------------------------------
// wasm func 4181 (contains adicionaFotoEmoldurada<CCandidaturasDSFoto> inlined; srcloc :487) // name inferred
// Small photo (111 x 155, top-right corner at (x+1, 288)) of running mate `indice` (1 or 2) with a caption
// (`legenda` = data source of the running mate's cargo title). Returns the photo rectangle.
SRect adicionaFotoSuplente(api::CFormBuilder& form, std::string (*legenda)(), TPosition x, uebyte indice)
{
    const SRect r = adicionaFotoEmoldurada(form, x, 287, 111, 155, comum::CCandidaturasDSFoto{indice});

    SPoint pos;
    const TPosition largura = static_cast<TPosition>(r.right - r.left + 1);
    if (largura <= 1)
        pos = SPoint{static_cast<TPosition>(x + 55), 443};   // ? same off-by-sign fallback as above (unreachable)
    else
        pos = SPoint{static_cast<TPosition>(r.left + largura / 2), static_cast<TPosition>(r.bottom + 1)};

    form.Add<api::CTextField>(pos, std::make_shared<api::CDataText<std::string (*)()>>(CENTRO, legenda),   // func 1191
                              FONTE_13, api::TColor{2}, api::TColor{1});
    return r;
}

// ---------------------------------------------------------------------------------------------------
// wasm func 4185                                                                    // name inferred
// Header of the "tela completa" (candidate found): cargo name in the candidate's gender, the typed
// number in grey boxes and the candidate's name (two lines, right limit `xLimiteNome`: Com1/Com2 pass 470 when
// the titular's photo is shown and 639 otherwise; Com0 always passes 639).
void adicionaDadosCandidato(api::CFormBuilder& form, const comum::md::CCargo& cargo, TPosition xLimiteNome)
{
    if (cargo.PossuiFoto()) {
        form.Add<api::CTextFieldDoubleLine>(SPoint{10, 60}, SPoint{10, 95},                    // func 3068
                                            static_cast<TPosition>(xLimiteNome + 237),         // (sic) maxX 707 (Com1/Com2) or
                                                                                               // 876 (Com0): past the screen edge
                                            std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_NomeCargoGeneroCandidato),
                                            FONTE_30_NEGRITO, api::TColor{1});
    } else {
        form.Add<api::CTextField>(SPoint{10, 60},                                              // func 1191
                                  std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_NomeCargoGeneroCandidato),
                                  FONTE_30_NEGRITO, api::TColor{2}, api::TColor{1});
    }

    form.Add<api::CMaskedTextField<api::CGrayedFramedText>>(
        api::CGrayedFramedText(cargo.GetNumeroDigitos(), SPoint{10, 115}),                   // CCargo +12
        std::make_shared<api::CDataText<comum::CCandidaturasDSNumero>>(ESQUERDA, comum::CCandidaturasDSNumero{}));   // empty functor

    form.Add<api::CTextFieldDoubleLine>(SPoint{10, 190}, SPoint{10, 215}, xLimiteNome,
                                        std::make_shared<api::CDataText<comum::CCandidaturasDSNome>>(ESQUERDA, comum::CCandidaturasDSNome{0}),
                                        FONTE_25, api::TColor{0});
}

} // namespace

// =====================================================================================================
// Data source used by the "candidato inapto" screen.
// ctelasvota.cpp:294 - wasm func 13101 (function-table slot 1099)
namespace {
std::string DS_CandidatoNaoConcorre()
{
    auto& candidaturas = comum::CCandidaturas::GetInst();                                     // func 521 ?
    if (!candidaturas.Posicionado())                                                          // current != end
        throw CUeVotaError(ERRO_DS_CANDIDATO_NAO_POSICIONADO,
                           "Nao foi posicionado no candidato corretamente");                  // :294
    const auto sexo = comum::CCandidaturasDSSexo{0}();                                        // 1 masc., 2 fem.
    return sexo == 2 ? "CANDIDATA NÃO CONCORRE" : "CANDIDATO NÃO CONCORRE";
}
} // namespace

// =====================================================================================================
// "Base" builders of the screens of each ETelaVotacao. All are called from the constructor (func 7787)
// except where noted; `nomeTela` is only used in error messages.
namespace {

// ctelasvota.cpp:822/827 - wasm func 6678. Candidate cargo without running mates (0 suplentes), e.g.
// Vereador or Deputado. Called from funcs 6639, 6681 (screen factories).
void adicionaBaseTelaCompletaCandidatoCom0(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheCandidato())                                                         // CCargo +84
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM0_NAO_CANDIDATO, MensagemErroCargo(nomeTela, cargo, "não é de candidato"));
    if (cargo.GetQtdSuplentes() != 0)                                                         // func 1157
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM0_SUPLENTES, MensagemErroCargo(nomeTela, cargo, "com número de suplentes incompatível"));

    adicionaDadosCandidato(form, cargo, 639);
    if (cargo.GetCodigo() != 25)                                                              // ? cargo 25 shows no party
        adicionaPartido(form);
    if (cargo.PossuiFoto())
        adicionaFotoCandidato(form, false);
}

// ctelasvota.cpp:853/858 - wasm func 6671. One running mate (vice).
void adicionaBaseTelaCompletaCandidatoCom1(api::CFormBuilder& form, const std::string& nomeTela,
                                           const comum::md::CCargo& cargo, TPosition& xLimite)
{
    if (!cargo.TemDetalheCandidato())
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM1_NAO_CANDIDATO, MensagemErroCargo(nomeTela, cargo, "não é de candidato"));
    if (cargo.GetQtdSuplentes() != 1)
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM1_SUPLENTES, MensagemErroCargo(nomeTela, cargo, "com número de suplentes incompatível"));

    const auto& detalhe = cargo.GetDetalheCandidato();                                        // func 1388 (ccargo.cpp:81)
    xLimite = detalhe.GetSuplente(1).PossuiFoto() ? 520 : 639;                                // CDetalheSuplente +1
    adicionaDadosCandidato(form, cargo, cargo.PossuiFoto() ? 470 : 639);
    adicionaPartido(form);
    form.Add<api::CTextFieldDoubleLine>(SPoint{10, 320}, SPoint{10, 340}, xLimite,           // func 3068
                                        std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_NomeSuplente1),
                                        FONTE_20, api::TColor{0});
    if (cargo.PossuiFoto())
        adicionaFotoCandidato(form, true);
    if (detalhe.GetSuplente(1).PossuiFoto())
        adicionaFotoSuplente(form, &DS_CargoSuplente1, 639, 1);
}

// ctelasvota.cpp:890/895 - wasm func 6660. Two running mates (1º and 2º suplentes of Senador).
void adicionaBaseTelaCompletaCandidatoCom2(api::CFormBuilder& form, const std::string& nomeTela,
                                           const comum::md::CCargo& cargo, TPosition& xLimite)
{
    if (!cargo.TemDetalheCandidato())
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM2_NAO_CANDIDATO, MensagemErroCargo(nomeTela, cargo, "não é de candidato"));
    if (cargo.GetQtdSuplentes() <= 1)                                                         // (3 or more pass this test)
        throw CUeVotaError(ERRO_TELA_COMPLETA_COM2_SUPLENTES, MensagemErroCargo(nomeTela, cargo, "com número de suplentes incompatível"));

    const bool fotoTitular = cargo.PossuiFoto();
    const bool fotoSuplente1 = cargo.GetDetalheCandidato().GetSuplente(1).PossuiFoto();
    const bool fotoSuplente2 = cargo.GetDetalheCandidato().GetSuplente(2).PossuiFoto();

    adicionaDadosCandidato(form, cargo, fotoTitular ? 470 : 639);
    adicionaPartido(form);
    const TPosition xTexto = fotoSuplente1 ? 403 : 639;
    form.Add<api::CTextFieldDoubleLine>(SPoint{10, 310}, SPoint{10, 330}, xTexto,
                                        std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_NomeSuplente1),
                                        FONTE_20, api::TColor{0});
    form.Add<api::CTextFieldDoubleLine>(SPoint{10, 355}, SPoint{10, 375}, xTexto,
                                        std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_NomeSuplente2),
                                        FONTE_20, api::TColor{0});
    if (cargo.PossuiFoto())
        adicionaFotoCandidato(form, true);

    TPosition x = 639;
    if (fotoSuplente2)
        x = static_cast<TPosition>(adicionaFotoSuplente(form, &DS_CargoSuplente2, 639, 2).left - 5);
    if (fotoSuplente1)
        x = adicionaFotoSuplente(form, &DS_CargoSuplente1, x, 1).left;
    xLimite = static_cast<TPosition>(x - 5);
}

// ctelasvota.cpp:942 - wasm func 6652. Consulta, answer found.
void adicionaBaseTelaCompletaConsulta(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheConsulta())                                                          // CCargo +136
        throw CUeVotaError(ERRO_TELA_COMPLETA_CONSULTA, MensagemErroCargo(nomeTela, cargo, "não é de consulta"));

    const TPosition y = adicionaPerguntaConsulta(form, cargo);
    const uebyte digitos = cargo.GetNumeroDigitos();
    form.Add<api::CMaskedTextField<api::CFramedText>>(
        api::CFramedText(digitos, SPoint{static_cast<TPosition>(100 - 40 * digitos), y}, FONTE_40, ESQUERDA),
        std::make_shared<api::CDataText<comum::CRespostasDSNumero>>(ESQUERDA, comum::CRespostasDSNumero(digitos)));  // ctor func 5729
    form.Add<api::CTextField>(SPoint{100, static_cast<TPosition>(y + 3)},                     // func 1191
                              std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_RespostaAtual),
                              FONTE_40, api::TColor{2}, api::TColor{1});
}

// ctelasvota.cpp:964 - wasm func 6646 (the TPosition parameter is unused/constant-propagated away).
void adicionaBaseTelaVotoBrancoCandidato(api::CFormBuilder& form, const std::string& nomeTela,
                                         const comum::md::CCargo& cargo, TPosition /*?*/)
{
    if (!cargo.TemDetalheCandidato())
        throw CUeVotaError(ERRO_TELA_BRANCO_CANDIDATO, MensagemErroCargo(nomeTela, cargo, "não é de candidato"));
    adicionaNomeCargo(cargo, form);
    form.Add<api::CTextFieldBlinking>(SPoint{195, 200}, "VOTO EM BRANCO", FONTE_40, CENTRO);  // func 1265
}

// ctelasvota.cpp:993 - wasm func 6641.
void adicionaBaseTelaVotoBrancoConsulta(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheConsulta())
        throw CUeVotaError(ERRO_TELA_BRANCO_CONSULTA, MensagemErroCargo(nomeTela, cargo, "não é de consulta"));
    const TPosition y = adicionaPerguntaConsulta(form, cargo);
    form.Add<api::CTextFieldBlinking>(SPoint{320, y}, "VOTO EM BRANCO", FONTE_40, CENTRO);
}

// ctelasvota.cpp:1014 - wasm func 6624. Called from funcs 6616, 6626.
void adicionaBaseTelaVotoNuloConsulta(api::CFormBuilder& form, const std::string& nomeTela,
                                      const comum::md::CCargo& cargo, const std::string& texto)
{
    if (!cargo.TemDetalheConsulta())
        throw CUeVotaError(ERRO_TELA_NULO_CANDIDATO /* 9344, sic: the enum used by :1049 */,
                           MensagemErroCargo(nomeTela, cargo, "não é de consulta"));
    const TPosition y = adicionaPerguntaConsulta(form, cargo);
    const uebyte digitos = cargo.GetNumeroDigitos();
    form.Add<api::CMaskedTextField<api::CFramedText>>(
        api::CFramedText(digitos, SPoint{static_cast<TPosition>(100 - 40 * digitos), y}, FONTE_40, ESQUERDA),
        std::make_shared<api::CDataText<const std::string& (*)()>>(ESQUERDA, &VotoDigitado));
    form.Add<api::CTextField>(SPoint{100, static_cast<TPosition>(y + 3)},                     // func 202
                              std::make_shared<api::CFixedText>(ESQUERDA, texto), FONTE_40, api::TColor{2}, api::TColor{1});
    form.Add<api::CTextFieldBlinking>(SPoint{320, 345}, "VOTO NULO", FONTE_40, CENTRO);
}

// ctelasvota.cpp:1049 - wasm func 6608. Called from funcs 1591, 1767. The three TPosition parameters and the
// bool of the original signature were constant-propagated away (both callers pass the same values).
// `digitosOcultos` is subtracted from the number of boxes; `xFimNumero` receives the right edge of the
// boxes + 5.
void adicionaBaseTelaVotoNuloCandidato(api::CFormBuilder& form, const std::string& nomeTela,
                                       const comum::md::CCargo& cargo, const std::string& texto,
                                       uebyte digitosOcultos, TPosition, TPosition, TPosition,
                                       TPosition& xFimNumero, const bool)
{
    if (!cargo.TemDetalheCandidato())
        throw CUeVotaError(ERRO_TELA_NULO_CANDIDATO, MensagemErroCargo(nomeTela, cargo, "não é de candidato"));
    adicionaNomeCargo(cargo, form);
    auto numero = adicionaNumeroDigitado(form, static_cast<uebyte>(cargo.GetNumeroDigitos() - digitosOcultos), SPoint{10, 115});
    xFimNumero = static_cast<TPosition>(numero->Rect().right + 5);                            // IFormField slot 8
    form.Add<api::CTextField>(SPoint{10, 190}, std::make_shared<api::CFixedText>(ESQUERDA, texto),
                              FONTE_25, api::TColor{2}, api::TColor{1});
    form.Add<api::CTextFieldBlinking>(SPoint{195, 345}, "VOTO NULO", FONTE_40, CENTRO);
}

// ctelasvota.cpp:1075 - wasm func 6605. Proportional office: the number does not exist but the party
// prefix does, so the vote goes to the party ("voto de legenda").
void adicionaBaseTelaCandidatoInexistente(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheCandidato() || cargo.GetTipo() != comum::md::CCargo::PROPORCIONAL)   // CCargo +4 == 1
        throw CUeVotaError(ERRO_TELA_CANDIDATO_INEXISTENTE, MensagemErroCargo(nomeTela, cargo, "não é de proporcional"));
    adicionaNomeCargo(cargo, form);
    adicionaNumeroDigitado(form, cargo.GetNumeroDigitos(), SPoint{10, 115});
    form.Add<api::CTextField>(SPoint{10, 190}, std::make_shared<api::CFixedText>(ESQUERDA, "CANDIDATO INEXISTENTE"),
                              FONTE_25, api::TColor{2}, api::TColor{1});
    adicionaPartido(form);
    form.Add<api::CTextFieldBlinking>(SPoint{195, 345}, "VOTO DE LEGENDA", FONTE_40, CENTRO);
}

// ctelasvota.cpp:1101 - wasm func 6603. Proportional candidate who does not run ("inapto"): null vote.
void adicionaBaseTelaCandidatoInapto(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheCandidato() || cargo.GetTipo() != comum::md::CCargo::PROPORCIONAL)
        throw CUeVotaError(ERRO_TELA_CANDIDATO_INAPTO, MensagemErroCargo(nomeTela, cargo, "não é de proporcional"));
    adicionaNomeCargo(cargo, form);
    adicionaNumeroDigitado(form, cargo.GetNumeroDigitos(), SPoint{10, 115});
    form.Add<api::CTextField>(SPoint{10, 190},                                                  // func 1191
                              std::make_shared<api::CDataText<std::string (*)()>>(ESQUERDA, &DS_CandidatoNaoConcorre),
                              FONTE_25, api::TColor{2}, api::TColor{1});
    form.Add<api::CTextFieldBlinking>(SPoint{320, 345}, "VOTO NULO", FONTE_40, CENTRO);
}

// ctelasvota.cpp:1126 - wasm func 6602. Proportional office, party-only vote (two digits typed + CONFIRMA).
void adicionaBaseTelaVotoLegenda(api::CFormBuilder& form, const std::string& nomeTela, const comum::md::CCargo& cargo)
{
    if (!cargo.TemDetalheCandidato() || cargo.GetTipo() != comum::md::CCargo::PROPORCIONAL)
        throw CUeVotaError(ERRO_TELA_VOTO_LEGENDA, MensagemErroCargo(nomeTela, cargo, "não é de proporcional"));
    adicionaNomeCargo(cargo, form);
    adicionaNumeroDigitado(form, cargo.GetNumeroDigitos(), SPoint{10, 115});
    adicionaPartido(form);
    form.Add<api::CTextFieldBlinking>(SPoint{195, 345}, "VOTO DE LEGENDA", FONTE_40, CENTRO);
}

} // namespace

// =====================================================================================================
// ctelasvota.cpp:3434 - wasm func 407 (the check-and-return body is the shared helper func 1406)
static std::unique_ptr<CTelasVota> s_pInstancia;       // @1833396 (set by CreateInst, :3435)
static std::mutex s_mutexInstancia;                     // @1833372

CTelasVota& CTelasVota::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutexInstancia);
    if (!s_pInstancia)
        throw ecourna::api::exception::CPatternError(1303, "CTelasVota - instancia nao criada");   // :3434
    return *s_pInstancia;
}

// wasm func 2369                                                                    // name inferred
void CTelasVota::AvancaBarraProgresso()
{
    m_barraProgresso->Incrementa();                     // api::CProgressBar, func 5508 (see cprogressbar.u07.cpp)
}

// =====================================================================================================
// ctelasvota.cpp:1585 - wasm func 3064. Screen of a cargo that has no candidate at all
// ("NÃO HÁ CANDIDATOS CONCORRENDO"); the voter only presses CONFIRMA. Called 3x from func 7787.
// The TPosition parameter of the original signature is not used in this build.
CFormInterativoTelaVota CTelasVota::CriaTelaVotoCargoSemCandidato(const comum::md::CCargo& cargo, TPosition)
{
    if (!cargo.TemDetalheCandidato())
        throw CUeVotaError(ERRO_TELA_CARGO_SEM_CANDIDATO,
                           MensagemErroCargo("CriaTelaVotoCargoSemCandidato", cargo, "não é de candidato"));   // :1585

    api::CFormBuilder form;
    adicionaNomeCargo(cargo, form);
    form.Add<api::CTextFieldBlinking>(SPoint{320, 200}, "NÃO HÁ CANDIDATOS CONCORRENDO", SFont{30, 0}, CENTRO);
    form.Add<api::CRectField>(SPoint{0, 400}, SPoint{639, 401});                              // separator line, func 3680
    form.Add<api::CTextField>(SPoint{1, 405}, std::make_shared<api::CFixedText>(ESQUERDA, "Aperte a tecla:"),
                              FONTE_20, api::TColor{2}, api::TColor{1});                         // func 202
    form.Add<api::CTextField>(SPoint{130, 430}, std::make_shared<api::CFixedText>(DIREITA, "CONFIRMA"),
                              FONTE_20, api::TColor{2}, api::TColor{1});
    form.Add<api::CTextField>(SPoint{130, 430}, std::make_shared<api::CFixedText>(ESQUERDA, " para continuar"),
                              FONTE_20, api::TColor{2}, api::TColor{1});
    form.AddInputControl();                               // func 901: CInputFieldControl<IScreen> + CControlValidation
    return form.CriaFormInterativo(std::make_shared<CPreShowProgressBar>(), "telaVotoCargoSemCandidato", true);   // func 554
}

// =====================================================================================================
// ctelasvota.cpp:3366 - wasm func 12272: operator() of the recursive lambda $_2 of
// CTelasVota::CriaTelaVisualizacaoCandidato(const CCandidatura&, size_t, size_t) (body: func 6569, unit u09).
// The "visualizar candidatos" screen (operator menu before the election) lists the running mates of a
// candidate: for running mate `indice` it adds the photo + caption, the "<cargo>: <name>" row and the
// "Gênero: ..." row, then recurses for indice + 1. Excerpt of the enclosing function:
//
//   std::function<void(TPosition, int)> adicionaSuplentes;
//   adicionaSuplentes = [&](TPosition y, int indice) {
//       if (indice > cargo.GetQtdSuplentes())
//           return;
//       if (indice >= 3)
//           throw CUeVotaError(ERRO_TELA_VISUALIZACAO_SUPLENTES, "Sem suporte a mais de 2 suplentes");   // :3366
//       const uebyte i = static_cast<uebyte>(indice);
//       if (cargo.GetDetalheCandidato().GetSuplente(i).PossuiFoto()) {
//           const SRect r = adicionaFotoEmoldurada(form,
//                               indice == cargo.GetQtdSuplentes() ? 639 : 523, 253, 111, 155,
//                               std::bind(fonteFoto /* lambda :3323 */, candidatura.GetSuplente(i)));    // func 6561
//           const TPosition largura = static_cast<TPosition>(r.right - r.left + 1);
//           form.Add<api::CTextField>(SPoint{static_cast<TPosition>(r.left + largura / 2),
//                                            static_cast<TPosition>(r.bottom + 1)},
//                                     std::make_shared<api::CFixedText>(CENTRO,
//                                         cargo.GetNomeCargoSuplente(i, candidatura.GetSuplente(i).sexo)),  // func 3715
//                                     FONTE_13, api::TColor{2}, api::TColor{1});
//       }
//       y = adicionaLinha(y, cargo.GetNomeCargoSuplente(i, candidatura.GetSuplente(i).sexo),
//                         candidatura.GetSuplente(i).nome);                          // func 1588 (lambda $_1), CDadosCandidato +12
//       const int sexo = candidatura.GetSuplente(i).sexo;                           // CDadosCandidato +40
//       y = adicionaLinha(y, "Gênero", sexo == 1 ? "masculino" : sexo == 2 ? "feminino" : "não informado");
//       adicionaSuplentes(static_cast<TPosition>(y + 10), indice + 1);              // std::function call (throws
//   };                                                                              //  bad_function_call if empty)

} // namespace vota

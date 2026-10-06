// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cformbuilder.cpp (srclocs cformbuilder.cpp:292, :305 AddStatusHeader
// and :372 GetLabeledKeyPos). See also cformbuilder.u02.cpp (AddText 202, AddLine 2244, AddLabeledKey
// 3678 ...) and cformbuilder.u07.cpp (Add 426 ...). Merge into cformbuilder.cpp.
#include <cmath>
#include <format>
#include <memory>
#include <string>

#include "api/gui/gui-common.u15.h"
#include "api/gui/cdsimagefield.h"
#include "api/gui/cpowerinformation.h"
#include "api/gui/ctextfieldupdate.h"
#include "api/pattern/cpolysingletonlist.h"

namespace api {

class IPower;   // slot 7 = battery percentage, slot 15 = refresh status word (+4)

// Status-header fields (the argument is a plain unsigned bit mask in the binary; the srcloc spells the
// parameter type as `StatusHeaderFields`). Screens use 5 (date/time + battery icon). Of the 34 direct
// calls in the binary, 32 pass 5, func 1255 passes 7 and func 7787 passes 4: no caller sets bit 8, so the
// battery-percentage text below is unreachable in this build.
enum StatusHeaderFields : unsigned {
    DataHora         = 1,   // "A DD/MM/YYYY hh:mm:ss" clock at (5,5), refreshed every 500 ms   names inferred
    ModoUrna         = 2,   // "SIMULADO" / "TREINAMENTO" / "DEMONSTRAÇÃO" at 53 % of the width
    IconeBateria     = 4,   // battery icon (horizontal) at (width-55, 5)
    PercentualBateria= 8,   // "{: >3}%" text left of the icon
};

// Static data used here:
//   ms_modoUrna @1577208 (int, initial '0'), written once by vota's initialisation (func 7787, from
//   CEstadoGeral +48, i.e. the EUrnaFase '1'/'2'/'3' of the loaded election).
//   ms_areaTeclas @1577200 = SRect{0, 0, 639, 479}: the whole 640x480 screen (used by the labelled keys).

// wasm func 5564 (tools: api_f5564; observed executing)                           // name inferred
// Text of the mode label: demo flag first, then the urna phase.
std::string CFormBuilder::GetModoUrnaTexto()
{
    if (CApplication::ms_demonstracao)          // @1839208, never set in this binary
        return "DEMONSTRAÇÃO";
    switch (ms_modoUrna) {
    case '2': return "SIMULADO";                // EUrnaFase::Simulado (50)
    case '3': return "TREINAMENTO";             // EUrnaFase::Treinamento (51) - what the simulator shows
    default:  return "";                        // oficial ('1'): no label
    }
}

// wasm func 4620 (tools: api_f4620; observed executing)                           // name inferred
// Draws the mode label directly on a screen (used by the pre-show hooks: vota::CPreShowFormVota::PreShow
// at (340,5) and vota::CPreShowProgressBar::PreShow at (320,30); the simulator log shows
// fillText("TREINAMENTO", 640, 78) on the 1280x800 canvas).
void CFormBuilder::DesenhaModoUrna(IScreen& tela, const SPoint& pos)
{
    const std::string modo = GetModoUrnaTexto();
    if (!modo.empty())
        tela.DrawText(pos, CFixedText(modo, ETextAlignment::Center), SFont{20, 1},
                      /*?*/ 3, /*?*/ 0);                                          // IScreen slot 19
}

// wasm func 502 (observed executing) - srclocs cformbuilder.cpp:292 and :305, with the constructors of
// CDSImageField (cdsimagefield.cpp:29) and BatteryIconDataSource<Horizontal> (cpowerinformation.cpp:119)
// inlined.
void CFormBuilder::AddStatusHeader(StatusHeaderFields campos)
{
    const TPosition largura =
        CPolySingletonList::instance<IScreen>().GetWidth();                          // slot 30, srcloc :292

    if (campos & DataHora) {
        // CDataTextFmt<std::string>(3117 = date/time data source, format) + CTextFieldUpdate (func 3058)
        AddDataTextFmt(3117, SPoint{5, 5}, std::chrono::milliseconds{500}, FONTE_ROTULO /*@520920 {20,0}*/,
                       "A DD/MM/YYYY hh:mm:ss", 0);
    }

    const bool bateria = campos & IconeBateria;
    if (campos & PercentualBateria) {
        IPower& energia = CPolySingletonList::instance<IPower>();                    // srcloc :305
        // CDataText<AddStatusHeader(unsigned)::$_0> (vtable @1578584); the lambda is func 11107.
        auto percentual = MakeDataText([&energia] {
            energia.AtualizaEstado();                                                 // IPower slot 15
            if ((energia.GetEstado() & 6) == 4)                                       // no battery reading
                return std::string{};
            return std::format("{: >3}%", energia.GetPercentualBateria());            // IPower slot 7
        });
        Add(std::make_shared<CTextFieldUpdate>(
            SPoint{static_cast<TPosition>(largura + (bateria ? -70 : -55)), 5}, percentual,
            std::chrono::milliseconds{1000}, FONTE_ROTULO));
    }

    if (bateria) {
        auto fonte = std::make_shared<BatteryIconDataSource<CPowerInformation::IconOrientation::Horizontal>>();
        // CDSImageField(pos, fonte, anchor 0): throws CUeGuiError(4923, "A fonte de dados não pode ser nula")
        // if fonte is null (cdsimagefield.cpp:29), then attaches itself as observer of `fonte`.
        std::shared_ptr<IFormField<IScreen>> icone(
            new CDSImageField(SPoint{static_cast<TPosition>(largura - 55), 5}, fonte, EAnchorPoint::TopLeft));
        m_campos.push_back(icone);    // NOTE: pushed directly, without Add(): the field gets no unique name
    }

    if (campos & ModoUrna) {
        const std::string modo = GetModoUrnaTexto();
        if (!modo.empty())
            AddText(modo, SPoint{static_cast<TPosition>(std::trunc(largura * 0.53)), 5}, SFont{20, 1},
                    ETextAlignment::Center, 3, 1);                                   // func 202
    }
}

// wasm func 5544 (not observed) - srcloc cformbuilder.cpp:372
// Position of the key name ("CONFIRMA", "CORRIGE", "BRANCO") and of its caption in the bottom band of
// `area`: left keys at left+23, right keys at right-19, centred keys in the middle; `dy` is 0 for the
// key name and 21 for the caption (see AddLabeledKey, func 3678).
SPoint CFormBuilder::GetLabeledKeyPos(const SRect& area, ETextAlignment alinhamento, TPosition dy)
{
    TPosition x;
    switch (alinhamento) {
    case ETextAlignment::Left:   x = static_cast<TPosition>(area.left + 23); break;
    case ETextAlignment::Right:  x = static_cast<TPosition>(area.right - 19); break;
    case ETextAlignment::Center: x = static_cast<TPosition>((area.right - area.left + 1) / 2 + area.left); break;
    default:
        throw CUeGuiError(static_cast<EUeGuiError>(4914),
                          std::format("Alinhamento desconhecido ({})", static_cast<int>(alinhamento)));
    }
    return SPoint{x, static_cast<TPosition>(area.bottom + dy - 47)};
}

} // namespace api

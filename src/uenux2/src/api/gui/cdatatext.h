// Reconstructed from vota_web_wasm.wasm (unit u32).
// Original: uenux2/src/api/gui/cdatatext.h (path inferred; ctelasvota.cpp of unit u07 already includes it).
//
// Two class templates that turn a "data source" (DS) into an api::IText:
//
//   CDataText<SRC>     GetText() = m_fonte()                 the source produces the whole text
//   CDataTextFmt<SRC>  GetText() = source + format string    (three behaviours, see below)
//
// SRC is anything callable: a plain function pointer (std::string (*)()), a std::function, a lambda, or one
// of the small "DS" functors of the application (comum::CCandidaturasDSNome, vota::CEscolheOpcao::COpcaoDS,
// comum::(anonymous)::CComparecimentoMesariosDS ...). The text is recomputed every time the field is drawn,
// which is how the voter screen follows the digits being typed and the microterminal shows live status.
//
// Every instantiation has its own vtable (4 slots, the IText protocol of itext.h). The destructors of the
// instantiations are tiny thunks into bodies merged by wasm-opt (the vtable is passed as a parameter):
//   api_f1566 / api_f1565   {vptr, align, 4-byte SRC, std::string @+12}   D1 / D0
//   shared_f6053 / 6052     {vptr, align, int, std::function @+16}         D1 / D0
// Instantiations whose SRC is trivially destructible and that have no format string use ICF 174 / 144.
#pragma once

#include <cstdio>
#include <format>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>

#include "api/gui/ctextsource.h"   // api::CTextSource (unit u17)
#include "api/gui/itext.h"

namespace api {

// =========================================================================================================
// CDataText<SRC>  - layout: +0 vptr, +4 ETextAlignment, +8 SRC m_fonte
// =========================================================================================================
template <class SRC>
class CDataText : public IText {
public:
    CDataText(ETextAlignment alinhamento, SRC fonte) : IText(alinhamento), m_fonte(std::move(fonte)) {}
    ~CDataText() override = default;

    // slot 2
    std::string GetText() const override { return m_fonte(); }

private:
    SRC m_fonte;   // +8
};

// =========================================================================================================
// CDataTextFmt<SRC> - layout: +0 vptr, +4 ETextAlignment, +8 SRC m_fonte, then std::string m_formato
//   (+12 for a 4-byte SRC, +16 for CTextSource (a shared_ptr), +32 for a std::function)
//
// GetText() has three behaviours depending on SRC (reconstructed as `if constexpr`; the original may use
// overloads or a specialisation - only the behaviour is attested):                                     // ?
//   * SRC callable with (const std::string&)  -> m_fonte(m_formato)      (the source formats by itself)
//   * SRC = CTextSource                       -> snprintf(buf, 512, m_formato.c_str(), texto.c_str())
//                                                 printf-style; every caller passes "%s"
//   * otherwise (SRC returns a value)          -> std::vformat(m_formato, std::make_format_args(valor))
// =========================================================================================================
template <class SRC>
class CDataTextFmt : public IText {
public:
    CDataTextFmt(ETextAlignment alinhamento, SRC fonte, std::string formato)
        : IText(alinhamento), m_fonte(std::move(fonte)), m_formato(std::move(formato))
    {
    }
    ~CDataTextFmt() override = default;

    // slot 2
    std::string GetText() const override
    {
        if constexpr (std::is_same_v<SRC, CTextSource>) {
            // wasm func 10746 (CDataTextFmt<CTextSource>)
            char buffer[512];
            std::snprintf(buffer, sizeof buffer, m_formato.c_str(), m_fonte().c_str());
            return std::string(buffer);
        } else if constexpr (std::is_invocable_v<const SRC&, const std::string&>) {
            // wasm func 12287 (std::function<std::string(const std::string&)>),
            //           6404  (ICF: both function-pointer instantiations)
            return m_fonte(m_formato);
        } else {
            // wasm funcs 5387 / 10368 (CComparecimentoMesariosDS: the source returns an unsigned count)
            const auto valor = m_fonte();
            return std::vformat(m_formato, std::make_format_args(valor));
        }
    }

private:
    SRC         m_fonte;     // +8
    std::string m_formato;   // after m_fonte
};

// =========================================================================================================
// Instantiations present in the binary: 12 CDataText and 7 CDataTextFmt types (vtable address: slot0 D1 /
// slot1 D0 / slot2 GetText; "*" = observed executing during the recorded votes). Only the functions of unit
// u32 are written out; the others are listed so that every instantiation of the templates can be found here.
// =========================================================================================================
//
// --- CDataText ---------------------------------------------------------------------------------------------
// CDataText<CTextSource>                                   @1590656  10536 / 10535 / 10534
//     microterminal lines bound to a shared std::string (e.g. the voter's name on "O eleitor está
//     demorando"); built by func 5409. GetText = *m_texto (CTextSource::operator()).
//     D1/D0 release the shared_ptr<std::string> at +8/+12.
//
// CDataText<vota::CEscolheOpcao::COpcaoDS>                 @1588108  10688 / 10686 / 10685
//     the numbered options of the poll worker's "outras opções" menu, built by vota::CEscolheOpcao::GetInst
//     (func 2753). COpcaoDS = { int m_numero (+8); std::function<std::string()> m_descricao (+16) };
//     its operator() is inlined in 10685:
//         return std::format("{}-{}", m_numero, m_descricao());       // "<n>-<descrição da opção>"
//     (std::bad_function_call if the std::function is empty).
//
// CDataText<CFormBuilder::AddStatusHeader(unsigned)::$_0>  @1578584  174 / 144 / 11107
//     battery percentage of the status header. The lambda (captures IPower& at +8) is inlined in 11107:
//         energia.AtualizaStatus(energia.m_status);  // IPower slot 15, called with a pointer to IPower's
//                                                    // own +4 field (ipower.h, unit u18)
//         if ((energia.m_status & 6) == 4) return "";                   // no battery reading
//         return std::format("{: >3}%", energia.GetPercentualBateria()); // IPower slot 7
//     (see cformbuilder.u15.cpp, unit u15, for the rest of AddStatusHeader).
//
// CDataText<std::function<std::string()>>                  @1539324  12305 / 12303 (u16) / 12301
// CDataText<const std::string& (*)()>                      @1538140  174 / 144 / 12580*
//     vota::VotoDigitado (func 13176, table slot 1098, returns the static @1833288): the digits the voter
//     typed; built by 3060 and adicionaBaseTelaVotoNuloConsulta (6624).
// CDataText<std::string (*)()>                             @1537088  174 / 144 / 12784*
// CDataText<comum::CCandidaturasDSNome>                    @1537440  174 / 144 / 12649*   (func 5807 =
//     CCandidaturasDSNome::operator(): name of the current candidate or of running mate n)
// Instantiations whose GetText is in other units (listed so that all 12 CDataText types are here):
// CDataText<comum::CCandidaturasDSNumero>                  @1537204  174 / 144 / 12742
// CDataText<comum::CCargoDSNomeSexoCandidato>              @1537644  174 / 144 / 12632
// CDataText<comum::CRespostasDSNumero>                     @1537840  174 / 144 / 12621
// CDataText<vota::(anonymous)::DS_NomeCargoNeutroComEscolha>
//                                                          @1538048  12589 / 12588 / 12587
// CDataText<comum::CPadDS<comum::CToUpperDS<comum::CCargoDSNome>>>
//                                                          @1575436  174 / 144 / 11247
//
// --- CDataTextFmt ------------------------------------------------------------------------------------------
// CDataTextFmt<CTextSource>                                @1587072  10748 / 10747 / 10746
//     D1/D0 free m_formato (+16) then release the shared_ptr (+8/+12).
// CDataTextFmt<std::function<std::string(const std::string&)>>
//                                                          @1539440  12291 / 12290 / 12287
//     D1/D0 free m_formato (+32) then destroy the std::function (+8; __f_ at +24: in-place -> destroy(),
//     heap -> destroy_deallocate()).
// CDataTextFmt<std::string (*)(const std::string&)>        @1539168  12323 / 12317 / 6404
// CDataTextFmt<const std::string (*)(const std::string&)>  @1538280  12567 / 12564 / 6404
// CDataTextFmt<comum::(anonymous)::CComparecimentoMesariosDS>, three distinct types with the same RTTI name
//   (an anonymous-namespace class defined in a header seen by three translation units; the three
//   StartState helpers were inlined by LTO into comum::CPedeTituloMesario::StartState, func 10388):
//                                                          @1594036  10377 / 10374 / 5387
//                                                          @1594188  10370 / 10369 / 10368
//                                                          @1594340  10363 / 10362 / 5387
//     GetText: std::vformat(m_formato, make_format_args(count)) with a count read through
//     api::persistencia::CDAORepositorio (mesário attendance, "comparecimento de mesários").

} // namespace api

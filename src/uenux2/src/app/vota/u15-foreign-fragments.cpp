// FRAGMENTS reconstructed by unit u15 from vota_web_wasm.wasm.
//
// The analysis tools attributed these vota functions to u15's api/gui files because an api/gui
// function (CInteractiveForm::Read, the CImageFieldUpdate constructor, CFormBuilder's mode label) is
// inlined into them. Their real homes (paths from srclocs of sibling functions, or inferred) are given
// per function. Merge each into its file.
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "api/gui/cformbuilder.h"
#include "api/gui/cinteractiveform.u15.h"
#include "comum/cappstate.h"

namespace vota {

// =================================================================================================
// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (anonymous-namespace helper; name as in
// ctelasvota.u02.cpp, which already calls it "adicionaBlocoMensagem")
// =================================================================================================
namespace {

// wasm func 3059 (observed executing; tools: api::CImageFieldUpdate::CImageFieldUpdate)   name inferred
// Common body of the zerésima-time screens (CriaTelaConfirmaImpressaoZeresima, func 6592, variante 2;
// CriaTelaAntesHorarioZeresima, func 6595, variante 0; and the CTelasVota constructor, inlined into
// func 7787 = vota::CInformacaoEleitor::Inicializar):
// a header, the correspondence summary, a rotating QR code with the urna identification, three lines
// of text, the software version and the first 8 Base64 characters of the data-package hash.
void adicionaBlocoMensagem(api::CFormBuilder& builder, const std::string& titulo,
                           const std::string& linha1, const std::string& linha2, int variante)
{
    const auto& estado = comum::GetEstado<comum::md::estadoaplicacao::CEstadoGeral>();   // funcs 185/457

    builder.AddStatusHeader(5);                          // date/time + battery icon (func 502)
    adicionaCabecalhoZeresima(builder);                  // func 4160: election names, município/UF, zona/seções
    builder.AddText("RESUMO DA CORRESPONDÊNCIA: " + FormataCorrespondencia(estado.GetCorrespondencia()),  // func 2792, +60
                    {320, 200}, api::SFont{20, 0} /*@474896*/, api::ETextAlignment::Center, 2, 1);

    // QR code of the urna state: comum_f5636 builds the tagged record (tags SERT, IDFL, SERI, NOME, UNFE,
    // IDCA ...), comum_f1300 splits it into (name, value) pairs and comum_f5783 adapts them to `variante`.
    auto pares = comum::QRCodeEstadoUrna(estado, variante);                          // names inferred
    auto qrcode = std::make_shared<api::CQRCodeImage>(148, comum::CQRCodeDS(std::move(pares)));   // func 3662
    builder.Add(std::make_shared<api::CImageFieldUpdate>(api::SPoint{626, 250}, qrcode,
                                                         std::chrono::milliseconds{15000},
                                                         api::EAnchorPoint::TopRight));   // new page every 15 s

    builder.AddText(titulo, {25, 260}, api::SFont{35, 0} /*@475016*/, api::ETextAlignment::Left, 2, 1);
    if (!linha1.empty())
        builder.AddText(linha1, {25, 310}, api::SFont{20, 0}, api::ETextAlignment::Left, 2, 1);
    if (!linha2.empty())
        builder.AddText(linha2, {25, 340}, api::SFont{20, 0}, api::ETextAlignment::Left, 2, 1);
    builder.AddText(std::format("Versão: {}", api::CApplication::ms_versao),       // @1839192
                    {25, 400}, api::SFont{15, 0} /*@475008*/, api::ETextAlignment::Left, 2, 1);
    builder.AddText(std::format("Dados: {}", HashPacotesBase64(estado)),         // func 3703: 8 chars
                    {620, 400}, api::SFont{15, 0}, api::ETextAlignment::Right, 2, 1);
}

} // namespace

// =================================================================================================
// vota/eleitor/comum/cpreshowformvota.cpp (path inferred; sibling of CPreShowProgressBar)
// =================================================================================================

// wasm func 7707 (observed executing) - vota::CPreShowFormVota vtable slot 2 (IPreShow<IScreen>::PreShow)
// Runs before a CPreShowFormVota form is drawn: clears the screen and draws the mode label.
void CPreShowFormVota::PreShow(api::IScreen& tela)
{
    tela.Clear(1);                                             // IScreen slot 4
    api::CFormBuilder::DesenhaModoUrna(tela, {340, 5});        // func 4620 -> "TREINAMENTO" in the simulator
}

// =================================================================================================
// Poll-worker (mesário) terminal states - microterminal forms CInteractiveForm<IScreenMT, IInputMT>
// =================================================================================================

// wasm func 5416 (tools: vota_f5416)                                               // name inferred
// How the voter was identified at the microterminal: IInformacaoThreadOperador slot 24
// (2 = CPF). Callers: func 10588 below and func 10664.
static int TipoIdentificacaoEleitor()
{
    return impl::IInformacaoThreadOperador::GetInst().GetTipoIdentificacao();       // func 599, slot 24
}

// uenux2/src/app/vota/operador/confirmaidentidade/cpedeanonascimento.cpp (path inferred)
// wasm func 5418 (tools: vota_f5418; not observed)                                 // name inferred
// Lazily created singleton state (pointer @1905980, mutex @1905956) that asks the voter's birth year
// before a justification ("justificativa" = the form a voter fills to justify not voting in their
// own section). Microterminal LCD layout (column, row):
//   (1,1) data text 3907 | (1,2) "Digite o ANO de nascimento: " | (29,2) 4-digit input |
//   (1,4) "CORRIGE: cancelar" | (40,4 right-aligned) "CONFIRMA: justificar"
CPedeAnoNascimento& CPedeAnoNascimento::GetInst()
{
    static std::unique_ptr<CPedeAnoNascimento> instancia;    // @1905980, replaced under the mutex
    if (!instancia)
        instancia = std::make_unique<CPedeAnoNascimento>();  // CAppState(flags 2 = accepts keys), vtable @1589884
    return *instancia;
}

// uenux2/src/app/vota/operador/justificativa/iconfirmajustificativa.cpp
// wasm func 10588 (not observed) - vtable slot 7 of IConfirmaJustificativa and of CConfirmaJustificativa,
// CConfirmaJustificativaTemporario, CConfirmaJustificativaTransito (table slot 3947). Named
// CInteractiveForm<IScreenMT, IInputMT>::Read by the tools because Read() (cinteractiveform.h:57) is inlined.
void IConfirmaJustificativa::ProcessInput()
{
    switch (m_tela->Read()) {                                   // m_tela at +12
    case api::EInputResult::Corrige:
        m_proximoEstado = &CPedeIdentidade::GetInst();          // func 652 (tools: CTextSource ctor)
        break;
    case api::EInputResult::Confirma:
        if (TipoIdentificacaoEleitor() == 2 /*CPF*/)
            // lazily created singleton @1905868 (mutex @1905844), screen:
            //   "Não é permitido justificar com o CPF" / "utilize o número do título" / "CORRIGE: retornar"
            m_proximoEstado = &CEleitorImpedidoJustificarVotoCPF::GetInst();
        else
            m_proximoEstado = &CPedeAnoNascimento::GetInst();   // func 5418
        break;
    default:
        break;
    }
}

// uenux2/src/app/vota/operador/leidentidade/ieleitorimpedidovotar.cpp (path inferred; its destructor,
// func 1257, is attributed to celeitorencontrado.cpp)
// wasm func 10624 (not observed) - vtable slot 7 of IEleitorImpedidoVotar and of CEleitorNaoEncontrado,
// CEleitorOptouPorVotarEmTransito, ... (table slot 3874). Same inlined Read().
// The "voter cannot vote here" message screens: the key that dismisses them depends on m_tecla (+20).
void IEleitorImpedidoVotar::ProcessInput()
{
    switch (m_tela->Read()) {
    case api::EInputResult::Corrige:
        if (m_tecla == 1)
            m_proximoEstado = &CPedeIdentidade::GetInst();
        break;
    case api::EInputResult::Confirma:
        if (m_tecla == 0)
            m_proximoEstado = &CPedeIdentidade::GetInst();
        break;
    default:
        break;
    }
}

} // namespace vota

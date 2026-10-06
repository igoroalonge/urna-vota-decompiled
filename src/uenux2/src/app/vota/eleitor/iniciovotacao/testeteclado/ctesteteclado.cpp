// uenux2/src/app/vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records of this file:
//   :38 / :42  CTesteTeclado::StartState       (func 11805)
//   :50 / :60 / :81 CTesteTeclado::ProcessInput (func 11804)
//   :121 impl::IGeradorTeclas::GetInst         (inlined into 11805)
// plus, inlined into 11805: ctelasvota.cpp:1301 (anonymous)::GetKeyboardLayout (see ctelasvota.u26.cpp),
// ctextbox.cpp:41 (CTextBox constructor) and the CPolySingletonList push/instance checks.
//
// Web build: the keyboard test belongs to the start-of-day flow (CPreZeresima / CRetomada) that the web page
// never enters; nothing here was observed executing.
#include "vota/eleitor/iniciovotacao/testeteclado/ctesteteclado.h"

#include <memory>
#include <mutex>
#include <source_location>

#include "api/gui/cformbuilder.h"
#include "api/hwil/iinputkbd.h"
#include "api/pattern/cpolysingletonlist.h"
#include "vota/comum/votadefs.h"                              // CUeVotaError
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/iniciovotacao/testeteclado/ctestefalhou.h"   // CTesteFalhou (unit u02, path inferred)
#include "vota/log/clogvota.h"

namespace vota::testeteclado {

namespace {

std::mutex s_mutex;                                   // @1835124 (unlock residue)
std::unique_ptr<CTesteTeclado> s_instancia;           // @1835148, reset at exit by func 11808

// wasm func 3054 (name inferred). Common top of the keyboard-test screens (also used by CEsperaRetestar,
// func 5936, and CTesteFalhou, funcs 11804/11809).
void AdicionaCabecalhoTesteTeclado(api::CFormBuilder& builder)
{
    builder.AddStatusHeader(5);                                                             // func 502
    builder.AddText("Teste de teclado", api::SPoint{320, 80}, FONTE_30 /* @474888 */, 2, 2, 1);   // func 202
    builder.AddLine(api::SPoint{640, 429}, api::SPoint{0, 429}, 3);                         // func 2244
}

struct STelaTesteTeclado {                            // name inferred
    CFormInterativoTelaVota tela;
    std::map<std::string, CTesteTeclado::STecla> teclas;
};

// Inlined into func 11805 (name inferred): the drawing of the voter keypad, one CTextBox per key.
STelaTesteTeclado CriaTelaTesteTeclado()
{
    api::CFormBuilder builder;
    const auto layout = CTelasVota::GetKeyboardLayout();     // ctelasvota.cpp:1301 (inlined; std::map built by func 6579)
    std::map<std::string, CTesteTeclado::STecla> teclas;
    for (const std::string& nome : CTelasVota::ms_nomesTeclas) {     // @1833360: "1".."9","0","BRANCO","CORRIGE","CONFIRMA"
        const auto& posicao = layout.at(nome);                         // "map::at:  key not found"
        // ctextbox.cpp:41: throws CUeGuiError 4961 "A posição da caixa não pode ter x < 3 ou y < 1."
        auto caixa = std::make_shared<api::CTextBox>(nome, api::ETextStatus(2), posicao.centro, posicao.fonte,
                                                     api::ETextAlignment(2) /*?*/, posicao.tamanho,
                                                     api::TColor{1}, api::TColor{2});   // colours ?
        builder.Add(caixa);                                                             // ecourna_f426
        teclas.emplace(nome, CTesteTeclado::STecla{posicao.tecla, caixa});
    }
    AdicionaCabecalhoTesteTeclado(builder);                                             // func 3054
    builder.AddText("Pressione a tecla destacada", api::SPoint{320, 440}, FONTE /* @474896 */, 2, 2, 1);
    return {CTelasVota::CriaFormInterativoVota(builder, "telaTesteTeclado"), std::move(teclas)};   // func 576
}

} // namespace

// ctesteteclado.cpp:121 - inlined into func 11805. Default registration of the random generator.
impl::IGeradorTeclas& impl::IGeradorTeclas::GetInst()
{
    auto& info = api::GetPolySingletonsInfo();
    if (!api::CPolySingletonList::contains<IGeradorTeclas>(info))                          // api_f3853
        api::CPolySingletonList::push<IGeradorTeclas>(std::make_unique<CGeradorTeclasAleatorio>(), info);
    return api::CPolySingletonList::instance<IGeradorTeclas>(info);                        // :121
}

// wasm func 3855 (name inferred)
CTesteTeclado& CTesteTeclado::GetInst()
{
    std::lock_guard trava(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CTesteTeclado());
    return *s_instancia;
}

// wasm func 2868 (vtable slot 0): m_sequencia, m_teclas (func 3685-like __tree::destroy), m_tela; the
// deleting destructor (slot 1) is func 11806.
CTesteTeclado::~CTesteTeclado() = default;

// wasm func 11805 (vtable slot 2)
void CTesteTeclado::StartState()
{
    CLogVota::GetInst().Loga("Início do teste de Teclado do TE");
    m_proximoEstado = this;

    auto tela = CriaTelaTesteTeclado();
    m_tela = std::move(tela.tela);
    m_teclas = std::move(tela.teclas);

    m_sequencia = impl::IGeradorTeclas::GetInst().GeraSequencia(CTelasVota::ms_nomesTeclas);   // slot 2
    m_indice = 0;
    if (m_sequencia.empty())
        throw CUeVotaError(9359, "Sem teclas para testar");                                        // :38

    const auto it = m_teclas.find(m_sequencia[0]);
    if (it == m_teclas.end())
        throw CUeVotaError(9360, "Tecla não encontrada: " + m_sequencia.at(m_indice));            // :42
    m_tela->Exibe();                                                                               // form slot 2
    it->second.caixa->SetEstado(1);                  // func 940: highlight the key to press (status 1)
}

// wasm func 11804 (vtable slot 7)
void CTesteTeclado::ProcessInput()
{
    auto& teclado = api::CPolySingletonList::instance<api::IInputKbd>();                  // :50 (func 455)
    if (!teclado.HasKey())                                                                 // slot 3
        return;
    // api::IInput::GetKey() inlined (iinput.h:86): HasKey() again, else CUeHwilError 5170
    // "IInput - Nao havia um caractere disponivel"; key = slot 2; key counter (+4) incremented.
    const char tecla = teclado.GetKey();

    const auto atual = m_teclas.find(m_sequencia[m_indice]);
    if (atual == m_teclas.end())
        throw CUeVotaError(9361, "Tecla não encontrada: " + m_sequencia.at(m_indice));            // :60

    if (atual->second.tecla != tecla) {
        // Wrong key. NOTE: the log record written here is the *success* one.
        CLogVota::GetInst().LogaFimTesteTecladoSucesso();                                         // func 4511
        auto& informacao = CInformacaoEleitor::GetInst();                                         // func 509
        informacao.SetTeclaTesteEsperada(atual->second.tecla);                                    // +9   name inferred
        informacao.SetTeclaTestePressionada(tecla);                                               // +10  name inferred
        // CTesteFalhou::GetInst() inlined: lazy singleton @1835120 (36 bytes, CAppState flags 2) whose
        // screen "telaTeclaErradaTesteTeclado" is: header (func 3054), "Teste Falhou" ({320,140}, FONTE_30),
        // a CDataText refreshed from table slot 1106 = func 12903 "Falha: Esperada {}, pressionada {}"
        // (api::KeyName of the two keys above) at {320,210}, keys {'C', "Repetir teste"}, {'D', "Prosseguir"}
        // and the control input (func 901).
        m_proximoEstado = &CTesteFalhou::GetInst();
        return;
    }

    atual->second.caixa->SetEstado(2);                                                            // back to normal
    ++m_indice;
    if (m_indice == m_sequencia.size()) {
        CLogVota::GetInst().LogaFimTesteTecladoSucesso();                                         // func 4511
        m_proximoEstado = m_estadoAposTeste;
        return;
    }

    const auto proxima = m_teclas.find(m_sequencia[m_indice]);
    if (proxima == m_teclas.end())
        throw CUeVotaError(9362, "Tecla não encontrada: " + m_sequencia.at(m_indice));            // :81
    proxima->second.caixa->SetEstado(1);
}

// Library instances the tools filed in this file:
//   func 4148 = std::__tree<std::string, ...>::destroy(node) (recursive; string key at node +16) of the layout map
//   func 6579 = std::map<std::string, SLayoutTecla>::map(std::initializer_list) (13 entries of 24 bytes)

} // namespace vota::testeteclado

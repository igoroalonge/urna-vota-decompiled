// uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :33
// (StartState). Func 10388 also contains the inlined constructors of CPedeTituloMesarioInicial
// (cpedetitulomesarioinicial.cpp:44), CPedeTituloMesarioVotacao (cpedetitulomesariovotacao.cpp:44) and
// CPedeTituloMesarioFinal (cpedetitulomesariofinal.cpp:45); they are reconstructed here and belong to
// those three files.
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <mutex>

#include "api/gui/cformbuildermt.h"
#include "api/pattern/cpolysingletonlist.h"

namespace comum {

using api::SPoint;

namespace {
IControladorRegistraMesarios& GetControlador()                                     // srcloc :33 / :44 / :45
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}

// Count of mesários already registered in the period, shown on line 3 ("{:02}"). The data-source
// functors differ per subclass (vtables @1594036, @1594340, @1594188) but their operator() bodies
// are ICF-merged (funcs 5387 / 10368, read CRegistradorMesario - func 815).                 ?
struct CComparecimentoMesariosDS;

// The MT form of the three title screens (inlined three times).                         name inferred
std::shared_ptr<TFormMT> CriaFormPedeTitulo()
{
    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(ESQUERDA, "Informe o título do mesário: "));
    campos.Add<api::CClockFieldMT>(SPoint{33, 1});                                  // func 728
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(ESQUERDA, "Quantidade de mesários registrados:"));
    campos.Add<api::CTextFieldMT>(SPoint{39, 3},
        std::make_shared<api::CDataTextFmt<CComparecimentoMesariosDS>>(ESQUERDA, "{:02}"));   // new CTextFieldMT (1262)
    campos.AddNumberInput(12, 1, SPoint{1, 2});                                     // func 1151: 12-digit título
    campos.Add<api::CTextFieldMT>(SPoint{1, 4}, std::make_shared<api::CFixedText>(ESQUERDA,
                                                "CORRIGE: Encerrar  CONFIRMA: Prosseguir"));
    return campos.CriaFormInterativo("", true);                                     // func 301
}
} // namespace

// ---- constructors (inlined into func 10388) --------------------------------------------------------
CPedeTituloMesarioInicial::CPedeTituloMesarioInicial()
    : IPedeTituloMesario(CriaFormPedeTitulo(), CriaFormEleitorRegistroMesarios())   // func 3608
{
    GetControlador().LogaRegistroAntesVotacao();                                    // slot 20 (:44)
}
CPedeTituloMesarioVotacao::CPedeTituloMesarioVotacao()
    : IPedeTituloMesario(CriaFormPedeTitulo(), CriaFormEleitorRegistroMesarios())
{
    GetControlador().LogaRegistroDuranteVotacao();                                  // slot 21 (:44)
}
CPedeTituloMesarioFinal::CPedeTituloMesarioFinal()
    : IPedeTituloMesario(CriaFormPedeTitulo(), CriaFormEleitorRegistroMesarios())
{
    GetControlador().LogaRegistroAposVotacao();                                     // slot 22 (:45)
}
// NOTE: the "Registrando mesários antes/durante/após a votação" log entries are emitted by the
// constructors, i.e. only the FIRST time each singleton is created in a process run.

CPedeTituloMesarioInicial& CPedeTituloMesarioInicial::GetInst()
{
    static std::mutex mutex;                                        // @1909516
    static std::unique_ptr<CPedeTituloMesarioInicial> s_inst;       // @1909540 (reset at exit: func 10379)
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeTituloMesarioInicial());
    return *s_inst;
}
CPedeTituloMesarioVotacao& CPedeTituloMesarioVotacao::GetInst()
{
    static std::mutex mutex;                                        // @1909572
    static std::unique_ptr<CPedeTituloMesarioVotacao> s_inst;       // @1909596 (reset at exit: func 10366)
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeTituloMesarioVotacao());
    return *s_inst;
}
CPedeTituloMesarioFinal& CPedeTituloMesarioFinal::GetInst()
{
    static std::mutex mutex;                                        // @1909544
    static std::unique_ptr<CPedeTituloMesarioFinal> s_inst;         // @1909568 (reset at exit: func 10372)
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeTituloMesarioFinal());
    return *s_inst;
}
// funcs 10366 / 10372 / 10379: the three exit-time resets, each `func 3881(&s_inst)` =
// std::unique_ptr<IPedeTituloMesario>::reset() (virtual destructor devirtualised to func 1378).

// wasm func 10388 - vtable slot 2 (srcloc :33)
void CPedeTituloMesario::StartState()
{
    m_proximoEstado = this;
    switch (GetControlador().GetPeriodoRegistro()) {                                // slot 6
    case EPeriodoRegistro::INICIAL:
        m_proximoEstado = &CPedeTituloMesarioInicial::GetInst();
        break;
    case EPeriodoRegistro::VOTACAO:
        m_proximoEstado = &CPedeTituloMesarioVotacao::GetInst();
        break;
    case EPeriodoRegistro::FINAL:
        m_proximoEstado = &CPedeTituloMesarioFinal::GetInst();
        break;
    default:
        break;   // NENHUM: stays in this state, nothing is drawn (see the u22 doc, "weird code")
    }
}

// wasm func 2729 (other unit)
CPedeTituloMesario& CPedeTituloMesario::GetInst()
{
    static std::mutex mutex;                                        // @1909432
    static std::unique_ptr<CPedeTituloMesario> s_inst;              // @1909456
    std::lock_guard trava(mutex);
    if (!s_inst)
        s_inst.reset(new CPedeTituloMesario());
    return *s_inst;
}

} // namespace comum

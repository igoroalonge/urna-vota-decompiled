// uenux2/src/app/comum/comparecimentomesario/estados/ipedetitulomesario.cpp
// Reconstructed from vota_web_wasm.wasm (unit u22). Attested by the std::source_location record :30
// (the anonymous-namespace GetControlador). Func 10313 (ProcessInput, 6002 bytes) also contains the
// inlined constructors + GetInst of CTituloMesarioVazio, CTituloMesarioInvalido,
// CTituloMesarioJaRegistrado and CPedeDigitalMesario (srcloc cpededigitalmesario.cpp:304); they are
// reconstructed in their own files.
#include "comum/comparecimentomesario/estados/estadosregistromesarios.h"

#include <format>

#include "api/pattern/cpolysingletonlist.h"
#include "comum/cconfiguracaoeleicao.h"
#include "comum/comparecimentomesario/cregistradormesario.h"
#include "comum/dados/clocal.h"
#include "comum/dados/md/cvalidadoridentidade.h"

namespace comum {

namespace {
// srcloc :30
IControladorRegistraMesarios& GetControlador()
{
    return api::CPolySingletonList::instance<IControladorRegistraMesarios>();
}
constexpr std::size_t TAMANHO_TITULO = 12;
} // namespace

// wasm func 3608 - also installs the IPedeTituloMesario vtable; the subclasses overwrite it.
IPedeTituloMesario::IPedeTituloMesario(std::shared_ptr<TFormMT> formMT, std::shared_ptr<TFormEleitor> formEleitor)
    : CAppState(2)                                                                // keys
    , m_formMT(std::move(formMT))
    , m_formEleitor(std::move(formEleitor))
{
}

// wasm func 1378 (slot 0 of IPedeTituloMesario and its three subclasses; slot 1 = ICF 3609 / 325)
IPedeTituloMesario::~IPedeTituloMesario() = default;

// wasm func 10314 - vtable slot 2 (shared by the three subclasses)
void IPedeTituloMesario::StartState()
{
    m_proximoEstado = this;
    m_formEleitor->Show();
    m_formMT->Show();
}

// wasm func 10313 - vtable slot 7 (shared). The tools named it "(anonymous namespace)::GetControlador"
// after the first srcloc found in it.
void IPedeTituloMesario::ProcessInput()
{
    const api::EInputResult resultado = m_formMT->Read();                          // cinteractiveform.h:57
    if (resultado != api::EInputResult::CONFIRMA && resultado != api::EInputResult::CORRIGE)
        return;

    const std::string& digitado = m_formMT->GetInputField(0).GetText();            // fields.at(0) +24

    if (resultado == api::EInputResult::CORRIGE) {
        // With digits typed, CORRIGE only erases them (handled by the input field).
        if (digitado.empty())
            m_proximoEstado = &CConfirmaFimRegistroMesarios::GetInst();             // func 5389
        return;
    }

    // CONFIRMA
    if (digitado.empty()) {
        m_proximoEstado = &CTituloMesarioVazio::GetInst();
        return;
    }

    const std::string titulo = std::format("{:0>{}}", digitado, TAMANHO_TITULO);   // left-pad with '0' to 12
    GetControlador().SetTituloMesario(titulo);                                     // slot 18

    if (!md::CValidadorIdentidade::GetInst().Valida(ETipoIdentificador::TITULO, titulo)   // vota_f2803
        || titulo == "000000000000") {
        m_proximoEstado = &CTituloMesarioInvalido::GetInst();
        return;
    }

    auto& registrador = CRegistradorMesario::GetInst();                            // func 815
    const md::CComparecimentoMesarioPK chave(md::CEleitorIdentidade(titulo, ETipoIdentificador::TITULO),
                                             GetPeriodoPresente());                // func 2738, slot 9
    if (registrador.Posiciona(chave)) {                                            // map::find (shared_f2222); name inferred
        m_proximoEstado = &CTituloMesarioJaRegistrado::GetInst();
        return;
    }

    // Biometric check only when the election uses biometrics and the urna is not abroad
    // (CLocal::EhExterior: UF == "ZZ"; inlined, VerificaLido("EhExterior") / VerificaLido("GetUF")).
    const bool exterior = CLocal::GetInst().EhExterior();                          // func 401
    if (!exterior && CConfiguracaoEleicao::GetInst().GetUtilizaBiometria())        // cfg +665 bit 0
        m_proximoEstado = &CPedeDigitalMesario::GetInst();
    else
        m_proximoEstado = &CGestorDadoMesario::GetInst();                          // func 5383
}

} // namespace comum

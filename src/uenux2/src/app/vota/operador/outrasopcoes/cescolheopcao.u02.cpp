// uenux2/src/app/vota/operador/outrasopcoes/cescolheopcao.cpp  --  FRAGMENT written by unit u02
// (owner u27). Three text providers of the "contadores de biometria" screen built by
// vota::CEscolheOpcao::ProcessInput (wasm 10689), which passes them as function pointers
// (table slots 3818/3819/3820) to a CDataTextFmt (wasm 619). Names inferred.

#include <format>
#include <string>

namespace vota {
namespace {

// Common shape: "n.a." when the polling place is not biometric, otherwise the counter as {:04}.
template <class CONTADOR>
std::string TextoContador(const char* rotulo, CONTADOR contador)
{
    std::string valor = "n.a.";
    if (comum::CLocal::GetInst().UrnaBiometrica())                                  // wasm 820
        valor = std::format("{:04}", contador(comum::CEleitores::GetInst()));
    return std::vformat(rotulo, std::make_format_args(valor));
}

// wasm func 10701 (slot 3818)
std::string TextoHabilitacaoBiometrica()
{
    return TextoContador("Habilitação biométrica: {}",
                         [](auto& e) { return e.GetQtdHabilitacoesBiometricas(); });     // wasm 2821
}

// wasm func 10700 (slot 3819)
std::string TextoHabilitacaoBiografica()
{
    return TextoContador("Habilitação biográfica: {}",
                         [](auto& e) { return e.GetQtdHabilitacoesBiograficas(); });     // wasm 1935
}

// wasm func 10699 (slot 3820)
std::string TextoHabilitacaoSemBiometria()
{
    return TextoContador("Habilitação sem biometria: {}",
                         [](auto& e) { return e.GetQtdHabilitacoesSemBiometria(); });    // wasm 2822
}

} // namespace
} // namespace vota

// uenux2/src/app/vota/eleitor/votaproporcional/ccompletaproporcional.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location records:
//   :60 virtual std::pair<api::EInputResult, std::string>
//       CCompletaProporcional::EmiteEcoComInputField(const CFormInterativoTelaVota&) const   (func 5930)
//   :83 CFormInterativoTelaVota CCompletaProporcional::GetTelaCargoAtual(const std::string&) (func 5931)
// Observed executing in the recorded votes: 5930, 5931, 11756, 11757 (as CPedeNominal, Vereador 91001).
#include "vota/eleitor/votaproporcional/ccompletaproporcional.h"

#include <source_location>

#include "api/gui/cinteractiveform.h"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/celeitorvotando.h"               // g_votoDigitado
#include "vota/eleitor/votaproporcional/cpedeproporcional.h"

namespace vota {

// wasm func 5932 = the constructor declared inline in the header (CVotacaoStateAudio ctor func 1785 with
// flags 6, then +28 = tela). Called by the lazy GetInst of CPedeNulo (5) and CPedeNominal (8).

// ccompletaproporcional.cpp:83 - wasm func 5931: thunk passing srcloc :83 and code 9384 to the merged body
// func 3921 (reconstructed by unit u06 in cconfirmavotoemcargo.cpp):
//   if (CCargos::GetInst().IsEnd()) throw CUeVotaError(9384, funcao + " - o cargo atual nao esta posicionado");
//   return CTelasVota::GetInst().GetTelaCargo(cargo atual, m_tela);

// wasm func 5930 (vtable slot 13, srcloc :60). Reads one key from the (single) input field and echoes it.
// Differs from CVotacaoStateAudio's version (func 3139) only in the refusal test: here only a digit that did
// not change the text (field already full) is refused.
std::pair<api::EInputResult, std::string>
CCompletaProporcional::EmiteEcoComInputField(const CFormInterativoTelaVota& tela) const
{
    const auto& campos = tela->GetInputs();
    if (campos.size() != 1)
        throw CUeVotaError(9383, "Nao havia campos de input no formulario sem nada digitado");   // :60
    auto* campo = campos[0];
    const std::string antes = campo->GetTexto();                       // input field +24
    const api::EInputResult resultado = tela->Read();                  // cinteractiveform.h:57 (inlined)
    const std::string depois = campo->GetTexto();

    char tecla = campo->GetUltimaTecla();                              // input field +55
    if (resultado == api::EInputResult::Tecla && antes == depois)      // 13
        tecla = 0;                                                     // PlayKey(0): "Tecla indevida" beep + log
    PlayKey(tecla);                                                    // func 1455
    return {resultado, depois};
}

// wasm func 11757 (vtable slot 10)
void CCompletaProporcional::StartStateAudio()
{
    GetTelaCargoAtual(__PRETTY_FUNCTION__)->Exibe();   // "virtual void vota::CCompletaProporcional::StartStateAudio()"
    m_proximoEstado = this;
}

// wasm func 11756 (vtable slot 9)
void CCompletaProporcional::ProcessInputAudio()
{
    const auto [resultado, digitado] =
        EmiteEcoComInputField(GetTelaCargoAtual(__PRETTY_FUNCTION__));   // "virtual void vota::CCompletaProporcional::ProcessInputAudio()"
    switch (resultado) {
    case api::EInputResult::Corrige:                                     // 5: start this vote again
        m_proximoEstado = &CPedeProporcional::GetInst();                 // func 3849
        break;
    case api::EInputResult::Confirma: {                                  // 9: remaining digits complete
        const std::string numero = g_votoDigitado + digitado;            // party digits + candidate digits
        g_votoDigitado = numero;
        m_proximoEstado = GetProximoEstado(numero);                      // slot 16
        break;
    }
    default:
        break;
    }
}

} // namespace vota

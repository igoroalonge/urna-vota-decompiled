// Reconstructed from vota_web_wasm.wasm (unit u17).
// Original: uenux2/src/api/hwil/iinput.h (srclocs iinput.h:61 KeyName, iinput.h:86 IInput::GetKey).
//
// Keyboards of the urna: IInputKbd (voter keypad; web: simulador::CWasmInputKbd fed by
// votaPressKey) and IInputMT (the poll worker's microterminal). Keys are chars:
// '0'..'9', 'B' BRANCO, 'C' CONFIRMA, 'D' CORRIGE.
#pragma once

#include <format>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"

namespace api {

// api::EUeHwilError (typeinfo @1531040); thunk func 1229 builds CBaseError<EUeHwilError>.
enum class EUeHwilError : int { TECLA_DESCONHECIDA = 5169, SEM_CARACTERE = 5170 };   // names inferred
using CUeHwilError = ecourna::api::exception::CBaseError<EUeHwilError>;

class IInput {
public:
    virtual ~IInput() = default;                // slots 0/1
    virtual char DoGetKey() = 0;                // slot 2 (?)  reads one buffered key
    virtual bool HasKey() const = 0;            // slot 3      a key is buffered
    virtual void Flush() = 0;                   // slot 4      (CInteractiveForm::ClearKeyboardInput)

    // iinput.h:86 - always inlined (into IInputField<IScreen>::Read 6319, IInputField<IScreenMT>::Read
    // 11024, 10426 CMostraEleitorVotando::ProcessInput, 11804 ...).
    char GetKey()
    {
        if (!HasKey())
            throw CUeHwilError(EUeHwilError::SEM_CARACTERE, "IInput - Nao havia um caractere disponivel");
        const char tecla = DoGetKey();
        ++m_teclasLidas;
        return tecla;
    }

protected:
    unsigned m_teclasLidas = 0;                 // +4   ?
};

// wasm func 744 - iinput.h:61 (callers: CInteractiveFormBuilder::AddLabeledInputControl,
// vota::CVotacaoStateAudio::PlayKey, the CTelasVota constructor blob 7787, 2243, 12903).
inline std::string KeyName(char tecla)
{
    if (tecla >= '0' && tecla <= '9')
        return std::string(1, tecla);
    switch (tecla) {
    case 'B': return "BRANCO";
    case 'C': return "CONFIRMA";
    case 'D': return "CORRIGE";
    default:
        throw CUeHwilError(EUeHwilError::TECLA_DESCONHECIDA, std::format("Tecla desconhecida ({})", tecla));
    }
}

}  // namespace api

// Reconstructed from vota_web_wasm.wasm (unit u06).
// Original: uenux2/src/app/vota/eleitor/cinstrucaovotacaoacessibilidade.cpp
//
// srcloc evidence:
//   :54          virtual void ProcessInputAudio()        "Número de inputs inválido" (EUeVotaError 9314)
//   :59 / :60    ProcessInputAudio: ISound / ITextToSpeech lookups
//   :129 / :131  virtual void StartStateAudio(): ISound / ITextToSpeech lookups
//   :157 / :161  void CacheInstructionAudio() const        (inlined into the init function 7787)
//   :175         std::string GetKeyboardPosition() const   (IUrna lookup)
//
// All spoken strings are Latin-1 in the binary (e.g. "est\xE1"), the encoding fed to RHVoice.

#include "vota/eleitor/cinstrucaovotacaoacessibilidade.h"

#include "api/hwil/iinputkbd.h"
#include "api/hwil/isound.h"
#include "api/hwil/itexttospeech.h"
#include "api/hwil/iurna.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// wasm func 4420 (analyzer: vota_f4420; no file). Observed executing (called by CEleitorVotando and
// by the start-up function 7787 through CacheInstructionAudio). Lazy singleton; destroyed at exit
// by func 7287 (unique_ptr reset).
CInstrucaoVotacaoAcessibilidade& CInstrucaoVotacaoAcessibilidade::GetInst()
{
    static std::unique_ptr<CInstrucaoVotacaoAcessibilidade> s_inst;   // @1833076, mutex @1833052
    if (!s_inst) {
        s_inst.reset(new CInstrucaoVotacaoAcessibilidade);
        //   : CVotacaoStateAudio(2)
        //   , m_tela(CTelasVota::GetInst().m_telaInstrucoesAcessibilidade)   // CTelasVota +124
    }
    return *s_inst;
}

// wasm func 2466 (slot 0) / 7255 (slot 1, deleting)
CInstrucaoVotacaoAcessibilidade::~CInstrucaoVotacaoAcessibilidade() = default;  // m_tela.reset()

// wasm func 7242 — vtable slot 10, srcloc :129/:131
void CInstrucaoVotacaoAcessibilidade::StartStateAudio()
{
    api::ISound::GetInst().m_volume = 7;                    // :129  ISound +4 (0..10)
    api::ITextToSpeech::GetInst().SetVelocidade(2);         // :131  ITextToSpeech slot 4 (name inferred)
    m_tela->Exibe();                                        // form slot 2
    m_proximoEstado = this;
}

// wasm func 7248 — vtable slot 9, srcloc :54/:59/:60. Called from CVotacaoStateAudio::ProcessInput.
void CInstrucaoVotacaoAcessibilidade::ProcessInputAudio()
{
    if (m_tela->GetInputs().size() != 1)
        throw CUeVotaError(9314, "Número de inputs inválido", std::source_location::current());  // :54

    auto& som = api::ISound::GetInst();                     // :59
    auto& voz = api::ITextToSpeech::GetInst();              // :60
    // the binary takes inputs.at(0) (vector out_of_range check) BEFORE Read(); +55 is read after it
    const auto& input = m_tela->GetInputs().at(0);

    switch (m_tela->Read()) {                               // CInteractiveForm::Read (cinteractiveform.h:57)
    case api::EInputResult::Corrige:                        // 5
        PlayKey(0);                                         // "Tecla indevida pressionada" + error beep
        break;

    case api::EInputResult::Confirma:                       // 9
        m_proximoEstado = nullptr;                          // CEleitorVotando moves to the 1st cargo
        PlayKey('C');
        break;

    case api::EInputResult::Tecla:                          // 13 (a digit was typed) name inferred
        switch (input->GetUltimaTecla()) {                  // input field +55
        case '3':                                           // louder
            if (som.m_volume == 10) {
                PlayMessage("Volume máximo");
            } else {
                ++som.m_volume;
                PlayMessage("Aumentando o volume");
            }
            break;
        case '9':                                           // quieter
            if (som.m_volume == 0) {
                PlayMessage("Volume mínimo");
            } else {
                --som.m_volume;
                PlayMessage("Diminuindo o volume");
            }
            break;
        case '4': {                                         // slower
            const auto antes = voz.GetVelocidade();         // ITextToSpeech slot 5
            voz.DiminuiVelocidade();                        // slot 7
            PlayMessage(voz.GetVelocidade() != antes ? "Fala mais lenta" : "Fala em velocidade mínima");
            break;
        }
        case '6': {                                         // faster
            const auto antes = voz.GetVelocidade();
            voz.AumentaVelocidade();                        // slot 6
            PlayMessage(voz.GetVelocidade() != antes ? "Fala mais rápida" : "Fala em velocidade máxima");
            break;
        }
        default:                                            // 0-2, 5, 7, 8
            PlayKey(0);
            break;
        }
        break;

    default:
        break;
    }
}

// wasm func 4407 — vtable slot 15 (the message that CVotacaoStateAudio speaks after its 1.5 s / 2 s
// ticks). The analyzer named it after the inlined GetKeyboardPosition (srcloc :175).
std::string CInstrucaoVotacaoAcessibilidade::GetMensagemAudio() const
{
    return "A urna está pronta para receber o seu voto. Use o teclado numérico " + GetKeyboardPosition() +
           " da urna para digitar o seu voto. Aperte a tecla 4 para fala mais lenta. Aperte a tecla 6 "
           "para fala mais rápida. Aperte a tecla 3 para aumentar o volume. Aperte a tecla 9 para "
           "diminuir o volume. Aperte confirma para iniciar o seu voto";
}

// srcloc :175 — inlined into func 4407
std::string CInstrucaoVotacaoAcessibilidade::GetKeyboardPosition() const
{
    // IUrna slot 0 = urna model (year). Models after 2019 have the keypad below the screen.
    return api::IUrna::GetInst().GetModelo() > 2019 ? "abaixo" : "à direita";
}

}  // namespace vota

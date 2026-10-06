// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original file: uenux2/src/app/vota/eleitor/comum/cinformacaoeleitor.cpp (unit u02; class name per u02, the
// singleton is wasm func 509). Merge into cinformacaoeleitor.cpp / .h.
//
// m_modoAudio (+4) = mode of the voter's audio guidance (accessibility, headphones):
//   0 = enabled because the voter's registration asks for it ("Áudio ativado conforme cadastro"),
//   1 = enabled (by the mesário / by the web option audioEleitorHabilitado),
//   2 = disabled (initial value). Every audio state tests `m_modoAudio != 2`.
// The three setters are only called by CThreadEleitor::ProcessarMensagens (messages 8, 9, 10) and votaInit.
#include "vota/eleitor/comum/cinformacaoeleitor.h"

namespace vota {

// wasm func 6745  (message 8, posted by the operator state CHabilitaAudioEleitor)    // name inferred
void CInformacaoEleitor::HabilitaAudioConformeCadastro() { m_modoAudio = 0; }

// wasm func 6743  (message 9, posted by CHabilitaAudioEleitor and by votaInit)       // name inferred
void CInformacaoEleitor::HabilitaAudio() { m_modoAudio = 1; }

// wasm func 4195 (unit of the web entry point; message 10 from CDesabilitaAudioEleitor, and called
// directly by votaInit when the page did not ask for voter audio)                    // name inferred
void CInformacaoEleitor::DesabilitaAudio() { m_modoAudio = 2; }

} // namespace vota

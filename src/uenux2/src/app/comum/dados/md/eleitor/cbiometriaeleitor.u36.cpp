// uenux2/src/app/comum/dados/md/eleitor/cbiometriaeleitor.cpp  --  FRAGMENT written by unit u36
// (file owned by u05). Reconstructed from vota_web_wasm.wasm.
//
// Layout correction for cbiometriaeleitor.h: the fingerprints are an OPTIONAL map,
//   +20 std::optional<std::map<CDedo::TipoDedo, CDedo>> m_dedos   (map +20..+32, engaged flag +32)
// which explains "a flag at +32 marks 'has fingers'". GetDedo (wasm 3720, srcloc :93) searches the map
// without testing the flag; this function tests it first.
#include "comum/dados/md/eleitor/cbiometriaeleitor.h"

namespace comum::md {

// wasm func 3719                                                             // name inferred (u10/u27)
// Whether the voter's biometric record holds a template for finger `tipo`. Callers: vota::CPedeDigital::
// ProcessTick (10465, voter identification by fingerprint), comum::CPedeDigitalMesario::ProcessTick
// (10316) and vota::CRegistraDigitalOperador::BiometriaMesarioPresenteNosEleitores (3614). None of them runs
// in the simulator (no scenario ships biometrics).
bool CBiometriaEleitor::PossuiDedo(CDedo::TipoDedo tipo) const
{
    return m_dedos.has_value() && m_dedos->contains(tipo);
}

}  // namespace comum::md

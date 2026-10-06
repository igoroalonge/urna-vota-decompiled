// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralgap.cpp (srcloc cservicoestadogeralgap.cpp:32).
//
// comum::CServicoEstadoGeralGap : comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralGap,
//                                              asn::CConversorEstadoGeralGap>
// The service object is 12 bytes: +0 vptr, +4 int m_midia (0 = MI internal flash, 1 = MV external),
// +8 EUrnaTurno m_turno. Its vtable has three slots: [0]/[1] destructors, [2] GetPathArquivo().
// Reading/writing (IServicoEstado::Carrega / Salva: CFileASN + the BER converter) is in other units.
#include "comum/appinfo/servicos/cservicoestadogeralgap.h"

#include <filesystem>

#include "comum/cpath.h"

namespace comum {

// wasm func 11567 (vtable slot 2). Body = shared helper wasm func 3896 (tools: comum_f3896), called with
// (srcloc cservicoestadogeralgap.cpp:32, error 7605, file name "gap.bin").
std::filesystem::path CServicoEstadoGeralGap::GetPathArquivo()
{
    if (m_turno == EUrnaTurno::SemTurno)                           // '0'
        throw CUeComumAppInfoError(EUeComumAppInfoError{7605},
                                   "Urna sem turno em contexto onde turno era esperado");
    return CPath::GetPathTrab(m_midia, m_turno) / "gap.bin";          // dinamico/trab<turno>/gap.bin
}

} // namespace comum

// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralsa.cpp (srcloc cservicoestadogeralsa.cpp:32).
//
// comum::CServicoEstadoGeralSA : comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralSA,
//                                              asn::CConversorEstadoGeralSA>
// The service object is 12 bytes: +0 vptr, +4 int m_midia (0 = MI internal flash, 1 = MV external),
// +8 EUrnaTurno m_turno. Its vtable has three slots: [0]/[1] destructors, [2] GetPathArquivo().
// Reading/writing (IServicoEstado::Carrega / Salva: CFileASN + the BER converter) is in other units.
#include "comum/appinfo/servicos/cservicoestadogeralsa.h"

#include <filesystem>

#include "comum/cpath.h"

namespace comum {

// wasm func 11565 (vtable slot 2). Body = shared helper wasm func 3896 (tools: comum_f3896), called with
// (srcloc cservicoestadogeralsa.cpp:32, error 7606, file name "sa.bin").
std::filesystem::path CServicoEstadoGeralSA::GetPathArquivo()
{
    if (m_turno == EUrnaTurno::SemTurno)                           // '0'
        throw CUeComumAppInfoError(EUeComumAppInfoError{7606},
                                   "Urna sem turno em contexto onde turno era esperado");
    return CPath::GetPathTrab(m_midia, m_turno) / "sa.bin";          // dinamico/trab<turno>/sa.bin
}

} // namespace comum

// Reconstructed from vota_web_wasm.wasm (unit u20).
// Original: uenux2/src/app/comum/appinfo/servicos/cservicoestadogeralvota.cpp (srcloc cservicoestadogeralvota.cpp:32).
//
// comum::CServicoEstadoGeralVota : comum::IServicoEstado<md::estadoaplicacao::CEstadoGeralVota,
//                                              asn::CConversorEstadoGeralVota>
// The service object is 12 bytes: +0 vptr, +4 int m_midia (0 = MI internal flash, 1 = MV external),
// +8 EUrnaTurno m_turno. Its vtable has three slots: [0]/[1] destructors, [2] GetPathArquivo().
// Reading/writing (IServicoEstado::Carrega / Salva: CFileASN + the BER converter) is in other units.
#include "comum/appinfo/servicos/cservicoestadogeralvota.h"

#include <filesystem>

#include "comum/cpath.h"

namespace comum {

// wasm func 11568 (vtable slot 2). Body = shared helper wasm func 3896 (tools: comum_f3896), called with
// (srcloc cservicoestadogeralvota.cpp:32, error 7607, file name "vota.bin").
std::filesystem::path CServicoEstadoGeralVota::GetPathArquivo()
{
    if (m_turno == EUrnaTurno::SemTurno)                           // '0'
        throw CUeComumAppInfoError(EUeComumAppInfoError{7607},
                                   "Urna sem turno em contexto onde turno era esperado");
    return CPath::GetPathTrab(m_midia, m_turno) / "vota.bin";          // dinamico/trab<turno>/vota.bin
}

} // namespace comum

// uenux2/src/app/comum/appinfo/servicos/cservicoestadogeral.cpp   (path inferred: its three siblings
// cservicoestadogeral{vota,gap,sa}.cpp are attested by srclocs in this directory; this one has none)
// Reconstructed from vota_web_wasm.wasm (unit u35). Class declared in iservicoestado.u29.h (unit u29): the
// "serviço de estado" of eg.bin, the urna's general state (EstadoGeralUrna: fase, turno, carga, local, ...).
//
// RTTI: comum::CServicoEstadoGeral : comum::IServicoEstado<md::estadoaplicacao::CEstadoGeral, asn::CConversorEstadoGeral>
//       vtable @1558024: [0] 174 [1] 144 [2] 11569 GetPathArquivo. sizeof 8 (+4 EFlashOrigem m_midia).
#include "comum/appinfo/servicos/iservicoestado.u29.h"

#include <filesystem>

#include "comum/cpath.h"

namespace comum {

// wasm func 11569 - vtable slot 2. Not in the observed list, although eg.bin is read and written at votaInit through
// this service (Carrega = func 3791, Salva = funcs 3592/2894): a very short function the sampler missed.
// "<flash>/dinamico/eg.bin" of the MI (/dsk/fi) or of the MV (/dsk/fe) - unlike vota.bin / gap.bin / sa.bin it does
// not depend on the turno.
std::filesystem::path CServicoEstadoGeral::GetPathArquivo() const
{
    return CPath::GetPathDinamico(m_midia) / "eg.bin";        // func 1082
}

} // namespace comum

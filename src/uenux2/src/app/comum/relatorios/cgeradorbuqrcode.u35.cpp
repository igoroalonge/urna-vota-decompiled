// FRAGMENT of uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp (attested; owner u25, declarations in
// cgeradorbuqrcode.h). Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/relatorios/cgeradorbuqrcode.h"

namespace comum {

// wasm func 11241 - vtable slot 3 of comum::CGeradorBUQRCodeVota (name as declared by u25). Not observed executing.
// Hook for extra per-eleição data in the QR codes printed at the end of the BU. The voting application (VOTA) has
// none: it returns an empty string (other applications built on the same base class presumably add fields here ?).
std::string CGeradorBUQRCodeVota::DadosEleicao(TEleicaoID /*eleicao*/) const
{
    return {};
}

} // namespace comum

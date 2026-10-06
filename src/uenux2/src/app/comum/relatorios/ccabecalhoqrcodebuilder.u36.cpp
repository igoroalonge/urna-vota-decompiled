// uenux2/src/app/comum/relatorios/ccabecalhoqrcodebuilder.cpp  --  FRAGMENT written by unit u36 (file owned by
// u25; attested by the srcloc :284 of the preBuild lambda, wasm 2791). Reconstructed from vota_web_wasm.wasm.
//
// comum::CCabecalhoQRCodeBuilder holds one comum::CCabecalhoQRCode (396 bytes = 33 std::string fields, each
// holding "TAG:value " with the trailing space; field list in cgeradorbuqrcode.u04-fragment.cpp). The
// setters format one field each: 5618 ZONA, 5620 SECA, 5621 IDUE, 5623 IDCA (other unit) and 5622 below.
// Users: vota::CGeraBU::StartState (12110, QR codes printed on the BU, 1100 characters each) and
// vota::GetQRDSInst (1956, the "BU digital" on the urna screen, 2500 characters each).
#include "comum/relatorios/ccabecalhoqrcodebuilder.h"

#include <cstddef>
#include <format>
#include <string>
#include <vector>

namespace comum {

// wasm func 5624                                                                       // name inferred
// memset(this, 0, 396): the 33 strings start empty (an all-zero libc++ string is a valid empty SSO string).
// preBuild() later refuses to build if a mandatory field is still empty (EUeComumRelatoriosError 9050).
CCabecalhoQRCodeBuilder::CCabecalhoQRCodeBuilder()
    : m_cabecalho{}
{
}

// wasm func 5622                                                                       // name inferred (u19)
// "Histórico de cargas": the codes of every carga (load of the election software/data) this urna received,
// taken from gap.bin (CEstadoGeralGap correspondences). One field (+168, "HistoricoCarga") holds BOTH tags:
//     "HIQT:<n> " followed by n x "HICA:<i>:<first 24 chars of the código de carga> "   (i = 1..n)
// e.g. "HIQT:1 HICA:1:123456789012345678901234 " in the simulator (fixture carga code, 24 digits), or
// "HIQT:0 " when the history is empty. Both numbers are formatted as unsigned (size_t).
void CCabecalhoQRCodeBuilder::SetHistoricoCargas(const std::vector<std::string>& codigosCarga)
{
    m_cabecalho.hica = std::format("HIQT:{} ", codigosCarga.size());                // @440013
    std::size_t sequencial = 0;
    for (const std::string& codigo : codigosCarga)
        m_cabecalho.hica += std::format("HICA:{}:{:.24s} ", ++sequencial, codigo);  // @440426
}

}  // namespace comum

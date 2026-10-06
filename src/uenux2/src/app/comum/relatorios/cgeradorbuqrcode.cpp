// uenux2/src/app/comum/relatorios/cgeradorbuqrcode.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// srclocs: :80  const SQtdeAptos& CGeradorBUQRCode::GetQtdAptos(md::ETipoAbrangencia) const  (in wasm 3696)
//          :405 std::string CGeradorBUQRCode::RetornaBlocoAssinado(const std::string&) const (in wasm 5604)
// Functions of this unit: 2251, 3696, 11243, 11244. The rest of the file (constructor 5603, GeraQRCodes
// 5604 with RetornaBlocoAssinado inlined, TotaisVotosCargo 5606, the ORIG tests 5616/5617/6028, std::sort
// of the answers 5605) is reconstructed in cgeradorbuqrcode.u04-fragment.cpp (unit u04) and documented in
// docs/bu/qrcode.md; it is not repeated here.
// Not executed in the recorded votes.

#include "comum/relatorios/cgeradorbuqrcode.h"

#include <format>

#include "comum/dados/ccargos.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

// wasm func 2251 (vtable slot 0). Destroys m_aptos (comum_f1405 = __tree::destroy) and m_cabecalho
// (comum_f2885 = ~CCabecalhoQRCode: 33 strings).
CGeradorBUQRCode::~CGeradorBUQRCode() = default;

// wasm func 11244 (vtable slot 0 of CGeradorBUQRCodeVota): nothing of its own to destroy
// (m_dataEmissao is trivially destructible), so it just runs ~CGeradorBUQRCode (2251).
// wasm func 11243 (vtable slot 1): deleting destructor = 2251 + operator delete.
CGeradorBUQRCodeVota::~CGeradorBUQRCodeVota() = default;

// srcloc :80 — inlined into 3696. The map holds one entry per abrangência (federal / estadual /
// municipal) filled from CEleitores::GetQtdAptos() by the constructor.
const SQtdeAptos& CGeradorBUQRCode::GetQtdAptos(md::ETipoAbrangencia abrangencia) const
{
    const auto it = m_aptos.find(abrangencia);
    if (it == m_aptos.end())
        throw CRelatoriosError(EUeComumRelatoriosError{9065},
                               std::format("Abrangência não encontrada: {}", abrangencia));   // :80
    return m_aptos.at(abrangencia);             // second lookup (map::at, "map::at:  key not found")
}

// wasm func 3696 (tools: comum::CGeradorBUQRCode::GetQtdAptos, the srcloc it inlines). The "aptos" part
// of every cargo block of the QR payload, for the abrangência of the CURRENT cargo:
//     "APTA:<originais+temporários> APTS:<originais> APTT:<temporários> "
// (all three keys are always written). Callers: GeraQRCodes (5604), TotaisVotosCargo (5606).
// name inferred (u04 calls it AptosCargo)
std::string CGeradorBUQRCode::AptosCargo() const
{
    const auto abrangencia = CCargos::GetInst().GetCurrent().GetAbrangencia();      // CCargo +8
    const SQtdeAptos& aptos = GetQtdAptos(abrangencia);                             // node value +20/+22
    std::string texto;
    texto += std::format("APTA:{} ", static_cast<unsigned short>(aptos.originais + aptos.temporarios));
    texto += std::format("APTS:{} ", aptos.originais);
    texto += std::format("APTT:{} ", aptos.temporarios);
    return texto;
}

} // namespace comum

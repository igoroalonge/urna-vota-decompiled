// FRAGMENT of uenux2/src/app/comum/dados/celeitordetalhe.cpp (attested; file of unit u04, declaration in
// celeitordetalhe.h). Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/celeitordetalhe.h"

#include "comum/appinfo/cappinfo.h"   // GetEstado<CEstadoGeral> (func 457), CAppInfo::GetInst (func 185)

namespace comum {

// wasm func 2266 - name as declared by u04. Not observed executing by the sampler (called for every voter when the
// "aptos" are counted, and by CEleitorEncontrado 10635 / CGravadorRCSecao / the voter-list report).
// A voter is "apto" in the current turno when its impedimento for that turno is 0 (NENHUM): the turno recorded in
// eg.bin (CEstadoGeral +32) selects the 2nd-turno impediment (+200) when it is '2', else the 1st (+196).
bool CEleitorDetalhe::PodeVotar() const
{
    const auto& eg = GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst());
    const md::ETipoImpedimento impedimento = (eg.GetTurno() == '2') ? m_impedimentoP2 : m_impedimentoP1;
    return impedimento == md::ETipoImpedimento::NENHUM;
}

} // namespace comum

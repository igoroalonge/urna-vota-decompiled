// uenux2/src/app/comum/asn/cconversorcabecalhoentidade.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/md/ccabecalhoentidade.h"   // md::CCabecalhoEntidade (20 bytes): +0 api::CDateTime dataGeracao (12),
                                           // +12 uedword id, +16 ETipoCabecalho tipo (0 processo, 1 pleito, 2 eleição)
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

// CabecalhoEntidade = the header of every TSE data/result file (BU, RDV, hashes, candidates, ...):
//   CabecalhoEntidade ::= SEQUENCE { dataGeracao DataHoraJE ("YYYYMMDDTHHMMSS"),
//                                    idEleitoral IDEleitoral CHOICE { [1] idProcessoEleitoral, [2] idPleito, [3] idEleicao } }
// RTTI: CConversorCabecalhoEntidade : IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, md::CCabecalhoEntidade>
// vtable @1556740: [0] 174 [1] 144 [2] 11589 DoConverte [3] 11588 DoDesconverte. sizeof 4.
// Built on the stack by 8 functions (BU, hashes, parties, candidates, eleitores, federações, ...).
class CConversorCabecalhoEntidade
    : public IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, md::CCabecalhoEntidade>
{
protected:
    TEntidade DoConverte(const TDado& cabecalho) const override;        // wasm func 11589 (srcloc line 35)
    md::CCabecalhoEntidade DoDesconverte(const TEntidade& cabecalho) const override;   // wasm func 11588 (line 56)
};

} // namespace comum::asn

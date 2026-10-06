// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeralsa.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralsa.h"
#include "ModuloEstadoGeralSA.h"

namespace comum::asn {

// RTTI: CConversorEstadoGeralSA : IConversorASN<ModuloEstadoGeralSA::EstadoGeralSA, md::estadoaplicacao::CEstadoGeralSA>
// vtable @1568732: [0] 174 [1] 144 [2] 11392 DoConverte [3] 11393 DoDesconverte
//
// md::estadoaplicacao::CEstadoGeralSA (16 bytes), from its constructor (func 5625):
//   +0 EEstadoSA estado (char '1'..'G')  +4 bool atualizacaoBloqueada  +8 int municipio  +12 short zona
//   +14 short secao
class CConversorEstadoGeralSA
    : public IConversorASN<ModuloEstadoGeralSA::EstadoGeralSA, md::estadoaplicacao::CEstadoGeralSA>
{
public:
    ModuloEstadoGeralSA::EstadoSA ConverteEstadoSA(const EEstadoSA estado) const;        // inlined in 11392
    EEstadoSA DesconverteEstadoSA(const ModuloEstadoGeralSA::EstadoSA& estado) const;    // inlined in 11393

protected:
    TEntidade DoConverte(const TDado& estado) const override;       // wasm func 11392
    TDado DoDesconverte(const TEntidade& estado) const override;    // wasm func 11393
};

} // namespace comum::asn

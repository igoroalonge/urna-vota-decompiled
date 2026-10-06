// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversorestadogeral.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cestadogeral.h"
#include "ModuloEstadoGeralUrna.h"

namespace comum::asn {

// RTTI: CConversorEstadoGeral : IConversorASN<ModuloEstadoGeralUrna::EstadoGeralUrna, md::estadoaplicacao::CEstadoGeral>
// vtable @1568180: [0] 174 [1] 144 [2] 11397 DoConverte [3] 11398 DoDesconverte
// Used through comum::IServicoEstado<CEstadoGeral, CConversorEstadoGeral> (RTTI) to read/write dinamico/eg.bin.
//
// md::estadoaplicacao::CEstadoGeral (180 bytes), from its constructor (func 5637) and DoConverte:
//   +0   EEstadoUrna estado (char '1'..'4')          +4   TPEID idPE
//   +8   CDadoLocal local (24: std::string uf + CLocalidadeEleitoral secaoCarga (12) ...)
//   +32  CDadoCarga carga (20)                       +52  CAjusteDataHora ajuste (8)
//   +60  CDadoCorrespondencia correspondencia (96)   +156 std::string versao
//   +168 std::vector<uebyte> hashVersoesPacotes
class CConversorEstadoGeral
    : public IConversorASN<ModuloEstadoGeralUrna::EstadoGeralUrna, md::estadoaplicacao::CEstadoGeral>
{
public:
    EEstadoUrna DesconverteEstadoUrna(const ModuloEstadoGeralUrna::EstadoUrna& estado) const;   // inlined in 11398
    ModuloEstadoGeralUrna::EstadoUrna ConverteEstadoUrna(const EEstadoUrna estado) const;       // inlined in 11397

protected:
    TEntidade DoConverte(const TDado& estado) const override;       // wasm func 11397
    TDado DoDesconverte(const TEntidade& estado) const override;    // wasm func 11398
};

} // namespace comum::asn

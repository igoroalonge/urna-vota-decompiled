// uenux2/src/app/comum/dados/asn/estadoaplicacao/cconversordadocarga.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/estadoaplicacao/cdadocarga.h"
#include "ModuloEstadoGeralUrna.h"
#include "ModuloTiposEcoUrna.h"

namespace comum::asn {

// RTTI: CConversorDadoCarga : IConversorASN<ModuloEstadoGeralUrna::DadoCarga, md::estadoaplicacao::CDadoCarga>
// vtable @1567836: [0] 174 [1] 144 [2] 11401 DoConverte [3] 11402 DoDesconverte
//
// md::estadoaplicacao::CDadoCarga (20 bytes):
//   +0 EUrnaTurno turno (char '0'..'2')   +4 EUrnaTipo tipoT1 (char '0'..'4')   +8 EUrnaTipo tipoT2
//   +12 EUrnaModelo modelo (the model YEAR: 2013, 2015, 2020, 2022)            +16 EUrnaFase fase
class CConversorDadoCarga
    : public IConversorASN<ModuloEstadoGeralUrna::DadoCarga, md::estadoaplicacao::CDadoCarga>
{
public:
    ModuloEstadoGeralUrna::TipoUrnaOperacao ConverteTipoUrna(const EUrnaTipo tipo) const;                 // wasm func 5693
    EUrnaTipo DesconverteTipoUrna(const ModuloEstadoGeralUrna::TipoUrnaOperacao& tipo) const;             // wasm func 5694
    ModuloTiposEcoUrna::ModeloEquipamento ConverteModelo(const EUrnaModelo modelo) const;                 // inlined in 11401
    EUrnaModelo DesconverteModelo(const ModuloTiposEcoUrna::ModeloEquipamento& modelo) const;             // inlined in 11402

protected:
    TEntidade DoConverte(const TDado& carga) const override;       // wasm func 11401
    TDado DoDesconverte(const TEntidade& carga) const override;    // wasm func 11402
};

} // namespace comum::asn

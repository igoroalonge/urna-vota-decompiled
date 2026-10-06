// uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.h   (path inferred; md class in comum/dados/md/cpartido.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/cpartido.h"   // md::CPartido (28 bytes): +0 uint16 numero, +4 sigla, +16 nome
                                       // md::CEntidadePartidos (52): +0 CCabecalhoEntidade (20), +20 CAbrangencia (20),
                                       //                            +40 std::vector<CPartido>
#include "ModuloPartidos.h"

namespace comum::asn {

// Parties file <...>-pa.dat:
//   EntidadePartidos ::= SEQUENCE { cabecalho CabecalhoEntidade, abrangencia Abrangencia, partidos SEQUENCE OF Partido }
//   Partido ::= SEQUENCE { numero INTEGER (0..99), sigla GeneralString (SIZE(1..24)), nome GeneralString (SIZE(1..55)) }
//
// RTTI: CConversorEntidadePartidos : IConversorASN<ModuloPartidos::EntidadePartidos, md::CEntidadePartidos>
// vtable @1561344: [0] 174 [1] 144 [2] 11458 DoConverte [3] 11457 DoDesconverte. sizeof 4.
class CConversorEntidadePartidos
    : public IConversorASN<ModuloPartidos::EntidadePartidos, md::CEntidadePartidos>
{
protected:
    TEntidade DoConverte(const TDado& partidos) const override;      // wasm func 11458
    TDado DoDesconverte(const TEntidade& partidos) const override;   // wasm func 11457
};

// RTTI: CConversorPartido : IConversorASN<ModuloPartidos::Partido, md::CPartido>
// vtable @1561268: [0] 174 [1] 144 [2] 11460 DoConverte [3] 11459 DoDesconverte (both other unit). sizeof 4.
class CConversorPartido : public IConversorASN<ModuloPartidos::Partido, md::CPartido>
{
protected:
    TEntidade DoConverte(const TDado& partido) const override;       // wasm func 11460 (other unit)
    TDado DoDesconverte(const TEntidade& partido) const override;    // wasm func 11459 (other unit)
};

} // namespace comum::asn

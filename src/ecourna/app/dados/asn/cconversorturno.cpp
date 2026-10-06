// ecourna-lib/ecourna/app/dados/asn/cconversorturno.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversorturno.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   Turno ::= ENUMERATED { semTurno(0), turno1(1), turno2(2) }  <->  TTurno = CBaseType<ushort, 1, 2, 11>
// semTurno is rejected in both directions. This is the only file of the unit whose literal is
// UTF-8 ("Turno inv\xC3\xA1lido."); the other files of ecourna/app/dados use ISO-8859-1.
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9192 (vtable slot 2; srcloc line 27)
ModuloTiposEleitorais::Turno CConversorTurno::DoConverte(const TDado& turno) const
{
    switch (static_cast<unsigned short>(turno)) {
    case 1: return ModuloTiposEleitorais::Turno(ModuloTiposEleitorais::Turno::turno1);
    case 2: return ModuloTiposEleitorais::Turno(ModuloTiposEleitorais::Turno::turno2);
    }
    throw CAsnError(2252, "Turno inválido.");   // line 27
}

// wasm func 9191 (vtable slot 3; srcloc line 41)
TTurno CConversorTurno::DoDeconverte(const TEntidade& turno) const
{
    switch (turno.asInt()) {
    case ModuloTiposEleitorais::Turno::turno1: return TTurno(1);
    case ModuloTiposEleitorais::Turno::turno2: return TTurno(2);
    }
    throw CAsnError(2253, "Turno inválido.");   // line 41
}

} // namespace ecourna::app::dados::asn

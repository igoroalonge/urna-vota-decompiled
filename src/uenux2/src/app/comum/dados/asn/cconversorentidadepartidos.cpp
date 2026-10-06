// uenux2/src/app/comum/dados/asn/cconversorentidadepartidos.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// Both methods were named by the tool after an inlined IConversorASN method (Abrangencia::Converte :56,
// Partido::Converte :56 / Partido::Desconverte :71); the vtable slots of CConversorEntidadePartidos give the real names.
#include "comum/dados/asn/cconversorentidadepartidos.h"

#include "comum/asn/cconversorabrangencia.h"
#include "comum/asn/cconversorcabecalhoentidade.h"

namespace comum::asn {

// wasm func 11458 (vtable slot 2). Not observed executing (parties are never written by the urna).
ModuloPartidos::EntidadePartidos CConversorEntidadePartidos::DoConverte(const md::CEntidadePartidos& partidos) const
{
    ModuloPartidos::EntidadePartidos entidade;
    entidade.set_cabecalho(CConversorCabecalhoEntidade().Converte(partidos.GetCabecalho()));   // thunk 2275
    entidade.set_abrangencia(CConversorAbrangencia().Converte(partidos.GetAbrangencia()));    // inlined (:56)
    ASN1::SEQUENCE_OF<ModuloPartidos::Partido> lista;
    for (const md::CPartido& partido : partidos.GetPartidos()) {
        lista.push_back(CConversorPartido().Converte(partido));                               // inlined (:56)
    }
    entidade.set_partidos(lista);
    return entidade;
}

// wasm func 11457 (vtable slot 3). Observed executing (parties file read at votaInit).
md::CEntidadePartidos CConversorEntidadePartidos::DoDesconverte(const ModuloPartidos::EntidadePartidos& partidos) const
{
    const md::CCabecalhoEntidade cabecalho = CConversorCabecalhoEntidade().Desconverte(partidos.get_cabecalho()); // 3738
    const md::CAbrangencia abrangencia = CConversorAbrangencia().Desconverte(partidos.get_abrangencia());         // 5721
    std::vector<md::CPartido> lista;
    for (const auto* item : partidos.get_partidos()) {
        // The wasm COPIES the converted CPartido into the vector (both strings copy-constructed, then the local is
        // destroyed) although vector growth moves elements: the source pushes a named (lvalue) local.
        const md::CPartido partido = CConversorPartido().Desconverte(*item);               // inlined (:71)
        lista.push_back(partido);
    }
    return md::CEntidadePartidos(cabecalho, abrangencia, lista);                             // member-wise, inline
}

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.cpp   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u35).
#include "comum/dados/asn/candidatura/cindexadorfotos.h"

#include <vector>

#include "comum/asn/cconversorabrangencia.h"   // vtable @1561128

namespace comum::asn {

// wasm func 11441 - vtable slot 2. Not in the observed list (the photo files are indexed at start-up by the loader
// func 7787 through the inlined IConversorParcialASN<...>::Desconverte; the sampler missed this call).
// The visitor's list {codigoCandidato, {início, fim}} (20-byte elements at visitante +16) is copied element by
// element into md::CIndiceFoto objects (func 5659); only the município of the decoded Abrangencia is kept (+16 of
// md::CAbrangencia).                                                                                   ?
md::CIndicesFotos CIndexadorFotos::DoDesconverte(const ModuloFotosCandidatos::EntidadeFotosCandidatos& entidade,
                                                 const CVisitanteFoto& visitante) const
{
    std::vector<md::CIndiceFoto> indices;
    for (std::size_t i = 0; i < visitante.GetIndices().size(); ++i) {
        const auto& [codigo, posicao] = visitante.GetIndices()[i];
        indices.emplace_back(codigo, posicao);                                            // func 5659
    }
    const md::CAbrangencia abrangencia = CConversorAbrangencia().Desconverte(entidade.get_abrangencia());  // 5721
    return md::CIndicesFotos(abrangencia.GetMunicipio(), indices);                        // {+0 int, +4 vector copy}
}

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/candidatura/cindexadorfotos.h   (path inferred: RTTI only; its visitor
// CVisitanteFoto lives in the attested dados/asn/candidatura/cvisitantefoto.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
// The candidate photo file "<fase><eleição:05><uf><município:05>-fo.dat" (ModuloFotosCandidatos::
// EntidadeFotosCandidatos {cabecalho, abrangencia, fotos SEQUENCE OF FotoCandidato {codigoCandidato, foto}})
// is large, so the urna does not decode it at start-up. The PARTIAL converter decodes the file with an
// ASN1::IAbstractVisitor (CVisitanteFoto, unit u03) that only records, for every FotoCandidato, the candidate code
// and the [início, fim) byte range; the image of a candidate is decoded later, on demand, by seeking into the file
// (CFotos::GetImagem, func 3755).
//
// RTTI: comum::asn::CIndexadorFotos
//         : comum::asn::IConversorParcialASN<ModuloFotosCandidatos::EntidadeFotosCandidatos, comum::md::CIndicesFotos,
//                                            comum::asn::CVisitanteFoto>
//       vtable @1562548: [0] 174 [1] 144 [2] 11441 DoDesconverte(entidade, visitante). sizeof 4.
#pragma once

#include "comum/asn/iconversorparcialasn.h"
#include "comum/dados/asn/candidatura/cvisitantefoto.h"
#include "comum/dados/md/candidatura/cindicesfotos.h"     // md::CIndicesFotos, md::CIndiceFoto (cindicesfotos.cpp:30)
#include "ModuloFotosCandidatos.h"

namespace comum::asn {

class CIndexadorFotos
    : public IConversorParcialASN<ModuloFotosCandidatos::EntidadeFotosCandidatos, md::CIndicesFotos, CVisitanteFoto>
{
protected:
    TDado DoDesconverte(const TEntidade& entidade, const TVisitor& visitante) const override;   // wasm func 11441
};

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/candidatura/cconversorcandidaturas.h   (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u21).
#pragma once

#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/candidatura/ccandidatura.h"
#include "ModuloCandidatos.h"

namespace comum::asn {

// Candidates file <...>-ca.dat:
//   EntidadeCandidatos ::= SEQUENCE { cabecalho, abrangencia, cargos SEQUENCE OF CandidatosPorCargos {
//       cargo CodigoCargoConsulta, quantidadeVagas, partidos SEQUENCE OF CandidatoPorPartido {
//           partido INTEGER (0..99), candidatosAptos SEQUENCE OF Candidatura, candidatosInaptos SEQUENCE OF Candidatura
//       } OPTIONAL } }
// RTTI: CConversorCandidaturas : IConversorASN<ModuloCandidatos::EntidadeCandidatos, std::vector<md::CCandidatura>>
// vtable @1562368: [0] 174 [1] 144 [2] 11444 (base DoConverte, throws 7655) [3] 11445 DoDesconverte. sizeof 4.
class CConversorCandidaturas
    : public IConversorASN<ModuloCandidatos::EntidadeCandidatos, std::vector<md::CCandidatura>>
{
protected:
    TDado DoDesconverte(const TEntidade& entidade) const override;   // wasm func 11445
};

} // namespace comum::asn

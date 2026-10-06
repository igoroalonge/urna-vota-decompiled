// uenux2/src/app/comum/dados/cintegridadereferencial.cpp  --  FRAGMENT written by unit u02 (owner u04)
//
// Referential-integrity checks of the static election data. A check returns
//   struct TResultado { bool ok; std::string erro; };        (16 bytes: +0 ok, +4 string)
// and CIntegridadeReferencial::Lanca(const TResultado&) (wasm 2263, srcloc line 344) throws when
// ok is false. Names inferred.

#include "comum/dados/cintegridadereferencial.h"

#include <format>

namespace comum {

// wasm func 5747                                                       // name inferred
// Office of a candidacy: must exist in `cargos`, must be a candidate office and must have the same
// number of alternates. `cargoEncontrado` receives the map ITERATOR (the tree node; end() when the
// office is missing: the binary stores the end node `c + 4`, not nullptr). The photo check in
// 7787 reads the CCargo through it (node + 20).
CIntegridadeReferencial::TResultado
CIntegridadeReferencial::VerificaCargoCandidatura(const md::CCandidatura& candidatura,
                                                  const std::map<TCargoID, md::CCargo>& cargos,
                                                  std::map<TCargoID, md::CCargo>::const_iterator& cargoEncontrado)
{
    const auto it = cargos.find(candidatura.GetCargo());               // byte +0 of the candidacy
    cargoEncontrado = it;
    if (it == cargos.end())
        return {false, std::format("o cargo ({}) da candidatura ({}) não foi encontrado",
                                   candidatura.GetCargo(), candidatura.GetNumero())};
    if (!it->second.TemDetalheCandidato())                     // optional engaged flag, CCargo +84 (node +104)
        return {false, std::format("o cargo ({}) da candidatura ({}) não é de candidato",
                                   candidatura.GetCargo(), candidatura.GetNumero())};
    const auto& detalhe = it->second.GetDetalheCandidato();                 // wasm 1388
    // compares the byte lengths of two vectors: detail +52/+56 vs candidacy +60/+64
    if (detalhe.GetSuplentes().size() != candidatura.GetSuplentes().size())
        return {false, std::format("o cargo ({}) e a candidatura ({}) não têm a mesma quantidade de suplentes",
                                   candidatura.GetCargo(), candidatura.GetNumero())};
    return {true, {}};
}

// wasm func 5746                                                       // name inferred
// Photo of the candidate (indice 0) or of alternate `indice`: must exist in the photo index,
// unless the candidacy has no photo (+52 != 0).
CIntegridadeReferencial::TResultado
CIntegridadeReferencial::VerificaFotoCandidatura(const md::CCandidatura& candidatura, uebyte indice,
                                                 const std::map<std::string, CIndiceFoto>& fotos)
{
    if (candidatura.SemFoto())                                             // int +52
        return {true, {}};
    const std::string& nome = indice ? candidatura.GetSuplente(indice).GetFoto()   // wasm 1389
                                     : candidatura.GetFoto();                       // +8
    if (fotos.find(nome) == fotos.end())
        return {false, "a foto (" + nome + ")"
                       + std::format(" da candidatura ({}[{}]) não foi encontrada",
                                     candidatura.GetNumero(), indice)};
    return {true, {}};
}

// wasm func 2543                                                       // name inferred
// Integrity of the dynamic data (RDV) against the static data; used by CEleitores::CompleteLoad
// (wasm 6734), vota::impl::CSincronismoVotoEleitor and vota::CGravaResultado.
void CIntegridadeReferencial::VerificaRdv()
{
    auto& rdv = CRdvVota::GetInst();                                       // wasm 555
    const auto cargos = CCargos::GetInst().GetMapaCargos();                // wasm 2834
    // wasm 11514 (6.8 KB, "um voto do cargo ({}) no RDV ..."): a check that RETURNS a TResultado
    // (16 bytes written to the caller's slot), not a constructor
    Lanca(VerificaVotosRdv(rdv, cargos, CCandidaturas::GetInst(), CPartidos::GetInst(),
                           CRespostas::GetInst()));                        // name inferred
}

} // namespace comum

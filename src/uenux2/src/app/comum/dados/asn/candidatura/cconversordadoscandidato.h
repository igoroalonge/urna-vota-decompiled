// uenux2/src/app/comum/dados/asn/candidatura/cconversordadoscandidato.h   (path inferred: RTTI only; its md class is
// the attested dados/md/candidatura/cdadoscandidato.cpp)
// Reconstructed from vota_web_wasm.wasm (unit u35).
//
//   DadosCandidato ::= SEQUENCE { codigo [1], candidatoSubstituido [2] OPTIONAL, nome, nomeUrna, nomeSocial OPTIONAL,
//                                 nomeFonetico [3] OPTIONAL, dataNascimento, sexo CodigoSexo, situacao,
//                                 situacaoCassacao [4] OPTIONAL, partido, ordemSuplencia [5] INTEGER (1..9) OPTIONAL,
//                                 reeleicao BOOLEAN }
// md::CDadosCandidato (52 bytes): +0 std::string codigo, +12 std::string nomeUrna,
//   +24 std::optional<std::string> nomeFonetico (flag +36), +40 CSexo::ESexo sexo, +44 ESituacao situacao,
//   +48 uebyte ordemSuplencia (0 = titular).
// Only these fields reach the urna. The ASN.1 nome, nomeSocial, dataNascimento, situacao, situacaoCassacao, partido,
// reeleicao and candidatoSubstituido are NOT read: the "apto / inapto" status comes from the list the candidacy was
// found in (candidatosAptos / candidatosInaptos of CandidatosPorCargos), passed to the converter's constructor.
//
// RTTI: comum::asn::CConversorDadosCandidato : IConversorASN<ModuloCandidatos::DadosCandidato, md::CDadosCandidato>
//       vtable @1562232: [2] 11449 default DoConverte (throws 7655) [3] 11450 DoDesconverte. sizeof 8.
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/candidatura/cdadoscandidato.h"
#include "ModuloCandidatos.h"

namespace comum::asn {

class CConversorDadosCandidato : public IConversorASN<ModuloCandidatos::DadosCandidato, md::CDadosCandidato>
{
public:
    explicit CConversorDadosCandidato(bool apto) : m_apto(apto) {}   // inlined in 11448   name inferred

protected:
    TDado DoDesconverte(const TEntidade& dados) const override;       // wasm func 11450

private:
    bool m_apto;                                                        // +4
};

} // namespace comum::asn

// uenux2/src/app/comum/dados/asn/eleitor/cconversorbiometriaeleitorcifrada.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <vector>

#include "comum/asn/iconversorbiometriaasn.h"
#include "comum/dados/md/cparametro.h"
#include "comum/dados/md/eleitor/cbiometriaeleitor.h"
#include "ModuloEleitores.h"

namespace comum::asn {

// RTTI: CConversorBiometriaEleitorCifrada
//         : IConversorBiometriaASN<ModuloEleitores::BiometriaEleitorCifrada, md::CBiometriaEleitor>
// vtable @1566300: [0] 174 [1] 144
//   [2] 11418 IConversorBiometriaASN::DoConverte(const TDado&, const md::CParametro&) — base default that
//       throws "Método DoConverte() não implementado para {}" (iconversorbiometriaasn.h:81): the urna never
//       re-encrypts biometrics.
//   [3] 11419 DoDesconverte(const TEntidade&, const md::CParametro&)
// IConversorBiometriaASN::Desconverte(entidade, parametro) validates the entity first (iconversorbiometriaasn.h:62).
//
// md::CParametro (as used here): +0 std::string (per-voter "info" that goes into the CEPESC salt, ?),
//                                +16 std::vector<uebyte> chave de sessão.
class CConversorBiometriaEleitorCifrada
    : public IConversorBiometriaASN<ModuloEleitores::BiometriaEleitorCifrada, md::CBiometriaEleitor>
{
public:
    std::vector<uebyte> LeChave() const;   // inlined into 11419 (srcloc lines 91, 101, 109)

protected:
    TDado DoDesconverte(const TEntidade& biometria, const md::CParametro& parametro) const override;   // wasm func 11419
};

} // namespace comum::asn

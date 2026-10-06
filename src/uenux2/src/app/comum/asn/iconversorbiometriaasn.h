// uenux2/src/app/comum/asn/iconversorbiometriaasn.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// Variant of IConversorASN for the encrypted fingerprint templates of the voters (ModuloEleitores::
// BiometriaEleitorCifrada inside the voter file). Both directions take an extra md::CParametro: the per-voter
// material needed to (de)cipher the templates (CEPESC key/salt, see cconversorbiometriaeleitorcifrada.h, unit u03).
// Only one instantiation exists: <ModuloEleitores::BiometriaEleitorCifrada, comum::md::CBiometriaEleitor>,
// subclass comum::asn::CConversorBiometriaEleitorCifrada (vtable @1566300).
//
// srcloc lines: Desconverte :62 (inlined into CEleitorDetalhe::GetBiometria, wasm 1937, unit u03/u04),
//               DoConverte  :81 (wasm 11418, this unit).
#pragma once

#include <format>
#include <sstream>
#include <typeinfo>

#include "comum/asn/iconversorasn.h"     // CUeComumAsnError, EUeComumAsnError
#include "comum/dados/md/cparametro.h"   // comum::md::CParametro

namespace comum::asn {

// vtable: [0] ~T  [1] deleting ~T  [2] DoConverte(const TDado&, const CParametro&)
//         [3] DoDesconverte(const TEntidade&, const CParametro&)
template <typename ENTIDADE, typename DADO>
class IConversorBiometriaASN
{
public:
    using TEntidade = ENTIDADE;
    using TDado = DADO;

    virtual ~IConversorBiometriaASN() = default;

    // Inlined into wasm 1937 (CEleitorDetalhe::GetBiometria); shown here for completeness.
    TDado Desconverte(const TEntidade& entidade, const md::CParametro& parametro) const
    {
        if (!entidade.isValid() || !entidade.isStrictlyValid()) {
            std::ostringstream erro;
            ASN1::trace_invalid(erro, typeid(TEntidade).name(), entidade);
            throw CUeComumAsnError(EUeComumAsnError(7658),
                                   std::format("Entidade está inválida: {}", erro.str()));             // line 62
        }
        return DoDesconverte(entidade, parametro);
    }

protected:
    // wasm func 11418 (vtable CConversorBiometriaEleitorCifrada slot 2; srcloc line 81).
    // The urna never re-encrypts fingerprints, so the subclass does not override it.
    // The body is the merged-body pattern of func 731 but kept as a separate function (different code 7659).
    virtual TEntidade DoConverte(const TDado& /*dado*/, const md::CParametro& /*parametro*/) const
    {
        throw CUeComumAsnError(EUeComumAsnError(7659),
            std::format("Método DoConverte() não implementado para {}", typeid(TEntidade).name()));    // line 81
    }

    // Overridden by CConversorBiometriaEleitorCifrada::DoDesconverte (wasm 11419, unit u03). The base default is not
    // present in the binary (probably pure virtual or never instantiated).   // ?
    virtual TDado DoDesconverte(const TEntidade& entidade, const md::CParametro& parametro) const = 0;   // ?
};

} // namespace comum::asn

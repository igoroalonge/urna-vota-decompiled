// uenux2/src/app/comum/dados/asn/rdv/cconversorvoto.h
// Reconstructed from vota_web_wasm.wasm (unit u03). See docs/modules/u03-uenux2-src-app-comum-dados.md.
//
// Conventions shared by every file of this unit (the declarations live in other units' files):
//  * comum::asn::IConversorASN<ENTIDADE, DADO>  (uenux2/src/app/comum/asn/iconversorasn.h)
//      TEntidade Converte(const TDado&) const     -> DoConverte(), then requires isValid() && isStrictlyValid()
//                                                    else CBaseError<EUeComumAsnError>(7653,
//                                                    "Entidade deixada em estado inválido: {}")   (line 56)
//      TDado Desconverte(const TEntidade&) const  -> requires isValid() && isStrictlyValid() else
//                                                    CBaseError<EUeComumAsnError>(7654, "Entidade está inválida: {}")
//                                                    (line 71), then DoDesconverte()
//      vtable: [0] ~dtor  [1] deleting dtor  [2] DoConverte  [3] DoDesconverte
//  * CDadosError(code, message [, std::source_location = current()]) is
//      ecourna::api::exception::CBaseError<comum::EUeComumDadosError, SErrorLimits{7800, 8600}>;
//    the numeric code is the EUeComumDadosError enumerator value (enumerator names are not in the binary).
//  * ASN.1 classes are III ASN.1 generated code (docs/libraries/asn1-runtime.md): get_x()/ref_x()/set_x(),
//    x_isPresent() (= SEQUENCE::hasOptionalField), omit_x() (= removeOptionalField),
//    CHOICE::currentSelection(), ENUMERATED::asInt().
#pragma once

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/rdv/cvoto.h"
#include "ModuloRegistroDigitalVoto.h"

namespace comum::asn {

// RTTI: comum::asn::CConversorVoto : IConversorASN<ModuloRegistroDigitalVoto::Voto, comum::md::CVoto>
// vtable @1570816: [0] 174 (trivial dtor) [1] 144 (operator delete) [2] 11355 DoConverte [3] 11354 DoDesconverte
// Object: +0 vptr only (4 bytes).
class CConversorVoto : public IConversorASN<ModuloRegistroDigitalVoto::Voto, md::CVoto>
{
public:
    static ModuloRegistroDigitalVoto::TipoVoto ConverteTipo(md::CVoto::ETipo tipo);             // inlined into 11355
    static md::CVoto::ETipo DesconverteTipo(ModuloRegistroDigitalVoto::TipoVoto tipo);         // inlined into 11354

protected:
    TEntidade DoConverte(const TDado& voto) const override;          // wasm func 11355
    TDado DoDesconverte(const TEntidade& voto) const override;       // wasm func 11354
};

} // namespace comum::asn

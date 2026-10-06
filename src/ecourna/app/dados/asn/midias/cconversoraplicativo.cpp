// ecourna-lib/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/midias/cconversoraplicativo.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   Aplicativo ::= SEQUENCE { tipoAplicativo TipoAplicativo {vota(1), sa(2), red(3), vpp(4), ste(5),
//                             adh(6), atue(7)}, autenticacao Autenticacao OPTIONAL }  <->  CAplicativo
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/midias/cconversorautenticacao.h"   // unit u40/u11
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9198 (srcloc line 67): thunk into the shared body func 6151 (count 7): identity mapping
// with range check [1, 7]. (The decompiler annotated the code 2485 with an unrelated string.)
ModuloInformacaoMidia::TipoAplicativo CConversorAplicativo::ConverteTipoAplicativo(CAplicativo::ETipoAplicativo tipo) const
{
    if (tipo < CAplicativo::Vota || tipo > CAplicativo::ATUE) {
        throw CAsnMidiasError(2485, "Aplicativo inválido");   // line 67
    }
    return ModuloInformacaoMidia::TipoAplicativo(static_cast<ModuloInformacaoMidia::TipoAplicativo::NamedNumber>(tipo));
}

// inlined into func 9196 (srcloc lines 88, 92). Both throws use the same code 2486.
CAplicativo::ETipoAplicativo CConversorAplicativo::DeconverteTipoAplicativo(ModuloInformacaoMidia::TipoAplicativo tipo) const
{
    switch (tipo.asInt()) {
    case ModuloInformacaoMidia::TipoAplicativo::vota:
    case ModuloInformacaoMidia::TipoAplicativo::sa:
    case ModuloInformacaoMidia::TipoAplicativo::red:
    case ModuloInformacaoMidia::TipoAplicativo::vpp:
    case ModuloInformacaoMidia::TipoAplicativo::ste:
    case ModuloInformacaoMidia::TipoAplicativo::adh:
    case ModuloInformacaoMidia::TipoAplicativo::atue:
        return static_cast<CAplicativo::ETipoAplicativo>(tipo.asInt());
    case -1:   // ? explicit "invalid" enumerator (see cconversorfase.cpp)
        throw CAsnMidiasError(2486, "Aplicativo inválido");   // line 88
    }
    throw CAsnMidiasError(2486, "Aplicativo inválido");       // line 92
}

// wasm func 9199 (vtable slot 2)
CConversorAplicativo::TEntidade CConversorAplicativo::DoConverte(const TDado& aplicativo) const
{
    ModuloInformacaoMidia::Aplicativo entidade;
    entidade.set_tipoAplicativo(ConverteTipoAplicativo(aplicativo.GetTipo()));
    if (aplicativo.PossuiAutenticacao()) {
        const CConversorAutenticacao conversor;
        entidade.set_autenticacao(conversor.Converte(aplicativo.GetAutenticacao()));   // includeOptionalField(0, 1)
    } else {
        entidade.omit_autenticacao();                                                 // removeOptionalField(0)
    }
    return entidade;
}

// wasm func 9196 (vtable slot 3; the tool named it after the inlined DeconverteTipoAplicativo;
// it also carries the srcloc iconversorasn.hpp:66 of the inlined CConversorAutenticacao::Deconverte)
CConversorAplicativo::TDado CConversorAplicativo::DoDeconverte(const TEntidade& aplicativo) const
{
    const auto tipo = DeconverteTipoAplicativo(aplicativo.get_tipoAplicativo());
    if (aplicativo.autenticacao_isPresent()) {
        const CConversorAutenticacao conversor;
        return CAplicativo(tipo, conversor.Deconverte(aplicativo.get_autenticacao()));   // func 9043
    }
    return CAplicativo(tipo);                                                         // inlined
}

} // namespace ecourna::app::dados::asn

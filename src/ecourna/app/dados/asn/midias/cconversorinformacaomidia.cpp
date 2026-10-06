// ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   InformacaoMidia ::= SEQUENCE { tipoMidia TipoMidia {mr(1), fc(2), fv(3)}, fase Fase,
//        idPE INTEGER (0..99999), uf SiglaUF, turno Turno, dadosGeracaoMidia DadosGeracaoMidia,
//        aplicativos SEQUENCE OF Aplicativo OPTIONAL }  <->  CInformacaoMidia
// The file "infomidia.dat" of a medium. Read by comum::CGravadorBU / CGravadorRDV /
// IGravadorEnvelope (GravaResultado, vf7) through api::CFileASN::ReadFromFile<InformacaoMidia>
// (func 3735, the only place that builds this converter), which calls Deconverte.
// comum::impl::CValidaMidia (func 11172) only uses the names "infomidia.dat"/"infomidia.vsc"
// (signature check); it does not decode the file with this converter.
#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/asn/midias/cconversordadosgeracaomidia.h"   // unit u40
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::api::asn {

// wasm func 1970 (no srcloc; attributed to this file by the tool). A template helper of the
// converter framework, shared by six instantiations through merge-similar-functions:
//   ConverteLista(out, dados, conversor, sizeof(TDado), &TConversor::Converte, SEQUENCE_OF vtable, info)
// It builds an ASN1::SEQUENCE_OF<TEntidade> and appends conversor.Converte(d) for every element;
// SEQUENCE_OF::push_back stores a clone (vtable slot 3, do_clone) of the temporary.
//   thunk  element type (ASN)                                   sizeof(TDado)  used by
//   9183   ModuloInformacaoMidia::Aplicativo                    72             CConversorInformacaoMidia::DoConverte
//   9098   ModuloResultadoUrnaCadastro::ComparecimentoMesario   40             CConversorDadosComparecimento (u40)
//   9100   ModuloResultadoUrnaCadastro::IdentificacaoJustificativa 24          CConversorDadosComparecimento (u40)
//   9123   ModuloResultadoUrnaCadastro::EstadoComparecimento    96             CConversorComparecimentoSecao (u40)
//   9140   ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio  40             CConversorConfiguracaoMunicipios (u40)
//   9210   ModuloFederacoes::Federacao                          40             CConversorFederacoes (u40)
// (name inferred; the real one probably lives in ecourna/api/asn/iconversorasn.hpp or a sibling)
template <class TConversor>
ASN1::SEQUENCE_OF<typename TConversor::TEntidade>
ConverteLista(const std::vector<typename TConversor::TDado>& dados, const TConversor& conversor)
{
    ASN1::SEQUENCE_OF<typename TConversor::TEntidade> lista;
    for (const auto& dado : dados) {
        lista.push_back(conversor.Converte(dado));
    }
    return lista;
}

} // namespace ecourna::api::asn

namespace ecourna::app::dados::asn {

// wasm func 9189 (srcloc line 78): thunk into the shared body func 2306 (count 3): 0-based -> 1-based.
ModuloInformacaoMidia::TipoMidia CConversorInformacaoMidia::ConverteTipoMidia(TDado::ETipoMidia tipo) const
{
    if (static_cast<unsigned>(tipo) >= 3) {
        throw CAsnMidiasError(2488, "Tipo de mídia inválido.");   // line 78
    }
    return ModuloInformacaoMidia::TipoMidia(static_cast<ModuloInformacaoMidia::TipoMidia::NamedNumber>(tipo + 1));
}

// inlined into func 9179 (srcloc line 94)
CConversorInformacaoMidia::TDado::ETipoMidia CConversorInformacaoMidia::DeconverteTipoMidia(ModuloInformacaoMidia::TipoMidia tipo) const
{
    const unsigned valor = static_cast<unsigned>(tipo.asInt() - 1);
    if (valor >= 3) {
        throw CAsnMidiasError(2489, "Tipo de mídia inválido.");   // line 94
    }
    return static_cast<TDado::ETipoMidia>(valor);
}

// wasm func 9190 (vtable slot 2)
// Library code: 9185 = uninitialized copy of CAplicativo (vector copy), 5104 = its exception guard,
// 2664 = ~vector<CAplicativo>, 9182 = generated InformacaoMidia::set_aplicativos (includeOptionalField(0, 6)
// + copy-and-swap of the SEQUENCE OF).
CConversorInformacaoMidia::TEntidade CConversorInformacaoMidia::DoConverte(const TDado& midia) const
{
    const CConversorTurno conversorTurno;
    const CConversorFase conversorFase;
    const CConversorAplicativo conversorAplicativo;
    const CConversorDadosGeracaoMidia conversorDadosGeracao;

    ModuloInformacaoMidia::InformacaoMidia entidade;
    entidade.set_tipoMidia(ConverteTipoMidia(midia.GetTipoMidia()));
    entidade.set_fase(conversorFase.Converte(midia.GetFase()));
    entidade.set_idPE(midia.GetIdPE());                                           // Constrained_INTEGER<0, 99999>
    entidade.set_uf(ModuloTiposEleitorais::SiglaUF(midia.GetUF()));
    entidade.set_turno(conversorTurno.Converte(midia.GetTurno()));
    entidade.set_dadosGeracaoMidia(conversorDadosGeracao.Converte(midia.GetDadosGeracaoMidia()));
    if (midia.GetTipoMidia() == CInformacaoMidia::MR) {
        const TVectorAplicativo aplicativos = midia.GetAplicativos();            // copied
        entidade.set_aplicativos(api::asn::ConverteLista(aplicativos, conversorAplicativo));   // funcs 9183 -> 1970, 9182
    } else {
        entidade.omit_aplicativos();
    }
    return entidade;
}

// wasm func 9179 (vtable slot 3; the tool named it after the inlined DeconverteTipoMidia)
// Quirk: when `aplicativos` is present the medium is built as MR whatever tipoMidia says (the
// converted tipoMidia is dropped); when it is absent a tipoMidia of MR makes the constructor throw
// 3043. So "aplicativos present" <=> MR.
CConversorInformacaoMidia::TDado CConversorInformacaoMidia::DoDeconverte(const TEntidade& midia) const
{
    const CConversorTurno conversorTurno;
    const CConversorFase conversorFase;
    const CConversorAplicativo conversorAplicativo;
    const CConversorDadosGeracaoMidia conversorDadosGeracao;

    const auto tipoMidia = DeconverteTipoMidia(midia.get_tipoMidia());
    const auto fase = conversorFase.Deconverte(midia.get_fase());
    // CBaseType<unsigned, 0, 99999, 0> (inlined): >= 100000 throws EPatternError 1300
    // "O tipo 'ProcessoEleitoralID' deve ter valores no intervalo [0,99999]." (cbasetype.hpp:39)
    const TProcessoEleitoralID idPE(midia.get_idPE());
    const std::string uf = midia.get_uf();
    const auto turno = conversorTurno.Deconverte(midia.get_turno());
    const auto dadosGeracao = conversorDadosGeracao.Deconverte(midia.get_dadosGeracaoMidia());

    if (midia.aplicativos_isPresent()) {
        const auto lista = midia.get_aplicativos();                               // SEQUENCE_OF copy
        TVectorAplicativo aplicativos;
        api::asn::DeconverteLista(lista.begin(), lista.end(), std::back_inserter(aplicativos),
                                  conversorAplicativo);                           // slot 6783 (unit u11)
        return CInformacaoMidia(fase, idPE, uf, turno, dadosGeracao, aplicativos);    // func 9033
    }
    return CInformacaoMidia(tipoMidia, fase, idPE, uf, turno, dadosGeracao);          // func 9031
}

} // namespace ecourna::app::dados::asn

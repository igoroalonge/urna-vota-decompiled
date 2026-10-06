// ecourna-lib/ecourna/app/dados/asn/cconversor*.h  (paths inferred: one header per converter in the
// original; the declarations of this unit's converters are gathered here to keep the tree small)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Every converter derives from ecourna::api::asn::IConversorASN<ENTIDADE, DADO>
// (ecourna/api/asn/iconversorasn.hpp, unit u11). Its vtable has 4 slots:
//   [0] ~T()           (ICF "return this", func 174)
//   [1] ~T() deleting  (func 144, operator delete)
//   [2] TEntidade DoConverte(const TDado&) const      data class  -> ASN.1 object
//   [3] TDado DoDeconverte(const TEntidade&) const    ASN.1 object -> data class
// Public non-virtual wrappers of the base:
//   Converte(d)   = DoConverte(d), then requires the result to be isValid() && isStrictlyValid()
//                   (iconversorasn.hpp:49);
//   Deconverte(e) = requires e.isValid() && e.isStrictlyValid(), else throws
//                   CBaseError<EApiAsnError>(1902, "<mangled type name>: <ASN1::trace_invalid text>")
//                   (iconversorasn.hpp:66); then DoDeconverte(e).
// The tool names many slot functions after an inlined static helper whose source_location they
// carry (e.g. func 9156 "DesconverterFormaSuspenderComVoto" is really
// CConversorParametrosUrna::DoDeconverte); the vtable slot decides the real method.
//
// ASN.1 accessors are written in the style of the III ASN.1 generated classes:
// get_x() / set_x(v) / x_isPresent() / omit_x() / ref_x(), CHOICE currentSelection() / select_x().
#pragma once

#include <memory>

#include "ModuloFederacoes.h"
#include "ModuloInformacaoMidia.h"
#include "ModuloParametrizacaoUrna.h"
#include "ModuloResultadoUrnaCadastro.h"
#include "ModuloTiposEcoUrna.h"
#include "ModuloTiposEleitorais.h"
#include "ecourna/api/asn/iconversorasn.hpp"
#include "ecourna/app/dados/ccabecalhoentidade.h"
#include "ecourna/app/dados/cfoto.h"
#include "ecourna/app/dados/federacoes/cfederacao.h"
#include "ecourna/app/dados/iidentificadoreleitor.h"
#include "ecourna/app/dados/midias/cinformacaomidia.h"
#include "ecourna/app/dados/parametrizacaourna/cparametrosurna.h"
#include "ecourna/app/dados/resultadournacadastro/cresultadournacadastro.h"

// Interfaces assumed from classes of unit u40 (not reconstructed here):
//   CCabecalhoEntidade (16 bytes): +0 ptime dataGeracao, +8 int id, +12 ETipoId tipo
//     { IdProcessoEleitoral = 0, IdPleito = 1, IdEleicao = 2, IdInvalido = 3 (?) };
//     ctor CCabecalhoEntidade(ptime, int id, ETipoId) = func 5112; GetDataGeracao/GetId/GetTipoId inline.
//   CFoto: +0 EFormatoImagem { FormatoJPEG = 1, FormatoBMP = 2 }, +4 std::vector<uebyte> imagem;
//     ctor CFoto(EFormatoImagem, const std::vector<uebyte>&) = shared_f5111.
//   ConverteDataHoraJE(const DataHoraJE&) -> ptime = func 1877; FormataDataHoraJE(ptime) -> std::string
//     = func 9220 ("{:04}{:02}{:02}T{:02}{:02}{:02}").   (names inferred)

namespace ecourna::app::dados::asn {

using api::asn::IConversorASN;

// ---- asn/cconversorturno.h -------------------------------------------------- vtable @1126344
class CConversorTurno : public IConversorASN<ModuloTiposEleitorais::Turno, TTurno> {
protected:
    ModuloTiposEleitorais::Turno DoConverte(const TDado&) const override;          // func 9192
    TTurno DoDeconverte(const TEntidade&) const override;                            // func 9191
};

// ---- asn/cconversorfase.h --------------------------------------------------- vtable @1126136
class CConversorFase : public IConversorASN<ModuloTiposEleitorais::Fase, CFaseID> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9195
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9193
};

// ---- asn/cconversorcargoid.h ------------------------------------------------ vtable @1122576
class CConversorCargoID : public IConversorASN<ModuloTiposEleitorais::CodigoCargoConsulta, TCargoID> {
protected:
    ModuloTiposEleitorais::CodigoCargoConsulta DoConverte(const TCargoID&) const override;   // func 9230
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9228
};

// ---- asn/cconversorfoto.h --------------------------------------------------- vtable @1122824
class CConversorFoto : public IConversorASN<ModuloTiposEleitorais::Foto, CFoto> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9227
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9226
private:
    ModuloTiposEleitorais::FormatoImagem::NamedNumber ConverteFormatoImagem(CFoto::EFormatoImagem) const;    // inlined
    CFoto::EFormatoImagem DeconverteFormatoImagem(ModuloTiposEleitorais::FormatoImagem::NamedNumber) const;  // inlined
};

// ---- asn/cconversorcabecalhoentidade.h -------------------------------------- vtable @1123740
class CConversorCabecalhoEntidade
    : public IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, CCabecalhoEntidade> {
protected:
    ModuloTiposEleitorais::CabecalhoEntidade DoConverte(const CCabecalhoEntidade&) const override;  // func 9219
    CCabecalhoEntidade DoDeconverte(const TEntidade&) const override;                // func 9218
};

// ---- asn/cconversoridentificadoreleitor.h ----------------------------------- vtable @1131700
class CConversorIdentificadorEleitor
    : public IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, CIdentificadorEleitor> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9117
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9116
};

// ---- asn/federacoes/cconversorfederacao.h (path inferred) ------------------- vtable @1123956
class CConversorFederacao : public IConversorASN<ModuloFederacoes::Federacao, CFederacao> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9217
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9216 (unit u40)
};

// ---- asn/midias/cconversoraplicativo.h -------------------------------------- vtable @1125716
class CConversorAplicativo : public IConversorASN<ModuloInformacaoMidia::Aplicativo, CAplicativo> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9199
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9196
private:
    ModuloInformacaoMidia::TipoAplicativo ConverteTipoAplicativo(CAplicativo::ETipoAplicativo) const;   // func 9198
    CAplicativo::ETipoAplicativo DeconverteTipoAplicativo(ModuloInformacaoMidia::TipoAplicativo) const; // inlined
};

// (asn/midias/cconversorautenticacao.h and cconversordadosgeracaomidia.h: unit u40/u11)
class CConversorAutenticacao;       // IConversorASN<Autenticacao, CAutenticacao>, vtable @1125420
class CConversorDadosGeracaoMidia;  // IConversorASN<DadosGeracaoMidia, CDadosGeracaoMidia>, vtable @1125156

// ---- asn/midias/cconversorinformacaomidia.h --------------------------------- vtable @1126684
class CConversorInformacaoMidia
    : public IConversorASN<ModuloInformacaoMidia::InformacaoMidia, CInformacaoMidia> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9190
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9179
private:
    ModuloInformacaoMidia::TipoMidia ConverteTipoMidia(TDado::ETipoMidia) const;    // func 9189
    TDado::ETipoMidia DeconverteTipoMidia(ModuloInformacaoMidia::TipoMidia) const;  // inlined
};

// ---- asn/parametrizacaourna/cconversortitulorelatorio.h --------------------- vtable @1129244
class CConversorTituloRelatorio
    : public IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, CTituloRelatorio> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9149
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9145
private:
    ModuloParametrizacaoUrna::EstiloTitulo::NamedNumber ConverterEstiloTitulo(CTituloRelatorio::EEstiloTitulo) const;           // func 9147
    ModuloParametrizacaoUrna::AlinhamentoTitulo::NamedNumber ConverterAlinhamentoTitulo(CTituloRelatorio::EAlinhamentoTitulo) const; // func 9148
    CTituloRelatorio::EEstiloTitulo DesconverterEstiloTitulo(const TEntidade&) const;            // inlined
    CTituloRelatorio::EAlinhamentoTitulo DesconverterAlinhamentoTitulo(const TEntidade&) const;  // inlined
};

// ---- asn/parametrizacaourna/cconversorlabelparametrizado.h ------------------ vtable @1127788
class CConversorLabelParametrizado
    : public IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, CLabelParametrizado> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9169
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9167
private:
    ModuloParametrizacaoUrna::GeneroLabel::NamedNumber ConverterGeneroLabel(CLabelParametrizado::EGeneroLabel) const;  // func 9168
    CLabelParametrizado::EGeneroLabel DesconverterGeneroLabel(const TEntidade&) const;   // inlined
};

// ---- asn/parametrizacaourna/cconversorparametrosurna.h ---------------------- vtable @1128320
class CConversorParametrosUrna
    : public IConversorASN<ModuloParametrizacaoUrna::ParametrosUrna, CParametrosUrna> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9164
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9156
private:
    ModuloParametrizacaoUrna::FormaSuspenderComVoto ConverterFormaSuspenderComVoto(CParametrosUrna::EFormaSuspenderComVoto) const; // func 9162
    ModuloParametrizacaoUrna::FormaSuspenderSemVoto ConverterFormaSuspenderSemVoto(CParametrosUrna::EFormaSuspenderSemVoto) const; // func 9163
    CParametrosUrna::EFormaSuspenderComVoto DesconverterFormaSuspenderComVoto(const TEntidade&) const;   // inlined
    CParametrosUrna::EFormaSuspenderSemVoto DesconverterFormaSuspenderSemVoto(const TEntidade&) const;   // inlined
    // Names inferred; members (not free functions) because the wasm signature keeps a dead `this`.
    TVectorTituloRelatorio DesconverterTitulos(
        const ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio>&) const;                      // func 5102
    ASN1::SEQUENCE_OF<ModuloParametrizacaoUrna::TituloRelatorio> ConverterTitulos(
        const TVectorTituloRelatorio&) const;                                                            // func 9161
};

// ---- asn/resultadournacadastro/*.h --------------------------------------------------------
class CConversorRegistroIdentificacaoEleitor;   // unit u40, vtable @1131924
class CConversorIdentificacaoJustificativa;     // unit u40, vtable @1136564
class CConversorComparecimentoSecao;            // unit u40, vtable @1131064
class CConversorDadosComparecimento;            // unit u40, vtable @1132868
class CConversorDadosCifracao;                  // unit u40, vtable @1136812
class CConversorDadosComparecimentoCifrado;     // unit u40, vtable @1137068

class CConversorApresentacaoFotoEleitor                                        // vtable @1134200
    : public IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, CApresentacaoFotoEleitor> {
public:
    static ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor ConverteEstado(CApresentacaoFotoEleitor::EEstado);        // func 9089
    static CApresentacaoFotoEleitor::EEstado DeconverteEstado(ModuloResultadoUrnaCadastro::EstadoApresentacaoFotoEleitor);      // inlined
    static ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor ConverteResultado(CApresentacaoFotoEleitor::EResultado); // func 9088
    static CApresentacaoFotoEleitor::EResultado DeconverteResultado(ModuloResultadoUrnaCadastro::ResultadoApresentacaoFotoEleitor); // inlined
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9091
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9087
};

class CConversorComparecimentoMesario                                          // vtable @1132424
    : public IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoMesario, CComparecimentoMesario> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9110
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9106
private:
    ModuloResultadoUrnaCadastro::EstadoColetaDigital ConverteEstadoColetaDigital(CComparecimentoMesario::EEstadoColetaDigital) const;  // func 9108
    CComparecimentoMesario::EEstadoColetaDigital DeconverteEstadoColetaDigital(ModuloResultadoUrnaCadastro::EstadoColetaDigital) const; // func 9103
};

class CConversorEstadoHabilitacaoPorCodigo                                     // vtable @1135500
    : public IConversorASN<ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo, CEstadoHabilitacaoPorCodigo> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9075
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9073
private:
    ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario ConverteSituacaoReconhecimentoMesario(
        CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario) const;          // func 9074
    CEstadoHabilitacaoPorCodigo::ESituacaoReconhecimentoMesario DeconverteSituacaoReconhecimentoMesario(
        ModuloResultadoUrnaCadastro::SituacaoReconhecimentoMesario) const;           // func 9072
};

class CConversorHabilitacaoBiometrica                                          // vtable @1136004
    : public IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, CHabilitacaoBiometrica> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9071
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9066
private:
    ModuloResultadoUrnaCadastro::ErroLeituraBiometria ConverteErroLeituraBiometria(TErroLeituraBiometria) const;          // func 9069
    TErroLeituraBiometria DeconverteErroLeituraBiometria(const ModuloResultadoUrnaCadastro::ErroLeituraBiometria&) const; // inlined
    ModuloTiposEleitorais::TipoDedo ConverteTipoDedo(const CDedo::ETipoDedo&) const;                                      // func 9070
    CDedo::ETipoDedo DeconverteTipoDedo(const ModuloTiposEleitorais::TipoDedo&) const;                                    // inlined
};

class CConversorEstadoComparecimento                                           // vtable @1134808
    : public IConversorASN<ModuloResultadoUrnaCadastro::EstadoComparecimento, CEstadoComparecimento> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9086
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9081
private:
    ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor ConverteEstadoComparecimento(TDado::ESituacaoComparecimento) const;    // func 9085
    TDado::ESituacaoComparecimento DeconverteEstadoComparecimento(ModuloResultadoUrnaCadastro::SituacaoComparecimentoEleitor) const;  // func 9080
    ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio ConverteSituacaoHabilitacaoAudio(TDado::ESituacaoHabilitacaoAudio) const;   // func 9083
    TDado::ESituacaoHabilitacaoAudio DeconverteSituacaoHabilitacaoAudio(ModuloResultadoUrnaCadastro::SituacaoHabilitacaoAudio) const; // func 9077
};

class CConversorResultadoUrnaCadastro                                          // vtable @1137524
    : public IConversorASN<ModuloResultadoUrnaCadastro::EntidadeResultadoUrnaCadastro, CResultadoUrnaCadastro> {
protected:
    TEntidade DoConverte(const TDado&) const override;                              // func 9056
    TDado DoDeconverte(const TEntidade&) const override;                             // func 9052
private:
    ModuloTiposEcoUrna::SituacaoArquivo ConverteSituacaoArquivo(CResultadoUrnaCadastro::ESituacaoArquivo) const;   // func 9055
    CResultadoUrnaCadastro::ESituacaoArquivo DeconverteSituacaoArquivo(ModuloTiposEcoUrna::SituacaoArquivo) const; // func 9051
};

} // namespace ecourna::app::dados::asn

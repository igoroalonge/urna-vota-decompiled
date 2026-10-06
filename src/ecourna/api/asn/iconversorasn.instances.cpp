// ecourna-lib/ecourna/api/asn/iconversorasn.hpp -- out-of-line instantiations present in the wasm.
// NOT an original file: iconversorasn.hpp is header-only. This file maps every wasm function of unit u11
// that is an instantiation (or a merged body) of IConversorASN<ENTIDADE, DADO>::Converte/Deconverte.
// Reconstructed from vota_web_wasm.wasm (unit u11).
//
// Each thunk is `body(result, this, arg, &srcloc, typeid(TEntidade).name())` (20/21 bytes), except the two
// ENUMERATED Deconverte thunks 5103/9178 (18 bytes): `body(this, arg, &srcloc, name)`, the small TDado being
// returned in a register. The srcloc record is iconversorasn.hpp:49 for Converte and :66 for Deconverte; its
// address is given for each thunk.
#include "ecourna/api/asn/iconversorasn.hpp"

// ---- merged bodies (wasm-opt merge-similar-functions) ---------------------------------------------------
// wasm func  682  Converte   (SEQUENCE entity, invoke_* + landing pads)             <- 22 thunks below
// wasm func 1167  Deconverte (invoke_* + landing pads)                              <- 11 thunks below
// wasm func 6031  Converte   (no landing pads; copy from a TU without exception catching) <- 3734, 5365
// wasm func 6032  Deconverte (no landing pads; same)                                <- 3733, 5708
// wasm func 6149  Deconverte for ENUMERATED (value > info->maxEnumValue is invalid)  <- 5103, 9178
// wasm func 6150  Converte   for ENUMERATED                                          <- 9187, 9188
// helpers used by every body:
//   wasm func  858  std::basic_stringstream<char>::basic_stringstream()   (in|out; libc++ default ctor)
//   wasm func 1074  CApiAsnError::CApiAsnError(EApiAsnError, std::string, std::source_location)
//                   -> shared CBaseError ctor body 710 with vtable CBaseError<EApiAsnError,{1900,1910}>
//   (func 1075 basic_stringstream::str(), func 671 ~basic_stringstream, func 208 ASN1::trace_invalid,
//    func 221/231 AbstractData::isValid/isStrictlyValid are outside this unit)

namespace ecourna::api::asn {

using namespace ::ecourna::app::dados;

// ---- ModuloTiposEleitorais ---------------------------------------------------------------------------------
template class IConversorASN<ModuloTiposEleitorais::CabecalhoEntidade, CCabecalhoEntidade>;
//   wasm func 2665 Deconverte -> 1167 (srcloc @1125116)   callers: CConversorResultadoUrnaCadastro::DoDeconverte,
//                                                          CConversorConfiguracaoMunicipios/Federacoes/ParametrizacaoUrna::DoDeconverte
//   wasm func 9211 Converte   ->  682 (srcloc @1125084)   callers: the same four converters' DoConverte
template class IConversorASN<ModuloTiposEleitorais::Fase, CFaseID>;
//   wasm func 5103 Deconverte -> 6149 (@1127328)  wasm func 9188 Converte -> 6150 (@1127264)
template class IConversorASN<ModuloTiposEleitorais::Turno, CBaseType<unsigned short, 1, 2, 11>>;
//   wasm func 9178 Deconverte -> 6149 (@1127360)  wasm func 9187 Converte -> 6150 (@1127280)
template class IConversorASN<ModuloTiposEleitorais::Foto, CFoto>;
//   wasm func 5708 Deconverte -> 6032 (@1562624)   callers: comum CConversorBiometriaEleitor::DoDesconverte,
//                                                  comum CConversorFotoCandidato::DoDesconverte (11440)
template class IConversorASN<ModuloTiposEleitorais::MunicipioZona, CMunicipioZona>;
//   wasm func 9128 Converte   ->  682 (@1131024)   (Deconverte only exists inlined into 9127)
template class IConversorASN<ModuloTiposEleitorais::IdentificacaoSecaoEleitoral, CIdentificacaoSecaoEleitoral>;
//   wasm func 9124 Converte   ->  682 (@1131612)   (Deconverte only inlined into 9120)
template class IConversorASN<ModuloTiposEleitorais::IdentificadorEleitor, CIdentificadorEleitor>;
//   CHOICE: full copies, CHOICE::isValid (1068) + CHOICE::isStrictlyValid (1067)
//   wasm func 9111 Deconverte (@1132228, caller CConversorRegistroIdentificacaoEleitor::DoDeconverte 9112)
//   wasm func 9113 Converte   (@1132212, caller CConversorRegistroIdentificacaoEleitor::DoConverte 9114;
//                              landing pad destroys the entity with ASN1::CHOICE::~CHOICE)

// ---- ModuloTiposEcoUrna -------------------------------------------------------------------------------------
template class IConversorASN<ModuloTiposEcoUrna::IdentificadorGeradorMidia, CIdentificadorGeradorMidia>;
//   wasm func 3733 Deconverte -> 6032 (@1565708)   wasm func 3734 Converte -> 6031 (@1565692)
//   callers: ecourna CConversorDadosGeracaoMidia, comum CConversorCarga, comum CConversorDadoCorrespondencia
template class IConversorASN<ModuloTiposEcoUrna::RegistroIdentificacaoEleitor, CRegistroIdentificacaoEleitor>;
//   wasm func 9105 Deconverte -> 1167 (@1132824)   wasm func 9109 Converte -> 682 (@1132808)

// ---- ModuloResultadoUrnaCadastro (attendance file, see the module doc) --------------------------------------
template class IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimento, CDadosComparecimento>;
//   wasm func 9050 Deconverte -> 1167 (@1137880)
//   wasm func 5365 Converte   -> 6031 (@1599524)   callers: CConversorResultadoUrnaCadastro::DoConverte,
//                                                  comum::CGravadorRCSecao (func 11616, writes the file)
template class IConversorASN<ModuloResultadoUrnaCadastro::DadosComparecimentoCifrado, CDadosComparecimentoCifrado>;
//   wasm func 9048 Deconverte -> 1167 (@1137896)   wasm func 9053 Converte -> 682 (@1137864)
template class IConversorASN<ModuloResultadoUrnaCadastro::DadosCifracao, CDadosCifracao>;
//   wasm func 9059 Converte   ->  682 (@1137404)   (Deconverte only inlined into 9057)
template class IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoSecao, CComparecimentoSecao>;
//   wasm func 9093 Deconverte -> 1167 (@1134096)   wasm func 9101 Converte -> 682 (@1133964)
template class IConversorASN<ModuloResultadoUrnaCadastro::ComparecimentoMesario, CComparecimentoMesario>;
//   wasm func 9096 Converte   ->  682 (@1134028)   (table slot 6929, used by list helper 1970 via 9098)
//   (Deconverte only inlined into the std::transform instance 9092)
template class IConversorASN<ModuloResultadoUrnaCadastro::IdentificacaoJustificativa, CIdentificacaoJustificativa>;
//   wasm func 9097 Converte   ->  682 (@1133980)   (table slot 6928, via 9100) (Deconverte inlined into 9094)
template class IConversorASN<ModuloResultadoUrnaCadastro::EstadoComparecimento, CEstadoComparecimento>;
//   wasm func 9121 Converte   ->  682 (@1131628)   (Deconverte inlined into 9119)
template class IConversorASN<ModuloResultadoUrnaCadastro::EstadoHabilitacaoPorCodigo, CEstadoHabilitacaoPorCodigo>;
//   wasm func 9067 Converte   ->  682 (@1136524)   (Deconverte inlined into CConversorHabilitacaoBiometrica 9066)
template class IConversorASN<ModuloResultadoUrnaCadastro::HabilitacaoBiometrica, CHabilitacaoBiometrica>;
//   wasm func 9078 Deconverte -> 1167 (@1135396)   wasm func 9082 Converte -> 682 (@1135380)
template class IConversorASN<ModuloResultadoUrnaCadastro::ApresentacaoFotoEleitor, CApresentacaoFotoEleitor>;
//   wasm func 9076 Deconverte -> 1167 (@1135412)   wasm func 9084 Converte -> 682 (@1135364)

// ---- ModuloConfiguracaoMunicipios (-cfm.dat) ----------------------------------------------------------------
template class IConversorASN<ModuloConfiguracaoMunicipios::ConfiguracaoMunicipio, CConfiguracaoMunicipio>;
//   wasm func 9136 Deconverte -> 1167 (@1130304)   wasm func 9138 Converte -> 682 (@1130288)
template class IConversorASN<ModuloConfiguracaoMunicipios::HorariosUrna, CHorariosUrna>;
//   wasm func 9143 Converte   ->  682 (@1129728)   (Deconverte inlined into 9142)

// ---- ModuloParametrizacaoUrna (-pu.dat) ---------------------------------------------------------------------
template class IConversorASN<ModuloParametrizacaoUrna::ParametrosUrna, CParametrosUrna>;
//   wasm func 9172 Converte   ->  682 (@1127700)   (Deconverte inlined into 9171)
template class IConversorASN<ModuloParametrizacaoUrna::TituloRelatorio, CTituloRelatorio>;
//   wasm func 9153 Deconverte -> 1167 (@1129124)   wasm func 9157 Converte -> 682 (@1129108)
template class IConversorASN<ModuloParametrizacaoUrna::LabelParametrizado, CLabelParametrizado>;
//   wasm func 9154 Deconverte -> 1167 (@1129072)   wasm func 9159 Converte -> 682 (@1129056)

// ---- ModuloInformacaoMidia (infomidia-*.dat) ----------------------------------------------------------------
template class IConversorASN<ModuloInformacaoMidia::DadosGeracaoMidia, CDadosGeracaoMidia>;
//   wasm func 9177 Deconverte -> 1167 (@1127376)   wasm func 9186 Converte -> 682 (@1127296)
template class IConversorASN<ModuloInformacaoMidia::Aplicativo, CAplicativo>;
//   wasm func 9180 Converte   ->  682 (@1127312)   (Deconverte inlined into the std::transform instance 9176)
template class IConversorASN<ModuloInformacaoMidia::Autenticacao, CAutenticacao>;
//   wasm func 9197 Converte   ->  682 (@1126032)   (Deconverte inlined into CConversorAplicativo 9196)

// ---- ModuloFederacoes (-fe.dat) -----------------------------------------------------------------------------
template class IConversorASN<ModuloFederacoes::Federacao, CFederacao>;
//   wasm func 9208 Converte   ->  682 (@1125100)   (Deconverte inlined into the std::transform instance 9206)

// ---- instantiations with NO out-of-line Converte/Deconverte (always inlined into the caller) ----------------
//   <ModuloTiposEleitorais::CodigoCargoConsulta, CBaseType<unsigned short,1,99,3>>   (comum RDV converters)
//   <ModuloBoletimUrna::DetalhamentoComparecimento, CDetalhamentoComparecimento>     (BU writer, func 10273)
//   <ModuloFederacoes::EntidadeFederacoes, CFederacoes>, <ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna,
//    CParametrizacaoUrna>, <ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios,
//    CConfiguracaoMunicipios>                                                        (func 7787, static data load)
//   <ModuloInformacaoMidia::InformacaoMidia, CInformacaoMidia>                       (api::CFileASN::ReadFromFile 3735)
//   <ModuloResultadoUrnaCadastro::EntidadeResultadoUrnaCadastro, CResultadoUrnaCadastro> (CFileASN::CodeObjectFunction 5366)

}  // namespace ecourna::api::asn

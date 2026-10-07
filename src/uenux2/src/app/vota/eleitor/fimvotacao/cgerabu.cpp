// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp
//
// Only ONE function of this file survives as such: CGeraBU::StartState (wasm func 12110, 17 KB).
// LTO inlined into it the whole construction of comum::CGeradorBU and its GeraRelatorio(), so the
// function carries the source_location records of other files:
//   cgerabu.cpp:54                     Assert (appInfo.GetVota().GetEstadoVota() == EAVGERARBU)  (3454)
//   cestadogeralvota.h:159             GetDtHrEmissaoBU()      "A data/hora da emissão do BU não foi registrada" (8093)
//   cestadogeralvota.h:90 / :106       GetDtHrInicioAquisicao / GetDtHrFimAquisicao  (8090 / 8091)
//   cgeradorbu.cpp:124                 comum::CriaQRCode(zona, secao, correspondencia, historico, bool,
//                                                         dtEmissao, bool, bool)   (IInterfaceInit lookup)
//   cgeradorbu.cpp:161                 char comum::DefineTipo(const bool)          (IInterfaceInit lookup)
//   cgeradorrelbase.h:48 / :51         CGeradorRelBase ctor  "header nulo" / "trailer nulo"  (9066/9067)
//   cgeradorbubase.h:66..111           CGeradorBUBase ctor   10 x "<parte> nulo"             (9055..9064)
//   cgeradorrelbase.h:61               CGeradorRelBase::GeraRelatorio(const std::string&)  (IPaperRelatorios)
//   ccalculacv.cpp:58 / :65 / :74      CCalculaCV ctor  (9051 / 9052 / 9053)
//   util.cpp:37                        comum::util::(anonymous)::LeChave(path)   ("cv.ber.pri")
//   ckey.cpp:31                        ecourna::api::security::CKeyData ctor "Bytes da chave vazio."
//   cpolysingletonlist.h:129           api::CPolySingletonList::push<comum::CCalculaCV>  (6756)
// The body below is written as the cgerabu.cpp author most likely wrote it (a CGeradorBU on the
// stack + GeraRelatorio); what the inlined comum code does is spelled out in comments, because it is
// the only place in the binary where that code exists as such. The functions of comum that the
// tools placed in this unit follow at the end of the file.
//
// Nothing here ran in the recorded sessions: the web page drives only the voter terminal, and the
// operator-side encerramento that leads to EAVGERARBU is never reached.

#include "vota/eleitor/fimvotacao/cgerabu.h"

#include <format>
#include <source_location>
#include <string>
#include <vector>

#include "api/gui/cpaperformbuilder.h"
#include "api/gui/cqrcodeimagepaper.h"
#include "api/hwil/ipaperrelatorios.h"
#include "api/pattern/cpolysingletonlist.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/ccargos.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/clocal.h"
#include "comum/dados/crdvvota.h"
#include "comum/iinterfaceinit.h"
#include "comum/relatorios/ccalculacv.h"
#include "comum/relatorios/cgeradorbu.h"
#include "comum/relatorios/cgeradorbuqrcode.h"
#include "comum/relatorios/crelutil.h"
#include "vota/eleitor/fimvotacao/cgerarelatorios.h"
#include "vota/log/clogvota.h"
#include "vota/sincronismo/csincronizavota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

// wasm func 12110 — vtable slot 2 (analyzer name vota::CGeraBU::vf2)
void CGeraBU::StartState()
{
    m_telaVotacaoEncerrada->Exibe();                                    // form slot 2

    auto& appInfo = comum::CAppInfo::GetInst();                         // func 185
    UE_ASSERT(appInfo.GetVota().GetEstadoVota() == EEstadoVota::EAVGERARBU);   // :54 (59)

    api::CWait::Sleep(1000);   // emscripten_sleep(1000) when the byte @1584624 == 1 (it is 1 in this build)
    m_telaPreparandoDados->Exibe();

    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::BU, false);   // func 1047
                                                         // "Gerando relatório [BU] [INÍCIO]"
    {
        auto& estadoGeral = appInfo.GetEstadoGeral();                   // GetEstado<CEstadoGeral> (func 291)
        auto& local = comum::CLocal::GetInst();                         // func 401
        comum::CCargos::GetInst().Inicio();                             // func 3784 (name inferred)
        const std::string separadorFase = comum::CRelUtil::GetSeparadorFase(estadoGeral.GetFase());
            // "==============TREINAMENTO=============" ('3'), "===============SIMULADO===============" ('1'),
            // "======================================" ('2' oficial), "============DEMONSTRAÇÃO==============" (demo)

        auto& vota = appInfo.GetVota();
        if (!vota.TemDtHrEmissaoBU())                                   // optional<CDateTime> +80 (flag +92)
            vota.SetDtHrEmissaoBU(api::CDateTime::Now());               // api_f479

        // codigoCarga of every correspondência recorded in gap.bin (EstadoGeralGap.correspondencias,
        // 96-byte CDadoCorrespondencia, string at +28). name inferred
        const std::vector<std::string> historicoCargas = appInfo.GetGap().GetCodigosCarga();   // func 3788

        const std::string& nomeMunicipio = local.GetNomeMunicipio();   // func 1077
        const std::string titulo = "Boletim de Urna";

        comum::CGeradorBU gerador(titulo,
                                  estadoGeral.GetMunicipio(),           // +20
                                  estadoGeral.GetZona(),                // +24 (u16)
                                  estadoGeral.GetSecao(),               // +26 (u16)
                                  nomeMunicipio,
                                  estadoGeral.GetCorrespondencia(),     // +60 CDadoCorrespondencia
                                  historicoCargas,
                                  vota.GetDtHrEmissaoBU(),              // h:159 throws 8093 if unset
                                  separadorFase);
        // ^ inlined constructor (cgeradorbu.cpp), in this order:
        //   header   = CriaHeader(titulo, municipio, zona, secao, nomeMunicipio, correspondencia, ?, dtEmissao):
        //              CRelUtil::IncluiCabecalhoEleicoesMZS(b, titulo, mun, zona, secao, nomeMunicipio, 0, false)
        //              "Eleitores aptos                   {:04}"   (std::function lambda over SQtdeAptos, func 1921)
        //              IncluiLinhaQuantidade("Comparecimento", CRdvVota::Comparecimento())          (func 2248)
        //              if (CLocal::UrnaBiometrica() && CEleitores +108):
        //                  "        Habilitação biométrica"   = CEleitores::QtdHabilitados(1)   (func 2821)
        //                  "        Habilitação biográfica"   = CEleitores::QtdHabilitados(2)   (func 1935)
        //                  "        Habilitação sem biometria"= CEleitores::QtdSemBiometria()   (func 2822)
        //              "Eleitores faltosos" = aptos + aptosOutros - comparecimento
        //              newline, "Código identificação UE       {:08}" (carga.numeroInternoUrna, func 1942)
        //              if (CAppInfo::TemVota(turno)) IncluiDatasAberturaFechamento(b, dhIniAquisicao,
        //                  dhFimAquisicao)  (func 3688; throws 8090/8091 when a date is missing)
        //              IncluiResumoCorrespondencia(b, correspondencia) "RESUMO DA CORRESPONDÊNCIA" (func 1541)
        //              data text 1633, newline
        //   trailer  = separadorFase, "Código de identificação da carga" + CRelUtil::GetIDCargaFormatado(
        //              carga.codigoCarga), "Ver: " + CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO"),
        //              IncluiTextoComUF(b, CConfiguracaoEleicao +212) (func 5584), 20 newlines, paper cut
        //   qrcode   = CriaQRCode(zona, secao, correspondencia, historicoCargas, ..., dtEmissao, ...):
        //              demo mode   -> "DEMONSTRAÇÃO PRÉ-ELEIÇÃO" + "NÃO HÁ QR CODE"
        //              CConfiguracaoEleicao +486 bit 0 (QR code enabled) ->
        //                  CGeradorBUQRCode(dados{zona, secao, numeroInternoUrna, codigoCarga, historico},
        //                                   comparecimento, dtEmissao).GeraQRCodes(1100) (func 5604):
        //                  "============= BU DIGITAL =============", for each payload i/n:
        //                      "-------------- {:02} / {:02} ---------------" + QR image   (funcs 5581, 2775)
        //                  "======== CERTIFICADO DIGITAL =========", MontaQRCodesCertificado(estadoGeral,
        //                      true) joined as "KEY:value ..." (funcs 5634, 3646), same i/n + image
        //                  if the signature is not empty: "ASSINATURA BU DIGITAL: " + signature
        //              otherwise   -> no QR form (nullptr)
        //   headerProporcional         = separadorFase, newline, CampoNomeCargo('-', 38, 2920)   (func 3699)
        //   headerProporcionalPartido  = data 2921
        //   partidoApenasVotoLegenda   = "Não há votos nominais"
        //   trailerProporcionalPartido = data 2922, newline
        //   cargoSemCandidato          = "Não há candidatos concorrendo", newline
        //   trailerCargoSemCandidato   = separator(38 '-'), data 1632, nl, data 1633, nl
        //   trailerProporcional        = separator, data 1625, nl, data 1633, nl
        //   headerMajoritario          = separadorFase, nl, CampoNomeCargo('-', 38, 2920), data 2923
        //   detalheCandidato           = CriaDetalheCandidato(true) = data 1636               (func 3872)
        //   trailerMajoritario         = nl, separator, data 1626, nl, data 1633, nl
        //   CGeradorRelBase(header, trailer, qrcode) checks header/trailer != nullptr (h:48/51),
        //   CGeradorBUBase(...) checks the ten parts (h:66..111); CGeradorBU stores historicoCargas.
        //   Then the "código verificador" calculator is created and published:
        //       ident = std::format("{:05}{:04}{:04}{}{}{}{}", municipio, zona, secao,
        //                           std::format("{:05}{:05}{}", cfg[+0], cfg.pleito /* +28 */,
        //                                       cfg.dataPleito /* +44 */ .Format("YYYYMMDD")),
        //                           carga.numeroInternoUrna /* EstadoGeral +60 */, cfg.pleito, fase char)
        //       (packed arg types 6,6,6,13,6,6,2 = unsigned x3, string_view, unsigned x2, char: the
        //        nested string is the 4th argument, stored at +704 of the format-arg array)
        //       api::CPolySingletonList::push(std::make_unique<comum::CCalculaCV>(
        //           ident, "0000000000", comum::DefineTipo(IInterfaceInit::GetDemoMode()) /* 'A' demo, 'F' */,
        //           16));   // cpolysingletonlist.h:129, "{}: instância já criada de {}" (6756)
        //       CCalculaCV ctor: ident must be >= 13 chars of "0123456789sodtSODT" (ccalculacv.cpp:58,
        //       "Identificação inválida [...]"), the first CV must be decimal (:65), the key is
        //       comum::util::LeChave("cv.ber.pri") (EntidadeChave envelope, CKeyLoader::DecipherKeyIfNeeded)
        //       and must have >= 16 bytes (:74 "Chave privada com tamanho pequeno..."); 16 bytes are kept;
        //       Reinicia(cv): buffer = tipo + ident + cv (ecourna_f5615).

        const std::string arquivo = "bu.dat";
        {
            api::CApplicationContextGuard contexto(2, "", "Gerando boletim de urna na MI",
                                                   "Ocorreu um erro durante a geração do boletim de urna na MI.");
            gerador.GeraRelatorio(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / arquivo);   // func 436
        // ^ inlined CGeradorRelBase<CRdvVota, CEleitores>::GeraRelatorio (cgeradorrelbase.h:61):
        //     auto& papel = api::IPaperRelatorios::GetInst();  papel.Abre(caminho);      // slot 5
        //     m_header->Imprime();  ImprimePreTexto();                                   // CGeradorBU slot 2
        //     cargos.Reset(); ultimaEleicao = eleição do último cargo; cargos.Primeiro();
        //     while (!cargos.Fim()) {
        //         if (eleição do cargo != ultimaEleicao)   // only printed when there are >= 2 eleições
        //             imprime "======================================" / nome da eleição / "===..." / nl;
        //         if (cargo.EhEleicao() && cargo.tipo == proporcional) ImprimeProporcionalPartido(); // slot 3
        //         else if (cargo.EhEleicao() && cargo.tipo == majoritario) ImprimeMajoritario();   // slot 4
        //         else ImprimeConsulta();                                                          // slot 5
        //         cargos.Next();
        //     }
        //     if (m_qrcode) m_qrcode->Imprime();  m_trailer->Imprime();
        //     papel.Fecha();                                                                   // slot 6
        //     api::CSynchronizer::CreateInst()->Sincroniza();
        }                                                                   // ~CApplicationContextGuard (func 675)
        comum::SalvaEstado();                                               // func 491
        CSincronizaVota::SincronizaRelatorios(arquivo);                     // copies "bu.dat" to the mirror area
    }                                                                       // ~CGeradorBU (func 3698)

    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::BU, true);   // "... [BU] [TÉRMINO]"
    appInfo.GetVota().SetEstadoVota(EEstadoVota::EAVGERARRELATORIOS);           // 60
    comum::SalvaEstado();
    m_proximoEstado = &CGeraRelatorios::GetInst();                              // func 6084
}

// =================================================================================================
// comum functions that the tools attributed to this file (their only or main caller is
// CGeraBU::StartState). Original files are in uenux2/src/app/comum/relatorios/ (paths inferred).
// =================================================================================================

namespace comum {

// wasm func 2248 — cgeradorbu.cpp (?). "label.....................value" line of the BU header.
// name inferred
void IncluiLinhaQuantidade(api::CPaperFormBuilder& b, const std::string& rotulo, unsigned quantidade)
{
    b.AddText(CRelUtil::CompletaDireita(rotulo, 34) + std::format("{:04}", quantidade), 1, 0);   // func 1881
}

// wasm func 3688 — crelutil.cpp (?). Also used by CGeraRelatorios (BUJ header). name inferred
void IncluiDatasAberturaFechamento(api::CPaperFormBuilder& b, const api::CDateTime& abertura,
                                   const api::CDateTime& fechamento)
{
    IncluiDataHora(b, abertura, "Data de abertura da UE", "Horário de abertura");        // func 1542
    IncluiDataHora(b, fechamento, "Data de fechamento da UE", "Horário de fechamento");
    b.AddNewLine(1);
}

// wasm func 3872 — cgeradorbu.cpp (?): part "detalheCandidato" (also used by CGeraZeresimaBase).
// name inferred
std::shared_ptr<api::IForm<api::IPaper>> CriaDetalheCandidato(bool bu)
{
    api::CPaperFormBuilder b;
    b.AddData(bu ? 1636 : 1635, 0);                    // CDataText<std::string> id (func 604)
    return b.Build();                                  // shared_f357
}

// wasm func 5581 — cgeradorbu.cpp (?): header of each QR code image. name inferred
void IncluiCabecalhoQRCode(api::CPaperFormBuilder& b, unsigned indice, unsigned total)
{
    b.AddText(std::format("-------------- {:02} / {:02} ---------------", indice, total), 1, 0);
    b.AddNewLine(1);
}

// wasm func 5584 — crelutil.cpp (?): adds a text whose "<uf>" tag is replaced by the section's UF.
// name inferred
void IncluiTextoComUF(api::CPaperFormBuilder& b, const std::string& texto)
{
    CSubstituidorTitulo::GetInst().Inclui(b, texto, CLocal::GetInst().GetUF());          // funcs 3689, 1702
}

// wasm func 5613 — api::CPaperFormBuilder helper (cpaperformbuilder.h, ?): adds a
// CTextFieldPaper over CDataText<CPadDS<CToUpperDS<CCargoDSNome>>> (cargo name, upper case, padded
// with `pad` to `largura`). name inferred
void AddNomeCargo(api::CPaperFormBuilder& b, const CPadDS<CToUpperDS<CCargoDSNome>>& fonte)
{
    // shared_ptr(new ...), not make_shared: the control blocks are __shared_ptr_pointer (vtables
    // @1575472 / @1543416); the field is pushed_back into the builder's vector<shared_ptr<IField>>
    const std::shared_ptr<api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>> dados(
        new api::CDataText<CPadDS<CToUpperDS<CCargoDSNome>>>(fonte));   // 24 bytes, 16-byte source copied
    b.AddField(std::shared_ptr<api::CTextFieldPaper>(new api::CTextFieldPaper(dados, 1)));   // func 2771
}

// wasm func 5634 — cgeradorbuqrcode.cpp or cgeradorbu.cpp (?): payloads of the "CERTIFICADO DIGITAL"
// QR codes. With `dividir`, the certificate (hex text of CEstadoGeral::RecuperarCertificado(), func
// 1243) is split in ceil(2 * bytes / 1082) parts; otherwise one part. name inferred
std::vector<std::vector<std::pair<std::string, std::string>>>
MontaQRCodesCertificado(const md::estadoaplicacao::CEstadoGeral& estado, bool dividir)
{
    // Despite the name `der`, the token returns the certificate as a 1,034-byte PEM text on UE2020/2022,
    // and DER only on UE2013/2015 (2026 urna data: investigation/README.md, finding H3).
    const std::vector<uebyte> der = md::estadoaplicacao::CEstadoGeral::RecuperarCertificado();   // func 5635
    const std::string texto = api::CStringUtils::ToHex(der);                                      // func 1243
    unsigned partes = 1;
    if (dividir)
        partes = static_cast<unsigned>(std::ceil(static_cast<float>(der.size() * 2) / 1082.0f));

    std::vector<std::vector<std::pair<std::string, std::string>>> qrcodes;
    if (partes == 0)
        return qrcodes;
    const std::size_t tamanho = static_cast<std::size_t>(std::ceil(float(texto.size()) / float(partes)));
    for (unsigned i = 0; i < partes; ++i) {
        std::vector<std::pair<std::string, std::string>> campos;
        campos.emplace_back("QRCE", std::format("{}:{}", i + 1, partes));
        campos.emplace_back("IDUE", std::to_string(estado.GetCorrespondencia().GetCarga().numeroInternoUrna));  // +60
        campos.emplace_back("MDUE", std::to_string(estado.GetModeloUrna()));            // +44 (?)
        campos.emplace_back("CERT", texto.substr(i * tamanho, tamanho));                // std::out_of_range
        qrcodes.push_back(std::move(campos));
    }
    return qrcodes;
}

// wasm func 3646 — joins "KEY:value" pairs with a space (the last space is removed). name inferred
std::string MontaConteudoQRCode(const std::vector<std::pair<std::string, std::string>>& campos)
{
    std::string conteudo;
    for (const auto& [chave, valor] : campos)
        conteudo = conteudo + chave + ":" + valor + " ";
    if (!conteudo.empty())
        conteudo.pop_back();
    return conteudo;
}

// wasm func 12039 — std::string(*)(), table slot 1529, used by vota::CMostraQRCodeCertificado::StartState
// (func 12040) as the source of the certificate QR code shown on the screen. name inferred
std::string ConteudoQRCodeCertificado()
{
    const auto qrcodes = MontaQRCodesCertificado(CAppInfo::GetInst().GetEstadoGeral(), false);
    return MontaConteudoQRCode(qrcodes.front());       // copy of the first (only) element
}

// wasm func 5614 — comum::CCalculaCV::~CCalculaCV() (ccalculacv.cpp), also reached from
// shared_ptr<CCalculaCV>::__on_zero_shared (func 11263). Layout (56 bytes):
//   +0 std::string m_identificacao   +12 char m_tipo   +16 std::string m_buffer
//   +28 std::vector<uebyte> m_chave (16 bytes)   +40 std::string m_cv   +52 ?
CCalculaCV::~CCalculaCV() = default;

}  // namespace comum

// Library / template instances that the tools attributed to this file:
//   wasm func 625  std::vector<std::pair<std::string, std::string>>::push_back(value_type&&) (reallocating
//                  path; 24-byte elements; also used by the <regex> bracket-expression code)
//   wasm func 5980 std::vector<std::vector<std::pair<std::string, std::string>>>::~vector()  (__destroy_vector)
//   wasm func 2775 api::CPaperFormBuilder::AddQRCode(const <QR image>& imagem)  — the argument is the
//                  struct returned by api::CQRCodeImagePaper::MontaImagem() (vector<uebyte> pixels +0,
//                  int +12, bool +16), copied into a new 44-byte CQRCodeImageFieldPaper (vtable @1582080)
//                  held by shared_ptr(new ...) and pushed into the builder's field vector
//                  (api/gui, path inferred; name inferred)

}  // namespace vota

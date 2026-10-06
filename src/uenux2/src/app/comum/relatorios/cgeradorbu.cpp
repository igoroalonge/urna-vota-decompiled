// uenux2/src/app/comum/relatorios/cgeradorbu.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// srclocs of this file:
//   :124  std::shared_ptr<api::IForm<api::IPaper>> comum::CriaQRCode(const TZonaID, const TSecaoID,
//             const md::estadoaplicacao::CDadoCorrespondencia&, const std::vector<std::string>&, const bool,
//             const api::CDateTime&, const bool, const bool)                  (inlined in wasm 12110)
//   :161  char comum::DefineTipo(const bool)                                  (inlined in wasm 12110)
//   :257  virtual void comum::CGeradorBU::ImprimePreTexto() const             wasm 11256
// RTTI: std::function<SQtdeAptos()> holding comum::CriaHeader(const std::string&, unsigned, unsigned short,
//       unsigned short, const std::string&, const CDadoCorrespondencia&, bool, api::CDateTime)::$_0
//       (vtable @1575244) -> signature of CriaHeader.
//
// Functions of this unit: 3698 (~CGeradorBU), 11258 (deleting dtor), 11256 (ImprimePreTexto).
// The constructor and the Cria* helpers were compiled only inlined into vota::CGeraBU::StartState
// (wasm 12110, unit u08, see src/uenux2/src/app/vota/eleitor/fimvotacao/cgerabu.cpp); they are written
// here from that function because this is their source file. Not executed in the recorded votes.
//
// Output (samples/bu-real/run-full/reports/bu.txt, produced by running this code in the patched wasm):
// header (IncluiCabecalhoEleicoesMZS + aptos/comparecimento/faltosos + "Código identificação UE" + opening
// and closing date/time + RESUMO DA CORRESPONDÊNCIA + CV), "Histórico de código de carga" + CV, one block
// per cargo, "BU DIGITAL" QR codes + "CERTIFICADO DIGITAL" QR + "ASSINATURA BU DIGITAL: <hex>", trailer.

#include "comum/relatorios/cgeradorbu.h"

#include <format>
#include <memory>

#include "api/gui/cqrcodeimagepaper.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cstringutils.h"
#include "comum/cappinfo.h"
#include "comum/dados/ccargods.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/clocal.h"
#include "comum/iinterfaceinit.h"
#include "comum/relatorios/ccabecalhoqrcodebuilder.h"
#include "comum/relatorios/ccalculacv.h"
#include "comum/relatorios/cdatasourcesrelatorio.h"
#include "comum/relatorios/cgeradorbuqrcode.h"
#include "comum/relatorios/crelutil.h"
#include "comum/relatorios/csubstituidortitulo.h"

namespace comum {

using DS = CDataSourcesRelatorio<CRdvVota, CEleitores>;

// ------------------------------------------------------------------------------------------------------
// Inlined into 12110. Header of the BU.
SharedPaperForm CriaHeader(const std::string& titulo, unsigned municipio, unsigned short zona, unsigned short secao,
                           const std::string& nomeMunicipio,
                           const md::estadoaplicacao::CDadoCorrespondencia& correspondencia, bool temVota,
                           api::CDateTime /*dataHoraEmissao*/)
{
    const CLocal& local = CLocal::GetInst();
    CEleitores& eleitores = CEleitores::GetInst();
    const CRdvVota& rdv = CRdvVota::GetInst();

    api::CPaperFormBuilder b;
    CRelUtil::IncluiCabecalhoEleicoesMZS(b, titulo, municipio, zona, secao, nomeMunicipio,
                                         ETipoCabecalho::SECAO, false);                   // func 1543
    const auto qtdAptos = eleitores.GetQtdAptos();                                        // func 2823
    b.AddText(CRelUtil::FormataQtdAptos([qtdAptos] { return qtdAptos; }), 1, 0);          // funcs 1922/1921
    const TQtdVoto comparecimento = rdv.Comparecimento();                                 // shared_f1269
    IncluiLinhaQuantidade(b, "Comparecimento", comparecimento);                           // func 2248
    if (local.UrnaBiometrica() && eleitores.UsaBiometria()) {                             // func 820, +108
        IncluiLinhaQuantidade(b, "        Habilitação biométrica", eleitores.QtdHabilitados(1));    // 2821
        IncluiLinhaQuantidade(b, "        Habilitação biográfica", eleitores.QtdHabilitados(2));    // 1935
        IncluiLinhaQuantidade(b, "        Habilitação sem biometria", eleitores.QtdSemBiometria());  // 2822
    }
    IncluiLinhaQuantidade(b, "Eleitores faltosos",
                          static_cast<unsigned short>(qtdAptos.originais + qtdAptos.temporarios - comparecimento));
    b.AddNewLine(1);
    b.AddText(CRelUtil::DSCodigoIdentificacaoUE(), 1, 0);                                  // func 1942
    if (temVota) {
        const auto& vota = CAppInfo::GetInst().GetVota();
        IncluiDatasAberturaFechamento(b, vota.GetDtHrInicioAquisicao(),                   // h:90  (8090)
                                      vota.GetDtHrFimAquisicao());                        // h:106 (8091), 3688
    }
    CRelUtil::IncluiResumoCorrespondencia(b, correspondencia);                            // func 1541
    b.AddData(&DS::CodVerificador, 0);                                                    // slot 1633
    b.AddNewLine(1);
    return b.Build();
}

// Inlined into 12110. Trailer: phase separator, carga code, software version, BU footer of the PU, cut.
static SharedPaperForm CriaTrailer(const std::string& separadorFase,
                                   const md::estadoaplicacao::CDadoCorrespondencia& correspondencia)   // name inferred
{
    api::CPaperFormBuilder b;
    b.AddText(separadorFase, 1, 0);
    b.AddNewLine(1);
    b.AddText("Código de identificação da carga", 1, 2);
    b.AddText(CRelUtil::GetIDCargaFormatado(correspondencia.GetCodigoCarga()), 1, 2);    // +28
    b.AddNewLine(1);
    b.AddText(std::format("Ver: {}", api::CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO")), 1, 0);
    b.AddNewLine(1);
    CSubstituidorTitulo::IncluiTitulos(b, CConfiguracaoEleicao::GetInst().GetParametros().GetRodapeBUVota(),
                                       CLocal::GetInst().GetUF());                        // func 5584, cfg +212
    b.AddNewLine(20);
    CortaPapel(b);                                                                        // func 1264
    return b.Build();
}

// srcloc :124 (inlined into 12110). The QR codes of the BU ("BU digital") and of the urna's certificate.
SharedPaperForm CriaQRCode(const TZonaID zona, const TSecaoID secao,
                           const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                           const std::vector<std::string>& historicoCargas, const bool imprimeQRCode,
                           const api::CDateTime& dataHoraEmissao, const bool origemRED, const bool incluiEmissao)
{
    if (api::CPolySingleton<IInterfaceInit>::instance().GetDemoMode()) {                 // :124
        api::CPaperFormBuilder b;
        b.AddNewLine(1);
        IncluiSeparador(b);                                                               // func 1156 ?
        b.AddNewLine(1);
        b.AddText("DEMONSTRAÇÃO PRÉ-ELEIÇÃO", 1, 2);
        b.AddText("NÃO HÁ QR CODE", 1, 2);
        b.AddNewLine(1);
        return b.Build();
    }
    if (!imprimeQRCode)                               // ParametrosUrna.imprimirQrCodeNoBU (cfg +486, bit 0)
        return nullptr;

    // Header fields of the payload (builder: 5624 ctor, 5618 zona, 5620 secao, 5621 idUE, 5623 carga,
    // 5622 histórico); preBuild validates them (lambda wasm 2791).
    const CCabecalhoQRCode cabecalho = CCabecalhoQRCodeBuilder()
                                           .SetZona(zona)
                                           .SetSecao(secao)
                                           .SetIdUrna(correspondencia.GetNumeroInternoUrna())
                                           .SetCodigoCarga(correspondencia.GetCodigoCarga())
                                           .SetHistoricoCarga(historicoCargas)
                                           .Build();
    CGeradorBUQRCodeVota gerador(cabecalho, CRdvVota::GetInst().Comparecimento(), dataHoraEmissao,
                                 origemRED, incluiEmissao);                                // func 5603
    const auto qrcodes = gerador.GeraQRCodes(1100);                                        // func 5604

    api::CPaperFormBuilder b;
    b.AddText("============= BU DIGITAL =============", 1, 0);
    b.AddNewLine(1);
    for (std::size_t i = 0; i < qrcodes.conteudos.size(); ++i) {
        IncluiCabecalhoQRCode(b, i + 1, qrcodes.conteudos.size());                        // func 5581
        b.AddQRCode(api::CQRCodeImagePaper::MontaImagem(qrcodes.conteudos.at(i)));         // func 2775
        if (i + 1 < qrcodes.conteudos.size())
            b.AddNewLine(1);
    }
    b.AddNewLine(1);
    b.AddText("======== CERTIFICADO DIGITAL =========", 1, 0);
    b.AddNewLine(1);
    std::vector<std::string> certificado;
    for (const auto& campos : MontaQRCodesCertificado(GetEstado<md::estadoaplicacao::CEstadoGeral>(), true))
        certificado.push_back(MontaConteudoQRCode(campos));                                // 5634 / 3646
    for (std::size_t i = 0; i < certificado.size(); ++i) {
        IncluiCabecalhoQRCode(b, i + 1, certificado.size());
        b.AddQRCode(api::CQRCodeImagePaper::MontaImagem(certificado.at(i)));
        if (i + 1 < certificado.size())
            b.AddNewLine(1);
    }
    b.AddNewLine(1);
    if (!qrcodes.assinatura.empty()) {
        b.AddText("ASSINATURA BU DIGITAL: " + qrcodes.assinatura, 1, 0);
        b.AddNewLine(1);
    }
    return b.Build();
}

// srcloc :161 (inlined into 12110). Type letter that starts the código-verificador chain.
char DefineTipo(const bool modoDemonstracao)
{
    return modoDemonstracao ? 'A' : 'F';
}

// ------------------------------------------------------------------------------------------------------
// Inlined into 12110. Builds the thirteen report parts, then publishes the CV calculator.
CGeradorBU::CGeradorBU(const std::string& titulo, TMunicipioID municipio, TZonaID zona, TSecaoID secao,
                       const std::string& nomeMunicipio,
                       const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                       const std::vector<std::string>& historicoCargas, const api::CDateTime& dataHoraEmissao,
                       const std::string& separadorFase)
    : CGeradorBUBase<CRdvVota, CEleitores>(
          CriaHeader(titulo, municipio, zona, secao, nomeMunicipio, correspondencia,
                     CAppInfo::GetInst().TemVota(GetEstado<md::estadoaplicacao::CEstadoGeral>().GetTurno()),
                     dataHoraEmissao),
          CriaTrailer(separadorFase, correspondencia),
          CriaQRCode(zona, secao, correspondencia, historicoCargas,
                     CConfiguracaoEleicao::GetInst().GetParametros().ImprimirQrCodeNoBU(),   // ?
                     dataHoraEmissao, false, false),                                        // ?
          CriaHeaderCargo(separadorFase),                        // headerProporcional (func 3699)
          [] { api::CPaperFormBuilder b; b.AddData(&DS::HeaderProporcionalPartido, 0); return b.Build(); }(),  // slot 2921 (11261)
          [] { api::CPaperFormBuilder b; b.AddText("Não há votos nominais", 1, 2); return b.Build(); }(),
          [] { api::CPaperFormBuilder b; b.AddData(&DS::TrailerProporcionalPartido, 0); b.AddNewLine(1);
               return b.Build(); }(),
          [] { api::CPaperFormBuilder b; b.AddText("Não há candidatos concorrendo", 1, 2); b.AddNewLine(1);
               return b.Build(); }(),
          [] { api::CPaperFormBuilder b; b.AddSeparador(0); b.AddData(&DS::TrailerComparecimento, 0);   // 1387, slot 1632
               b.AddNewLine(1); b.AddData(&DS::CodVerificador, 0); b.AddNewLine(1); return b.Build(); }(),
          [] { api::CPaperFormBuilder b; b.AddSeparador(0); b.AddData(&DS::TrailerProporcional, 0);
               b.AddNewLine(1); b.AddData(&DS::CodVerificador, 0); b.AddNewLine(1); return b.Build(); }(),
          [&separadorFase] {                                                               // headerMajoritario
              api::CPaperFormBuilder b;
              b.AddText(separadorFase, 1, 0);
              b.AddNewLine(1);
              AddNomeCargo(b, CPadDS<CToUpperDS<CCargoDSNome>>('-', 38, 2));               // func 5613 (slot 2920)
              b.AddData(&DS::HeaderDetalheSeHouverVotos, 0);                            // slot 2923 (11259)
              return b.Build();
          }(),
          CriaDetalheCandidato(true),                                                      // func 3872 (slot 1636)
          [] { api::CPaperFormBuilder b; b.AddNewLine(1); b.AddSeparador(0);
               b.AddData(&DS::TrailerMajoritario, 0); b.AddNewLine(1); b.AddData(&DS::CodVerificador, 0);
               b.AddNewLine(1); return b.Build(); }())
    , m_historicoCargas(historicoCargas)
{
    // The header's CodVerificador must not receive the "final" strings: if the CCargos cursor is on the
    // last cargo, move it past the end.
    CCargos& cargos = CCargos::GetInst();
    cargos.Inicio();                                                                       // func 3784
    if (cargos.GetIndice() + 1 == cargos.GetQuantidade())
        cargos.Next();

    // Identification that seeds the código-verificador chain.
    const auto& cfg = CConfiguracaoEleicao::GetInst();
    const auto& estadoGeral = GetEstado<md::estadoaplicacao::CEstadoGeral>();
    const bool demo = api::CPolySingleton<IInterfaceInit>::instance().GetDemoMode();      // :161
    const std::string pleito = std::format("{:05}{:05}{}", cfg.GetIdProcessoEleitoral(), cfg.GetPleito().GetId(),
                                           cfg.GetPleito().GetData().Format("YYYYMMDD"));
    const std::string identificacao =
        std::format("{:05}{:04}{:04}{}{}{}{}", municipio, zona, secao, pleito,
                    estadoGeral.GetCorrespondencia().GetNumeroInternoUrna(), cfg.GetPleito().GetId(),
                    estadoGeral.GetDadoCarga().GetFaseChar());                             // "o"/"s"/"t"
    api::CPolySingletonList::push(std::make_unique<CCalculaCV>(identificacao, "0000000000", DefineTipo(demo), 16));
    // (cpolysingletonlist.h:129: "{}: instância já criada de {}" if one is already published)
}

// ------------------------------------------------------------------------------------------------------
// wasm func 3698 (vtable slot 0). Unpublishes the calculator, then the members / bases are destroyed:
// m_historicoCargas (+116), ~CGeradorBUBase (5612).
CGeradorBU::~CGeradorBU()
{
    api::CPolySingletonList::erase<CCalculaCV>();        // func 640 with typeid name "N5comum10CCalculaCVE"
}
// wasm func 11258 (vtable slot 1) = deleting destructor: ~CGeradorBU(); operator delete(this).

// ------------------------------------------------------------------------------------------------------
// wasm func 11256 (srcloc :257), vtable slot 2. "Histórico de código de carga": every carga (load of the
// election data into this urna) recorded in gap.bin, numbered, then its own código verificador.
//
//     --------------------------------------
//     Histórico de código de carga
//     --------------------------------------
//     1: 123.456.789.012.345.678.901.234
//
//     Código Verificador: 5.170.454.836
void CGeradorBU::ImprimePreTexto() const
{
    api::CPaperFormBuilder b;
    b.AddText("--------------------------------------", 1, 2);
    b.AddText("Histórico de código de carga", 1, 2);
    b.AddText("--------------------------------------", 1, 2);
    for (std::size_t i = 0; i < m_historicoCargas.size();) {
        const std::string carga = CRelUtil::GetIDCargaFormatado(m_historicoCargas[i]);     // func 2786
        ++i;
        b.AddText(std::format("{}: {}", i, carga), 1, 2);
    }
    b.AddNewLine(1);
    b.AddData(&CRelUtil::DSCodigoVerificador, 0);                                         // slot 3048
    b.AddNewLine(1);
    const SharedPaperForm form = b.Build();

    // The raw 24-digit codes, each followed by '@', enter the chain before the form (and its CV) prints.
    std::string cadeia;
    for (const std::string& carga : m_historicoCargas)
        cadeia = cadeia + carga + "@";
    api::CPolySingleton<CCalculaCV>::instance().IncluiString(cadeia);                     // :257
    form->Imprime();
}

} // namespace comum

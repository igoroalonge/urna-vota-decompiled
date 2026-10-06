// uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
// srcloc: :173  std::string comum::(anonymous namespace)::ConverteTipoAlimentacao()  (inlined in wasm 11188)
// Functions of this unit: 11183..11202, 11216 (vtable slots), 5472, 5473 (date/time data sources) and
// 5782 (CQRCodeDS::operator()). The constructor (wasm 5586) and the printing template (vota_f5591) were
// filed elsewhere by the tools. Only 5472/5473 ran in the recorded votes (clock of the screens).
//
// Output skeleton (vota_f5591 calling the hooks below; "[n]" = vtable slot):
//     IMPRESSO EM MODO DEMONSTRAÇÃO           if [22]
//     <títulos do PU>                          [11]   (IncluiTitulos, "<uf>" substituted)
//     Eleições Comunitárias                    if [12]
//     <processo eleitoral> / <pleito> / (DD/MM/YYYY)   [13] [14]
//     ESTADO DA URNA                           [21]
//     ======... separador de fase ...          [20]
//     UE DE VOTAÇÃO | UE DE CONTINGÊNCIA       [17] "UE DE {}\n\n"
//     UF                                  AC   [16] (after [10] configured the labels)
//     Município / Zona / Código identificação UE / MC / carga / Data e Hora da carga
//     <linhas do local>                        [15]
//     EMISSÃO DO RELATÓRIO / DATA: ... HORA: ...  [25]
//     RESUMO DA CORRESPONDÊNCIA ...            [9]
//     Previsão de horário de verão ...         [3] [7] [8] (skipped when [2] treinamento)
//     <QR code>                                [4] [5] [6]
//     Ver: ... / URNA OPERANDO EM PERFEITAS / CONDIÇÕES DE FUNCIONAMENTO   [18] [19]
//     ASSINATURAS

#include "comum/relatorios/crelatoriotesteimpressora.h"

#include <format>
#include <map>
#include <regex>

#include "api/application/capplication.h"      // ms_versao (@1839192)
#include "api/hwil/ipower.h"
#include "api/pattern/cpolysingletonlist.h"
#include "api/util/cdatetime.h"
#include "api/util/cstringutils.h"
#include "api/util/isystemdatetime.h"
#include "comum/cappinfo.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/clocal.h"
#include "comum/dados/cpe.h"
#include "comum/dados/md/ctradutorfrase.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "comum/relatorios/crelutil.h"

namespace comum {

namespace {

// srcloc :173 (inlined into 11188). Power source reported by api::IPower: vtable slot 15 (AtualizaStatus,
// see api/hwil/ipower.h) refreshes the status word CACHED INSIDE the IPower object (+4; the wasm passes
// `power + 4` as the argument and then reads that byte), bits 1..2 = source.
std::string ConverteTipoAlimentacao()
{
    api::IPower& power = api::CPolySingletonList::instance<api::IPower>();          // func 862, :173
    power.AtualizaStatus(power.m_status);                                           // slot 15, status at +4
    switch ((power.m_status.flags >> 1) & 3) {
    case 1:  return "B. Interna";           // bateria interna
    case 2:  return "B. Externa";           // bateria externa
    case 3:  return "Não identificada";
    default: return "R. Elétrica";          // rede elétrica
    }
}

const auto& EstadoGeral() { return CAppInfo::GetInst().GetGeral(); }                 // funcs 185 + 291

} // namespace

// ---- simple hooks ---------------------------------------------------------------------------------------
// wasm 11201 [2]: fase == '3' (treinamento): the report then omits the daylight-saving block.
bool CRelatorioTesteImpressora::EhTreinamento() const
{
    return EstadoGeral().GetFase() == EUrnaFase::TREINAMENTO;                          // eg +48
}

// wasm 11202 [3]
bool CRelatorioTesteImpressora::PossuiHorarioVerao() const
{
    return CHV::GetInst().PossuiHorarioVerao();                                        // CHV +24
}

// wasm 11199 [7]: {início, fim} of the daylight-saving period of the município.
const CHV::SPeriodo& CRelatorioTesteImpressora::GetHorarioVerao() const
{
    return CHV::GetInst().VerificaEntraEmHorarioVerao();                               // func 5745
}

// wasm 11198 [8]: início <= agora < fim (CDate/CDateTime three-way compare, func 1261).
bool CRelatorioTesteImpressora::EstaEmHorarioVerao() const
{
    const CHV::SPeriodo& periodo = CHV::GetInst().VerificaEntraEmHorarioVerao();
    const api::CDateTime agora = api::CDateTime::Agora();                               // api_f479
    return periodo.inicio <= agora && agora < periodo.fim;
}

// wasm 11197 [9]: copy of the correspondência of eg.bin (func 1249 = copy constructor), CEstadoGeral +60.
md::estadoaplicacao::CDadoCorrespondencia CRelatorioTesteImpressora::GetCorrespondencia() const
{
    return EstadoGeral().GetCorrespondencia();
}

// wasm 11196 [10]: installs the parametrized labels of the -pu.dat (município, zona, seção, partido) in the
// label translator used by CTradutorFrase::TraduzLabel ("<MLSN>", "<ZCSN>", ...): static map @1839056.
void CRelatorioTesteImpressora::ConfiguraLabels() const
{
    const auto& parametros = CConfiguracaoEleicao::GetInst().GetParametros();
    md::CTradutorFrase::SetLabels({{'M', parametros.GetLabelMunicipio()},   // cfg +272 (pair ctor 1552)
                                   {'Z', parametros.GetLabelZona()},        // cfg +324
                                   {'S', parametros.GetLabelSecao()},       // cfg +376
                                   {'P', parametros.GetLabelPartido()}});   // cfg +428   (api_f5795 / 5794)
}

// wasm 11195 [11]: EntidadeParametrizacaoUrna.cabecalho (CConfiguracaoEleicao +188).
const std::vector<ecourna::app::dados::CTituloRelatorio>& CRelatorioTesteImpressora::GetCabecalho() const
{
    return CConfiguracaoEleicao::GetInst().GetParametros().GetCabecalho();
}

// wasm 11194 [12]
bool CRelatorioTesteImpressora::EhEleicaoComunitaria() const
{
    return CConfiguracaoEleicao::GetInst().GetOrigem() == md::EOrigemConfiguracao::COMUNITARIA;   // +20 == 2
}

// wasm 11192 [13]: CConfiguracaoEleicao +8.
std::string CRelatorioTesteImpressora::GetNomeProcessoEleitoral() const
{
    return CConfiguracaoEleicao::GetInst().GetNomeProcessoEleitoral();
}

// wasm 11191 [14]: pleito 2 for a contingency urna on/after the 2nd-round date (flag set by the
// constructor), else the configured pleito (CConfiguracaoEleicao +28).
const md::CPleito& CRelatorioTesteImpressora::GetPleito() const
{
    if (m_usaPleito2 && CPE::GetInst().TemPleito2())                                   // +156
        return CPE::GetInst().GetPleito2();                                            // func 1267
    return CConfiguracaoEleicao::GetInst().GetPleito();
}

// wasm 11189 [16]: CEstadoGeral +8 (the UF of the carga).
std::string CRelatorioTesteImpressora::GetUF() const
{
    return EstadoGeral().GetUF();
}

// wasm 11190 [17]: tipo de urna of the current turno (EUrnaTipoOperacao, see cqrcodeds.cpp: '0' sem tipo,
// '1' vota, '2' contingência, '3' contingência-vota, '4' contingência-vota-recupera). The test is UNSIGNED
// (i32.gt_u): '1', '3', '4' -> VOTAÇÃO; '2' and every other value, including '0', -> CONTINGÊNCIA.
std::string CRelatorioTesteImpressora::GetTipoUrna() const
{
    const unsigned tipo = EstadoGeral().GetTipoUrnaTurnoAtual() - '1';   // turno '1' ? eg +36 : eg +40
    if (tipo > 3 || tipo == 1)
        return "CONTINGÊNCIA";
    return "VOTAÇÃO";
}

std::string CRelatorioTesteImpressora::GetMensagemFinal1() const { return "URNA OPERANDO EM PERFEITAS"; }     // 11186
std::string CRelatorioTesteImpressora::GetMensagemFinal2() const { return "CONDIÇÕES DE FUNCIONAMENTO\n"; }   // 11184

// wasm 11183 [20]
std::string CRelatorioTesteImpressora::GetSeparadorFase() const
{
    return CRelUtil::GetSeparadorFase(EstadoGeral().GetFase());                        // func 1919
}

std::string CRelatorioTesteImpressora::GetTitulo() const { return "ESTADO DA URNA"; }                         // 11187

// wasm 11193 [22]
bool CRelatorioTesteImpressora::EhModoDemonstracao() const
{
    return CInformacaoEleicao(CConfiguracaoEleicao::GetInst()).EhModoDemonstracao();   // funcs 603, 2286
}

// ---- wasm 11188 [15]: the "local" block ---------------------------------------------------------------
std::vector<std::string> CRelatorioTesteImpressora::GetLinhasLocal() const
{
    std::vector<std::string> linhas;
    const CLocal& local = CLocal::GetInst();
    const auto& estadoGeral = EstadoGeral();
    const unsigned tipo = estadoGeral.GetTipoUrnaTurnoAtual() - '1';                    // unsigned, as in 11190

    if (!(tipo > 3 || tipo == 1)) {                                                    // urna de votação
        const TSecaoID secao = local.GetSecaoID();
        if (estadoGeral.GetFase() == EUrnaFase::OFICIAL)
            linhas.push_back(std::format("Local                             {:04}", local.GetLocalID()));
        ConfiguraLabels();                                                             // virtual slot 10
        linhas.push_back(std::format("{}{:04}",
                                     CRelUtil::CompletaDireita(md::CTradutorFrase::TraduzLabel("<SCSN>"), 34),
                                     secao));
        // The two "agregadas" lines exist only when the section has aggregated sections (func 2816 != 0),
        // exactly as in CRelUtil::IncluiCabecalhoEleicoesMZS.
        if (local.GetQtdAgregadas() != 0) {                                            // func 2816
            linhas.push_back(std::format("{:<33s} {:04}",
                                         md::CTradutorFrase::TraduzLabel("Quantidade de <SCPB> agregad<S|os|os|as>"),
                                         local.GetQtdAgregadas()));
            linhas.push_back(CRelUtil::FormataSecoesAgregadas(
                md::CTradutorFrase::TraduzLabel("<SCPN> agregad<S|os|os|as>: {}")));   // func 5738
        }

        std::string tipoLocal;
        switch (local.GetTipoLocalVotacao()) {                // VerificaEhSecao("GetTipoLocalVotacao"), secao +0
        case 1:  tipoLocal = "Normal"; break;
        case 2:  tipoLocal = "Voto em Trânsito"; break;
        case 3:  tipoLocal = "Preso provisório"; break;
        case 4:  tipoLocal = "Temporário"; break;
        default: tipoLocal = "Inválido"; break;
        }
        linhas.push_back("Tipo de local" + api::CStringUtils::PadLeft(tipoLocal, ' ', 25));        // func 753
        linhas.push_back("Local com biometria"
                         + api::CStringUtils::PadLeft(local.UrnaBiometrica() ? "Sim" : "Não", ' ', 19));   // 820
    }
    linhas.push_back("Tipo de alimentação" + api::CStringUtils::PadLeft(ConverteTipoAlimentacao(), ' ', 19));
    return linhas;
}

// ---- wasm 11200 [5]: payload of the report's QR code ----------------------------------------------------
// The CQRCodeDS is built first (func 1300 = copy of the temporary field vector), then the origin field is
// appended to the functor's own vector (func 5783, AdicionaOrigemQRCode of cqrcodeds.cpp, unit u36:
// 0 -> {"ORIG","T"}, 1 -> {"ORIG","I"}; same sequence in api::CImageFieldUpdate, 3059), then it is called.
std::string CRelatorioTesteImpressora::GetConteudoQRCode() const
{
    CQRCodeDS ds(MontaCamposQRCodeEstadoUrna(EstadoGeral()));        // func 5636 (cqrcodeds.cpp), 1300
    AdicionaOrigemQRCode(ds.Campos(), 1);                             // func 5783 -> {"ORIG", "I"}
    return ds();                                                      // func 5782
}

// ---- wasm 11216 [25] ----------------------------------------------------------------------------------
void CRelatorioTesteImpressora::IncluiEmissao(api::CPaperFormBuilder& b) const
{
    b.AddNewLine(1);
    b.AddText("EMISSÃO DO RELATÓRIO", 1, 2);
    b.AddText(std::format("DATA: {:.10}       HORA: {:.8}", FormataDataAtual("DD/MM/YYYY"),
                          FormataHoraAtual("hh:mm:ss")), 1, 0);
}

// ---- data sources of the current date / time (table slots 1103 / 1104) ---------------------------------
// wasm 5472 — observed executing (voter-screen status header).
std::string FormataDataAtual(const std::string& formato)
{
    const api::CDateTime agora(api::CPolySingletonList::instance<api::ISystemDateTime>().GetTime());   // 1155, 1000
    return agora.GetDate().Format(formato);                                                          // func 706
}

// wasm 5473 — observed executing.
std::string FormataHoraAtual(const std::string& formato)
{
    const api::CDateTime agora(api::CPolySingletonList::instance<api::ISystemDateTime>().GetTime());
    return agora.GetTime().Format(formato);                                                          // shared_f779
}

// ---- wasm 5782: CQRCodeDS::operator()() ----------------------------------------------------------------
// Called at print time (and, through std::function<CQRCodeDS> = wasm 12553, every time the screens refresh
// their QR image). Appends the volatile fields to the fixed ones and joins them as "KEY:value KEY:value".
std::string CQRCodeDS::operator()() const
{
    const md::CPleito& pleito = CConfiguracaoEleicao::GetInst().GetPleito();
    const api::CDateTime agora;                                          // CDate() 1382 + CTime() 2230 (now)
    const std::string dataPleito = pleito.GetData().Format("YYYYMMDD");
    const bool horarioVerao = CHV::GetInst().PossuiHorarioVerao();      // CHV +24

    std::vector<std::pair<std::string, std::string>> campos = m_campos;
    // Software version "a.b.c.d" taken from the application version string (default "0.0.0.0").
    // NOTE: the '.' of the pattern is not escaped (matches any character); a new std::regex is compiled on
    // every call.
    std::string versao = "0.0.0.0";
    std::smatch m;
    if (std::regex_search(api::CApplication::ms_versao, m, std::regex("[0-9]+.[0-9]+.[0-9]+.[0-9]+")))
        versao = m.str(0);
    campos.emplace_back("VERS", versao);
    campos.emplace_back("DTAG", agora.GetDate().Format("YYYYMMDD"));
    campos.emplace_back("HRAG", agora.GetTime().Format("hhmmss"));
    campos.emplace_back("DTPL", dataPleito);
    campos.emplace_back("HRVR", horarioVerao ? "1" : "0");             // func 1348 (pair ctor)
    return MontaConteudoQRCode(campos);                                 // func 3646
}

} // namespace comum

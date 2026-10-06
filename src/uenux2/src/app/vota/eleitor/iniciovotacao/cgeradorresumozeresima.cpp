// Reconstructed from vota_web_wasm.wasm (unit u09).
// Original: uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp
//
// Report parts of the ZERÉSIMA (trab/ze.dat) and of its summary (trab/rze.dat). The zerésima is printed
// right before the vote opens: it lists every candidate of every cargo with zero votes and ends with
// an "EXTRATO DO RDV" proving that the Registro Digital do Voto holds no vote and no justification.
//
// srcloc evidence (func 5971):
//   :43  api::SharedPaperForm vota::(anonymous namespace)::CriaTituloExtratoRDV()
//        Assert (qtdJustificativas == 0)          (3479)
//   :47  Assert (votosCargos.Total() == 0)        (3480)
//
// File of CGeraZeresimaBase::StartState (func 11946) - NOT settled by the call graph. The
// anonymous-namespace signature above belongs to CriaTituloExtratoRDV, which is INLINED into func 5971;
// 5971 itself (the outer "CriaExtratoRDV", name inferred) may well have external linkage, so its being
// called from 11946 and from CGeraResumoZeresimaBase::StartState (11943, unit u20) says nothing about
// where those StartStates are defined. 11946 also contains two nested inlined frames (64 and 400
// bytes of stack) around the report construction, i.e. the report builder of this file was LTO-inlined
// into a StartState that can live in cgerazeresima.cpp as unit u07 assumes. 11946 is kept below only
// because the unit definition assigns it here.
//
// api::CPaperFormBuilder helpers used below (library-like, see the appendix):
//   AddText(texto, fonte, alinhamento) = shared_f193, AddNewLine(n) = func 198,
//   AddData(&fonteDeDados, flags) = func 604 (CTextFieldPaper over CDataText<std::string(*)()>: the
//   "ids" 1625/1626/1632/... that other units saw are FUNCTION-TABLE SLOTS of the data sources, e.g.
//   1625 = CDataSourcesRelatorio<CRdvVota,CEleitores>::TrailerProporcional), Build() = shared_f357.
// Not executed in the recorded sessions.

#include <memory>
#include <string>
#include <vector>

#include "api/gui/cformpart.h"
#include "api/gui/cpaperformbuilder.h"
#include "api/gui/reports/creport.h"
#include "comum/cappinfo.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/cjustificativas.h"
#include "comum/dados/crdvvota.h"
#include "comum/relatorios/cdatasourcesrelatorio.h"
#include "comum/relatorios/cpartecandidatos.h"
#include "comum/relatorios/crelutil.h"
#include "vota/comum/crelvotautil.h"
#include "vota/comum/csincronizavota.h"
#include "vota/eleitor/comum/ctelasvota.h"
#include "vota/eleitor/iniciovotacao/cgerazeresima.h"
#include "vota/log/clogvota.h"

namespace vota {

using DS = comum::CDataSourcesRelatorio<comum::CRdvVota, comum::CEleitores>;
using api::SharedPaperForm;
using SharedFormPart = std::shared_ptr<api::IFormPart>;   // name inferred

namespace {

// ---------------------------------------------------------------------------------------------------
// srcloc :43/:47 — inlined into func 5971
SharedPaperForm CriaTituloExtratoRDV()
{
    UE_ASSERT(comum::CJustificativas::GetInst().size() == 0);            // :43 qtdJustificativas (func 1391, +8)
    for (const auto& [cargo, votosCargos] : comum::CRdvVota::GetInst().GetVotosCargos())   // CRdvVota slot 0 (copy)
        UE_ASSERT(votosCargos.Total() == 0);                             // :47 (16-byte votes vector empty)

    api::CPaperFormBuilder b;
    b.AddText("======================================", 1, 2);
    b.AddText("Não há votos ou justificativas registrados", 2, 2);
    b.AddText("======================================", 1, 2);
    b.AddNewLine(1);
    b.AddText("-----------EXTRATO DO RDV-------------", 1, 0);
    b.AddNewLine(1);
    return b.Build();
}

// wasm func 5580 — banner printed before the cargos of each eleição (when there are several).  name inferred
SharedPaperForm CriaSeparadorEleicao()
{
    api::CPaperFormBuilder b;
    b.AddText("======================================", 1, 0);
    b.AddData(&DS_NomeEleicaoAtual, 2);            // slot 3049 -> func 11556: CCargos::GetCurrentEleicao() name
    b.AddText("======================================", 1, 0);
    b.AddNewLine(1);
    return b.Build();
}

// wasm func 5970 — "trailer majoritário" part.                                             name inferred
SharedPaperForm CriaTrailerMajoritario()
{
    api::CPaperFormBuilder b;
    b.AddData(&DS::TrailerMajoritario, 0);         // slot 1626 -> func 11980
    b.AddNewLine(1);
    return b.Build();
}

// wasm func 3873 — header of the candidate details in the zerésima.                       name inferred
SharedPaperForm CriaHeaderDetalheZE()
{
    api::CPaperFormBuilder b;
    b.AddData(&comum::CCargoDSLabelRelatorio::HeaderDetalheZE, 0);   // slot 1634 -> func 11555
    b.AddText("", 1, 0);
    return b.Build();
}

}  // namespace

// ---------------------------------------------------------------------------------------------------
// wasm func 5971 (analyzer name vota::(anonymous namespace)::CriaTituloExtratoRDV, after its inlined
// part). Builds the two parts of the "extrato do RDV" appended to the zerésima and to its summary
// (callers 11946 and 11943). Linkage/namespace of this outer function unknown.             name inferred
std::vector<SharedFormPart> CriaExtratoRDV(const std::string& separadorFase)
{
    auto titulo = std::make_shared<api::CFormPart>(CriaTituloExtratoRDV());        // func 601 = CFormPart ctor
    auto separador = std::make_shared<api::CFormPart>(CriaSeparadorEleicao());     // func 5580

    // per-cargo parts of the (empty) RDV
    auto headerCargo = std::make_shared<api::CFormPart>(CriaHeaderCargo(separadorFase));   // func 3699
    auto trailerMaj = std::make_shared<api::CFormPart>(CriaTrailerMajoritario());          // func 5970
    api::CPaperFormBuilder b;
    b.AddData(&DS::TrailerProporcional, 0);                                                 // slot 1625
    b.AddNewLine(1);
    auto trailerProp = std::make_shared<api::CFormPart>(b.Build());
    auto trailerMaj2 = std::make_shared<api::CFormPart>(CriaTrailerMajoritario());
    auto vazio = std::make_shared<api::CFormPart>(api::CPaperFormBuilder().Build());

    auto parteRdv = std::make_shared<comum::CParteRdv>(headerCargo, trailerMaj, trailerProp, trailerMaj2, vazio);
    auto cargos = std::make_shared<comum::CParteCargos>(separador, parteRdv);              // func 5587
    return {titulo, cargos};
}

// wasm func 5579 — trailer shared by the zerésima and its summary (and, inlined, by the BU). name inferred
SharedPaperForm CriaTrailer(const std::string& separadorFase, const std::string& textoUF,
                            const comum::md::estadoaplicacao::CDadoCorrespondencia& correspondencia)
{
    api::CPaperFormBuilder b;
    b.AddText(separadorFase, 1, 0);
    b.AddNewLine(1);
    b.AddText("Código de identificação da carga", 1, 2);
    b.AddText(comum::CRelUtil::GetIDCargaFormatado(correspondencia.GetCodigoCarga()), 1, 2);   // +28, func 2786
    b.AddNewLine(1);
    b.AddText("Ver: " + api::CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO"), 1, 0);  // func 1539
    b.AddNewLine(1);
    comum::CSubstituidorTitulo::GetInst().Inclui(b, textoUF, comum::CLocal::GetInst().GetUF());   // func 3689
    b.AddNewLine(20);
    b.AddCut();                                                                  // func 1264 (CCutFieldPaper)
    return b.Build();
}

// =================================================================================================
// wasm func 11946 — CGeraZeresimaBase::StartState (vtable slot 2 of CGeraZeresimaBase, CGeraZeresima and
// CRegerarZeresima; analyzer name vota::CGeraZeresimaBase::vf2). Layout: CAppState + m_tela (+12/+16,
// CTelasVota +212 "gerando zerésima"). Slot 9 = hook after generation (CRegerarZeresima: CortaPapel),
// slot 10 = GetEstadoResumo() (next state: CGeraResumoZeresima / CRegeraResumoZeresima).
// =================================================================================================
void CGeraZeresimaBase::StartState()
{
    m_proximoEstado = this;
    m_tela->Exibe();

    auto& appInfo = comum::CAppInfo::GetInst();
    const std::string separadorFase =
        comum::CRelUtil::GetSeparadorFase(appInfo.GetEstadoGeral().GetFase());           // func 1919
    const auto& estadoGeral = appInfo.GetEstadoGeral();

    std::vector<SharedFormPart> extrato = CriaExtratoRDV(separadorFase);                  // func 5971
    const SharedPaperForm headerCargo = CriaHeaderCargo(separadorFase);                  // func 3699

    // "cargo sem candidato" part
    api::CPaperFormBuilder b1;
    b1.AddText("Não há candidatos concorrendo", 1, 2);
    b1.AddNewLine(1);
    b1.AddSeparador(0);                           // func 1387: 38 x '-' (alinhamento 0)
    b1.AddData(&DS_TrailerCargoSemCandidato, 0);  // slot 1632 -> func 11975
    b1.AddNewLine(1);
    const SharedPaperForm cargoSemCandidato = b1.Build();

    // majoritarian cargos: header of details, detail line, blank line
    auto majoritarios = std::make_shared<comum::CParteCandidatosMajoritarios>(
        CriaHeaderDetalheZE(),                                  // func 3873
        CriaDetalheCandidato(/*bu*/ false),                     // func 3872: DS::DetalheCandidatoZE (slot 1635)
        api::CPaperFormBuilder().AddNewLine(1).Build());

    // proportional cargos: blank(0), party title (TituloPartido, slot 3050 -> func 11181),
    // details header/lines, blank lines, and "Não há candidato registrado" +
    // CTradutorFrase::TraduzLabel("para <P|este|este|esta> <PLSB>") (func 654) for empty parties;
    // shown per party only when the predicate slot 1631 (func 11976: the current party has
    // candidates in the current cargo) is true.
    auto proporcionais = std::make_shared<comum::CParteCandidatosProporcionais>(
        partesProporcionais /* 7 CFormParts, see above */, std::function<bool()>(&PartidoTemCandidatos));

    // consultas (referendums): details header, DS answers (slot 3051 -> CRespostas), blank line
    auto consultas = std::make_shared<comum::CParteCandidatosConsultas>(
        CriaHeaderDetalheZE(), CriaRespostas() /* AddData(slot 3051) */, CriaLinhaEmBranco());

    const SharedPaperForm cabecalho = CRelVotaUtil::CriaCabecalho("Zer\xE9sima");       // func 5977
    api::CPaperFormBuilder b2;
    b2.AddText("---------LISTA DE CANDIDATOS----------", 1, 0);
    b2.AddNewLine(1);
    const SharedPaperForm tituloLista = b2.Build();

    auto candidatos = std::make_shared<comum::CParteCandidatos>(
        std::make_shared<api::CFormPart>(headerCargo), std::make_shared<api::CFormPart>(cargoSemCandidato),
        majoritarios, proporcionais, consultas);
    auto cargos = std::make_shared<comum::CParteCargos>(
        std::make_shared<api::CFormPart>(CriaSeparadorEleicao()), candidatos);             // func 5587
    const SharedPaperForm trailer = CriaTrailer(separadorFase,
        comum::CConfiguracaoEleicao::GetInst().GetTextoRodape() /*cfg +200*/,
        estadoGeral.GetCorrespondencia() /*+60*/);                                         // func 5579

    std::vector<SharedFormPart> partes{
        std::make_shared<api::CFormPart>(cabecalho), std::make_shared<api::CFormPart>(tituloLista),
        cargos, std::make_shared<api::CFormPart>(trailer)};
    partes.insert(partes.begin() + 1, extrato.begin(), extrato.end());                    // vota_f2875
    api::CReport relatorio(std::move(partes));                                            // shared_f2259

    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::ZERESIMA, false);      // func 1047 [INÍCIO]
    {
        const std::string arquivo = "ze.dat";
        relatorio.Gera(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / arquivo);
        // ^ inlined: api::CScopedReportFile (creport.cpp:31): IPaperRelatorios::Abre(path) (slot 5);
        //   CReport::Imprime (api_f5486); IPaperRelatorios::Fecha() (slot 6, creport.cpp:37)
        CSincronizaVota::SincronizaRelatorios(arquivo);                                   // func 1836
    }
    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::ZERESIMA, true);       // [TÉRMINO]

    PosGeracao();                          // slot 9 (CRegerarZeresima: CRelVotaUtil::CortaPapel) name inferred
    m_proximoEstado = GetEstadoResumo();   // slot 10
}

// wasm func 11845 — CRegeraResumoZeresima vtable slot 10 (after regenerating the summary, reprint).
comum::CAppState* CRegeraResumoZeresima::GetProximoEstado()
{
    return &CReimprimindoZeresima::GetInst();                                   // func 5941
}

// =================================================================================================
// Appendix: helpers of other files that the tools placed near this one (library-like or comum).
// =================================================================================================

// wasm func 5587 — comum::CParteCargos::CParteCargos(SharedFormPart separadorEleicao, SharedFormPart parte)
//   (inline constructor instantiated here: vptr @1576564, +4/+8 and +12/+16 = the two shared_ptrs moved in)
// wasm func 2877 — std::map<K, std::vector<{int, std::string}>>::__tree::destroy (the copy of
//   CRdvVota's votes map scanned by CriaTituloExtratoRDV). library instantiation.
// wasm func 601  — api::CFormPart::CFormPart(SharedPaperForm&&) { vptr @1583696; m_form = std::move(f); }
// wasm func 604  — api::CPaperFormBuilder::AddData(std::string (*fonte)(), int flags):
//                  push_back(make_shared<CTextFieldPaper>(make_shared<CDataText<std::string(*)()>>(fonte, flags), 1))
// wasm func 198  — api::CPaperFormBuilder::AddNewLine(int n): push_back(make_shared<CNewLineFieldPaper>(n))
//                  (through func 3890, the shared "push a new field" body)
// wasm func 3699 — CriaHeaderCargo(separadorFase) (comum; also inlined in CGeraBU): AddText(sep, 1, 0),
//                  AddNewLine(1), AddNomeCargo(CPadDS<CToUpperDS<CCargoDSNome>>{'-', 38, slot 2920})
//                  (func 5613), Build().                                              name inferred
// wasm func 1541 — comum::CRelUtil::IncluiResumoCorrespondencia(builder, correspondencia):
//                  AddText("RESUMO DA CORRESPONDÊNCIA", 1, 2); AddText(<formatted correspondence,
//                  api_f2792>, 2, 2); AddNewLine(1).                                  name as in unit u08
// wasm func 5977 — vota::CRelVotaUtil::CriaCabecalho(const std::string&): see
//                  src/uenux2/src/app/vota/comum/crelvotautil.u09.cpp

}  // namespace vota

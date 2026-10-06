// Reconstructed from vota_web_wasm.wasm (unit u08).
// Original: uenux2/src/app/vota/eleitor/fimvotacao/cgerarelatorios.cpp
//
// As for cgerabu.cpp, the single surviving function (CGeraRelatorios::StartState, wasm func 12105,
// 18.8 KB) has the report generators of comum inlined into it. srcloc records found inside:
//   cgerarelatorios.cpp:51   Assert (poInfo.GetVota().GetEstadoVota() == EAVGERARRELATORIOS)   (3455)
//   cgeradorbuj.cpp:189      void comum::CGeradorBUJ::GeraBUJ(const std::string &)   (IPaperRelatorios)
//   cdatamap.h:117           CDataMap<CNumeroInscricaoEleitoral, CJustificadorDetalhe>::Next()
//                            "Operação inválida {}" (EUeIoError 5977)
//   cestadogeralvota.h:90/106  GetDtHrInicioAquisicao / GetDtHrFimAquisicao (8090 / 8091), twice
//   cimprimiridentificacaomesariosfinal.h:41   CImprimirIdentificacaoMesariosFinal::GetPrinter()
// Not executed in the recorded sessions (encerramento is unreachable from the web page).
//
// Note: the report strings of comum/relatorios are Latin-1 in the binary (the printer's encoding).

#include "vota/eleitor/fimvotacao/cgerarelatorios.h"

#include <algorithm>
#include <format>
#include <map>
#include <source_location>
#include <string>
#include <vector>

#include "api/gui/cpaperformbuilder.h"
#include "comum/cappinfo.h"
#include "comum/comparecimentomesario/estados/cimprimiridentificacaomesariosfinal.h"
#include "comum/cpath.h"
#include "comum/dados/cconfiguracaoeleicao.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/clocal.h"
#include "comum/informacao/cinformacaoeleicao.h"
#include "comum/relatorios/cgeradorbuj.h"
#include "comum/relatorios/crelutil.h"
#include "vota/eleitor/fimvotacao/ciniciobu.h"
#include "vota/log/clogvota.h"
#include "vota/sincronismo/csincronizavota.h"

namespace vota {

using comum::md::estadoaplicacao::EEstadoVota;

namespace {

// Each report: log "Gerando relatório [X] [INÍCIO]", error context, render into
// GetPathTrab(INTERNA)/<arquivo>, mirror it (before the context is destroyed), log "[TÉRMINO]".
// Used for BIM and BEHB, whose writers are set up inside the error context (BUJ builds its generator
// before it, see below). Written out in the binary (inlined); factored here only for readability.
// name inferred
template <typename Gera>
void GeraRelatorio(comum::ERelatoriosUE tipo, const char* gerando, const char* erro,
                   const std::string& arquivo, Gera&& gera)
{
    CLogVota::GetInst().LogaGeracaoRelatorio(tipo, false);                 // func 1047
    {
        api::CApplicationContextGuard contexto(2, "", gerando, erro);
        gera(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / arquivo);   // func 436
        CSincronizaVota::SincronizaRelatorios(arquivo);
    }                                                                      // func 675
    CLogVota::GetInst().LogaGeracaoRelatorio(tipo, true);
}

}  // namespace

// wasm func 12105 — vtable slot 2
void CGeraRelatorios::StartState()
{
    // The variable is really called poInfo here: the stringified assert reads
    // "Assert (poInfo.GetVota().GetEstadoVota() == EAVGERARRELATORIOS)" (the other files use appInfo).
    auto& poInfo = comum::CAppInfo::GetInst();
    UE_ASSERT(poInfo.GetVota().GetEstadoVota() == EEstadoVota::EAVGERARRELATORIOS);    // :51 (60)

    m_telaPreparandoDados->Exibe();

    // ---- 1. Boletim de Justificativa (always) ----------------------------------------------------
    // Written out (not through GeraRelatorio) because the binary builds the CGeradorBUJ BEFORE the
    // error context: an exception of the ctor (8090/8091) escapes without the "Gerando ..." context.
    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::BUJ, false);       // func 1047, 4
    {
        const auto& estadoGeral = poInfo.GetEstadoGeral();
        comum::CGeradorBUJ gerador(comum::CRelUtil::GetSeparadorFase(estadoGeral.GetFase()));
        // ^ inlined ctor builds four forms:
        //   header : IncluiCabecalhoEleicoesMZS(b, "Boletim de Justificativa Eleitoral", municipio,
        //            zona, secao (EstadoGeral +20/+24/+26), nomeMunicipio, 0, false); data 1567;
        //            if TemVota(turno): IncluiDatasAberturaFechamento (func 3688, 8090/8091 when unset);
        //            IncluiResumoCorrespondencia (func 1541)
        //   total  : separadorFase, nl, CDataTextFmt(2948, "{:05} Justificativas"), nl
        //   detalhe: data 2949 (one justificativa: the current entry of the justificativas CDataMap)
        //   trailer: nl, separadorFase, nl, "Código de identificação da carga", GetIDCargaFormatado,
        //            nl, "Ver: 10.23.0.1", nl, "ASSINATURAS:", nl, "PRESIDENTE:", 2 nl, "MESÁRIOS:",
        //            3 nl, "FISCAIS:", 20 nl, paper cut
        api::CApplicationContextGuard contexto(2, "", "Gerando boletim de justificativa na MI",
            "Ocorreu um erro durante a geração do boletim de justificativa na MI.");
        const std::string arquivo = "buj.dat";
        gerador.GeraBUJ(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / arquivo);   // func 436
        // ^ cgeradorbuj.cpp:189: papel = IPaperRelatorios::GetInst(); papel.Abre(caminho);
        //   header->Imprime(); total->Imprime();
        //   for (auto& j = CJustificativas::GetInst() /* func 1391 */; j.First(); !j.End(); j.Next())
        //       detalhe->Imprime();                  // Next(): "Operação inválida {}" past the end
        //   trailer->Imprime(); papel.Fecha(); CSynchronizer::CreateInst()->Sincroniza();
        CSincronizaVota::SincronizaRelatorios(arquivo);
    }   // ~CApplicationContextGuard (func 675), then ~CGeradorBUJ (four shared_ptr forms)
    CLogVota::GetInst().LogaGeracaoRelatorio(comum::ERelatoriosUE::BUJ, true);

    // ---- 2. Boletim de Identificação de Mesários --------------------------------------------------
    if (comum::CInformacaoEleicao(comum::CConfiguracaoEleicao::GetInst()).IdentificaMesarios()) {
        // func 1950 (analyzer: CInformacaoEleicao::EhModoDemonstracao, which it inlines):
        //   return !EhModoDemonstracao() && flag at CConfiguracaoEleicao +488
        GeraRelatorio(comum::ERelatoriosUE::BIM,                                       // 9
                      "Gerando boletim de mesários na MI",
                      "Ocorreu um erro durante a geração do boletim de mesários na MI.",
                      "bim.dat", [&](const std::string& caminho) {
            comum::CImprimirIdentificacaoMesariosFinal bim(/*final*/ true);   // {red=false, final=true, emissao=false}
            bim.Imprime(caminho);
            // ^ inlined: title "Boletim de Identificação de Mesários" (prefixed by
            //   "Sistema Recuperador de Dados\n" in RED mode); IncluiCabecalhoEleicoesMZS(..., !final, false);
            //   data 1567; if TemVota && final: datas de abertura/fechamento; if emissao: "Data da emissão"
            //   / "Hora da emissão" = now; resumo da correspondência;
            //   "Mesários registrados na abertura" (or "Não houve registro na abertura"): one line per
            //   comparecimento_mesario row of type != 2 (+ GetNomeMesario when the name is known);
            //   "======"; "Mesários registrados no fechamento" (or "Não houve registro no fechamento"):
            //   rows of type != 1, each with "Nome:" (when unknown) and "Assinatura:", separated by
            //   dashed lines; "======"; código de identificação da carga; "Ver: ..."; cut.
            //   Rows come from api::persistencia::CDAORepositorio::Entregar<IComparecimentoMesarioDAO>()
            //   (func 815, SQLite table comparecimento_mesario in uenux.db). Printed through
            //   GetPrinter() (h:41) = IPaperRelatorios: Abre(caminho) ... Fecha().
        });
    }

    // ---- 3. Eleitores com habilitação biográfica -------------------------------------------------
    if (!comum::CInformacaoEleicao::EhModoDemonstracao() &&                       // func 2286
        comum::CLocal::GetInst().UrnaBiometrica()) {                              // func 820
        GeraRelatorio(comum::ERelatoriosUE::ELEITORES_HABILITADOS_BIOGRAFICAMENTE,  // 12
                      "Gerando relatório de eleitores habilitados biograficamente",
                      "Ocorreu um erro durante a geração do relatório de eleitores habilitados "
                      "biograficamente na MI.",
                      "behb.dat", [&](const std::string& caminho) {
            api::CScopedReportFile arquivo(caminho);      // opens IPaperRelatorios on `caminho`
            // Inlined report ("relatório de eleitores habilitados biograficamente"):
            //   title "Eleitores com habilitação biográfica" (prefixed by "Sistema Recuperador de Dados\n" in RED
            //   mode, flag of the zero-initialised object built by func 2262); IncluiCabecalhoEleicoesMZS; lambda header
            //   (MontaCabecalho, std::function) with the aptos line; "Data da emissão"/"Hora da emissão"
            //   (now); resumo da correspondência; separator of the phase (func 2785);
            //   if CEleitores::QtdHabilitados(2) == 0: "Nenhum eleitor passou por habilitação biográfica"
            //   else: for every voter of every section (CLocal::GetTodasSecoes: principal + agregadas +
            //   "Transferência temporária") whose dynamic data says "voted with biographic habilitação",
            //   collect {número, nome} per section into std::map<SChaveSecao, std::vector<SEleitor>>
            //   (vector push: func 5602), sort each vector by número (std::sort: funcs 5600/5599/2789),
            //   print "Seção agregada: {:04}" / "Transferência temporária" headings, data 2961 and the
            //   entries; destroy the map (func 3695).
            //   Then código de identificação da carga (func 2252), "Ver: ...", cut.
        });
    }

    poInfo.GetVota().SetEstadoVota(EEstadoVota::EAVIMPRIMIRBU);                   // 61
    comum::SalvaEstado();                                                         // func 491
    m_proximoEstado = &CInicioBU::GetInst();                                      // func 5983
}

// =================================================================================================
// comum helpers that the tools attributed to this file (paths inferred)
// =================================================================================================
namespace comum {

// wasm func 1542 — crelutil.cpp (?): "<rótulo data>          DD/MM/YYYY" and
// "<rótulo hora>            hh:mm:ss" lines. Used by BU, BUJ, BIM, BEHB, PU, lista de eleitores and
// CGeradorRelVersaoPacoteDados. name inferred
void IncluiDataHora(api::CPaperFormBuilder& b, const api::CDateTime& dataHora,
                    const std::string& rotuloData, const std::string& rotuloHora)
{
    b.AddText(CRelUtil::CompletaDireita(rotuloData, 28) + dataHora.GetDate().Format("DD/MM/YYYY"), 1, 0);  // func 706
    b.AddText(CRelUtil::CompletaDireita(rotuloHora, 28) + dataHora.GetTime().Format("  hh:mm:ss"), 1, 0);  // func 779
}

// wasm func 2252 — crelutil.cpp (?): codigoCarga of the correspondência as seven groups of 3 plus the
// rest, joined by '.' ("abc.def.ghi.jkl.mno.pqr.stu.vwx" for the usual 24 characters). It is ONE
// unrolled expression, not a loop: the 8th piece is substr(21) (unbounded), and a code shorter than
// 21 characters throws std::out_of_range from the first substr whose pos > size(). Same expression
// as CRelUtil::GetIDCargaFormatado(const std::string&) (func 2786, crelutil.cpp:69), which first
// checks size() > 20 and throws "ID de carga inválido: [...]" instead. name inferred
std::string GetIDCargaFormatado(const md::estadoaplicacao::CDadoCorrespondencia& correspondencia)
{
    const std::string& codigo = correspondencia.GetCarga().codigoCarga;          // +28
    return codigo.substr(0, 3) + "." + codigo.substr(3, 3) + "." + codigo.substr(6, 3) + "." +
           codigo.substr(9, 3) + "." + codigo.substr(12, 3) + "." + codigo.substr(15, 3) + "." +
           codigo.substr(18, 3) + "." + codigo.substr(21);
}

// wasm func 2785 — crelutil.cpp (?): the phase separator line as a centred, styled text. name inferred
void IncluiSeparadorFase(api::CPaperFormBuilder& b, EUrnaFase fase)
{
    (void)CInformacaoEleicao(CConfiguracaoEleicao::GetInst());    // leftover of an inlined ctor
    b.AddText(CRelUtil::GetSeparadorFase(fase), 1, 2);
}

}  // namespace comum

// Library / template instances that the tools attributed to this file:
//   wasm func 5602  std::vector<SEleitorBEHB>::push_back(value_type&&) (16-byte {int numero; std::string nome})
//   wasm func 5600  std::__introsort<..., SEleitorBEHB*>  (std::sort by numero; depth 2*log2(n))
//   wasm func 5599  std::__insertion_sort_incomplete<...>
//   wasm func 2789  std::__sort4<...>
//   wasm func 3695  std::__tree<...>::destroy(node*) for std::map<SChaveSecao, std::vector<SEleitorBEHB>>
//                   (node 40 bytes: key {u16 secao, int tipo, bool} + vector at +28)

}  // namespace vota

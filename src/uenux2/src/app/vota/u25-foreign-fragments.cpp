// FRAGMENTS reconstructed by unit u25 (uenux2/src/app/comum/relatorios) from vota_web_wasm.wasm.
// Functions of the vota application that the unit builder put in u25 (they call report helpers of
// comum/relatorios). Each block names its original file ("(path inferred)" when so).

// =====================================================================================================
// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp  (units u07 / u22)
// =====================================================================================================
// wasm func 6627 (tools: vota_f6627, observed executing: every voting screen):
//   std::make_shared<api::CDataText<DS_NomeCargoNeutroComEscolha>>(DS_NomeCargoNeutroComEscolha{cargo})
//   = std::allocate_shared: one 160-byte block {__shared_ptr_emplace vtable @1538008, counters,
//   CDataText vtable @1538048, copy of the 140-byte md::CCargo (optional details copied by 365 / 374)}.
// wasm func 12589 / 12588 (vtable @1538048 slots 0 / 1): ~CDataText<DS_NomeCargoNeutroComEscolha>()
//   (complete / deleting): resets the two optional details of the copied cargo (267 / 242).

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/auxiliares/cmenufiltrarcandidatospornumero.cpp  (path inferred)
// "Visualização de candidatos" (candidate browser offered to the voter before voting; unit u09 wrote the
// sibling CMenuFiltrarCandidatosPorCargo).
// =====================================================================================================
namespace vota {

// wasm func 11892 (tools: vota::CMenuFiltrarCandidatosPorNumero::vf2 = StartState). Builds the number-entry
// screen "telaFiltragemCandidatosPorNumero" for the cargo chosen in the previous step and keeps it in
// m_tela (+12); input is handled by the other states of the menu.
void CMenuFiltrarCandidatosPorNumero::StartState()
{
    const comum::md::CCargo cargo =                                             // copy (365 / 374)
        comum::CConfiguracaoEleicao::GetInst().GetCargo(*CVisualizarCandidatos::GetInst().m_cargo);   // 1279, 5951, 861

    api::CFormBuilder b;
    b.AddStatusHeader(5);                                                       // func 502
    b.AddLabel("Visualização de candidatos", {320, 40}, FONTE_TITULO /*474888*/, 2, 2, 1);            // api_f202
    b.AddLabel("Filtragem por número do candidato", {320, 75}, FONTE_TEXTO /*474992*/, 2, 2, 1);
    b.AddLabel("Número do " + cargo.GetNome(comum::md::CSexo::ESexo{1}) + ": ", {320, 215},
               FONTE_TEXTO, 2, 2, 1);                                           // func 2796, ": " @445417
    b.AddNumberInput(cargo.GetNumeroDigitos() /* +12 */, 1, 0, 0, 1, 0, {320, 265}, FONTE_TITULO, 2);   // api_f2383 ?
    b.AddLabeledInputControl({{'C', "Confirmar seleção"}, {'D', "Retornar"}});  // func 653
    m_tela = api::CriaFormInterativo(b, "telaFiltragemCandidatosPorNumero");    // vota_f576
}

} // namespace vota

// =====================================================================================================
// uenux2/src/app/vota/operador/outrasopcoes/cimpressaopu.cpp  (path inferred; RTTI vota::CImpressaoPU,
// vtable slot 2 = StartState). "Parâmetros de urna" report of the "Mais informações" menu, generated with
// comum::CGeradorRelPU (vtable @1545288: slots 11901 / 11900) — a line-based report whose body is inlined here.
// =====================================================================================================
namespace vota {

namespace {
// wasm func 2686 (tools: vota_f2686). Formats a std::chrono::microseconds time-of-day as "{:02}:{:02}:{:02}"
// (hours may exceed 23; the extreme values of the representation map to ±2562047788:±54:...).
std::string FormataHorario(std::chrono::microseconds horario);   // name inferred
// wasm func 2687: the matching end-of-window formatter (other unit).
} // namespace

// wasm func 11902 (tools: vota::CImpressaoPU::vf2), summarised: 6.5 KB of inlined report code.
void CImpressaoPU::StartState()
{
    auto& appInfo = comum::CAppInfo::GetInst();
    comum::CGeradorRelPU relatorio;                                              // on the stack
    CLogVota::GetInst().Loga("Imprimindo parâmetros de urna");                   // api_f233
    const std::string via = std::format("{}ª via", appInfo.GetVota().GetNumViasPU() /* +76 */ + 1);

    const auto& local = comum::CLocal::GetInst();
    const auto& estadoGeral = appInfo.GetGeral();
    auto& b = relatorio.Builder();                                               // CGeradorRelPU +4
    comum::CRelUtil::IncluiCabecalhoEleicoesMZS(
        b, "Parâmetros de urna", local.GetMunicipio(), local.GetZonaID(), local.GetSecaoID(),
        local.GetNomeMunicipio(),
        estadoGeral.GetTipoUrnaTurnoAtual() == '2' ? comum::ETipoCabecalho::CONTINGENCIA : comum::ETipoCabecalho::SECAO,
        true);                                                                   // func 1543
    b.AddNewLine(1);
    b.AddText(comum::CRelUtil::DSCodigoIdentificacaoUE(), 1, 0);                  // func 1942
    comum::CRelUtil::IncluiDataHora(b, api::CDateTime::Agora(), "Data", "Hora"); // func 1542
    b.AddNewLine(1);
    comum::CRelUtil::IncluiResumoCorrespondencia(b, estadoGeral.GetCorrespondencia());   // func 1541
    b.AddNewLine(1);
    IncluiSeparadorFase(b, estadoGeral.GetFase());                               // func 2785
    b.AddNewLine(1);

    // Voting-day time windows of the configuration (4 x microseconds at CConfiguracaoEleicao +496..),
    // printed "<rótulo>   hh:mm:ss -" / "hh:mm:ss" with CGeradorRelPU::AdicionaLinha (vota_f677:
    // rótulo + PadLeft(valor, ' ', 38 - rótulo.size())). The pleito used is pleito 2 for a contingency
    // urna on/after the 2nd-round date (same test as IncluiCabecalhoEleicoesMZS).
    //   "Emissão de zerésima", "Início da votação", "Encerramento da votação", "Término compulsório"
    // Copies and flags of the -pu.dat (CParametrosUrna at cfg +88):
    //   "Zerésimas" (+96), "BUs do Vota obrigatórios" (CInformacaoEleicao, demo = 1), "BUs do Vota adicionais"
    //   (+104), "BUs do RED obrigatórios" (+108), "BUs do RED adicionais" (+112), "BUs do SA obrigatórios"
    //   (+124), "BUs do SA adicionais" (+128), "Aceitar justificativa" (+482), "Apresentar partido" (+484),
    //   "Imprimir QR Code no BU" (+486), "Registrar mesários" (+488), "Pedir ano nascimento" (+489)
    //   ("Sim"/"Não"), "Tempo desligamento auto" (+180), "Tempo aviso desligamento auto" (+184).
    relatorio.IncluiRodape();       // shared body 5596: "Código de identificação da carga" + "Ver: {}"
    relatorio.Imprime(via);         // func 3671
    ++appInfo.GetVota().NumViasPU();
    comum::SalvaEstado();           // func 491
    m_proximoEstado = api_f1280(0); // back to the menu
}

} // namespace vota

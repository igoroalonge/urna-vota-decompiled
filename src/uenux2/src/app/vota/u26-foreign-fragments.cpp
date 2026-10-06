// FRAGMENTS reconstructed by unit u26 from vota_web_wasm.wasm: functions of uenux2/src/app/vota/** files owned by
// other units (or of classes without an attested file) that the tools filed in unit u26. Merge each block into
// the file named in its header ("path inferred" = no std::source_location record names the file).
//
// None of these ran in the recorded votes: they belong to the start-of-day menus ("Mais informações"), the
// zerésima flow, the operator terminal and the printed reports, which the web page never reaches.
#include <format>
#include <memory>
#include <mutex>
#include <string>

namespace vota {

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cmaisinformacoes.cpp (path inferred)
// vota::CMaisInformacoes (typeinfo @1545348, vtable @1545312): the "Mais informações" menu (print the
// estado-da-urna report, the voter list, the package versions, the PU...; slots 2 = 11897, 7 = 11896).
// =====================================================================================================

// wasm func 1280 (tools: api_f1280). Lazy singleton @1834520 (24 bytes: CAppState flags 2, +12..+23 zeroed,
// +20 = the state to return to). The argument, when not null, becomes the "back" state.    name inferred
CMaisInformacoes& CMaisInformacoes::GetInst(comum::CAppState* retorno)
{
    static std::mutex s_mutex;                                  // @1834496
    static std::unique_ptr<CMaisInformacoes> s_instancia;       // @1834520
    std::lock_guard trava(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CMaisInformacoes());              // CAppState(2), m_retorno = nullptr
    if (retorno)
        s_instancia->m_retorno = retorno;                       // +20
    return *s_instancia;
}

// =====================================================================================================
// uenux2/src/app/vota/comum/crelvotautil.cpp (path inferred; fragment of unit u09 in crelvotautil.u09.cpp)
// =====================================================================================================

// wasm func 2882. Advances the paper and cuts it (2 blank lines + cut mark), as an untitled print job.
// Callers: CRegerarZeresima slot 9 (11842), CQuerReimprimirZeresima::ProcessInput (11848),
// CImprimindoZeresima::StartState (11991).
void CRelVotaUtil::CortaPapel()
{
    api::CPaperFormBuilder builder;
    builder.AddNewLine(2);                                      // comum_f198
    builder.AddCut();                                           // comum_f1264
    api::CSubReport relatorio("", "", false);                   // func 3660
    api::CLp::GetInst().Imprime(relatorio, [&builder] { builder.Renderiza(); });   // 3876 / 3875; lambda vtable @1543240
}

// wasm func 11842 (vtable vota::CRegerarZeresima slot 9 = the "after printing" hook of CGeraZeresimaBase).
void CRegerarZeresima::AposImpressao()                          // name inferred (u07: "post-print hook")
{
    CRelVotaUtil::CortaPapel();
}

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/ciniciozeresima.cpp (path inferred)
// =====================================================================================================

// wasm func 3865: CInicioZeresima::GetInst() = merged lazy-singleton body vota_f764(mutex @1834272,
// &instance @1834296, vtable @1544792, CAppState flags 0). Callers: CAjusteInicial::ValidaTemposDesligamento,
// CVerificaHorarioZeresima::StartState / ProcessTickNaoDesligamento.                     name as in unit u06

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp (path inferred)
// vota::CInicioVotacao (typeinfo @1543636, vtable @1543600): waits for the opening time of the vote, then
// hands the voter terminal to CAguardaMensagem (the idle state that waits for the mesário to enable a voter).
// Layout: +12/+16 and +20/+24 two screens (shared_ptr), +28 CDateTime horário de início, +40 uebyte tick.
// =====================================================================================================

namespace {
// wasm func 5972 (tools: vota_f5972; name inferred). Start of the vote acquisition.
void IniciaVotacao()
{
    int tela = 140;                                                                 // CTelasVota +140
    if (comum::CRdvVota::GetInst().Comparecimento() == 0) {                         // shared_f1269: no vote yet
        comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual).MarcaInicioAquisicao();   // wasm_entry_f5627 (dhIniAquisicao)
        comum::SalvaEstado();                                                       // func 491
        tela = 132;                                                                 // CTelasVota +132
    }
    CTelasVota::GetInst().GetTela(tela)->Exibe();       // +132 first opening / +140 resumed (func 2380 screens)
    CThreadOperador::GetInst().GetFila().Envia(api::SMessage{8}, 1);                // operator queue +36, rhvoice_f501 (?)
}
} // namespace

// wasm func 11988 (vtable slot 2)                                                          name inferred
void CInicioVotacao::StartState()
{
    const api::CDateTime agora;                                                     // func 479
    if (agora.Compare(m_horarioInicio) < 0) {                                       // func 759
        CThreadEleitor::GetInst().StartTick(m_tick);
        m_telaA->Exibe();                                                           // +12
        m_telaB->Exibe();                                                           // +20
        m_proximoEstado = this;
        return;
    }
    m_proximoEstado = &CAguardaMensagem::GetInst();                                 // func 1337
    IniciaVotacao();
}

// wasm func 11987 (vtable slot 8 = ProcessTick)                                            name inferred
void CInicioVotacao::ProcessTick(uebyte tick)
{
    if (tick != m_tick)
        return;
    const api::CDateTime agora;
    if (m_horarioInicio.Compare(agora) >= 0)            // note: strictly after the opening time here
        return;
    m_proximoEstado = &CAguardaMensagem::GetInst();
    IniciaVotacao();
}

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cconfirmaimpressaozeresima.cpp (path inferred)
// vota::CConfirmaImpressaoZeresima: "print the zerésima?" (CONFIRMA) / "Mais informações" (BRANCO).
// =====================================================================================================

// wasm func 11927 (vtable slot 7)                                                          name inferred
void CConfirmaImpressaoZeresima::ProcessInput()
{
    switch (m_tela->Read()) {                                   // +12; cinteractiveform.h:57
    case api::EInputResult::Branco:
        m_proximoEstado = &CMaisInformacoes::GetInst(this);
        break;
    case api::EInputResult::Confirma: {
        const api::CDateTime agora;
        if (agora.Compare(m_limiteZeresima) > 0) {              // +20: too late for a normal zerésima
            // lazy singleton @1834240: 20 bytes, CAppState(2), screen = CTelasVota +60/+64
            m_proximoEstado = &CImpressaoZeresimaTardia::GetInst();
        } else {
            comum::CAppInfo::GetInst().GetVota(comum::EUrnaTurno::Atual)
                .SetEstadoVota(comum::md::estadoaplicacao::EEstadoVota('4'));   // 52
            comum::SalvaEstado();
            m_proximoEstado = &CGeraZeresima::GetInst();       // func 5962 (lazy @1834212)
        }
        break;
    }
    default:
        break;                                                  // m_proximoEstado unchanged
    }
}

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaoestadourna.cpp (path inferred)
// vota::CImpressaoEstadoUrna (typeinfo @1545100, vtable @1545064): item of "Mais informações".
// =====================================================================================================

// wasm func 11911 (vtable slot 2). Unlike CVerificaHorarioZeresima there is no limit on the number of copies.
void CImpressaoEstadoUrna::StartState()
{
    auto& appInfo = comum::CAppInfo::GetInst();
    comum::CRelatorioTesteImpressora relatorio;                                        // func 5586
    CLogVota::GetInst().LogaImprimindoRelatorioEstadoUrna();                           // func 5880
    auto& vias = appInfo.GetVota(comum::EUrnaTurno::Atual).GetNumViasImpressas();     // +73
    relatorio.Imprime("estado da urna", std::format("{}ª via", vias.estadoUrna + 1));   // func 5591
    ++appInfo.GetVota(comum::EUrnaTurno::Atual).GetNumViasImpressas().estadoUrna;
    comum::SalvaEstado();                                                              // func 491
    m_proximoEstado = &CMaisInformacoes::GetInst(nullptr);                             // func 1280
}

// =====================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cimpressaolistaeleitores.cpp (path inferred)
// vota::CImpressaoListaEleitores (typeinfo @1545156, vtable @1545120): item of "Mais informações".
// =====================================================================================================

// wasm func 11908 (vtable slot 2; 4.7 KB: the report parts are inlined).                     condensed
void CImpressaoListaEleitores::StartState()
{
    auto& appInfo = comum::CAppInfo::GetInst();
    CLogVota::GetInst().Loga("Imprimindo lista de eleitores");
    auto& vota = appInfo.GetVota(comum::EUrnaTurno::Atual);
    const std::string via = std::format("{}ª via", vota.GetNumViasImpressas().listaEleitores + 1);   // +74

    api::CReport relatorio;                                  // list of api::CFormPart (vtable @1544112 parts)
    {   // comum::(anonymous namespace)::MontaCabecalho() (inlined; its std::function lambda vtable @1576132)
        auto& local = comum::CLocal::GetInst();
        const auto& geral = appInfo.GetGeral();
        const api::CDateTime agora;
        api::CPaperFormBuilder b;
        comum::CRelUtil::IncluiCabecalhoEleicoesMZS(b, "Lista de eleitores", local.GetMunicipio(),
                                                   local.GetZonaID(), local.GetSecaoID(),
                                                   local.GetNomeMunicipio(), 0, 0);   // func 1543
        b.AddText(/* MontaCabecalho lambda: comum_f1921(comum_f1922(...)) */ TextoCabecalho(), 1, 0);
        b.AddNewLine(1);
        b.AddText(comum::CRelUtil::TextoIdentificacao(), 1, 0);                        // func 1942
        comum::CRelUtil::IncluiDataHora(b, agora, "Data da emissão", "Hora da emissão");   // func 1542
        b.AddNewLine(1);
        comum::CRelUtil::IncluiResumoCorrespondencia(b, geral.GetCorrespondencia());   // func 1541 (+60)
        b.AddNewLine(1);
        comum::CRelUtil::IncluiSeparadorFase(b, geral.GetFase());                     // func 2785 (+48)
        b.AddNewLine(1);
        relatorio.Adiciona(api::CFormPart(std::move(b)));                             // vota_f601 / shared_f357
    }
    {   api::CPaperFormBuilder b; b.AddData(/* table slot 2970 */, 0); b.AddNewLine(1);   // func 604: column titles ?
        relatorio.Adiciona(api::CFormPart(std::move(b))); }
    {   api::CPaperFormBuilder b; b.AddData(/* table slot 2971 */, 0);
        relatorio.Adiciona(api::CFormPart(std::move(b))); }
    relatorio.Adiciona(std::make_shared<comum::CParteEleitores>(/* ... */));          // the voter lines (vtable @1576596)
    {   // legend of the flags printed next to each voter
        api::CPaperFormBuilder b;
        b.AddNewLine(1);
        comum::CRelUtil::IncluiTracejado(b, 2);                                        // func 1387 (38 x '-')
        b.AddText("AUD: Áudio", 1, 0);
        b.AddText("BIO: Biometria", 1, 0);
        b.AddText("IMP: Impedido", 1, 0);
        b.AddText("TTE: Transferência Temporária", 1, 0);
        b.AddNewLine(1);
        relatorio.Adiciona(api::CFormPart(std::move(b)));
    }
    {   // footer
        api::CPaperFormBuilder b;
        b.AddNewLine(1);
        comum::CRelUtil::IncluiSeparador(b);                                           // func 1156
        b.AddText("Código de identificação da carga", 1, 2);
        b.AddText(comum::CRelUtil::FormataCodigoCarga(appInfo.GetGeral().GetCorrespondencia()), 1, 2);   // func 2252
        b.AddNewLine(1);
        comum::CRelUtil::IncluiSeparador(b);
        b.AddText("Ver: " + api::CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO"), 1, 0);
        b.AddNewLine(2);
        b.AddCut();
        b.AddNewLine(4);
        relatorio.Adiciona(api::CFormPart(std::move(b)));
    }
    api::CSubReport sub("Lista de eleitores" /* ? */, via, true);                     // func 3660
    api::CLp::GetInst().Imprime(sub, [&relatorio] { DoPrintJob(relatorio); });        // func 3875; lambda @1583760

    ++vota.GetNumViasImpressas().listaEleitores;
    comum::SalvaEstado();
    m_proximoEstado = &CMaisInformacoes::GetInst(nullptr);
}

// =====================================================================================================
// Operator-terminal states (uenux2/src/app/vota/operador/..., owner: unit u10)
// =====================================================================================================

// wasm func 10416 (vtable vota::CPerguntaEleitorVotando slot 2 = StartState)             name inferred
void CPerguntaEleitorVotando::StartState()
{
    CLogVota::GetInst().LogaAviso("Mesário indagado se eleitor está votando");     // api_f1398 (severity 2)
    m_form->Show();                                                                 // +12, slot 2
    m_proximoEstado = this;
}

// wasm func 10642 (vtable vota::CEleitorJaVotou slot 2 = StartState)                     name inferred
void CEleitorJaVotou::StartState()
{
    CLogVota::GetInst().LogaAviso("O eleitor identificado já votou");
    m_form->Show();
    m_proximoEstado = this;
}

// wasm func 10774 (vtable vota::CControladorRegistraMesariosVota slot 28)                  name inferred
// The maximum number of registered mesários is the constant 6.
void CControladorRegistraMesariosVota::LogaLimiteMesariosAtingido()
{
    CLogVota::GetInst().LogaAviso(std::format("Limite de mesários registrados atingido. Limite máximo: {}", 6u));
}

} // namespace vota

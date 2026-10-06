// FRAGMENTS reconstructed by unit u26 from vota_web_wasm.wasm: functions of uenux2/src/app/comum/** files owned
// by other units (or with no attested file) that the tools filed in unit u26. Merge each block into the file
// named in its header. Report-builder vocabulary (api::CPaperFormBuilder, units u09/u15/u16):
//   AddNewLine(n) = comum_f198, AddText(texto, fonte, alinhamento) = shared_f193 (CFixedText{alinhamento,
//   texto} in a CTextFieldPaper(fonte); alinhamento 0 = esquerda, 2 = centro), AddCut() = comum_f1264,
//   AddQRCode(imagem) = comum_f2775, Show(título, via) = comum_f3671.
#include <format>
#include <string>
#include <vector>

namespace comum {

// =====================================================================================================
// uenux2/src/app/comum/cpath.cpp (owner: unit u22)
// =====================================================================================================
// wasm func 949 (tools: comum_f949; observed executing): CPath::GetPathMR() = GetPathRoot("/dsk/mr/").
// Already reconstructed in src/uenux2/src/app/comum/cpath.cpp.

// =====================================================================================================
// uenux2/src/app/comum/log/ieventoslog.h (owner: unit u24)
// =====================================================================================================
// wasm func 1398 (tools: api_f1398): the severity-2 ("aviso") copy of IEventosLog::Loga:
//   void IEventosLog::LogaAviso(const std::string& texto) const { api::CLoga::loga(m_aplicativo, 2, texto); }
// Callers: CPerguntaEleitorVotando, CPerguntaCodigoSuspensao, CControlaReconhecimento, CEleitorJaVotou,
// CPedeIdentidade, CControladorRegistraMesariosVota, api_f6115.                     name inferred

// =====================================================================================================
// uenux2/src/app/comum/relatorios/crelutil.cpp (path inferred by unit u09)
// =====================================================================================================

// wasm func 1156 (tools: comum_f1156). A 38-character "=====" line, centred.         name inferred
// Callers: CGeraBU (12110), CGeraRelatorios (12105), CImpressaoListaEleitores (11908), the estado-da-urna
// report (5591), CGeradorRelVersaoPacoteDados (5596).
void CRelUtil::IncluiSeparador(api::CPaperFormBuilder& builder)
{
    builder.AddText(std::string(38, '='), 1, 2);
}

// =====================================================================================================
// uenux2/src/app/comum/relatorios/crelatoriotesteimpressora.cpp (the other methods belong to another unit)
// =====================================================================================================

// wasm func 5586 (tools: comum_f5586). Constructor. Callers: CVerificaHorarioZeresima::ProcessInput (11869)
// and vota::CImpressaoEstadoUrna::StartState (11911). The flag (+5) selects the second "pleito" of the
// processo eleitoral when this urna is of the kind '2' for the current turno and the date of pleito 2 has
// been reached (slot 14 then returns CPE::GetPleito2() instead of the configuration's pleito).
CRelatorioTesteImpressora::CRelatorioTesteImpressora()
    : CImprimirExtratoCarga()                       // +4 = 0 (m_flag0 ?), vtable @1576660
{
    m_usaPleito2 = false;                           // +5                                         name inferred
    const auto& geral = CAppInfo::GetInst().GetGeral();
    const auto tipo = geral.GetTurno() == '1' ? geral.GetTipoUrnaT1() : geral.GetTipoUrnaT2();   // +36 / +40
    if (tipo != ETipoUrnaOperacao('2'))
        return;
    if (!CPE::Existe())                                                            // comum_f2788
        CPE::CreateInst(geral);
    if (!CPE::GetInst().TemPleito2())                                              // +156
        return;
    const api::CDate hoje;                                                         // api_f1382
    const api::CDate& data = CPE::GetInst().GetPleito2().GetData();               // pleito +16
    if (data == hoje || data.Compare(hoje) < 0)                                    // unknown_f1261
        m_usaPleito2 = true;
}

// =====================================================================================================
// uenux2/src/app/comum/relatorios/cimprimirextratocarga.cpp (path inferred; class comum::CImprimirExtratoCarga,
// typeinfo @1576380, abstract: no vtable of its own; subclass CRelatorioTesteImpressora)
// =====================================================================================================
//
// Hook slots (names inferred from CRelatorioTesteImpressora's implementations 11183..11216):
//   [2] EhTreinamento (fase '3')         [3] TemHorarioVerao (CHV +24)     [4] ImprimeQRCode (always 1)
//   [5] GetConteudoQRCode (11200)         [6] GetConteudosQRCodeAdicionais  [7] GetHorarioVerao (CHV period)
//   [8] EmHorarioVerao (início <= agora < fim)  [9] GetCorrespondencia (copy of CEstadoGeral +60)
//   [10] vf10 (11196, 631 B)              [11] GetTituloCarga (cfg +188)    [12] EhEleicaoComunitaria (cfg +20 == 2)
//   [13] GetNomeEleicao (cfg +8)          [14] GetPleito                     [15] GetLinhasLocal (11188, 3.3 KB)
//   [16] GetUF (CEstadoGeral +8)          [17] GetTipoUE ("SEÇÃO"/"CONTINGÊNCIA", 11190)
//   [18] "URNA OPERANDO EM PERFEITAS"     [19] "CONDIÇÕES DE FUNCIONAMENTO\n"
//   [20] GetSeparadorFase (CRelUtil::GetSeparadorFase(fase))   [21] GetTitulo ("ESTADO DA URNA")
//   [22] EhModoDemonstracao (func 2286)   [23]/[24] hooks (nop here)   [25] AdicionaEmissao (11216:
//   "EMISSÃO DO RELATÓRIO", "DATA: {:.10}       HORA: {:.8}")

// wasm func 5591 (tools: vota_f5591 in cverificahorariozeresima.cpp). The "RELATÓRIO DE ESTADO DA URNA"
// (also called "teste de impressora"): identification of the urna, of the section and of the carga
// (installed election data), daylight-saving forecast and QR code. `via` is e.g. "1ª via".   name inferred
void CImprimirExtratoCarga::Imprime(const std::string& titulo, const std::string& via)
{
    const md::estadoaplicacao::CDadoCorrespondencia correspondencia = GetCorrespondencia();   // [9]
    const std::vector<std::string> linhasLocal = GetLinhasLocal();                            // [15]
    auto& local = CLocal::GetInst();                                                          // func 401

    api::CPaperFormBuilder b;
    b.AddNewLine(1);
    b.AddCut();
    b.AddNewLine(1);
    b.AddNewLine(1);
    if (EhModoDemonstracao())                                                                 // [22]
        CRelUtil::IncluiModoDemonstracao(b);          // func 5583: "IMPRESSO EM MODO DEMONSTRAÇÃO"
    CRelUtil::IncluiTextoComUF(b, GetTituloCarga());  // func 5584: "<uf>" replaced by the section's UF   [11]
    b.AddNewLine(1);
    if (EhEleicaoComunitaria()) {                                                             // [12]
        b.AddText("Eleições Comunitárias", 1, 2);
        b.AddNewLine(1);
    }
    b.AddText(GetNomeEleicao(), 1, 2);                                                        // [13]
    const auto& pleito = GetPleito();                                                         // [14]
    b.AddText(pleito.GetNome(), 1, 2);                                                        // +4
    b.AddText(pleito.GetData().Format("(DD/MM/YYYY)"), 1, 2);                                 // +16
    b.AddNewLine(1);
    if (pleito.GetEleicoes().size() >= 2) {                                                   // 52-byte elements
        for (const auto& eleicao : pleito.GetEleicoes())
            b.AddText(eleicao.GetNome(), 1, 2);                                               // +12
        b.AddNewLine(1);
    }
    CRelUtil::IncluiSeparador(b);                                                             // func 1156
    b.AddText(GetTitulo(), 1, 2);                                                             // [21]
    b.AddText(GetSeparadorFase(), 1, 2);                                                      // [20]
    AdicionaCabecalho(b);                                                                     // [24]
    b.AddText(std::format("UE DE {}\n\n", GetTipoUE()), 1, 2);                                // [17]
    vf10();                                                                                   // [10]
    b.AddText(std::format("UF                                  {}", GetUF()), 1, 2);          // [16]
    b.AddText(std::format("{:33}{:05}", md::CTradutorFrase::TraduzLabel("<MCSN>"), local.GetMunicipio()), 1, 2);
    b.AddText(local.GetNomeMunicipio(), 1, 0);                                                // func 1077
    b.AddText(std::format("{:34}{:04}", md::CTradutorFrase::TraduzLabel("<ZCSN>"), local.GetZonaID()), 1, 2);
    for (const auto& linha : linhasLocal)
        b.AddText(linha, 1, 2);
    b.AddText(std::format("Código identificação UE       {:08}", correspondencia.GetCodigoUE()), 1, 2);      // +0
    b.AddText(std::format("Código identificação MC       {:.8}", correspondencia.GetCodigoMC()), 1, 2);      // +4
    b.AddNewLine(1);
    b.AddText("Código de identificação carga", 1, 2);
    b.AddText(CRelUtil::FormataCodigoCarga(correspondencia), 1, 2);                          // func 2252
    b.AddNewLine(1);
    b.AddText(std::format("Data da carga               {:.10}", correspondencia.GetData().Format("DD/MM/YYYY")), 1, 0);
    b.AddText(std::format("Hora da carga                 {:.8}\n", correspondencia.GetHora().Format("hh:mm:ss")), 1, 0);
    AdicionaEmissao(b);                                                                       // [25]
    b.AddNewLine(1);
    b.AddText("RESUMO DA CORRESPONDÊNCIA", 1, 2);
    b.AddText(std::format("{}\n", ResumoCorrespondencia(correspondencia)), 2, 2);            // api_f2792
    AdicionaResumo(b);                                                                        // [23]
    b.AddText(GetSeparadorFase(), 1, 2);

    if (!EhTreinamento()) {                                                                   // [2]
        if (TemHorarioVerao()) {                                                              // [3]
            const auto& periodo = GetHorarioVerao();                                          // [7]
            b.AddText("Previsão de horário de verão:", 1, 2);
            b.AddText(std::format("Início: {}, fim: {}.", periodo.inicio.Format("DD/MM/YYYY"),
                                  periodo.fim.Format("DD/MM/YYYY")), 1, 2);
            if (EmHorarioVerao())                                                             // [8]
                b.AddText("-> Urna em horário de verão <-", 1, 2);
        } else {
            b.AddText("Não existe previsão de horário de verão.", 1, 2);
        }
        b.AddText(GetSeparadorFase(), 1, 2);
    }

    if (ImprimeQRCode()) {                                                                    // [4]
        b.AddQRCode(api::CQRCodeImagePaper::MontaImagem(GetConteudoQRCode()));                // [5], func 2775
        for (const auto& conteudo : GetConteudosQRCodeAdicionais()) {                         // [6]
            b.AddNewLine(1);
            b.AddQRCode(api::CQRCodeImagePaper::MontaImagem(conteudo));
        }
    }
    b.AddNewLine(1);
    b.AddText(std::format("Ver: {}", std::string_view("10.23.0.1 - DESENVOLVIMENTO")), 1, 0);   // literal @326597
    b.AddNewLine(1);
    b.AddNewLine(1);
    b.AddText(GetRodape1(), 1, 2);                                                            // [18]
    b.AddText(GetRodape2(), 1, 2);                                                            // [19]
    b.AddNewLine(1);
    b.AddText(GetSeparadorFase(), 1, 2);                                                      // [20]
    b.AddText("ASSINATURAS", 2, 2);
    b.AddNewLine(8);
    b.AddCut();
    b.AddNewLine(1);
    b.AddNewLine(1);
    b.AddNewLine(1);
    b.Show(titulo, via);                                                                      // func 3671
}

// =====================================================================================================
// uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (path inferred; fragment of unit u02)
// =====================================================================================================

// wasm func 5592 (vtable slot 2). Caller: vota::CImpressaoVersaoPacotes::StartState (11905).   name inferred
void CGeradorRelVersaoPacoteDados::Imprime(const std::string& titulo, const std::string& via)
{
    MontaCorpo();                   // slot 3, func 11223 (unit u02's name)
    MontaDados();                   // slot 4, func 11222 ("Dados:")                               name inferred
    MontaRodape();                  // slot 5, func 5596
    m_relatorio.Show(titulo, via);  // +4, func 3671
}

// wasm func 5596 (vtable slot 5)                                                             name inferred
void CGeradorRelVersaoPacoteDados::MontaRodape()
{
    auto& b = m_relatorio;                                                          // +4
    b.AddNewLine(1);
    CRelUtil::IncluiSeparador(b);
    b.AddText("Código de identificação da carga", 1, 2);
    b.AddText(CRelUtil::FormataCodigoCarga(CAppInfo::GetInst().GetGeral().GetCorrespondencia()), 1, 2);   // +60
    b.AddNewLine(1);
    CRelUtil::IncluiSeparador(b);
    b.AddText(std::format("Ver: {}", api::CStringUtils::GetVersionNumber("10.23.0.1 - DESENVOLVIMENTO")), 1, 0);
    b.AddNewLine(2);
    b.AddCut();
    b.AddNewLine(4);
}

} // namespace comum

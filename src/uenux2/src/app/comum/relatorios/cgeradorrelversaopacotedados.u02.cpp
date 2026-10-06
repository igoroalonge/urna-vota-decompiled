// uenux2/src/app/comum/relatorios/cgeradorrelversaopacotedados.cpp (path inferred)
// FRAGMENT written by unit u02 (the class belongs to u35). "Versões de Pacotes" report, printed from
// the "Mais informações" menu (item 3, vota::CItemVersoesPacotesVota).
//
// comum::CGeradorRelVersaoPacoteDados (vtable @1576284, 6 slots; slot 5 = wasm 5596).
// The report object (a CReport-like line buffer) is at this+4; shared_f193 = AdicionaLinha(texto, 1, 0),
// comum_f198 = AdicionaLinhasEmBranco(1).

namespace comum {

// wasm func 11223 (vtable slot 3)                                       // name inferred
void CGeradorRelVersaoPacoteDados::MontaCorpo()
{
    auto& local = CLocal::GetInst();
    auto& eg = CAppInfo::GetInst().GetGeral();
    const auto agora = api::CDateTime::Agora();                                        // wasm 479
    // tipo de urna of the current turn: CEstadoGeral +36 (1st turn) / +40 (2nd turn), see u20
    const int tipoUrna = (eg.GetTurno() == '1') ? eg.m_tipoUrnaT1 : eg.m_tipoUrnaT2;   // (i + 32 + 4|8)
    CRelUtil::IncluiCabecalhoEleicoesMZS(m_relatorio, "Versões de Pacotes", local.GetMunicipio(),
                                         local.GetZonaID(), local.GetSecaoID(), local.GetNomeMunicipio(),
                                         tipoUrna == '2' ? 2 : 0, 1);                    // wasm 1543
    // binary: `if (tipo != '2') { title; goto B_b } AdicionaLinhasEmBranco(1); B_b: AdicionaLinhasEmBranco(1)`
    // -> title + 1 blank line, or 2 blank lines
    if (tipoUrna != '2')
        // title line produced by a std::function<CGeradorRelVersaoPacoteDados::ImprimeTitulo...>
        m_relatorio.AdicionaLinha(ImprimeTitulo(), 1, 0);
    else
        m_relatorio.AdicionaLinhasEmBranco(1);
    m_relatorio.AdicionaLinhasEmBranco(1);
    m_relatorio.AdicionaLinha(TextoIdentificacao(), 1, 0);                              // wasm 1942
    m_relatorio.AdicionaDataHora(agora, "Data", "Hora");                               // wasm 1542
    m_relatorio.AdicionaLinhasEmBranco(1);

    if (!CPE::Existe())                                                                // wasm 2788
        CPE::CreateInst(CarregaProcessoEleitoral());                                   // wasm 2273
    const auto& pe = CPE::GetInst();
    m_relatorio.AdicionaLinha(std::format("{:<32s} {:05}", "Código do processo eleitoral", pe.m_codigo /*+0*/), 1, 0);
    m_relatorio.AdicionaLinha(std::format("{:<32s} {:05}", "Código do pleito 1", pe.m_pleito1 /*+36*/), 1, 0);
    if (pe.m_temPleito2 /*+156*/)
        m_relatorio.AdicionaLinha(std::format("{:<32s} {:05}", "Código do pleito 2", pe.m_pleito2), 1, 0);
    m_relatorio.AdicionaLinhasEmBranco(1);
    ImprimeVersoes(m_relatorio, eg.m_versoes /*+60*/);                                  // wasm 1541
    m_relatorio.AdicionaLinhasEmBranco(1);
    // wasm 2785: AdicionaLinha(CRelUtil::GetSeparadorFase(fase), 1, 2) (curated: separator line
    // with the election phase); CEstadoGeral +48 = fase
    IncluiSeparadorFase(m_relatorio, eg.GetFase());                                   // name inferred
    m_relatorio.AdicionaLinhasEmBranco(1);
}

} // namespace comum

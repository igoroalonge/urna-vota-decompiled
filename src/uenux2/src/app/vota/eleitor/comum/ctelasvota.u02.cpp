// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp  --  FRAGMENT written by unit u02
//
// This file belongs to unit u07 (ctelasvota.cpp). The functions below were attributed to
// chkdfseed.cpp by the tools because their only caller is the giant wasm func 7787 (see
// cinformacaoeleitor.cpp), into which vota::CTelasVota::CreateInst() and the constructor
// vota::CTelasVota::CTelasVota() (ctelasvota.cpp:3435/3502) are inlined. They are reconstructed
// here so that u07 can merge them; names come from the strings the functions carry (every
// CriaTela* passes __func__ to its adicionaBase* helper, and a "tela..." form name).
//
// Common vocabulary:
//   CFormBuilder builder;                       12 bytes = std::vector<shared_ptr<IFormField>>
//   adicionaAnimacao(tipo, builder, conferencia)   wasm func 1593 (mis-named api::getResourceMovie,
//        which is inlined into it): the caller zero-initialises its CFormBuilder and passes it by
//        reference as the SECOND argument (not an sret return, which would be the first);
//        tipo 1 -> ":/resource/gifs/votoLegenda.gif", 3 -> "votoBranco.gif", 4 -> "votoNulo.gif",
//        other -> no animation; the CMovie is cached in a std::map @1833352 keyed by (path, conferencia).
//   CriaFormInterativo(builder, preShow, nome, true)  wasm func 554 (see cformbuilder.u02.cpp)
//   CriaFormInterativoVota(builder, nome)       wasm func 576 = CriaFormInterativo with a new
//                                               vota::CPreShowFormVota
//   CriaFormVota(builder, nome)                 wasm func 886 -> 6117 (non-interactive IForm with
//                                               CPreShowFormVota)
//   Fonts are static descriptors: FONTE_20 @474896, FONTE_30 @474888, FONTE_35 @475016,
//   FONTE_40 @474880, FONTE_RODAPE @475008, FONTE_TEXTO @474992 (first short = size in px).
//   A TPosition pair {x, y} written as one int32 is shown decoded, e.g. 26542081 -> {1, 405}.

#include "vota/eleitor/comum/ctelasvota.h"

#include <format>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace vota {

using TTeclasRotuladas = std::vector<std::pair<char, std::string>>;   // 16-byte elements

namespace {

// ---------------------------------------------------------------------------------------------
// wasm func 1102                                                       // name inferred
// Footer of the "vote" screens: separator line and the CONFIRMA / CORRIGE instructions.
void adicionaInstrucoesConfirmaCorrige(api::CFormBuilder& builder, const std::string& acaoConfirma,
                                       api::TPosition largura)
{
    builder.AddRect({0, 400}, {largura, 401});                                           // wasm 3680
    builder.AddText("Aperte a tecla:", {1, 405}, FONTE_20, 0, 2, 1);                     // wasm 202
    builder.AddText("CONFIRMA", {130, 430}, FONTE_20, 1, 2, 1);
    builder.AddText(" para " + acaoConfirma, {130, 430}, FONTE_20, 0, 2, 1);
    builder.AddText("CORRIGE", {130, 455}, FONTE_20, 1, 2, 1);
    builder.AddDataText(1097, {130, 455}, FONTE_20, 0);    // wasm 1191: text of data source 1097 (?)
}

// wasm func 1190                                                       // name inferred
// Footer of the "conferência" (review) screens: separator and a blinking "CONFIRA O SEU VOTO".
void adicionaRodapeConfiraSeuVoto(api::CFormBuilder& builder, api::TPosition largura)
{
    builder.AddRect({0, 400}, {largura, 401});
    builder.Add(std::make_shared<api::CTextFieldBlinking>(
        "CONFIRA O SEU VOTO", api::SPoint{static_cast<api::TPosition>(largura / 2), 420}, FONTE_30, 2));
}

// wasm func 2384                                                       // name inferred
// Number entry area of the empty vote screens (CriaTelaInputVazio*, inlined in CTelasVota()).
void adicionaCampoNumero(api::CFormBuilder& builder, api::TPosition x, api::TPosition y,
                         uebyte qtdDigitos, int qtdMascara)
{
    builder.AddImageField({176, 445}, std::make_shared<api::CDataImage<std::vector<uebyte>>>(1086)); // wasm 6689
    builder.AddDataText(1087, {10, 445}, FONTE_TEXTO, 0);                                           // wasm 1191
    auto campoNumero = builder.AddNumberInput({x, y}, qtdDigitos, 0, 1, 0, 1, 0, FONTE_40, 0);      // wasm 2383
    // Only when qtdMascara != 0 (binary: `if (eqz(e)) goto skip`): a
    // CMaskedTextField<CFramedText> over an empty CFixedText, qtdMascara characters, FONTE_40,
    // placed 5 px right of the number field (x = right edge returned by the field's vtable slot 8,
    // + 5; same y).
    if (qtdMascara != 0)
        builder.AddMaskedText(std::string{}, qtdMascara,
                              {static_cast<api::TPosition>(campoNumero->GetRect().direita + 5), y},
                              FONTE_40);                                                            // (?) names
}

// wasm func 4161                                                       // name inferred
// Bottom band with the polling place: filled bar {0,458}-{640,480} (colour 22) and one line of text.
// md::CLocal (u04): optional<CSecaoEleitoral> engaged flag at +120, optional
// <CIdentificacaoUrnaContingencia> engaged flag at +136.
void adicionaRodapeLocal(api::CFormBuilder& builder)
{
    auto& local = comum::CLocal::GetInst();                                  // wasm 401
    local.Carrega();                                                         // wasm 5740
    std::string texto;
    if (local.EhSecao()) {                                                   // wasm 5739: data byte +120
        texto = std::format("{}: {:05} - {}     {}: {:04}     {}: {:04}",
                            comum::md::CTradutorFrase::TraduzLabel("<MCSN>"), local.GetMunicipio(),
                            local.GetNomeMunicipio(),
                            comum::md::CTradutorFrase::TraduzLabel("<ZCSN>"), local.GetZonaID(),
                            comum::md::CTradutorFrase::TraduzLabel("<SCSN>"), local.GetSecaoID());
    } else if (local.EhContingencia()) {                                     // inline test of data byte +136
        // contingency urna (no seção): the sixth argument is the constant string_view
        // "CONTINGÊNCIA" (@335107, 12 bytes, packed as {ptr, len} in the format args)
        texto = std::format("{}: {:05} - {}     {}: {:04}     {}",
                            comum::md::CTradutorFrase::TraduzLabel("<MCSN>"), local.GetMunicipio(),
                            local.GetNomeMunicipio(),
                            comum::md::CTradutorFrase::TraduzLabel("<ZCSN>"), local.GetZonaID(),
                            std::string_view{"CONTINGÊNCIA"});
    }
    // neither a seção nor a contingency urna: the band is drawn with an empty text
    builder.AddFill({0, 458}, {640, 480}, 22);                               // wasm 2245
    builder.AddText(texto, {320, 460}, FONTE_RODAPE, 2, 2, 22);             // wasm 202
}

// wasm func 4160                                                       // name inferred
// Header block of the zerésima screens: election names (CConfiguracaoEleicao +8 / +32) at y=45/80,
// "<município>/<UF>" and "{}: {:04} {}: {:04}" (<ZCSA>/<SCSA> = zona / seções agregadas, count from
// "GetQtdAgregadas"), "{}{:04}". Built from CConfiguracaoEleicao, CAppInfo::GetGeral() (+20 int64)
// and CLocal. Also called by wasm func 3059. Body not reconstructed line by line (3.7 KB of
// std::format plumbing); see decompiled/app-api/_by_index/04000.dcmp.
void adicionaCabecalhoZeresima(api::CFormBuilder& builder);

} // namespace

// ---------------------------------------------------------------------------------------------
// Vote screens of a candidate office. largura 389 = screens with photo column, 639 = full width.

// wasm func 1591
CFormInterativoTelaVota CTelasVota::CriaTelaVotoNuloCandidato(const comum::md::CCargo& cargo,
                                                             const std::string& acaoConfirma,
                                                             const std::string& texto,
                                                             uebyte qtdDigitos)
{
    api::CFormBuilder builder;
    adicionaAnimacao(ETipoAnimacao::VOTO_NULO /*4*/, builder, false);                                      // wasm 1593
    api::TPosition xNumero;
    adicionaBaseTelaVotoNuloCandidato(builder, __func__, cargo, texto, qtdDigitos, xNumero);   // wasm 6608
    adicionaInstrucoesConfirmaCorrige(builder, acaoConfirma, 389);
    if (qtdDigitos == 0)
        builder.AddControlInput();                                                          // wasm 901
    else
        builder.AddNumberInput({xNumero, 115}, qtdDigitos, 0, 1, 1, 0, 1, FONTE_40, 0);     // wasm 2383
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaVotoNuloCandidato");                                     // wasm 554
}

// wasm func 1767
CFormInterativoTelaVota CTelasVota::CriaTelaConferenciaVotoNuloCandidato(const comum::md::CCargo& cargo,
                                                                        const std::string& texto)
{
    api::CFormBuilder builder;
    adicionaAnimacao(ETipoAnimacao::VOTO_NULO, builder, true);                                      // wasm 1593
    api::TPosition xNumero;
    adicionaBaseTelaVotoNuloCandidato(builder, __func__, cargo, texto, 0, xNumero);
    adicionaRodapeConfiraSeuVoto(builder, 389);
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaConferenciaVotoNuloCandidato");
}

// wasm func 3063
CFormInterativoTelaVota CTelasVota::CriaTelaVotoBrancoCandidato(const comum::md::CCargo& cargo)
{
    api::CFormBuilder builder;
    adicionaAnimacao(ETipoAnimacao::VOTO_BRANCO /*3*/, builder, false);                                      // wasm 1593
    adicionaBaseTelaVotoBrancoCandidato(builder, __func__, cargo);                            // wasm 6646
    adicionaInstrucoesConfirmaCorrige(builder, "CONFIRMAR este voto", 389);
    builder.AddControlInput();
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaVotoBrancoCandidato");
}

// wasm func 3067
CFormInterativoTelaVota CTelasVota::CriaTelaConferenciaVotoBrancoCandidato(const comum::md::CCargo& cargo)
{
    api::CFormBuilder builder;
    adicionaAnimacao(ETipoAnimacao::VOTO_BRANCO, builder, true);                                      // wasm 1593
    adicionaBaseTelaVotoBrancoCandidato(builder, __func__, cargo);
    adicionaRodapeConfiraSeuVoto(builder, 389);
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaConferenciaVotoBrancoCandidato");
}

// wasm func 6639
CFormInterativoTelaVota CTelasVota::CriaTelaCompletaCandidatoCom0(const comum::md::CCargo& cargo)
{
    api::CFormBuilder builder;
    adicionaBaseTelaCompletaCandidatoCom0(builder, __func__, cargo);                          // wasm 6678
    adicionaInstrucoesConfirmaCorrige(builder, "CONFIRMAR este voto", 639);
    builder.AddControlInput();
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaCompletaCandidatoCom0");
}

// wasm func 6681
CFormInterativoTelaVota CTelasVota::CriaTelaConferenciaCandidatoCom0(const comum::md::CCargo& cargo)
{
    api::CFormBuilder builder;
    adicionaBaseTelaCompletaCandidatoCom0(builder, __func__, cargo);
    adicionaRodapeConfiraSeuVoto(builder, 639);
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaConferenciaCandidatoCom0");
}

// wasm func 6626
CFormInterativoTelaVota CTelasVota::CriaTelaVotoNuloConsulta(const comum::md::CCargo& cargo,
                                                            const std::string& texto)
{
    api::CFormBuilder builder;
    adicionaBaseTelaVotoNuloConsulta(builder, __func__, cargo, texto);                         // wasm 6624
    adicionaInstrucoesConfirmaCorrige(builder, "CONFIRMAR este voto", 639);
    builder.AddControlInput();
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaVotoNuloConsulta");
}

// wasm func 6616
CFormInterativoTelaVota CTelasVota::CriaTelaConferenciaVotoNuloConsulta(const comum::md::CCargo& cargo,
                                                                       const std::string& texto)
{
    api::CFormBuilder builder;
    adicionaBaseTelaVotoNuloConsulta(builder, __func__, cargo, texto);
    adicionaRodapeConfiraSeuVoto(builder, 639);
    return CriaFormInterativo(builder, std::make_shared<CPreShowProgressBar>(),
                              "telaConferenciaVotoNuloConsulta");
}

// ---------------------------------------------------------------------------------------------
// Other screens created by CTelasVota::CTelasVota() or by vota states.

// wasm func 6601                                                       // name inferred
// "FIM" screen shown after the last office was voted.
std::shared_ptr<api::IForm<api::IScreen>> CTelasVota::CriaTelaFim(const std::string& subtitulo)
{
    api::CFormBuilder builder;
    builder.AddStatusHeader(5);                                                          // wasm 502
    builder.AddText("FIM", {320, 140}, api::SFonte{200, 0}, 2, 2, 1);
    builder.AddText(subtitulo, {117, 408}, api::SFonte{40, 0}, 1, 5, 1);
    adicionaRodapeLocal(builder);
    return CriaFormVota(builder, "telaFim");                                             // wasm 886
}

// wasm func 2380                                                       // name inferred
// Centred message; '&' separates lines. Also used by vota::CInspecionaUrna and wasm 6557.
std::shared_ptr<api::IForm<api::IScreen>> CTelasVota::CriaTelaNeutra(const std::string& texto,
                                                                     api::TPosition alturaFonte,
                                                                     bool comRodapeLocal)
{
    const auto linhas = comum::Split(texto, '&');                                        // wasm 1880
    api::CFormBuilder builder;
    builder.AddStatusHeader(5);
    if (!linhas.empty()) {
        const api::TPosition passo = alturaFonte + alturaFonte / 2;
        const api::TPosition y0 = static_cast<api::TPosition>(
            240 + alturaFonte / 2 - static_cast<int>(linhas.size()) * alturaFonte - alturaFonte / 4);
        for (std::size_t i = 0; i < linhas.size(); ++i)
            builder.AddText(linhas[i], {320, static_cast<api::TPosition>(y0 + i * passo)},
                            api::SFonte{alturaFonte, 0}, 2, 2, 1);
    }
    if (comRodapeLocal)
        adicionaRodapeLocal(builder);
    return CriaFormVota(builder, "telaNeutra");
}

// wasm func 6557                                                       // name inferred
std::shared_ptr<api::IForm<api::IScreen>> CTelasVota::CriaTelaContinuacaoVotacao()
{
    return CriaTelaNeutra("CONTINUAÇÃO DA VOTAÇÃO&IDENTIFIQUE O ELEITOR", 40, true);
}

// wasm func 2376                                                       // name inferred
// Message with optional CONFIRMA ('C') / CORRIGE ('D') key labels.
CFormInterativoTelaVota CTelasVota::CriaTelaTextoConfirmaCorrige(const std::string& texto,
                                                                const std::optional<std::string>& rotuloConfirma,
                                                                const std::optional<std::string>& rotuloCorrige)
{
    api::CFormBuilder builder;
    TTeclasRotuladas teclas;
    if (rotuloConfirma)
        teclas.emplace_back('C', *rotuloConfirma);
    if (rotuloCorrige)
        teclas.emplace_back('D', *rotuloCorrige);
    builder.AddStatusHeader(5);
    builder.AddText(texto, {320, 200}, FONTE_35, 2, 2, 1);
    builder.AddControlInput();
    builder.AddLabeledInputControl(teclas);                                              // wasm 653
    return CriaFormInterativoVota(builder, "telaTextoConfirmaCorrige");                  // wasm 576
}

// wasm func 6592                                                       // name inferred
// Question before printing the zerésima ("relatório zerésima" = zero-votes report).
CFormInterativoTelaVota CTelasVota::CriaTelaConfirmaImpressaoZeresima(const std::string& rotuloConfirma)
{
    api::CFormBuilder builder;
    // wasm 5156 -> 3942: copy of the label passed through the text filter at table slot 2920
    // (wasm 3509), then the message block helper wasm 3059 (mis-named CImageFieldUpdate ctor).
    adicionaBlocoMensagem(builder, FiltraTexto(rotuloConfirma), "", "", 2);
    builder.AddLabeledInputControl(TTeclasRotuladas{{'C', rotuloConfirma}, {'B', "Mais informações"}});
    return CriaFormInterativoVota(builder, "telaConfirmaImpressaoZeresima");
}

// wasm func 6595 (with the vector<pair<char,string>>::emplace_back slow path wasm 6594)
// Shown while the urna is waiting for the zerésima time.                // name inferred
CFormInterativoTelaVota CTelasVota::CriaTelaAntesHorarioZeresima()
{
    const api::CDateTime inicio = comum::CConfiguracaoEleicao::GetInst().GetDataHoraZeresima();   // +544, name inferred
    const std::string texto = inicio.Format("h") + "h" + inicio.Format("mm") + " do dia "
                            + inicio.GetDate().Format("DD/MM/YYYY") + ".";
    api::CFormBuilder builder;
    adicionaBlocoMensagem(builder, "ATENÇÃO", "Esta urna eletrônica só funcionará a partir de", texto, 0); // wasm 3059

    TTeclasRotuladas teclas;
    const auto maxVias = comum::CInformacaoEleicao(comum::CConfiguracaoEleicao::GetInst()).GetQtdMaxVias(); // wasm 603/2865, name inferred
    const auto viasImpressas = comum::CAppInfo::GetInst().GetVota(EUrnaTurno::ATUAL).GetViasEstadoUrna(); // byte +73
    if (maxVias > viasImpressas)
        teclas.emplace_back('C', "Emissão do estado da urna");
    teclas.emplace_back('B', "Mais informações");
    builder.AddLabeledInputControl(teclas);
    return CriaFormInterativoVota(builder, "telaAntesHorarioZeresima");
}

// wasm func 6599 (+ wasm 2285 = comum::CMenuBase::AdicionaItem, wasm 5913 = CMenuBase::Monta)
// "Mais informações" menu.                                             // name inferred
CFormInterativoTelaVota CTelasVota::CriaTelaMaisInformacoes()
{
    api::CFormBuilder builder;
    builder.AddStatusHeader(5);
    builder.AddText("Mais informações", {64, 100}, FONTE_30, 2, 2, 1);
    {
        CMenuMaisInformacoesVota menu(builder);          // vtable @1539820 : comum::CMenuBase
        const auto& estado = comum::CAppInfo::GetInst().GetVota(EUrnaTurno::ATUAL);
        const comum::CInformacaoEleicao info(comum::CConfiguracaoEleicao::GetInst());   // wasm 603
        menu.AdicionaItem(std::make_shared<CItemImprimeEstadoUrnaVota>(1,
            std::format("Estado da urna ({}/{})", estado.GetVias(0) /*+73*/, info.GetMaxVias(0)), api::SPoint{100, 155}));
        menu.AdicionaItem(std::make_shared<CItemImprimeListaEleitoresVota>(2,
            std::format("Lista de eleitores ({}/{})", estado.GetVias(1) /*+74*/, info.GetMaxVias(1)), api::SPoint{100, 180}));
        menu.AdicionaItem(std::make_shared<CItemVersoesPacotesVota>(3,
            std::format("Versões de pacotes ({}/{})", estado.GetVias(2) /*+75*/, info.GetMaxVias(2)), api::SPoint{100, 205}));
        menu.AdicionaItem(std::make_shared<CItemParametrosUrnaVota>(4,
            std::format("Parâmetros de urna ({}/{})", estado.GetVias(3) /*+76*/, info.GetMaxVias(3)), api::SPoint{100, 230}));
        menu.AdicionaItem(std::make_shared<CItemVisualizarCandidatosVota>(5, "Visualizar candidatos",
                                                                         api::SPoint{100, 255}));
        menu.Monta();                                                                    // vtable slot 2
    }
    const auto& config = comum::CConfiguracaoEleicao::GetInst();
    builder.AddText(std::format("Processo eleitoral: {:05}", config.GetProcessoEleitoral() /*+0*/), {25, 360}, FONTE_RODAPE, 0, 2, 1);
    builder.AddText(std::format("Pleito: {:05}", config.GetPleito() /*+28*/), {25, 380}, FONTE_RODAPE, 0, 2, 1);
    builder.AddText(std::string("Local com biometria: ") +
                        (comum::CLocal::GetInst().UrnaBiometrica() ? "Sim" : "Não"),   // wasm 820
                    {25, 400}, FONTE_RODAPE, 0, 2, 1);
    builder.AddLabeledInputControl(TTeclasRotuladas{{'C', "Selecionar"}, {'D', "Retornar"}});
    return CriaFormInterativoVota(builder, "telaMaisInformacoes");
}

// ---------------------------------------------------------------------------------------------
// wasm func 6548: vota::CTelasVota::~CTelasVota() (non-deleting). Releases the 30 shared_ptr form
// members (+16 .. +248, every second word is a control block) and the std::map of the per-office
// screen sets at +4 (tree destroy wasm 4137 -> 1314: node value holds a shared_ptr at +24).
CTelasVota::~CTelasVota() = default;

// wasm func 12882: atexit handler of the static std::unique_ptr<CTelasVota> @1833396
// (`p = exchange(ms_instancia, nullptr); if (p) { p->~CTelasVota(); free(p); }`).

// wasm func 4137 / 1314: std::__tree<...>::destroy for std::map<comum::TCargoID, CTelasCargo> and
// its inner map<K, shared_ptr<IForm>> (library instantiations, see the u02 mapping table).

} // namespace vota

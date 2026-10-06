// Reconstructed from vota_web_wasm.wasm by unit u04 - FRAGMENTS of other original files.
//
// The unit builder put these functions in u04 (uenux2/src/app/comum/dados) because they call or inline
// CCargos / CConfiguracaoEleicao / CEleitores code. Their real homes are other files, most of them
// owned by other units; each section names the original file (or "(path inferred)") so the code can
// be merged there. The BU QR-code generator (5603-5617, 6028) is in
// ../relatorios/cgeradorbuqrcode.u04-fragment.cpp.
//
// Error types: see ccargos.cpp. CErroRelatorios = CBaseError<comum::EUeComumRelatoriosError>
// (typeinfo @1543944, thunk comum_f580); CErroVota = CBaseError<vota::EUeVotaError> (typeinfo @1532388).

// =================================================================================================
// uenux2/src/app/comum/dados/ccandidaturas.cpp  (unit u03)
// =================================================================================================
namespace comum {

// wasm func 2272 - name inferred. Numbers of the APT candidacies of a cargo, optionally of one party
// (partido == 0 -> every party). CCandidaturas' map is keyed by cargo*1000000+numero; value layout:
// cargo +0 (node +20), partido +2, numero +4, situação/inapto +52 (must be 0).
// ecourna_f2840 (not in u04) is the same loop without the party filter.
std::vector<TCandidatoID> CCandidaturas::GetNumerosCandidatosAptos(TCargoID cargo, TPartidoID partido) const
{
    std::vector<TCandidatoID> numeros;
    for (const auto& [chave, c] : m_candidaturas) {
        if (c.GetCargo() != cargo || c.GetSituacao() != 0)
            continue;
        if (partido != 0 && c.GetPartido() != partido)
            continue;
        numeros.push_back(c.GetNumero());
    }
    return numeros;
}

// wasm func 12742 = api::CDataText<CCandidaturasDSNumero>::GetText() (api::IText vtable slot 2,
// name inferred) with CCandidaturasDSNumero::operator() inlined: the candidate number of the
// candidacy being confirmed, zero-padded to the cargo's number of digits. Observed executing
// (confirmation screen of every nominal vote).
std::string CCandidaturasDSNumero::operator()() const
{
    const md::CCandidatura& c = GetCandidaturaAtual("CCandidaturasDSNumero");     // 2838
    const unsigned digitos = CConfiguracaoEleicao::GetInst().GetCargo(c.GetCargo()).GetNumeroDigitos(); // +12
    return std::format("{:0{}}", c.GetNumero(), digitos);
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/comum/dados/ccargods.cpp  (unit u03)  (path inferred for this function)
// =================================================================================================
namespace comum {

// wasm func 12811 (table slot 1088) - name inferred. Title of the current cargo on the voting screen,
// e.g. "Senador" or, for a cargo with several choices, "Senador - 1ª vaga" (GetOrdinalEscolha).
// Observed executing. s_escolhaAtual is a global byte @1536340 owned by the vote state machine
// (CEleitorVotando::IniciaCiclo, 7377, sets it to 1).
std::string NomeCargoComEscolha()
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    const std::string nome = CCargoDSNomeSexoCandidato{}();                      // 2268
    if (CCargos::GetInst().GetCurrent().GetQtdeEscolhas() == 1)                  // +13
        return nome;
    return nome + " - " + cargo.GetOrdinalEscolha(s_escolhaAtual);             // 2258
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/comum/dados/md/eleitor/celeitordinamico.h/.cpp  (unit u05)
// =================================================================================================
namespace comum::md {

// wasm func 2800 - constructor used (in place, by vector::emplace_back) by CEleitores::DynamicCreate.
CEleitorDinamico::CEleitorDinamico(const CEleitorIdentidade& titulo, const CEleitorIdentidade& identidadeHabilitacao,
                                   EEstadoComparecimento estado)
    : m_titulo(titulo)                           // +0
    , m_identidadeHabilitacao(identidadeHabilitacao)  // +16
    , m_estado(estado)                           // +32
    , m_tipoHabilitacao(ETipoHabilitacao{0})     // +36
    , m_dedo(0)                                  // +40
    , m_score(0)                                 // +44
    , m_tentativas(0)                            // +46
    , m_tipoAtivacaoAudio(2)                     // +48   (2 = "não habilitado"?)
    , m_erroDecifrarBiometria(0)                 // +52
    , m_tituloMesario(std::nullopt)              // +56 (flag +72)
    , m_apresentacaoFoto{0, 0}                   // +76 / +80
{
}

}  // namespace comum::md

// =================================================================================================
// Wrapper of CEleitores::CompleteLoad (wasm func 6734). `this` is the 12-byte lazily created singleton
// returned by wasm 509, which units u02/u08 name vota::CInformacaoEleitor (cinformacaoeleitor.cpp);
// u02 names this method CarregarDadosDinamicos() and 6737 GerarDadosDinamicos() (names inferred).
// Callers: the start-up function 7787 (only when EstadoVota >= '3' AGUARDAHORAZERESIMA) and
// vota::CGeraDadosDinamicos::StartState (11865, right after 6737). Byte +1 = "dynamic data loaded".
// =================================================================================================
namespace vota {

void CInformacaoEleitor::CarregarDadosDinamicos()                        // name inferred (u02)
{
    if (m_dadosDinamicosCarregados)                                      // +1
        return;

    // 1. RDV (Registro Digital do Voto) from rdv.dat (encrypted; CEncryptedFile)
    comum::CRdvVota& rdv = comum::CRdvVota::GetInst();
    {
        // shared_f1551 = GetPathTrab(interna, turno) / "rdv.dat"; unknown_f1692 = CEncryptedFile ctor
        // taking the key material at rdv +8; the path goes to Load (3653), not to the constructor.
        api::CEncryptedFile arquivo(rdv.GetChaveCifragem());                     // rdv +8   // ?
        arquivo.Load(comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) / "rdv.dat");
        std::vector<uebyte> conteudo;
        arquivo.MemRead(conteudo);
        rdv.Desconverte(conteudo);                                                // vtable slot 12
    }

    // 2. Voter list with dynamic data - SKIPPED in "treinamento do eleitor" mode
    //    (CEstadoGeralVota +72; TRUE in the web simulator's vota.bin)
    const auto& estadoVota = comum::GetEstado<comum::md::estadoaplicacao::CEstadoGeralVota>(comum::ESTADO_VOTA); // @261, id 51
    if (!estadoVota.GetTreinamentoEleitor())
        comum::CEleitores::GetInst().CompleteLoad(comum::CPath::Estatico(),                  // wasm_entry_f762
                                                  comum::CPath::GetPathTrab(comum::EFlashOrigem::INTERNA) /
                                                      "uenux.db");                           // shared_f2826

    // 3. Justificativas (table registro_justificativa) -> CJustificador (comum_f1391)
    auto& justificador = comum::CJustificador::GetInst();
    justificador.Clear();                                                // tree destroy (2809)
    for (const auto& j : justificador.GetServico().SelectAll())          // 5723: CJustificadorServico at +28
        justificador.Add({ecourna::app::dados::CNumeroInscricaoEleitoral(j.GetIdentificador()), j});

    // 4. Comparecimento de mesários (table comparecimento_mesario) -> CDAORepositorio
    auto& mesarios = api::persistencia::CDAORepositorio<comum::md::CComparecimentoMesario>::Entregar();   // 815
    mesarios.Clear();                                                    // tree destroy (2728)
    for (const auto& m : mesarios.GetServico().SelectAll()) {            // 5723: CComparecimentoMesarioServico at +40
        mesarios.Add(m);                                                 // 5377
        if (m.TemIdentificacao())                                        // +20
            mesarios.IndexaIdentificacao(m.GetIdentificacao(), m);       // api_f5379      // ?
    }

    // 5. Cross checks between cargos, candidaturas, partidos, respostas... (comum_f2543 ->
    //    CIntegridadeReferencial::Lanca on failure)
    comum::VerificaIntegridadeReferencial();                             // name inferred
    m_dadosDinamicosCarregados = true;
}

}  // namespace vota

// =================================================================================================
// uenux2/src/app/comum/relatorios/cpartecandidatos.cpp  (unit u25)  - api::IReportPart parts of the
// BU / zerésima. api::IReportPart vtable: slot 0/1 destructors, slot 2 Imprime() const.
// =================================================================================================
namespace comum {

// wasm func 11203 - CParteRdv::Imprime (slot 2). Header, then the part matching the current cargo's
// kind, then trailer. Parts are shared_ptr<IReportPart> at +4, +12, +20, +28, +36.
void CParteRdv::Imprime() const
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    m_cabecalho->Imprime();                                                        // +4
    if (cargo.GetTipo() == md::ETipoCargo::MAJORITARIO && cargo.TemDetalheCandidato())
        m_majoritario->Imprime();                                                  // +12
    else if (cargo.GetTipo() == md::ETipoCargo::PROPORCIONAL && cargo.TemDetalheCandidato())
        m_proporcional->Imprime();                                                 // +20
    else if (cargo.TemDetalheConsulta())
        m_consulta->Imprime();                                                     // +28
    m_rodape->Imprime();                                                           // +36
}

// wasm func 11209 - CParteCargos::Imprime (slot 2). Walks every cargo; prints the election header
// part (+4, optional) whenever the election changes. The "previous election" starts as the election
// of the LAST cargo, so with a single election the header is never printed (with two elections it is
// printed for both).
void CParteCargos::Imprime() const
{
    CCargos& cargos = CCargos::GetInst();
    TEleicaoID anterior = cargos.Ultimo().eleicao;                                 // m_cargos.back()
    cargos.First();
    while (!cargos.IsEnd()) {
        if (cargos.GetCurrentEleicaoID() != anterior && m_cabecalhoEleicao)
            m_cabecalhoEleicao->Imprime();                                         // +4
        m_parteCargo->Imprime();                                                   // +12
        anterior = cargos.GetCurrentEleicaoID();
        cargos.Next();
    }
}

// wasm func 11213 - CParteCandidatosProporcionais::Imprime (slot 2). For every party that passes the
// filter std::function (+80, slot 6 = operator()), prints the party header, then either the "no
// candidates" part (+52) or one line per apt candidate (+28), then the party trailer (+36); finally
// the proportional trailer (+44). CCandidaturas::Localiza (ccandidaturas.cpp:56, code 7800
// "Candidato {} não encontrado para cargo {} ") is inlined to position the candidacy cursor.
void CParteCandidatosProporcionais::Imprime() const
{
    const TCargoID cargo = CCargos::GetInst().GetCurrentCargoID();
    CPartidos& partidos = CPartidos::GetInst();
    CCandidaturas& candidaturas = CCandidaturas::GetInst();
    m_cabecalho->Imprime();                                                        // +4
    for (partidos.First(); !partidos.IsEnd(); partidos.Next()) {
        if (!m_filtroPartido())                                                    // +80 (throws bad_function_call if empty)
            continue;
        m_cabecalhoPartido->Imprime();                                             // +12
        const auto numeros = candidaturas.GetNumerosCandidatosAptos(cargo, partidos.GetCurrent()->GetNumero());
        if (numeros.empty()) {
            m_semCandidatos->Imprime();                                            // +52
        } else {
            m_colunas->Imprime();                                                  // +20
            for (const TCandidatoID numero : numeros) {
                candidaturas.Localiza(cargo, numero);                              // sets current, throws 7800
                m_candidato->Imprime();                                            // +28
            }
        }
        m_rodapePartido->Imprime();                                                // +36
    }
    m_rodape->Imprime();                                                           // +44
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/comum/relatorios/cdatasourcesrelatorio.h  (unit u25)
// template<class RDV, class DSAptos> CDataSourcesRelatorio  - instantiated with <CRdvVota, CEleitores>
// =================================================================================================
namespace comum {

// wasm func 11968 - GetLinhasEleitoresAptos()::lambda (cdatasourcesrelatorio.h:212), stored in a
// std::function<SQtdeAptos()> (vtable slot 6 = operator()).
//   [] {
//       const auto aptos = DSAptos::GetInst().GetQtdAptos();                  // CEleitores 2824
//       const auto abr = CCargos::GetInst().GetCurrent().GetAbrangencia();    // CCargo +8
//       if (!aptos.contains(abr))
//           throw CErroRelatorios(EUeComumRelatoriosError{9089}, std::format("Abrangência inválida: {}", abr));
//       return aptos.at(abr);
//   }

// wasm func 11972 - name inferred (DetalheCandidato; sibling of DetalheCandidatoZE 11973): one
// candidate line of the BU: name (cut at 36, padded to 23; names of 24+ chars are filled with '-'
// and continued on a second line "  ---------------------->"), then "  {}  {}{:0{}}  {:04}" = name, padding (5 - digits spaces),
// number zero-padded to the cargo's digits, votes (CRdvVota::Candidato, slot 3).
template <class RDV, class DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::DetalheCandidato()
{
    const md::CCargo& cargo = CCargos::GetInst().GetCurrent();
    const md::CCandidatura& c = *CCandidaturas::GetInst().GetCurrent();          // 1200
    std::string nome = c.GetNomeUrna();                                           // +20
    if (nome.size() >= 37) nome.resize(36);
    if (nome.size() <= 22) nome.resize(23, ' ');                                  // unknown_f3511
    if (nome.size() >= 24) {
        // long name: fill the line with '-' and continue on a second line with an arrow
        nome.append(36 - nome.size(), '-');                                       // shared_f1054
        nome += "\n  ";                                                           // 3 chars @445547
        nome.append(22, '-');
        nome += ">";                                                              // 1 char @337169
    }
    const TQtdVoto votos = RDV::GetInst().Candidato(cargo.GetCodigo(), c.GetNumero(), cargo.GetNumeroDigitos());
    const std::string pad(5 - cargo.GetNumeroDigitos(), ' ');
    return std::format("  {}  {}{:0{}}  {:04}", nome, pad, c.GetNumero(), cargo.GetNumeroDigitos(), votos);
}

// wasm func 11975 - name inferred (TrailerComparecimento). Used by CGeraBU and CGeraZeresimaBase
// (table slot 1632): separator line (comum_f3871) + "Comparecimento                    {:04}\n"
// with CRdvVota::Cargo(current cargo) (total votes in the cargo).
template <class RDV, class DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::TrailerComparecimento()
{
    const TQtdVoto total = RDV::GetInst().Cargo(CCargos::GetInst().GetCurrent().GetCodigo());   // slot 10
    std::string s = Separador();                                                  // comum_f3871
    s += std::format("Comparecimento                    {:04}\n", total);
    return s;
}

// wasm func 11259 - name inferred (table slot 2923). Column header of the candidate list, printed
// only when the current cargo received nominal votes.
template <class RDV, class DSAptos>
std::string CDataSourcesRelatorio<RDV, DSAptos>::HeaderDetalheSeHouverVotos()
{
    const TCargoID cargo = CCargos::GetInst().GetCurrent().GetCodigo();
    if (RDV::GetInst().Nominais(cargo) == 0)                                      // slot 6
        return {};
    return CCargoDSLabelRelatorio::HeaderDetalhe() + "\n";                        // 5802
}

}  // namespace comum

// =================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cgeradorresumozeresima.cpp  (unit u09)
// =================================================================================================
namespace vota {
// wasm func 11556 (table slot 3049) - captureless lambda converted to a function pointer, used by
// vota_f5580 (title block "====" / <election name> / "====" of the zerésima summary and of the RDV
// extract). CCargos::GetInst (ccargos.cpp:24) is inlined, hence the misleading db name.
//   +[]() -> std::string { return comum::CCargos::GetInst().GetCurrentEleicao().GetNome(); }   // CEleicaoPE +12
}  // namespace vota

// =================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresimabase.cpp  (path inferred)
// =================================================================================================
namespace vota {
// wasm func 11976 (table slot 1631) - name inferred. Predicate given to the zerésima generator
// (CGeraZeresimaBase::StartState, 11946): print the current party for the current cargo if it has apt
// candidates, or else if its federation (api_f2832/unknown_f5799) has candidates in the cargo
// (unknown_f5809).
bool PartidoTemCandidatos()
{
    const comum::TCargoID cargo = comum::CCargos::GetInst().GetCurrent().GetCodigo();
    const comum::TPartidoID partido = comum::CPartidos::GetInst().GetCurrent()->GetNumero();
    auto& candidaturas = comum::CCandidaturas::GetInst();
    if (!candidaturas.GetNumerosCandidatosAptos(cargo, partido).empty())
        return true;
    const auto* federacao = comum::CFederacoes::GetInst().GetFederacaoDoPartido(partido);   // ?
    return federacao != nullptr && candidaturas.FederacaoTemCandidatos(cargo, *federacao); // ?
}
}  // namespace vota

// =================================================================================================
// uenux2/src/app/vota/eleitor/votamajoritario/cconfirmamajoritario.cpp  (path inferred)
// CVotacaoStateAudio vtable slot 15 = template of the accessibility (audio) message of the screen;
// slot 14 (6999) substitutes {cargo-atual}, {voto}, {candidato}... Name inferred: GetMensagemAudio.
// CCargo +136 == 1 -> the cargo is a referendum question ("consulta").
// =================================================================================================
namespace vota {

// wasm func 11687
std::string CMajoritarioValido::GetMensagemAudio() const
{
    if (comum::CCargos::GetInst().GetCurrent().TemDetalheConsulta())
        return "Você está respondendo à pergunta: {cargo-atual}. Você escolheu a opção {voto}: "
               "{escolha-referendo}. Aperte confirma ou corrige.";
    return "Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato-com-suplentes}. "
           "Aperte confirma ou corrige.";
}

// wasm func 11690
std::string CMajoritarioRepetido::GetMensagemAudio() const
{
    if (comum::CCargos::GetInst().GetCurrent().TemDetalheConsulta())
        return "Você já votou na opção \"{escolha-referendo}\". Se apertar confirma, este voto será nulo. "
               "Aperte confirma ou corrige.";
    return std::string(/* 201 chars @371777 */
        "Você está votando para {cargo-atual} n{o/a} candidat{o/a} {voto}: {candidato}. Candidat{o/a} já foi "
        "escolhid{o/a} em voto anterior. Se apert...");   // (full text in strings.tsv @371777)
}

// wasm func 11693
std::string CMajoritarioNulo::GetMensagemAudio() const
{
    if (comum::CCargos::GetInst().GetCurrent().TemDetalheConsulta())
        return "Número errado. Se apertar confirma, este voto será nulo. Aperte confir...";   // 84 chars @372037
    return "Você está votando para {cargo-atual} no candidato {voto}. Número errad...";      // 142 chars @371979
}

// wasm func 11696
std::string CMajoritarioBranco::GetMensagemAudio() const
{
    if (comum::CCargos::GetInst().GetCurrent().TemDetalheConsulta())
        return "Você está votando em branco. Aperte confirma ou corrige.";
    return "Você está votando em branco para {cargo-atual}. Aperte confirma ou corrige.";
}

}  // namespace vota

// =================================================================================================
// uenux2/src/app/vota/eleitor/votaproporcional/cpedenominal.cpp  (path inferred)
// =================================================================================================
namespace vota {

// wasm func 11724 - vtable slot 16 = GetProximoEstado(const std::string&) const (same slot is
// srcloc-named in the sibling CPedeNulo, 11744). Observed executing. Decides the confirmation screen
// after the voter typed `numero` for a proportional cargo. The states are function-local statics
// (lazy singletons: CConfereVotoEmCargo<Tela, id>).
comum::CAppState* CPedeNominal::GetProximoEstado(const std::string& numero) const
{
    s_numeroDigitado = numero;                                          // static std::string @1833288
    const comum::md::CCargo& cargo = comum::CCargos::GetInst().GetCurrent();
    if (cargo.GetNumeroDigitos() != numero.size())                      // fewer digits: party only
        return &CConfereVotoEmCargo<CConfirmaVotoLegenda, ETelaVotacao(10)>::Instancia();
    const comum::md::CCandidatura* c = comum::CCandidaturas::GetInst().Localiza(
        cargo.GetCodigo(), ecourna::api::util::CStringUtils::ToDWord(numero));        // shared_f1273
    if (c == nullptr)
        return &CConfereVotoEmCargo<CCandidatoInexistente, ETelaVotacao(12)>::Instancia();
    if (c->GetSituacao() != 0)                                          // +52: candidate not apt
        return &CConfereVotoEmCargo<CCandidatoInapto, ETelaVotacao(14)>::Instancia();
    return &CConfereVotoEmCargo<CConfirmaVotoNominal, ETelaVotacao(2)>::Instancia();
}

}  // namespace vota

// =================================================================================================
// uenux2/src/app/vota/eleitor/comum/cpreshowprogressbar.cpp  (path inferred)
// =================================================================================================
// wasm func 13550 - vota::CPreShowProgressBar::PreShow (api::IPreShow<IScreen> slot 2, name
// inferred). Observed executing. Before each voting screen is shown, builds an api::CStepsProgressBar
// (cstepsprogressbar.cpp:428 "Quantidade de segmentos deve ser maior que zero") with one segment per
// cargo (and per choice of multi-choice cargos, CCargo::GetOrdinalEscolha; referendum questions use
// CCargo::GetDetalheConsulta) of the CURRENT election, highlights the current one
// (GetCurrentEleicaoID / GetCurrentCargoID) and draws it (CStepsProgressBar slot 2), then destroys it.

// =================================================================================================
// uenux2/src/app/vota/operador/confirmaidentidade/  (ccontrolareconhecimento.cpp owned by unit u10;
// cdadoeleitornaoconfere.cpp path inferred)
// =================================================================================================
namespace vota {

// wasm func 5404 - name inferred. `s_qtdVerificacoes` is a static counter (data @1590928, initial 1)
// of the "confirm another voter datum" attempts; the limit comes from CConfiguracaoEleicao (+140).
bool CControlaReconhecimento::LimiteVerificacoesAtingido()
{
    return comum::CConfiguracaoEleicao::GetInst().GetLimiteVerificacoesDadoEleitor() == s_qtdVerificacoes;
}

// wasm func 10448 - CDadoEleitorNaoConfere::StartState (comum::CAppState slot 2). Shows one of two
// forms: +20 when the limit was reached, +12 otherwise. +4 is the "next state" field of CAppState
// (the same field ProcessInput writes): StartState sets it to `this` (stay in this state).
void CDadoEleitorNaoConfere::StartState()
{
    m_proximo = this;                                                    // +4
    (CControlaReconhecimento::LimiteVerificacoesAtingido() ? m_formLimite : m_form)->Show();   // slot 2
}

// wasm func 10447 - CDadoEleitorNaoConfere::ProcessInput (comum::CAppState slot 7). Reads the key
// (CInteractiveForm<IScreenMT, IInputMT>::Read, cinteractiveform.h:57); on key code 9 (CONFIRMA):
//   limit reached -> log (CLogVota) and go to the next state (api::CTextSource 652);
//   otherwise     -> CControlaReconhecimento::AvancaVerificacao() (ccontrolareconhecimento.cpp:176):
//                    if (limite == s_qtdVerificacoes) throw CErroVota(9398,
//                        "Limite de verificações de dado do eleitor atingido."); ++s_qtdVerificacoes;
//                    then api_f2735 (next state).
void CDadoEleitorNaoConfere::ProcessInput()
{
    auto& input = api::CPolySingletonList::instance<api::IInputMT>();
    auto* campo = m_form->Campos().at(m_form->CampoAtual());            // form +12: vector +72, index +84
    if (campo->Read(input) != api::EInputResult{9})                     // field vtable slot 8
        return;
    if (CControlaReconhecimento::LimiteVerificacoesAtingido()) {
        CLogVota::GetInst().Registra(/*...*/);                           // vota_f2097
        m_proximo = ProximoEstadoLimite();                               // ?
    } else {
        CControlaReconhecimento::AvancaVerificacao();
        m_proximo = ProximoEstadoVerificacao();                          // api_f2735 ?
    }
}

}  // namespace vota

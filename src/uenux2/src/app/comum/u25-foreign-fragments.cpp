// FRAGMENTS reconstructed by unit u25 (uenux2/src/app/comum/relatorios) from vota_web_wasm.wasm.
// These functions were assigned to u25 by the unit builder (their main callers are the report code) but
// belong to other source files; each block names its original file ("(path inferred)" when so).
// Library-like special members (copy/move/destructors, std::sort instantiations) are summarised.

// =====================================================================================================
// uenux2/src/app/comum/appinfo/cappinfo.cpp  (class reconstructed by unit u20 in cappinfo.h)
// =====================================================================================================
namespace comum {

// wasm func 185 (tools: comum_f185, 118+ callers, observed executing): CAppInfo::GetInst().
//   if (!s_inst) s_inst.reset(new CAppInfo);   return *s_inst;       (unique_ptr @1838664, mutex @1838640)
// See cappinfo.h (u20).

// wasm func 5813 (tools: comum_f5813): CAppInfo::CAppInfo() — every std::optional member disengaged
// (flags +0, +180, +184, +228, +232, +276, +280, +380, +384, +484, +488, +504, +508, +524) and the int at
// +528 set to -1 (cached turno index ?).
CAppInfo::CAppInfo() = default;

// wasm func 3792 (tools: comum_f3792): CAppInfo::~CAppInfo() — resets the optionals: the two
// optional<CEstadoGeralGap> (each holding a vector of 96-byte CDadoCorrespondencia), the two
// optional<CEstadoGeralVota> (func 857) and the optional<CEstadoGeral>.
CAppInfo::~CAppInfo() = default;

// wasm func 11574 (table slot 2336): atexit destructor of the static unique_ptr<CAppInfo> @1838664.

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/dados/clocal.cpp  (unit u04 / u02)
// =====================================================================================================
namespace comum {
// wasm func 2261 (tools: comum_f2261): CLocal::~CLocal() = std::unique_ptr<md::CLocal>::reset()
//   (md::CLocal destructor = func 5744). Callers: CLocal::GetInst (401, when replacing the instance),
//   IncluiCabecalhoEleicoesMZS (1543) and FormataSecoesAgregadas (5738) through their inlined GetInst.
CLocal::~CLocal() = default;
// wasm func 11508 (table slot 2415): atexit destructor of the static unique_ptr<CLocal> @1838900.
} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/informacao/cinformacaoeleicao.h  (unit u23)
// =====================================================================================================
// wasm func 603 (tools: vota_f603, 22 callers): CInformacaoEleicao's constructor, "a[0] = b; return a". The +88
// (CParametrosUrna inside CConfiguracaoEleicao) is added by EVERY CALLER (`vota_f603(tmp, GetInst() + 88)`), not
// by 603 (checked at 1543, 1919, 2520, 4442, 5583, 6595, 7160, 11193, 11666), so the constructor's parameter
// is the CParametrosUrna sub-object (cfg.GetParametros(), or a base class located at +88). The project writes
// CInformacaoEleicao(CConfiguracaoEleicao::GetInst()) (unit u23 convention); read it as "the -pu.dat part".

// =====================================================================================================
// uenux2/src/app/comum/dados/cpe.cpp  (path inferred)
// =====================================================================================================
namespace comum {
// wasm func 2788 (tools: comum_f2788): static bool CPE::Existe() { lock; return s_inst != nullptr; }
// (instance @1839112, mutex residue @1839088). Name from unit u02.
bool CPE::Existe()
{
    std::lock_guard lock(s_mutex);
    return s_inst != nullptr;
}
} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/dados/ccandidaturas.cpp  (unit u03)
// =====================================================================================================
namespace comum {
// wasm func 2271 (tools: comum_f2271). True when the cargo has at least one APT candidacy
// (CCandidatura +0 == cargo and the titular's situação, CCandidatura +52, == 0). Callers: BU printers
// (11254, 11255), CParteCandidatos::Imprime (11215), CEleitorVotando::suspenderEleitor (4442), 3819.
// Name as in unit u23 (u06 calls it ExisteCandidato).                                    name inferred
bool CCandidaturas::PossuiCandidatos(TCargoID cargo) const
{
    for (const auto& [chave, candidatura] : m_candidaturas)
        if (candidatura.GetTitular().GetSituacao() == 0 && candidatura.GetCargo() == cargo)
            return true;
    return false;
}
} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp / .h  (units u04 / u05)
// =====================================================================================================
namespace comum {

// wasm func 271 (tools: comum_f271; file attributed to crelutil.cpp by callers). Inline accessor:
const md::CSituacoesEleicoes& CConfiguracaoEleicao::GetSituacoesEleicoes(TEleicaoID eleicao) const
{
    return m_pleito.GetSituacoesEleicoes(eleicao);            // CPleito at +28, func 5648 (cpleito.cpp:175)
}

// wasm func 3775 (tools: comum::CConfiguracaoEleicao::GetCargos). Cargos of one eleição of the pleito; with
// `ordenados`, sorted by CCargo::ordemImpressao (byte +16) through the function-pointer comparator of table slot 2363
// (wasm 11547: `return a.ordemImpressao < b.ordemImpressao`). An unknown eleição gives an empty vector.
// Callers: CGravadorBU::vf7 (11629, true), vota::CInformacaoEleitor::Inicializar (7787, formerly shown as
// CHKDFSeed::GetSeed: CConfiguracaoEleicao/CRdvVota creation, false), api_f5735.                  name from u23
std::vector<md::CCargo> CConfiguracaoEleicao::GetCargos(TEleicaoID eleicao, bool ordenados) const
{
    const auto& eleicoes = m_pleito.GetEleicoes();                                    // +52 .. +56
    if (std::ranges::none_of(eleicoes, [&](const md::CEleicaoPE& e) { return e.GetId() == eleicao; }))
        return {};
    std::vector<md::CCargo> cargos = m_pleito.GetEleicao(eleicao).GetCargos();        // funcs 5651, 2257
    if (!ordenados)
        return cargos;
    std::vector<md::CCargo> resultado = cargos;                                       // func 2289 (copy)
    std::sort(resultado.begin(), resultado.end(), &ComparaOrdemImpressaoCargo);       // func 5789
    return resultado;
}
// std::sort<md::CCargo*, bool(*)(const CCargo&, const CCargo&)> instantiation (140-byte elements):
//   wasm 5789  std::__introsort          wasm 5785  std::__insertion_sort_incomplete
//   wasm 2829  std::__sort4              wasm 320   std::swap(md::CCargo&, md::CCargo&) (via moves)
//   wasm 761   md::CDetalheCandidato::operator=(CDetalheCandidato&&)      (implicit)
//   wasm 783   std::optional<md::CDetalheConsulta>::operator=(optional&&)   (__assign_from)
//   wasm 5784  std::vector<md::CRespostaConsulta>::operator=(vector&&)     (28-byte elements)

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/dados/md/processoeleitoral/*.h — implicitly defined special members that the
// compiler emitted out of line (no source code of their own):
//   wasm 365   md::CDetalheCandidato::CDetalheCandidato(const CDetalheCandidato&)  (64 bytes: bool temFoto,
//              4 strings of CNomesCargo, vector<CSuplencia> of 52-byte items via func 2341)   observed executing
//   wasm 242   md::CDetalheCandidato::~CDetalheCandidato()
//   wasm 374   md::CDetalheConsulta::CDetalheConsulta(const CDetalheConsulta&)    (48 bytes: 2 strings,
//              vector<CRespostaConsulta> (28-byte items, func 3018), string)
//   wasm 267   md::CDetalheConsulta::~CDetalheConsulta()
//   wasm 730   md::CEleicaoPE::~CEleicaoPE()         (52 bytes: see the layout below)
//   wasm 817   md::CEleicaoPE::operator=(CEleicaoPE&&)        wasm 818  std::swap(CEleicaoPE&, CEleicaoPE&)
//   wasm 1953  std::uninitialized_copy(const CEleicaoPE*, const CEleicaoPE*, CEleicaoPE*)  (copy of
//              vector<CEleicaoPE>; used by CPleito's copy constructor)
//   wasm 5595  std::copy loop of CEleicaoPE::operator=(const&) (vector<CEleicaoPE>::assign), which calls
//              CCargo::operator= (unknown_f5594) and vector<int>::assign (wasm 3693 -> merged body 6027)
//   wasm 2289  std::vector<md::CCargo>::vector(const vector&)      (140-byte items)  observed executing
//   wasm 5777  std::vector<md::CEleicaoPE>::push_back(const CEleicaoPE&) (reallocating path)  observed
//   wasm 1281  md::CPleito::CPleito(const CPleito&)   (+0 id, +4 nome, +16 CDate data, +24 vector<CEleicaoPE>,
//              +36 std::map<int, std::string> (func 3870 inserts), +48 vector<CSituacoesEleicoes>)
//   wasm 5597  md::CPleito::operator=(const CPleito&)
// md::CEleicaoPE layout (read in wasm 11371 below): +0 TEleicaoID id, +4 TPleitoID idPleito,
//   +8 id da eleição no pleito 2 (0 when absent), +12 std::string nome, +24 abrangência,
//   +28 std::vector<CCargo> cargos, +40 std::vector<int> municipios (EntidadeEleicao.municipios).
//   NOTE for unit u02: the vector at +40 is filled from `municipios`, not from a list of turnos.

// =====================================================================================================
// uenux2/src/app/comum/dados/asn/processoeleitoral/cconversoreleicaope.cpp  (path inferred; converters of
// this directory are unit u03). RTTI comum::asn::CConversorEleicaoPE, vtable slot 3.
// Observed executing (every eleição file "<fase><eleição:05><uf>-ce.dat" read at start-up).
// srclocs inlined: iconversorasn.h:71 (IConversorASN<CargoPergunta, CCargo>::Desconverte),
//                  celeicaope.cpp:63 (ValidaNome), :79 / :87 / :92 (ValidaCargos).
// =====================================================================================================
namespace comum::asn {

// wasm func 11371 (tools: comum::asn::CConversorEleicaoPE::vf3)
md::CEleicaoPE CConversorEleicaoPE::DoDesconverte(const ModuloEleicao::EntidadeEleicao& entidade) const
{
    const TEleicaoID id = entidade.get_cabecalho().get_idEleitoral().get_idEleicao();      // ?
    const TPleitoID idPleito = entidade.get_idPleito();
    const auto abrangencia = Utils::DesconverteAbrangencia(entidade.get_abrangencia());      // func 5827
    const TEleicaoID idPleito2 = entidade.has_idEleicaoPleito2() ? entidade.get_idEleicaoPleito2() : 0;

    // Every cargo goes through the CargoPergunta converter, created with the eleição's abrangência.
    const CConversorCargo conversorCargo(abrangencia);                                     // vtable @1569972
    std::vector<md::CCargo> cargos;
    for (const auto& cargo : entidade.get_cargos())
        cargos.push_back(conversorCargo.Desconverte(cargo));   // IConversorASN::Desconverte (h:71):
                                                               // !isStrictlyValid -> EUeComumAsnError 7654
                                                               // "Entidade está inválida: {}" + ASN trace
    std::vector<int> municipios;
    for (const auto& municipio : entidade.get_municipios())
        municipios.push_back(municipio);

    md::CEleicaoPE eleicao(id, idPleito, idPleito2, entidade.get_nome(), abrangencia, cargos, municipios);
    eleicao.ValidaNome();      // celeicaope.cpp:63: 1..38 characters, else CDadosError 8149 "Nome inválido."
    eleicao.ValidaCargos();    // celeicaope.cpp:79  empty           -> 8150 "Lista de cargos vazia."
                               //               :87  cargo abrangência != eleição -> 8151 "Há cargos com abrangência diferente."
                               //               :92  repeated código -> 8152 "Há cargos com mesmo identificador."
                               //  then ValidaOrdem (func 3714, :70, 8153) for "impressão" (+16),
                               //  "aquisição" (+15) and "apuração" (+17) of every cargo
    return eleicao;
}

} // namespace comum::asn

// =====================================================================================================
// uenux2/src/api/pattern/cpolysingletonlist.h  (unit u19)
// =====================================================================================================
// wasm func 1954 (tools: api_f1954): CPolySingletonList::exists<comum::CCalculaCV>(TPolySingletonsInfo&)
//   = merged 20-character-name body 6057(info, "N5comum10CCalculaCVE"...). Used by the BU data sources
//   (11260, 11980, 11981) and by instance<CCalculaCV> (1282).

// =====================================================================================================
// uenux2/src/api/util/cdatetime.cpp, cdate.cpp, ctime.cpp  (unit u20 owns cdatetime.cpp)
// =====================================================================================================
namespace api {

// wasm func 1382 (tools: api_f1382, observed executing): CDate::CDate() = today, from
// ISystemDateTime (singleton 1155, slot 0 -> time_t) + gmtime_r: {dia, mês, ano, dia da semana} (4 x u16).
CDate::CDate()
{
    const std::time_t agora = CPolySingletonList::instance<ISystemDateTime>().GetTime();
    std::tm tm;
    ::gmtime_r(&agora, &tm);
    m_dia = tm.tm_mday;
    m_mes = tm.tm_mon + 1;
    m_ano = tm.tm_year + 1900;
    m_diaSemana = tm.tm_wday;
}

// wasm func 2230 (tools: api_f2230, observed executing): CTime::CTime() = now, seconds since midnight.
CTime::CTime()
{
    const std::time_t agora = CPolySingletonList::instance<ISystemDateTime>().GetTime();
    std::tm tm;
    ::gmtime_r(&agora, &tm);
    m_segundos = tm.tm_sec + tm.tm_min * 60 + tm.tm_hour * 3600;
}

// wasm func 1000 (tools: api_f1000, observed executing): CDateTime::CDateTime(std::time_t) — default parts
// (today / now) then ConvertFromLocalTime(instante) (func 5476, cdatetime.cpp:61).
CDateTime::CDateTime(std::time_t instante)
    : m_data()
    , m_hora()
{
    ConvertFromLocalTime(instante);
}

} // namespace api

// FRAGMENTS reconstructed by unit u18 from vota_web_wasm.wasm - comum (and two ecourna) functions that the
// analyzer put in unit u18, mostly because an api::CFileASN / api::CDataMap template was inlined into them
// and they inherited its srcloc-based name. Each block names its real original file (path inferred unless
// attested). Offsets are byte offsets in the wasm32 objects.
//
// Constants used below:
//   VERSAO_VOTA = "10.23.0.1 - DESENVOLVIMENTO" (string @326597; the result files carry it as versaoVotacao)
//   CPath::Estatico(EFlashOrigem)  wasm_entry_f762: "/dsk/fi/estatico/" (INTERNA = 0) or "/dsk/fe/estatico/"
//                                  (EXTERNA = 1, the MV "mídia de votação" card)
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <vector>

#include "api/io/asn/cfileasn.h"
#include "api/io/cdatamap.h"

// =================================================================================================
// ecourna-lib/ecourna/api/util/cstringutils.cpp (attested for the class; the table "0123456789ABCDEF" is
// stored between the srcloc records of HexToQWord (line 754) and HexStringToBytes (line 808))
// =================================================================================================
namespace ecourna::api::util {

// wasm func 1243 (tools: comum_f1243; name inferred - the inverse of HexStringToBytes). Upper-case hex,
// two characters per byte. Callers: the BU/RDV/envelope writers (numeroSerieFV from infomidia.dat),
// CConversorEntidadeBU::DoConverte (10273), CCargos::GetCurrentEleicaoVersaoPacote, the QR/BU code (u04).
// Observed executing.
std::string CStringUtils::BytesToHexString(const std::vector<uebyte>& bytes)
{
    static constexpr char DIGITOS[] = "0123456789ABCDEF";                 // @1118016
    std::string texto;
    texto.reserve(bytes.size() * 2);
    for (const uebyte b : bytes) {
        texto.push_back(DIGITOS[b >> 4]);
        texto.push_back(DIGITOS[b & 0x0F]);
    }
    return texto;
}

// wasm func 9257 (tools: ecourna_f9257): a thunk that only calls 1243 - a second instantiation/overload
// folded onto the same body (merge-similar-functions). Caller: ecourna CConversorDadosGeracaoMidia slot 2.

}  // namespace ecourna::api::util

namespace comum {

// =================================================================================================
// uenux2/src/app/comum/gravadores/md/curna.cpp (attested: CUrna::ValidaCriacao, lines 58..69)
// md::CUrna = the "Urna" SEQUENCE of ModuloTiposResultadosEcoUrna (tipoUrna, versaoVotacao,
// correspondenciaResultado, tipoArquivo, numeroSerieFV, motivoUtilizacaoSA OPTIONAL), 136 bytes:
//   +0 EUrnaTipo m_tipoUrna ('0' invalid)   +4 std::string m_versaoVotacao
//   +16 CCorrespondenciaResultado m_correspondencia (88 bytes: município +16, zona +20, seção +22,
//       CCarga +24, ETipoUrna +100)
//   +104 ETipoArquivo m_tipoArquivo ('0' invalid)   +108 bool (always true)                          ?
//   +112 std::string m_numeroSerieFV (exactly 8 hex digits, else "Serial da MV inválido [...]")
//   +124 std::optional<CTipoApuracaoSA> m_motivoUtilizacaoSA (8 bytes, engaged flag +132)
// =================================================================================================
namespace md {

// wasm func 2854 (constructor without motivoUtilizacaoSA)                                    name inferred
CUrna::CUrna(EUrnaTipo tipoUrna, const std::string& versaoVotacao,
             const CCorrespondenciaResultado& correspondencia, ETipoArquivo tipoArquivo,
             const std::string& numeroSerieFV)
    : m_tipoUrna(tipoUrna), m_versaoVotacao(versaoVotacao), m_correspondencia(correspondencia)
    , m_tipoArquivo(tipoArquivo), m_flag108(true), m_numeroSerieFV(numeroSerieFV)
{
    ValidaCriacao();                                                      // wasm 5862
}

// wasm func 2855 (constructor with motivoUtilizacaoSA: the "sistema de apuração" was used)   name inferred
CUrna::CUrna(EUrnaTipo tipoUrna, const std::string& versaoVotacao,
             const CCorrespondenciaResultado& correspondencia, ETipoArquivo tipoArquivo,
             const std::string& numeroSerieFV, const CTipoApuracaoSA& motivoUtilizacaoSA)
    : m_tipoUrna(tipoUrna), m_versaoVotacao(versaoVotacao), m_correspondencia(correspondencia)
    , m_tipoArquivo(tipoArquivo), m_flag108(true), m_numeroSerieFV(numeroSerieFV)
    , m_motivoUtilizacaoSA(motivoUtilizacaoSA)
{
    ValidaCriacao();
}

// wasm func 1394 (tools: comum_f1394): CUrna::~CUrna() - frees the seven std::string members
// (+4, carga strings +28/+52/+64/+76/+88, +112). Implicitly defined.
CUrna::~CUrna() = default;

// uenux2/src/app/comum/dados/md/correspondencia/ccarga.h (u05)
// wasm func 1557 (tools: comum_f1557): CCarga::CCarga(const CCarga&) - implicitly defined copy constructor
// (int +0, string +4, 12 trivially copied bytes +16, strings +28/+40/+52/+64).
CCarga::CCarga(const CCarga&) = default;

}  // namespace md

// =================================================================================================
// Shared by the two writers below (inlined in both; also in CGravadorBU::GravaResultado 11629):
// numeroSerieFV = serial of the MV read from <CPath::Estatico(EXTERNA)>/infomidia.dat when it exists.
// In the simulator that file does not exist (the scenario has infomidia-fv-<turno>-t.dat), so every result
// file says numeroSerieFV = 00000000 (checked in samples/bu-real/run-full/analysis/tse_{bu,rdv}_dump.txt).
// =================================================================================================
namespace {
std::string LeSerialMV()                                                                  // name inferred
{
    std::string serial = "00000000";
    const std::string caminho = CPath::Estatico(EFlashOrigem::EXTERNA) / "infomidia.dat";
    if (api::CSystem::IsRegularFile(caminho)) {                                           // 412
        const auto info = api::CFileASN::ReadDataFromFile<
            ecourna::app::dados::asn::CConversorInformacaoMidia>(caminho);               // 3735
        serial = ecourna::api::util::CStringUtils::BytesToHexString(info.GetNumeroSerie());   // 1243 (+32)
    }                                                                                     // ~CInformacaoMidia = 3815
    return serial;
}

// eg.bin's DadoCarga: tipo de urna of the current turno ('1' -> tipoUrnaT1 +36, else tipoUrnaT2 +40).
md::EUrnaTipo TipoUrnaTurnoAtual(const md::estadoaplicacao::CEstadoGeral& eg)             // name inferred
{
    return eg.GetDadoCarga().GetTurno() == '1' ? eg.GetDadoCarga().GetTipoUrnaT1()
                                               : eg.GetDadoCarga().GetTipoUrnaT2();
}
}  // namespace

// =================================================================================================
// uenux2/src/app/comum/gravadores/cgravadorrdv.cpp (path inferred; class from RTTI, vtable @1556856)
// =================================================================================================

// wasm func 11587 (vtable slot 7 = IGravador::GravaResultado(api::CFile&) const; the signature is
// attested by CGravadorBU's override, cgravadorbu.cpp:484). Writes "<prefixo>-rdv.dat" in the MI work dir:
// ModuloRegistroDigitalVoto::EntidadeResultadoRDV {cabecalho, urna, rdv}, plain BER, not encrypted.
// Its srclocs are cfileasn.h:161/171 (CodeObjectFunction<EntidadeResultadoRDV>, inlined).
void CGravadorRDV::GravaResultado(api::CFile& arquivo) const
{
    // 1. the RDV as the urna keeps it: BER of EntidadeRegistroDigitalVoto (IRdv slot 11 = CRdvVota::Converte,
    //    u13), decoded again to be embedded as an ASN.1 value.
    const std::vector<uebyte> rdvBer = m_rdv->Converte();                                 // +164, slot 11
    const auto entidadeRdv = api::CFileASN::DecodeObject<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto>(
        std::vector<char>(rdvBer.begin(), rdvBer.end()));                                 // 5825

    const auto& configuracao = CConfiguracaoEleicao::GetInst();                           // 187
    const auto& eg = GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst());   // 291(185)
    const md::EUrnaTipo tipoUrna = TipoUrnaTurnoAtual(eg);
    const std::string serialMV = LeSerialMV();

    // 2. correspondência: this urna's carga (the IResultado copy of eg's CDadoCorrespondencia) + section
    const md::CCarga carga(m_numeroInternoUrna, m_serialMC, m_dataHoraCarga, m_codigoCarga,
                           m_geradorMidia);                                               // 2802 (+56,+60,+72,+84,+116)
    const md::CCorrespondenciaResultado correspondencia(
        m_municipio, m_zona, m_secao, carga,                                              // +4, +8, +16
        m_secao != 0 ? md::ETipoUrna::SECAO : md::ETipoUrna::CONTINGENCIA);               // '1' / '2'; 2801 validates
    const md::CCabecalhoEntidade cabecalho(m_dataGeracao, configuracao.GetPleito(),
                                           md::ETipoCabecalho(1));                        // 1945 (+40, cfg +28)

    const md::CUrna urna = m_motivoUtilizacaoSA                                          // flag +160
        ? md::CUrna(tipoUrna, VERSAO_VOTA, correspondencia, m_tipoArquivo, serialMV, *m_motivoUtilizacaoSA)   // 2855 (+52, +152)
        : md::CUrna(tipoUrna, VERSAO_VOTA, correspondencia, m_tipoArquivo, serialMV);                         // 2854

    // 3. the entity
    ModuloRegistroDigitalVoto::EntidadeResultadoRDV resultado;
    resultado.set_cabecalho(asn::CConversorCabecalhoEntidade().Converte(cabecalho));      // iconversorasn.h:56 instance
    resultado.set_urna(asn::CConversorUrna().Converte(urna));
    resultado.set_rdv(entidadeRdv);

    // 4. BER (CFileASN::CodeObjectFunction, lines 161/171: 5955 / 5956) and write
    const std::vector<char> ber = api::CFileASN::CodeObject(resultado);                   // "CodeObject de N25..."
    arquivo.RawWrite(std::vector<uebyte>(ber.begin(), ber.end()));                        // 5180
}

// =================================================================================================
// uenux2/src/app/comum/gravadores/igravadorenvelope.cpp (path inferred; class from RTTI, vtable @1554456,
// shared by comum::CGravadorEnvelopeArquivo @1554512)
// =================================================================================================

// wasm func 11626 (vtable slot 7 GravaResultado). Writes "-imgbu.dat" / "-imgze.dat": an
// EntidadeEnvelopeGenerico whose conteudo is the bytes returned by slot 8 (CGravadorEnvelopeArquivo::
// LeConteudo 11623 = the printed BU image trab/bu.dat, or the printed zerésima trab/ze.dat).
void IGravadorEnvelope::GravaResultado(api::CFile& arquivo) const
{
    const auto& eg = GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst());
    const md::EUrnaTipo tipoUrna = TipoUrnaTurnoAtual(eg);
    const std::string serialMV = LeSerialMV();

    const std::vector<uebyte> conteudo = LeConteudo();                                    // slot 8
    const md::CCabecalhoEntidade cabecalho(m_dataGeracao, CConfiguracaoEleicao::GetInst().GetPleito(),
                                           md::ETipoCabecalho(1));                        // 1945 (+40)
    const md::CCarga carga(m_numeroInternoUrna, m_serialMC, m_dataHoraCarga, m_codigoCarga,
                           m_geradorMidia);                                               // 2802 (+76,+80,+92,+104,+136)
    // Here the correspondência uses the eg tipo de urna (CGravadorRDV derives it from the seção number).
    const md::CCorrespondenciaResultado correspondencia(
        m_correspondencia.municipio, m_correspondencia.zona, m_correspondencia.secao,     // +116 / +120 / +122
        carga, md::ETipoUrna(tipoUrna));                                                  // 2801

    const md::CUrna urna = m_motivoUtilizacaoSA                                          // flag +68
        ? md::CUrna(tipoUrna, VERSAO_VOTA, correspondencia, m_tipoArquivo, serialMV, *m_motivoUtilizacaoSA)   // 2855 (+56, +60)
        : md::CUrna(tipoUrna, VERSAO_VOTA, correspondencia, m_tipoArquivo, serialMV);                         // 2854

    const md::CEnvelopeGenerico envelope(cabecalho, m_tipoEnvelope, urna,                 // 5860 (+52)
                                         m_municipio, m_zona, std::optional<TLocalID>(m_local),   // +4, +8, +12
                                         m_secao, m_campo72, conteudo);                   // +16, +72 ?
    api::CFileASN::WriteDataToFile<asn::CConversorEnvelopeGenerico>(arquivo, envelope);   // 2724 -> 6006
}

// =================================================================================================
// uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp (attested: SaveCurrentInternal :85)
// comum::CRegistradorMesario : api::CDataMap<md::CComparecimentoMesarioPK, md::CComparecimentoMesario>
// (the object of the singleton built by wasm 815, 52 bytes; name "CRegistradorMesario" inferred from the
// srcloc of the method inlined after this one):
//   +0  CDataMap (28 bytes; records: PK = {string +0, int +12, int +16}, value 52 bytes)
//   +28 std::map<md::CEleitorIdentidade, md::CComparecimentoMesario> m_porIdentidade  (key {string, int})
//   +40 servico::CComparecimentoMesarioServico m_servico (vptr; +44 shared_ptr<dao::IComparecimentoMesarioDAO>
//       from CDAORepositorio::Entregar, cdaorepositorio.hpp:79 - SQLite table comparecimento_mesario)
// =================================================================================================

// wasm func 5378 (tools: CDataMap<...>::Update because of the inlined srclocs cdatamap.h:158 and :143).
// Only caller: IGestorDadoMesario::GetControlador (10337), right before SaveCurrentInternal().
// The wasm takes one record: it is both the key (sliced to its PK) and the value.        name inferred
void CRegistradorMesario::AtualizaInternal(const md::CComparecimentoMesario& registro)
{
    if (registro.TemIdentidade())                                     // bool at record +20            ?
        m_porIdentidade.insert_or_assign(registro.GetIdentidade(), registro);   // find 1703 / emplace 5379
    if (Localiza(registro))                                           // CDataMap find + current (2222)
        Update(registro, registro);                                   // cdatamap.h:158 (inlined, 5979)
    else
        Add(registro, registro);                                      // cdatamap.h:143 (wasm 5377)
}

// =================================================================================================
// uenux2/src/app/comum/appinfo/servicos/iservicoestado.h (path inferred from the attested
// cservicoestadogeral*.cpp files) - comum::IServicoEstado<TDado, TConversor>
// vtable: [0]/[1] dtor, [2] GetPathArquivo() (CServicoEstadoGeral = 11569: <trab>/eg.bin).
// =================================================================================================

// wasm func 3791 (tools: CFileASN::ReadFromFile@3791). TDado = md::estadoaplicacao::CEstadoGeral,
// TConversor = asn::CConversorEstadoGeral, entity ModuloEstadoGeralUrna::EstadoGeralUrna (eg.bin).
// Callers: shared_f2273 (= CServicoEstadoGeral(INTERNA).Carrega()), mock 3897 and 11572 (votaInit).
// Observed executing.                                                                     name inferred
template <>
md::estadoaplicacao::CEstadoGeral
IServicoEstado<md::estadoaplicacao::CEstadoGeral, asn::CConversorEstadoGeral>::Carrega()
{
    const std::string caminho = GetPathArquivo();                                   // slot 2
    return api::CFileASN::ReadDataFromFile<asn::CConversorEstadoGeral>(caminho);     // lines 78/48/60/135/143,
                                                                                    // then iconversorasn.h:71 (7654)
}

// =================================================================================================
// uenux2/src/app/comum/dados/clocal.cpp (u04)
// =================================================================================================

// wasm func 5740 (tools: CFileASN::ReadFromFile@5740; name inferred - "Carrega" as in ctelasvota.u02.cpp).
// Loads "<município:05><zona:04><seção:04>-lo.dat" (ModuloLocal::Local) from the internal static dir,
// e.g. /dsk/fi/estatico/0000100010001-lo.dat, into m_local. Observed executing.
void CLocal::Carrega()
{
    // eg: the cached copy in CAppInfo when it is loaded (+180 == 1 and +132 == 0), else read eg.bin now
    const md::estadoaplicacao::CEstadoGeral eg = CAppInfo::GetInst().TemEstadoGeralEmCache()
        ? GetEstado<md::estadoaplicacao::CEstadoGeral>(CAppInfo::GetInst())                 // 291
        : CServicoEstadoGeral(EFlashOrigem::INTERNA).Carrega();                           // 2273 -> 3791
    const auto& local = eg.GetDadoLocal();                                                // município +20, zona +24, seção +26
    const std::string nome = FormataNumero(local.municipio, 5) + FormataNumero(local.zona, 4) +
                             FormataNumero(local.secao, 4) + "-" + "lo" + "." + "dat";    // 1161
    const std::string caminho = CPath::Estatico(EFlashOrigem::INTERNA) / nome;
    m_local = std::make_unique<md::CLocal>(
        api::CFileASN::ReadDataFromFile<asn::CConversorLocal>(caminho));                  // 140-byte md::CLocal
}                                                                                         // old one: ~md::CLocal 5744

// =================================================================================================
// uenux2/src/app/comum/dados/cpe.cpp (u04 calls it asn::LeProcessoEleitoral) - the processo eleitoral
// =================================================================================================
namespace asn {

// wasm func 5778 (tools: CFileASN::ReadFromFile@5778; name inferred). Builds one md::CPleito from its DTO:
//   situações:  <fase><pleito:05><uf>-ste.dat  (ModuloSituacoesEleicoes, optional: only if the file exists)
//   for every eleição of the pleito:
//     <fase><eleição:05><uf>-ce.dat  (ModuloEleicao::EntidadeEleicao -> CEleicaoPE via CConversorEleicaoPE)
//     <fase><eleição:05><uf>-ce.pid  (CabecalhoPacote, wasm 5706): the version of that data package,
//                                     kept in a map<eleição id, versão>
// Names built by func 1705; ".dat" -> ".pid" by func 5461. Only caller: LeProcessoEleitoral (3771).
md::CPleito LePleito(char fase, const std::string& uf, const std::filesystem::path& dirEstatico,
                     const md::CPleitoDTO& dto)
{
    std::vector<md::CSituacoesEleicoes> situacoes;
    const auto arqSituacoes = dirEstatico / NomeArquivo(fase, dto.id, uf, "ste", "dat");     // 1705
    if (api::CSystem::IsRegularFile(arqSituacoes))
        situacoes = api::CFileASN::ReadDataFromFile<CConversorSituacoesEleicoes>(arqSituacoes);   // 7654 path

    std::vector<md::CEleicaoPE> eleicoes;
    std::map<int, std::string> versoesPacote;
    for (const auto& ref : dto.eleicoes) {                                                // 8-byte entries
        const std::string nome = NomeArquivo(fase, ref.id, uf, "ce", "dat");
        eleicoes.push_back(api::CFileASN::ReadDataFromFile<CConversorEleicaoPE>(dirEstatico / nome));   // 5777
        const auto cabecalho = api::CFileASN::ReadDataFromFile<CConversorCabecalhoPacote>(
            dirEstatico / TrocaExtensao(nome, "pid"));                                    // 5461, 5706
        versoesPacote.emplace(cabecalho.GetId(), cabecalho.GetVersao());
    }
    return md::CPleito(dto.id, dto.campo4, dto.campo16, eleicoes, versoesPacote, situacoes);   // comum_md_CPleito_CPleito
}

// wasm func 3771 (tools: CFileASN::ReadFromFile@3771; name as in u04's cpe.cpp, inferred). Reads
// "<fase><processo:05>-cp.dat" (ModuloProcessoEleitoral, e.g. /dsk/fi/estatico/t02400-cp.dat) with
// CConversorProcessoEleitoral, then the pleito(s). Callers: CPE::CreateInst (2787), the start-up loader 7787.
// Observed executing.
md::CProcessoEleitoral LeProcessoEleitoral(const md::estadoaplicacao::CEstadoGeral& eg)
{
    const auto dirEstatico = CPath::Estatico(EFlashOrigem::INTERNA);
    const std::string nome = FormataFase(eg.GetFase()) + FormataNumero(eg.GetProcesso(), 5) + "-" + "cp" + "." + "dat";
    const md::CProcessoEleitoralDTO dto =
        api::CFileASN::ReadDataFromFile<CConversorProcessoEleitoral>(dirEstatico / nome);

    const md::CPleito pleito1 = LePleito(eg.GetFase(), eg.GetUF(), dirEstatico, dto.GetPleito1());
    std::string uf = eg.GetUF();
    std::ranges::transform(uf, uf.begin(), [](unsigned char c) { return std::toupper(c); });

    std::optional<md::CPleito> pleito2;
    if (dto.TemPleito2())                                                                 // +240
        pleito2 = LePleito(eg.GetFase(), eg.GetUF(), dirEstatico,
                           dto.GetPleito2());   // cprocessoeleitoraldto.cpp:57: 8166 "Não há pleito 2." if absent
    return md::CProcessoEleitoral(dto.GetId(), uf, dto.GetDescricao(), dto.campo8(), dto.flag(),
                                  pleito1, pleito2, /* ... */ {});                        // fields partly unnamed  // ?
}

}  // namespace asn

// =================================================================================================
// uenux2/src/app/comum/dados/crespostas.cpp area - api::CDataText<CRespostasDSNumero> (RTTI @1537856)
// =================================================================================================

// wasm func 12621 (IText slot 2; name inferred). The number of the current consulta answer, zero-padded to
// the width stored in the text object (+8, one byte: i32.load8_u). CRespostas::GetInst (crespostas.cpp:28) is inlined:
// "CRespostas - instancia nao criada" (EPatternErr 1303) when the singleton does not exist.
template <>
std::string api::CDataText<CRespostasDSNumero>::GetText() const
{
    const auto* resposta = CRespostas::GetInst().GetCurrent();                   // 3125 (cdatamap.h:98)
    return std::format("{:0{}}", resposta->numero, m_digitos);                   // +8 (uint8_t m_digitos)
}

}  // namespace comum

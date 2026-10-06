// FRAGMENTS reconstructed by unit u20 from vota_web_wasm.wasm.
// These comum functions were attributed to unit u20 only because an api/util, api/persistencia or
// api/uelog function of this unit is inlined into them. Each section names the original file
// (all paths inferred unless a srcloc is quoted) and the unit that owns the rest of that file.
#include <format>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "api/persistencia/cdaorepositorio.hpp"
#include "api/uelog/cloga.h"
#include "api/util/cdate.h"
#include "api/util/cgenerictags.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/dados/celeitores.h"
#include "comum/dados/clocal.h"
#include "comum/dao/icomparecimentomesariodao.h"
#include "comum/dao/ijustificadordao.h"
#include "comum/iinterfacesavd.h"
#include "comum/log/ieventoslog.h"
#include "comum/relatorios/cgeradorbuqrcode.h"

// =====================================================================================================
// uenux2/src/app/comum/relatorios/cgeradorbuqrcodevota.cpp  (path inferred; class also in u04 / u35)
// =====================================================================================================
namespace comum {

// wasm func 11242 (vtable slot 2 of CGeradorBUQRCodeVota; tools: comum::CGeradorBUQRCodeVota::vf2).
// Name as in unit u04. Fills the VOTA-specific fields of the header of the BU QR codes; the other
// fields (ORLC PROC DTPL PLEI TURN FASE UNFE ZONA SECA IDUE IDCA HIQT/HICA VERS) are filled by the
// caller from the BU header. Every field is "TAG:value " (with the trailing space), see docs/bu/qrcode.md.
// Inlined srclocs: cestadogeralvota.h:90 GetDtHrInicioAquisicao, :106 GetDtHrFimAquisicao.
// Generator members used: +416 ueword m_comparecimento, +418 bool m_origemRED, +419 bool m_incluiEmissao,
// +420 api::CDateTime m_dataEmissao.
void CGeradorBUQRCodeVota::PreencheCabecalho(CCabecalhoQRCode& cab) const
{
    auto& app = CAppInfo::GetInst();
    const bool temVota = app.TemVota(app.GetGeral().GetTurno());               // func 2841
    CLocal& local = CLocal::GetInst();                                          // func 401
    const SQtdeAptos aptos = CEleitores::GetInst().ContaAptos();                // func 2823 {secao, tte}

    cab.orig = std::format("ORIG:{} ", m_origemRED ? "RED" : "VOTA");          // +0

    // AGRE: sections aggregated into this one, "n1.n2.n3"
    // CLocal::GetAgregadas() is called twice: the first call is inlined (VerificaLido("GetAgregadas"),
    // then a copy of GetSecao()'s vector<ueword> when the section is loaded (data +120), else empty) and
    // only tested for emptiness; the second call is func 3752.
    if (!local.GetAgregadas().empty()) {
        std::string lista;
        for (ueword secao : local.GetAgregadas())                               // func 3752
            lista += std::format("{}.", secao);
        lista.pop_back();                                                        // drop the last '.'
        cab.agre = std::format("AGRE:{} ", lista);                              // +132
    }

    cab.loca = std::format("LOCA:{} ", local.GetLocalID());                     // +192, func 1933
    cab.apto = std::format("APTO:{} ", static_cast<ueword>(aptos.secao + aptos.tte));   // +204
    cab.apts = std::format("APTS:{} ", aptos.secao);                            // +216
    cab.aptt = std::format("APTT:{} ", aptos.tte);                              // +228
    cab.comp = std::format("COMP:{} ", m_comparecimento);                       // +240
    cab.muni = std::format("MUNI:{} ", app.GetGeral().GetMunicipio());          // +96 (CEstadoGeral +20)

    if (local.UrnaBiometrica()) {                                               // func 820 (+58)
        auto& eleitores = CEleitores::GetInst();
        cab.hbbm = std::format("HBBM:{} ", eleitores.QtdHabilitados(1));        // +264, func 2821: biometria
        cab.hbbg = std::format("HBBG:{} ", eleitores.QtdHabilitados(2));        // +276, func 1935: biográfica
        cab.hbsb = std::format("HBSB:{} ", eleitores.QtdVotaramSemBiometria()); // +288, func 2822
    }
    cab.falt = std::format("FALT:{} ",                                           // +252 (ueword arithmetic)
                           static_cast<ueword>(aptos.secao + aptos.tte - m_comparecimento));

    if (temVota) {
        const auto& vota = app.GetVota(EUrnaTurno::Atual);
        const api::CDateTime& inicio = vota.GetDtHrInicioAquisicao();   // throws EUeComumDadosError 8090
                                                                        // "Início da aquisição não marcado"
        const api::CDateTime& fim = vota.GetDtHrFimAquisicao();         // throws 8091
                                                                        // "Fim da aquisição não marcado"
        cab.dtab = std::format("DTAB:{} ", inicio.GetData().Format("YYYYMMDD"));   // +300, func 706
        cab.hrab = std::format("HRAB:{} ", inicio.GetHora().Format("hhmmss"));     // +312, func 779
        cab.dtfc = std::format("DTFC:{} ", fim.GetData().Format("YYYYMMDD"));      // +324
        cab.hrfc = std::format("HRFC:{} ", fim.GetHora().Format("hhmmss"));        // +336
    }
    if (m_incluiEmissao) {
        cab.dtem = std::format("DTEM:{} ", m_dataEmissao.GetData().Format("YYYYMMDD"));   // +372
        cab.hrem = std::format("HREM:{} ", m_dataEmissao.GetHora().Format("hhmmss"));     // +384
    }
}

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/comparecimentomesario/cregistradormesario.cpp  (path and class name inferred from
// the name string "CRegistradorMesario" stored in the object)
// uenux2/src/app/comum/servico/ccomparecimentomesarioservico.cpp       (path inferred)
// =====================================================================================================
namespace comum {

namespace servico {
// 12 bytes: +0 vptr (@1595676), +4/+8 std::shared_ptr<dao::IComparecimentoMesarioDAO> m_dao.
class CComparecimentoMesarioServico {
public:
    CComparecimentoMesarioServico()
        : m_dao(api::persistencia::CDAORepositorio::Entregar<dao::IComparecimentoMesarioDAO>())
    {
    }
    // wasm func 3607 (slot 0; body = shared helper comum_f2899(this, vtable): store vptr, release m_dao)
    // wasm func 10312 (slot 1) = deleting destructor
    virtual ~CComparecimentoMesarioServico() = default;
private:
    std::shared_ptr<dao::IComparecimentoMesarioDAO> m_dao;
};
} // namespace servico

// 52-byte cache of the mesário attendance rows (table comparecimento_mesario of uenux.db):
//   +0  std::map<md::CComparecimentoMesarioPK, md::CComparecimentoMesario> m_registros
//       (key: {std::string titulo, int tipoIdentificador, int periodo}; counted per periodo by func 6007)
//   +12 iterator m_ultimo (last lookup, initialised to end())
//   +16 std::string m_nome = "CRegistradorMesario"
//   +28 std::map<...> (second cache)                                                        // ?
//   +40 servico::CComparecimentoMesarioServico m_servico
class CRegistradorMesario {
public:
    // wasm func 815 (tools: api::persistencia::CDAORepositorio::Entregar@815, srcloc cdaorepositorio.hpp:79)
    // Lazy singleton @1909932 (mutex residue @1909908); the constructor, the service constructor and
    // CDAORepositorio::Entregar<IComparecimentoMesarioDAO>() are inlined.            name inferred
    static CRegistradorMesario& GetInst()
    {
        if (!s_instancia)
            s_instancia.reset(new CRegistradorMesario());
        return *s_instancia;
    }
    // wasm func 2221 (other unit): bool Existe() { return s_instancia != nullptr; }

    // wasm func 5380 (tools: api_f5380) - destructor: ~m_servico (3607), ~second map (vota_f1902),
    // ~m_nome, ~m_registros (comum_f2728).                                           name inferred
    ~CRegistradorMesario() = default;

    // wasm func 6007 (other unit) through thunks 3606 (periodo 1) / 2727 (periodo 2)
    unsigned QuantidadeRegistrados(int periodo) const;

private:
    CRegistradorMesario() : m_ultimo(m_registros.end()), m_nome("CRegistradorMesario") {}

    std::map<md::CComparecimentoMesarioPK, md::CComparecimentoMesario> m_registros;
    std::map<md::CComparecimentoMesarioPK, md::CComparecimentoMesario>::iterator m_ultimo;
    std::string m_nome;
    std::map<md::CComparecimentoMesarioPK, md::CComparecimentoMesario> m_pendentes;          // ?
    servico::CComparecimentoMesarioServico m_servico;

    static inline std::unique_ptr<CRegistradorMesario> s_instancia;                           // @1909932
};

// wasm func 10310 (tools: api_f10310): atexit destructor of s_instancia (reset -> func 5380 + free).

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/justificativa/cjustificador.cpp  (path and class name inferred from the name
// string "CJustificador" stored in the object; lazy singleton @1839012 = func 1391, other unit)
// =====================================================================================================
namespace comum {

// 40 bytes: +0 std::map<std::string titulo, ...> m_registros, +12 iterator m_ultimo, +16 std::string m_nome,
// +28 servico::CJustificadorServico {vptr @1560936, shared_ptr<dao::IJustificadorDAO> +32/+36}.
// wasm func 3742 (tools: api::persistencia::CDAORepositorio::Entregar@3742, srcloc cdaorepositorio.hpp:79)
// = constructor, with CJustificadorServico() and CDAORepositorio::Entregar<IJustificadorDAO>() inlined.
CJustificador::CJustificador()                                                          // name inferred
    : m_ultimo(m_registros.end())
    , m_nome("CJustificador")
    , m_servico()     // m_dao = api::persistencia::CDAORepositorio::Entregar<dao::IJustificadorDAO>()
{
}

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/comparecimentomesario/estados/cpedetitulomesario.cpp  (path from the constructor
// site: the three data sources are created in CPedeTituloMesario::StartState, func 10388)
// =====================================================================================================
namespace comum {
namespace {

// "Quantidade de mesários registrados: <n>" on the micro-terminal while poll workers register their
// attendance. Three instances of api::CDataTextFmt<CComparecimentoMesariosDS> exist (vtables @1594036,
// @1594188, @1594340); two bodies survive ICF:
//   wasm func 5387 (vtables 1 and 3): periodo 1 (opening)   wasm func 10368 (vtable 2): periodo 2 (closing)
template <int PERIODO>
struct CComparecimentoMesariosDS {
    unsigned operator()() const { return CRegistradorMesario::GetInst().QuantidadeRegistrados(PERIODO); }
};

} // namespace
} // namespace comum

namespace api {
// wasm funcs 5387 / 10368 (IText slot 2)                                               name inferred
// +12: std::string m_formato (e.g. "{}")
template <typename DS>
std::string CDataTextFmt<DS>::Text() const
{
    return std::vformat(m_formato, std::make_format_args(m_fonte()));
}
} // namespace api

// =====================================================================================================
// uenux2/src/app/comum/iinterfacesavd.cpp  (the SAVD = the urna's signing/validation service;
// other functions of this file: u01/u02/u07)
// =====================================================================================================
namespace comum {
namespace {

const std::string TAG_PACOTE = "pkg";
const std::string TAG_CHAVE = "key";
const std::string TAG_ARQUIVO = "fil";
const std::string TAG_SUPER = "sup";                    // static std::string (@494896 literal)

api::CGenericTags CriaTags()                            // inlined in 3831 and 5892      name inferred
{
    api::CGenericTags tags(4, 3);
    tags.insert(TAG_PACOTE, 1);
    tags.insert(TAG_CHAVE, 1);
    tags.insert(TAG_ARQUIVO, 1);
    tags.insert(TAG_SUPER, 3);
    return tags;
}

} // namespace

// wasm func 3831 (tools: api_f3831) - observed executing. Callers: CPacoteArquivos::
// ValidarChaveEAplicacaoValida, comum::ValidarUE (func 5890), func 5892.                  name inferred
// "sup{ pkg{<pacote>} key{<chave in decimal>} }"
std::string MontaMensagemPacote(const std::string& pacote, int chave)
{
    const api::CGenericTags tags = CriaTags();
    std::string corpo;
    {
        const api::CGenericTags interno = CriaTags();
        corpo = interno.EncodeTLV(TAG_PACOTE, pacote);                               // func 3647
        interno.AppendTLV(corpo, TAG_CHAVE, std::format("{}", chave));               // func 5466
    }
    return tags.EncodeTLV(TAG_SUPER, corpo);
}

// wasm func 5892 (tools: api::CGenericTags::WalkTreeTLV) - observed executing. Callers:
// CPacoteArquivos::ValidarChaveEAplicacaoValida (4625), CSigVerifier (11179).        name inferred
// Builds "sup{pkg, key, fil}" and sends it to the SAVD with a request header
// {0x00, aplicacao, 0x01, 0x10, 0...} (the 64-bit constant 0x10010000 with byte 1 = aplicacao; for
// comparison ValidarUE uses 0x20210000). The request/response exchange is func 3830 (named
// IInterfaceSavd::DesconverteMensagem by the tools because of inlined srclocs).
bool IInterfaceSavd::AssinarVerificarArquivo(ESavdAplic aplicacao, ueint32 chave,
                                             const std::string& pacote, const std::string& arquivo)
{
    if (static_cast<unsigned>(aplicacao) > 255 || chave > 255 || pacote.empty() || arquivo.empty()) {
        m_ultimoErro = "Argumento inválido assinar verificar arquivo.";       // +4
        return false;
    }
    const api::CGenericTags tags = CriaTags();
    std::string mensagem = MontaMensagemPacote(pacote, chave);                // func 3831
    std::string corpo = tags.DecodeTLV(mensagem, TAG_SUPER);                  // WalkTreeTLV + erase
    tags.AppendTLV(corpo, TAG_ARQUIVO, arquivo);
    const std::string requisicao = tags.EncodeTLV(TAG_SUPER, corpo);

    SRequisicaoSavd cabecalho{};                                              // 8 bytes          // ?
    cabecalho.bytes = 0x10010000;
    cabecalho.aplicacao = static_cast<uebyte>(aplicacao);
    return Executa(m_conexao /* +16 */, cabecalho, requisicao);               // func 3830
}

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/log/ieventoslog.cpp  (comum::IEventosLog, base of vota::CLogVota: 8 bytes,
// +4 ELogAplicativos m_aplicativo; other functions: u09 / u12)
// Both are called by votaInit through the function table (invoke slots 37 and 38) on
// CLogVota::GetInst(). Log texts are Latin-1 in the binary ("1\xBA turno", "Inv\xE1lido").
// =====================================================================================================
namespace comum {

// wasm func 11641 (tools: comum_f11641) - observed executing                          name inferred
void IEventosLog::LogaInicioAplicacao(EUrnaTurno turno)
{
    std::string texto;
    switch (turno) {
    case EUrnaTurno::SemTurno: texto = "Sem turno"; break;     // '0'
    case EUrnaTurno::Primeiro: texto = "1º turno";  break;     // '1'
    case EUrnaTurno::Segundo:  texto = "2º turno";  break;     // '2'
    case EUrnaTurno::Atual:    texto = "Padrão";    break;     // '3'
    default:                   texto = "Inválido";  break;
    }
    api::CLoga::loga(m_aplicativo, api::ESeveridade{1}, std::format("Iniciando aplicação - {}", texto));
}

// wasm func 11642 (tools: api_f11642) - observed executing                            name inferred
void IEventosLog::LogaVersaoAplicacao()
{
    api::CLoga::loga(m_aplicativo, api::ESeveridade{1},
                     std::format("Versão da aplicação: {}", std::string_view("10.23.0.1 - DESENVOLVIMENTO")));
}

} // namespace comum

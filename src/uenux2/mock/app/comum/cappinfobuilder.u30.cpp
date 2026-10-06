// FRAGMENT of uenux2/mock/app/comum/cappinfobuilder.cpp reconstructed by unit u30 from vota_web_wasm.wasm.
// The class declaration is in cappinfobuilder.u28.h (unit u28: SalvaGeral, SalvaApps, SalvaApp); unit u29 has
// Converte(EMidia), the turno thunks' shared body (6004) and the destructor (5644). This fragment: the
// constructor, the fluent setters, Salva, the "CriaEstado" rebuilders, the fixture values and EscreveArquivo.
// File attribution of every function here is inferred from callers/callees ("path inferred").
//
// comum::teste::CAppInfoBuilder is a TEST fixture of the urna code base (uenux2/mock/, namespace "teste") that
// the web build runs in production: CVotaWebEngine::Init (votaInit, func 7840) uses it, the first time only
// (when <MI>/dinamico/eg.bin does not exist), to fabricate the whole persistent state of an urna that has been
// "loaded" (carga) and tested:
//
//   comum::teste::CAppInfoBuilder builder(pe);                  // func 10268
//   builder.SetFase(fase).SetTurno(turno).SetTipoUrna('1')      // 10197, 10188, 10181
//          .SetLocal(municipio, zona, secao);                   // 10176
//   builder.<geral>.uf = ToUpper(uf); vota[0..1] = {inicial, treinamentoEleitor}   (inlined in votaInit)
//   builder.Salva(false, {MI, MV});                             // 10256
//
// Additional members declared by this fragment (to be merged into cappinfobuilder.h):
//   explicit CAppInfoBuilder(TPEID idPE);                                         // 10268
//   CAppInfoBuilder& SetFase(md::estadoaplicacao::EFase fase);                   // 10197
//   CAppInfoBuilder& SetTurno(EUrnaTurno turno);                                  // 10188
//   CAppInfoBuilder& SetTipoUrna(ETipoUrna tipo);                                 // 10181
//   CAppInfoBuilder& SetLocal(TMunicipioID municipio, TZonaID zona, TSecaoID secao);   // 10176
//   CAppInfoBuilder& Salva(bool assina, std::vector<EMidia> midias);             // 10256
//   CAppInfoBuilder& SalvaAppsPrimeiroTurno(bool, std::vector<EMidia>, std::vector<EApp>);   // 10242 (u29: 6004)
//   CAppInfoBuilder& SalvaAppsSegundoTurno(bool, std::vector<EMidia>, std::vector<EApp>);    // 10239 (u29: 6004)
//   static std::vector<EApp> TodosApps();                                         // 10286
//
// Layout (500 bytes), identical to the md classes' layouts (see cappinfobuilder.u28.h):
//   +0   CEstadoGeral (180): +0 estadoUrna, +4 idPE, +8 CDadoLocal {+8 uf string, +20 municipio u32,
//        +24 zona u16, +26 secao u16, +28 tipoLocal}, +32 CDadoCarga {turno, tipoUrnaT1, tipoUrnaT2, modelo, fase},
//        +52 CAjusteDataHora {tipo, delta}, +60 CDadoCorrespondencia (96), +156 versao, +168 hashVersoesPacotes
//   +180 CEstadoGeralGap[2] (44 each)   +268 CEstadoGeralSA[2] (16 each)   +300 CEstadoGeralVota[2] (100 each)
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

#include "api/util/cdate.h"
#include "api/util/cdatetime.h"
#include "ecourna/api/util/cstringutils.hpp"
#include "ecourna/app/dados/midias/cidentificadorgeradormidia.h"
#include "mock/app/comum/cappinfobuilder.u28.h"

namespace comum::teste {

namespace md = comum::md::estadoaplicacao;

// =========================================================================================================
// Fixture values. The wasm constructs these objects IN PLACE, field by field, without calling the md
// member-wise constructors (5637/5630/5626/5628/5625) or their validations; the three bigger ones are
// out-of-line constructors (this-return) with no argument. They are therefore default constructors / default
// member initialisers of the types held by the builder - the md classes as compiled for this build, or
// builder-side types with the same layout (the binary cannot tell). Only the builder uses them.        // ?
// =========================================================================================================

namespace {

// wasm func 9963 (table slot 410) - observed executing                                   name inferred
// A fake 139-byte signature (139 = the maximum DER size of an ECDSA P-521 signature): the 21-character pattern
// repeated and cut at 278 hex digits, then decoded. Bytes: 12 34 56 78 9a bc de fa bc de f1 23 45 67 89 ab
// cd ef ab cd ef 12 34 ... (the pattern is an odd number of digits, so the byte alignment shifts each round).
std::vector<uebyte> AssinaturaFicticia()
{
    std::string hex;                                  // the literal lives on the stack (22 bytes copied from rodata)
    hex.reserve(556);                                 // func 7735 = std::string::reserve
    while (hex.size() <= 277)
        hex += "123456789abcdefABCDEF";               // @466384; 14 appends -> 294 digits
    hex = hex.substr(0, 278);
    return ecourna::api::util::CStringUtils::HexStringToBytes(hex);    // func 3506
}

}  // namespace

// wasm func 9952 (table slot 411) - observed executing                                   name inferred
// Default of the "identificador do gerador de mídia" (the machine that generated the load media). Real media
// carry the TSE generator host name and its TPM certificate serial (2024 examples: "tsesevinw09").
//   ecourna::app::dados::CIdentificadorGeradorMidia::CIdentificadorGeradorMidia()   (3 std::string, 36 bytes)
//       : m_nome("nome_maquina"),                    // @227209
//         m_serialCertificadoTPM("12345678"),        // @341300
//         m_serialInstalacao("99999999") {}          // @339893

// wasm func 10261 (table slot 404) - observed executing                                  name inferred
// Default of the load correspondence (md::CDadoCorrespondencia, 96 bytes): what the BU prints as
// urna.correspondenciaResultado.carga in the simulator.
//   md::CDadoCorrespondencia::CDadoCorrespondencia()
//       : m_idUrna(87654321),                                        // +0  numeroInternoUrna (= CUrnaMock serial)
//         m_numeroSerieFC("12345678"),                               // +4  serial of the load flash (FC)
//         m_dataHoraCarga(api::CDateTime("31122020235958")),          // +16 func 5475: 31/12/2020 23:59:58
//         m_codigoCarga("123456789012345678901234"),                 // +28 @346764 (24 digits)
//         m_localidade(1, 1, 1),                                     // +40 i64 0x0001000100000001 (no validation)
//         m_assinatura(AssinaturaFicticia()),                        // +48 func 9963
//         m_gerador() {}                                             // +60 func 9952

// wasm func 10278 (table slot 408) - observed executing                                  name inferred
// Default of the launcher state (md::CEstadoGeralGap, 44 bytes):
//   md::CEstadoGeralGap::CEstadoGeralGap()
//       : m_correspondencias(),                                  // +0  empty vector
//         m_appId(EUrnaAplicativo{14}),                          // +12 14 = "apsemaplicativo" (no application)
//         m_appAnteriorId(EUrnaAplicativo{14}),                  // +16
//         m_executadoRED(false), m_identificadoATUE(false),      // +20, +21
//         m_audio(false), m_data2T(false),                       // +22, +23
//         m_numVias1T{}, m_numVias2T{},                          // +24, +28 (4 x uebyte each)
//         m_dataSegundoTurno(api::CDate("31122080")) {}          // +32 func 3649, flag +40: 31/12/2080

// =========================================================================================================
// wasm func 10268 (table slot 24) - observed executing                                    name inferred
// The constructor. Everything but idPE is a fixture value; votaInit then overrides fase, turno, tipo de urna,
// município/zona/seção, UF and the two vota states. Exception paths destroy the members already built.
//
// NOTE - the initialiser list below is NOTATION for the values, not the code shape. The binary never calls the
// md member-wise constructors here (no 5637/5632/5631/5633/5625/5628, so none of their validations run): it
// stores the scalars field by field and constructs each non-trivial sub-object DIRECTLY at its member
// address - the "EE" string at +8, CDadoCorrespondencia by func 10261 at +60, the version string at +156, the
// hash vector (func 10258) at +168, the two gaps by func 10278 at +180/+224. A by-value argument of a
// member-wise constructor could not be built in place like that (10261 receives a+60 itself). This is the
// shape of default member initialisers / default constructors of the member types, followed by
// m_geral.SetIdPE(idPE) (the 15000 placeholder is stored first and overwritten by the last store).        // ?
// =========================================================================================================
CAppInfoBuilder::CAppInfoBuilder(TPEID idPE)
    : m_geral(md::EEstadoUrna{'3'},                       // +0  '3' = testada ("tested": the carga is complete)
              15000,                                      // +4  placeholder idPE, overwritten in the body
              md::CDadoLocal("EE",                        // +8  UF placeholder (@331321), votaInit sets ToUpper(uf)
                             md::CLocalidadeEleitoral(1, 1, 1),   // +20/+24/+26 município/zona/seção
                             md::ETipoLocal{1}),          // +28
              md::CDadoCarga(EUrnaTurno{'1'},             // +32 turno 1
                             ETipoUrna{'1'}, ETipoUrna{'1'},      // +36/+40 tipo de urna T1/T2 ('1' = seção)
                             md::EUrnaModelo::UE2015,     // +44 2015
                             md::EFase{'2'}),             // +48 '2' = simulado
              md::CAjusteDataHora{},                      // +52 {0, 0}
              md::CDadoCorrespondencia{},                 // +60 func 10261
              "7.2.1.3 - TESTE EG ASN1",                  // +156 @354064: the software version recorded in eg.bin
              std::vector<uebyte>(64, 0x0A)),             // +168 hashVersoesPacotes = 64 bytes '\n' (func 10258)
      m_gap{md::CEstadoGeralGap{}, md::CEstadoGeralGap{}},          // +180, +224: func 10278 twice
      m_sa{md::CEstadoGeralSA(md::EEstadoSA{'1'}, false, 1, 1, 2),  // +268: inicial, not blocked, 1/1/2
           md::CEstadoGeralSA(md::EEstadoSA{'1'}, false, 1, 1, 2)}, //       (secao 2, unlike the geral's 1)
      m_vota{md::CEstadoGeralVota(md::EEstadoVota{'1'}, md::EEstadoEncerramento{'1'}, 0,   // +300: inicial,
                                  std::nullopt, std::nullopt, std::nullopt, 0, 0,          // encerramento
                                  std::nullopt, false, {}, std::nullopt, false),           // inicial, zeros
             md::CEstadoGeralVota(md::EEstadoVota{'1'}, md::EEstadoEncerramento{'1'}, 0,   // +400
                                  std::nullopt, std::nullopt, std::nullopt, 0, 0,
                                  std::nullopt, false, {}, std::nullopt, false)}
{
    m_geral.SetIdPE(idPE);                                // +4 (the last store of the function)          ?
}

// =========================================================================================================
// Fluent setters (all return *this; never inlined into votaInit: another translation unit).
// =========================================================================================================

// wasm func 10197 (table slot 25)                                                         name inferred
CAppInfoBuilder& CAppInfoBuilder::SetFase(md::EFase fase)             // '1' oficial, '2' simulado, '3' treinamento
{
    m_geral.GetCarga().SetFase(fase);                                 // +48                            ?
    return *this;
}

// wasm func 10188 (table slot 26)                                                         name inferred
CAppInfoBuilder& CAppInfoBuilder::SetTurno(EUrnaTurno turno)          // votaInit: turno == 2 ? '2' : '1'
{
    m_geral.GetCarga().SetTurno(turno);                               // +32                            ?
    return *this;
}

// wasm func 10181 (table slot 27)                                                         name inferred
CAppInfoBuilder& CAppInfoBuilder::SetTipoUrna(ETipoUrna tipo)         // same type for both turnos
{
    m_geral.GetCarga().SetTipoUrnaT1(tipo);                           // +36                            ?
    m_geral.GetCarga().SetTipoUrnaT2(tipo);                           // +40
    return *this;
}

// wasm func 10176 (table slot 28)                                                         name inferred
// votaInit passes the zona and seção already truncated to 16 bits (JSON int & 0xFFFF).
CAppInfoBuilder& CAppInfoBuilder::SetLocal(TMunicipioID municipio, TZonaID zona, TSecaoID secao)
{
    m_geral.GetLocal().SetLocalidade(municipio, zona, secao);         // +20 u32, +24 u16, +26 u16      ?
    return *this;
}

// =========================================================================================================
// wasm func 10286 (table slot 415) - observed executing                                  name inferred
// Called twice by Salva (one temporary per call).
// =========================================================================================================
std::vector<EApp> CAppInfoBuilder::TodosApps()
{
    return {EApp::Gap, EApp::SA, EApp::Vota};         // {0, 1, 2}; vector(initializer_list) inlined
}

// =========================================================================================================
// wasm func 10256 (table slot 32) - observed executing                                   name inferred
// "Save everything": eg.bin on every medium, then gap.bin/sa.bin/vota.bin of both turnos. votaInit calls it
// with assina = false and midias = {MI, MV}: the .vsu "signatures" are written afterwards by
// WriteSimulatedSignatures (vota_web_wasm.u30.cpp) with a different fixed text.
// Every call below receives fresh copies of `midias` (by-value parameters; copy ctor func 2720 -> 6005).
// The two turno helpers are thunks 10242 ('1') and 10239 ('2') of the shared body func 6004 (unit u29), which
// copies both vectors again for SalvaApps (func 10212).
// =========================================================================================================
CAppInfoBuilder& CAppInfoBuilder::Salva(bool assina, std::vector<EMidia> midias)
{
    return SalvaGeral(assina, midias)                                      // func 10243
        .SalvaAppsPrimeiroTurno(assina, midias, TodosApps())               // func 10242 -> 6004(.., '1')
        .SalvaAppsSegundoTurno(assina, midias, TodosApps());               // func 10239 -> 6004(.., '2')
}

// =========================================================================================================
// "CriaEstado": rebuild each stored state through the md member-wise constructors before it is serialised
// (names from unit u28's header). This re-runs the md validations that the in-place fixture skipped:
// CLocalidadeEleitoral (município < 100000, zona < 10000, seção < 10000: errors 8095-8097), CDadoCarga
// (model 2013..2022, fase '1'..'3': 8075/8077), and the gap's data2T recomputation.
// Field order = the md constructors' parameter order (see the u03 reconstructions of 5637/5626/5628/5625).
// =========================================================================================================

// Leaf copy helpers: one overload set in the anonymous namespace. Unit u29 reconstructs two members of the same
// set, Copia(CNumViasImpressasRelatorios) = func 5324 and Copia(CIdentificadorGeradorMidia) = func 9862
// (cappinfobuilder.u29.cpp), so the u30 members use the same name. func 10162 (tools: libcxx_f10162, table
// slot 424 between 10167 and 10155) is most likely Copia(CAjusteDataHora): its body just stores {tipo, delta}.
namespace {

// wasm func 9863 (table slot 447) - observed executing                                   name inferred
md::CLocalidadeEleitoral Copia(const md::CLocalidadeEleitoral& l)
{
    return md::CLocalidadeEleitoral(l.GetMunicipio(), l.GetZona(), l.GetSecao());    // func 5631 (validates)
}

// wasm func 10167 (table slot 423) - observed executing                                  name inferred
md::CDadoCarga Copia(const md::CDadoCarga& c)
{
    return md::CDadoCarga(c.GetTurno(), c.GetTipoUrnaT1(), c.GetTipoUrnaT2(),        // func 5633 (validates
                          c.GetModelo(), c.GetFase());                               // the model and the fase)
}

// wasm func 10162 (table slot 424; not in this unit, tools: libcxx_f10162)               name inferred ?
md::CAjusteDataHora Copia(const md::CAjusteDataHora& a)
{
    return md::CAjusteDataHora(a.GetTipo(), a.GetDelta());                           // {+0 tipo, +4 delta}
}

// wasm func 10155 (table slot 425) - observed executing                                  name inferred
md::CDadoCorrespondencia Copia(const md::CDadoCorrespondencia& c)
{
    return md::CDadoCorrespondencia(c.GetIdUrna(),                          // +0
                                    c.GetNumeroSerieFC(),                   // +4  (string copy, func 243)
                                    c.GetDataHoraCarga(),                   // +16 (12-byte CDateTime copy)
                                    c.GetCodigoCarga(),                     // +28
                                    Copia(c.GetLocalidade()),               // +40 func 9863
                                    c.GetAssinatura(),                      // +48 vector copy (func 10145)
                                    Copia(c.GetGerador()));                 // +60 func 9862 (unit u29)
                                                                            // ctor func 5630
}

}  // namespace

// wasm func 10205 (table slot 421) - observed executing (called by SalvaGeral)          name inferred
md::CEstadoGeral CAppInfoBuilder::CriaEstado(const md::CEstadoGeral& g)
{
    md::CDadoLocal local(std::string(g.GetLocal().GetUF()),                  // copy of +8
                         Copia(g.GetLocal().GetLocalidade()),                 // func 9863
                         g.GetLocal().GetTipoLocal());                        // +28; ctor func 5632
    return md::CEstadoGeral(g.GetEstado(),                                    // +0
                            g.GetIdPE(),                                      // +4
                            std::move(local),
                            Copia(g.GetCarga()),                              // func 10167
                            Copia(g.GetAjuste()),                             // func 10162 (slot 424)
                            Copia(g.GetCorrespondencia()),                    // func 10155
                            std::string(g.GetVersao()),                       // +156
                            std::vector<uebyte>(g.GetHashVersoesPacotes()));  // func 10145; ctor func 5637
}

// wasm func 10123 (table slot 432) - observed executing (SalvaApps, EApp::Gap)          name inferred
md::CEstadoGeralGap CAppInfoBuilder::CriaEstado(const md::CEstadoGeralGap& gap)
{
    // vector<CDadoCorrespondencia> copy: __vallocate func 9905 (96-byte elements, max 44739242) +
    // __construct_at_end func 9912.
    return md::CEstadoGeralGap(gap.GetCorrespondencias(),
                               gap.GetAppId(), gap.GetAppAnteriorId(),                  // +12, +16
                               gap.GetExecutadoRED(), gap.GetIdentificadoATUE(),        // +20, +21
                               gap.GetAudio(), gap.GetData2T(),                         // +22, +23
                               gap.GetNumViasImpressas1T(), gap.GetNumViasImpressas2T(), // +24, +28 (func 5324)
                               gap.GetDataSegundoTurno());                              // +32 (optional<CDate>)
    // ctor func 5626 recomputes data2T = dataSegundoTurno <= today (false for the 2080 fixture)
}

// wasm func 10101 (table slot 435) - SalvaApps, EApp::SA                                 name inferred
md::CEstadoGeralSA CAppInfoBuilder::CriaEstado(const md::CEstadoGeralSA& sa)
{
    return md::CEstadoGeralSA(sa.GetEstado(), sa.GetAtualizacaoBloqueada(),      // +0, +4
                              sa.GetMunicipio(), sa.GetZona(), sa.GetSecao());   // +8, +12, +14; ctor 5625
}

// wasm func 10067 (table slot 439) - SalvaApps, EApp::Vota                               name inferred
md::CEstadoGeralVota CAppInfoBuilder::CriaEstado(const md::CEstadoGeralVota& v)
{
    return md::CEstadoGeralVota(v.GetEstado(),                   // +0  estadoVota
                                v.GetEstadoEncerramento(),       // +4
                                v.GetQtdBU(),                    // +8  (uebyte)
                                v.GetUrnaIdGerouZeresima(),      // +12 optional<uedword>
                                v.GetDtHrInicioAquisicao(),      // +20 optional<CDateTime>
                                v.GetDtHrFimAquisicao(),         // +36 optional<CDateTime>
                                v.GetComparecimento(),           // +52 ueword
                                v.GetQtdJustificativa(),         // +54 ueword
                                v.GetDtHrUltimoVoto(),           // +56 optional<CDateTime>
                                v.GetTreinamentoEleitor(),       // +72 bool
                                v.GetNumViasImpressas(),         // +73 4 x uebyte (func 5324)
                                v.GetDtHrEmissao(),              // +80 optional<CDateTime>
                                v.GetTecladoTestadoPosConversao());   // +96 bool; ctor func 5628
}

// =========================================================================================================
// wasm func 10293 (table slot 420)                                                        name inferred
// Writes `conteudo` to `arquivo`, creating the parent directories. Used only for the "assinatura EG ..." .vsu
// placeholders of SalvaGeral/SalvaApp, i.e. only when assina == true - never in the web build.
// path(const std::string&) = func 5641, parent_path = 3706, create_directories = 12121,
// basic_filebuf::open(const std::string&, mode) = func 10304 with mode 20 = ios::out | ios::binary
// (libc++: binary 0x04, out 0x10; NOT out|trunc - but opening with `out` alone truncates anyway).
// =========================================================================================================
void EscreveArquivo(std::string arquivo, std::string conteudo)
{
    std::filesystem::create_directories(std::filesystem::path(arquivo).parent_path());
    std::ofstream saida(arquivo, std::ios::binary);
    saida << conteudo;
}

}  // namespace comum::teste

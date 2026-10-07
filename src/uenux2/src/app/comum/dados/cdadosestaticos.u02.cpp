// FRAGMENTS written by unit u02 for several comum data-set files (owners: u03/u04/u05/u24).
// One section per original file; every function below was attributed to chkdfseed.cpp only because
// wasm func 7787 (see vota/eleitor/comum/cinformacaoeleitor.cpp) is its caller.
//
// Singleton pattern used by all these classes (lazy creation, residue of a mutex unlock at a
// neighbouring address, and an atexit handler that deletes the object):
//   static std::unique_ptr<C> ms_instancia;  C& GetInst() { if (!ms_instancia) ms_instancia.reset(new C()); return *ms_instancia; }

#include <map>
#include <set>
#include <string>
#include <vector>

namespace comum {

// =============================================================================================
// uenux2/src/app/comum/dados/cfederacoes.cpp   (owner u04? file known from srcloc cfederacoes.cpp:141/149)
// CFederacoes (28 bytes): +0 std::map<TNumeroFederacao, CFederacao> m_federacoes, +16 std::string m_nome.

// wasm func 2832                                                       // name inferred
CFederacoes& CFederacoes::GetInst()
{
    static std::unique_ptr<CFederacoes> ms_instancia;          // @1838748, atexit wasm 11553
    if (!ms_instancia)
        ms_instancia.reset(new CFederacoes());                // map empty, m_nome = "CFederacoes"
    return *ms_instancia;
}

// wasm func 5800: CFederacoes::~CFederacoes()  (m_nome, then the map through wasm 1272)
CFederacoes::~CFederacoes() = default;

// ecourna::app::dados::CFederacao (40 bytes): +0 ushort numero, +4 std::string sigla,
// +16 std::string nome, +28 std::vector<uebyte> partidos.
// wasm func 3780: implicit copy constructor CFederacao(const CFederacao&).
// wasm func 5796: std::vector<CFederacao>::__construct_at_end(map_iterator first, n) (copies
//                 map values: `new (end) CFederacao(it->second)`, n times).
// wasm func 5797: std::move_backward for CFederacao (used by vector::insert).
// wasm func 2831: __uninitialized_allocator_move_if_noexcept for CFederacao + destroy of the source.
// wasm func 3779: std::__split_buffer<CFederacao>::~__split_buffer().
// wasm func 3781: std::__tree<map<K, V>>::destroy(node): V holds a vector (+44) and two strings
//                 (+32, +20) -> the per-file map<TNumeroFederacao, CFederacao> built by Load.
// (These are the std::vector<CFederacao>::insert(pos, first, last) inlined in CFederacoes::Load.)

// =============================================================================================
// uenux2/src/app/comum/dados/cpartidos.cpp   (path inferred; no srcloc for this file)
// CPartidos (28 bytes): +0 std::map<TNumeroPartido, CPartido>, +16 std::string m_nome = "CPartidos".
// GetInst is wasm func 819 (not in this unit), destructor wasm 2815.
// wasm func 1932: std::__tree<map<TNumeroPartido, CPartido>>::destroy(node) (value: two strings at
//                 node+24 and node+36: sigla and nome).

// =============================================================================================
// uenux2/src/app/comum/dados/cfotos.cpp   (owner: u? ; srcloc cfotos.cpp:111/121)
// CFotos (28 bytes): +0 std::map<std::string, CIndiceFoto>, +16 std::string m_nome = "CFotos".

// wasm func 2819                                                       // name inferred
CFotos& CFotos::GetInst()
{
    static std::unique_ptr<CFotos> ms_instancia;               // @1838844 (ctor wasm 3756, dtor wasm 2818)
    if (!ms_instancia)
        ms_instancia.reset(new CFotos());
    return *ms_instancia;
}
// wasm func 1934: std::__tree<map<std::string, CIndiceFoto>>::destroy(node)
//                 (three strings in the node: +16 key, +28, +48).
// wasm func 5704: std::vector<SFotoRef>::assign(first, last, n) where SFotoRef = {std::string nome;
//                 std::shared_ptr<...> dado;} (20 bytes); wasm 5703 is its std::copy part.
//                 Also used by std::function<CEleitores::GetEleitoresEstaticos::$_0>::operator().

// =============================================================================================
// uenux2/src/app/comum/dados/ccandidaturas.cpp   (owner u03; srcloc ccandidaturas.cpp:201/208/245)
// CCandidaturas (44 bytes, GetInst wasm 521, dtor wasm 3786): +0 map<..., CCandidatura>,
// +16 std::string "CCandidaturas", +32 map, +40 bool.
// wasm func 1709: std::__tree<map<SChaveCandidatura, CCandidatura>>::destroy(node): strings at
//                 +28/+40, optional<string> at +52 (flag +64), vector of candidates at +80 (wasm 1719).
// wasm func 3785: std::__tree<map<K, V>>::destroy(node): V = {std::string (+16), std::vector (+28)}.
// wasm func 2870: std::vector<md::CDadosCandidato>(first, last) (52-byte elements: two strings and an
//                 optional<string>); called by md::CCandidatura::CCandidatura and by wasm 5808.
// wasm func 5808: std::map<K, md::CCandidatura>::insert(first, last) (92-byte nodes; value copy uses
//                 wasm 2870; lookup wasm 3869 = __find_equal).

// =============================================================================================
// uenux2/src/app/comum/dados/ccargos.cpp   (owner u04; srcloc ccargos.cpp:25)
// CCargos (28 bytes, @1838720): +0 int, +4 std::vector<SCargoEleicao> (8-byte {eleicao, cargo}),
// +16 std::vector<...>. Built by the inlined CCargos::CreateInst (wasm 3774 twice + std::sort with
// comparator slot 2357).

// wasm func 5805: CCargos::~CCargos() (frees the vectors at +16 and +4); atexit handler wasm 11561.
CCargos::~CCargos() = default;

// wasm func 2834                                                       // name inferred
// std::map<TCargoID, md::CCargo> with the CCargo of every (eleição, cargo) pair of CCargos, taken
// from CConfiguracaoEleicao::GetCargo (wasm 861). Used by the referential-integrity checks, by
// wasm 2543, by vota::CItemVisualizarCandidatosVota and by the candidate filter menus.
std::map<TCargoID, md::CCargo> CCargos::GetMapaCargos() const
{
    std::map<TCargoID, md::CCargo> mapa;
    for (const auto& par : m_cargos)                                    // 8-byte items, cargo id at +4
        mapa.emplace(CConfiguracaoEleicao::GetInst().GetCargo(par.cargo).GetCodigo(),
                     CConfiguracaoEleicao::GetInst().GetCargo(par.cargo));
    return mapa;
}
// wasm func 1835: std::__tree<map<TCargoID, md::CCargo>>::destroy(node) (optionals at +108/+40).

// =============================================================================================
// uenux2/src/app/comum/dados/crespostas.cpp   (owner u05; srcloc crespostas.cpp:33)
// CRespostas (28 bytes, @1838984): +0 std::map<uedword, SResposta{std::string, std::string}>,
// +16 std::string m_nome = "CRespostas". Key = resposta + cargo * 1000000 (built by CreateInst).
// wasm func 5730: CRespostas::~CRespostas()            ; atexit handler wasm 11481.
// wasm func 1929: std::__tree<map<uedword, SResposta>>::destroy(node) (strings at +24 and +36).
CRespostas::~CRespostas() = default;

// =============================================================================================
// uenux2/src/app/comum/dados/celeitores.cpp   (owner u04; srcloc celeitores.cpp:395/407)
// wasm func 5773: CEleitores::~CEleitores() (tree +112 via wasm 2265, vectors of strings +88/+76,
//                 sets via wasm 1594 ...); atexit handler wasm 11535 (static unique_ptr @1838792).
CEleitores::~CEleitores() = default;
// wasm func 1594: std::__tree<set<T>>::destroy(node) for a trivially destructible T.

// wasm func 3745                                                       // name inferred
// Name of a per-section voter file: "<fase><processo eleitoral><uf><município><zona><seção>-el.dat".
// id.pleito (here and in NomeArquivoTTE) holds the processo eleitoral id (idPE), not the pleito: real
// 2026 -el/-tte names carry 01219, result files the pleito 03220 (2026 urna data: investigation/README.md,
// finding E11).
std::string CEleitores::NomeArquivoEleitores(const SIdentificacaoCarga& id, TMunicipioID municipio,
                                             TZonaID zona, TSecaoID secao)
{
    return CNomeArquivo::MontaNome(id.fase, id.pleito, id.uf, municipio, zona, secao, "el", "dat");  // wasm 3772
}

// wasm func 5727                                                       // name inferred
// Same with the 3-letter suffix at @173207 ("tte"): voters' TTE/biometric file of the section.
std::string CEleitores::NomeArquivoTTE(const SIdentificacaoCarga& id, TMunicipioID municipio,
                                       TZonaID zona, TSecaoID secao)
{
    return CNomeArquivo::MontaNome(id.fase, id.pleito, id.uf, municipio, zona, secao, "tte", "dat");
}

} // namespace comum

// uenux2/src/app/comum/dados/cconfiguracaoeleicao.cpp (+ chv.cpp, md/*)  --  FRAGMENT by unit u02
// (files owned by u04/u05). Out-of-line helpers of comum::CConfiguracaoEleicao::CreateInst and
// comum::CHV::CreateInst, both inlined into wasm func 7787 (dcmp 8100-10990). Names inferred.
//
// Data read by CreateInst (scenario files under /dsk/fi/estatico, names built by CNomeArquivo):
//   ModuloProcessoEleitoral::EntidadeProcessoEleitoral  (wasm 3771)  -> CProcessoEleitoralDTO
//   ModuloParametrizacaoUrna::EntidadeParametrizacaoUrna "...-pu.dat" -> CParametrosUrna
//   ModuloConfiguracaoMunicipios::EntidadeConfiguracaoMunicipios      -> per-municipality config
// Errors: "Urna em 2º turno sem dados de pleito 2: {}" (cconfiguracaoeleicao.cpp:117),
//         "Configuração do município {} não encontrada" (:130),
//         "Tipo de identificador principal inválido {}" (:239), "Instância já criada" (:103).

#include "comum/dados/cconfiguracaoeleicao.h"

namespace comum {

namespace {

// wasm func 5649                                                       // name inferred
// Elections of the pleito that cover `municipio` (all of them when `municipio` is 0). The filter
// looks the município up in each CEleicaoPE's vector<int> at +40/+44 (the ASN.1
// EntidadeEleicao.municipios, wasm 736 = std::find) and also accepts an election whose list
// contains a 0 entry; an election with an empty list is skipped. The int is the município in both
// callers: CEstadoGeral +20 in the CConfiguracaoEleicao::CreateInst call of CriaPleito (7787,
// `q = o[5]`) and CConfiguracaoEleicao +644 in NomesPorAbrangencia (2811). (The earlier name
// "EleicoesDoTurno" was wrong.)
std::vector<md::CEleicaoPE> EleicoesDoMunicipio(const md::CPleitoDTO& pleito, TMunicipioID municipio)
{
    std::vector<md::CEleicaoPE> eleicoes;                                   // 52-byte elements
    if (municipio == 0)
        return {pleito.eleicoes.begin(), pleito.eleicoes.end()};            // wasm 1953 (uninitialized_copy)
    for (const auto& eleicao : pleito.eleicoes)
        if (std::ranges::find(eleicao.municipios, municipio) != eleicao.municipios.end() ||
            std::ranges::find(eleicao.municipios, 0) != eleicao.municipios.end())
            eleicoes.push_back(eleicao);                                    // wasm 5777
    return eleicoes;
}

// wasm func 3713                                                       // name inferred
// md::CPleito from the DTO: elections that cover the município + the situações
// (CPleito::GetSituacoesEleicoes) of those elections. Called three times by the inlined
// CConfiguracaoEleicao::CreateInst with municipio = CEstadoGeral +20.
md::CPleito CriaPleito(const md::CPleitoDTO& dto, TMunicipioID municipio)
{
    auto eleicoes = EleicoesDoMunicipio(dto, municipio);
    std::vector<md::CSituacaoEleicao> situacoes;
    if (municipio == 0) {
        // vector copy of dto+48. The element type is an empty class (sizeof 1, __datasizeof 0):
        // libc++'s __constexpr_memmove copies (n-1)*1 + 0 bytes, which is why the wasm copies n-1.
        situacoes.assign(dto.situacoes.begin(), dto.situacoes.end());
    } else {
        for (const auto& eleicao : dto.eleicoes) {                         // +24/+28, 52-byte items
            // same test as EleicoesDoMunicipio: the município (or a 0 entry) is in the election's
            // vector<int> of municípios (+40/+44)
            if (std::ranges::find(eleicao.municipios, municipio) != eleicao.municipios.end() ||
                std::ranges::find(eleicao.municipios, 0) != eleicao.municipios.end())
                situacoes.append_range(md::CPleito::GetSituacoesEleicoes(dto, eleicao.id));   // wasm 5648 + 760
        }
    }
    return md::CPleito(dto.id, dto.nome, dto.descricao, eleicoes, dto.datas, situacoes);   // wasm 5652
}

// wasm func 5790                                                       // name inferred
// Every CCargo of every election (vector of 140-byte CCargo copied with their optional members).
std::vector<md::CCargo> TodosOsCargos(const CConfiguracaoEleicao& config)
{
    std::vector<md::CCargo> cargos;
    for (const auto& eleicao : config.GetEleicoes())                        // +52 .. +56, 52-byte items
        for (const auto& cargo : eleicao.GetCargos())                       // wasm 2257
            cargos.push_back(cargo);
    return cargos;
}

// wasm func 5654                                                       // name inferred
std::set<TCargoID> CodigosDosCargos(const std::vector<md::CCargo>& cargos)
{
    std::set<TCargoID> codigos;
    for (const auto& cargo : cargos)
        codigos.insert(cargo.GetCodigo());                                  // byte +0 of the 140-byte CCargo
    return codigos;
}

// wasm func 5728                                                       // name inferred
// {CEstadoGeral +48 (fase), CEstadoGeral +4, lower-cased copy (ToLower, wasm 1879) of the UF at
// CEstadoGeral +8} used to build the pleito-level file names (wasm 1705). Unlike the 4-argument
// constructor wasm 5726 (u36), it does not set the CPleito* member at +20.
SIdentificacaoCarga::SIdentificacaoCarga(const estadoaplicacao::CEstadoGeral& eg)
    : modo(eg.m_modo), pleito(eg.m_pleito), carga(eg.m_carga) {}

} // namespace

// wasm func 5792: comum::CConfiguracaoEleicao::~CConfiguracaoEleicao() (non-deleting)
// Frees, in this order: two vectors (+672, +652), the string at +628, set<uebyte> +604 (wasm 1594),
// map +592 (wasm 1079), the CParametrosUrna at +88 (wasm 2267), vector +76, map +64 (wasm 681),
// the vector<CEleicaoPE> +52 (wasm 730 per element) ... Called by the atexit handler wasm 11551
// (static unique_ptr @1838752).
CConfiguracaoEleicao::~CConfiguracaoEleicao() = default;

// ---------------------------------------------------------------------------------------------
// comum::CHV (horário de verão, chv.cpp) and md::CHorarioVeraoMunicipio

// wasm func 3754: trivially copies a 28-byte md::CHorarioVerao (7 words).          // name inferred
// wasm func 5677                                                       // name inferred
md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio(TMunicipioID municipio, const CHorarioVerao& hv)
    : m_municipio(municipio), m_horarioVerao(hv)          // optional engaged (+24 = 1)
{
    ValidaCriacao();
}

// wasm func 5679                                                       // name inferred
md::CHorarioVeraoMunicipio::CHorarioVeraoMunicipio(TMunicipioID municipio)
    : m_municipio(municipio), m_horarioVerao(std::nullopt)
{
    ValidaCriacao();
}

// ---------------------------------------------------------------------------------------------
// Library instantiations used by CreateInst (not reconstructed):
//   wasm 3776  std::vector<std::unique_ptr<T>>::push_back(unique_ptr&&)  (slow path inlined)
//   wasm 5671  std::vector<std::unique_ptr<T>>::~vector()  (virtual deleting dtor of each element)
//   wasm 3778  std::set<uebyte>::insert(first, last)      (node key byte at +13)
//   wasm 5787  std::insert_iterator<std::set<uebyte>>::operator=  (set::insert(hint, v))
//   wasm 5788  std::__lower_bound_onesided on set<uebyte>::iterator  (the two form the inlined
//              std::set_intersection of two office-code sets)

} // namespace comum

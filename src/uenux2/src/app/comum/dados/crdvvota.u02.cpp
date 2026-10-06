// uenux2/src/app/comum/dados/crdvvota.cpp  --  FRAGMENT written by unit u02 (owner u05)
//
// comum::CRdvVota : comum::CRdv (vtable @1560172, 100 bytes, singleton @1838956). The constructor
// and CreateInst() are inlined into wasm func 7787 (dcmp 15644-16951); see crdv.cpp (also in this
// unit) for CRdv::CRdv and the key derivation. Out-of-line pieces:

#include "comum/dados/crdvvota.h"

#include <algorithm>

namespace comum {

// inlined (dcmp 15652-16951), srcloc crdvvota.cpp:92 "Instancia ja criada" (EUeRdvError 4658)
void CRdvVota::CreateInst()
{
    if (ms_instancia)
        throw CRdvError(api::EUeRdvError{4658}, "Instancia ja criada");
    ms_instancia.reset(new CRdvVota());
}

// inlined (dcmp 15655-16935)
CRdvVota::CRdvVota()
    : CRdv(std::make_unique<CRdvPosicionadorVota>(),                    // vtable @1560252, 4 bytes
           CodigosCargosOrdenados(),
           CConfiguracaoEleicao::GetInst().GetQtdDigitosPartido())       // byte +164
{
    auto eleicoesCargos = CriaMapaEleicoesCargos();                      // wasm 5735
    m_votosCargos = md::MontaMapa(eleicoesCargos);                       // cvotoscargos.cpp:30/37 ("Vetor de cargos vazio", "Cargo duplicado ")
    // CConversorRegistroDigitalVoto<CConversorEleicoesVota> at +44, filled with: the turn and
    // CConfiguracaoEleicao +28, CEstadoGeral +48, CEstadoGeralGap +?, município (wasm 1004),
    // zona (GetZonaID), local (wasm 1933), seção (GetSecaoID) and a copy of the list of files.
    m_conversor = asn::CConversorRegistroDigitalVoto<asn::CConversorEleicoesVota>(/* ... */);
}

// inlined helper (dcmp 15657-15698): sorted office codes (first byte of every CCargo of every election)
std::vector<uebyte> CRdvVota::CodigosCargosOrdenados()
{
    std::vector<uebyte> codigos;
    for (const auto& cargo : TodosOsCargos(CConfiguracaoEleicao::GetInst()))   // wasm 5790 (140-byte CCargo)
        codigos.push_back(cargo.GetCodigo());
    std::ranges::sort(codigos);                     // std::sort<uebyte*>: wasm 4820 (introsort), 4817
                                                    // (insertion sort), 4819/4818/1507/2579 (sort3/4/5),
                                                    // 4811 (heap sift-down), 4812 (bitset partition swap)
    return codigos;
}

// wasm func 5735                                                       // name inferred
// std::map<TEleicaoID, std::vector<md::CCargo>> with the offices of every election:
//   ids = wasm 5791 (vector<int>: first int of every 52-byte CEleicaoPE of CConfiguracaoEleicao +52)
//   cargos of each id = wasm 3775 (CConfiguracaoEleicao::GetCargos(id, 0))
std::map<TEleicaoID, std::vector<md::CCargo>> CRdvVota::CriaMapaEleicoesCargos()
{
    std::map<TEleicaoID, std::vector<md::CCargo>> mapa;
    for (const TEleicaoID id : IdsEleicoes(CConfiguracaoEleicao::GetInst()))      // wasm 5791
        mapa.emplace(id, CConfiguracaoEleicao::GetInst().GetCargos(id, 0));        // wasm 3775
    return mapa;
}

// wasm func 5791                                                       // name inferred
std::vector<TEleicaoID> IdsEleicoes(const CConfiguracaoEleicao& config)
{
    std::vector<TEleicaoID> ids;
    for (const auto& eleicao : config.GetEleicoes())                       // +52/+56, 52-byte items
        ids.push_back(eleicao.GetId());
    return ids;
}

// wasm func 5734: CRdvVota::~CRdvVota() (non-deleting): resets the vtable of the converter at +44
// (vtable @1560304), frees its vector<std::string> (+88), the maps +52 (wasm 1160), +32 (wasm 1550),
// +20 (wasm 1159), then the CRdv base (vtable @1559900: shared_ptr cipher +8/+12, posicionador +4).
// wasm func 11495: atexit handler of the static unique_ptr<CRdvVota> @1838956.
CRdvVota::~CRdvVota() = default;

// wasm funcs 11485 / 11484: asn::CConversorRegistroDigitalVoto<asn::CConversorEleicoesVota>
// destructor (vtable slot 0) and deleting destructor (slot 1, + free(this)): stores the vtable
// @1560304, frees the std::vector<std::string> at +44 (the list of files), then destroys the
// std::map<TEleicaoID, std::vector<md::CCargo>> at +8 (tree root at +12, wasm 1160). No base-class
// vtable (@1571224) is stored and nothing else is released (the same sequence appears inlined in
// ~CRdvVota at converter offset +44: vector +88, map +52).

// Library instantiations (std::__tree<...>::destroy, recursive left/right then node):
//   wasm 1159: map<K, map<K2, std::vector<X16>>> (inner destroy = wasm 1268)
//   wasm 1160: map<TEleicaoID, std::vector<md::CCargo>> (CCargo optionals at +84/+52 via wasm 242/267)
//   wasm 1268: map<K, std::vector<X16>> with X16 = {..., std::string (last 12 bytes)}

} // namespace comum

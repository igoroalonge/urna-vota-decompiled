// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/cpe.cpp
//
// See ccargos.cpp for the error types.
#include "cpe.h"

#include <utility>

#include "asn/processoeleitoral/cconversorprocessoeleitoral.h"
#include "../nomearquivo/cnomearquivo.h"

namespace comum {

using CErroDados   = ecourna::api::exception::CBaseError<EUeComumDadosError>;
using CErroPattern = ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternErr>;

std::mutex CPE::s_mutex;
CPE*       CPE::s_inst = nullptr;

// wasm func 1073 (srcloc line 21)
CPE& CPE::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (s_inst == nullptr)
        throw CErroPattern(ecourna::api::pattern::EPatternErr{1303}, "CPE - instancia nao criada");
    return *s_inst;
}

// wasm func 2787 (srcloc line 26). The `origem` parameter was removed by the optimiser (unused or
// constant). Callers test comum_f2788 ("instance exists", not in u04) first.
void CPE::CreateInst(const md::estadoaplicacao::CEstadoGeral& eg, const EFlashOrigem /*origem*/)
{
    if (s_inst != nullptr)
        throw CErroDados(EUeComumDadosError{7876}, "Instância já criada");

    // api::CFileASN::ReadFromFile@3771 + CConversorProcessoEleitoral: reads
    //   CPath::Estatico() / (FormataFase(eg.fase) + FormataNumero(eg.processo, 5) + "-cp.dat")
    // e.g. "/dsk/fi/estatico/t02400-cp.dat", and converts it to md::CProcessoEleitoral.
    const md::CProcessoEleitoral pe = asn::LeProcessoEleitoral(eg);           // name inferred

    // The binary COPIES pe into the new object (string copy ctors, CPleito copy comum_f1281, vector
    // memcpy) and destroys the local afterwards (comum_f2830): no move.
    auto* novo = new CPE(pe, eg);
    delete std::exchange(s_inst, novo);          // (the old value is necessarily null here)
}

// Constructor - only inlined into CreateInst.
CPE::CPE(const md::CProcessoEleitoral& pe, const md::estadoaplicacao::CEstadoGeral& eg)
    : m_pe(pe)
    , m_turno(eg.GetDadoCarga().GetTurnoRaw())                                // eg +32   // ?
    , m_identificacao{eg.GetFase(),                                           // eg +48
                      m_pe.GetId(),                                           // pe +0
                      eg.GetTexto8(),                                         // eg +8     // ?
                      // round 1 -> pleito1 (+36); otherwise CProcessoEleitoral::GetPleito2()
                      m_turno == '1' ? &m_pe.GetPleito1() : &m_pe.GetPleito2()}
{
}

// wasm func 5598 - destructor (frees the string at +188, then ~CProcessoEleitoral = wasm 2830).
CPE::~CPE() = default;

// wasm func 11226 (table slot 2984; no code references that slot, so the at-exit role is an
// inference): takes an unused void* argument and releases the singleton.
static void DestroiInstanciaCPE(void*)                                        // name inferred
{
    delete std::exchange(CPE::s_inst, nullptr);   // (friend/static access)                // ?
}

// wasm func 2830 = md::CProcessoEleitoral::~CProcessoEleitoral() (implicitly defined; frees the
// vectors of 52-byte elements (comum_f730 per element), the optional pleito2, the map at +72 and the
// strings at +4/+16/+40). Emitted here because CPE is its only non-inlined owner.

}  // namespace comum

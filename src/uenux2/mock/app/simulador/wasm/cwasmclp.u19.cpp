// FRAGMENT reconstructed by unit u19 from vota_web_wasm.wasm.
// Original file: uenux2/mock/app/simulador/wasm/cwasmclp.cpp (srcloc :10, "api::CLp::CLp()").
//
// api::CLp is the report printer front end ("lp" = line printer) used by the BU/zerésima/list printing code
// (vota::CRelVotaUtil::CortaPapel 2882, comum_f3671, vota::CImpressaoListaEleitores::vf2 11908). In the web
// build its constructor only looks up api::IImpressoraRelatorios, which is simulador::CWasmNullPrinter
// (no printer), registered by the simulator start-up (func 8302).
#include "api/print/clp.h"          // path inferred

#include <memory>
#include <mutex>
#include <source_location>

#include "api/pattern/cpolysingleton.h"

namespace api {

// Layout (12 bytes):
//   +0 IImpressoraRelatorios* m_impressora            (both +0 and +4 receive the same pointer)
//   +4 IImpressoraRelatorios* m_impressoraRelatorios  (used by the print call, func 3875: slot 9)
//   +8 std::unique_ptr<SArquivoSpool> m_spool         (object with a std::vector at +8, an
//                                                       ecourna::api::io::CFile at +20 and a std::string at +24)  (?)

// Inlined into CLp::GetInst (func 3876).
CLp::CLp()
{
    IImpressoraRelatorios* impressora = nullptr;
    if (CPolySingleton<IImpressoraRelatorios>::exists())                       // func 3392
        impressora = &CPolySingleton<IImpressoraRelatorios>::instance(GetPolySingletonsInfo(),
                                                                     std::source_location::current());   // :10
    m_impressora = impressora;
    m_impressoraRelatorios = impressora;
}

// wasm func 5978                                                                              // name inferred
CLp::~CLp()
{
    // m_spool.reset(): closes its CFile if open (ecourna::api::io::CFile::Close), frees string and vector.
}

namespace {
std::mutex           s_mutexLp;     // @1834020 (unlock residue only)
std::unique_ptr<CLp> s_lp;          // @1834044
}  // namespace

// wasm func 3876 (tools name "api::CPolySingletonList::instance@3876")                      // name inferred
CLp& CLp::GetInst()
{
    std::lock_guard trava(s_mutexLp);
    if (!s_lp)
        s_lp.reset(new CLp());
    return *s_lp;
}

// wasm func 12016 (table slot 1565): exit-time destructor of s_lp (-> 5978 + free). Never runs in the browser.

}  // namespace api

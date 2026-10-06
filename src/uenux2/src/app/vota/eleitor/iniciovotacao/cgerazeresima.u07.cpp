// FRAGMENT reconstructed by unit u07 from vota_web_wasm.wasm.
// Original files (paths inferred; the classes may be split differently):
//   uenux2/src/app/vota/eleitor/iniciovotacao/cgerazeresima.cpp        CGeraZeresimaBase, CGeraZeresima
//   uenux2/src/app/vota/eleitor/iniciovotacao/cregerarzeresima.cpp     CRegerarZeresima
//   uenux2/src/app/vota/eleitor/iniciovotacao/cgeraresumozeresima.cpp  CGeraResumoZeresimaBase, CGeraResumoZeresima,
//                                                                      CRegeraResumoZeresima
// Other methods of these classes are in units u09, u20, u26, u34, u39.
//
// Zerésima = the report printed when the urna is opened on election day, proving that every candidate
// starts with zero votes (the "resumo" is a shorter summary / "extrato do RDV" printed after it).
// "Regerar" = generate again (after CConfirmaRegerarZeresima, e.g. when the media was prepared by
// another urna: CReinicioVotacao::StartState).
//
// RTTI (all : comum::CAppState, 20 bytes = CAppState(12) + shared_ptr m_tela at +12/+16):
//   CGeraZeresimaBase        vtable @1544392   [0] ~ (1720) [2] StartState (11946: builds and prints the report)
//                                              [9] pure  [10] pure
//     CGeraZeresima          vtable @1544584   [9] nop            [10] GetEstadoResumo (11935)
//     CRegerarZeresima       vtable @1546500   [9] 11842 CortaPapel  [10] GetEstadoResumo (11841)
//   CGeraResumoZeresimaBase  vtable @1544456   [0] ~ (1952) [2] StartState (11943)  [9] pure  [10] pure
//     CGeraResumoZeresima    vtable @1544520   [9] 11940  [10] 11939
//     CRegeraResumoZeresima  vtable @1546436   [9] nop    [10] 11845
#include "vota/eleitor/iniciovotacao/cgerazeresima.h"

#include <mutex>

#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// wasm func 5965 (called by CGeraZeresima::GetInst, func 5962, and by CConfirmaRegerarZeresima::ProcessInput,
// func 11838, which builds CRegerarZeresima)                                         // name inferred
CGeraZeresimaBase::CGeraZeresimaBase()
    : comum::CAppState(0),                                                       // no input: runs and leaves
      m_tela(CTelasVota::GetInst().m_telaGeraZeresima)                           // CTelasVota +212
{
}

// wasm func 11945 (called only through the merged GetInst body 6056)                 // name inferred
CGeraResumoZeresimaBase::CGeraResumoZeresimaBase()
    : comum::CAppState(0),
      m_tela(CTelasVota::GetInst().m_telaGeraResumoZeresima)                     // CTelasVota +220
{
}

// wasm func 1952 (vtable slot 0). Body is the shared helper func 2902(this, vtable): store the class
// vtable, release m_tela. Also reached from the ICF thunks 5963/5964 of the derived classes.
CGeraResumoZeresimaBase::~CGeraResumoZeresimaBase() = default;

// Singletons of the two concrete "resumo" states.
static std::unique_ptr<CGeraResumoZeresimaBase> s_pGeraResumo;      // @1834184 (mutex @1834160)
static std::unique_ptr<CGeraResumoZeresimaBase> s_pRegeraResumo;    // @1834884 (mutex @1834860)
static std::mutex s_mutexGeraResumo, s_mutexRegeraResumo;
// wasm func 11942 / 11847: their atexit destructors (s_pGeraResumo.reset() / s_pRegeraResumo.reset()).

// wasm func 6056: merged body (wasm-opt merge-similar-functions) of CGeraResumoZeresima::GetInst() and
// CRegeraResumoZeresima::GetInst() as inlined into the two GetEstadoResumo() below; the mutex, the
// static pointer and the vtable of the concrete class are extra parameters, the caller's `this` is unused.
template <typename T>
static comum::CAppState* ObtemInstancia(std::mutex& mutex, std::unique_ptr<CGeraResumoZeresimaBase>& instancia)
{
    std::lock_guard<std::mutex> lock(mutex);
    if (!instancia)
        instancia.reset(new T());          // CGeraResumoZeresimaBase() (func 11945) + vtable of T
    return instancia.get();
}

// wasm func 11935 (CGeraZeresima vtable slot 10)                                    // name inferred
comum::CAppState* CGeraZeresima::GetEstadoResumo()
{
    return ObtemInstancia<CGeraResumoZeresima>(s_mutexGeraResumo, s_pGeraResumo);           // = CGeraResumoZeresima::GetInst()
}

// wasm func 11841 (CRegerarZeresima vtable slot 10)                                 // name inferred
comum::CAppState* CRegerarZeresima::GetEstadoResumo()
{
    return ObtemInstancia<CRegeraResumoZeresima>(s_mutexRegeraResumo, s_pRegeraResumo);     // = CRegeraResumoZeresima::GetInst()
}

} // namespace vota

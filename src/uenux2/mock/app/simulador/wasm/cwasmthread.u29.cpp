// FRAGMENT of uenux2/mock/app/simulador/wasm/cwasmthread.cpp (attested by srcloc cwasmthread.cpp:31,
// virtual void simulador::CWasmThread::Create(void *(*)(void *), void *), func 9655, unit u31)
// reconstructed by unit u29 from vota_web_wasm.wasm. Only the static below belongs to this unit.
//
// The build has no pthreads, so CWasmThread does not start anything: Create() only records the thread's entry
// point in a process-wide list (16-byte record {rotina, argumento, 0, id}; ids from the counter @1832660), and
// the "threads" are stepped cooperatively (votaTick -> CExecucaoVotaCooperativa, docs/modules/u18 §2.6).

#include <vector>

namespace simulador {
namespace {

struct SThreadWasm;                                   // 16-byte record, see CWasmThread::Create (u31)

// @1832648 (begin) / @1832652 (end) / @1832656 (capacity). Its destructor is wasm func 9662 (table slot 494,
// registered with atexit): free the buffer; the records themselves are never deleted (they leak at exit,
// which is harmless).
std::vector<SThreadWasm*> s_threads;                                                          // name inferred

}  // namespace
}  // namespace simulador

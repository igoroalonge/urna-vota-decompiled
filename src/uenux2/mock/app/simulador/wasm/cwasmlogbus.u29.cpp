// FRAGMENT reconstructed by unit u29 from vota_web_wasm.wasm.
// Original file: UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmlogbus.cpp (+ .h). No RTTI, no
// srcloc and no string names this object; everything below except the behaviour is inferred ("name inferred").
//
// A small in-memory "log bus" of the web simulator. Two mocks publish to it:
//   channel 0  simulador::CWasmInit   every command of the urna's init service, "CWasmInit::{} {}" (func 1524)
//   channel 2  simulador::CWasmLogd   every line of the application log (logd.dat), after writing the file
//                                     (CWasmLogd vf4, func 8327, unit u31)
//   channel 1  no publisher in this build
// Each line is time-stamped with the local time, kept in a per-channel history of at most 2000 lines, and
// handed to the channel's subscribers. NOTHING in the binary subscribes: the only code that touches the
// subscriber maps is the publisher (reads) and the static destructor (api_f5154), so the "subscribe" member was
// dropped by the linker. In this build the bus is a bounded ring buffer that nobody reads.
//
// Static object (168 bytes @1832676, guard byte @1832844, zero-initialised inline by its accessor in 1524 and
// 8327, destroyed at exit by func 5154):
//   +0    std::mutex (24 bytes; lock/unlock are no-ops without pthreads, only the unlock residue remains)
//   +24   SCanal[3], 48 bytes each:
//           +0  std::deque<std::string> historico       (341 strings per 4092-byte block)
//           +24 std::unordered_map<int, std::function<void(const std::string&)>> ouvintes  (max_load_factor 1.0)
//           +44 int proximoId = 1

#include <ctime>
#include <deque>
#include <functional>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace simulador {

class CWasmLogBus                                                                        // name inferred
{
public:
    enum ECanal : int { INIT = 0, CANAL1 = 1, LOGD = 2 };                                // names inferred

    static CWasmLogBus& GetInst()                    // inline; inlined into funcs 1524 and 8327
    {
        static CWasmLogBus s_bus;                    // @1832676, guard @1832844
        return s_bus;
    }

    void Publica(ECanal canal, const std::string& texto);                                // wasm func 5152

private:
    static constexpr std::size_t MAX_LINHAS = 2000;

    struct SCanal {
        std::deque<std::string> historico;
        std::unordered_map<int, std::function<void(const std::string&)>> ouvintes;
        int proximoId = 1;                           // for the (absent) subscribe method
    };

    std::mutex m_mutex;                              // +0
    SCanal m_canais[3];                              // +24
};

// ------------------------------------------------------------------------------------------------
// wasm func 5152 (3,711 bytes; `this` was constant-propagated away, the object is addressed as @1832676).
// Callers: CWasmInit's logger (1524, channel 0) and CWasmLogd vf4 (8327, channel 2).
// (wasm funcs 3501 and 5150 are libc++'s __split_buffer<std::string*>::push_back / push_front, used when the
//  deque's block map grows.)
// ------------------------------------------------------------------------------------------------
void CWasmLogBus::Publica(ECanal canal, const std::string& texto)
{
    const std::time_t agora = std::time(nullptr);
    std::tm local{};
    ::localtime_r(&agora, &local);

    std::ostringstream carimbo;
    carimbo << '[' << std::put_time(&local, "%Y-%m-%d %H:%M:%S") << ']';
    const std::string linha = carimbo.str() + " " + texto;

    std::vector<std::function<void(const std::string&)>> ouvintes;
    std::string ultima;
    {
        std::lock_guard lock(m_mutex);
        SCanal& c = m_canais[canal];
        c.historico.push_back(linha);
        while (c.historico.size() > MAX_LINHAS)
            c.historico.pop_front();
        ultima = c.historico.back();

        ouvintes.reserve(c.ouvintes.size());
        for (const auto& [id, ouvinte] : c.ouvintes)
            ouvintes.push_back(ouvinte);
    }
    // Called outside the lock. Always empty in this build (see header comment).
    for (const auto& ouvinte : ouvintes)
        ouvinte(ultima);                             // std::bad_function_call if an entry were empty
}

}  // namespace simulador

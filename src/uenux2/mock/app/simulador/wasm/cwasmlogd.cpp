// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmlogd.cpp
//
// simulador::CWasmLogd : api::CEscritorLog - the web replacement of the urna's log daemon (logd).
// RTTI typeinfo @1530828; vtable @1530808: [0] ~ (8318) [1] deleting (8316) [2] CEscritorLog::fazOperacao
// (10262) [3] CEscritorLog::loga (10260, the tools called it "CWasmLogd::vf3") [4] Escreve (8327).
// Built by func 8302 with <log dir>/logd.dat = /dsk/fi/dinamico/log/logd.dat and registered as
// api::CEscritorLog (the last platform push, sz[27]).
//
// On the urna, CEscritorLog sends each record to the logd process, which keeps the signed log of the
// machine (the "log da urna" published after the election). Here each record is one Latin-1 text line
//     <aplicacao>|<severidade>|<mensagem>          e.g. "1|1|Voto confirmado para [Vereador]"
// appended to the MEMFS file, and mirrored into the simulator's in-memory log bus (func 5152, unit u29).
// The file is truncated at every start of the program (page load). Nothing is signed or hashed.
#include <cstdio>
#include <filesystem>
#include <format>
#include <mutex>
#include <string>

#include "api/uelog/cescritorlog.h"
#include "simulador/wasm/cwasmlogbus.h"   // CWasmLogBus (func 5152, unit u29: cwasmlogbus.u29.cpp)

namespace simulador {

class CWasmLogd : public api::CEscritorLog {
public:
    explicit CWasmLogd(const std::filesystem::path& arquivo);   // inlined into func 8302
    ~CWasmLogd() override;                                       // 8318 / 8316

protected:
    void Escreve(uebyte aplicativo, api::ESeveridade severidade, const std::string& mensagem) override;   // 8327

private:
    std::mutex m_mutex;          // +4  (24 bytes; lock/unlock are no-ops in this thread-less build)
    std::string m_arquivo;       // +28 (object = 40 bytes)
};

// Inlined into func 8302: create the directory and truncate the file ("wb").
CWasmLogd::CWasmLogd(const std::filesystem::path& arquivo)
    : m_arquivo(arquivo.string())
{
    std::filesystem::create_directories(std::filesystem::path(m_arquivo).parent_path());   // api_f1840 = parent_path
    if (std::FILE* f = std::fopen(m_arquivo.c_str(), "wb"))
        std::fclose(f);
}

// wasm func 8318 - slot 0; wasm func 8316 - slot 1 (same + free)
CWasmLogd::~CWasmLogd() = default;

// wasm func 8327 - slot 4. Observed executing (every CLoga::loga of the vote).
void CWasmLogd::Escreve(uebyte aplicativo, api::ESeveridade severidade, const std::string& mensagem)
{
    const std::string linha = std::format("{}|{}|{}", aplicativo, static_cast<int>(severidade), mensagem);
    {
        std::lock_guard<std::mutex> trava(m_mutex);
        if (std::FILE* f = std::fopen(m_arquivo.c_str(), "ab")) {   // failures are silently ignored
            std::fwrite(linha.data(), 1, linha.size(), f);
            std::fputc('\n', f);
            std::fflush(f);
            std::fclose(f);
        }
    }
    // func 5152 (u29): time-stamped copy into channel 2 of the simulator's in-memory "log bus" (at most 2000 lines;
    // no subscriber exists in this build). The bus singleton (@1832676, guard @1832844) is inlined here.
    CWasmLogBus::GetInst().Publica(CWasmLogBus::LOGD, linha);
}

}  // namespace simulador

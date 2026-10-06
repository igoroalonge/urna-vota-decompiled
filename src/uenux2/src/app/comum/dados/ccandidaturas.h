// uenux2/src/app/comum/dados/ccandidaturas.h
// Reconstructed from vota_web_wasm.wasm (unit u03) — only the members needed by this unit's functions.
// Other methods of the class (Localiza, LoadFromFile, AdicionaCandidato — srclocs ccandidaturas.cpp:56/201/208/245)
// are compiled into other functions (11213, 7787) outside this unit.
#pragma once

#include <map>
#include <memory>
#include <string>

#include "api/io/cdatamap.h"
#include "comum/dados/md/candidatura/ccandidatura.h"

namespace comum {

// No RTTI (not polymorphic). 44 bytes, from the constructor (func 5811) and destructor (func 3786):
//   api::CDataMap<md::CCandidatura> base:
//     +0  std::map<...> container (begin node +0, root +4, size +8)
//     +12 iterator m_atual (== end() when not positioned)
//     +16 std::string m_nome = "CCandidaturas"
//     +28 bool (true)
//   +32 std::map<uedword, std::string> m_versoesPacote   (TCargoID -> package version)
// Singleton: static std::unique_ptr at 0x1C0E64 (@1838692), mutex @1838668 (the lock vanished in the
// single-threaded build; only the no-op unlock residue remains). Accessor = func 521 (wasm-entry component).
class CCandidaturas : public api::CDataMap<md::CCandidatura>
{
public:
    CCandidaturas() : api::CDataMap<md::CCandidatura>("CCandidaturas") {}   // wasm func 5811
    ~CCandidaturas() = default;                                              // wasm func 3786

    static CCandidaturas& GetInst();   // func 521: creates the instance on first use (unique_ptr reset)

    const std::string RecuperaVersaoPacote(TCargoID cargo) const;   // wasm func 5810 (srcloc line 89)

private:
    std::map<uedword, std::string> m_versoesPacote;   // +32
};

const md::CCandidatura& GetCandidaturaAtual(const std::string& contexto);   // wasm func 2838 (srcloc line 261)

} // namespace comum

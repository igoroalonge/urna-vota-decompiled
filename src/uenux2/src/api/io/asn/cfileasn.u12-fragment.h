// uenux2/src/api/io/asn/cfileasn.h  -- FRAGMENT reconstructed by unit u12
// (the header belongs to the uenux2 api/io/asn unit; these functions were attributed to ecourna by the analyzer)
//
// Reconstructed from vota_web_wasm.wasm.
//
// api::CFileASN::WriteToFile<T>(fileName, objeto): opens `fileName` with mode "wb" and writes the BER
// encoding of `objeto`. wasm-opt merged the instantiations in two steps (merge-similar-functions):
//
//   thunk (table slot)      T                                   -> writer callback (slot) -> CodeObjectFunction<T>
//   func 9985  (460)        ModuloEstadoGeralVota::EstadoGeralVota  -> func 9684 (463)      -> func 9673   vota.bin
//   func 10008 (457)        ModuloEstadoGeralSA::EstadoGeralSA      -> func 9701 (462)      -> func 9695   sa.bin
//   func 10040 (455)        ModuloEstadoGeralGap::EstadoGeralGap    -> func 9703 (461)      -> func 9702   gap.bin
//   func 10168 (429)        ModuloEstadoGeralUrna::EstadoGeralUrna  -> func 9820 (446)      -> func 9801   eg.bin
//
//   func 2892  = the shared body of WriteToFile(const std::string&, const T&)   (unit u12)
//   func 2891  = the shared body of WriteToFile(const CFile&, const T&)        (not in u12)
//
// The four thunks were seen running during every recorded session. They are called by func 2894
// (uenux2/mock/app/comum/cappinfobuilder.cpp), which writes the fixture state files of the web build
// (eg.bin, vota.bin, gap.bin, sa.bin) under /dsk/{fi,fe}/dinamico/trab{1,2}/ during votaInit.
#pragma once

#include <string>
#include <vector>

#include "ecourna/api/io/cfile.hpp"

namespace api {

class CFileASN {
public:
    // func 2891 (merged body; the per-type CodeObjectFunction is passed as a function pointer)
    template <typename T>
    static void WriteToFile(const ecourna::api::io::CFile& arquivo, const T& objeto)
    {
        std::vector<char> buffer;
        CodeObjectFunction(buffer, objeto, "WriteToFile de " + arquivo.GetName());   // cfileasn.h:161/171
        arquivo.RawWrite(buffer.data(), static_cast<uedword>(buffer.size()));        // func 1886
    }

    // wasm func 2892 (merged body) + thunks 9985 / 10008 / 10040 / 10168 (one per T, see above)
    template <typename T>
    static void WriteToFile(const std::string& fileName, const T& objeto)
    {
        ecourna::api::io::CFile arquivo(fileName, "wb");                             // func 517, FM_NORMAL
        WriteToFile(arquivo, objeto);                                                // callback slot (func 9684 ...)
    }                                                                                // ~CFile (func 3564): Close
                                                                                     // -> Sync (fflush+fsync) -> fclose.
    // No explicit Close(): the file is closed by ~CFile, which catches and DISCARDS every exception. A failing
    // fsync() (1183) or fclose() (1179) of eg.bin / vota.bin / gap.bin / sa.bin is therefore never reported.
    // (RawWrite already fflush()es after each write, so only the durability step can fail silently.)

    template <typename T>
    static void CodeObjectFunction(std::vector<char>& buffer, const T& objeto, const std::string& contexto);   // (other unit)
};

} // namespace api

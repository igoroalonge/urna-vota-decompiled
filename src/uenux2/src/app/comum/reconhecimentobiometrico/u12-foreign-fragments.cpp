// uenux2/src/app/comum/...  -- FRAGMENT reconstructed by unit u12 (owning file and class NOT known)
//
// wasm func 5371 ("comum_f5371") inlines ecourna::api::io::CFile (Close) and CFile::RawWrite(vector)
// (func 5180), which is why it is in unit u12. It stores a poll worker's (mesário's) fingerprint image
// as "me{:06}.wsq" in the internal WSQ "operador/" directory and copies it to the external one.
// Callers: comum::CPedeDigitalMesario::GetControlador (5382) and vota::CRegistraDigitalOperador::ProcessTick
// (10451). The directory pair comes from vota_f3795 / comum_f3794 ("<wsq>/operador/").
// The name, the class and the file are guesses (low confidence). It is close to
// CControlaArmazenamentoDeImagens::GerarCaminhosUnicos (ccontrolaarmazenamentodeimagens.cpp:73), which
// returns std::tuple<uint32_t, std::string, std::string>.
#include <cstdint>
#include <filesystem>
#include <format>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <sys/statfs.h>

#include "api/util/csystem.h"
#include "ecourna/api/io/cfile.hpp"

namespace comum {

struct CRegistroImagemMesario {                       // name inferred; only the fields used here
    // ... (+0..+11 unknown)
    std::optional<std::uint32_t> m_idImagem;          // +12 value, +16 engaged flag
};

// wasm func 5371 (name inferred)
void SalvaWsqMesario(CRegistroImagemMesario& registro, const std::vector<uebyte>& wsq,
                     const std::string& dirInterno, const std::string& dirExterno,
                     const std::function<std::uint32_t()>& geraId)
{
    // Needs at least 5 MiB free on the dynamic area. If statfs() itself fails, the image is saved anyway.
    struct statfs info;
    if (statfs(CPath::GetPathDinamico(true).c_str(), &info) == 0 &&           // wasm_entry_f1082(…, 1): "dinamico/"
        static_cast<double>(info.f_bavail) * static_cast<double>(info.f_bsize) < 5242880.0) {
        return;                                                                // silently not saved
    }

    std::uint32_t id;
    std::string interno, externo;
    do {                                                                       // first free id
        id = geraId();                                                         // throws bad_function_call if empty
        const std::string nome = std::format("me{:06}.wsq", id);
        interno = (std::filesystem::path(dirInterno) / nome).string();
        externo = (std::filesystem::path(dirExterno) / nome).string();
    } while (api::CSystem::IsRegularFile(interno));                           // func 412

    registro.m_idImagem = id;

    ecourna::api::io::CFile file(interno, "w+b", ecourna::api::io::CFile::FM_NOATIME);   // FileMode 2
    file.RawWrite(wsq);                                                        // func 5180 (no-op if empty)
    file.Close();                                                              // fflush + fsync + fclose
    api::CSystem::CopyFile(interno, externo, false);                          // func 378 (mirror copy). Its srcloc
                                                                               // signature has a 4th bool; wasm sig
                                                                               // (i32,i32,i32): dead-argument
                                                                               // elimination removed it (same value
                                                                               // at every call site)
}

} // namespace comum

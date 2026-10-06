// Reconstructed from vota_web_wasm.wasm (unit u31).
// Original file UNKNOWN - path inferred: uenux2/mock/app/simulador/wasm/cwasmresource.cpp
//
// simulador::CWasmResource : api::IResource - the embedded resources of the voting application (images,
// GIF animations, sounds), named Qt-style ":/resource/...". On the urna they are compiled into the program;
// in the web build they are files of the main data package (vota_web_wasm.data, mounted under /pkg and
// /uenux/app) found by the resolver func 2626 (unit u29): strip the leading ':', map "/resource/images/..."
// to "/uenux/app/img/..." and "/pkg/img/...", "/resource/gifs/..." to "/pkg/gifs/...", try each candidate
// with an ifstream, log "resolve" or "missing" through js_resource_log. The file is read by func 3452.
//
// RTTI typeinfo @1530212; vtable @1530184: [0] ~ (ICF 174) [1] deleting (ICF 144) [2] GetImage (8633)
// [3] GetFile (8611) [4] GetMovie (8605) [5] Exists (8600) [6] IsResource (8591). 4-byte object, no state.
// js_resource_log(acao, recurso, caminho, tamanho) prints only when Module.uenuxDebug is set.
#include <memory>
#include <string>
#include <vector>

#include "api/gui/cfixedimage.h"
#include "api/gui/cmovie.h"
#include "api/gui/iresource.h"
#include "simulador/wasm/cwasmimagesurfaceops.h"   // TamanhoImagem (the GetImageSize parser)
#include "simulador/wasm/cwasmjs.h"                // js_resource_log
#include "simulador/wasm/cwasmutil.h"              // ResolveCaminho (2626), LeArquivo (3452): unit u29

namespace simulador {

class CWasmResource : public api::IResource {
public:
    std::shared_ptr<api::IImage> GetImage(const std::string& nome, int, int) override;   // slot 2 (8633)
    api::SharedVector GetFile(const std::string& nome) override;                         // slot 3 (8611)
    api::CMovie GetMovie(const std::string& nome) override;                              // slot 4 (8605)
    bool Exists(const std::string& nome) override;                                       // slot 5 (8600) name inferred
    bool IsResource(const std::string& nome) override;                                   // slot 6 (8591) iresource.h:97
};

// wasm func 8633 - slot 2 (the two int parameters are ignored ?). A missing resource gives an image with no
// bytes, which the screen silently skips.
std::shared_ptr<api::IImage> CWasmResource::GetImage(const std::string& nome, int, int)
{
    const std::string caminho = ResolveCaminho(nome);
    std::vector<uebyte> dados = LeArquivo(caminho);
    js_resource_log("image", nome.c_str(), caminho.c_str(), static_cast<int>(dados.size()));
    return std::make_shared<api::CFixedImage>(std::move(dados));                // CFixedImage{vptr, vector}
}

// wasm func 8611 - slot 3 (getResourceFile, iresource.h:82). Observed executing (images of the vote screens).
api::SharedVector CWasmResource::GetFile(const std::string& nome)
{
    const std::string caminho = ResolveCaminho(nome);
    std::vector<uebyte> dados = LeArquivo(caminho);
    js_resource_log("file", nome.c_str(), caminho.c_str(), static_cast<int>(dados.size()));
    return std::make_shared<std::vector<uebyte>>(std::move(dados));
}

// wasm func 8605 - slot 4 (getResourceMovie, iresource.h:87). Observed executing (the GIF animations).
// The urna builds a CMovie from decoded frames; the web build makes ONE 80 ms frame holding the whole GIF
// file and lets the browser animate it.
api::CMovie CWasmResource::GetMovie(const std::string& nome)
{
    const std::string caminho = ResolveCaminho(nome);
    std::vector<uebyte> dados = LeArquivo(caminho);
    js_resource_log("movie", nome.c_str(), caminho.c_str(), static_cast<int>(dados.size()));

    const api::SPoint tamanho = TamanhoImagem(dados);                           // inlined copy of func 3493
    // The frame is built by moving the bytes, but the vector is built from an initializer_list, whose elements
    // are const: the whole GIF is COPIED once more (operator new(size) + memcpy in 8605), then the temporary frame
    // is freed.
    std::vector<api::CMovieFrame> quadros{api::CMovieFrame{std::move(dados), std::chrono::milliseconds{80}}};
    return api::CMovie(std::move(quadros), tamanho);                            // func 5526 (cmovie.cpp:29, by value)
}

// wasm func 8600 - slot 5: the resource resolves to an existing file.
bool CWasmResource::Exists(const std::string& nome)
{
    return !ResolveCaminho(nome).empty();
}

// wasm func 8591 - slot 6 (isResource): resource names start with ':' (":/resource/...").
bool CWasmResource::IsResource(const std::string& nome)
{
    return !nome.empty() && nome[0] == ':';
}

}  // namespace simulador

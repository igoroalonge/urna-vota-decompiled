// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cfixedimage.cpp (srcloc cfixedimage.cpp:24, with iresource.h:97
// isResource and iresource.h:82 getResourceFile inlined). Merge into cfixedimage.cpp.
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "api/gui/gui-common.u15.h"
#include "api/gui/iresource.h"
#include "ecourna/api/io/cfile.hpp"

namespace api {

using uebyte = unsigned char;

// CFixedImage (vtable @1530272) - an IImage whose encoded bytes are loaded once.
//   +4 std::vector<uebyte> m_dados
class CFixedImage /* : public IImage */ {
public:
    explicit CFixedImage(const std::string& nome);
    virtual ~CFixedImage();          // 8563 (slot 0) / 8554 (slot 1): merged bodies 3937/3936
private:
    std::vector<uebyte> m_dados;     // +4
};

// wasm func 2246 (observed executing) - srcloc cfixedimage.cpp:24
// `nome` is either a resource path (":/resource/images/..." - IResource slot 6 tests the leading ':'
// in simulador::CWasmResource) or a file of the urna's storage (e.g. a candidate photo).
// Callers: battery icon table (1263), comum::md::CBiometriaEleitor::GetFoto, comum_f3617, vota_f13441.
CFixedImage::CFixedImage(const std::string& nome)
{
    if (nome.empty())
        throw CUeGuiError(static_cast<EUeGuiError>(4913), "Nome do arquivo vazio");

    if (!isResource(nome)) {                                     // IResource slot 6, iresource.h:97
        m_dados = ecourna::api::io::CFile::ReadFileBinary(std::filesystem::path(nome));
    } else {
        // getResourceFile (iresource.h:82) returns a fresh SharedVector (IResource slot 3; the web
        // implementation reads the packaged file and logs it with js_resource_log). Its contents are
        // swapped into the member.
        SharedVector recurso = getResourceFile(nome);
        m_dados.swap(*recurso);
    }
}

CFixedImage::~CFixedImage() = default;

} // namespace api

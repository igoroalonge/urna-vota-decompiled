// uenux2/src/app/comum/dados/cfotos.cpp  -- FRAGMENT written by unit u21 (srclocs cfotos.cpp:39/51, plus the
// legravaentidade.h:92/98 records of the inlined LeEntidadeEm). Other parts: cdadosestaticos.u02.cpp (GetInst,
// wasm 2819), the loader inlined into wasm 7787 (CFotos::Load, cfotos.cpp:111/121).
//
// CFotos (28 bytes, singleton std::unique_ptr @1838844, "mutex" @1838820 = no-op in this single-threaded build):
//   +0  std::map<std::string, SIndiceFoto> m_fotos     key = candidate code
//   +12 iterator m_atual                                last entry found (initialised to end())
//   +16 std::string m_nome = "CFotos"
// SIndiceFoto (map value, 32 bytes; names inferred): +0 std::string id, +12 md::CIndexer {offset, tamanho},
//   +20 std::string arquivo (the -fo.dat file).
#include "comum/dados/cfotos.h"

#include <format>
#include <memory>
#include <string>
#include <vector>

#include "api/util/cfilesystem.h"                              // api::ExisteArquivo (wasm 412: stat + S_ISREG)   // name inferred
#include "comum/asn/legravaentidade.h"                        // LeEntidadeEm<>
#include "comum/dados/asn/candidatura/cconversorfotocandidato.h"

namespace comum {

// wasm func 3756 - inline constructor, name inferred (called by GetInst 2819, by 3755 and by 12641, where GetInst is
// inlined).
CFotos::CFotos()
    : m_fotos(), m_atual(m_fotos.end()), m_nome("CFotos")
{
}

// wasm func 2818 - CFotos::~CFotos() (m_nome, then the tree: func 1934). name inferred
CFotos::~CFotos() = default;

// wasm func 11513 - atexit handler of the static std::unique_ptr<CFotos> (reset + delete). Compiler-generated.

namespace {

// cfotos.cpp:39 (inlined into 3755)
md::CFotoCandidato GetFotoPara(const std::string& contexto, const CFotos::SIndiceFoto& indice)
{
    const std::string arquivo = indice.arquivo;
    if (!api::ExisteArquivo(arquivo)) {
        throw CUeComumDadosError(7860,
            std::format("{} - arquivo [{}] da foto [{}] não encontrado", contexto, arquivo, indice.id));   // line 39
    }
    return LeEntidadeEm<asn::CConversorFotoCandidato>(arquivo, indice.indexador, "LeFotoCandidato(" + indice.id + ")");
}

// cfotos.cpp:51 (inlined into 3755)
md::CFotoCandidato GetFotoPara(const std::string& contexto, const std::string& id)
{
    CFotos& fotos = CFotos::GetInst();
    const auto it = fotos.Busca(id);                 // m_fotos.find(id); caches the hit in m_atual   // name inferred
    if (it == fotos.Fim()) {
        throw CUeComumDadosError(7861, std::format("{} - id de foto [{}] não encontrado", contexto, id));  // line 51
    }
    return GetFotoPara(contexto, it->second);
}

} // namespace

// wasm func 3755 - name inferred from the context string "CFotos::GetImagem(id)". The tool named it
// (anonymous namespace)::LeEntidadeEm after the inlined template (legravaentidade.h:92/98). Static: no `this`.
// Observed executing: each time a candidate screen with a photo is drawn (twice per photo, see
// CCandidaturasDSFoto in ccandidaturas.u21.cpp; a headless run typing Vereador 91001, with FS.open hooked, showed
// the -fo.dat opened twice after the 5th digit and twice more when the screen settled on CConfirmaVotoNominal,
// i.e. one pair per draw). Decodes ONE FotoCandidato by seeking into the -fo.dat file.
// Callers: 12641 (CDataImage<CCandidaturasDSFoto>, voting screen) and 12520 (the CDataImage of
// vota::CTelasVota::CriaTelaVisualizacaoCandidato, which reads the photo once, without the JPEG check). Both call
// CFotos::GetInst() (2819) just before and drop the result, which also fits a source form
// CFotos::GetInst().GetImagem(id) with the unused `this` removed; "static" is therefore an inference.   // ?
std::vector<uebyte> CFotos::GetImagem(const std::string& id)
{
    const md::CFotoCandidato foto = GetFotoPara("CFotos::GetImagem(id)", id);
    return foto.GetFoto().GetImagem();              // copy of the OCTET STRING bytes (vector at CFotoCandidato +16)
}

} // namespace comum

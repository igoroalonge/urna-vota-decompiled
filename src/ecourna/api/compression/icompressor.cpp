// ecourna-lib/ecourna/api/compression/icompressor.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/compression/icompressor.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// Where the code of this file ended up in the binary:
//   func 5192   ICompressor::ICompressor()
//   func 9530   ICompressor::Add(const std::vector<path>&)
//   func 9529   ICompressor::Add(const std::map<path, path>&)              (unit u15; reproduced for completeness)
//   func 9532   the per-entry lambda of globToFileList (regex match + push_back)
//   ---         ICompressor::Add(const path&) and globToFileList() (srcloc line 61) have no body of their
//               own: both are inlined into their only caller, comum::CGravadorWSQ's archive routine (func 5821,
//               see src/uenux2/src/app/comum/gravadores/u12-foreign-fragments.cpp). The analyzer named
//               func 5821 after the srcloc.
//
// Error type: exception::CBaseError<ECompressionError, SErrorLimits{1000, 1100}> (typeinfo @1108484,
// vtable @1110656), built by the thunk func 9584 -> shared body func 1011.   (CCompressionError: alias name inferred)
#include "ecourna/api/compression/icompressor.hpp"

#include <filesystem>
#include <regex>
#include <string>
#include <vector>

#include "ecourna/api/compression/ecompressionerror.hpp"   // ECompressionError / CCompressionError (not reconstructed)

namespace ecourna::api::compression {

// -------------------------------------------------------------------------------------------------------
// ECompressionError codes used in this binary (limits 1000..1100; enumerator names unknown):
//   1013  CLzmaCompress::DoAdd  "(0x{:X} - {}) Não foi possível incluir o arquivo {} no pacote {}."   clzmacompress.cpp:114
//   1039  CZip::Close           "O arquivo {} não pode ser fechado. {}"                               czip.cpp:139
//   1040  CZip::ConvertToOpen   "Modo de abertura do zip incorreto."                                  czip.cpp:155
//   1041  CZip::CreateZipFile   "O arquivo {} não pode ser criado."                                   czip.cpp:170 (u15)
//   1042  CZip::AssertZipIsOpened "O arquivo {} está fechado."                                        czip.cpp:188
//   1043  CZip::ConvertToCompressLevel "Modo de compressão incorreto."                                czip.cpp:207
//   1044  CZip::DoAdd           "O arquivo {} não pode ser adicionado. {}"                            czip.cpp:244
//   1045  CZip::DoAdd           "Falha ao adicionar o arquivo {}. {}"                                 czip.cpp:264
//   1046  CZip::DoAdd           "A adição do arquivo {} não pode ser concluída. {}"                   czip.cpp:281
//   1047  globToFileList        "[<glob>] é um glob inválido"                                         icompressor.cpp:61
// -------------------------------------------------------------------------------------------------------

namespace {

// Glob -> ECMAScript regex, character by character. Only these four characters are translated; the other
// regex metacharacters ('(', '[', '+', '^', '$', '|', '{') are copied as they are.
std::string GlobParaRegex(const std::string& glob)                    // name inferred (inlined in func 5821)
{
    std::string regex;
    for (const char c : glob) {
        switch (c) {
        case '*':  regex.append(".*");   break;   // @378615
        case '.':  regex.append("\\.");  break;   // @375844
        case '?':  regex.append(".");    break;   // @378041
        case '\\': regex.append("\\\\"); break;   // @319771
        default:   regex.push_back(c);   break;
        }
    }
    return regex;
}

// srcloc line 61 (inlined in func 5821)
std::vector<std::filesystem::path> globToFileList(const std::filesystem::path& padrao)
{
    std::vector<std::filesystem::path> arquivos;

    std::filesystem::path glob = padrao;
    std::filesystem::path diretorio = padrao.parent_path();
    if (diretorio == std::filesystem::path{}) {      // compiled as path::compare(string_view{""}) == 0
        diretorio = ".";                             // @378041
        glob = diretorio / padrao;
    }

    if (!std::filesystem::is_directory(diretorio)) {  // status() (funcs 7745 -> 1055 -> 3334), type != directory
        throw CCompressionError(ECompressionError(1047), "[" + glob.string() + "] é um glob inválido");   // line 61
    }

    // regex over the WHOLE path (directory included), matched with regex_match against entry.path().string()
    const std::regex expressao(GlobParaRegex(glob.string()));        // flags = ECMAScript (512 in libc++ ABI v2)

    // wasm func 9532 (the loop body, reached through table slot 6145)
    const auto adicionaSeCombina = [&arquivos, &expressao](const std::filesystem::directory_entry& entrada) {
        const std::filesystem::path caminho = entrada.path();
        if (entrada.is_regular_file() && std::regex_match(caminho.string(), expressao))   // match_flags 4160 = __full_match|match_continuous
            arquivos.push_back(caminho);
    };

    // A glob that ends in "/*" (@378604) is expanded RECURSIVELY; any other glob only lists `diretorio`.
    if (glob.string().ends_with("/*")) {
        for (const auto& entrada : std::filesystem::recursive_directory_iterator(diretorio))
            adicionaSeCombina(entrada);
    } else {
        for (const auto& entrada : std::filesystem::directory_iterator(diretorio))
            adicionaSeCombina(entrada);
    }
    return arquivos;            // directory order (not sorted)
}

bool EhGlob(const std::filesystem::path& arquivo)                      // name inferred (inlined)
{
    return arquivo.string().find_first_of("*?") != std::string::npos;
}

} // namespace

// wasm func 5192
ICompressor::ICompressor()
    : NonCopyable(), IObservableProgressWithDescription()   // signal<>: new signal_impl (20 bytes, func via slot 6135)
{
    // vptr = ICompressor (@1110896)
}

// inlined into func 5821
void ICompressor::Add(const std::filesystem::path& arquivo)
{
    if (ExpandeGlob() && EhGlob(arquivo))      // vtable slot 3
        Add(globToFileList(arquivo));
    else
        Add(std::vector<std::filesystem::path>{arquivo});   // func 9531 = vector(first, last, n = 1)
}

// wasm func 9530
void ICompressor::Add(const std::vector<std::filesystem::path>& arquivos)
{
    for (const auto& arquivo : arquivos)
        DoAdd(arquivo, arquivo.filename());    // vtable slot 4; the name inside the archive = file name only
}

// wasm func 9529 (unit u15)
ICompressor& ICompressor::Add(const std::map<std::filesystem::path, std::filesystem::path>& arquivos)
{
    for (const auto& [origem, destino] : arquivos)
        DoAdd(origem, destino);
    return *this;
}

} // namespace ecourna::api::compression

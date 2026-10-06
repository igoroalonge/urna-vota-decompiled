// uenux2/src/api/io/cinistrings.cpp  -- FRAGMENT reconstructed by unit u12
// (the file belongs to the uenux2 api/io unit; u12 owns these functions only because the analyzer attached
//  them to ecourna's cfile.cpp: func 5480 inlines CFile::ReadLine and got named after its srcloc)
//
// Reconstructed from vota_web_wasm.wasm. See docs/modules/u12-...md §4.
//
// api::CIniStrings is the ".properties"/INI reader of uenux2. In this binary its only user is
// comum::CGeracaoVersoesContratos (inlined into vota::CGravaResultado::StartState, func 12098), which loads
//     <root>/etc/dependencias.properties   and   <root>/etc/versoes.properties
// and then checks the "tag" key. (In the simulator both files are 0-byte stubs: the parser returns empty maps.)
//
// Syntax accepted (from the code):
//   * physical lines of any length (CFile::ReadLine), '\n' removed;
//   * a line that ends with an odd number of '\' continues on the next line (the '\' is dropped);
//   * comments start at the first unescaped '#' or ';' (an embedded NUL also cuts the line);
//   * blanks (isspace, or ' '/'\t') are trimmed on both sides, except an escaped trailing blank ("\ ");
//   * "[name]" opens a section (a repeated section MERGES into the first one);
//   * "key = value" (the first unescaped '='); keys before any section go to a global key map;
//   * escapes in names and values: \n \t \; \: \= \# "\ " \[ \] \\  (replaced in this order).
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "api/io/cinikey.h"          // api::CIniKey      { std::string nome (+0); std::string valor (+12); }  24 bytes
#include "api/io/cinisection.h"      // api::CIniSection  { std::string nome (+0); std::map<std::string, CIniKey> chaves (+12); }
#include "api/io/cinistrings.h"
#include "api/io/euioerror.h"        // CUeIoError = ecourna::api::exception::CBaseError<api::EUeIoError, ...> (typeinfo @1528172)
#include "ecourna/api/io/cfile.hpp"

namespace api {

namespace {

// Escape table: static std::vector<std::pair<std::string, char>> @1839288 (10 x 16-byte entries), filled
// by __wasm_call_ctors (func 14478), destroyed by func 10879 (atexit).                        name inferred
const std::vector<std::pair<std::string, char>> ESCAPES = {
    {"\\n", '\n'}, {"\\t", '\t'}, {"\\;", ';'}, {"\\:", ':'}, {"\\=", '='},
    {"\\#", '#'},  {"\\ ", ' '},  {"\\[", '['}, {"\\]", ']'}, {"\\\\", '\\'},
};
// Note: the rules run in table order with a plain replace-all, and "\\\\" -> '\' runs LAST. So the file text
// `a\\n` (two backslashes, then n) does not give `a\n`: the "\n" rule runs first on the second backslash
// and the 'n' and gives `a\` + newline. A literal backslash followed by n t ; : = # [ ] or a blank cannot be
// written.

// wasm func 6272 (name inferred)
bool EhBrancoExtra(const int c)
{
    return c == ' ' || c == '\t';
}

// wasm func 2234 (name inferred). Position of the first occurrence of `alvo` that is NOT preceded by an odd
// number of backslashes, or npos. An escaped hit restarts the search after it (recursively).
std::size_t FindNaoEscapado(const std::string& texto, const std::string& alvo)
{
    const std::size_t pos = texto.find(alvo);
    if (pos == std::string::npos || pos == 0)
        return pos;

    std::size_t barras = 0;
    for (std::size_t i = std::min(pos, texto.size()); i > 0 && texto[i - 1] == '\\'; --i)
        ++barras;
    if (barras % 2 == 0)
        return pos;

    const std::size_t resto = FindNaoEscapado(texto.substr(pos + 1), alvo);   // substr: out_of_range if pos >= size
    return resto == std::string::npos ? std::string::npos : resto + pos + 1;
}

// wasm func 2236 (name inferred). Cuts the line at the first unescaped '#', ';' or NUL.
std::string RemoveComentario(const std::string& linha)
{
    const std::size_t pos = std::min({FindNaoEscapado(linha, "#"),
                                      FindNaoEscapado(linha, ";"),
                                      FindNaoEscapado(linha, std::string(1, '\0'))});
    if (pos == std::string::npos)
        return linha;
    return linha.substr(0, pos);
}

// wasm func 2235 (name inferred). Trims blanks; a trailing blank preceded by an odd number of '\' is kept.
std::string Trim(const std::string& texto)
{
    std::size_t inicio = 0;
    while (inicio < texto.size() && (std::isspace(texto[inicio]) || EhBrancoExtra(texto[inicio])))
        ++inicio;

    // Index of the last kept character. The binary computes size - 1 as an unsigned value, so for an EMPTY
    // string `ultimo` is SIZE_MAX and the loop reads texto[SIZE_MAX], the byte just before the (SSO) buffer,
    // and keeps scanning backwards while it finds blanks. The result is "" anyway (substr length wraps to 0).
    // Callers pass empty strings routinely (EhLinhaUtil / RemoveComentario of a comment-only line).
    std::size_t ultimo = texto.size() - 1;
    while (inicio < ultimo) {
        const char c = texto[ultimo];
        if (!std::isspace(c) && !EhBrancoExtra(c))
            break;
        std::size_t barras = 0;
        for (std::size_t i = ultimo; i > 0 && texto[i - 1] == '\\'; --i)
            ++barras;
        if (barras % 2 != 0)
            break;                                  // escaped blank: keep it
        --ultimo;
    }
    return texto.substr(inicio, ultimo - inicio + 1);
}

// wasm func 3651 (name inferred). Comment removal + trim + escape decoding (ReplaceAll = func 2677).
std::string Decodifica(const std::string& texto)
{
    std::string resultado = Trim(RemoveComentario(texto));
    for (const auto& [sequencia, caractere] : ESCAPES)
        resultado = ReplaceAll(resultado, sequencia, std::string(1, caractere));   // api_f2677 -> shared_f9407
    return resultado;
}

// wasm func 5484 (name inferred). True when the line ends with an odd number of '\'.
bool TerminaComContinuacao(const std::string& linha)
{
    std::size_t barras = 0;
    for (std::size_t i = linha.size(); i > 0 && linha[i - 1] == '\\'; --i)
        ++barras;
    return barras % 2 != 0;
}

// wasm func 5485 (name inferred). True when something is left after removing comments and blanks.
bool EhLinhaUtil(const std::string& linha)
{
    return !Trim(RemoveComentario(linha)).empty();
}

} // namespace

// srcloc cinistrings.cpp:183, :187 (inlined into func 5480)
void CIniStrings::ParseKeyLine(std::string& chave, std::string& valor, const std::string& linha)
{
    const std::string texto = Trim(RemoveComentario(linha));
    const std::size_t igual = FindNaoEscapado(texto, "=");
    if (igual == std::string::npos)
        throw CUeIoError(EUeIoError(6009), std::format("Linha inválida [{}]", texto));              // line 183

    chave = Decodifica(texto.substr(0, igual));
    if (chave.empty())
        throw CUeIoError(EUeIoError(6010), std::format("Linha com nome inválido [{}]", texto));     // line 187
    valor = Decodifica(texto.substr(igual + 1));
}

// wasm func 5482 (name inferred): insert-or-assign a key in a section. The same code is inlined in func
// 5480 for the global key map.
void CIniSection::SetChave(const CIniKey& chave)
{
    auto it = m_chaves.find(chave.GetNome());
    if (it != m_chaves.end())
        it->second = chave;                       // CIniKey::operator= (func 5483): throws 6005 "O nome de uma
                                                  // chave não pode ser modificado." if the names differ
    else
        m_chaves.emplace(chave.GetNome(), chave);
}

// wasm func 5480 (constructor or static loader: the object is built in place; name inferred).
// Phase 1 reads the logical lines; phase 2 parses them. Both loops are inlined into one function.
CIniStrings::CIniStrings(const std::string& arquivo)
{
    ecourna::api::io::CFile file(arquivo, "rb");                    // func 517, mode 25202 = "rb"

    // ---- phase 1: logical lines ------------------------------------------------------------------
    std::vector<std::string> linhas;
    for (;;) {
        std::string logica;
        bool continua = false;
        do {
            std::string fisica;
            file.ReadLine(fisica);                                  // CFile::ReadLine inlined (cfile.cpp:336/356)
            if (fisica.empty() && file.Eof())
                break;
            if (fisica.back() == '\n')
                fisica.pop_back();
            logica += fisica;
            continua = TerminaComContinuacao(logica);
            if (continua)
                logica.pop_back();
        } while (continua);

        logica = Trim(RemoveComentario(logica));
        if (EhLinhaUtil(logica))
            linhas.push_back(logica);
        else if (file.Eof())
            break;
    }

    // ---- phase 2: sections and keys (looks like a separate Parse(const std::vector<std::string>&)) ----
    CIniSection* secaoAtual = nullptr;
    for (auto it = linhas.begin(); it != linhas.end(); ++it) {
        std::string linha;
        bool continua = false;
        do {                                    // continuation joining again (never true for phase-1 output)
            std::string parte = *it;
            if (parte.back() == '\n')           // no emptiness check (phase-1 lines are never empty)
                parte.pop_back();
            linha += parte;
            continua = TerminaComContinuacao(linha);
            if (continua)
                linha.pop_back();
        } while (continua && ++it != linhas.end());
        // BUG (unreachable from files): if the last line still ended with '\', `it` is now end() and the
        // `++it` of the for-loop steps past the end of the vector.

        linha = Trim(RemoveComentario(linha));
        if (!EhLinhaUtil(linha))
            continue;

        if (linha.size() >= 2 && linha.front() == '[' && linha.back() == ']') {
            const std::string nome = Decodifica(linha.substr(1, linha.size() - 2));
            const CIniSection nova(nome);
            auto existente = m_secoes.find(nome);
            if (existente != m_secoes.end()) {
                CIniSection juncao(existente->second);              // copy of the section read before ...
                for (const auto& [n, chave] : nova.GetChaves())     // ... plus the new one's keys (none yet)
                    juncao.SetChave(chave);
                existente->second = juncao;                         // CIniSection::operator= (cinisection.cpp:36):
                                                                    // throws 6007 "O nome de uma seção é constante."
                                                                    // if the names differ (they cannot here)
            } else {
                existente = m_secoes.emplace(nome, nova).first;
            }
            secaoAtual = &existente->second;
            continue;
        }

        std::string chave, valor;
        ParseKeyLine(chave, valor, linha);
        const CIniKey par(chave, valor);
        if (secaoAtual == nullptr) {                                // keys before the first [section]
            auto it2 = m_chaves.find(par.GetNome());
            if (it2 != m_chaves.end())
                it2->second = par;                                  // CIniKey::operator= (func 5483)
            else
                m_chaves.emplace(par.GetNome(), par);
        } else {
            secaoAtual->SetChave(par);                              // func 5482
        }
    }
}   // ~vector<string>, ~CFile (Close)

// Library code the analyzer attributed to this file:
//   func 1397  std::__tree<pair<string, CIniKey>>::destroy(node)   (52-byte nodes: 3 strings)
//   func 3650  std::__tree<pair<string, CIniKey>>::__emplace_hint_unique_key_args  (map::insert(hint, value),
//              used by the CIniSection copy constructor)

} // namespace api

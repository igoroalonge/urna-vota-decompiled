// uenux2/src/app/comum/validamidia/cvalidamidia.cpp
// Reconstructed from vota_web_wasm.wasm (unit u26). std::source_location record of this file: :182
// (IValidaMidia::GetInst, func 5575, reconstructed by unit u19 in cvalidamidia.u19.cpp). The content list of an
// initialised medium comes from cfabricaconteudomidiastart.cpp (:62, inlined into func 11172).
//
// Only caller of the interface: vota::CCopiaResultadoParaMR (func 12134, slots 3 and 6), i.e. the end of the
// day - never reached in the web build (and /dsk/mr/ does not exist in the simulator's MEMFS).
//
// Name matching (func 2758 + inlined lambda + 5464/5465): a permitted/expected name is either
//   * "#regex#<ECMAScript regex>": boost::regex_search (func 2199), NOT anchored - the pattern may match any
//     part of the file name; or
//   * a literal of the same length where, in either string, '#' matches a digit, '@' a letter and '?' a
//     letter, digit, '-' or '_'.
#include "comum/validamidia/cvalidamidia.h"

#include <cctype>
#include <cstdarg>
#include <filesystem>
#include <format>
#include <map>
#include <string>
#include <sys/stat.h>
#include <syslog.h>
#include <system_error>
#include <vector>

#include "api/util/cdirreader.h"
#include "api/util/cstringutils.h"        // api::CStringUtils::RegexFind (func 2199)
#include "api/util/csystem.h"
#include "comum/cpath.h"
#include "comum/validamidia/cfabricaconteudomidiastart.h"

namespace comum::impl {

namespace {

constexpr std::string_view PREFIXO_REGEX = "#regex#";

// Names of the regex of result files: <fase o|s|t><pleito:05><uf:2><município:05><zona:04><seção:04>-<tipo>.<ext>
constexpr const char* REGEX_ARQUIVOS_RESULTADO = R"(#regex#[ost][0-9]{5}[a-z]{2}[0-9]{13}-[a-z]+\.[a-z]+)";

// Everything that may be found on a result stick (inlined vector of 14 strings in func 5572).
const std::vector<std::string>& ArquivosPermitidos()                     // name inferred
{
    static const std::vector<std::string> permitidos{
        "infomidia.dat",
        "infomidia.vsc",
        "turno2.jez",
        "turno2.vsc",
        R"(#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.jez)",
        R"(#regex#[ost][0-9]{5}[a-z]{2}-pkgsa\.vsc)",
        REGEX_ARQUIVOS_RESULTADO,
        R"(#regex#[ost][0-9]{5}[0-9]{4}[0-9]{4}[0-9]{8}-[a-z]+\.[a-z]+)",
        R"(#regex#[0-9]{8}\.[ste|STE])",          // a character class, not "(ste|STE)" as intended   (sic)
        "ste.config",
        "#regex#[0-9]{2}.pub",
        "#regex#[0-9]{2}.id",
        "#regex#[0-9]{2}ue",
        R"(#regex#[0-9]{3}ue[0-9]{2}\.vpe)",
    };
    return permitidos;                             // (built on the stack at every call in the binary)
}

// wasm func 5574 (name inferred): LOG_INFO variant of the header helper shared with cthreadeleitor.cpp
// (merged body vota_f2921): syslog when /dev/urna exists (cached in the static @1577092), otherwise printf
// only if the environment variable DEBUG_UENUX is set. Silent in the browser.
void LogInfo(const char* formato, ...)
{
    static int s_urnaReal = -1;                   // @1577092
    va_list args;
    va_start(args, formato);
    LogSistema(formato, args, LOG_INFO, s_urnaReal);                      // vota_f2921
    va_end(args);
}

// wasm func 5464 (name inferred). Wildcards of the literal names.
bool CaractereCoringa(char mascara, char c)
{
    switch (mascara) {
    case '#': return c >= '0' && c <= '9';
    case '@': return std::isalpha(static_cast<unsigned char>(c)) != 0;
    case '?': return (c >= '0' && c <= '9') || ((c | 0x20) >= 'a' && (c | 0x20) <= 'z') || c == '-' || c == '_';
    default:  return false;
    }
}

// wasm func 5465 (name inferred). `padrao` starts with "#regex#"; the rest is searched in `texto`.
bool BuscaRegex(const std::string& padrao, const std::string& texto)
{
    return api::CStringUtils::RegexFind(padrao.substr(PREFIXO_REGEX.size()), texto).first != -1;   // func 2199
}

// Inlined lambda of func 2758 (name inferred).
bool Corresponde(const std::string& a, const std::string& b)
{
    if (a.starts_with(PREFIXO_REGEX))
        return BuscaRegex(a, b);
    if (b.starts_with(PREFIXO_REGEX))
        return BuscaRegex(b, a);
    if (a.size() != b.size())
        return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        if (b[i] == a[i] || CaractereCoringa(b[i], a[i]) || CaractereCoringa(a[i], b[i]))
            continue;
        return false;
    }
    return true;
}

// wasm func 2758 (name inferred). True when every name of `nomes` corresponds to some name of `referencia`.
// Returns false when either list is empty.
bool TodosCorrespondem(const std::vector<std::string>& nomes, const std::vector<std::string>& referencia)
{
    bool resultado = false;
    for (const std::string& nome : nomes) {
        if (referencia.empty())
            return false;
        const std::string copia = nome;
        resultado = std::any_of(referencia.begin(), referencia.end(),
                                [&](const std::string& r) { return Corresponde(copia, r); });
        if (!resultado)
            return false;
    }
    return resultado;
}

// wasm func 5571 (name inferred). Deletes the metadata that desktop operating systems leave on a USB stick.
// `diretorio` must end with '/': the names are appended to it. The THROWING overload remove_all(path) is
// used (the libc++ ErrorHandler built inline has ec = nullptr): a missing name is not an error
// (remove_all_impl, libcxx_f4678, clears ENOENT and returns 0), but any other failure (EACCES, EROFS, EBUSY,
// EIO ...) makes ErrorHandler::report (ecourna_f4679) throw std::filesystem::filesystem_error("remove_all"),
// which escapes ValidaConteudo / ValidaMidiaResultado (no try/catch in 5571, 5572 or 11172).
void RemoveArquivosSO(const std::string& diretorio)
{
    const std::map<std::string, std::vector<std::string>> arquivosSO{
        {"macOS", {".DS_Store", ".AppleDouble", ".LSOverride", ".DocumentRevisions-V100", ".fseventsd",
                   ".Spotlight-V100", ".TemporaryItems", ".Trashes", ".VolumeIcon.icns",
                   ".com.apple.timemachine.donotpresent"}},
        {"Windows", {"Thumbs.db", "Thumbs.db:encryptable", "ehthumbs.db", "ehthumbs_vista.db",
                     "System Volume Information", "$RECYCLE.BIN"}},
    };
    for (const auto& [sistema, arquivos] : arquivosSO) {                   // map order: "Windows" < "macOS"
        for (const auto& arquivo : arquivos)
            std::filesystem::remove_all(diretorio + arquivo);                // __remove_all(p, nullptr) inlined
    }
}

// wasm func 5572 (name inferred). The general validation of a medium directory.
SResultadoValidacao ValidaConteudo(const std::string& diretorio, const std::vector<std::string>& esperados)
{
    RemoveArquivosSO(diretorio);

    if (!api::CSystem::IsDirectory(diretorio) || api::CSystem::IsEmptyDir(diretorio)) {      // funcs 5459, IsEmptyDir
        if (esperados.empty())
            return {true, EMotivoValidacao::MIDIA_VAZIA};
        return {false, EMotivoValidacao::MIDIA_VAZIA_ESPERAVA_ARQUIVOS};
    }

    std::vector<std::string> arquivos;
    {
        api::CDirReader leitor(diretorio);                                                  // func 1914
        while (leitor.NextEntry()) {                                                        // func 1260
            const std::string nome = leitor.GetEntryName();
            if (nome == "." || nome == "..")
                continue;
            if (leitor.IsDirectory())                                                       // func 1912
                return {false, EMotivoValidacao::CONTEM_DIRETORIO};
            if (leitor.IsFile())                                                            // func 2231
                arquivos.push_back(nome);
        }
        leitor.Close();                                                                     // func 2232
    }

    if (!esperados.empty() && esperados.size() > arquivos.size())
        return {false, EMotivoValidacao::ARQUIVOS_INSUFICIENTES};
    if (!TodosCorrespondem(arquivos, ArquivosPermitidos()))
        return {false, EMotivoValidacao::ARQUIVO_NAO_PERMITIDO};
    if (!esperados.empty() && !TodosCorrespondem(esperados, arquivos))
        return {false, EMotivoValidacao::ARQUIVO_ESPERADO_AUSENTE};
    return {true, EMotivoValidacao::MIDIA_VALIDA};
}

// wasm func 2784 (name inferred): only the `valida` flag of ValidaConteudo.
bool ConteudoValido(const std::string& diretorio, const std::vector<std::string>& esperados)
{
    return ValidaConteudo(diretorio, esperados).valida;
}

} // namespace

// ---------------------------------------------------------------------------------------------------------
// wasm func 11173 (vtable slot 2)
bool CValidaMidia::MidiaResultadoValida(EAplicativosDeUrna aplicativo, EUrnaTurno turno)
{
    return ValidaMidiaResultado(aplicativo, turno).valida;                // virtual call, slot 3
}

// wasm func 11172 (vtable slot 3; CFabricaConteudoMidiaStart::ConteudoIncializacao inlined, srcloc :62)
SResultadoValidacao CValidaMidia::ValidaMidiaResultado(EAplicativosDeUrna aplicativo, EUrnaTurno turno)
{
    const int app = static_cast<int>(aplicativo);
    if (app < 1 || app > 20)
        return {false, EMotivoValidacao::APLICATIVO_INVALIDO};

    const bool turnoValido = turno == EUrnaTurno('1') || turno == EUrnaTurno('2');
    const bool exigeTurno = (app >= 7 && app <= 10) || app == 15 || app == 16 || app == 17;
    if (exigeTurno && !turnoValido)
        return {false, EMotivoValidacao::TURNO_INVALIDO};

    const std::vector<std::string> esperados = CFabricaConteudoMidiaStart::ConteudoIncializacao(aplicativo, turno);
    const SResultadoValidacao resultado = ValidaConteudo(CPath::GetPathMR(), esperados);   // func 949: "/dsk/mr/"
    if (static_cast<int>(resultado.motivo) >= 4) {
        LogInfo("mídia inválida [%d] arquivos:", static_cast<int>(resultado.motivo));
        for (const auto& arquivo : esperados)
            LogInfo("- %s", arquivo.c_str());
    }
    return resultado;
}

// wasm func 11171 (vtable slot 4). Empty/absent stick, or only permitted files.
bool CValidaMidia::MidiaSemArquivosIndevidos()
{
    return ConteudoValido(CPath::GetPathMR(), {});
}

// wasm func 11170 (vtable slot 5). The stick holds the result files of this section.
// Format argument types (packed 207828162): char, unsigned, string, unsigned, unsigned, unsigned.
bool CValidaMidia::MidiaContemResultadosSecao(TPleito pleito, const std::string& uf, TMunicipio municipio,
                                         TZona zona, TSecao secao, char fase)
{
    const std::string padrao = std::format(R"(#regex#{}{:05}{}{:05}{:04}{:04}-[a-z]+\.[a-z]+)",
                                           fase, pleito, uf, municipio, zona, secao);
    return ConteudoValido(CPath::GetPathMR(), {padrao});
}

// wasm func 11169 (vtable slot 6). The stick holds result files of some urna.   name as in unit u09
bool CValidaMidia::MidiaContemResultados()
{
    return ConteudoValido(CPath::GetPathMR(), {REGEX_ARQUIVOS_RESULTADO});
}

// wasm func 11168 (vtable slot 7)
bool CValidaMidia::MidiaContemArquivo(const std::string& arquivo)
{
    return ConteudoValido(CPath::GetPathMR(), {arquivo});
}

// wasm func 11167 (vtable slot 8). Strict check (inlined helper): the stick is a directory holding only
// result files - no OS metadata cleanup, no permitted extras, no sub-directory.
bool CValidaMidia::MidiaContemSomenteResultados()
{
    const std::vector<std::string> esperados{REGEX_ARQUIVOS_RESULTADO};
    const std::string diretorio = CPath::GetPathMR();

    struct stat info {};
    if (esperados.empty() || !api::ExistResource(info, diretorio) || !S_ISDIR(info.st_mode)
        || api::CSystem::IsEmptyDir(diretorio))
        return false;

    std::vector<std::string> arquivos;
    api::CDirReader leitor(diretorio);
    while (leitor.NextEntry()) {
        const std::string nome = leitor.GetEntryName();
        if (leitor.IsDirectory() && nome != "." && nome != "..")
            return false;
        if (leitor.IsFile())
            arquivos.push_back(nome);
    }
    leitor.Close();

    if (esperados.size() > arquivos.size() || !TodosCorrespondem(esperados, arquivos))
        return false;
    return TodosCorrespondem(arquivos, esperados);
}

// wasm func 11166 (vtable slot 9)
void CValidaMidia::RemoveArquivosSistemaOperacional()
{
    RemoveArquivosSO(CPath::GetPathMR());
}

} // namespace comum::impl

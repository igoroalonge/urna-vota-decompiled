// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/md/parametrizacaourna/ctradutorfrase.cpp
// Observed executing during the recorded votes (every screen/report text goes through Traduz).
#include "ctradutorfrase.h"

#include <cctype>
#include <set>
#include <utility>
#include <vector>

#include "api/util/cstringutils.h"   // api::CStringUtils::RegexFind (api_f2199), Split (comum_f1880)
#include "ecourna/api/util/cstringutils.h"   // ToWord (2201)

namespace comum::md {

namespace {
using CUeComumDadosError = ecourna::api::exception::CBaseError<comum::EUeComumDadosError,
                                                               ecourna::api::exception::SErrorLimits{7800, 8600}>;

// Upper-case accented letters of Latin-1 as (signed) char values. The table is stored in .rodata as
// 19 ints (@1118052) followed by "xXabcdefABCDEF0123456789" (unrelated neighbour).
constexpr int MAIUSCULAS_ACENTUADAS[] = {
    char('\xC7'), char('\xC1'), char('\xC9'), char('\xCD'), char('\xD3'), char('\xDA'),   // Ç Á É Í Ó Ú
    char('\xC0'), char('\xC8'), char('\xCC'), char('\xD2'), char('\xD9'),                // À È Ì Ò Ù
    char('\xC2'), char('\xCA'), char('\xCE'), char('\xD4'), char('\xDB'),                // Â Ê Î Ô Û
    char('\xC3'), char('\xD5'), char('\xD1'),                                             // Ã Õ Ñ
};

// {inicio, fim} of the first match of `regex` in `texto`, or {npos, npos}
// (api_f2199: boost::basic_regex::do_assign + boost::regex_search; exceptions are swallowed).
using TIntervalo = std::pair<std::size_t, std::size_t>;
TIntervalo Procura(const std::string& regex, const std::string& texto)
{
    return api::CStringUtils::RegexFind(regex, texto);
}
}  // namespace

std::map<char, ecourna::app::dados::CLabelParametrizado> CTradutorFrase::s_labels;
api::CDate CTradutorFrase::s_dataReferencia;

// wasm func 3509  name inferred
void ConverteMaiusculas(std::string& texto)
{
    // function-local static (guard byte @1911832, set @1911820): lower-case accented letters
    static const std::set<int> minusculas = [] {
        std::set<int> s;
        for (int c : std::set<int>(std::begin(MAIUSCULAS_ACENTUADAS), std::end(MAIUSCULAS_ACENTUADAS)))
            s.insert(c | 0x20);
        return s;
    }();
    for (char& c : texto) {
        if (minusculas.contains(c))
            c = static_cast<char>(c & 0xDF);
        else if (std::isalpha(c))        // NOTE: called with a possibly negative char
            c = static_cast<char>(std::toupper(c));
    }
}

// wasm func 5158  name inferred
void ConverteMinusculas(std::string& texto)
{
    // function-local static (guard byte @1911848, set @1911836); built by func 5157 =
    // std::set<int>::set(std::initializer_list<int>) (library instantiation)
    static const std::set<int> maiusculas(std::begin(MAIUSCULAS_ACENTUADAS), std::end(MAIUSCULAS_ACENTUADAS));
    for (char& c : texto) {
        if (maiusculas.contains(c))
            c = static_cast<char>(c | 0x20);
        else if (std::isalpha(c))
            c = static_cast<char>(std::tolower(c));
    }
}

// wasm func 5658 (srcloc line 95)
const ecourna::app::dados::CLabelParametrizado& CTradutorFrase::RecuperaLabel(const std::string& token)
{
    const auto it = s_labels.find(token.at(0));   // empty token -> std::out_of_range ("basic_string")
    if (it == s_labels.end())
        throw CUeComumDadosError(8128, "Token inválido: " + token);
    return it->second;
}

// inlined into func 654 (srclocs lines 104, 119, 132). conteudo = "KFTC" without the angle brackets.
std::string CTradutorFrase::TraduzLabel(const std::string& conteudo)
{
    if (conteudo.size() != 4)
        throw CUeComumDadosError(8129, "Label inválido: " + conteudo);              // line 104
    const auto& label = RecuperaLabel(conteudo);

    const std::string forma = conteudo.substr(1, 2);
    std::string texto;
    if (forma == "CS")      texto = label.GetCurto();
    else if (forma == "CP") texto = label.GetCurtoPlural();
    else if (forma == "LS") texto = label.GetLongo();
    else if (forma == "LP") texto = label.GetLongoPlural();
    else throw CUeComumDadosError(8130, "Label inválido: " + conteudo);            // line 119

    switch (conteudo.at(3)) {
    case 'A': ConverteMaiusculas(texto); break;
    case 'B': ConverteMinusculas(texto); break;
    case 'N': break;
    default:  throw CUeComumDadosError(8131, "Label inválido: " + conteudo);       // line 132
    }
    return texto;
}

// inlined into func 654 (srclocs lines 141, 156). conteudo = "K|neutro|masculino|feminino".
std::string CTradutorFrase::TraduzTexto(const std::string& conteudo)
{
    const std::vector<std::string> partes = api::CStringUtils::Split(conteudo, '|');   // comum_f1880
    if (partes.size() != 4)
        throw CUeComumDadosError(8132, "Texto inválido: " + conteudo);             // line 141
    switch (RecuperaLabel(conteudo).GetGenero()) {    // 0/1/2 (ASN.1 neutro=1, masculino=2, feminino=3)
    case 0: return partes.at(1);
    case 1: return partes.at(2);
    case 2: return partes.at(3);
    }
    throw CUeComumDadosError(8133, "Texto inválido: " + conteudo);                 // line 156
}

// inlined into func 654. conteudo = "DT+N" / "DT-N".
std::string CTradutorFrase::TraduzData(const std::string& conteudo)
{
    const auto dias = ecourna::api::util::CStringUtils::ToWord(conteudo.substr(3));   // uint16
    const api::CDate data = conteudo.at(2) == '+'
        ? (api::CDate(s_dataReferencia) += dias)       // func 5478 (CDate add days)
        : s_dataReferencia - dias;                     // func 3648 -> 5477 (CDate subtract days)
    return data.Format("DD/MM/YYYY");                  // api::CDate::Format (706)
}

// wasm func 654  name inferred (tools: CTradutorFrase::TraduzLabel)
std::string CTradutorFrase::Traduz(const std::string& frase)
{
    if (s_labels.empty())
        return frase;

    std::string chaves = "[";
    for (const auto& [chave, label] : s_labels)
        chaves += chave;          // NOTE: keys are not escaped inside the regex character class
    chaves += "]";

    const std::string reLabel = "<" + chaves + "[CL][PS][ABN]>";
    const std::string qualquer = "[^|>]*";
    const std::string reTexto = "<" + chaves + "\\|" + qualquer + "\\|" + qualquer + "\\|" + qualquer + ">";
    const std::string reData = "<DT[+-][0-9]+>";

    std::string resto = frase;
    std::string resultado;
    while (!resto.empty()) {
        // three boost::regex compilations + searches per iteration
        const TIntervalo label = Procura(reLabel, resto);
        const TIntervalo texto = Procura(reTexto, resto);
        const TIntervalo data  = Procura(reData, resto);

        const TIntervalo& primeiro = label.first < texto.first ? label : texto;   // unsigned compare
        const TIntervalo& proximo  = primeiro.first < data.first ? primeiro : data;
        const auto [inicio, fim] = proximo;

        const std::string antes = resto.substr(0, std::min(inicio, resto.size()));
        if (inicio == std::string::npos) resto.clear(); else resto.erase(0, inicio);
        const std::size_t tamanho = fim - inicio;
        const std::string token = resto.substr(0, std::min(tamanho, resto.size()));
        if (tamanho == std::string::npos) resto.clear(); else resto.erase(0, tamanho);

        resultado += antes;
        if (token.empty())
            continue;
        const std::string conteudo = token.substr(1, token.size() - 2);   // strip '<' '>'
        if (inicio == label.first && fim == label.second)
            resultado += TraduzLabel(conteudo);
        else if (inicio == texto.first && fim == texto.second)
            resultado += TraduzTexto(conteudo);
        else
            resultado += TraduzData(conteudo);
    }
    return resultado;
}

}  // namespace comum::md

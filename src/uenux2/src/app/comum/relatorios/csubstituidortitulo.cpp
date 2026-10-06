// uenux2/src/app/comum/relatorios/csubstituidortitulo.cpp
// Reconstructed from vota_web_wasm.wasm (unit u25).
//
// srcloc records: :21 static CSubstituidorTitulo& GetInst()   :26 static void CreateInst(const std::map&)
// Both only survive inlined into wasm func 3689, which the tools therefore named "GetInst".
// Not executed in the recorded votes (no report is printed by the web page).
//
// Rendered example (samples/bu-real/run-full/reports/bu.txt): the -pu.dat cabecalho gives
//     "Justiça Eleitoral" / "Tribunal Regional Eleitoral [AC]"
// and the BU footer (rodapeBUVOTA, CConfiguracaoEleicao +212, through wasm 5584) gives
//     "O conteúdo deste BU poderá ser" / "conferido no endereço" / "resultados.tse.jus.br"

#include "comum/relatorios/csubstituidortitulo.h"

#include <memory>
#include <mutex>

#include "api/pattern/epatternerr.h"
#include "api/util/cstringutils.h"
#include "comum/dados/md/ctradutorfrase.h"
#include "comum/relatorios/relatoriosdefs.h"

namespace comum {

namespace {
std::mutex s_mutex;                                           // @1839116
std::unique_ptr<CSubstituidorTitulo> s_instancia;             // @1839140 (atexit reset = wasm 11175)
} // namespace

// srcloc :21 (inlined into 3689)
CSubstituidorTitulo& CSubstituidorTitulo::GetInst()
{
    std::lock_guard lock(s_mutex);
    if (!s_instancia)
        throw ecourna::api::exception::CBaseError<ecourna::api::pattern::EPatternErr>(
            ecourna::api::pattern::EPatternErr{1303}, "CSubstituidorTitulo - instancia nao criada");   // :21
    return *s_instancia;
}

// srcloc :26 (inlined into 3689)
void CSubstituidorTitulo::CreateInst(const std::map<std::string, std::string>& substituicoes)
{
    std::lock_guard lock(s_mutex);
    if (s_instancia)
        throw CRelatoriosError(EUeComumRelatoriosError{9088}, "Instância já criada");               // :26
    s_instancia.reset(new CSubstituidorTitulo(substituicoes));   // map copy: __emplace_hint_unique (func 1221)
}

bool CSubstituidorTitulo::Existe()                                                   // inlined, name inferred
{
    std::lock_guard lock(s_mutex);
    return s_instancia != nullptr;
}

// inlined into 3689: every tag of the map is replaced, in key order.                  name inferred
std::string CSubstituidorTitulo::Substitui(std::string texto) const
{
    for (const auto& [tag, valor] : m_substituicoes)
        texto = api::CStringUtils::ReplaceAll(texto, tag, valor);                  // func 2677
    return texto;
}

// wasm func 5576: the map's __tree::destroy(root) (shared_f720). Also reached from the atexit
// destructor of s_instancia (wasm 11175: p = s_instancia.release(); p->~CSubstituidorTitulo(); free(p)).
CSubstituidorTitulo::~CSubstituidorTitulo() = default;

// wasm func 3689 (tools: comum::CSubstituidorTitulo::GetInst). Callers: CRelUtil::IncluiCabecalhoEleicoesMZS
// (1543, list = PU cabecalho, CConfiguracaoEleicao +188), vota_f5579 (zerésima trailer, rodapeZEVOTA +200)
// and comum_f5584 (= IncluiTitulos(b, lista, CLocal::GetInst().GetUF())), which is called by vota::CGeraBU
// (12110, BU footer rodapeBUVOTA +212) and by the ESTADO DA URNA template vota_f5591 (its header, hook [11]).
// NOTE: the UF captured on the FIRST call is kept for the lifetime of the process.
void CSubstituidorTitulo::IncluiTitulos(api::CPaperFormBuilder& b,
                                        const std::vector<ecourna::app::dados::CTituloRelatorio>& titulos,
                                        const std::string& uf)
{
    using ecourna::app::dados::CTituloRelatorio;

    if (!Existe()) {
        std::map<std::string, std::string> substituicoes;
        substituicoes["<uf>"] = uf;
        CreateInst(substituicoes);                                                   // :26
    }
    const CSubstituidorTitulo& substituidor = GetInst();                            // :21

    for (const CTituloRelatorio& titulo : titulos) {                                // 20-byte items
        // The text may hold several lines; a trailing "\n" does not produce an extra blank line.
        std::vector<std::string> linhas =
            api::CStringUtils::Split(md::CTradutorFrase::TraduzLabel(titulo.GetTexto()), '\n');   // 654, 1880
        if (!linhas.empty() && linhas.back().empty())
            linhas.pop_back();

        for (const std::string& linha : linhas) {
            if (linha.empty()) {
                b.AddNewLine(1);
                continue;
            }
            // CTituloRelatorio enums are 0-based (unit u14): Normal 0 / Expandido 1; Esquerdo 0 / Direito 1 /
            // Centro 2. wasm: fonte = select(2, 1, estilo == 1); alinhamento = alinhamento == 2 ? 2 : (== 1).
            const int fonte = titulo.GetEstilo() == CTituloRelatorio::Expandido ? 2 : 1;   // +4
            int alinhamento = 0;                                                    // esquerda
            switch (titulo.GetAlinhamento()) {                                      // +0
            case CTituloRelatorio::Centro:  alinhamento = 2; break;                 // value 2
            case CTituloRelatorio::Direito: alinhamento = 1; break;                 // value 1
            default: break;
            }
            b.AddText(substituidor.Substitui(linha), fonte, alinhamento);
        }
    }
}

} // namespace comum

// uenux2/src/app/comum/dados/cpartidos.cpp  (path inferred: class comum::CPartidos, next to its relatives
// CCargos / CEleitores / CRespostas in comum/dados)  --  FRAGMENT written by unit u36.
// Reconstructed from vota_web_wasm.wasm.
//
// comum::CPartidos = api::CDataMap<TPartidoID (uint16), md::CPartido>("CPartidos"): the parties of the
// pleito with a cursor ("current" party). 28 bytes; constructor wasm 3751 (empty map + name "CPartidos"),
// destructor ecourna_f2815. Lazy singleton: std::unique_ptr @1838928, mutex residue @1838904
// (GetInst = wasm 819; atexit handlers 11502 / 11503, other unit). md::CPartido: +0 TPartidoID número
// (uint16: 11499 reads it with i32.load16_u), +4 std::string sigla, +16 std::string nome.
//
// The two text sources below are passed as plain function pointers to api::CDataTextFmt and evaluated at
// draw time; the current party is the one selected by the voter-screen code (CPartidos::SetCurrent).
// Wrapper type: api::CDataTextFmt<const std::string (*)(const std::string&)> (vtable @1538280), hence the
// `const std::string` return type.
#include <format>
#include <string>

namespace comum {

// wasm func 11499 (table slot 1100) - observed executing                  // class and method names inferred
// The party NUMBER on the party screen of a proportional vote (vota::CTelasVota::CriaTelaPartido,
// ctelasvota.cpp:1741, built at start-up by 7787: CDataTextFmt(slot 1100, "{}")), shown while the voter has
// typed only the party's digits (e.g. "91" of "91001") or votes for the party only (voto de legenda).
const std::string CPartidosDSNumero::Text(const std::string& formato)
{
    const md::CPartido* partido = CPartidos::GetInst().GetCurrent();        // wasm 1283 (cdatamap.h:98, throws
                                                                            // when the cursor is not on a record)
    return std::vformat(formato, std::make_format_args(partido->GetNumero()));   // uint16, stored as an int arg (type 3)
}

// wasm func 11501 (table slot 1089)                                       // class and method names inferred
// The party SIGLA (acronym) of the candidate on screen: the "partido" line added by vota_f2028
// (adicionaPartido, ctelasvota.cpp) when CConfiguracaoEleicao +484 ("apresentar partido") is set, format "{}".
// Checked with `node tools/run/headless.mjs --scenario municipal-t1 --keys "91001 " --draw`: the candidate
// screen draws fillText("PEsp", 20, 451) - "PEsp" is the sigla of party 91 in t02411ac00001-pa.dat
// ("Partido dos Esportes" is its name, CPartido +16). Not caught by the sampling profiler; an entry counter
// (review run) gives 8..20 calls per voter in every scenario, municipal-t2 (Prefeito only) included.
const std::string CPartidosDSSigla::Text(const std::string& formato)
{
    const md::CPartido* partido = CPartidos::GetInst().GetCurrent();
    return std::vformat(formato, std::make_format_args(partido->GetSigla()));   // string_view of +4
}

}  // namespace comum

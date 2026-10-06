// uenux2/src/app/comum/... (file unknown)  --  FRAGMENT written by unit u02
//
// wasm func 3942 (comum_f3942): body shared by several "return a transformed copy of a string"
// helpers, produced by wasm-opt's merge-similar-functions: the transform is passed as a function-table
// slot and called through invoke_vi. Instances:
//   wasm 5156 -> slot 2920 = wasm 3509: accent-aware upper-casing in place (Latin-1 table
//                "ÇÁÉÍÓÚÀÈÌÒÙÂÊÎÔÛÃÕ..." @1118052, built once, flag @1911832). Used by votaInit and
//                by CTelasVota::CriaTelaConfirmaImpressaoZeresima (wasm 6592).
//   wasm 1374 -> slot 6416, wasm 1879 -> slot 6425 (other in-place transforms, not identified here).

namespace comum {

// wasm func 3942                                                       // name inferred
std::string CopiaTransformada(const std::string& texto, void (*transforma)(std::string&))
{
    std::string copia = texto;
    transforma(copia);
    return copia;
}

// wasm func 5156 (not in this unit, listed for context)
std::string ParaMaiusculas(const std::string& texto) { return CopiaTransformada(texto, &ParaMaiusculasInPlace); }

} // namespace comum

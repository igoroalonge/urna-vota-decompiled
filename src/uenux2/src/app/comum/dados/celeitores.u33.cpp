// uenux2/src/app/comum/dados/celeitores.cpp  -- FRAGMENT written by unit u33 (srcloc celeitores.cpp exists;
// the file is reconstructed by unit u05, which calls this helper "MontaCaminhos").
#include <string>
#include <vector>

namespace comum {
namespace {

// wasm func 1936                                                                    // name inferred (u05)
// {diretorio + nome for each nome}: full paths of the voter-roll files listed in the election data
// (*-el.dat, *-tte.dat, the impedidos files), relative to the static data directory of the urna.
// Callers: CEleitores::CompleteLoad (6734), CEleitores::GetEleitoresEstaticos (5772, twice) and the start-up
// code inlined in func 7787 (three times: +64, +76, +88 lists).
std::vector<std::string> MontaCaminhos(const std::string& diretorio, const std::vector<std::string>& arquivos)
{
    std::vector<std::string> caminhos;
    caminhos.reserve(arquivos.size());
    for (const std::string& nome : arquivos)
        caminhos.push_back(diretorio + nome);            // operator+ inlined; push_back(&&) = func 376
    return caminhos;
}

} // namespace
} // namespace comum

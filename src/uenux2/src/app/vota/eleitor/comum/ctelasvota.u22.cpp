// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp  --  FRAGMENT written by unit u22
// wasm func 12587 was assigned to u22 because it calls CCargo accessors; its owner is ctelasvota.cpp
// (see u07/u02: DS_NomeCargoNeutroComEscolha is built by func 6627 and used by adicionaNomeCargo,
// func 1192). Merge there.
#include "vota/eleitor/comum/ctelasvota.h"

#include "comum/dados/md/processoeleitoral/ccargo.h"

namespace vota {
namespace {

// Data source of the office name at the top of every voting screen. Keeps a copy of the CCargo
// (the api::CDataText<DS> object stores it at +8; qtdEscolhas lands at +21).
struct DS_NomeCargoNeutroComEscolha {
    comum::md::CCargo cargo;

    // wasm func 12587 = api::CDataText<DS_NomeCargoNeutroComEscolha> vtable slot 2 (the text getter),
    // with operator() inlined. Observed executing. "Prefeito", "Senador - 1ª vaga", ...
    std::string operator()() const
    {
        const std::string nome = cargo.GetNome();                                   // func 1547 (neutral form)
        if (cargo.GetQtdEscolhas() == 1)
            return nome;
        return nome + " - " + cargo.GetOrdinalEscolha(g_numeroEscolha);            // func 2258; g_numeroEscolha = byte @1536340 (u06)
    }
};

} // namespace
} // namespace vota

// uenux2/src/app/comum/dados/asn/rdv/cconversoreleicoesvota.h
// Reconstructed from vota_web_wasm.wasm (unit u03). Conventions: see asn/rdv/cconversorvoto.h.
#pragma once

#include <map>
#include <vector>

#include "comum/asn/iconversorasn.h"
#include "comum/dados/md/processoeleitoral/ccargo.h"
#include "comum/dados/md/rdv/cvotoseleicoesvota.h"
#include "ModuloRegistroDigitalVoto.h"

namespace comum::asn {

// RTTI: comum::asn::CConversorEleicoesVota
//         : IConversorASN<ModuloRegistroDigitalVoto::Eleicoes, comum::md::CVotosEleicoesVota>
// vtable @1571224: [0] 11348 ~CConversorEleicoesVota [1] 11347 deleting dtor [2] 11350 DoConverte
//                  [3] 11349 DoDesconverte
// Owned (at +4) by comum::asn::CConversorRegistroDigitalVoto<CConversorEleicoesVota>, which builds it by
// moving in the map of configured cargos per eleição (see func 7787 around the vtable store).
class CConversorEleicoesVota
    : public IConversorASN<ModuloRegistroDigitalVoto::Eleicoes, md::CVotosEleicoesVota>
{
public:
    using TMapaCargos = std::map<TEleicaoID, std::vector<md::CCargo>>;   // md::CCargo is 140 bytes   // name inferred

    explicit CConversorEleicoesVota(TMapaCargos cargos) : m_cargos(std::move(cargos)) {}
    ~CConversorEleicoesVota() override = default;   // wasm func 11348 (complete), 11347 (deleting)

protected:
    TEntidade DoConverte(const TDado& votos) const override;        // wasm func 11350
    TDado DoDesconverte(const TEntidade& eleicoes) const override;  // wasm func 11349

private:
    TMapaCargos m_cargos;   // +4 (begin node), +8 (root), +12 (size)
};

} // namespace comum::asn

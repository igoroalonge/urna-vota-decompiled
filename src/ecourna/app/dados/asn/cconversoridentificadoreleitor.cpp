// ecourna-lib/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/asn/cconversoridentificadoreleitor.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
//   IdentificadorEleitor ::= CHOICE { numeroInscricao [0] NumericString (SIZE(12)),
//                                     numeroCPF       [1] NumericString (SIZE(11)),
//                                     identificacaoLivre [2] NumericString (SIZE(12)) }
//   <-> CIdentificadorEleitor (a shared_ptr to one of the three IIdentificadorEleitor classes)
#include <memory>

#include "ecourna/app/dados/asn/cconversores.h"
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"
#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados::asn {

// wasm func 9117 (vtable slot 2)
// No default branch: an identifier of any other type leaves the CHOICE unselected (rejected later
// by the validity check of Converte). Each alternative is created through the create function of
// its NumericString info table (@1913336, @1913380, @1913424) and gets a copy of GetNumero()
// (the unpadded/padded stored number, not the formatted one).
CConversorIdentificadorEleitor::TEntidade CConversorIdentificadorEleitor::DoConverte(const TDado& identificador) const
{
    ModuloTiposEleitorais::IdentificadorEleitor entidade;
    switch (identificador->GetTipo()) {
    case TipoIDNumeroInscricao: entidade.select_numeroInscricao() = identificador->GetNumero(); break;
    case TipoIDCPF:             entidade.select_numeroCPF() = identificador->GetNumero(); break;
    case TipoIDNumeroLivre:     entidade.select_identificacaoLivre() = identificador->GetNumero(); break;
    }
    return entidade;
}

// wasm func 9116 (vtable slot 3; srcloc line 47)
CConversorIdentificadorEleitor::TDado CConversorIdentificadorEleitor::DoDeconverte(const TEntidade& identificador) const
{
    TTipoIdentificadorEleitor tipo;       // CBaseType constructor func 2666 (range 1..3)
    const std::string* numero = nullptr;
    switch (identificador.currentSelection()) {
    case ModuloTiposEleitorais::IdentificadorEleitor::numeroInscricao_id:
        tipo = TTipoIdentificadorEleitor(TipoIDNumeroInscricao);
        numero = &identificador.get_numeroInscricao().getValue();
        break;
    case ModuloTiposEleitorais::IdentificadorEleitor::numeroCPF_id:
        tipo = TTipoIdentificadorEleitor(TipoIDCPF);
        numero = &identificador.get_numeroCPF().getValue();
        break;
    case ModuloTiposEleitorais::IdentificadorEleitor::identificacaoLivre_id:
        tipo = TTipoIdentificadorEleitor(TipoIDNumeroLivre);
        numero = &identificador.get_identificacaoLivre().getValue();
        break;
    default:
        throw CAsnError(2268, "Identificador do eleitor não definido corretamente.");   // line 47
    }

    switch (tipo) {
    case TipoIDNumeroInscricao: return std::make_shared<CNumeroInscricaoEleitoral>(*numero);   // func 1241
    case TipoIDCPF:             return std::make_shared<CNumeroCPF>(*numero);                  // func 5107
    case TipoIDNumeroLivre:     return std::make_shared<CNumeroIdentificacaoLivre>(*numero);   // func 5106
    }
    return {};   // unreachable in practice: empty shared_ptr
}

} // namespace ecourna::app::dados::asn

// uenux2/src/app/comum/asn/cconversorabrangencia.cpp
// Reconstructed from vota_web_wasm.wasm (unit u21).
#include "comum/asn/cconversorabrangencia.h"

#include <format>
#include <string>
#include <utility>

#include "comum/asn/util.h"

namespace comum::asn {

// wasm func 11462 (vtable slot 2; srclocs util.cpp:513 (inlined Utils::ConverteAbrangencia) and line 45).
// Not observed executing (the voting application does not write Abrangencia except inside party/candidate
// entities it never writes; see CConversorEntidadePartidos::DoConverte).
ModuloTiposEleitorais::Abrangencia CConversorAbrangencia::DoConverte(const md::CAbrangencia& abrangencia) const
{
    ModuloTiposEleitorais::Abrangencia entidade;
    entidade.set_tipo(Utils::ConverteAbrangencia(abrangencia.GetTipo()));    // throws 7685 when tipo >= 3

    switch (abrangencia.GetTipo()) {
    case md::ETipoAbrangencia::Municipal: {                                   // 0
        ModuloTiposEleitorais::IdentificacaoAbrangencia id;
        id.set_siglaUF(abrangencia.GetUF());
        id.set_codigoMunicipio(abrangencia.GetMunicipio());                   // includeOptionalField(0, 1)
        entidade.set_id(id);                                                  // includeOptionalField(0, 1)
        break;
    }
    case md::ETipoAbrangencia::Estadual: {                                    // 1
        ModuloTiposEleitorais::IdentificacaoAbrangencia id;
        id.set_siglaUF(abrangencia.GetUF());
        id.omit_codigoMunicipio();                                            // removeOptionalField(0)
        entidade.set_id(id);
        break;
    }
    case md::ETipoAbrangencia::Federal:                                       // 2
        entidade.omit_id();
        break;
    default:   // unreachable in practice: ConverteAbrangencia above already threw 7685 for tipo >= 3
        throw CUeComumAsnError(EUeComumAsnError(7650),
                               std::format("Tipo inválido [{}]", std::to_underlying(abrangencia.GetTipo())));  // line 45
    }
    return entidade;
}

// wasm func 11461 (vtable slot 3). Observed executing (parties / candidates files are loaded at votaInit).
// The UF is copied through its C string (strlen), not as a std::string. A missing municipality is passed as 0;
// md::CAbrangencia::CAbrangencia (func 3739, cabrangencia.cpp:31..51) then checks the combination
// ("UF não informada para abrangência não federal", "Município zerado para abrangência municipal", ...).
md::CAbrangencia CConversorAbrangencia::DoDesconverte(const ModuloTiposEleitorais::Abrangencia& abrangencia) const
{
    const md::ETipoAbrangencia tipo = Utils::DesconverteAbrangencia(abrangencia.get_tipo());   // func 5827
    if (!abrangencia.id_isPresent()) {
        return md::CAbrangencia(tipo, std::string(), 0);
    }
    const auto& id = abrangencia.get_id();
    if (!id.codigoMunicipio_isPresent()) {
        return md::CAbrangencia(tipo, std::string(id.get_siglaUF().c_str()), 0);
    }
    return md::CAbrangencia(tipo, std::string(id.get_siglaUF().c_str()), id.get_codigoMunicipio());
}

} // namespace comum::asn

// FRAGMENTS reconstructed by unit u24 from vota_web_wasm.wasm.
// These functions were assigned to unit u24 but belong to source files owned by other units; each
// block names its original file (path inferred when marked).

// =====================================================================================================
// uenux2/src/app/comum/carquivosresultado.cpp  (file of unit u21)
// =====================================================================================================
#include "comum/carquivosresultado.h"

#include <memory>
#include <mutex>

namespace comum {

namespace {
std::mutex                          s_mutexArquivosResultado;   // @1838492
std::unique_ptr<CArquivosResultado> s_arquivosResultado;         // @1838516 (atexit reset = wasm 11660)
} // namespace

// wasm func 348 (tools: comum_f348) - observed executing (CArquivosSavd construction, func 1164).
// Lazy singleton of an empty class: operator new(1). The body is merged (merge-similar-functions) with
// the other empty singletons as func 2900(mutex, unique_ptr): 948 CLogComum::GetInst, 2742 (WSQ codec),
// 3603 (helper of CConversorEntidadeBU). Callers: 1164, CGravadorUtil::DeterminaNomeArquivoSemLetra (3798),
// CLogComum::LogaGerandoResultados (5876), the merged CLogComum body 6045.
const CArquivosResultado& CArquivosResultado::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutexArquivosResultado);   // lock is a no-op; unlock residue = func 150
    if (!s_arquivosResultado)
        s_arquivosResultado.reset(new CArquivosResultado());
    return *s_arquivosResultado;
}

} // namespace comum

// =====================================================================================================
// uenux2/src/app/comum/asn/cconversorseguranca.cpp  (path inferred from the class name; the converters of
// comum/asn are unit u21; slot 2 DoConverte = wasm 11413 is unit u35)
// RTTI: comum::asn::CConversorSeguranca : comum::asn::IConversorASN<ModuloTiposEleitorais::Seguranca,
// comum::md::CSeguranca>; vtable @1566572 = {174, 144, 11413 DoConverte, 11412 DoDesconverte}.
// =====================================================================================================
#include "comum/asn/iconversorasn.h"
#include "comum/md/cseguranca.h"
#include "ModuloTiposEleitorais.h"

namespace comum::asn {

class CConversorSeguranca : public IConversorASN<ModuloTiposEleitorais::Seguranca, md::CSeguranca> {
protected:
    TEntidade DoConverte(const TDado& seguranca) const override;          // wasm 11413 (unit u35)
    md::CSeguranca DoDesconverte(const TEntidade& seguranca) const override;   // wasm 11412
};

// wasm func 11412 - vtable slot 3 (curated name correct). Reads fields 0, 1 and 3 of the SEQUENCE;
// idArquivoCD (field 2) is ignored. The public wrapper IConversorASN<...>::Desconverte is wasm 3730.
md::CSeguranca CConversorSeguranca::DoDesconverte(const ModuloTiposEleitorais::Seguranca& seguranca) const
{
    const std::vector<uebyte> chave(seguranca.get_idArquivoChave().begin(),
                                    seguranca.get_idArquivoChave().end());
    return md::CSeguranca(static_cast<uebyte>(seguranca.get_idTipoArquivo()),
                          static_cast<uebyte>(seguranca.get_idCriptografia()), chave);   // func 3823
}

// For reference, wasm 11413 (slot 2, unit u35): builds the SEQUENCE (info @1141080), sets idTipoArquivo
// and idCriptografia from the two bytes and copies the key into idArquivoChave; idArquivoCD keeps 0.

} // namespace comum::asn

// =====================================================================================================
// Destructors that only exist as 12-byte thunks into the merged body wasm 2899 (see cjustificador.cpp):
// both classes are {vptr, std::shared_ptr<...>} report parts (api::IReportPart).
// =====================================================================================================
namespace api {
// uenux2/src/api/gui/cformpart.cpp (path inferred). vtable @1583696 = {10888, 10887 (D0), 10889 Imprime}.
// wasm func 10888 - slot 0: comum_f2899(this, vtable @1583696) = ~CFormPart(): releases the shared form.
CFormPart::~CFormPart() = default;
} // namespace api

namespace comum {
// uenux2/src/app/comum/relatorios/cparteeleitores.cpp (path inferred). vtable @1576596 = {11205, 11204 (D0),
// 11206 Imprime}.
// wasm func 11205 - slot 0: comum_f2899(this, vtable @1576596) = ~CParteEleitores().
CParteEleitores::~CParteEleitores() = default;
} // namespace comum

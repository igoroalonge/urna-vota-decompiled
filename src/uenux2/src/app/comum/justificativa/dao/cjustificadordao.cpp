// uenux2/src/app/comum/justificativa/dao/cjustificadordao.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// SQLite persistence of the justifications ("justificativas") typed at this urna. One row per voter:
//   CREATE TABLE IF NOT EXISTS registro_justificativa ( numero_titulo BIGINT PRIMARY KEY UNIQUE NOT NULL,
//                                                       ano_nascimento SMALLINT )
// in <trab>/uenux.db (MI = internal flash; the file is signed and copied to the external flash after each
// write by the caller, see CJustificador::SaveCurrentInternal).
//
// ecourna::api::sql vtable slots used (unit u13): ISqlConnection::Prepare = 5;
// ISqlStatement::SetInt = 2, SetInt64 = 3, ExecuteQuery = 10, Close = 12;
// ISqlResultSet::GetInt = 2, GetInt64 = 4, Next = 18.
// All SQL texts start with a blank and end with " \n" exactly as in the binary.
//
// Web build: the table is created at votaInit (it is present in analysis/runtime/memfs-*/dsk/.../uenux.db),
// but no row is ever written: justifications are entered on the mesário terminal, which the public
// simulator does not drive.
#include "comum/justificativa/dao/cjustificadordao.h"

#include <format>

#include "comum/justificativa/cjustificador.h"            // CUeComumJustificativaError
#include "ecourna/api/sql/sqlite/csqlconnection.hpp"
#include "ecourna/api/util/cstringutils.hpp"
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"
#include "ecourna/app/dados/cregistroidentificacaoeleitor.h"

namespace comum::dao {

using ecourna::api::util::CStringUtils;
using ecourna::app::dados::CIdentificacaoJustificativa;
using ecourna::app::dados::CNumeroInscricaoEleitoral;
using ecourna::app::dados::CRegistroIdentificacaoEleitor;

namespace {

// Row -> entity; inlined in Recuperar and RecuperarTodos.
// numero_titulo is read as int64 and printed with "{}" (CNumeroInscricaoEleitoral pads it back to 12
// digits); ano_nascimento is read with GetInt, truncated to 16 bits and range-checked 0..9999 by
// CBaseType<0, 9999> (a NULL year reads as 0).
CIdentificacaoJustificativa LeLinha(ecourna::api::sql::ISqlResultSet& rs)
{
    const std::string titulo = std::format("{}", rs.GetInt64(0));
    const int ano = rs.GetInt(1);
    const CNumeroInscricaoEleitoral numero(titulo);
    return CIdentificacaoJustificativa(                                              // shared_f1875
        CRegistroIdentificacaoEleitor(std::make_shared<CNumeroInscricaoEleitoral>(numero)),   // 1675
        ecourna::app::dados::TAnoNascimento(static_cast<ueword>(ano)));             // CBaseType<0,9999>
}

} // namespace

// Inlined into wasm 7787 (unit u02/u07). Table text @435360.
CJustificadorDAO::CJustificadorDAO(const std::string& caminhoBanco)
    : m_conexao(std::make_shared<ecourna::api::sql::sqlite::CSqlConnection>(caminhoBanco))
{
    auto stmt = m_conexao->Prepare(
        "CREATE TABLE IF NOT EXISTS registro_justificativa ( \n"
        "numero_titulo     BIGINT PRIMARY KEY UNIQUE NOT NULL, \n"
        "ano_nascimento    SMALLINT \n"
        ")");
    stmt->ExecuteQuery();
    stmt->Close();
}

// wasm func 11468 - slot 2. Merged body comum_f3894(this, vtable): new(12) copying the vptr and the
// shared connection (use_count + 1). Used by CDAORepositorio::Entregar to hand out a private copy.
api::persistencia::IDAO* CJustificadorDAO::Clonar() const
{
    return new CJustificadorDAO(*this);
}

// wasm func 11473 - slot 3.
void CJustificadorDAO::Inserir(const CIdentificacaoJustificativa& justificativa) const
{
    auto stmt = m_conexao->Prepare(
        " INSERT INTO registro_justificativa (numero_titulo, ano_nascimento) VALUES (?, ?) \n");
    stmt->SetInt64(1, CStringUtils::ToQWord(
                          justificativa.GetIdentificacao().GetIdentificadorHabilitacao()->GetNumero()));
    stmt->SetInt(2, justificativa.GetAnoNascimento());                            // ushort at +20
    stmt->ExecuteQuery();                                                         // result set discarded
    stmt->Close();
}

// wasm func 11472 - slot 5 (srcloc line 56). Justifications are never deleted.
void CJustificadorDAO::ExcluirID(const std::string& /*titulo*/) const
{
    throw CUeComumJustificativaError(EUeComumJustificativaError{8804},
        "Nao é esperado que o CJustificadorDAO exclua registros do banco");             // line 56
}

// wasm func 11471 - slot 6 (srcloc line 63). Justifications are never updated.
void CJustificadorDAO::Atualizar(const CIdentificacaoJustificativa& /*justificativa*/) const
{
    throw CUeComumJustificativaError(EUeComumJustificativaError{8805},
        "Nao é esperado que o CJustificadorDAO atualize registros do banco");           // line 63
}

// wasm func 11470 - slot 7. Null when the título has no row.
std::shared_ptr<CIdentificacaoJustificativa> CJustificadorDAO::Recuperar(const std::string& titulo) const
{
    auto stmt = m_conexao->Prepare(
        " SELECT numero_titulo, ano_nascimento FROM registro_justificativa WHERE numero_titulo = ? \n");
    stmt->SetInt64(1, CStringUtils::ToQWord(titulo));
    auto rs = stmt->ExecuteQuery();
    std::shared_ptr<CIdentificacaoJustificativa> resultado;
    if (rs->Next()) {
        const CIdentificacaoJustificativa linha = LeLinha(*rs);
        resultado = std::shared_ptr<CIdentificacaoJustificativa>(                     // __shared_ptr_pointer
            new CIdentificacaoJustificativa(linha));                                   // (not make_shared)
    }
    stmt->Close();
    return resultado;
}

// wasm func 11469 - slot 8. Called through comum_f5723 (m_servico -> DAO slot 8) by
// comum::CEleitores::CompleteLoad (6734, celeitores.cpp:205..238; called by 7787 CarregarDadosEstaticos and
// 11865 CGeraDadosDinamicos::StartState), which clears CJustificador (tree destroy 2809, cursor = end())
// and re-fills it with CDataMap::Add per row. unknown_f5842 also relocates this vector type (jufa.dat writer?).
std::vector<CIdentificacaoJustificativa> CJustificadorDAO::RecuperarTodos() const
{
    auto stmt = m_conexao->Prepare(" SELECT numero_titulo, ano_nascimento FROM registro_justificativa \n");
    auto rs = stmt->ExecuteQuery();
    std::vector<CIdentificacaoJustificativa> justificativas;
    while (rs->Next())
        justificativas.push_back(LeLinha(*rs));       // growth path: wasm 5831
    stmt->Close();
    return justificativas;
}

// wasm func 5831 (tools: comum_f5831) - library: std::__uninitialized_allocator_relocate for
// std::vector<CIdentificacaoJustificativa> (24-byte elements: two shared_ptrs of the
// CRegistroIdentificacaoEleitor, the optional's flag at +16, the year at +20). Move-constructs
// [first, last) into the new buffer, then destroys the moved-from elements. Callers: 11469, 5842.

} // namespace comum::dao

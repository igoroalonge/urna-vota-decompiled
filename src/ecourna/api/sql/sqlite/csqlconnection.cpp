// ecourna-lib/ecourna/api/sql/sqlite/csqlconnection.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/sql/sqlite/csqlconnection.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13. SQLite 3.50.4 is linked statically (see docs/libraries/sqlite.md).
//
// All string literals are stored in Latin-1 in the binary (the TSE code is compiled with a Latin-1 execution
// character set); they are written here in UTF-8.
#include "ecourna/api/sql/sqlite/csqlconnection.hpp"

#include <sqlite3.h>

#include <format>
#include <memory>
#include <string>

#include "ecourna/api/sql/esqlerror.hpp"
#include "ecourna/api/sql/sqlite/csqlstatement.hpp"

namespace ecourna::api::sql::sqlite {

// wasm func 9411 - name inferred, original file unknown (see the header).
std::string GetErrorMessage(sqlite3* db)
{
    if (const char* mensagem = sqlite3_errmsg(db))
        return mensagem;
    return "Erro desconhecido do SQLite.";
}

// wasm func 3515 (srcloc lines 29 and 44). Observed executing (votaInit creates uenux.db through the DAOs).
CSqlConnection::CSqlConnection(const std::string& arquivo)
    : m_arquivo(arquivo)
{
    // Default flags (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE): a missing file is silently created.
    // The path passed is the ARGUMENT's buffer (the wasm reads b[0]/b[11]), not the member copy.
    if (sqlite3_open(arquivo.c_str(), &m_db) != SQLITE_OK) {
        const std::string erro = GetErrorMessage(m_db);
        sqlite3_close(m_db);
        throw CSqlError(ESqlError::AbrirBanco, std::string(erro));                    // line 29
    }
    m_fechada = false;

    try {
        auto comando = Prepare("PRAGMA foreign_keys = ON");
        comando->Execute();     // the result set steps once in its constructor: the pragma runs here
        comando->Close();
    } catch (...) {
        // The caught exception is discarded; the new one carries SQLite's current error text.
        const std::string erro = GetErrorMessage(m_db);
        sqlite3_close(m_db);
        m_fechada = true;
        throw CSqlError(ESqlError::ConfigurarBanco, std::string(erro));                // line 44
    }
}

// wasm func 5167 (complete destructor; also reached from shared_ptr's __on_zero_shared, func 11539)
// wasm func 9456 (deleting destructor = the same body + operator delete)
CSqlConnection::~CSqlConnection()
{
    if (!m_fechada) {
        try {
            Close();
        } catch (...) {
            // swallowed (e.g. SQLITE_BUSY because a statement was not finalized: the handle then leaks)
        }
    }
}

// wasm func 9455 - name inferred
void CSqlConnection::BeginTransaction()
{
    auto comando = Prepare("begin transaction");
    comando->Execute();
    comando->Close();
}

// wasm func 9454 - name inferred
void CSqlConnection::Commit()
{
    auto comando = Prepare("commit transaction");
    comando->Execute();
    comando->Close();
}

// wasm func 9453 - name inferred
void CSqlConnection::Rollback()
{
    auto comando = Prepare("rollback transaction");
    comando->Execute();
    comando->Close();
}

// wasm func 9460 (srcloc line 86). Observed executing.
// Neither m_fechada nor m_db is checked: preparing on a closed connection uses a freed sqlite3 handle.
TSharedSqlStatement CSqlConnection::Prepare(const std::string& sql)
{
    sqlite3_stmt* comando = nullptr;
    if (sqlite3_prepare_v2(m_db, sql.c_str(), -1, &comando, nullptr) != SQLITE_OK) {
        throw CSqlError(ESqlError::Preparar,                                           // line 86
                        std::format("Falha ao executar operação no banco de dados.\n"
                                    "Mensagem do banco de dados: '{}'\n"
                                    "Operação: '{}'",
                                    GetErrorMessage(m_db), sql));
    }
    return TSharedSqlStatement(new CSqlStatement(m_db, comando));   // CSqlStatement ctor = func 9430
}

// wasm func 9458 (srcloc line 103)
// m_db is not reset: the dangling handle stays in the object (only m_fechada protects the destructor).
void CSqlConnection::Close()
{
    if (sqlite3_close(m_db) != SQLITE_OK)
        throw CSqlError(ESqlError::FecharBanco, GetErrorMessage(m_db));               // line 103
    m_fechada = true;
}

// wasm func 9452 - name inferred. Not observed executing and no direct caller (virtual only).
// NOTE: the result row of "PRAGMA integrity_check(1)" ("ok" or the first problem found) is never read.
// The function only reports whether preparing and stepping the pragma threw, so a database whose
// integrity check reports a problem as a normal result row is still reported as healthy.
bool CSqlConnection::CheckIntegrity()
{
    try {
        auto comando = Prepare("PRAGMA integrity_check(1)");
        comando->Execute();
        comando->Close();
        return true;
    } catch (...) {
        return false;
    }
}

} // namespace ecourna::api::sql::sqlite

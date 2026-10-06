// ecourna-lib/ecourna/api/sql/sqlite/csqlconnection.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: CSqlConnection : ISqlConnection : pattern::NonCopyable   (typeinfo @1116148, vtable @1115936)
// Objects are 24 bytes (operator new(24) in every creator: comum::dao::CEleitorDinamicoDAO ctor (func 3768)
// and the DAO constructors inlined in func 7787, which open /dsk/fi/dinamico/trab1/uenux.db and friends)
// and are owned by a std::shared_ptr<ISqlConnection> (control block vtable @1559008).
#pragma once

#include <sqlite3.h>

#include <string>

#include "ecourna/api/sql/isqlconnection.hpp"

namespace ecourna::api::sql::sqlite {

// wasm func 9411 - name inferred, file unknown (shared by the three sqlite/*.cpp files; LTO kept a
// single copy). sqlite3_errmsg(db), or "Erro desconhecido do SQLite." when SQLite returns NULL.
std::string GetErrorMessage(sqlite3* db);

class CSqlConnection : public ISqlConnection {
public:
    explicit CSqlConnection(const std::string& arquivo);   // wasm func 3515 (srcloc lines 29, 44)
    ~CSqlConnection() override;                             // wasm func 5167 (complete) / 9456 (deleting)

    void BeginTransaction() override;                       // wasm func 9455   name inferred
    void Commit() override;                                 // wasm func 9454   name inferred
    void Rollback() override;                               // wasm func 9453   name inferred
    TSharedSqlStatement Prepare(const std::string& sql) override;   // wasm func 9460 (srcloc line 86)
    bool IsClosed() const override { return m_fechada; }    // wasm func 9457   name inferred
    void Close() override;                                  // wasm func 9458 (srcloc line 103)
    bool CheckIntegrity() override;                         // wasm func 9452   name inferred
    const std::string& GetPath() const override { return m_arquivo; }   // wasm func 5322 (ICF "return this+4")

private:
    std::string m_arquivo;        // +4   path of the database file
    sqlite3*    m_db{nullptr};    // +16
    bool        m_fechada{true};  // +20  "closed"; false only between a successful open and Close()
};                                // sizeof 24

} // namespace ecourna::api::sql::sqlite

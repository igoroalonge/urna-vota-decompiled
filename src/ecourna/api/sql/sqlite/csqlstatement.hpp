// ecourna-lib/ecourna/api/sql/sqlite/csqlstatement.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: CSqlStatement : ISqlStatement   (typeinfo @1117148, vtable @1116948). 20-byte objects created only by
// CSqlConnection::Prepare (operator new(20)) and owned by std::shared_ptr<ISqlStatement>
// (control block vtable @1116292: __on_zero_shared = func 9451, which runs func 3513 then free).
#pragma once

#include <sqlite3.h>

#include <string>
#include <vector>

#include "ecourna/api/sql/isqlconnection.hpp"

namespace ecourna::api::sql::sqlite {

class CSqlStatement : public ISqlStatement {
public:
    CSqlStatement(sqlite3* db, sqlite3_stmt* comando);   // wasm func 9430 (name inferred: no srcloc)
    ~CSqlStatement() override;                           // wasm func 3513 (complete) / 9426 (deleting)

    void SetInt(int indice, int valor) override;                                     // wasm func 5160 (line 53)
    void SetInt64(int indice, ueint64 valor) override;                               // wasm func 9425 (line 63)
    void SetDouble(int indice, double valor) override;                               // wasm func 9424 (line 73)
    void SetText(int indice, const std::string& valor) override;                     // wasm func 9423 (line 83)
    void SetDateTime(int indice, const boost::posix_time::ptime& valor) override;    // wasm func 9422
    void SetBool(int indice, bool valor) override;                                   // wasm func 9421
    void SetBlob(int indice, const std::vector<uebyte>& valor) override;             // wasm func 9420 (line 109)
    void SetNull(int indice) override;                                               // wasm func 9419 (line 119)
    TSharedSqlResultSet Execute() override;                                          // wasm func 9417
    void Reset() override;                                                           // wasm func 9416 (line 133)
    void Close() override;                                                           // wasm func 9429 (line 143)
    bool IsClosed() const override { return m_fechado; }                             // wasm func 9428

private:
    void VerifyParameterIndex(int indice) const;                                     // wasm func 1882 (line 158)

    sqlite3*      m_db;                  // +4   (not owned; used only for error messages)
    sqlite3_stmt* m_comando;             // +8   finalized by Close()
    bool          m_fechado{false};      // +12
    int           m_qtdParametros;       // +16  sqlite3_bind_parameter_count(m_comando)
};                                       // sizeof 20

} // namespace ecourna::api::sql::sqlite

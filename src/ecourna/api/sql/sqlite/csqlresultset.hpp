// ecourna-lib/ecourna/api/sql/sqlite/csqlresultset.hpp   (path inferred from the .cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// RTTI: CSqlResultSet : ISqlResultSet   (typeinfo @1116840, vtable @1116620). 32-byte objects created only
// by CSqlStatement::Execute (operator new(32)) and owned by std::shared_ptr<ISqlResultSet>
// (control block vtable @1117256: __on_zero_shared = func 9413).
//
// Protocol: the constructor already steps once. Callers must call Next() before reading a column: the
// first Next() returns the result of that pre-fetched step, later calls step again. Getters throw
// "Chame o método 'Next()' antes de buscar resultados." while the pre-fetched row is still pending.
#pragma once

#include <sqlite3.h>

#include <map>
#include <string>
#include <vector>

#include "ecourna/api/sql/isqlconnection.hpp"

namespace ecourna::api::sql::sqlite {

class CSqlResultSet : public ISqlResultSet {
public:
    CSqlResultSet(sqlite3* db, sqlite3_stmt* comando);    // wasm func 9449 (no srcloc)
    ~CSqlResultSet() override = default;                   // wasm func 9443 (complete) / 9442 (deleting):
                                                           //   both only destroy m_colunas (func 1677)

    int GetInt(int coluna) override;                                         // wasm func 2679 (line 39)
    int GetInt(const std::string& coluna) override;                          // wasm func 9441
    ueint64 GetInt64(int coluna) override;                                   // wasm func 5165 (line 50)
    ueint64 GetInt64(const std::string& coluna) override;                    // wasm func 9440
    double GetDouble(int coluna) override;                                   // wasm func 5164 (line 61)
    double GetDouble(const std::string& coluna) override;                    // wasm func 9439
    std::string GetText(int coluna) override;                                // wasm func 3514 (line 72)
    std::string GetText(const std::string& coluna) override;                 // wasm func 9438
    bool IsNull(int coluna) override;                                        // wasm func 5163 (line 90)
    bool IsNull(const std::string& coluna) override;                         // wasm func 9435
    boost::posix_time::ptime GetDateTime(int coluna) override;               // wasm func 5162
    boost::posix_time::ptime GetDateTime(const std::string& coluna) override;// wasm func 9434
    bool GetBool(int coluna) override;                                       // wasm func 9446
    bool GetBool(const std::string& coluna) override;                        // wasm func 9433
    std::vector<uebyte> GetBlob(int coluna) override;                        // wasm func 5161 (line 128)
    std::vector<uebyte> GetBlob(const std::string& coluna) override;         // wasm func 9432
    bool Next() override;                                                    // wasm func 9444

private:
    void VerifyColumnIndex(int coluna) const;                  // wasm func 1883 (line 143)
    int IndexForColumnName(const std::string& coluna);         // wasm func 1525 (line 153)
    bool StepStatement();                                      // wasm func 5166 (line 185)

    sqlite3*                   m_db;                     // +4   (not owned)
    sqlite3_stmt*              m_comando;                // +8   (not owned; finalized by CSqlStatement::Close)
    bool                       m_temLinha{false};        // +12  result of the step done by the constructor
    bool                       m_aguardandoNext{false};  // +13  true until the first Next()
    int                        m_qtdColunas{0};          // +16  sqlite3_column_count, refreshed on every step
    std::map<std::string, int> m_colunas;                // +20  lower-cased column name -> index
};                                                       // sizeof 32

} // namespace ecourna::api::sql::sqlite

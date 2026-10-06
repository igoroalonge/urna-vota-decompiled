// ecourna-lib/ecourna/api/sql/isqlconnection.hpp, isqlstatement.hpp, isqlresultset.hpp   (paths inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// The three abstract interfaces of the ecourna SQL layer. Only RTTI survives for them
// (typeinfo @1116204 ISqlConnection : pattern::NonCopyable; @1117204 ISqlStatement; @1116896 ISqlResultSet).
// The virtual-slot order comes from the vtables of the only implementations (sql::sqlite::*), and the
// method names from the srcloc records of those implementations, from the SQL each slot runs, and from the
// behaviour. Names marked "name inferred" have no srcloc.
//
// Put in one header here for readability; the original very probably has one header per interface.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/ptime.hpp>

#include "ecourna/api/pattern/noncopyable.hpp"
#include "ecourna/types.hpp"   // uebyte, ueint64 ...

namespace ecourna::api::sql {

class ISqlResultSet;
class ISqlStatement;
using TSharedSqlResultSet = std::shared_ptr<ISqlResultSet>;   // name inferred (cf. TSharedSqlStatement)
using TSharedSqlStatement = std::shared_ptr<ISqlStatement>;   // name from srcloc (Prepare's return type)

// -------------------------------------------------------------------------------------------------
// vtable layout (CSqlConnection @1115936)
//   [0] ~dtor  [1] deleting dtor  [2] BeginTransaction  [3] Commit  [4] Rollback  [5] Prepare
//   [6] IsClosed  [7] Close  [8] CheckIntegrity  [9] GetPath
class ISqlConnection : public pattern::NonCopyable {
public:
    virtual ~ISqlConnection() = default;
    virtual void BeginTransaction() = 0;                                  // name inferred ("begin transaction")
    virtual void Commit() = 0;                                            // name inferred ("commit transaction")
    virtual void Rollback() = 0;                                          // name inferred ("rollback transaction")
    virtual TSharedSqlStatement Prepare(const std::string& sql) = 0;      // srcloc
    virtual bool IsClosed() const = 0;                                    // name inferred
    virtual void Close() = 0;                                             // srcloc
    virtual bool CheckIntegrity() = 0;                                    // name inferred ("PRAGMA integrity_check(1)")
    virtual const std::string& GetPath() const = 0;                       // name inferred (returns this+4)
};

// -------------------------------------------------------------------------------------------------
// vtable layout (CSqlStatement @1116948)
//   [0] ~dtor [1] deleting dtor [2] SetInt [3] SetInt64 [4] SetDouble [5] SetText [6] SetDateTime
//   [7] SetBool [8] SetBlob [9] SetNull [10] Execute [11] Reset [12] Close [13] IsClosed
// Parameter indices are 1-based (SQLite convention).
class ISqlStatement {
public:
    virtual ~ISqlStatement() = default;
    virtual void SetInt(int indice, int valor) = 0;                                       // srcloc
    virtual void SetInt64(int indice, ueint64 valor) = 0;                                 // srcloc
    virtual void SetDouble(int indice, double valor) = 0;                                 // srcloc
    virtual void SetText(int indice, const std::string& valor) = 0;                       // srcloc
    virtual void SetDateTime(int indice, const boost::posix_time::ptime& valor) = 0;      // name inferred
    virtual void SetBool(int indice, bool valor) = 0;                                     // name inferred
    virtual void SetBlob(int indice, const std::vector<uebyte>& valor) = 0;               // srcloc
    virtual void SetNull(int indice) = 0;                                                 // srcloc
    virtual TSharedSqlResultSet Execute() = 0;                                            // name inferred
    virtual void Reset() = 0;                                                             // srcloc
    virtual void Close() = 0;                                                             // srcloc
    virtual bool IsClosed() const = 0;                                                    // name inferred
};

// -------------------------------------------------------------------------------------------------
// vtable layout (CSqlResultSet @1116620): every getter exists twice, by 0-based column index and by
// column name (the name overload is always the next slot).
//   [0] ~dtor [1] deleting dtor [2/3] GetInt [4/5] GetInt64 [6/7] GetDouble [8/9] GetText [10/11] IsNull
//   [12/13] GetDateTime [14/15] GetBool [16/17] GetBlob [18] Next
class ISqlResultSet {
public:
    virtual ~ISqlResultSet() = default;
    virtual int GetInt(int coluna) = 0;                                                   // srcloc
    virtual int GetInt(const std::string& coluna) = 0;
    virtual ueint64 GetInt64(int coluna) = 0;                                             // srcloc
    virtual ueint64 GetInt64(const std::string& coluna) = 0;
    virtual double GetDouble(int coluna) = 0;                                             // srcloc
    virtual double GetDouble(const std::string& coluna) = 0;
    virtual std::string GetText(int coluna) = 0;                                          // srcloc
    virtual std::string GetText(const std::string& coluna) = 0;
    virtual bool IsNull(int coluna) = 0;                                                  // srcloc
    virtual bool IsNull(const std::string& coluna) = 0;
    virtual boost::posix_time::ptime GetDateTime(int coluna) = 0;                         // name inferred
    virtual boost::posix_time::ptime GetDateTime(const std::string& coluna) = 0;          // name inferred
    virtual bool GetBool(int coluna) = 0;                                                 // name inferred
    virtual bool GetBool(const std::string& coluna) = 0;                                  // name inferred
    virtual std::vector<uebyte> GetBlob(int coluna) = 0;                                  // srcloc
    virtual std::vector<uebyte> GetBlob(const std::string& coluna) = 0;
    virtual bool Next() = 0;                                                              // name from the error text "Chame o método 'Next()' ..."
};

using TSharedSqlConnection = std::shared_ptr<ISqlConnection>;   // name inferred; the DAOs hold
                                                                // shared_ptr<ISqlConnection> (__shared_ptr_pointer vtable @1559008)

} // namespace ecourna::api::sql

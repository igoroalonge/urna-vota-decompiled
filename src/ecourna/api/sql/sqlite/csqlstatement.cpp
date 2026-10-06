// ecourna-lib/ecourna/api/sql/sqlite/csqlstatement.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/sql/sqlite/csqlstatement.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// Library code that the unit builder attributed to this file (it is inlined/instantiated here because
// SetDateTime formats a boost::posix_time::ptime):
//   wasm func 1376  boost::gregorian::gregorian_calendar::from_day_number() -> year_month_day, with the
//                   greg_year range check "Year is out of valid range: 1400..9999" (boost::gregorian::bad_year)
//   wasm func 5183  simple_exception_policy<ushort, 1, 12, bad_month>::on_error   ("Month number is out of range 1..12")
//   wasm func 5182  simple_exception_policy<ushort, 1, 31, bad_day_of_month>::on_error
//                   ("Day of month value is out of range 1..31")
//   wasm func 9414  boost::date_time::month_formatter<greg_month, iso_extended_format<char>, char>::format_month
//                   (setw(2), setfill('0') with an ios fill saver)
// They are not reconstructed.
#include "ecourna/api/sql/sqlite/csqlstatement.hpp"

#include <sqlite3.h>

#include <mutex>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "ecourna/api/sql/esqlerror.hpp"
#include "ecourna/api/sql/sqlite/csqlconnection.hpp"   // GetErrorMessage (func 9411)
#include "ecourna/api/sql/sqlite/csqlresultset.hpp"
#include "ecourna/api/util/cstringutils.hpp"

namespace ecourna::api::sql::sqlite {

namespace {
// Global std::mutex at 0x1D2BF4 (@1911796), used only by Close(). Its destructor is registered with
// atexit (wasm func 9431 = ~mutex, table slot 6389). In this single-threaded build lock() compiles to
// nothing (the pthread stub always succeeds) and only the unlock() residue (func 150) is left.
std::mutex s_mutexFinalizacao;   // name inferred
} // namespace

// wasm func 9430 - name inferred. Observed executing.
CSqlStatement::CSqlStatement(sqlite3* db, sqlite3_stmt* comando)
    : m_db(db), m_comando(comando), m_fechado(false), m_qtdParametros(sqlite3_bind_parameter_count(comando))
{
}

// wasm func 3513 (complete destructor, also called directly by Prepare's cleanup path)
// wasm func 9426 (deleting destructor)
CSqlStatement::~CSqlStatement()
{
    if (!m_fechado) {
        try {
            Close();
        } catch (...) {
        }
    }
}

// wasm func 5160 (srcloc line 53)
void CSqlStatement::SetInt(int indice, int valor)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_int(m_comando, indice, valor) != SQLITE_OK)
        throw CSqlError(ESqlError::SetInt, GetErrorMessage(m_db));
}

// wasm func 9425 (srcloc line 63)
void CSqlStatement::SetInt64(int indice, ueint64 valor)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_int64(m_comando, indice, static_cast<sqlite3_int64>(valor)) != SQLITE_OK)
        throw CSqlError(ESqlError::SetInt64, GetErrorMessage(m_db));
}

// wasm func 9424 (srcloc line 73)
void CSqlStatement::SetDouble(int indice, double valor)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_double(m_comando, indice, valor) != SQLITE_OK)
        throw CSqlError(ESqlError::SetDouble, GetErrorMessage(m_db));
}

// wasm func 9423 (srcloc line 83)
void CSqlStatement::SetText(int indice, const std::string& valor)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_text(m_comando, indice, valor.data(), static_cast<int>(valor.size()), SQLITE_TRANSIENT) != SQLITE_OK)
        throw CSqlError(ESqlError::SetText, GetErrorMessage(m_db));
}

// wasm func 9422 - name inferred (no srcloc; between lines 83 and 109).
// Stores the timestamp as text "YYYY-MM-DD HH:MM:SS[.ffffff]" (ISO extended with the 'T' replaced by a
// space); special values become "not-a-date-time", "-infinity" or "+infinity" (no 'T' part).
// CSqlResultSet::GetDateTime (func 5162) is the inverse; of the special values only "+infinity" reads back.
void CSqlStatement::SetDateTime(int indice, const boost::posix_time::ptime& valor)
{
    std::string texto = boost::posix_time::to_iso_extended_string(valor);
    util::CStringUtils::Replace(texto, 'T', " ");   // wasm func 9398 (not in u13): every 'T' -> " "   // ?name
    SetText(indice, texto);
}

// wasm func 9421 - name inferred (a tail call to SetInt)
void CSqlStatement::SetBool(int indice, bool valor)
{
    SetInt(indice, valor);
}

// wasm func 9420 (srcloc line 109)
// An empty vector that never allocated has data() == nullptr, for which sqlite3_bind_blob binds SQL NULL, not a
// zero-length blob (an empty vector that still owns a buffer, e.g. after clear(), binds a zero-length blob).
void CSqlStatement::SetBlob(int indice, const std::vector<uebyte>& valor)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_blob(m_comando, indice, valor.data(), static_cast<int>(valor.size()), SQLITE_TRANSIENT) != SQLITE_OK)
        throw CSqlError(ESqlError::SetBlob, GetErrorMessage(m_db));
}

// wasm func 9419 (srcloc line 119)
void CSqlStatement::SetNull(int indice)
{
    VerifyParameterIndex(indice);
    if (sqlite3_bind_null(m_comando, indice) != SQLITE_OK)
        throw CSqlError(ESqlError::SetNull, GetErrorMessage(m_db));
}

// wasm func 9417 - name inferred. Observed executing.
// The result set steps the statement once in its constructor, so Execute() alone runs DDL/DML.
// The result set keeps raw sqlite3/sqlite3_stmt pointers: it must not outlive Close().
TSharedSqlResultSet CSqlStatement::Execute()
{
    return TSharedSqlResultSet(new CSqlResultSet(m_db, m_comando));   // CSqlResultSet ctor = func 9449
}

// wasm func 9416 (srcloc line 133). Bindings are kept (no sqlite3_clear_bindings).
void CSqlStatement::Reset()
{
    if (sqlite3_reset(m_comando) != SQLITE_OK)
        throw CSqlError(ESqlError::Reset, GetErrorMessage(m_db));
}

// wasm func 9429 (srcloc line 143). Observed executing.
// The lock/try/catch(...){unlock; throw;} shape is what the landing pads show (begin_catch, unlock,
// __cxa_rethrow); a lock_guard would have produced a cleanup pad instead.
void CSqlStatement::Close()
{
    s_mutexFinalizacao.lock();
    try {
        if (sqlite3_finalize(m_comando) != SQLITE_OK)
            throw CSqlError(ESqlError::FecharComando, GetErrorMessage(m_db));
        m_fechado = true;
    } catch (...) {
        s_mutexFinalizacao.unlock();
        throw;
    }
    s_mutexFinalizacao.unlock();
}

// wasm func 1882 (srcloc line 158). Valid indices: 1 .. sqlite3_bind_parameter_count().
void CSqlStatement::VerifyParameterIndex(int indice) const
{
    if (indice <= 0 || indice > m_qtdParametros)
        throw CSqlError(ESqlError::IndiceParametroInvalido, "Índice do parâmetro fora do intervalo válido.");
}

} // namespace ecourna::api::sql::sqlite

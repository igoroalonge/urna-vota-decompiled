// ecourna-lib/ecourna/api/sql/sqlite/csqlresultset.cpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/sql/sqlite/csqlresultset.cpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u13.
//
// Library code attributed to this file by the unit builder (not reconstructed):
//   wasm func 2678  std::map<std::string, int>::find (also used by shared_f5149 for another string map)
//   wasm func 9447  std::map<std::string, int>::__emplace_unique_key_args (operator[] with piecewise_construct)
//   wasm func 1677  std::__tree<std::string, int>::destroy(node) (recursive)
//
// Every getter has its OWN "Next() not called" check with its own error code and source line, so the check
// is written inline in each one (it is not a shared helper).
#include "ecourna/api/sql/sqlite/csqlresultset.hpp"

#include <sqlite3.h>

#include <format>
#include <string>
#include <vector>

#include <boost/date_time/posix_time/posix_time.hpp>

#include "ecourna/api/sql/esqlerror.hpp"
#include "ecourna/api/sql/sqlite/csqlconnection.hpp"   // GetErrorMessage (func 9411)
#include "ecourna/api/util/cstringutils.hpp"

namespace ecourna::api::sql::sqlite {

namespace {
constexpr const char* MSG_CHAME_NEXT = "Chame o método 'Next()' antes de buscar resultados.";   // @360275, name inferred
} // namespace

// wasm func 9449 - no srcloc (the name comes from the curated table). Observed executing.
CSqlResultSet::CSqlResultSet(sqlite3* db, sqlite3_stmt* comando)
    : m_db(db), m_comando(comando)
{
    m_temLinha = StepStatement();   // invoked; on exception the column map is destroyed and the exception resumes
    m_aguardandoNext = true;
}

// wasm func 2679 (srcloc line 39)
int CSqlResultSet::GetInt(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::GetIntSemNext, MSG_CHAME_NEXT);
    return sqlite3_column_int(m_comando, coluna);
}

// wasm func 5165 (srcloc line 50)
ueint64 CSqlResultSet::GetInt64(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::GetInt64SemNext, MSG_CHAME_NEXT);
    return static_cast<ueint64>(sqlite3_column_int64(m_comando, coluna));
}

// wasm func 5164 (srcloc line 61)
double CSqlResultSet::GetDouble(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::GetDoubleSemNext, MSG_CHAME_NEXT);
    return sqlite3_column_double(m_comando, coluna);
}

// wasm func 3514 (srcloc line 72)
// The copy uses strlen: text with an embedded NUL is truncated. NULL or empty -> "".
std::string CSqlResultSet::GetText(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::GetTextSemNext, MSG_CHAME_NEXT);
    const auto* texto = reinterpret_cast<const char*>(sqlite3_column_text(m_comando, coluna));
    if (sqlite3_column_bytes(m_comando, coluna) != 0 && texto != nullptr)
        return std::string(texto);
    return {};
}

// wasm func 5163 (srcloc line 90)
// "NULL" is decided through the text conversion, not sqlite3_column_type(). sqlite3_column_text returns NULL
// only for SQL NULL (or out of memory): a zero-length TEXT or BLOB gives a non-NULL "" (checked with SQLite
// 3.51 on the host: `x''`, `zeroblob(0)` -> type BLOB, text != NULL, bytes 0), so the result is still correct.
// Side effect: the column value is converted to text in place (an earlier GetBlob pointer may be invalidated).
// A never-allocated empty vector written with SetBlob is stored as SQL NULL (see CSqlStatement::SetBlob), so it
// reads back as NULL.
bool CSqlResultSet::IsNull(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::IsNullSemNext, MSG_CHAME_NEXT);
    const unsigned char* texto = sqlite3_column_text(m_comando, coluna);
    const int bytes = sqlite3_column_bytes(m_comando, coluna);
    return texto == nullptr && bytes == 0;
}

// wasm func 5162 - name inferred (no srcloc, between lines 90 and 128).
// Inverse of CSqlStatement::SetDateTime: "YYYY-MM-DD HH:MM:SS" -> "YYYYMMDDTHHMMSS" -> from_iso_string.
// Special values: parse_iso_time (func 5851) runs boost's special_values_parser only when the text starts with
// '+', '-', 'n' or 'm'. "+infinity" survives the edits and round-trips; "not-a-date-time" becomes
// "notadatetime" and "-infinity" becomes "infinity", which both fail and throw a boost exception (not CSqlError).
boost::posix_time::ptime CSqlResultSet::GetDateTime(int coluna)
{
    std::string texto = GetText(coluna);
    util::CStringUtils::Replace(texto, ' ', "T");   // wasm func 9398 (not in u13)   // ?name
    util::CStringUtils::Replace(texto, '-', "");
    util::CStringUtils::Replace(texto, ':', "");
    return boost::date_time::parse_iso_time<boost::posix_time::ptime>(texto, 'T');   // = from_iso_string
}

// wasm func 9446 - name inferred. Only the value 1 is true.
bool CSqlResultSet::GetBool(int coluna)
{
    return GetInt(coluna) == 1;
}

// wasm func 5161 (srcloc line 128)
std::vector<uebyte> CSqlResultSet::GetBlob(int coluna)
{
    VerifyColumnIndex(coluna);
    if (m_aguardandoNext)
        throw CSqlError(ESqlError::GetBlobSemNext, MSG_CHAME_NEXT);
    const auto* dados = static_cast<const uebyte*>(sqlite3_column_blob(m_comando, coluna));
    const int tamanho = sqlite3_column_bytes(m_comando, coluna);
    if (tamanho == 0)
        return {};
    return std::vector<uebyte>(dados, dados + tamanho);
}

// The by-name overloads: IndexForColumnName + the index overload (called non-virtually).
int CSqlResultSet::GetInt(const std::string& coluna)       { return GetInt(IndexForColumnName(coluna)); }       // wasm func 9441
ueint64 CSqlResultSet::GetInt64(const std::string& coluna) { return GetInt64(IndexForColumnName(coluna)); }     // wasm func 9440
double CSqlResultSet::GetDouble(const std::string& coluna) { return GetDouble(IndexForColumnName(coluna)); }    // wasm func 9439
std::string CSqlResultSet::GetText(const std::string& coluna) { return GetText(IndexForColumnName(coluna)); }   // wasm func 9438
bool CSqlResultSet::IsNull(const std::string& coluna)      { return IsNull(IndexForColumnName(coluna)); }       // wasm func 9435
boost::posix_time::ptime CSqlResultSet::GetDateTime(const std::string& coluna)
{
    return GetDateTime(IndexForColumnName(coluna));                                                             // wasm func 9434
}
bool CSqlResultSet::GetBool(const std::string& coluna)     { return GetInt(IndexForColumnName(coluna)) == 1; }  // wasm func 9433
std::vector<uebyte> CSqlResultSet::GetBlob(const std::string& coluna) { return GetBlob(IndexForColumnName(coluna)); } // wasm func 9432

// wasm func 9444 - name from the error message. The first call consumes the step made by the constructor.
// Later steps are NOT stored in m_temLinha (it is only meaningful for the first row).
bool CSqlResultSet::Next()
{
    if (m_aguardandoNext) {
        m_aguardandoNext = false;
        return m_temLinha;
    }
    return StepStatement();
}

// wasm func 1883 (srcloc line 143)
void CSqlResultSet::VerifyColumnIndex(int coluna) const
{
    if (coluna < 0 || coluna >= m_qtdColunas)
        throw CSqlError(ESqlError::IndiceColunaInvalido, "Índice da coluna fora do intervalo válido.");
}

// wasm func 1525 (srcloc line 153). Case-insensitive: the key is lower-cased with the Latin-1-aware
// CStringUtils::ToLower (func 1879); the message shows the name as the caller wrote it.
int CSqlResultSet::IndexForColumnName(const std::string& coluna)
{
    const auto it = m_colunas.find(util::CStringUtils::ToLower(coluna));
    if (it == m_colunas.end())
        throw CSqlError(ESqlError::ColunaInexistente, std::format("Coluna {} não faz parte do resultado.", coluna));
    return it->second;
}

// wasm func 5166 (srcloc line 185). Observed executing.
// The name->index map is rebuilt after EVERY step (existing keys are overwritten).
bool CSqlResultSet::StepStatement()
{
    const int rc = sqlite3_step(m_comando);

    m_qtdColunas = sqlite3_column_count(m_comando);
    for (int i = 0; i < m_qtdColunas; ++i) {
        if (const char* nome = sqlite3_column_name(m_comando, i))
            m_colunas[util::CStringUtils::ToLower(std::string(nome))] = i;
    }

    if (rc == SQLITE_ROW || rc == SQLITE_OK || rc == SQLITE_DONE)
        return rc == SQLITE_ROW;

    const char* sql = sqlite3_sql(m_comando);
    const std::string operacao = sql ? std::string(sql) : std::string("INDISPONÍVEL");   // @327997
    throw CSqlError(ESqlError::Executar,
                    std::format("Falha ao executar operação no banco de dados.\n"
                                "Mensagem do banco de dados: '{}'\n"
                                "Operação: '{}'",
                                GetErrorMessage(m_db), operacao));
}

} // namespace ecourna::api::sql::sqlite

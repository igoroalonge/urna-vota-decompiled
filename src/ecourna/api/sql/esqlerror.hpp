// ecourna-lib/ecourna/api/sql/esqlerror.hpp   (path inferred: ecourna keeps one error enum per API module)
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build). Unit u13,
// see docs/modules/u13-ecourna-lib-ecourna-api-security-ecourna-lib-ecourna-api-sql.md.
//
// Exception type of the SQL module: exception::CBaseError<sql::ESqlError, SErrorLimits{1725, 1775}>
// (typeinfo @1115992, vtable @1116272). Every throw site goes through the 18-byte thunk wasm func 9461,
// which calls the shared CBaseError constructor body (func 1011) with that vtable.
//
// The numeric values are certain (they are the constants passed to func 9461). They are numbered in
// source order, file by file: csqlconnection.cpp (1725..1728), csqlresultset.cpp (1729..1737),
// csqlstatement.cpp (1738..1746). The enumerator NAMES are inferred.
#pragma once

#include "ecourna/api/exception/cbaseerror.hpp"

namespace ecourna::api::sql {

enum class ESqlError : int {
    // csqlconnection.cpp
    AbrirBanco              = 1725,   // CSqlConnection(const std::string&)  line 29 - sqlite3_open failed
    ConfigurarBanco         = 1726,   // CSqlConnection(const std::string&)  line 44 - "PRAGMA foreign_keys = ON" failed
    Preparar                = 1727,   // CSqlConnection::Prepare             line 86
    FecharBanco             = 1728,   // CSqlConnection::Close               line 103
    // csqlresultset.cpp
    GetIntSemNext           = 1729,   // GetInt      line 39
    GetInt64SemNext         = 1730,   // GetInt64    line 50
    GetDoubleSemNext        = 1731,   // GetDouble   line 61
    GetTextSemNext          = 1732,   // GetText     line 72
    IsNullSemNext           = 1733,   // IsNull      line 90
    GetBlobSemNext          = 1734,   // GetBlob     line 128
    IndiceColunaInvalido    = 1735,   // VerifyColumnIndex  line 143
    ColunaInexistente       = 1736,   // IndexForColumnName line 153
    Executar                = 1737,   // StepStatement      line 185
    // csqlstatement.cpp
    SetInt                  = 1738,   // line 53
    SetInt64                = 1739,   // line 63
    SetDouble               = 1740,   // line 73
    SetText                 = 1741,   // line 83
    SetBlob                 = 1742,   // line 109
    SetNull                 = 1743,   // line 119
    Reset                   = 1744,   // line 133
    FecharComando           = 1745,   // Close line 143
    IndiceParametroInvalido = 1746,   // VerifyParameterIndex line 158
};

// alias name inferred
using CSqlError = exception::CBaseError<ESqlError, exception::SErrorLimits{1725, 1775}>;

} // namespace ecourna::api::sql

// ecourna-lib/ecourna/api/exception/cerror.cpp   (path inferred)
//
// Reconstructed from vota_web_wasm.wasm. Unit u40. No std::source_location record points into this file
// (the constructor receives the location of its caller), which is why the tools could not place it.
#include "ecourna/api/exception/cerror.hpp"

#include <string>

namespace ecourna::api::exception {

using namespace std::string_literals;

// wasm func 1143
// The message is copied into m_what first (member initialiser), then m_what is replaced by the long form.
// Every piece is a std::string temporary (the literals are built as 1- and 3-char SSO strings on the stack,
// and the first concatenation is `const char* + std::string&&`, i.e. string::insert(0, ...)), which is what
// "..."s literals produce. With a default-constructed source_location the function name is "" and the line 0.
//
// Example what(): "virtual void vota::CPedeIdentidade::StartState():62:9409 - Tipo de identidade ..."
// (function_name() of libc++ is the full pretty signature).
CError::CError(int codigo, std::string mensagem, std::source_location local)
    : m_codigo(codigo)
    , m_mensagem(std::move(mensagem))
    , m_local(local)
    , m_what(m_mensagem)
{
    // Every landing pad of the body goes to __cxa_find_matching_catch_3(0) = catch (...), destroys the
    // temporaries, then __cxa_begin_catch/__cxa_end_catch and a normal return: a failure while building the
    // long text (e.g. std::bad_alloc) is swallowed and what() keeps the plain message.
    try {
        m_what = m_local.function_name() + ":"s + std::to_string(m_local.line())   // api_f327 = to_string(unsigned)
               + ":"s + std::to_string(m_codigo)                                   // ecourna_f296 = to_string(int)
               + " - "s + m_mensagem;                                               // literal @445458
    } catch (...) {
    }
}

// wasm func 4636 (vtable slot 0): restores the CError vptr, ~m_what (+28), ~m_mensagem (+8).
// wasm func 1940 (vtable slot 1): the deleting destructor = func 4636 + operator delete.
CError::~CError() = default;

// wasm func 11562 (vtable slot 2)
const char* CError::what() const noexcept
{
    return m_what.c_str();
}

} // namespace ecourna::api::exception

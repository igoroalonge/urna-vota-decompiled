// ecourna-lib/ecourna/api/exception/cerror.hpp   (path inferred: class CError -> cerror.hpp, next to
//                                                   cbaseerror.hpp which 34 reconstructed files include)
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build). Unit u40.
//
// RTTI: ecourna::api::exception::CError : std::exception   (typeinfo @1526376, vtable @1526468)
//   [0] ~CError()            wasm func 4636  (also slot 0 of all 42 CBaseError<> vtables)
//   [1] ~CError() deleting   wasm func 1940  (CError + the 5 families EPatternError, EUePatternError,
//                                             EUeComumDadosError, EUeComumAsnError, EUeIoError - the same five
//                                             whose constructor goes through 2294/2379). 37 CBaseError<>
//                                             vtables and the derived CIoError / CUeDesligandoError use func
//                                             454 instead (the same destructor with the two ~string inlined);
//                                             api::CUePrinterError has its own (4910).
//   [2] what() const         wasm func 11562 (shared by every CBaseError<>)
//
// CError is the root of every exception thrown by the TSE code (ecourna and uenux2). Nobody throws a bare
// CError: each module derives an error family CBaseError<E, SErrorLimits{min, max}> (cbaseerror.hpp) whose
// only difference is the RTTI (the catch type) and the enum of codes. Objects are always 40 bytes
// (__cxa_allocate_exception(40) at every throw site).
#pragma once

#include <exception>
#include <source_location>
#include <string>

namespace ecourna::api::exception {

class CError : public std::exception {
public:
    // wasm func 1143 (reached directly from the CBaseError<> bodies 710 / 2379 and through invoke slot 169
    // from body 1011). Also called directly by functions that build a derived error in place:
    // vota::CSincronizaVota::VerificaUrnaDesligando 1685, vota::CThreadMonitor::GetInst 1898 (inlined
    // comum::util::CMonitoraAlimentacao::CreateInst), io::CIoError 3520, api::CEncryptedFile::Load 3653,
    // CConversorResultadoUrnaCadastro::DoConverte 9056, api::CUePrinterError 10257, api::CRHVoiceTextToSpeech 10841,
    // api::IImpressoraRelatorios::SetStyle 10881 and comum::impl::CValidaMidia 11172.
    CError(int codigo, std::string mensagem, std::source_location local);

    ~CError() override;                                   // wasm func 4636 (D1) / 1940 (D0)

    // wasm func 11562: returns m_what.c_str() ("<function>:<line>:<code> - <message>").
    const char* what() const noexcept override;

    // Inlined accessors (e.g. vota::CThreadVota::TrataExcecao reads +4 directly).        // ? names inferred
    int GetCodigo() const { return m_codigo; }
    const std::string& GetMensagem() const { return m_mensagem; }
    const std::source_location& GetLocal() const { return m_local; }

private:
    // +0 vptr (std::exception)
    int m_codigo;                     // +4   error code (the value of the family enum)
    std::string m_mensagem;           // +8   message as given by the thrower (moved in)
    std::source_location m_local;     // +20  libc++: one pointer to the static {file, function, line, column}
    // +24  4 bytes never written by the constructor (padding or an unused member)          // ?
    std::string m_what;               // +28  full text returned by what()
};                                    // sizeof 40

} // namespace ecourna::api::exception

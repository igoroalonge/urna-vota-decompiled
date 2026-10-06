// ecourna-lib/ecourna/api/io/cioerror.hpp   (path inferred: class name -> file name, next to cfile.cpp)
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build).
// Unit u12, see docs/modules/u12-ecourna-lib-ecourna-api-compression-ecourna-lib-ecourna-api-.md
//
// RTTI: ecourna::api::io::CIoError (typeinfo @1554360, vtable @1112640)
//         : ecourna::api::exception::CBaseError<EIoError, SErrorLimits{1175, 1275}>
//           : ecourna::api::exception::CError
// Every error thrown by CFile (and by DeserializeFromBuffer) is a CIoError. The object is 40 bytes
// (__cxa_allocate_exception(40)), like every other CBaseError<> of the code base.
#pragma once

#include <source_location>
#include <string>

#include "ecourna/api/exception/cbaseerror.hpp"   // CBaseError<E, SErrorLimits{min, max}>  (not in this unit)

namespace ecourna::api::io {

// The operation during which the error happened. Values = index in the name table @1112652
// ("None", "Open", "Close", "Read", "Write", "Seek", "Tell", "Sync", "Mode").
enum class EFileOperation : int {
    None  = 0,
    Open  = 1,
    Close = 2,
    Read  = 3,
    Write = 4,
    Seek  = 5,
    Tell  = 6,
    Sync  = 7,
    Mode  = 8,
};

// Error codes (limits 1175..1275). Enumerator names inferred; values and texts are exact.
enum class EIoError : int {
    ErroAbrir                    = 1175,   // Open: fopen() failed                        (cfile.cpp:60)
    ErroAlterarModo              = 1176,   // Open: SetFileMode() failed                  (cfile.cpp:74)
    /* 1177 not referenced in this binary */
    ModoArquivoNaoAberto         = 1178,   // SetFileMode on a closed file (passes errno)  (cfile.cpp:117)
    ErroFechar                   = 1179,   // Close: fclose() failed                      (cfile.cpp:111)
    FlushArquivoNaoAberto        = 1180,   // Flush on a closed file                      (cfile.cpp:145)
    SyncArquivoNaoAberto         = 1181,   // Sync on a closed file                       (cfile.cpp:160)
    SyncErroDescritor            = 1182,   // Sync: fileno() == -1                        (cfile.cpp:170)
    SyncErroFsync                = 1183,   // Sync: fsync() failed                        (cfile.cpp:175)
    SyncErroFflush               = 1184,   // Sync: fflush() failed                       (cfile.cpp:165)
    EscritaArquivoNaoAberto      = 1185,   // RawWrite on a closed file                   (cfile.cpp:184)
    EscritaBufferInvalido        = 1186,   // RawWrite(nullptr, n)                        (cfile.cpp:187)
    EscritaTamanhoZero           = 1187,   // RawWrite(p, 0)                              (cfile.cpp:190)
    ErroEscrita                  = 1188,   // fwrite() == 0 or fflush() != 0              (cfile.cpp:195)
    /* 1189, 1190: not referenced. Probably CFile::WriteLine(const std::string&) const: its two srcloc
       records (cfile.cpp:211 @1112360, :214 @1112376) are in the data segment, between RawWrite's and
       RawRead's, but no function references them (the method itself is not linked). */
    LeituraArquivoNaoAberto      = 1191,   // RawRead on a closed file                    (cfile.cpp:222)
    LeituraBufferInvalido        = 1192,   // RawRead(nullptr, n)                         (cfile.cpp:225)
    LeituraTamanhoZero           = 1193,   // RawRead(p, 0)                               (cfile.cpp:228)
    ErroLeitura                  = 1194,   // fread() == 0 and ferror()                   (cfile.cpp:234)
    ErroLeituraSemEof            = 1195,   // fread() == 0, no error and no EOF           (cfile.cpp:240)
    /* 1196..1204: not referenced (other CFile methods not linked into this binary) */
    ReadLineArquivoNaoAberto     = 1205,   // ReadLine on a closed file                   (cfile.cpp:336)
    ErroReadLine                 = 1206,   // ReadLine read nothing and not at EOF         (cfile.cpp:356)
    SeekArquivoNaoAberto         = 1207,   //                                             (cfile.cpp:363)
    ErroSeek                     = 1208,   //                                             (cfile.cpp:366)
    EofArquivoNaoAberto          = 1209,   //                                             (cfile.cpp:374)
    PositionArquivoNaoAberto     = 1210,   //                                             (cfile.cpp:385)
    ErroPosition                 = 1211,   //                                             (cfile.cpp:389)
    /* 1212: not referenced */
    ReadFileContentErroAbrir     = 1213,   //                                             (cfile.cpp:420)
    ReadFileBinaryErroAbrir      = 1214,   //                                             (cfile.cpp:435)
    ReadFileBinaryErroLeitura    = 1215,   //                                             (cfile.cpp:447)
    /* 1216..1220: not referenced */
    DecodeBerFalhou              = 1221,   // DeserializeFromBuffer                       (serialization.hpp:44)
    ConteudoInvalido             = 1222,   // DeserializeFromBuffer                       (serialization.hpp:47)
};

// The error text for an EFileOperation. Out-of-range values give the literal
// "Description(EFileOperation) - não reconhecida". (Table @1112652, index checked with `op <= 8`.)
const char* Description(EFileOperation op);                                    // inlined into func 3520

class CIoError : public exception::CBaseError<EIoError, exception::SErrorLimits{1175, 1275}> {
public:
    // wasm func 3520 (reached through table slot 6210 by every CFile method).
    // Builds the final message and forwards (code, message, location) to CBaseError (func 1143).
    //   errnum != 0:  "<msg> [<Description(op)>] - <errnum> - <strerror(errnum)>"
    //   errnum == 0:  "<msg> [<Description(op)>]"
    CIoError(EIoError code, EFileOperation op, const std::string& msg, int errnum,
             const std::source_location& location = std::source_location::current());
};

} // namespace ecourna::api::io

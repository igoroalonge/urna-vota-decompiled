// ecourna-lib/ecourna/api/io/asn/serialization.hpp
// (/home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/api/io/asn/serialization.hpp in the srcloc records)
//
// Reconstructed from vota_web_wasm.wasm. Unit u12.
//
// Header-only BER (de)serialisation helpers of ecourna. Only one instantiation survives in the binary:
//
//   wasm func 2280  =  DeserializeFromFile<ModuloEnvelopeChave::EntidadeChave>(obj, fileName)   [name inferred]
//                      with DeserializeFromBuffer<EntidadeChave> (srcloc lines 44, 47) inlined.
//
// The analyzer named func 2280 "DeserializeFromBuffer" because the only srclocs inside it are the buffer
// function's. The function actually receives a *file name* (2nd argument, a std::string that is passed to
// the CFile constructor with mode "rb"). It reads the whole file and then decodes it.
//
// Callers (all read a key envelope "EntidadeChave"):
//   vota::CGeraBU::vf2 (12110)                            key of the BU "código verificador"
//   comum::CGravadorBU::vf7 (11629)                       public key used to protect the BU file
//   comum::CGravadorRCSecao::LeChavePublica (11616)       attendance (comparecimento) file key
//   comum::CControlaArmazenamentoDeImagens::LeChavePublica (2725)   WSQ (fingerprint image) key
//   comum::asn::CConversorBiometriaEleitorCifrada::vf3 (11419)
// After decoding, the callers hand the entity to security::CKeyLoader::DecipherKeyIfNeeded (unit u01).
//
// Exception model: func 2280 has NO landing pads. Every callee (CFile ctor, Seek, Position, RawRead,
// CoderEnv::decode, CIoError ctor) is a direct call, not an invoke_*, so nothing is destroyed when an
// exception leaves it: the CFile is never closed (FILE* and fd leak), and the vector buffer and the CoderEnv
// leak. The linker kept a COMDAT copy compiled in a uenux2 translation unit, and uenux2 is built with
// exception catching disabled for almost all functions (docs/modules/u11-...md §4.2). Four of the five callers
// (2725, 11419, 11616, 11629) also call it directly; only vota::CGeraBU (12110) goes through invoke_vii.
#pragma once

#include <cstdio>
#include <string>
#include <vector>

#include "asn1/asn1.h"                       // ASN1::CoderEnv, ASN1::AbstractData (asn1-runtime, not in this unit)
#include "ecourna/api/io/cfile.hpp"
#include "ecourna/api/io/cioerror.hpp"

namespace ecourna::api::io {

// inlined into func 2280 (srcloc lines 44 and 47)
template <typename ClasseASN1>
void DeserializeFromBuffer(ClasseASN1& objeto, const std::vector<char>& buffer)
{
    ASN1::CoderEnv env;                       // 40-byte object on the stack, all zero ...
    env.set_encodingRule(ASN1::CoderEnv::ber);   // ... except the rule field (+0) = 1 (BER)

    if (!env.decode(buffer.data(), buffer.data() + buffer.size(), objeto)) {           // func 679
        throw CIoError(EIoError::DecodeBerFalhou /*1221*/, EFileOperation::None,
                       "Falha ao fazer decode BER.", 0);                               // line 44
    }
    if (!objeto.isValid() || !objeto.isStrictlyValid()) {                               // funcs 221, 231
        throw CIoError(EIoError::ConteudoInvalido /*1222*/, EFileOperation::None,
                       "Foi lido conteúdo inválido.", 0);                              // line 47
    }
}   // ~CoderEnv (func 518)

// wasm func 2280 [ClasseASN1 = ModuloEnvelopeChave::EntidadeChave]   (name and exact shape inferred)
template <typename ClasseASN1>
void DeserializeFromFile(ClasseASN1& objeto, const std::string& fileName)
{
    std::vector<char> buffer;
    {
        CFile arquivo(fileName, "rb");                                     // func 517, FileMode FM_NORMAL
        arquivo.Seek(0, SEEK_END);
        buffer.resize(static_cast<std::size_t>(arquivo.Position()));       // zero-filled
        arquivo.Seek(0, SEEK_SET);
        // An empty file gives buffer.data() == nullptr, so RawRead throws 1192 "buffer invalido" before
        // any decoding happens (and, without landing pads in this copy, `arquivo` is never closed).
        const auto lidos = arquivo.RawRead(buffer.data(), static_cast<uedword>(buffer.size()));
        arquivo.Close();
        buffer.resize(lidos);
    }                                                                      // ~CFile (Close already done;
                                                                           //  normal path only, see above)
    DeserializeFromBuffer(objeto, buffer);
}

} // namespace ecourna::api::io

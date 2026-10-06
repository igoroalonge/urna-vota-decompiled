// uenux2/src/api/io/asn/cfileasn.h
//
// Reconstructed from vota_web_wasm.wasm (VOTA "10.23.0.1 - DESENVOLVIMENTO", web simulator build).
// Unit u18. See docs/modules/u18-uenux2-src-api-hwil-ipower-h-uenux2-src-api-io-asn-uenux2-sr.md
// (older fragment by u12: cfileasn.u12-fragment.h - WriteToFile bodies 2891/2892 and the four state-file thunks).
//
// api::CFileASN: static helpers that move an ASN.1 (III ASN.1 runtime, BER) object between a file and
// memory. It is header-only (templates), so no function exists "as is": every instantiation was inlined
// into its caller or merged by wasm-opt. What survives:
//
//   srcloc line  member                                   error (api::EUeIoError, CBaseError limits {5950, 6150})
//   cfileasn.h:48   ReadFromFile(const CFile&)            5950 "O arquivo [{}] não estava aberto"
//   cfileasn.h:60   ReadFromFile(const CFile&)            5951 "O arquivo [{}] é muito grande para ser lido" (> 5 MiB)
//   cfileasn.h:78   ReadFromFile(const std::string&)      5952 "O arquivo [{}] não existe"
//   cfileasn.h:135  DecodeObjectFunction                  5953 "Conteúdo não foi decodificado para {}: {}"
//   cfileasn.h:143  DecodeObjectFunction                  5954 "Conteúdo inválido para {}: {}"
//   cfileasn.h:161  CodeObjectFunction                    5955 "Objeto com conteúdo inválido para {}: {}"
//   cfileasn.h:171  CodeObjectFunction                    5956 "Arquivo não foi codificado para " + contexto
//
// Instantiations (T = ASN.1 type) that exist as functions of their own:
//   wasm 5825  DecodeObjectFunction<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto> (+ its DecodeObject wrapper)
//   wasm 2927  merged body of CodeObjectFunction<T> for the four state files, thunks:
//              9673 EstadoGeralVota (vota.bin)  9695 EstadoGeralSA (sa.bin)  9702 EstadoGeralGap (gap.bin)
//              9801 EstadoGeralUrna (eg.bin)    - the thunk passes the two srclocs of its own T
//   wasm 6006  merged body of WriteDataToFile<TConversor> for comum converters, thunks:
//              2724 CConversorEnvelopeGenerico (bu.dat, imgbu.dat, imgze.dat)   5367 CConversorEntidadeHashes (hash.dat)
//   wasm 5366  WriteDataToFile<ecourna CConversorResultadoUrnaCadastro>   (jufa.dat)
//   wasm 3735  ReadDataFromFile<ecourna CConversorInformacaoMidia>        (infomidia.dat)
//   wasm 5706  ReadDataFromFile<comum CConversorCabecalhoPacote>          (package headers "<fase><eleição:05><uf>-ce.pid")
//   wasm 9790  std::__format_arg_store ctor for (const std::string&, std::string&) used by 2927 (library)
// Callers whose body is mostly an inlined ReadFromFile/CodeObjectFunction (the analyzer named them after
// these srclocs) are reconstructed in their own files, see cfileasn.instances.cpp and the u18 fragments:
//   3771 / 5778 (processo eleitoral + pleitos), 3791 (IServicoEstado<CEstadoGeral>), 5740 (CLocal),
//   11587 (CGravadorRDV::GravaResultado), 11626 (IGravadorEnvelope::GravaResultado).
#pragma once

#include <format>
#include <sstream>
#include <string>
#include <typeinfo>
#include <vector>

#include "asn1/asn1.h"                       // ASN1::AbstractData, ASN1::CoderEnv, ASN1::trace_invalid (III ASN.1)
#include "api/io/euioerror.h"                // api::EUeIoError, api::CUeIoError (= CBaseError<EUeIoError, {5950,6150}>)
#include "api/util/csystem.h"                // api::CSystem::IsRegularFile (wasm 412)
#include "ecourna/api/io/cfile.hpp"          // ecourna::api::io::CFile (wasm 517 ctor, 598 RawRead, 1886 RawWrite ...)

namespace api {

using CFile = ecourna::api::io::CFile;

class CFileASN {
public:
    // Largest file ReadFromFile accepts: 5 MiB (the test is `tamanho >= 5242881`).
    static constexpr long TAMANHO_MAXIMO_ARQUIVO = 5 * 1024 * 1024;                  // name inferred

    // ---------------------------------------------------------------------------------------------
    // Reading
    // ---------------------------------------------------------------------------------------------

    // cfileasn.h:48 / :60 - reads the rest of an already opened file and decodes it.
    // Only exists inlined (every ReadFromFile(const std::string&) instance, e.g. wasm 5706).
    template <typename T>
    static T ReadFromFile(const CFile& arquivo)
    {
        if (!arquivo.IsOpen())                                                       // m_file == nullptr
            throw CUeIoError(EUeIoError(5950),
                             std::format("O arquivo [{}] não estava aberto", arquivo.GetName()));   // line 48

        const long inicio = arquivo.Position();                                      // wasm 418
        arquivo.Seek(0, SEEK_END);                                                   // wasm 419
        const long fim = arquivo.Position();
        arquivo.Seek(inicio, SEEK_SET);

        const long tamanho = fim - inicio;
        if (tamanho > TAMANHO_MAXIMO_ARQUIVO)
            throw CUeIoError(EUeIoError(5951),
                             std::format("O arquivo [{}] é muito grande para ser lido", arquivo.GetName()));   // line 60

        std::vector<char> buffer(static_cast<std::size_t>(tamanho));                // zero-filled
        // wasm 598 (result ignored). CFile::RawRead returns a short count without throwing (it only throws when
        // fread returns 0 without EOF, or for a null buffer - which is what an empty file gets), so a short read
        // decodes a zero-padded tail.
        arquivo.RawRead(buffer.data(), static_cast<uedword>(buffer.size()));
        return DecodeObjectFunction<T>(buffer, "ReadFromFile de " + arquivo.GetName());
    }

    // cfileasn.h:78 - opens `fileName` ("rb") and reads it. Only exists inlined.
    template <typename T>
    static T ReadFromFile(const std::string& fileName)
    {
        if (!CSystem::IsRegularFile(fileName))                                       // wasm 412 (stat + S_IFREG)
            throw CUeIoError(EUeIoError(5952), std::format("O arquivo [{}] não existe", fileName));  // line 78

        CFile arquivo(fileName, "rb");                                               // wasm 517, FM_NORMAL
        T objeto = ReadFromFile<T>(arquivo);
        arquivo.Close();                                                             // wasm 336 (then ~CFile)
        return objeto;
    }

    // Converter-based overload (no srcloc of its own; name inferred). The converter is a local object
    // (its vptr is stored in the callee frame), then Desconverte/Deconverte validates the entity.
    // (The ecourna IConversorASN spells the method "Deconverte" - iconversorasn.hpp:66 - so the 3735 instance
    // really calls conversor.Deconverte(entidade); written generically below.)
    //   wasm 3735: TConversor = ecourna::app::dados::asn::CConversorInformacaoMidia (ecourna IConversorASN,
    //              "Deconverte", iconversorasn.hpp:66, EApiAsnError 1902)
    //   wasm 5706: TConversor = comum::asn::CConversorCabecalhoPacote (comum IConversorASN::Desconverte,
    //              iconversorasn.h:71, EUeComumAsnError 7654 - not inlined here)
    template <typename TConversor>
    static typename TConversor::TDado ReadDataFromFile(const std::string& fileName)   // name inferred
    {
        const TConversor conversor;
        const auto entidade = ReadFromFile<typename TConversor::TEntidade>(fileName);
        return conversor.Desconverte(entidade);
    }

    // cfileasn.h:135 / :143 - decodes BER bytes and requires a valid object.
    // wasm 5825 (T = EntidadeRegistroDigitalVoto; the caller-side "DecodeObject de " + typeid(T).name()
    // context is built inside the same function, so 5825 is really DecodeObject<T>(buffer)).
    template <typename T>
    static T DecodeObjectFunction(const std::vector<char>& buffer, const std::string& contexto)
    {
        T objeto;
        ASN1::CoderEnv env;
        env.set_encodingRule(ASN1::CoderEnv::ber);                                   // first word = 1 (docs/libraries/asn1-runtime.md §5)

        if (!env.decode(buffer.data(), buffer.data() + buffer.size(), objeto)) {    // wasm 679
            std::ostringstream detalhe;
            ASN1::trace_invalid(detalhe, "", objeto);                                // wasm 208
            throw CUeIoError(EUeIoError(5953),
                             std::format("Conteúdo não foi decodificado para {}: {}", contexto, detalhe.str()));   // line 135
        }
        if (!objeto.isValid() || !objeto.isStrictlyValid()) {                        // wasm 221 / 231
            std::ostringstream detalhe;
            ASN1::trace_invalid(detalhe, "", objeto);
            throw CUeIoError(EUeIoError(5954),
                             std::format("Conteúdo inválido para {}: {}", contexto, detalhe.str()));             // line 143
        }
        return objeto;
    }

    // wasm 5825 as a whole (name inferred for the wrapper).
    template <typename T>
    static T DecodeObject(const std::vector<char>& buffer)
    {
        return DecodeObjectFunction<T>(buffer, "DecodeObject de " + std::string(typeid(T).name()));
    }

    // ---------------------------------------------------------------------------------------------
    // Writing
    // ---------------------------------------------------------------------------------------------

    // cfileasn.h:161 / :171 - BER-encodes a valid object into `buffer` (replacing its content).
    // wasm 2927 (merged body; its two last parameters are the srclocs of the instance) and
    // thunks 9673 / 9695 / 9702 / 9801. Also inlined in 6006, 5366, 11587 and in the RDV serialiser.
    // Note: here only isValid() is checked (not isStrictlyValid()).
    template <typename T>
    static void CodeObjectFunction(std::vector<char>& buffer, const T& objeto, const std::string& contexto)
    {
        if (!objeto.isValid()) {
            std::ostringstream detalhe;
            ASN1::trace_invalid(detalhe, "", objeto);
            throw CUeIoError(EUeIoError(5955),
                             std::format("Objeto com conteúdo inválido para {}: {}", contexto, detalhe.str()));   // line 161
        }

        ASN1::CoderEnv env;
        env.set_encodingRule(ASN1::CoderEnv::ber);                                   // 2927: f[74] = 1
        std::vector<char> codificado;
        if (!env.encode(objeto, std::back_inserter(codificado)))                     // wasm 9770 (6006/5366: encodeBER, 1532)
            throw CUeIoError(EUeIoError(5956), "Arquivo não foi codificado para " + contexto);                   // line 171
        buffer.swap(codificado);
    }

    template <typename T>
    static std::vector<char> CodeObject(const T& objeto)                             // name inferred (inlined in 11587 / 11488)
    {
        std::vector<char> buffer;
        CodeObjectFunction(buffer, objeto, "CodeObject de " + std::string(typeid(T).name()));
        return buffer;
    }

    // wasm 2891 (merged body, unit u12 fragment): encodes and writes to an open file.
    template <typename T>
    static void WriteToFile(const CFile& arquivo, const T& objeto)
    {
        std::vector<char> buffer;
        CodeObjectFunction(buffer, objeto, "WriteToFile de " + arquivo.GetName());
        arquivo.RawWrite(buffer.data(), static_cast<uedword>(buffer.size()));        // wasm 1886
    }

    // wasm 2892 (merged body, unit u12 fragment) + thunks 9985/10008/10040/10168.
    template <typename T>
    static void WriteToFile(const std::string& fileName, const T& objeto)
    {
        CFile arquivo(fileName, "wb");
        WriteToFile(arquivo, objeto);
    }

    // Converter-based overload (no srcloc of its own; name inferred): model object -> ASN.1 entity -> file.
    //   wasm 6006 (merged, comum converters) with thunks 2724 (CConversorEnvelopeGenerico) and
    //   5367 (CConversorEntidadeHashes): Converte = DoConverte (vtable slot 2) + "Entidade deixada em estado
    //   inválido: {}" (EUeComumAsnError 7653, iconversorasn.h:56; trace prefix = typeid(TEntidade).name()).
    //   wasm 5366: ecourna CConversorResultadoUrnaCadastro; ecourna Converte (iconversorasn.hpp:49) throws
    //   EApiAsnError 1900 with typeid(TEntidade).name() + ": " + trace.
    template <typename TConversor>
    static void WriteDataToFile(const CFile& arquivo, const typename TConversor::TDado& dado)   // name inferred
    {
        const TConversor conversor;
        const auto entidade = conversor.Converte(dado);
        WriteToFile(arquivo, entidade);
    }
};

} // namespace api

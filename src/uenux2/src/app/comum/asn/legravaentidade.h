// uenux2/src/app/comum/asn/legravaentidade.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// "Lê / grava entidade" (read / write an ASN.1 entity). Header-only helpers, in an anonymous namespace, that read
// ONE BER-encoded element stored at a known byte range of a big data file, without decoding the whole file:
//   * LeEntidadeEm          - candidate photos: one FotoCandidato inside <...>-fo.dat (index built by CVisitanteFoto)
//   * LeEntidadeBiometria   - voter fingerprints: one BiometriaEleitorCifrada inside the voter file (index in CEleitor)
// The byte range is a md::CIndexer {offset, tamanho} computed when the file was indexed at start-up.
//
// Only the srcloc records of the two templates survive (lines 92/98 and 119/125); both are fully inlined into their
// only callers:
//   LeEntidadeEm<CConversorFotoCandidato>         -> wasm 3755 comum::CFotos::GetImagem (cfotos.cpp, see cfotos.u21.cpp)
//   LeEntidadeBiometria<CConversorBiometriaEleitorCifrada> -> wasm 1937 comum::CEleitorDetalhe::GetBiometria (other unit)
// No "Grava" function survives in the binary.
#pragma once

#include <format>
#include <string>
#include <vector>

#include "api/io/asn/cfileasn.h"                  // api::CFileASN::DecodeObjectFunction<T>
#include "comum/asn/iconversorasn.h"              // CUeComumAsnError
#include "comum/dados/md/cindexer.h"              // comum::md::CIndexer {+0 offset, +4 tamanho}   // names inferred
#include "comum/dados/md/cparametro.h"
#include "ecourna/api/io/cfile.hpp"               // ecourna::api::io::CFile

namespace {

template <typename CONVERSOR>
typename CONVERSOR::TDado LeEntidadeEm(const std::string& arquivo, const comum::md::CIndexer& indice,
                                       const std::string& contexto)
{
    using TEntidade = typename CONVERSOR::TEntidade;
    using comum::CUeComumAsnError;
    using comum::EUeComumAsnError;

    ecourna::api::io::CFile file(arquivo, "rb");                                  // func 517
    if (!file.Seek(indice.GetOffset(), SEEK_SET)) {
        throw CUeComumAsnError(EUeComumAsnError(7667),
            std::format("CLeitorASN::{} - não foi possível fazer o seek em {}", contexto, arquivo));  // line 92
    }
    std::vector<char> buffer(indice.GetTamanho());
    if (file.RawRead(buffer.data(), buffer.size()) != buffer.size()) {
        throw CUeComumAsnError(EUeComumAsnError(7668),
            std::format("CLeitorASN::{} - não foi possível ler a entidade de {}", contexto, arquivo)); // line 98
    }
    const CONVERSOR conversor;
    // cfileasn.h:135 "Conteúdo não foi decodificado para {}: {}" (EUeIoError 5953) / :143 "Conteúdo inválido
    // para {}: {}" (5954) are thrown from here (inlined).
    const TEntidade entidade = api::CFileASN::DecodeObjectFunction<TEntidade>(
        buffer, "DecodeObject de " + std::string(typeid(TEntidade).name()));
    return conversor.Desconverte(entidade);                                       // iconversorasn.h:71 inlined
}   // ~CFile closes the file

template <typename CONVERSOR>
typename CONVERSOR::TDado LeEntidadeBiometria(const std::string& arquivo, const comum::md::CParametro& parametro,
                                              const comum::md::CIndexer& indice, const std::string& contexto)
{
    using TEntidade = typename CONVERSOR::TEntidade;
    using comum::CUeComumAsnError;
    using comum::EUeComumAsnError;

    ecourna::api::io::CFile file(arquivo, "rb");
    if (!file.Seek(indice.GetOffset(), SEEK_SET)) {
        throw CUeComumAsnError(EUeComumAsnError(7669),
            std::format("CLeitorASN::{} - não foi possível fazer o seek em {}", contexto, arquivo));  // line 119
    }
    std::vector<char> buffer(indice.GetTamanho());
    if (file.RawRead(buffer.data(), buffer.size()) != buffer.size()) {
        throw CUeComumAsnError(EUeComumAsnError(7670),
            std::format("CLeitorASN::{} - não foi possível ler a entidade de {}", contexto, arquivo)); // line 125
    }
    const CONVERSOR conversor;
    const TEntidade entidade = api::CFileASN::DecodeObjectFunction<TEntidade>(
        buffer, "DecodeObject de " + std::string(typeid(TEntidade).name()));
    return conversor.Desconverte(entidade, parametro);                            // iconversorbiometriaasn.h:62
}

} // namespace

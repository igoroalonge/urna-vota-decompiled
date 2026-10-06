// uenux2/src/app/comum/gravadores/igravador.cpp   (path inferred: class comum::IGravador is known from RTTI only;
// its base IResultado lives in the attested gravadores/iresultado.cpp and every writer in gravadores/)
// Reconstructed from vota_web_wasm.wasm (unit u35). The class is declared in iresultado.h (unit u23).
//
// IGravador = the writer of ONE result file of the urna ("arquivo de resultado": bu.dat, rdv.dat, jufa.dat,
// imgbu.dat, imgze.dat, hash.dat, wsq*.jez, mr.ver, log.jez). The file name (IResultado::m_nome, +20) is
//   "<fase o|s|t><pleito:05><uf><município:05><zona:04><seção:04>-<sufixo>"      e.g. "t02410ac0000100010001-bu.dat"
// (CGravadorUtil::DeterminaNomeArquivo). The five non-pure virtual methods below are inherited unchanged by
// CGravadorBU, CGravadorRDV, CGravadorRCSecao, CGravadorHashes, CGravadorLog, CGravadorVersoesArquivos,
// IGravadorEnvelope and CGravadorEnvelopeArquivo (same table slots 2187..2191 in all their vtables). CGravadorWSQ
// is the exception: it overrides all five (slot 2 = func 11579 on resultado(MI)/<nome> and slots 3/6 = funcs
// 11577/11578 -> comum_f6041(this, 0|1) all run the WSQ packaging body func 5821 (glob "*.wsq", cgravadorwsq.cpp:175/
// 192); slots 4/5 and 7 = the empty ICF bodies 218/425), so its Grava() writes nothing. Slot 7
// GravaResultado(CFile&) is the per-file encoder.
//
// Directories (comum::CPath, unit u22): EFlashOrigem 0 = MI ("memória interna", /dsk/fi), 1 = MV ("memória de
// votação", the removable card /dsk/fe). "trab" = dinamico/trab<turno>/ (work area), "resultado" = dinamico/
// res<turno>/ (final results). GetPathTrab(origem) = func 436, GetPathResult(origem) = func 1274 (both use the
// turno recorded in eg.bin).
//
// None of these functions ran during the recorded votes: the web page never reaches the encerramento (end of
// voting), where vota::CGravaResultado::StartState (func 12098, unit u07) calls them in this order:
//   Grava() for every writer, CopiaParaResultado(), signature, then (when the MV is present) CopiaParaMV().
#include "comum/gravadores/iresultado.h"

#include <filesystem>
#include <string>

#include "api/util/csystem.h"            // api::CSystem::CopyFile (func 378)
#include "comum/cpath.h"
#include "ecourna/api/io/cfile.hpp"      // ecourna::api::io::CFile (ctor = func 517, Close = func 336)

namespace comum {

// wasm func 11632 - vtable slot 2 (IResultado::CopiaParaResultado, pure in IResultado).     name inferred (u23)
// trab(MI)/<nome>  ->  resultado(MI)/<nome>
void IGravador::CopiaParaResultado() const
{
    const std::filesystem::path origem  = CPath::GetPathTrab(EFlashOrigem::INTERNA) / GetNome();
    const std::filesystem::path destino = CPath::GetPathResult(EFlashOrigem::INTERNA) / GetNome();
    api::CSystem::CopyFile(origem.string(), destino.string(), false);
}

// wasm func 11630 - vtable slot 3 (IResultado::CopiaParaMV, pure in IResultado).            name inferred (u23)
// Two copies: the work copy and the final copy, both from the MI to the MV.
//   trab(MI)/<nome>      -> trab(MV)/<nome>
//   resultado(MI)/<nome> -> resultado(MV)/<nome>
void IGravador::CopiaParaMV() const
{
    const std::filesystem::path trabMI = CPath::GetPathTrab(EFlashOrigem::INTERNA) / GetNome();
    const std::filesystem::path trabMV = CPath::GetPathTrab(EFlashOrigem::EXTERNA) / GetNome();
    api::CSystem::CopyFile(trabMI.string(), trabMV.string(), false);

    const std::filesystem::path resultadoMI = CPath::GetPathResult(EFlashOrigem::INTERNA) / GetNome();
    const std::filesystem::path resultadoMV = CPath::GetPathResult(EFlashOrigem::EXTERNA) / GetNome();
    api::CSystem::CopyFile(resultadoMI.string(), resultadoMV.string(), false);
}

// wasm func 11634 - vtable slot 4.                                                          name inferred (u23)
// Creates (truncates) trab(MI)/<nome> and lets the concrete writer encode the file into it.
void IGravador::Grava() const
{
    ecourna::api::io::CFile arquivo((CPath::GetPathTrab(EFlashOrigem::INTERNA) / GetNome()).string(), "wb");
    GravaResultado(arquivo);                                    // slot 7 (pure here)
    arquivo.Close();
}                                                               // ~CFile: Close() again only if still open

// wasm func 11633 - vtable slot 5. Same as Grava() but on the MV.                            name inferred (u23)
// vota::CGravaResultado (func 12098), the only place that drives the writers, calls slots 2, 3 and 4 only; the
// VOTA application copies to the MV with CopiaParaMV instead. (CGravadorWSQ overrides slots 4 and 5 with a no-op.)
void IGravador::GravaMV() const
{
    ecourna::api::io::CFile arquivo((CPath::GetPathTrab(EFlashOrigem::EXTERNA) / GetNome()).string(), "wb");
    GravaResultado(arquivo);
    arquivo.Close();
}

// wasm func 11631 - vtable slot 6. trab(MV)/<nome> -> resultado(MV)/<nome>.                name inferred (u23)
// Pairs with GravaMV(); not called by CGravaResultado either (CGravadorWSQ overrides it, func 11578).
void IGravador::CopiaResultadoParaMV() const
{
    const std::filesystem::path origem  = CPath::GetPathTrab(EFlashOrigem::EXTERNA) / GetNome();
    const std::filesystem::path destino = CPath::GetPathResult(EFlashOrigem::EXTERNA) / GetNome();
    api::CSystem::CopyFile(origem.string(), destino.string(), false);
}

} // namespace comum

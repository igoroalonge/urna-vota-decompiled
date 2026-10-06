// uenux2/src/api/io/asn/cfileasn.h  -- the template instances that survive as wasm functions.
//
// Reconstructed from vota_web_wasm.wasm. Unit u18 (companion of cfileasn.h; this .cpp does not exist in the
// original tree: it only lists, as explicit instantiations, which CFileASN instances became functions).
#include "api/io/asn/cfileasn.h"

#include "ModuloEstadoGeralGap.h"
#include "ModuloEstadoGeralSA.h"
#include "ModuloEstadoGeralUrna.h"
#include "ModuloEstadoGeralVota.h"
#include "ModuloRegistroDigitalVoto.h"
#include "comum/asn/cconversorcabecalhopacote.h"                         // comum::asn::CConversorCabecalhoPacote
#include "comum/gravadores/asn/cconversorenvelopegenerico.h"
#include "comum/gravadores/asn/cconversorentidadehashes.h"               // path inferred
#include "ecourna/app/dados/asn/midias/cconversorinformacaomidia.hpp"
#include "ecourna/app/dados/asn/resultadournacadastro/cconversorresultadournacadastro.hpp"   // path inferred

namespace api {

// ------------------------------------------------------------------------------------------------
// CodeObjectFunction<T>: wasm 2927 is the merged body (it takes the srclocs of lines 171 and 161 as
// two extra parameters); the thunks below are what the state writers (CFileASN::WriteToFile, u12)
// call through function pointers (table slots 490/489/488/478).
// ------------------------------------------------------------------------------------------------
// wasm func 9673 (srcloc lines 161/171) - vota.bin
template void CFileASN::CodeObjectFunction(std::vector<char>&, const ModuloEstadoGeralVota::EstadoGeralVota&, const std::string&);
// wasm func 9695 (srcloc lines 161/171) - sa.bin
template void CFileASN::CodeObjectFunction(std::vector<char>&, const ModuloEstadoGeralSA::EstadoGeralSA&, const std::string&);
// wasm func 9702 (srcloc lines 161/171) - gap.bin
template void CFileASN::CodeObjectFunction(std::vector<char>&, const ModuloEstadoGeralGap::EstadoGeralGap&, const std::string&);
// wasm func 9801 (srcloc lines 161/171) - eg.bin
template void CFileASN::CodeObjectFunction(std::vector<char>&, const ModuloEstadoGeralUrna::EstadoGeralUrna&, const std::string&);
// wasm func 9790: std::__format_arg_store<std::format_context, const std::string, std::string> constructor
// (the two {} of the line-161 message), libc++ - not reconstructed.

// ------------------------------------------------------------------------------------------------
// DecodeObject<T>: wasm 5825 = DecodeObjectFunction (lines 135/143) + the "DecodeObject de " context.
// Callers: comum::asn::IConversorASN<EntidadeRegistroDigitalVoto, CVotosEleicoesVota>::Desconverte (5680,
// reading rdv.dat back) and comum::CGravadorRDV::GravaResultado (11587, re-decoding the RDV to embed it).
// ------------------------------------------------------------------------------------------------
// wasm func 5825 (srcloc lines 135, 143)
template ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto
CFileASN::DecodeObject<ModuloRegistroDigitalVoto::EntidadeRegistroDigitalVoto>(const std::vector<char>&);

// ------------------------------------------------------------------------------------------------
// WriteDataToFile<TConversor> (name inferred): converter + WriteToFile(const CFile&, entity).
// ------------------------------------------------------------------------------------------------
// wasm func 6006 = merged body for comum converters. Extra parameters: srcloc 171, srcloc 161,
// srcloc iconversorasn.h:56, typeid(TEntidade).name() (trace prefix) and the converter's vtable.
// wasm func 2724 (thunk, srclocs 161/171 + iconversorasn.h:56): envelope files. Callers:
//   comum::CGravadorBU::GravaResultado (11629, "-bu.dat"), comum::IGravadorEnvelope::GravaResultado
//   (11626, "-imgbu.dat"/"-imgze.dat") and func 2725.
template void CFileASN::WriteDataToFile<comum::asn::CConversorEnvelopeGenerico>(
    const CFile&, const comum::md::CEnvelopeGenerico&);
// wasm func 5367 (thunk): comum::CGravadorHashes::GravaResultado (11620, "-hash.dat").
template void CFileASN::WriteDataToFile<comum::asn::CConversorEntidadeHashes>(
    const CFile&, const comum::md::CEntidadeHashes&);
// wasm func 5366 (srcloc iconversorasn.hpp:49 + cfileasn.h:161/171): comum::CGravadorRCSecao::GravaResultado
// (11616, "-jufa.dat"). The ecourna converter throws EApiAsnError 1900 (message = typeid name + ": " + trace).
template void CFileASN::WriteDataToFile<ecourna::app::dados::asn::CConversorResultadoUrnaCadastro>(
    const CFile&, const ecourna::app::dados::CResultadoUrnaCadastro&);

// ------------------------------------------------------------------------------------------------
// ReadDataFromFile<TConversor> (name inferred): ReadFromFile(const std::string&) + converter.
// ------------------------------------------------------------------------------------------------
// wasm func 3735 (srclocs 78, 48, 60, 135, 143 + ecourna iconversorasn.hpp:66): "infomidia.dat" of the
// external flash (MV). Callers: CGravadorRDV / IGravadorEnvelope / CGravadorBU ::GravaResultado, which
// take numeroSerieFV (the MV serial) from it. In the simulator the file does not exist (the scenario
// ships "infomidia-fv-<turno>-t.dat" instead), so the serial stays "00000000".
template ecourna::app::dados::CInformacaoMidia
CFileASN::ReadDataFromFile<ecourna::app::dados::asn::CConversorInformacaoMidia>(const std::string&);
// wasm func 5706 (srclocs 78, 48, 60, 135, 143): ModuloTiposEleitorais::CabecalhoPacote -> md::CCabecalhoPacote
// (Desconverte not inlined). Callers: comum::asn::LePleito (5778, the version of each eleição's data
// package) and the start-up loader 7787.
template comum::md::CCabecalhoPacote
CFileASN::ReadDataFromFile<comum::asn::CConversorCabecalhoPacote>(const std::string&);

} // namespace api

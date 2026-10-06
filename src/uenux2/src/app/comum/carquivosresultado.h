// uenux2/src/app/comum/carquivosresultado.h
// Reconstructed from vota_web_wasm.wasm (unit u21).
//
// Suffixes of the RESULT files the urna writes at the end of the day (encerramento) and copies to the result medium
// (Memória de Resultado, "MR"). The full name is built by the callers (CGravadorUtil::DeterminaNomeArquivoSemLetra,
// CCopiaResultadoParaMR, ...) as  <fase><pleito:05><uf?><município:05><zona:04><seção:04>-<suffix>,
// e.g. "o00406sp71072000100123-bu.dat".
#pragma once

#include <string>

namespace comum {

// Enumerator names inferred from the suffix. Values 1..21 (0 and >21 are rejected).
enum class EExtensaoArquivoResultado : int {
    AssinaturaVota = 1,          // "vota.vsc"    signatures of the VOTA result files
    AssinaturaSA = 2,            // "sa.vsc"      signatures of the SA (Sistema de Apuração) files
    AssinaturaRed = 3,           // "red.vsc"     signatures of the "red" (Registro/Resultado em Divergência?) files   // ?
    BU = 4,                      // "bu.dat"      Boletim de Urna (EntidadeResultadoUrna / EntidadeBoletimUrna)
    BUSA = 5,                    // "busa.dat"    BU produced by the SA (contingency counting)
    RDV = 6,                     // "rdv.dat"     Registro Digital do Voto
    RDVRed = 7,                  // "rdvred.dat"
    Justificativas = 8,          // "jufa.dat"    attendance / justifications (EntidadeResultadoUrnaCadastro)
    ImagemBU = 9,                // "imgbu.dat"   image of the printed BU (envelope)
    ImagemBUSA = 10,             // "imgbusa.dat"
    ImagemZeresima = 11,         // "imgze.dat"   image of the zerésima
    Hashes = 12,                 // "hash.dat"    EntidadeHashes of the result files
    Log = 13,                    // "log.jez"     application log
    Log2 = 14,                   // "log.jez"     (same suffix as 13)   // ?
    LogSA = 15,                  // "logsa.jez"
    WsqBiometria = 16,           // "wsqbio.jez"  fingerprints (WSQ) of voters
    WsqManual = 17,              // "wsqman.jez"  fingerprints collected by the mesário ("manual")   // ?
    WsqMesarios = 18,            // "wsqmes.jez"  fingerprints of poll workers
    VersaoMR = 19,               // "mr.ver"      version of the result medium
    AssinaturaSW = 20,           // "asw.vsc"     ?
    AssinaturaHW = 21,           // "ahw.vsc"     ?
};

// Singleton (GetInst = func 348 -> 2900, seen by u22). No data member is ever read: wasm-opt removed the unused
// `this` parameter of operator[] (dead-argument elimination).
class CArquivosResultado
{
public:
    static const CArquivosResultado& GetInst();                                // wasm func 348 -> 2900 (other unit)
    const std::string operator[](EExtensaoArquivoResultado extensao) const;   // wasm func 347 (srcloc line 99)
};

} // namespace comum

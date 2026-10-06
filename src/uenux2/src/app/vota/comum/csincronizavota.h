// uenux2/src/app/vota/comum/csincronizavota.h   (path inferred from csincronizavota.cpp, attested by
// std::source_location records :47, :290, :303, :392)
// Reconstructed from vota_web_wasm.wasm (unit u26 = owner; fragments by u02 and u07 in
// csincronizavota.u02.cpp / csincronizavota.u07.cpp).
//
// vota::CSincronizaVota ("synchronise the vote data") keeps the urna's two flash memories consistent:
// every file written in the internal memory (MI = /dsk/fi) is signed by SAVD and copied, together with its
// signature package (.vsu), to the external memory / memory card (MV = /dsk/fe). It also owns the global
// "the urna is being switched off" flag that stops any further write.
//
// Static class (no instance, no RTTI).
#pragma once

#include <string>

namespace vota {

class CSincronizaVota {
public:
    /// csincronizavota.cpp:47. Throws api::CUeDesligandoError(4201, "Urna desligando") when the shutdown flag
    /// is set. The throw-expression is out of line (wasm func 1685: builds the exception object); the test
    /// is inlined in every caller (funcs 491, 1836, 3333, 4662, 6737, 7174 ...).
    static void VerificaUrnaDesligando();

    /// wasm func 3336 (name inferred). Sets the shutdown flag. Callers: CThreadMonitor::Run (power key
    /// turned off) and CThreadVota::TrataExcecao* (fatal error).
    static void MarcaUrnaDesligando() { ms_desligando = true; }

    /// Inline read of the flag (CThreadVota::TrataExcecao*).                      name inferred
    static bool UrnaDesligando() { return ms_desligando; }

    /// csincronizavota.cpp:392 - wasm func 1836. Signs one report file of the current work directory of the
    /// MI and copies the file and its signature package to the MV. Only the six report files below are
    /// accepted; callers pass the name returned by comum::CArquivosSavd for the id:
    ///   bu.dat (86, BU), buj.dat (84, BU de justificativas), ze.dat (88, zerésima), rze.dat (89, resumo da
    ///   zerésima), bim.dat (106, boletim de identificação de mesários), behb.dat (108, eleitores habilitados
    ///   biograficamente).
    /// Callers: vota::CGeraBU (12110), CGeraRelatorios (12105), CGeraZeresimaBase (11946),
    /// CGeraResumoZeresimaBase.
    static void SincronizaRelatorios(const std::string& arquivo);

    // Other members reconstructed by units u02/u07 (csincronizavota.u02.cpp / .u07.cpp):
    //   SincronizaBancoDados() (func 4662), SincronizaRDVInterno()/SincronizaRDVExterno() (inlined in 7174).

private:
    static inline bool ms_desligando = false;          // byte @1832936          name as in unit u02
};

} // namespace vota

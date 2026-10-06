// FRAGMENT of uenux2/mock/app/comum/cappinfobuilder.h (path inferred from cappinfobuilder.cpp) reconstructed by
// unit u28 from vota_web_wasm.wasm. Only the members used by SalvaGeral / SalvaApps / SalvaApp are declared
// here; the constructor, the fluent setters and the helpers that rebuild the estados belong to units u29/u30.
//
// comum::teste::CAppInfoBuilder is a TEST FIXTURE of the urna code base (directory uenux2/mock/, namespace
// "teste") that the web build uses in production: votaInit (func 7840) fills one with hard-coded values
// (versão "7.2.1.3 - TESTE EG ASN1"; fase '1'/'2'/'3' from the JSON field "fase" = "oficial"|"o" / "simulado"|"s" /
// anything else; turno '1' or '2'; the scenario's município/zona/seção) and then calls
//     builder.SalvaGeral(false, {MI, MV})
//            .SalvaApps(false, {MI, MV}, '1', {GAP, SA, VOTA})
//            .SalvaApps(false, {MI, MV}, '2', {GAP, SA, VOTA});          (wasm_entry_f10256, unit u30)
// to create the urna's persistent state files on both flash memories:
//     /dsk/fi|fe/dinamico/eg.bin                        CEstadoGeral      (ModuloEstadoGeralUrna)
//     /dsk/fi|fe/dinamico/trab1|trab2/gap.bin           CEstadoGeralGap   (ModuloEstadoGeralGap)
//     /dsk/fi|fe/dinamico/trab1|trab2/sa.bin            CEstadoGeralSA    (ModuloEstadoGeralSA)
//     /dsk/fi|fe/dinamico/trab1|trab2/vota.bin          CEstadoGeralVota  (ModuloEstadoGeralVota)
// On a real urna these files are produced by the carga (media preparation) and by the applications
// themselves; in the simulator they are fabricated here.
#pragma once

#include <cstddef>
#include <source_location>
#include <string>
#include <vector>

#include "comum/appinfo/cappinfo.h"                                  // EUrnaTurno
#include "comum/cpath.h"                                             // EFlashOrigem
// NOTE: cpath.h (another unit) currently also declares `enum class EUrnaTurno : char {PRIMEIRO, SEGUNDO}`, which
// clashes with cappinfo.h's `enum class EUrnaTurno : int {SemTurno, Primeiro, Segundo, Atual}`. The wasm of
// SalvaApps supports the int-sized one: the turno is stored with i32.store and the enum formatter handle (9868)
// reads it back with i32.load (a char-based enum would be stored/loaded as one byte).
#include "comum/dados/md/estadoaplicacao/cestadogeral.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralgap.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralsa.h"
#include "comum/dados/md/estadoaplicacao/cestadogeralvota.h"

namespace comum::teste {

// Which flash memory to write (converted 1:1 to EFlashOrigem by Converte(EMidia), func 5344).  names inferred
enum class EMidia : int { FlashInterna = 0 /* MI */, FlashExterna = 1 /* MV */ };             // namespace ?

// Which application state file SalvaApp writes. Values from the switch in func 10212.         names inferred
enum class EApp : int { Gap = 0 /* gap.bin */, SA = 1 /* sa.bin */, Vota = 2 /* vota.bin */ };  // namespace ?

class CAppInfoBuilder {                                           // 500 bytes (vota[1] ends at +500)
public:
    // ... constructor (func 10268) and fluent setters (10197, 10188, 10181, 10176): unit u30

    CAppInfoBuilder& SalvaGeral(bool assina, std::vector<EMidia> midias);                         // func 10243 (:112)
    CAppInfoBuilder& SalvaApps(bool assina, std::vector<EMidia> midias, EUrnaTurno turno,
                               std::vector<EApp> apps);                                             // func 10212 (:277)
    CAppInfoBuilder& SalvaApp(bool assina, EFlashOrigem origem, EUrnaTurno turno, EApp app);      // inlined (:287)

private:
    // The four estados are kept by value, one per turno for the per-turno files. Their sizes are exactly
    // those of the md classes (CAppInfo caches EG/GAP/VOTA in std::optional of 184/48/104 bytes), and the
    // builder's constructor and the temporaries of SalvaApps share the same destructors (2793, 1698).
    // Before saving, each one is rebuilt through its field-wise md constructor (funcs 10205 -> ctor 5637,
    // 10123 -> 5626, 10101 -> 5625, 10067 -> 5628; unit u30), which e.g. recomputes the gap's data2T flag.
    md::estadoaplicacao::CEstadoGeral     m_geral;      // +0    180 B  (dtor 2793: +8 std::string, +60 sub-object
                                                        //       with its own dtor 857, +156 versão, +168 vector)
    md::estadoaplicacao::CEstadoGeralGap  m_gap[2];     // +180  44 B each, index = Converte(turno)
    md::estadoaplicacao::CEstadoGeralSA   m_sa[2];      // +268  16 B each
    md::estadoaplicacao::CEstadoGeralVota m_vota[2];    // +300  100 B each (votaInit sets +0 = '1', +72 = true)

    // Helpers defined in cappinfobuilder.cpp by other units (names inferred):
    static md::estadoaplicacao::CEstadoGeral     CriaEstado(const md::estadoaplicacao::CEstadoGeral&);      // 10205
    static md::estadoaplicacao::CEstadoGeralGap  CriaEstado(const md::estadoaplicacao::CEstadoGeralGap&);   // 10123
    static md::estadoaplicacao::CEstadoGeralSA   CriaEstado(const md::estadoaplicacao::CEstadoGeralSA&);    // 10101
    static md::estadoaplicacao::CEstadoGeralVota CriaEstado(const md::estadoaplicacao::CEstadoGeralVota&);  // 10067
};

// Free helpers of cappinfobuilder.cpp (anonymous namespace or static; names from their error texts).
// Both take the CALLER's std::source_location as a default argument, and only use its line() in the message.
EFlashOrigem Converte(EMidia midia,
                      const std::source_location& local = std::source_location::current());     // func 5344 (u29)
std::size_t Converte(EUrnaTurno turno,
                     const std::source_location& local = std::source_location::current());      // inlined in 10212

// Writes `conteudo` to the file `arquivo`. func 10293 (u30).                                     name inferred
// Body: std::filesystem::create_directories(path(arquivo).parent_path()) (api_f12121 -> 2548), then
// std::ofstream(arquivo, std::ios::binary) - filebuf::open mode 20 = out|binary (libc++: binary 0x04, out 0x10;
// no explicit trunc, but "out" alone opens with "wb", which truncates) - setstate(failbit) if it fails, then
// `ofs << conteudo`.
void EscreveArquivo(std::string arquivo, std::string conteudo);

}  // namespace comum::teste

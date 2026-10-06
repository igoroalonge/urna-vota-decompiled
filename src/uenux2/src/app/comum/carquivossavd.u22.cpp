// uenux2/src/app/comum/carquivossavd.cpp  --  FRAGMENT written by unit u22
// (the operator[] accessors, funcs 275 and 680, are in carquivossavd.cpp by unit u21; merge there).
//
// This fragment holds the singleton accessor and the constructor of comum::CArquivosSavd, which only
// exist as wasm func 1164 (77,002 bytes, 35,493 instructions: the constructor is fully inlined into
// GetInst, and every std::filesystem::path / std::string temporary of the ~190 insertions below is
// expanded in place). Observed executing (first call during votaInit).
//
// The table contents below were extracted from the code and cross-checked against the live object:
// tools/run/headless.mjs was run with a hook that walks the three std::map trees of the singleton
// (@1838544) after votaInit; the trailing comment of each m_pacotes line is the value read back.
//
// Object layout (36 bytes, three std::map, 12 bytes each; node = {left,right,parent,color,key +16,
// value std::string +20}):
//   +0  std::map<ESavdPacote, std::string>     m_pacotes     ids 120..203: full path of each signature
//                                                             package (.vsu on the urna, .vsc on results)
//   +12 std::map<ESavdArquivoUE, std::string>  m_arquivos    ids 25..115: name (or std::format pattern)
//                                                             of each signed file
//   +24 std::map<ESavdAplicacao, std::string>  m_aplicacoes  ids 1..22: SAVD application / key names.
//                                                             No accessor reads this map in this build.
//                                                             (member and enum names inferred)
// PREFIXO is CGravadorUtil's result-file prefix "{:c}{:05}{}{:05}{:04}{:04}-" (fase, pleito, UF,
// município, zona, seção; literal @378043 / @377019+1024) and `resultado` is
// CArquivosResultado::GetInst() (func 348 -> 2900), whose operator[] (func 347) maps
// EExtensaoArquivoResultado to a suffix: 1 "vota.vsc", 2 "sa.vsc", 3 "red.vsc", 4 "bu.dat",
// 5 "busa.dat", 6 "rdv.dat", 7 "rdvred.dat", 8 "jufa.dat", 9 "imgbu.dat", 10 "imgbusa.dat",
// 11 "imgze.dat", 12 "hash.dat", 13 "log.jez", 15 "logsa.jez", 16 "wsqbio.jez", 17 "wsqman.jez",
// 18 "wsqmes.jez".
#include "comum/carquivossavd.h"

#include <memory>
#include <mutex>

#include "comum/carquivosresultado.h"
#include "comum/cpath.h"

namespace comum {

namespace {
constexpr auto INTERNA = EFlashOrigem::INTERNA;
constexpr auto EXTERNA = EFlashOrigem::EXTERNA;
constexpr auto T1 = EUrnaTurno::PRIMEIRO;
constexpr auto T2 = EUrnaTurno::SEGUNDO;
const std::string PREFIXO = "{:c}{:05}{}{:05}{:04}{:04}-";

std::mutex s_mutex;                               // @1838520 (only the unlock stub remains)
std::unique_ptr<CArquivosSavd> s_instancia;       // @1838544; reset at exit by func 11658
} // namespace

// wasm func 1164 (GetInst with the constructor inlined)                              // name inferred
const CArquivosSavd& CArquivosSavd::GetInst()
{
    std::lock_guard trava(s_mutex);
    if (!s_instancia)
        s_instancia.reset(new CArquivosSavd());
    return *s_instancia;
}

// Constructor (inlined into func 1164). Insertion order as in the binary.
CArquivosSavd::CArquivosSavd()
{
    const CArquivosResultado& resultado = CArquivosResultado::GetInst();   // func 348

    // ---- m_pacotes: signature packages ----------------------------------------------------------
    m_pacotes[ESavdPacote{120}] = CPath::GetPathDinamico(INTERNA) / "eg.vsu";   // /dsk/fi/dinamico/eg.vsu
    m_pacotes[ESavdPacote{121}] = CPath::GetPathDinamico(EXTERNA) / "eg.vsu";   // /dsk/fe/dinamico/eg.vsu
    m_pacotes[ESavdPacote{146}] = CPath::GetPathTrab(INTERNA, T1) / "gap.vsu";   // /dsk/fi/dinamico/trab1/gap.vsu
    m_pacotes[ESavdPacote{147}] = CPath::GetPathTrab(INTERNA, T2) / "gap.vsu";   // /dsk/fi/dinamico/trab2/gap.vsu
    m_pacotes[ESavdPacote{148}] = CPath::GetPathTrab(EXTERNA, T1) / "gap.vsu";   // /dsk/fe/dinamico/trab1/gap.vsu
    m_pacotes[ESavdPacote{149}] = CPath::GetPathTrab(EXTERNA, T2) / "gap.vsu";   // /dsk/fe/dinamico/trab2/gap.vsu
    m_pacotes[ESavdPacote{122}] = CPath::GetPathTrab(INTERNA, T1) / "vota.vsu";   // /dsk/fi/dinamico/trab1/vota.vsu
    m_pacotes[ESavdPacote{123}] = CPath::GetPathTrab(INTERNA, T2) / "vota.vsu";   // /dsk/fi/dinamico/trab2/vota.vsu
    m_pacotes[ESavdPacote{124}] = CPath::GetPathTrab(EXTERNA, T1) / "vota.vsu";   // /dsk/fe/dinamico/trab1/vota.vsu
    m_pacotes[ESavdPacote{125}] = CPath::GetPathTrab(EXTERNA, T2) / "vota.vsu";   // /dsk/fe/dinamico/trab2/vota.vsu
    m_pacotes[ESavdPacote{126}] = CPath::GetPathTrab(INTERNA, T1) / "bu.vsu";   // /dsk/fi/dinamico/trab1/bu.vsu
    m_pacotes[ESavdPacote{127}] = CPath::GetPathTrab(INTERNA, T2) / "bu.vsu";   // /dsk/fi/dinamico/trab2/bu.vsu
    m_pacotes[ESavdPacote{128}] = CPath::GetPathTrab(EXTERNA, T1) / "bu.vsu";   // /dsk/fe/dinamico/trab1/bu.vsu
    m_pacotes[ESavdPacote{129}] = CPath::GetPathTrab(EXTERNA, T2) / "bu.vsu";   // /dsk/fe/dinamico/trab2/bu.vsu
    m_pacotes[ESavdPacote{130}] = CPath::GetPathTrab(INTERNA, T1) / "buj.vsu";   // /dsk/fi/dinamico/trab1/buj.vsu
    m_pacotes[ESavdPacote{131}] = CPath::GetPathTrab(INTERNA, T2) / "buj.vsu";   // /dsk/fi/dinamico/trab2/buj.vsu
    m_pacotes[ESavdPacote{132}] = CPath::GetPathTrab(EXTERNA, T1) / "buj.vsu";   // /dsk/fe/dinamico/trab1/buj.vsu
    m_pacotes[ESavdPacote{133}] = CPath::GetPathTrab(EXTERNA, T2) / "buj.vsu";   // /dsk/fe/dinamico/trab2/buj.vsu
    m_pacotes[ESavdPacote{138}] = CPath::GetPathTrab(INTERNA, T1) / "ze.vsu";   // /dsk/fi/dinamico/trab1/ze.vsu
    m_pacotes[ESavdPacote{139}] = CPath::GetPathTrab(INTERNA, T2) / "ze.vsu";   // /dsk/fi/dinamico/trab2/ze.vsu
    m_pacotes[ESavdPacote{140}] = CPath::GetPathTrab(EXTERNA, T1) / "ze.vsu";   // /dsk/fe/dinamico/trab1/ze.vsu
    m_pacotes[ESavdPacote{141}] = CPath::GetPathTrab(EXTERNA, T2) / "ze.vsu";   // /dsk/fe/dinamico/trab2/ze.vsu
    m_pacotes[ESavdPacote{142}] = CPath::GetPathTrab(INTERNA, T1) / "rze.vsu";   // /dsk/fi/dinamico/trab1/rze.vsu
    m_pacotes[ESavdPacote{143}] = CPath::GetPathTrab(INTERNA, T2) / "rze.vsu";   // /dsk/fi/dinamico/trab2/rze.vsu
    m_pacotes[ESavdPacote{144}] = CPath::GetPathTrab(EXTERNA, T1) / "rze.vsu";   // /dsk/fe/dinamico/trab1/rze.vsu
    m_pacotes[ESavdPacote{145}] = CPath::GetPathTrab(EXTERNA, T2) / "rze.vsu";   // /dsk/fe/dinamico/trab2/rze.vsu
    m_pacotes[ESavdPacote{134}] = CPath::GetPathTrab(INTERNA, T1) / "rdv.vsu";   // /dsk/fi/dinamico/trab1/rdv.vsu
    m_pacotes[ESavdPacote{135}] = CPath::GetPathTrab(INTERNA, T2) / "rdv.vsu";   // /dsk/fi/dinamico/trab2/rdv.vsu
    m_pacotes[ESavdPacote{136}] = CPath::GetPathTrab(EXTERNA, T1) / "rdv.vsu";   // /dsk/fe/dinamico/trab1/rdv.vsu
    m_pacotes[ESavdPacote{137}] = CPath::GetPathTrab(EXTERNA, T2) / "rdv.vsu";   // /dsk/fe/dinamico/trab2/rdv.vsu
    m_pacotes[ESavdPacote{150}] = CPath::GetPathTrab(INTERNA, T1) / "sa.vsu";   // /dsk/fi/dinamico/trab1/sa.vsu
    m_pacotes[ESavdPacote{151}] = CPath::GetPathTrab(INTERNA, T2) / "sa.vsu";   // /dsk/fi/dinamico/trab2/sa.vsu
    m_pacotes[ESavdPacote{152}] = CPath::GetPathTrab(INTERNA, T1) / "red.vsu";   // /dsk/fi/dinamico/trab1/red.vsu
    m_pacotes[ESavdPacote{153}] = CPath::GetPathTrab(INTERNA, T2) / "red.vsu";   // /dsk/fi/dinamico/trab2/red.vsu
    m_pacotes[ESavdPacote{154}] = CPath::GetPathTrab(EXTERNA, T1) / "red.vsu";   // /dsk/fe/dinamico/trab1/red.vsu
    m_pacotes[ESavdPacote{155}] = CPath::GetPathTrab(EXTERNA, T2) / "red.vsu";   // /dsk/fe/dinamico/trab2/red.vsu
    m_pacotes[ESavdPacote{156}] = CPath::GetPathRoot("/dsk/fe/dinamico/trab/") / "tabcorr.vsc";   // /dsk/fe/dinamico/trab/tabcorr.vsc
    m_pacotes[ESavdPacote{157}] = CPath::GetPathEstatico(EXTERNA) / "{:05}{:04}{:04}-lo.vsc";   // /dsk/fe/estatico/{:05}{:04}{:04}-lo.vsc
    m_pacotes[ESavdPacote{203}] = CPath::GetPathEstatico(EXTERNA) / "dadoscarga.vsu";   // /dsk/fe/estatico/dadoscarga.vsu
    m_pacotes[ESavdPacote{158}] = ((CPath::GetPathResult(INTERNA, T1) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(1)]);   // /dsk/fi/dinamico/res1/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc
    m_pacotes[ESavdPacote{159}] = ((CPath::GetPathResult(INTERNA, T2) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(1)]);   // /dsk/fi/dinamico/res2/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc
    m_pacotes[ESavdPacote{160}] = ((CPath::GetPathResult(EXTERNA, T1) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(1)]);   // /dsk/fe/dinamico/res1/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc
    m_pacotes[ESavdPacote{161}] = ((CPath::GetPathResult(EXTERNA, T2) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(1)]);   // /dsk/fe/dinamico/res2/{:c}{:05}{}{:05}{:04}{:04}-vota.vsc
    m_pacotes[ESavdPacote{162}] = PREFIXO + resultado[EExtensaoArquivoResultado(2)];   // NO directory (see note) -> {:c}{:05}{}{:05}{:04}{:04}-sa.vsc
    m_pacotes[ESavdPacote{163}] = PREFIXO + resultado[EExtensaoArquivoResultado(2)];   // NO directory (see note) -> {:c}{:05}{}{:05}{:04}{:04}-sa.vsc
    m_pacotes[ESavdPacote{164}] = PREFIXO + resultado[EExtensaoArquivoResultado(2)];   // NO directory (see note) -> {:c}{:05}{}{:05}{:04}{:04}-sa.vsc
    m_pacotes[ESavdPacote{165}] = PREFIXO + resultado[EExtensaoArquivoResultado(2)];   // NO directory (see note) -> {:c}{:05}{}{:05}{:04}{:04}-sa.vsc
    m_pacotes[ESavdPacote{166}] = ((CPath::GetPathResult(INTERNA, T1) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(3)]);   // /dsk/fi/dinamico/res1/{:c}{:05}{}{:05}{:04}{:04}-red.vsc
    m_pacotes[ESavdPacote{167}] = ((CPath::GetPathResult(INTERNA, T2) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(3)]);   // /dsk/fi/dinamico/res2/{:c}{:05}{}{:05}{:04}{:04}-red.vsc
    m_pacotes[ESavdPacote{168}] = ((CPath::GetPathResult(EXTERNA, T1) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(3)]);   // /dsk/fe/dinamico/res1/{:c}{:05}{}{:05}{:04}{:04}-red.vsc
    m_pacotes[ESavdPacote{169}] = ((CPath::GetPathResult(EXTERNA, T2) / PREFIXO).string() + resultado[EExtensaoArquivoResultado(3)]);   // /dsk/fe/dinamico/res2/{:c}{:05}{}{:05}{:04}{:04}-red.vsc
    m_pacotes[ESavdPacote{170}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "rdv.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/rdv.vsu
    m_pacotes[ESavdPacote{171}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "rdv.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/rdv.vsu
    m_pacotes[ESavdPacote{172}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "rdv.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/rdv.vsu
    m_pacotes[ESavdPacote{173}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "rdv.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/rdv.vsu
    m_pacotes[ESavdPacote{174}] = (CPath::GetPathRootSemSA(EXTERNA) / "dinamico/sa/{}/trab1/").string() + "rdv.vsu";   // /dsk/fe/dinamico/sa/{}/trab1/rdv.vsu
    m_pacotes[ESavdPacote{175}] = (CPath::GetPathRootSemSA(EXTERNA) / "dinamico/sa/{}/trab2/").string() + "rdv.vsu";   // /dsk/fe/dinamico/sa/{}/trab2/rdv.vsu
    m_pacotes[ESavdPacote{176}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "ze.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/ze.vsu
    m_pacotes[ESavdPacote{177}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "ze.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/ze.vsu
    m_pacotes[ESavdPacote{178}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "ze.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/ze.vsu
    m_pacotes[ESavdPacote{179}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "ze.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/ze.vsu
    m_pacotes[ESavdPacote{180}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "bu.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/bu.vsu
    m_pacotes[ESavdPacote{181}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "bu.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/bu.vsu
    m_pacotes[ESavdPacote{182}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "saraiz.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/saraiz.vsu
    m_pacotes[ESavdPacote{183}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "saraiz.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/saraiz.vsu
    m_pacotes[ESavdPacote{184}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "sasecao.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/sasecao.vsu
    m_pacotes[ESavdPacote{185}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "sasecao.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/sasecao.vsu
    m_pacotes[ESavdPacote{186}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab1/").string() + "busa.vsu";   // /dsk/fi/dinamico/sa/{}/trab1/busa.vsu
    m_pacotes[ESavdPacote{187}] = (CPath::GetPathRootSemSA(INTERNA) / "dinamico/sa/{}/trab2/").string() + "busa.vsu";   // /dsk/fi/dinamico/sa/{}/trab2/busa.vsu
    m_pacotes[ESavdPacote{188}] = (CPath::GetPathRootSemSA(EXTERNA) / "dinamico/sa/{}/trab1/").string() + "busa.vsu";   // /dsk/fe/dinamico/sa/{}/trab1/busa.vsu
    m_pacotes[ESavdPacote{189}] = (CPath::GetPathRootSemSA(EXTERNA) / "dinamico/sa/{}/trab2/").string() + "busa.vsu";   // /dsk/fe/dinamico/sa/{}/trab2/busa.vsu
    m_pacotes[ESavdPacote{190}] = CPath::GetPathMR() / "sieco-dados/";   // /dsk/mr/sieco-dados/
    m_pacotes[ESavdPacote{191}] = CPath::GetPathTrab(INTERNA, T1) / "bim.vsu";   // /dsk/fi/dinamico/trab1/bim.vsu
    m_pacotes[ESavdPacote{192}] = CPath::GetPathTrab(INTERNA, T2) / "bim.vsu";   // /dsk/fi/dinamico/trab2/bim.vsu
    m_pacotes[ESavdPacote{193}] = CPath::GetPathTrab(EXTERNA, T1) / "bim.vsu";   // /dsk/fe/dinamico/trab1/bim.vsu
    m_pacotes[ESavdPacote{194}] = CPath::GetPathTrab(EXTERNA, T2) / "bim.vsu";   // /dsk/fe/dinamico/trab2/bim.vsu
    m_pacotes[ESavdPacote{195}] = CPath::GetPathTrab(INTERNA, T1) / "behb.vsu";   // /dsk/fi/dinamico/trab1/behb.vsu
    m_pacotes[ESavdPacote{196}] = CPath::GetPathTrab(INTERNA, T2) / "behb.vsu";   // /dsk/fi/dinamico/trab2/behb.vsu
    m_pacotes[ESavdPacote{197}] = CPath::GetPathTrab(EXTERNA, T1) / "behb.vsu";   // /dsk/fe/dinamico/trab1/behb.vsu
    m_pacotes[ESavdPacote{198}] = CPath::GetPathTrab(EXTERNA, T2) / "behb.vsu";   // /dsk/fe/dinamico/trab2/behb.vsu
    m_pacotes[ESavdPacote{199}] = CPath::GetPathTrab(INTERNA, T1) / "uenux.vsu";   // /dsk/fi/dinamico/trab1/uenux.vsu
    m_pacotes[ESavdPacote{200}] = CPath::GetPathTrab(INTERNA, T2) / "uenux.vsu";   // /dsk/fi/dinamico/trab2/uenux.vsu
    m_pacotes[ESavdPacote{201}] = CPath::GetPathTrab(EXTERNA, T1) / "uenux.vsu";   // /dsk/fe/dinamico/trab1/uenux.vsu
    m_pacotes[ESavdPacote{202}] = CPath::GetPathTrab(EXTERNA, T2) / "uenux.vsu";   // /dsk/fe/dinamico/trab2/uenux.vsu
    // NOTE (162..165): the four SA ".vsc" packages are stored WITHOUT any directory, while every other
    // result package (158..161, 166..169) is prefixed with CPath::GetPathResult(...). Entries 172/173
    // repeat 170/171 and 178/179 repeat 176/177 (INTERNA twice): probable copy-paste slips in the
    // original table (both are only used by the SA application, not by VOTA).

    // ---- m_arquivos: signed files (names / std::format patterns, no directory) -------------------
    m_arquivos[ESavdArquivoUE{25}] = "eg.bin";
    m_arquivos[ESavdArquivoUE{26}] = "eg.bin";
    m_arquivos[ESavdArquivoUE{27}] = "gap.bin";
    m_arquivos[ESavdArquivoUE{28}] = "gap.bin";
    m_arquivos[ESavdArquivoUE{29}] = "gap.bin";
    m_arquivos[ESavdArquivoUE{30}] = "gap.bin";
    m_arquivos[ESavdArquivoUE{31}] = "vota.bin";
    m_arquivos[ESavdArquivoUE{32}] = "vota.bin";
    m_arquivos[ESavdArquivoUE{33}] = "sa.bin";
    m_arquivos[ESavdArquivoUE{115}] = "red.bin";
    m_arquivos[ESavdArquivoUE{34}] = "tabcorr.dat";
    m_arquivos[ESavdArquivoUE{112}] = "{:05}{:04}{:04}-lo.dat";
    m_arquivos[ESavdArquivoUE{113}] = "dadoscarga.dat";
    m_arquivos[ESavdArquivoUE{35}] = PREFIXO + resultado[EExtensaoArquivoResultado(4)];   // "{:c}{:05}{}{:05}{:04}{:04}-bu.dat"
    m_arquivos[ESavdArquivoUE{36}] = PREFIXO + resultado[EExtensaoArquivoResultado(5)];   // "{:c}{:05}{}{:05}{:04}{:04}-busa.dat"
    m_arquivos[ESavdArquivoUE{37}] = PREFIXO + resultado[EExtensaoArquivoResultado(6)];   // "{:c}{:05}{}{:05}{:04}{:04}-rdv.dat"
    m_arquivos[ESavdArquivoUE{39}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{41}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{42}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{43}] = PREFIXO + resultado[EExtensaoArquivoResultado(9)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgbu.dat"
    m_arquivos[ESavdArquivoUE{44}] = PREFIXO + resultado[EExtensaoArquivoResultado(11)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgze.dat"
    m_arquivos[ESavdArquivoUE{45}] = PREFIXO + resultado[EExtensaoArquivoResultado(10)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgbusa.dat"
    m_arquivos[ESavdArquivoUE{46}] = PREFIXO + resultado[EExtensaoArquivoResultado(12)];   // "{:c}{:05}{}{:05}{:04}{:04}-hash.dat"
    m_arquivos[ESavdArquivoUE{47}] = PREFIXO + resultado[EExtensaoArquivoResultado(12)];   // "{:c}{:05}{}{:05}{:04}{:04}-hash.dat"
    m_arquivos[ESavdArquivoUE{48}] = PREFIXO + resultado[EExtensaoArquivoResultado(16)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqbio.jez"
    m_arquivos[ESavdArquivoUE{49}] = PREFIXO + resultado[EExtensaoArquivoResultado(17)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqman.jez"
    m_arquivos[ESavdArquivoUE{50}] = PREFIXO + resultado[EExtensaoArquivoResultado(18)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqmes.jez"
    m_arquivos[ESavdArquivoUE{51}] = PREFIXO + resultado[EExtensaoArquivoResultado(16)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqbio.jez"
    m_arquivos[ESavdArquivoUE{52}] = PREFIXO + resultado[EExtensaoArquivoResultado(17)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqman.jez"
    m_arquivos[ESavdArquivoUE{53}] = PREFIXO + resultado[EExtensaoArquivoResultado(18)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqmes.jez"
    m_arquivos[ESavdArquivoUE{54}] = PREFIXO + resultado[EExtensaoArquivoResultado(16)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqbio.jez"
    m_arquivos[ESavdArquivoUE{55}] = PREFIXO + resultado[EExtensaoArquivoResultado(17)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqman.jez"
    m_arquivos[ESavdArquivoUE{56}] = PREFIXO + resultado[EExtensaoArquivoResultado(18)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqmes.jez"
    m_arquivos[ESavdArquivoUE{57}] = PREFIXO + resultado[EExtensaoArquivoResultado(16)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqbio.jez"
    m_arquivos[ESavdArquivoUE{58}] = PREFIXO + resultado[EExtensaoArquivoResultado(17)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqman.jez"
    m_arquivos[ESavdArquivoUE{59}] = PREFIXO + resultado[EExtensaoArquivoResultado(18)];   // "{:c}{:05}{}{:05}{:04}{:04}-wsqmes.jez"
    m_arquivos[ESavdArquivoUE{60}] = PREFIXO + resultado[EExtensaoArquivoResultado(13)];   // "{:c}{:05}{}{:05}{:04}{:04}-log.jez"
    m_arquivos[ESavdArquivoUE{61}] = PREFIXO + resultado[EExtensaoArquivoResultado(15)];   // "{:c}{:05}{}{:05}{:04}{:04}-logsa.jez"
    m_arquivos[ESavdArquivoUE{63}] = PREFIXO + resultado[EExtensaoArquivoResultado(4)];   // "{:c}{:05}{}{:05}{:04}{:04}-bu.dat"
    m_arquivos[ESavdArquivoUE{64}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{65}] = PREFIXO + resultado[EExtensaoArquivoResultado(6)];   // "{:c}{:05}{}{:05}{:04}{:04}-rdv.dat"
    m_arquivos[ESavdArquivoUE{66}] = PREFIXO + resultado[EExtensaoArquivoResultado(9)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgbu.dat"
    m_arquivos[ESavdArquivoUE{67}] = PREFIXO + resultado[EExtensaoArquivoResultado(11)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgze.dat"
    m_arquivos[ESavdArquivoUE{68}] = PREFIXO + resultado[EExtensaoArquivoResultado(12)];   // "{:c}{:05}{}{:05}{:04}{:04}-hash.dat"
    m_arquivos[ESavdArquivoUE{69}] = PREFIXO + resultado[EExtensaoArquivoResultado(13)];   // "{:c}{:05}{}{:05}{:04}{:04}-log.jez"
    m_arquivos[ESavdArquivoUE{73}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{74}] = PREFIXO + resultado[EExtensaoArquivoResultado(7)];   // "{:c}{:05}{}{:05}{:04}{:04}-rdvred.dat"
    m_arquivos[ESavdArquivoUE{75}] = PREFIXO + resultado[EExtensaoArquivoResultado(9)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgbu.dat"
    m_arquivos[ESavdArquivoUE{76}] = PREFIXO + resultado[EExtensaoArquivoResultado(11)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgze.dat"
    m_arquivos[ESavdArquivoUE{77}] = PREFIXO + resultado[EExtensaoArquivoResultado(13)];   // "{:c}{:05}{}{:05}{:04}{:04}-log.jez"
    m_arquivos[ESavdArquivoUE{78}] = PREFIXO + resultado[EExtensaoArquivoResultado(8)];   // "{:c}{:05}{}{:05}{:04}{:04}-jufa.dat"
    m_arquivos[ESavdArquivoUE{79}] = PREFIXO + resultado[EExtensaoArquivoResultado(7)];   // "{:c}{:05}{}{:05}{:04}{:04}-rdvred.dat"
    m_arquivos[ESavdArquivoUE{80}] = PREFIXO + resultado[EExtensaoArquivoResultado(9)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgbu.dat"
    m_arquivos[ESavdArquivoUE{81}] = PREFIXO + resultado[EExtensaoArquivoResultado(11)];   // "{:c}{:05}{}{:05}{:04}{:04}-imgze.dat"
    m_arquivos[ESavdArquivoUE{82}] = PREFIXO + resultado[EExtensaoArquivoResultado(13)];   // "{:c}{:05}{}{:05}{:04}{:04}-log.jez"
    m_arquivos[ESavdArquivoUE{83}] = "rdv.dat";
    m_arquivos[ESavdArquivoUE{84}] = "buj.dat";
    m_arquivos[ESavdArquivoUE{106}] = "bim.dat";
    m_arquivos[ESavdArquivoUE{108}] = "behb.dat";
    m_arquivos[ESavdArquivoUE{86}] = "bu.dat";
    m_arquivos[ESavdArquivoUE{85}] = "buj.dat";
    m_arquivos[ESavdArquivoUE{107}] = "bim.dat";
    m_arquivos[ESavdArquivoUE{109}] = "behb.dat";
    m_arquivos[ESavdArquivoUE{87}] = "bu.dat";
    m_arquivos[ESavdArquivoUE{88}] = "ze.dat";
    m_arquivos[ESavdArquivoUE{89}] = "rze.dat";
    m_arquivos[ESavdArquivoUE{90}] = "bur.dat";
    m_arquivos[ESavdArquivoUE{91}] = "bujr.dat";
    m_arquivos[ESavdArquivoUE{92}] = "bimr.dat";
    m_arquivos[ESavdArquivoUE{93}] = "behbr.dat";
    m_arquivos[ESavdArquivoUE{94}] = "bur.dat";
    m_arquivos[ESavdArquivoUE{95}] = "bujr.dat";
    m_arquivos[ESavdArquivoUE{96}] = "bimr.dat";
    m_arquivos[ESavdArquivoUE{97}] = "behbr.dat";
    m_arquivos[ESavdArquivoUE{98}] = "rdv.dat";
    m_arquivos[ESavdArquivoUE{99}] = "ze.dat";
    m_arquivos[ESavdArquivoUE{100}] = "bu.dat";
    m_arquivos[ESavdArquivoUE{101}] = "saraiz.bin";
    m_arquivos[ESavdArquivoUE{102}] = "sasecao.bin";
    m_arquivos[ESavdArquivoUE{103}] = "busa.dig";
    m_arquivos[ESavdArquivoUE{110}] = "uenux.db";
    m_arquivos[ESavdArquivoUE{111}] = "uenux.db";
    m_arquivos[ESavdArquivoUE{104}] = "uesieco-20[0-9]{2}.[0-1][0-9].[0-3][0-9]-[0-2][0-9].[0-5][0-9].[0-5][0-9]-[0-9]{8}-cert.dat";
    m_arquivos[ESavdArquivoUE{105}] = "uesieco-20[0-9]{2}.[0-1][0-9].[0-3][0-9]-[0-2][0-9].[0-5][0-9].[0-5][0-9]-[0-9]{8}-aut.dat";

    // ---- m_aplicacoes: SAVD application / key identifiers (never read in this build) -------------
    m_aplicacoes[ESavdAplicacao{19}] = "partido.pub";
    m_aplicacoes[ESavdAplicacao{18}] = "partido.app";
    m_aplicacoes[ESavdAplicacao{17}] = "partido";
    m_aplicacoes[ESavdAplicacao{20}] = "partido.id";
    m_aplicacoes[ESavdAplicacao{1}] = "vota.of";
    m_aplicacoes[ESavdAplicacao{2}] = "vota.si";
    m_aplicacoes[ESavdAplicacao{3}] = "vota.te";
    m_aplicacoes[ESavdAplicacao{4}] = "vota.tm";
    m_aplicacoes[ESavdAplicacao{5}] = "sa.of";
    m_aplicacoes[ESavdAplicacao{6}] = "sa.si";
    m_aplicacoes[ESavdAplicacao{7}] = "sa.tr";
    m_aplicacoes[ESavdAplicacao{9}] = "red";
    m_aplicacoes[ESavdAplicacao{10}] = "vpp";
    m_aplicacoes[ESavdAplicacao{11}] = "adh";
    m_aplicacoes[ESavdAplicacao{12}] = "atue";
    m_aplicacoes[ESavdAplicacao{13}] = "ste";
    m_aplicacoes[ESavdAplicacao{8}] = "gap";
    m_aplicacoes[ESavdAplicacao{16}] = "savd";
    m_aplicacoes[ESavdAplicacao{15}] = "turno2.jez";
    m_aplicacoes[ESavdAplicacao{22}] = "infomidia.pwd";
}

// wasm func 5907 - ~CArquivosSavd(): destroys the three maps (funcs 3841, 3840, 3839 are the three
// std::__tree<...>::destroy(node) instantiations, identical bodies: recurse left/right, free the value
// string, free the node).
CArquivosSavd::~CArquivosSavd() = default;

// wasm func 11658 - atexit handler registered for s_instancia: s_instancia.reset().

} // namespace comum

// FRAGMENT reconstructed by unit u39 from vota_web_wasm.wasm.
// Original file (attested by 31 srclocs of siblings): uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp
// Other parts: ctelasvota.cpp (u07), ctelasvota.u02.cpp, .u09, .u22, .u26.
//
// Two groups of code without a srcloc of their own that belong to this translation unit:
//
// 1. The five "Mais informações" menu items of VOTA (vota::CItem*Vota). Their typeinfos and vtables
//    (@1539644 ... @1539800) are emitted inside ctelasvota.cpp's data, immediately after the typeinfo of the
//    CTelasVota::CriaTelaVisualizacaoCandidato lambda and together with CMenuMaisInformacoesVota and even
//    comum::CItemMenu's vtable: the classes are defined in ctelasvota.cpp itself (or in a header that only
//    ctelasvota.cpp includes). They are created by CTelasVota::CriaTelaMaisInformacoes (func 6599).
//    Base classes and slot names: unit u37 (comum/citemmenu.h, comum/citemimprime*.h).
//
// 2. Four data sources passed to the form builder as plain function pointers (function-table slots) by the
//    CTelasVota constructor (inlined into func 7787, vota::CInformacaoEleitor::Inicializar) and its helpers.
//    Their names are not attested; "DS_" follows the convention of ctelasvota.cpp (u07).
#include <memory>
#include <string>
#include <vector>

#include "api/gui/cfixedimage.h"
#include "comum/appinfo/cappinfo.h"
#include "comum/citemimprimeestadourna.h"
#include "comum/citemimprimelistaeleitores.h"
#include "comum/citemparametrosurna.h"
#include "comum/citemversoespacotes.h"
#include "ecourna/api/util/cstringutils.hpp"
#include "vota/comum/votadefs.h"
#include "vota/eleitor/celeitorvotando.h"
#include "vota/eleitor/comum/cinformacaoeleitor.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

// =========================================================================================================
// 1. "Mais informações" items. Slot 3 of the four report items = GetNumViasImpressas(): how many copies of
// that report were already printed, read from vota.bin (EstadoGeralVota.numViasImpressasRelatorios,
// ModuloEstadoGeralDefs: {numViasEstadoUrna, numViasEleitores, numViasVersoesDados, numViasPU} = bytes
// +73..+76 of CEstadoGeralVota). comum::CItemImprime*::Disponivel (slot 2) compares it with the maximum of
// the election parameters, so a report can only be reprinted a limited number of times.
// =========================================================================================================

namespace {
const comum::md::estadoaplicacao::CEstadoGeralVota& EstadoGeralVota()
{
    return comum::CAppInfo::GetInst().GetVota();              // funcs 185 + 903
}
}  // namespace

// typeinfo @1539660, vtable @1539644: [0] 2937 [1] ICF 2939 [2] comum 11669 [3] 12267
class CItemImprimeEstadoUrnaVota final : public comum::CItemImprimeEstadoUrna {
public:
    using comum::CItemImprimeEstadoUrna::CItemImprimeEstadoUrna;
protected:
    unsigned GetNumViasImpressas() const override;
};

// typeinfo @1539696, vtable @1539680: [0] 2937 [1] ICF 2939 [2] comum 11668 [3] 12266
class CItemImprimeListaEleitoresVota final : public comum::CItemImprimeListaEleitores {
public:
    using comum::CItemImprimeListaEleitores::CItemImprimeListaEleitores;
protected:
    unsigned GetNumViasImpressas() const override;
};

// typeinfo @1539732, vtable @1539716: [0] 2937 [1] ICF 2939 [2] comum 11666 [3] 12259
class CItemVersoesPacotesVota final : public comum::CItemVersoesPacotes {
public:
    using comum::CItemVersoesPacotes::CItemVersoesPacotes;
protected:
    unsigned GetNumViasImpressas() const override;
};

// typeinfo @1539768, vtable @1539752: [0] 2937 [1] ICF 2939 [2] comum 11667 [3] 12255
class CItemParametrosUrnaVota final : public comum::CItemParametrosUrna {
public:
    using comum::CItemParametrosUrna::CItemParametrosUrna;
protected:
    unsigned GetNumViasImpressas() const override;
};

// typeinfo @1539800, vtable @1539788: [0] 2937 [1] 12251 [2] Disponivel 12244 (u02: true when some cargo
// has candidates to show)
class CItemVisualizarCandidatosVota final : public comum::CItemMenu {
public:
    using comum::CItemMenu::CItemMenu;
    ~CItemVisualizarCandidatosVota() override = default;       // [1] deleting variant = wasm func 12251
    bool Disponivel() const override;                          // [2] wasm func 12244 (u02)
};

// wasm func 12267 - vtable slot 3. Observed executing: CMenuBase::AdicionaItem -> CItemImprimeEstadoUrna::
// Disponivel -> here, while CTelasVota builds "telaMaisInformacoes" at start-up.
unsigned CItemImprimeEstadoUrnaVota::GetNumViasImpressas() const
{
    return EstadoGeralVota().GetNumViasImpressas().numViasEstadoUrna;          // byte +73
}

// wasm func 12266 - vtable slot 3
unsigned CItemImprimeListaEleitoresVota::GetNumViasImpressas() const
{
    return EstadoGeralVota().GetNumViasImpressas().numViasEleitores;           // byte +74
}

// wasm func 12259 - vtable slot 3
unsigned CItemVersoesPacotesVota::GetNumViasImpressas() const
{
    return EstadoGeralVota().GetNumViasImpressas().numViasVersoesDados;        // byte +75
}

// wasm func 12255 - vtable slot 3
unsigned CItemParametrosUrnaVota::GetNumViasImpressas() const
{
    return EstadoGeralVota().GetNumViasImpressas().numViasPU;                  // byte +76 (PU = parâmetros de urna)
}

// wasm func 12251 - vtable slot 1: deleting destructor of CItemVisualizarCandidatosVota
// = comum::CItemMenu::~CItemMenu (func 2937: vptr + ~m_texto) + operator delete (free). Its twin for the other
// four items is the ICF body 2939. Reached through shared_ptr<CItemVisualizarCandidatosVota>'s control block
// (func 12172) when the temporary menu of CriaTelaMaisInformacoes is destroyed.

// =========================================================================================================
// 2. Data sources of CTelasVota screens (std::string (*)() / std::vector<uebyte> (*)(), evaluated at draw time)
// =========================================================================================================
namespace {

// wasm func 13176 (table slot 1098): the number typed so far for the current cargo, shown on the vote
// screens (adicionaBaseTelaVotoNuloConsulta 6624 and func 3060). Returns the global of celeitorvotando.cpp
// (declared in celeitorvotando.h). ctelasvota.cpp (u07) forward-declares it INSIDE its anonymous namespace,
// so the definition belongs to the same anonymous namespace of this translation unit.
const std::string& VotoDigitado()                                             // name as in ctelasvota.cpp (u07; inferred)
{
    return g_votoDigitado;                                                    // @1833288
}

// wasm func 13137 (table slot 1097). Second half of the footer line "CORRIGE para REINICIAR este voto" drawn
// by adicionaInstrucoesConfirmaCorrige (func 1102, u02): api_f1191(builder, slot 1097, {130,455}, FONTE_20, 0).
// Literal @125042 (25 characters).
std::string DS_ParaReiniciarEsteVoto()                                        // name inferred
{
    return " para REINICIAR este voto";
}

// wasm func 13441 (table slot 1086): image of the "audio" icon next to the number field of every empty vote
// screen (adicionaCampoNumero, func 2384 -> api_f6689: CImageField over CDataImage<std::vector<uebyte>>).
// While the voter votes with audio (CInformacaoEleitor::m_modoAudio != 2) it is the JPEG resource
// ":/resource/images/audioHabilitado.jpg" (loaded through api::CFixedImage, func 2246, and copied out);
// otherwise a 66-byte 1x1 monochrome BMP whose single pixel is white, i.e. nothing visible on the white screen.
std::vector<uebyte> DS_IconeAudio()                                           // name inferred
{
    if (CInformacaoEleitor::GetInst().m_modoAudio != 2)                       // func 509
        return api::CFixedImage(":/resource/images/audioHabilitado.jpg").GetImagem();   // vector copy, then
                                                                                          // ~CFixedImage
    return ecourna::api::util::CStringUtils::HexStringToBytes(               // func 3506
        "424d42000000000000003e000000280000000100000001000000010001000000000004000000"
        "0000000000000000000000000000000000000000ffffff0080000000");
}

// wasm func 12974 (table slot 1105): third line of the screen "telaNumeroCopiasErrado" ("Número de cópias /
// acima do limite permitido / O número máximo permitido é N"), shown by CEmitirMaisBU when the mesário asks
// for more extra BU copies than allowed. api_f1191(builder, slot 1105, {64,260}, FONTE_25 @474992, 2).
std::string DS_NumeroMaximoCopiasBU()                                          // name inferred
{
    return "O número máximo permitido é " + std::to_string(GetQuantidadeMaximaBUsAdicionais());   // 4582, 296, 499
}

}  // namespace

}  // namespace vota

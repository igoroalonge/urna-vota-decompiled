// FRAGMENTS reconstructed by unit u17 from vota_web_wasm.wasm.
//
// Voter-thread code that the tools attributed to u17's api files (iresource.h, ctextsource.h) because
// api::getResourceMovie (iresource.h:87) or a form helper is inlined into it. Merge each block into
// the file named in its header.
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>

#include "api/gui/cformbuilder.h"
#include "api/gui/cmoviefield.h"
#include "api/gui/iresource.h"
#include "api/util/cdatetime.h"
#include "comum/cappstate.h"
#include "comum/cconfiguracaoeleicao.h"
#include "vota/eleitor/cthreadeleitor.h"
#include "vota/eleitor/comum/ctelasvota.h"

namespace vota {

using api::SPoint;

// =================================================================================================
// uenux2/src/app/vota/eleitor/comum/ctelasvota.cpp (anonymous-namespace helpers; unit u02 lists 1593
// as "criaBuilderComAnimacao" in ctelasvota.u02.cpp)
// Both put an animated GIF (api::CMovieField) at {640, 480} of a voter screen with anchor 7 (bottom-right),
// i.e. in the bottom-right corner (observed: js_image(..., 865, 400, 415, 400) on the 1280x800 canvas).
// Both ran during the recorded votes (every "voto nulo"/"branco"/"legenda" and every cargo screen).
// =================================================================================================
namespace {

// wasm func 1593 (observed executing; tools: "api::getResourceMovie@1593")          name inferred
// Animation of the vote type: 1 legenda (party-only vote), 3 branco, 4 nulo; other types add nothing.
// With `conferencia` the animation is frozen on its current frame (a one-frame CMovie).
// The CMovie objects are cached per (file, conferencia) in a static std::map @1833352 (never freed).
void adicionaAnimacaoVoto(int tipoVoto, api::CFormBuilder& builder, bool conferencia)
{
    const char* gif = nullptr;
    switch (tipoVoto) {
    case 1: gif = ":/resource/gifs/votoLegenda.gif"; break;
    case 3: gif = ":/resource/gifs/votoBranco.gif"; break;
    case 4: gif = ":/resource/gifs/votoNulo.gif"; break;
    default: return;
    }

    static std::map<std::pair<std::string, bool>, std::shared_ptr<api::CMovie>> s_cache;   // @1833348..
    const auto chave = std::make_pair(std::string(gif), conferencia);
    std::shared_ptr<api::CMovie> filme;
    if (auto it = s_cache.find(chave); it != s_cache.end()) {                // func 6643 (__find_equal)
        filme = it->second;
    } else {
        api::CMovie original = api::getResourceMovie(gif);                   // iresource.h:87, IResource slot 4
        // The current frame is copied (vector::at, throws out_of_range) BEFORE the conferencia
        // test, i.e. in both cases (func 1593 does the at() + copy ahead of `if (c)`).
        const api::CMovieFrame quadroAtual = original.m_frames.at(original.m_frameAtual);
        if (conferencia)
            filme = std::make_shared<api::CMovie>(
                std::vector<api::CMovieFrame>{quadroAtual},
                original.m_tamanho);                                          // func 5526 (u15)
        else
            filme = std::make_shared<api::CMovie>(original);                  // frames copied: func 6696
        s_cache.emplace(chave, filme);
        filme = s_cache.at(chave);                                            // "map::at:  key not found"
    }
    builder.Add<api::CMovieField>(SPoint{640, 480}, filme);                   // func 5543 (packed 0x01E00280;
                                                                              //   anchor 7 = bottom-right)
}

// wasm func 3076 (observed executing; tools: "api::getResourceMovie@3076")          name inferred
// Animation of the cargo being voted, chosen by the cargo code; unknown codes add nothing.
// Caller: the CTelasVota constructor, inlined into func 7787 (vota::CInformacaoEleitor::Inicializar).
// Not cached: loaded at every call.
void adicionaAnimacaoCargo(const uebyte& codigoCargo, api::CFormBuilder& builder)
{
    static const std::map<uebyte, std::string> s_gifs = {                     // @1833400, guard @1833412
        {1, ":/resource/gifs/presidente.gif"},                                // (pairs built by func 1444)
        {3, ":/resource/gifs/governador.gif"},
        {5, ":/resource/gifs/senador.gif"},
        {6, ":/resource/gifs/depFederal.gif"},
        {7, ":/resource/gifs/depEstadual.gif"},
        {8, ":/resource/gifs/depDistrital.gif"},
        {25, ":/resource/gifs/conselheiroDistrital.gif"},
        {11, ":/resource/gifs/prefeito.gif"},
        {13, ":/resource/gifs/vereador.gif"},
    };
    std::string gif;
    if (auto it = s_gifs.find(codigoCargo); it != s_gifs.end())
        gif = it->second;
    if (gif.empty())
        return;
    auto filme = std::make_shared<api::CMovie>(api::getResourceMovie(gif));   // iresource.h:87
    builder.Add<api::CMovieField>(SPoint{640, 480}, filme);                   // func 5543 (packed 0x01E00280;
                                                                              //   anchor 7 = bottom-right)
}

}  // namespace

// =================================================================================================
// uenux2/src/app/vota/eleitor/iniciovotacao/ciniciovotacao.cpp  (path inferred)
// "Início da votação": waits for the configured start time of the vote. RTTI: comum::CAppState <-
// vota::CInicioVotacao (vtable @1543600): slot 2 StartState (11988), slot 8 ProcessTick (11987).
// =================================================================================================

// Constructor (inlined into GetInst, func 2880). 44 bytes:
//   +12/+16 m_telaVoto        CTelasVota member +116 (the voter-screen form of this state)
//   +20/+24 m_formMT          microterminal form built below
//   +28     m_inicioVotacao   CDateTime (12 bytes) copied from CConfiguracaoEleicao +556
//   +40     m_tick            1000 ms tick of CThreadEleitor
CInicioVotacao::CInicioVotacao()
    : comum::CAppState(4)                                                    // ticks only
    , m_telaVoto(CTelasVota::GetInst().m_telaInicioVotacao)                  // +116, name inferred
    , m_inicioVotacao(comum::CConfiguracaoEleicao::GetInst().GetInicioVotacao())   // +556
    , m_tick(CThreadEleitor::GetInst().CriaTick(1000))                       // func 807
{
    const std::string hora = m_inicioVotacao.GetTime().Format("h");          // shared_f779 (CTime::Format)
    const std::string minuto = m_inicioVotacao.GetTime().Format("mm");

    api::CFormBuilderMT campos;
    campos.Add<api::CTextFieldMT>(SPoint{1, 1}, std::make_shared<api::CFixedText>(api::ETextAlignment(0),
                                                    "Aguarde o horário de início da votação"));
    campos.Add<api::CTextFieldMT>(SPoint{1, 2}, std::make_shared<api::CFixedText>(api::ETextAlignment(0),
                                                    "Votação a partir das "));
    // "8h" when the minutes are "00", else "8h30min".
    const std::string horario = (minuto == "00") ? hora + "h" : hora + "h" + minuto + "min";
    campos.Add<api::CTextFieldMT>(SPoint{22, 2}, std::make_shared<api::CFixedText>(api::ETextAlignment(0), horario));
    campos.Add<api::CTextFieldMT>(SPoint{1, 3}, std::make_shared<api::CFixedText>(api::ETextAlignment(0), "Hora atual: "));
    campos.Add<api::CClockFieldMT>(SPoint{13, 3});
    m_formMT = campos.CriaFormInterativo("", true);                          // func 301
}

// wasm func 2880 (tools: vota_f2880). @1834128, mutex @1834104.
// Callers: CIniciodeCiclo::AjustaDataHora (7306), CQuerReimprimirZeresima (11848),
// CDefineRotaPosReinicio::NeedChangeState (11858), CQuerImprimirZeresima (11920).
CInicioVotacao& CInicioVotacao::GetInst()
{
    static std::mutex mutex;                                  // @1834104
    static std::unique_ptr<CInicioVotacao> s_inst;            // @1834128
    std::lock_guard lock(mutex);
    if (!s_inst)
        s_inst.reset(new CInicioVotacao());
    return *s_inst;
}

}  // namespace vota

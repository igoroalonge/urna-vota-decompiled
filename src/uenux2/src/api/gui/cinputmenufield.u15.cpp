// FRAGMENT reconstructed by unit u15 from vota_web_wasm.wasm.
// Original file: uenux2/src/api/gui/cinputmenufield.cpp (srclocs 49, 74, 142, 198, 210, 251, 278, 298,
// 320, 326, 339, 419, 477). Not here: CMenuItem::SetFormatString / CInputMenuField::SetFormatString
// (func 11884) and CMenuValidation slot 3 (func 10899, unit u32). Merge into cinputmenufield.cpp.
#include "api/gui/cinputmenufield.u15.h"

#include <algorithm>
#include <chrono>
#include <regex>
#include <thread>

#include "api/pattern/cpolysingletonlist.h"

namespace api {

class IBeep;   // slot 6 = error beep

// ------------------------------------------------------------------------------------------------
// CMenuItem

// wasm func 5492 (tools: api_f5492; not observed)                                   // name inferred
// Expands the item format ("[%S] - %T" by default): %S = search text, %T = item text, everything
// else copied; a trailing blank is appended. The regex is a function-local static (@1839244, guard
// @1839284) compiled on first use.
std::string CMenuItem::GetDisplayText() const
{
    static const std::regex marcadores("%[ST]");
    const std::string& formato = m_formato.empty() ? m_menu->m_formato : m_formato;
    std::string resultado;
    for (std::sregex_token_iterator it(formato.begin(), formato.end(), marcadores, {-1, 0}), fim;
         it != fim; ++it) {
        const std::string parte = *it;
        if (parte == "%S")
            resultado += m_textoBusca;
        else if (parte == "%T")
            resultado += m_texto;
        else
            resultado += parte;
    }
    return resultado + " ";
}

// wasm func 3657 (not observed) - srcloc cinputmenufield.cpp:49
void CMenuItem::UpdateRect()
{
    auto& tela = CPolySingletonList::instance<IScreen>();                          // srcloc :49
    TPosition largura = 0, altura = 0;
    tela.GetTextSize(largura, altura, m_fonte, GetDisplayText());                  // IScreen slot 3
    const TPosition x1 = static_cast<TPosition>(m_pos.x + largura);
    const TPosition y1 = static_cast<TPosition>(m_pos.y + altura + 1);
    m_rect = SRect{std::min(m_pos.x, x1), std::min(m_pos.y, y1), std::max(m_pos.x, x1), std::max(m_pos.y, y1)};
}

// wasm func 3655 (not observed) - srcloc cinputmenufield.cpp:142
CMenuItem& CMenuItem::SetSearchText(const std::string& texto)
{
    if (texto == m_textoBusca)
        return *this;
    for (const auto& item : m_menu->m_itens)
        if (item.m_textoBusca == texto)
            throw CUeGuiError(static_cast<EUeGuiError>(4929), "Texto de busca repetido [" + texto + "]");
    m_textoBusca = texto;
    UpdateRect();
    if (!m_menu->m_itens.empty())
        m_menu->SetLength(m_menu->MaiorTextoBusca());   // the input box gets as many digits as the
                                                        // longest search text (vtable slot 12)
    m_menu->Invalidate();
    return *this;
}

// Inlined in CInputMenuField::Draw (func 10897) - srcloc cinputmenufield.cpp:74
void CMenuItem::Draw() const
{
    auto& tela = CPolySingletonList::instance<IScreen>();                          // srcloc :74
    tela.FillRect(m_rect, m_destacado ? 3 : 1);                                    // slot 6
    tela.DrawText(m_pos, CFixedText(GetDisplayText()), m_fonte, m_visivel ? 2 : 3, 0);   // slot 19
}

// wasm func 5490 (tools: api_f5490)                                                // name inferred
// ~m_formato, m_dado.reset() (libc++ any handler called with action 0 = destroy), ~m_textoBusca,
// ~m_callback, ~m_texto.
CMenuItem::~CMenuItem() = default;

// ------------------------------------------------------------------------------------------------
// CMenuValidation

// wasm func 10900 (not observed; CMenuValidation vtable slot 2 = IsValid) - srclocs :198 and :210
// When the typed text has the full length it must equal a visible item's search text; otherwise it
// must be the prefix of one. A mismatch beeps (IBeep slot 6) and rejects the key.
bool CMenuValidation::IsValid(const std::string& texto) const
{
    if (texto.size() == m_menu->m_tamanhoMaximo)
        return matchesExactly(texto);
    return matchesPartially(texto);
}

bool CMenuValidation::matchesExactly(const std::string& texto) const
{
    for (const auto& item : m_menu->m_itens)
        if (item.m_visivel && item.m_textoBusca == texto)
            return true;
    CPolySingletonList::instance<IBeep>().BeepErro();                              // srcloc :198
    return false;
}

bool CMenuValidation::matchesPartially(const std::string& texto) const
{
    for (const auto& item : m_menu->m_itens)
        if (item.m_visivel && item.m_textoBusca.starts_with(texto))   // std::search(...) == begin
            return true;
    CPolySingletonList::instance<IBeep>().BeepErro();                              // srcloc :210
    return false;
}

// ------------------------------------------------------------------------------------------------
// CInputMenuField

// Constructor - only inlined, in func 3675 (CInteractiveFormBuilder::AddInputMenu).
CInputMenuField::CInputMenuField(const SPoint& pos, const SFont& fonte, TPosition alturaMaxima)
    : CInputField<CFramedText>(
          std::make_shared<CMenuValidation>("1234567890", this),   // 32-byte make_shared block
          /*tamanhoMaximo*/ 1, true, true, true, true, false,       // IInputField ctor (func 2024)
          CFramedText(1, pos, fonte, ETextAlignment::Left)),         // func 1385
      m_pos(pos),
      m_fonte(fonte),
      m_rectItens{pos.x, pos.y, pos.x, pos.y},
      m_textoInstrucao(ms_textoInstrucaoPadrao),
      m_rectInstrucao(GetInstructionTextRect()),
      m_rect132{pos.x, pos.y, pos.x, pos.y},
      m_selecionado(nullptr),
      m_alturaMaxima(alturaMaxima),
      m_formato("[%S] - %T")
{
}

// srcloc cinputmenufield.cpp:251 (inlined)
SRect CInputMenuField::GetInstructionTextRect() const
{
    auto& tela = CPolySingletonList::instance<IScreen>();                          // srcloc :251
    TPosition largura = 0, altura = 0;
    tela.GetTextSize(largura, altura, m_fonte, m_textoInstrucao);                 // slot 3
    // NOTE: the rectangle is anchored at m_pos (the first item's position), as in the binary.
    const TPosition x1 = static_cast<TPosition>(m_pos.x + largura);
    const TPosition y1 = static_cast<TPosition>(m_pos.y + altura + 1);
    return SRect{std::min(m_pos.x, x1), std::min(m_pos.y, y1), std::max(m_pos.x, x1), std::max(m_pos.y, y1)};
}

// srcloc cinputmenufield.cpp:477 (inlined in func 3675)
void CInputMenuField::SetInstructionText(const std::string& texto)
{
    if (texto == m_textoInstrucao)
        return;
    m_textoInstrucao = texto;
    CPolySingletonList::instance<IScreen>().FillRect(Rect(), 1);                   // erase, srcloc :477
    m_rectInstrucao = GetInstructionTextRect();
    AtualizaLayout();
}

// wasm func 2770 (tools: api_f2770)                                               // name inferred
// Instruction text 5 px below the items; the input box right after the instruction text.
void CInputMenuField::AtualizaLayout()
{
    const SRect area = Expande(m_rectItens, 0, 0, 0, 5);    // func 2768 (other unit): margins l,t,r,b
    m_rectInstrucao.MoveTo(SPoint{area.left, area.bottom});
    SPoint caixa{m_rectInstrucao.right, static_cast<TPosition>(m_rectInstrucao.top - 1)};
    if (!m_textoInstrucao.empty())
        caixa.x = static_cast<TPosition>(caixa.x + 7);
    m_mascara.Move(caixa);                                  // func 3676
    Invalidate();
}

size_t CInputMenuField::MaiorTextoBusca() const
{
    return std::max_element(m_itens.begin(), m_itens.end(), [](const CMenuItem& a, const CMenuItem& b) {
               return a.m_textoBusca.size() < b.m_textoBusca.size();
           })->m_textoBusca.size();
}

// srcloc cinputmenufield.cpp:298 (inlined in AddItem): items fill a column downwards while they fit
// above min(screen height, m_pos.y + m_alturaMaxima) minus the instruction line and 5 px; then a new
// column starts 20 px to the right of the widest item.
SPoint CInputMenuField::NextItemPosition() const
{
    if (m_itens.empty())
        return m_pos;
    const TPosition alturaTela = CPolySingletonList::instance<IScreen>().GetHeight();    // slot 31, :298
    const SRect ultimo = m_itens.back().m_rect;
    const TPosition limite = static_cast<TPosition>(
        std::min<int>(alturaTela, m_pos.y + m_alturaMaxima)
        - (m_rectInstrucao.bottom - m_rectInstrucao.top + 1) - 5);
    if ((ultimo.bottom - ultimo.top + 1) + ultimo.bottom < limite)
        return SPoint{ultimo.left, ultimo.bottom};
    const auto maisLargo = std::max_element(m_itens.begin(), m_itens.end(),
        [](const CMenuItem& a, const CMenuItem& b) {
            return (a.m_rect.right - a.m_rect.left + 1) < (b.m_rect.right - b.m_rect.left + 1);
        });
    return SPoint{static_cast<TPosition>(maisLargo->m_rect.right - maisLargo->m_rect.left + ultimo.left + 21),
                  m_pos.y};
}

// wasm func 1693 (not observed) - srclocs cinputmenufield.cpp:320, :326, :339 (+ :298)
CMenuItem& CInputMenuField::AddItem(const std::string& texto, TMenuCallback callback)
{
    if (m_itens.size() == 99)
        throw CUeGuiError(static_cast<EUeGuiError>(4933), "Um CInputMenuField não comporta mais que 99 itens");

    const SPoint pos = NextItemPosition();
    auto& tela = CPolySingletonList::instance<IScreen>();                                   // srcloc :326
    CMenuItem& item = m_itens.emplace_back(CMenuItem{
        .m_menu = this, .m_numero = static_cast<int>(m_itens.size() + 1), .m_texto = texto, .m_pos = pos,
        .m_fonte = m_fonte, .m_callback = std::move(callback), .m_rect = {}, .m_destacado = false,
        .m_visivel = true, .m_textoBusca = std::to_string(m_itens.size() + 1), .m_dado = {}, .m_formato = {}});
    item.UpdateRect();

    const SRect anterior = m_rectItens;
    m_rectItens = Union(m_rectItens, item.m_rect);                                          // func 1916
    AtualizaLayout();

    const SRect total = Rect();
    if (total.right > tela.GetWidth() || total.bottom > tela.GetHeight() ||
        m_alturaMaxima < total.bottom - total.top + 1) {
        m_itens.pop_back();                  // undo
        m_rectItens = anterior;
        AtualizaLayout();
        throw CUeGuiError(static_cast<EUeGuiError>(4934), "Item de menu não cabe na tela [" + texto + "]");
    }
    if (!m_itens.empty())
        SetLength(MaiorTextoBusca());        // vtable slot 12
    Invalidate();
    return m_itens.back();
}

// wasm func 5489 (tools: api_f5489; not observed)                                   // name inferred
void CInputMenuField::SelecionaPorTextoBusca(const std::string& texto)
{
    LimpaSelecao();
    for (auto& item : m_itens) {
        if (item.m_visivel && item.m_textoBusca == texto) {
            item.m_destacado = true;
            m_selecionado = &item;
            Invalidate();
            return;
        }
    }
}

void CInputMenuField::LimpaSelecao()
{
    if (!m_selecionado)
        return;
    m_selecionado = nullptr;
    for (auto& item : m_itens)
        item.m_destacado = false;
    Invalidate();
}

// wasm func 10894 (not observed) - srcloc cinputmenufield.cpp:419, vtable slot 10
// Blocking read: the whole selection dialogue happens inside this call.
EInputResult CInputMenuField::Read(IInput& entrada)
{
    LimpaSelecao();
    entrada.Clear();                                      // IInput slot 4: discards pending keys
    for (;;) {
        while (!entrada.HasKey())                         // IInput slot 3
            std::this_thread::sleep_for(std::chrono::milliseconds(5));   // web: emscripten_sleep -> abort
        const EInputResult resultado = CInputField<CFramedText>::Read(entrada);   // func 6319
        switch (resultado) {
        case EInputResult::Branco:
            LimpaSelecao();
            Clear();                                      // slot 11
            return resultado;
        case EInputResult::Corrige:
            if (!m_selecionado)
                return resultado;
            LimpaSelecao();
            Clear();
            break;                                        // keep reading
        case EInputResult::Confirma:
            SelecionaPorTextoBusca(m_texto);
            if (m_selecionado) {
                // blink the chosen item once
                m_selecionado->m_destacado = !m_selecionado->m_destacado;
                Invalidate();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                m_selecionado->m_destacado = !m_selecionado->m_destacado;
                Invalidate();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                Clear();
                return resultado;
            }
            CPolySingletonList::instance<IBeep>().BeepErro();                             // slot 6, :419
            break;
        case EInputResult::Tecla:
            SelecionaPorTextoBusca(m_texto);             // highlight while typing
            break;
        default:
            break;
        }
    }
}

// wasm func 10897 (not observed) - vtable slot 2 (CMenuItem::Draw, :74, inlined)
void CInputMenuField::Draw(IScreen& tela) const
{
    tela.FillRect(Rect(), 1);
    for (const auto& item : m_itens)
        item.Draw();
    tela.DrawText(SPoint{m_rectInstrucao.left, m_rectInstrucao.top}, CFixedText(m_textoInstrucao),
                  m_fonte, 2, 0);                                                  // slot 19
    m_mascara.MaskText(tela, m_texto);                                             // func 2780
    if (m_foco && !m_cursorVisivel && m_texto.size() != m_tamanhoMaximo)
        m_mascara.DesenhaMoldura(m_texto.size(), 1, tela);                         // func 2779
}

// wasm func 10896 (not observed) - vtable slot 8
SRect CInputMenuField::Rect() const
{
    return Union(Union(m_rectItens, m_rectInstrucao), m_mascara.Rect());            // 1916, 1916, 3677
}

// wasm func 10895 (not observed) - srcloc cinputmenufield.cpp:278, vtable slot 9
void CInputMenuField::Move(const SPoint& pos)
{
    if (pos == m_pos)
        return;
    CPolySingletonList::instance<IScreen>().InvalidateRect(Rect(), 1);             // slot 5, :278  ?
    m_pos = pos;
    for (auto& item : m_itens) {
        // NOTE: adds the NEW absolute position to each item (not the displacement pos - old m_pos):
        // items only land in the right place if the menu was at (0,0) before.
        const SPoint novo{static_cast<TPosition>(pos.x + item.m_pos.x),
                          static_cast<TPosition>(pos.y + item.m_pos.y)};
        item.m_rect.MoveTo(novo);                                                  // func 1915
        item.m_pos = novo;
    }
    m_rectItens.MoveTo(m_pos);
    AtualizaLayout();
}

// wasm func 5491 (vtable slot 0) / 10898 (slot 1): ~m_formato, m_itens.clear() (each node: ~CMenuItem =
// func 5490, then free), ~m_textoInstrucao, then ~IInputField<IScreen> (func 4021).
CInputMenuField::~CInputMenuField() = default;

// wasm func 4021 (IInputField<IScreen>, slot 0) and 5517 (IInputField<IScreenMT>, slot 0) are thunks to
// the shared body func 6061 (tools: api_f6061), called with the two vtables of the instantiation:
//     vptr = IInputField vtable; if (m_timerCursor->IsRunning()) m_timerCursor->Stop();
//     ~m_timerCursor; ~m_validacao; ~m_texto; vptr = IFormFieldBase vtable; ~m_nome.

} // namespace api

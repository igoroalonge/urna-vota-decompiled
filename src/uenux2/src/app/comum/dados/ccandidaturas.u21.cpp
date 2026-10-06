// uenux2/src/app/comum/dados/ccandidaturas.cpp  -- FRAGMENT written by unit u21 (see u04-foreign-fragments.cpp for
// the sibling CCandidaturasDSNumero). Data source of the candidate photo shown on the confirmation screen.
#include "comum/dados/ccandidaturas.h"

#include <string>
#include <vector>

#include "comum/dados/cfotos.h"

namespace comum {

namespace {

// Inlined into wasm 12641 - name inferred. JPEG SOI (FF D8) at the start and EOI (FF D9) at the end.
// at() is used: an empty image, or a 1-byte image whose byte is 0xFF (the && stops earlier otherwise), throws
// std::out_of_range (vector::__throw_out_of_range, func 234). The last two at() are proven in range and compiled
// without a check.
bool EhJpeg(const std::vector<uebyte>& imagem)
{
    return imagem.at(0) == 0xFF && imagem.at(1) == 0xD8
        && imagem.at(imagem.size() - 2) == 0xFF && imagem.at(imagem.size() - 1) == 0xD9;
}

} // namespace

// wasm func 12641 = api::CDataImage<CCandidaturasDSFoto>::GetImage() (api::IImage vtable slot 2, vtable @1537532)
// with CCandidaturasDSFoto::operator() inlined. CDataImage<T> layout: +0 vptr, +4 T (here: uebyte m_indice).
// m_indice 0 = the titular, 1..n = suplente / vice n. Observed executing (every nominal vote confirmation screen).
// Returns an empty vector (no photo drawn) when the candidate has no photo or the photo is not a JPEG.
// NOTE: the photo is read and decoded TWICE from the -fo.dat file (once for the JPEG check, once for the result).
std::vector<uebyte> CCandidaturasDSFoto::operator()() const
{
    const md::CCandidatura& candidatura = GetCandidaturaAtual("CCandidaturasDSFoto");        // func 2838
    const md::CDadosCandidato& dados = (m_indice == 0) ? candidatura.GetTitular()             // +8
                                                       : candidatura.GetSuplente(m_indice);   // func 1389
    const std::string codigo = dados.GetCodigo();     // CDadosCandidato +0 (the key of the photo index)
    CFotos::GetInst();                                // func 2819; result unused (the lookup below inlines it again)
    if (!CFotos::Existe(codigo)) {                    // inlined map find + m_atual cache   // name inferred
        return {};
    }
    if (!EhJpeg(CFotos::GetImagem(codigo))) {         // func 3755
        return {};
    }
    return CFotos::GetImagem(codigo);                 // func 3755, again
}

} // namespace comum

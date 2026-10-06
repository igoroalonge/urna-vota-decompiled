// ecourna-lib/ecourna/app/dados/midias/cinformacaomidia.cpp
// (original build path: /home/rubio/.conan2/p/b/libecea1da310e5107/b/src/ecourna/app/dados/midias/cinformacaomidia.cpp)
// Reconstructed from vota_web_wasm.wasm, unit u14.
//
// Implicit members of CDadosGeracaoMidia compiled here: copy constructor = func 9032
// (vector + string + CIdentificadorGeradorMidia copy func 9872 + ptime), destructor = func 3490.
// ~vector<CAplicativo> = func 2664.
#include "ecourna/app/dados/midias/cinformacaomidia.h"

#include "ecourna/app/dados/dadoserros.h"

namespace ecourna::app::dados {

// wasm func 9031 (srcloc line 52)
CInformacaoMidia::CInformacaoMidia(ETipoMidia tipo, TFaseID fase, TProcessoEleitoralID idPE,
                                   const std::string& uf, TTurno turno, const CDadosGeracaoMidia& dadosGeracao)
    : m_tipoMidia(tipo)
    , m_fase(fase)
    , m_idPE(idPE)
    , m_uf(uf)
    , m_turno(turno)
    , m_dadosGeracao(dadosGeracao)
    , m_aplicativos()
{
    if (m_tipoMidia == MR) {
        throw CDadosMidiasError(3043, "Tipo de mídia inválido para este construtor: MR.");   // line 52
    }
}

// wasm func 9033 (no srcloc: nothing can throw except allocation; name inferred).
// Called by CConversorInformacaoMidia::DoDeconverte when `aplicativos` is present in the file.
// The element copy is the libc++ uninitialized-copy helper func 9185 (CAplicativo copy inlined),
// guarded by func 5104.
CInformacaoMidia::CInformacaoMidia(TFaseID fase, TProcessoEleitoralID idPE, const std::string& uf,
                                   TTurno turno, const CDadosGeracaoMidia& dadosGeracao,
                                   const TVectorAplicativo& aplicativos)
    : m_tipoMidia(MR)
    , m_fase(fase)
    , m_idPE(idPE)
    , m_uf(uf)
    , m_turno(turno)
    , m_dadosGeracao(dadosGeracao)
    , m_aplicativos(aplicativos)
{
}

// wasm func 9030 (srcloc line 61)
const TVectorAplicativo& CInformacaoMidia::GetAplicativos() const
{
    if (m_tipoMidia != MR) {
        throw CDadosMidiasError(3044,
                                "Apenas mídias de resultado possuem informação de inicialização de aplicativos.");   // line 61
    }
    return m_aplicativos;
}

} // namespace ecourna::app::dados

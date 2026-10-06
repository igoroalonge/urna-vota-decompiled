// Reconstructed from vota_web_wasm.wasm (unit u27). Original: uenux2/src/app/vota/operador/justificativa/iiniciajustificativa.cpp
// (attested: srcloc iiniciajustificativa.cpp:27 in the constructor).
//
// Functions: 3624 IIniciaJustificativa::IIniciaJustificativa, 10587 StartState (slot 2).
// Also filed here by the tools: 2747 = CJustificativaEfetuada::GetInst (constructor inlined) - moved to
// u27-foreign-fragments.cpp (path inferred justificativa/cjustificativaefetuada.cpp).
// The GetInst of the three concrete classes is inlined into CProcuraEleitor / CEleitorEncontrado.
//
// WEB BUILD: dead code (operator thread not run).
#include "vota/operador/justificativa/iiniciajustificativa.h"

#include "comum/justificativa/cjustificador.h"
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"
#include "vota/operador/comum/iinformacaothreadoperador.h"
#include "vota/operador/justificativa/cconfirmajustificativa.h"
#include "vota/operador/justificativa/cjustificativaefetuada.h"
#include "vota/votaerrors.h"                                   // CUeVotaError / EUeVotaError

namespace vota {

// ---------------------------------------------------------------------------------------------------
// wasm func 3624 (srcloc :27). CAppState(0): the state only transits.
IIniciaJustificativa::IIniciaJustificativa(IConfirmaJustificativa* confirma)
    : comum::CAppState(0)
    , m_confirma(confirma)
{
    if (m_confirma == nullptr)
        throw CUeVotaError(EUeVotaError{9396}, "Confirmação de justificativa nula");     // line 27
}

// ---------------------------------------------------------------------------------------------------
// wasm func 10587 (vtable slot 2 of the four classes)
void IIniciaJustificativa::StartState()
{
    const comum::md::CEleitorIdentidade identidade =
        impl::IInformacaoThreadOperador::GetInst().GetIdentidadeEleitor();                // func 1535 (slot 25)
    const std::string numero = identidade.GetIdentidade();

    comum::CJustificador& justificador = comum::CJustificador::GetInst();               // func 1391
    // CDataMap<CNumeroInscricaoEleitoral, CJustificadorDetalhe>::Localiza: find + make current (+12)
    if (justificador.Localiza(ecourna::app::dados::CNumeroInscricaoEleitoral(numero))) { // funcs 1241, 3741
        // Already justified in this urna: "já justificou" variant of the result screen.
        CJustificativaEfetuada& efetuada = CJustificativaEfetuada::GetInst();            // func 2747
        efetuada.m_novaJustificativa = false;                                            // +28
        m_proximoEstado = &efetuada;
        return;
    }
    m_proximoEstado = m_confirma;
}

// Concrete classes: GetInst + constructor inlined into their callers (lazy singletons).
CIniciaJustificativa::CIniciaJustificativa()
    : IIniciaJustificativa(&CConfirmaJustificativa::GetInst()) {}                // "não pertence <S|a|ao|à> <SCSN>"
CIniciaJustificativaTemporario::CIniciaJustificativaTemporario()
    : IIniciaJustificativa(&CConfirmaJustificativaTemporario::GetInst()) {}      // "está impedido de votar nesta seção"
CIniciaJustificativaTransito::CIniciaJustificativaTransito()
    : IIniciaJustificativa(&CConfirmaJustificativaTransito::GetInst()) {}        // "optou por votar EM TRÂNSITO"

}  // namespace vota

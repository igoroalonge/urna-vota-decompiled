// uenux2/src/app/comum/justificativa/cjustificador.cpp
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// srclocs of this file: :55 CJustificador::Justifica, :83 CJustificador::SaveCurrentInternal (both inlined
// into vota::CPedeAnoNascimento, wasm 10590) and :104 CJustificadorDadoTitulo::Text (wasm 11475).
// Functions of the unit: 2810, 2899, 3740, 11464, 11475, 11477, 2260.
// Nothing here ran in the recorded sessions: the public simulator only drives the voter terminal, and a
// justificativa is entered on the mesário terminal.
#include "comum/justificativa/cjustificador.h"

#include <format>

#include "api/gui/capplicationcontextguard.h"
#include "api/persistencia/cdaorepositorio.hpp"
#include "api/util/csynchronizer.h"
#include "comum/appinfo/cappinfo.h"
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"   // CIdentificacaoJustificativa

namespace comum {

using ecourna::app::dados::CNumeroInscricaoEleitoral;

// ---------------------------------------------------------------------------------------------------
// wasm func 2260 (tools: comum_f2260) - merged "construct CUeComumJustificativaError(code, msg, where)"
// thunk: calls the shared CBaseError constructor body (ecourna func 710) with vtable @1560732.
// Callers: 11471, 11472, 11475 and the inlined Justifica/SaveCurrentInternal in 10590. Every
// `throw CUeComumJustificativaError(...)` below goes through it.

// ---------------------------------------------------------------------------------------------------
// servico::CJustificadorServico

// wasm func 2899 (tools: comum_f2899) - merge-similar-functions body shared by four destructors
// whose classes have the layout {vptr, std::shared_ptr<X>}; the vtable is passed as a parameter:
//   3740  comum::servico::CJustificadorServico           (vtable @1560936)   - this file
//   3607  comum::servico::CComparecimentoMesarioServico  (vtable @1595676)   - other unit
//   10888 api::CFormPart                                 (vtable @1583696)   - see u24-foreign-fragments.cpp
//   11205 comum::CParteEleitores                         (vtable @1576596)   - see u24-foreign-fragments.cpp
// Body: store the vptr, release the shared_ptr (use_count at ctrl+4; on zero: ctrl->__on_zero_shared()
// = vtable slot 2, then __release_weak), return this.

// wasm func 3740 - vtable slot 0 (complete-object destructor): comum_f2899(this, vtable @1560936)
servico::CJustificadorServico::~CJustificadorServico() = default;   // releases m_dao

// wasm func 11464 - vtable slot 1 (deleting destructor): ~CJustificadorServico() then operator delete.

// Constructor, inlined into wasm 3742 (unit u20).
servico::CJustificadorServico::CJustificadorServico()
    : m_dao(api::persistencia::CDAORepositorio::Entregar<dao::IJustificadorDAO>())   // cdaorepositorio.hpp:79
{
    // Entregar looks up typeid(IJustificadorDAO).name() = "N5comum3dao16IJustificadorDAOE" in the
    // repository map (@1909964), clones the registered prototype (IDAO slot 2), dynamic_casts it to
    // IJustificadorDAO and wraps it in a new shared_ptr; otherwise throws CUePersistenciaError 6802
    // "Classe DAO {} não foi registrada.". The prototype is registered by
    // CInformacaoEleitor::InicializarPersistencia (inlined in wasm 7787).
}

// ---------------------------------------------------------------------------------------------------
// CJustificador

std::mutex                     CJustificador::s_mutex;        // @1838988
std::unique_ptr<CJustificador> CJustificador::s_pInstancia;   // @1839012

// wasm func 1391 (unit u35) and inlined in 11474 / 11475.
CJustificador& CJustificador::GetInst()
{
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!s_pInstancia)
        s_pInstancia.reset(new CJustificador());                 // operator new(40) + wasm 3742
    return *s_pInstancia;
}

// wasm func 3742 (unit u20; the tools call it CDAORepositorio::Entregar@3742).
CJustificador::CJustificador()
    : api::CDataMap<CNumeroInscricaoEleitoral, CJustificadorDetalhe>("CJustificador"),
      m_servico()
{
}

// wasm func 2810 (tools: comum_f2810). Also called by 1391/11474/11475/11477 when the unique_ptr is reset.
// Member order of destruction: m_servico (direct call of D1 3740 on this+28), then the CDataMap base:
// m_nome (+16) and the map (tree destroy, wasm 2809 = std::__tree<...>::destroy, whose node value
// destructor is the IIdentificadorEleitor dtor 778 on node+16).
CJustificador::~CJustificador() = default;

// wasm func 11477 (tools: comum_f11477, table slot 2465) - the atexit destructor of s_pInstancia:
//   CJustificador* p = s_pInstancia.release(); if (p) { p->~CJustificador(); operator delete(p); }
// (i.e. the compiler-generated ~unique_ptr<CJustificador>() for the static above)

// cjustificador.cpp:55 - only inlined (wasm 10590, unit u17).
// Called when the mesário confirms the year of birth of a voter who justifies.
void CJustificador::Justifica(const CNumeroInscricaoEleitoral& titulo, ueword anoNascimento)
{
    const auto it = m_container.find(titulo);                            // shared_f3741
    if (it != m_container.end()) {
        m_atual = it;                                                     // the cursor is moved first
        throw CUeComumJustificativaError(EUeComumJustificativaError{8800},
                                         "Já havia justificativa para o título");         // line 55
    }
    Add({titulo, CJustificadorDetalhe{anoNascimento}});                   // CDataMap::Add (wasm 5725)
}

// cjustificador.cpp:83 - only inlined (wasm 10590). Persists the *current* justification.
// In 10590 it is surrounded by (probably another CJustificador method, name unknown):          // ?
//   if (urna desligando) throw api::CUeDesligandoError;                          (flag @1832936)
//   CApplicationContextGuard(2, "", "Gravando a justificativa na MI",
//        "Ocorreu um erro durante a sincronização da justificativa na MI.");
//   CAppInfo::SalvaVotaInterno();  SaveCurrentInternal();
//   AssinarUE(CAssinador(<uenux.db SAVD file 122 if CEstadoGeral+32 == '1' else 123>), 31);
//   CSynchronizer::Sincroniza(); <signs uenux.db and copies it, wasm 4657>;
//   CApplicationContextGuard(4, "", "Gravando a justificativa na MV",
//        "Ocorreu um erro durante a sincronização da justificativa na MV.");
//   CAppInfo::SalvaVotaExterno();  <copy uenux.db to the external flash, wasm 4687>;  Sincroniza().
void CJustificador::SaveCurrentInternal()
{
    if (m_container.empty())                                              // tests size (+8), not the cursor
        throw CUeComumJustificativaError(EUeComumJustificativaError{8801},
                                         "Não há dados a serem salvos na MI");            // line 83

    const auto& [titulo, detalhe] = *m_atual;
    const ecourna::app::dados::CIdentificacaoJustificativa justificativa(          // shared_f1875
        ecourna::app::dados::CRegistroIdentificacaoEleitor(                          // func 1675
            std::make_shared<CNumeroInscricaoEleitoral>(titulo)),
        ecourna::app::dados::TAnoNascimento(detalhe.m_anoNascimento));             // CBaseType<0,9999>

    const auto& dao = m_servico.GetDAO();
    const std::string numero = justificativa.GetIdentificacao().GetIdentificadorHabilitacao()->GetNumero();
    if (!dao.Recuperar(numero))                                           // IJustificadorDAO slot 7
        dao.Inserir(justificativa);                                       // slot 3 (only if not yet stored)
}

// ---------------------------------------------------------------------------------------------------
// wasm func 11475 (srcloc line 104). Table slot 2949: a text source (function pointer) of the BUJ /
// justification screens. Returns the título of the current justification (the CDataMap cursor).
std::string CJustificadorDadoTitulo::Text()
{
    const CJustificador& justificador = CJustificador::GetInst();         // inlined 1391
    if (justificador.Eof())                                               // m_atual == end()  (+12 vs +4)
        throw CUeComumJustificativaError(EUeComumJustificativaError{8802},
                                         "Justificativa inválida");                       // line 104
    return justificador.GetTituloAtual().GetNumero();                     // body ICF-merged: func 1139
}

} // namespace comum

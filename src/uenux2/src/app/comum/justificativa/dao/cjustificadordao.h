// uenux2/src/app/comum/justificativa/dao/cjustificadordao.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// RTTI: comum::dao::CJustificadorDAO : comum::dao::IJustificadorDAO
//         : api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string>
//         : api::persistencia::IDAO
// vtable @1560768 (same slot order as CEleitorDinamicoDAO / CComparecimentoMesarioDAO):
//   [0] 1704  ~CJustificadorDAO (ICF)          [1] 3767  deleting dtor (ICF)
//   [2] 11468 Clonar                           [3] 11473 Inserir
//   [4] 11467 Excluir (IUenuxGenericDAO default, throws 6804; unit u20)
//   [5] 11472 ExcluirID (srcloc :56)           [6] 11471 Atualizar (srcloc :63)
//   [7] 11470 Recuperar                        [8] 11469 RecuperarTodos
// Slots 5/6 are named by srcloc, 4 by the base-class srcloc; 2/3/7/8 are inferred from behaviour and from
// the sibling DAOs (unit u05 calls slot 2 "Clone", unit u20 "Clonar").
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "api/persistencia/iuenuxgenericdao.h"
#include "ecourna/api/sql/isqlconnection.hpp"
#include "ecourna/app/dados/resultadournacadastro/cdadoscomparecimento.h"   // CIdentificacaoJustificativa (24 bytes)

namespace comum::dao {

class IJustificadorDAO
    : public api::persistencia::IUenuxGenericDAO<ecourna::app::dados::CIdentificacaoJustificativa, std::string> {
public:
    // No new virtual slot: the vtable ends at slot 8, so these are (re)declarations of the IDAO (slot 2) and
    // IUenuxGenericDAO (slots 3, 7, 8) virtuals; their slot numbers come from the base-class declaration order
    // (iuenuxgenericdao.h: Excluir :41 = slot 4, ExcluirID :49 = 5, Atualizar :57 = 6).
    virtual api::persistencia::IDAO* Clonar() const = 0;                         // slot 2
    virtual void Inserir(const TDado& justificativa) const = 0;                  // slot 3
    virtual std::shared_ptr<TDado> Recuperar(const TDadoID& titulo) const = 0;   // slot 7
    virtual std::vector<TDado> RecuperarTodos() const = 0;                       // slot 8
};

// sizeof 12: vptr, std::shared_ptr<ISqlConnection> m_conexao (+4 / +8).
class CJustificadorDAO : public IJustificadorDAO {
public:
    // Inlined into CInformacaoEleitor::InicializarPersistencia (wasm 7787, units u02/u07):
    // opens <trab MI>/uenux.db and runs the CREATE TABLE below.
    explicit CJustificadorDAO(const std::string& caminhoBanco);

    api::persistencia::IDAO* Clonar() const override;                            // wasm 11468
    void Inserir(const TDado& justificativa) const override;                     // wasm 11473
    void ExcluirID(const TDadoID& titulo) const override;                        // wasm 11472 (line 56)
    void Atualizar(const TDado& justificativa) const override;                   // wasm 11471 (line 63)
    std::shared_ptr<TDado> Recuperar(const TDadoID& titulo) const override;      // wasm 11470
    std::vector<TDado> RecuperarTodos() const override;                          // wasm 11469

private:
    std::shared_ptr<ecourna::api::sql::ISqlConnection> m_conexao;                // +4 / +8 (CSqlConnection)
};

} // namespace comum::dao

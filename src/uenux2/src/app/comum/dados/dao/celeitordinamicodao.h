// Reconstructed from vota_web_wasm.wasm (unit u05).
// Original: uenux2/src/app/comum/dados/dao/celeitordinamicodao.h
//
// RTTI: comum::dao::CEleitorDinamicoDAO : comum::dao::IEleitorDinamicoDAO
//         : api::persistencia::IUenuxGenericDAO<comum::md::CEleitorDinamico, std::string>
//         : api::persistencia::IDAO
// vtable @1558904. Slot order (identical in CJustificadorDAO and CComparecimentoMesarioDAO):
//   0 ~CEleitorDinamicoDAO (ICF 1704)   1 deleting dtor (ICF 3767)   2 Clone
//   3 Inserir   4 Excluir (base default, throws)   5 ExcluirID   6 Atualizar   7 Recuperar
//   8 RecuperarTodos
// Names of slots 2/3/7/8 are inferred from behaviour; 4/5/6 come from iuenuxgenericdao.h srclocs.
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "api/persistencia/iuenuxgenericdao.h"
#include "ecourna/api/sql/isqlconnection.h"
#include "md/eleitor/celeitordinamico.h"

namespace comum::dao {

class IEleitorDinamicoDAO : public api::persistencia::IUenuxGenericDAO<md::CEleitorDinamico, std::string> {};

class CEleitorDinamicoDAO : public IEleitorDinamicoDAO {
public:
    explicit CEleitorDinamicoDAO(const std::string& caminhoBanco);   // e.g. <trab>/uenux.db

    api::persistencia::IDAO* Clone() const override;                         // slot 2 name inferred
    void Inserir(const TDado& eleitor) const override;                       // slot 3 name inferred
    void ExcluirID(const TDadoID& titulo) const override;                    // slot 5 (srcloc :130)
    void Atualizar(const TDado& eleitor) const override;                     // slot 6
    std::shared_ptr<TDado> Recuperar(const TDadoID& titulo) const override;  // slot 7 name inferred
    std::vector<TDado> RecuperarTodos() const override;                      // slot 8 name inferred

private:
    static const std::string TABELA;   // "eleitor_dinamico" (std::string global @1838780, static init)
    std::shared_ptr<ecourna::api::sql::ISqlConnection> m_conexao;   // +4/+8 (CSqlConnection)
};

}  // namespace comum::dao

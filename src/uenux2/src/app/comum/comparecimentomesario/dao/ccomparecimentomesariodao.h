// uenux2/src/app/comum/comparecimentomesario/dao/ccomparecimentomesariodao.h  (path inferred)
// Reconstructed from vota_web_wasm.wasm (unit u22).
//
// RTTI: comum::dao::CComparecimentoMesarioDAO : comum::dao::IComparecimentoMesarioDAO
//         : api::persistencia::IUenuxGenericDAO<md::CComparecimentoMesario, md::CComparecimentoMesarioPK>
//         : api::persistencia::IDAO                          typeinfo @1593440, vtable @1593360
// Slot order (same as CEleitorDinamicoDAO / CJustificadorDAO, see celeitordinamicodao.h of u05):
//   0 dtor (ICF 1704)  1 deleting dtor (ICF 3767)  2 Clone (10399)  3 Inserir (10403)
//   4 Excluir / 5 ExcluirID / 6 Atualizar (IUenuxGenericDAO defaults 10398/10397/10396: throw)
//   7 Recuperar (10402)  8 RecuperarTodos (10401)
// Construction (CREATE TABLE IF NOT EXISTS comparecimento_mesario ...) happens in the voter start-up
// routine (func 7787, u02 step 5).
#pragma once

#include <memory>
#include <vector>

#include "api/persistencia/iuenuxgenericdao.h"
#include "comum/comparecimentomesario/md/ccomparecimentomesario.h"
#include "ecourna/api/sql/isqlconnection.h"

namespace comum::dao {

class IComparecimentoMesarioDAO
    : public api::persistencia::IUenuxGenericDAO<md::CComparecimentoMesario, md::CComparecimentoMesarioPK> {};

class CComparecimentoMesarioDAO : public IComparecimentoMesarioDAO
{
public:
    explicit CComparecimentoMesarioDAO(std::shared_ptr<ecourna::api::sql::ISqlConnection> conexao);

    api::persistencia::IDAO* Clone() const override;                          // slot 2 (func 10399)
    void Inserir(const TDado& comparecimento) const override;                 // slot 3 (func 10403)
    std::shared_ptr<TDado> Recuperar(const TDadoID& chave) const override;    // slot 7 (func 10402)
    std::vector<TDado> RecuperarTodos() const override;                       // slot 8 (func 10401)

private:
    md::CComparecimentoMesario::EstadoReconhecimentoBiometrico
        ConverteEstadoBiometria(std::uint32_t valor) const;                   // func 5391 (srcloc :200)
    md::CDedo::TipoDedo ConverteDedoHabilitacao(const std::uint32_t valor) const;   // func 5390 (srcloc :232)

    std::shared_ptr<ecourna::api::sql::ISqlConnection> m_conexao;             // +4/+8
};

} // namespace comum::dao

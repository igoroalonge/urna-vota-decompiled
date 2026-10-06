// uenux2/src/app/comum/justificativa/cjustificador.h
// Reconstructed from vota_web_wasm.wasm (unit u24).
//
// "Justificativa eleitoral": a voter who is away from his electoral domicile goes to any polling station
// and *justifies* his absence instead of voting. The mesário types the voter's título (número de
// inscrição eleitoral, 12 digits) and year of birth; the urna records them in the SQLite table
// registro_justificativa (uenux.db) and, at the end of the day, in the result file "jufa.dat"
// (ModuloResultadoUrnaCadastro) and on the printed "BUJ" (Boletim de Justificativa).
//
// Classes of this file (RTTI exists only for the polymorphic ones):
//   comum::CJustificador            not polymorphic, 40 bytes, lazy singleton (static unique_ptr @1839012,
//                                   mutex @1838988). = api::CDataMap<CNumeroInscricaoEleitoral,
//                                   CJustificadorDetalhe>("CJustificador") + a CJustificadorServico member.
//   comum::servico::CJustificadorServico   RTTI "class" (no base), vtable @1560936 = {D1 3740, D0 11464}.
//                                   Thin service around the DAO obtained from api::persistencia::CDAORepositorio.
//   comum::CJustificadorDadoTitulo  no RTTI; only a static Text() used as a report/screen text source.
#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "api/io/cdatamap.h"                                     // api::CDataMap<K, V> (unit u18)
#include "comum/justificativa/dao/cjustificadordao.h"           // dao::IJustificadorDAO
#include "ecourna/app/dados/cnumeroinscricaoeleitoral.h"
#include "ecourna/api/exception/cbaseerror.hpp"

namespace comum {

// Error family of this directory. CBaseError<EUeComumJustificativaError, SErrorLimits{8800, 8850}>
// (typeinfo @1560680, vtable @1560732). Codes used in the binary:
//   8800 "Já havia justificativa para o título"            cjustificador.cpp:55  (inlined in 10590)
//   8801 "Não há dados a serem salvos na MI"               cjustificador.cpp:83  (inlined in 10590)
//   8802 "Justificativa inválida"                          cjustificador.cpp:104 (11475)
//   8804 "Nao é esperado que o CJustificadorDAO exclua registros do banco"     cjustificadordao.cpp:56
//   8805 "Nao é esperado que o CJustificadorDAO atualize registros do banco"   cjustificadordao.cpp:63
//   (8803 does not occur in the binary.)
enum class EUeComumJustificativaError : int {};                                   // enumerators unknown
using CUeComumJustificativaError =
    ecourna::api::exception::CBaseError<EUeComumJustificativaError,
                                        ecourna::api::exception::SErrorLimits{8800, 8850}>;

// Value type of the justification table: the year of birth typed by the mesário.
// Map node layout: key CNumeroInscricaoEleitoral at node+16 (24 bytes), detalhe at node+40.
struct CJustificadorDetalhe {                                                      // shape inferred
    ueword m_anoNascimento;                                                        // +0 (node+40), 0..9999
};

namespace servico {

// sizeof 12. vptr +0, shared_ptr<dao::IJustificadorDAO> +4/+8.
class CJustificadorServico {
public:
    // Constructor inlined into CJustificador::CJustificador (wasm 3742, unit u20):
    //   m_dao = api::persistencia::CDAORepositorio::Entregar<dao::IJustificadorDAO>()  (cdaorepositorio.hpp:79)
    CJustificadorServico();
    virtual ~CJustificadorServico();          // wasm 3740 (D1, via ICF body 2899) / 11464 (D0)

    const dao::IJustificadorDAO& GetDAO() const { return *m_dao; }                 // inlined

private:
    std::shared_ptr<dao::IJustificadorDAO> m_dao;                                  // +4 / +8
};

} // namespace servico

// sizeof 40.
class CJustificador
    : public api::CDataMap<ecourna::app::dados::CNumeroInscricaoEleitoral, CJustificadorDetalhe> {
public:
    // wasm 1391 (unit u35; also inlined in 11474/11475). Lazy creation guarded by s_mutex (the lock is
    // a no-op in this single-threaded build: only the unlock residue remains).
    static CJustificador& GetInst();

    CJustificador();                          // wasm 3742 (unit u20): CDataMap("CJustificador") + m_servico()
    ~CJustificador();                         // wasm 2810

    // cjustificador.cpp:55 / :83, both only inlined into vota::CPedeAnoNascimento (wasm 10590, unit u17).
    void Justifica(const ecourna::app::dados::CNumeroInscricaoEleitoral& titulo, ueword anoNascimento);
    void SaveCurrentInternal();

    // Key of the record under the cursor (inlined in 11475; name inferred). Undefined when Eof().
    const ecourna::app::dados::CNumeroInscricaoEleitoral& GetTituloAtual() const { return m_atual->first; }

private:
    // +0  std::map<CNumeroInscricaoEleitoral, CJustificadorDetalhe> m_container   (CDataMap)
    // +12 iterator m_atual                                                           (CDataMap)
    // +16 std::string m_nome = "CJustificador"                                       (CDataMap)
    servico::CJustificadorServico m_servico;                                       // +28

    static std::mutex                     s_mutex;      // @1838988
    static std::unique_ptr<CJustificador> s_pInstancia; // @1839012 (atexit reset = wasm 11477)
};

// Text source "título of the current justification" (reports / micro-terminal). Registered by
// address (table slot 2949); the neighbouring slot 2948 is wasm 11474 (unit u36), another text
// source over the same singleton that prints the number of justifications.
class CJustificadorDadoTitulo {
public:
    static std::string Text();                // wasm 11475 (srcloc line 104)
};

} // namespace comum

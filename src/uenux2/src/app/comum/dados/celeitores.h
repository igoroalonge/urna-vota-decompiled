// Reconstructed from vota_web_wasm.wasm (unit u04). Original: uenux2/src/app/comum/dados/celeitores.h
//
// CEleitores = the section's voter list ("cadastro de eleitores da seção"), keyed by the voter's
// principal identifier (título / CPF / livre). Process-wide singleton (GetInst line 44). The object is
// constructed, and StaticLoad runs, inlined into the start-up function 7787; StaticLoad's own srclocs
// are its two error sites: line 395 = 7840 "Eleitor {} com biometria em urna não biométrica" (a roll
// record has biometrics while +48 is false) and line 407 = 7841 "Identidade principal {} duplicada".
//
//   StaticLoad(dir)            reads the static roll (*-el.dat + *-tte.dat via GetEleitoresEstaticos,
//                              *-imp.dat via GetEleitoresImpedidos) -> one CEleitorDetalhe per voter
//                              with no dynamic data.
//   DynamicCreate(db)          (once, at "gera base dinâmica") writes one row per voter in the SQLite
//                              table eleitor_dinamico of uenux.db (estado FALTOU / SEM_CARGO_PARA_VOTAR).
//   CompleteLoad(dir, db)      rebuilds the list joining static roll + impediments + SQLite rows.
//   MarcaEleitorFoiHabilitado  / MarcaVotou (line 456/460, inlined in 10210) update the dynamic row
//                              of the current voter after the poll worker enables him / after he votes.
//
// RTTI: comum::CEleitores [vmi] : api::CDataMap<md::CEleitorIdentidade, CEleitorDetalhe> (offset 4),
//                                 comum::IDataSourceAptos (offset 0)
// The vmi base list gives the DECLARATION order (CDataMap first). IDataSourceAptos is the only
// polymorphic base (vtable @1559120 has exactly one slot -> wasm 2824), so the Itanium ABI makes it the
// primary base at +0 and the non-polymorphic CDataMap sub-object lands at +4.
#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "celeitordetalhe.h"
#include "../../../api/io/cdatamap.h"          // api::CDataMap<K, V>

namespace comum {

using TMunicipioID = std::uint32_t;

// Quantities of voters able to vote ("aptos") for one abrangência. 4 bytes (two uint16 at +0/+2 of
// the map value, stored with 2-byte alignment).
struct SQtdeAptos {                         // name from RTTI (std::function<comum::SQtdeAptos()>)
    std::uint16_t qtdAptosSecao = 0;        // voters of the section roll        -> QR "APTS:"
    std::uint16_t qtdAptosTTE   = 0;        // voters in temporary transfer (TTE) -> QR "APTT:"
};                                          // APTA / APTO = sum of both

class IDataSourceAptos {                    // (idatasourceaptos.h) - no virtual destructor slot
public:
    virtual std::map<md::ETipoAbrangencia, SQtdeAptos> GetQtdAptos() const = 0;   // slot 0, name inferred
};

class CEleitorDadosHabilitacao;            // md: data collected when the voter is enabled (unit u05)

class CEleitores : public api::CDataMap<md::CEleitorIdentidade, CEleitorDetalhe>,
                   public IDataSourceAptos {
public:
    static CEleitores& GetInst();                                        // line 44  (wasm 326)

    void StaticLoad(const std::string& dirEstatico);                     // lines 395/407 (in 7787)
    void DynamicCreate(const std::string& arquivoBanco) const;           // lines 301/305/358 (wasm 5761)
    void CompleteLoad(const std::string& dirEstatico,
                      const std::string& arquivoBanco);                  // lines 205/219/238 (wasm 6734, inlined)
    void MarcaEleitorFoiHabilitado(const CEleitorDadosHabilitacao& dados); // lines 435/439/447 (wasm 2825)
    void MarcaVotou();                                                   // lines 456/460 (inlined in 10210):
                                                                         // 7845 "Item inexistente", 7846 "Dados dinâmicos
                                                                         // não carregados"; estado <- VOTOU and ++m_qtdVotaram
                                                                         // unless it already was VOTOU (then: no-op)
    md::ETipoAbrangencia GetCurrentAbrangenciaEleitor() const;           // line 497 (inlined in 7377)

    std::map<md::ETipoAbrangencia, SQtdeAptos> GetQtdAptos() const override;   // wasm 2824
    SQtdeAptos GetQtdAptosSecao() const;                                 // comum_f2823 (not in u04) name inferred
    std::uint16_t QtdVotaramPorTipoHabilitacao(md::ETipoHabilitacao tipo) const; // wasm 6034 name inferred
    std::uint32_t GetQtdVotaram() const { return m_qtdVotaram; }

private:
    CEleitorDetalhe LoadEleitorDetalhe(const md::CEleitorDecorator& eleitor,
                                       const std::vector<md::CImpedido>& impedidos,
                                       std::size_t& indiceImpedido,
                                       const md::CEleitorDinamico* dinamico) const;   // line 719 (wasm 5764)
    void ValidaIdentidadesEleitor(const md::CEleitorDecorator& eleitor) const;        // line 738 (wasm 5765)

    // Loaders (in unit u02/u05 code; their std::function lambdas have RTTI):
    //   GetEleitoresEstaticos(const std::string&)::$_0          -> vector<md::CEleitor>  (api_f5772)
    //   GetEleitoresImpedidos(const std::vector<std::string>&)::$_0 -> vector<md::CImpedido> (api_f5767)
    std::vector<md::CEleitorDecorator> GetEleitoresEstaticos(const std::string& dirEstatico) const;
    std::vector<md::CImpedido> GetEleitoresImpedidos(const std::vector<std::string>& arquivos) const;

    // +0   vptr (IDataSourceAptos)
    // +4   api::CDataMap: std::map<CEleitorIdentidade, CEleitorDetalhe> (+4 begin, +8 root, +12 size),
    //      current iterator (+16), record-type name for messages (+20, std::string "CEleitores")
    std::string                    m_uf;                        // +32  UF of this urna (lower case)
    TMunicipioID                   m_municipio;                 // +44  município of this urna
    bool                           m_urnaBiometrica;            // +48  checked by StaticLoad (7840)  name inferred
    std::set<md::ETipoAbrangencia> m_abrangenciasCargos;        // +52  abrangências of the cargos voted here
    std::vector<std::string>       m_arquivosEleitores;         // +64  roll file names read by
    std::vector<std::string>       m_arquivosTTE;               // +76  GetEleitoresEstaticos (api_f5772)  // ? which is which
    std::vector<std::string>       m_arquivosImpedidos;         // +88  names of the *-imp.dat files (one per
                                                                //      aggregated section)            // ?
    ecourna::app::dados::ETipoIdentificadorEleitor m_tipoIdentificadorPrincipal;   // +100
    std::uint32_t                  m_qtdVotaram = 0;            // +104 voters whose dynamic state is VOTOU
                                                                //      (i32 store in 6734, i32 ++ in MarcaVotou/10210)
    bool                           m_dinamicosCarregados = false; // +108
    std::map<md::CEleitorIdentidade, md::CEleitorIdentidade> m_identidadePrincipal; // +112 any id -> principal id

    static std::mutex  s_mutex;   // @1838796
    static CEleitores* s_inst;    // @1838792
};

// Text sources for the poll worker's screen ("dados do eleitor"): static functions called through
// function pointers (table slots 4056.. / 4113) by api::CTextSource (vota_f5401).
struct CEleitorDadoNomeParaUrna { static std::string Text(const std::string& formato); };  // line 756 (11529)
struct CEleitorDadoSequencial   { static std::string Text(const std::string& formato); };  // line 772 (11528)
struct CEleitorDadoSecao        { static std::string Text(const std::string& formato); };  // line 784 (11527)
struct CEleitorDadoTTE          { static std::string Text(); };                            // line 796 (11526)

}  // namespace comum

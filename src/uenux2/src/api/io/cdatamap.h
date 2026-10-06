// uenux2/src/api/io/cdatamap.h
//
// Reconstructed from vota_web_wasm.wasm. Unit u18.
//
// api::CDataMap<KeyType, RecordType>: a std::map with a "current record" cursor and a name used in the
// error messages. It is the base of the application's in-memory tables:
//   comum::CEleitores        CDataMap<md::CEleitorIdentidade, CEleitorDetalhe>        (voter roll)
//   comum::CCandidaturas     CDataMap<unsigned, md::CCandidatura>                     (candidates)
//   comum::CPartidos         CDataMap<unsigned short, md::CPartido>                   (parties)
//   comum::CRespostas        CDataMap<unsigned, md::CRespostaConsulta>                (consulta answers)
//   comum::CJustificador     CDataMap<ecourna::app::dados::CNumeroInscricaoEleitoral, CJustificadorDetalhe>
//   comum::CRegistradorMesario  CDataMap<md::CComparecimentoMesarioPK, md::CComparecimentoMesario>
//                                                     (mesários' attendance; class name inferred, see u18 doc)
// Screens and reports walk a table with the cursor: "position on first", GetCurrent(), Next() ...
//
// Not polymorphic, 28 bytes (layout from the inlined code of every instance):
//   +0  Container m_container      std::map<KeyType, RecordType> (begin node +0, root +4, size +8)
//   +12 iterator  m_atual          == end() when there is no current record
//   +16 std::string m_nome         table name used in the messages (e.g. "CCandidaturas")
//
// Errors: api::EUeIoError (CBaseError limits {5950, 6150}), all thrown as `throw CUeIoError(..., current())`
// at column 19 of the srcloc records:
//   98  GetCurrent  5976 "O registro corrente estava inválido {}"
//   117 Next        5977 "Operação inválida {}"
//   143 Add         5978 "Duplicidade de registro {}"
//   158 Update      5979 "Registro não encontrado {}"
//
// wasm-opt merged the code of the instances that only differ in constants:
//   wasm 2919 = GetCurrent body, parameters (this, offset of the value inside the tree node, srcloc)
//               thunks 656 (CEleitorIdentidade: value at node+32), 1200 (unsigned/CCandidatura: node+20),
//               1283 (unsigned short/CPartido: node+20), 3125 (unsigned/CRespostaConsulta: node+20)
//   wasm 3892 = Next body, parameters (this, srcloc); thunks 3697 (CPartido), 5601 (CEleitorDetalhe),
//               5611 (CCandidatura). CJustificadorDetalhe's Next is inlined in 12105.
#pragma once

#include <format>
#include <map>
#include <source_location>
#include <string>
#include <utility>

#include "api/io/euioerror.h"         // api::EUeIoError, api::CUeIoError

namespace api {

template <typename KeyType, typename RecordType>
class CDataMap {
public:
    using Container  = std::map<KeyType, RecordType>;
    using iterator   = typename Container::iterator;

    explicit CDataMap(const std::string& nome) : m_atual(m_container.end()), m_nome(nome) {}   // inlined everywhere

    // cdatamap.h:98 - wasm 2919 (merged) via thunks 656 / 1200 / 1283 / 3125. Only 656 (CEleitores: the voter
    // being enabled / voting) was observed executing in the recorded sessions.
    RecordType* GetCurrent() const
    {
        if (m_atual == m_container.end())
            throw CUeIoError(EUeIoError(5976), std::format("O registro corrente estava inválido {}", m_nome));   // line 98
        return &m_atual->second;
    }

    // cdatamap.h:117 - wasm 3892 (merged) via thunks 3697 / 5601 / 5611.
    void Next()
    {
        if (m_atual == m_container.end())
            throw CUeIoError(EUeIoError(5977), std::format("Operação inválida {}", m_nome));                    // line 117
        ++m_atual;
    }

    // cdatamap.h:143 - inserts a new record and makes it the current one.
    // wasm 5725 (CNumeroInscricaoEleitoral -> CJustificadorDetalhe): this overload as is (the pair is the argument).
    void Add(const typename Container::value_type& registro)
    {
        if (m_container.find(registro.first) != m_container.end())
            throw CUeIoError(EUeIoError(5978), std::format("Duplicidade de registro {}", m_nome));             // line 143
        m_atual = m_container.insert(registro).first;
    }

    // wasm 5377 (CComparecimentoMesarioPK -> CComparecimentoMesario; srcloc of line 143): the wasm function takes
    // (this, key, record), builds the pair and inlines Add(value_type).                    overload name inferred
    void Add(const KeyType& chave, const RecordType& registro)
    {
        Add(typename Container::value_type(chave, registro));
    }

    // cdatamap.h:158 - replaces an existing record and makes it the current one. Only exists inlined
    // (in wasm 5378, whose outer function is comum::CRegistradorMesario's "save" method, see u18 doc).
    void Update(const KeyType& chave, const RecordType& registro)
    {
        const auto it = m_container.find(chave);
        if (it == m_container.end())
            throw CUeIoError(EUeIoError(5979), std::format("Registro não encontrado {}", m_nome));             // line 158
        it->second = registro;
        m_atual = it;
    }

    // Seen inlined in the callers (names inferred):
    bool Localiza(const KeyType& chave)                    // find + make current; false when absent
    {
        const auto it = m_container.find(chave);
        if (it == m_container.end())
            return false;
        m_atual = it;
        return true;
    }
    void First()             { m_atual = m_container.begin(); }
    bool Eof() const         { return m_atual == m_container.end(); }
    std::size_t Size() const { return m_container.size(); }
    void Clear()             { m_container.clear(); m_atual = m_container.end(); }

protected:
    Container   m_container;   // +0
    iterator    m_atual;       // +12
    std::string m_nome;        // +16
};

} // namespace api

// uenux2/src/app/comum/gravadores/cgravadorbu.h
// Reconstructed from vota_web_wasm.wasm (unit u23).
//
// CGravadorBU writes the digital Boletim de Urna, "<fase><pleito><uf><mun><zona><secao>-bu.dat":
//   ModuloEnvelopeGenerico::EntidadeEnvelopeGenerico { tipoEnvelope = envelopeBoletimUrna,
//       conteudo = DER(ModuloBoletimUrna::EntidadeBoletimUrna) [or its CEPESC ciphertext when criptografarBU] }
// Constructed (inline) by vota::CGravaResultado::StartState (func 12098, unit u07), which also runs it.
#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "api/util/cdatetime.h"
#include "comum/dados/crdv.h"                                   // IRdv (CRdvVota)
#include "comum/dados/md/estadoaplicacao/cdadocorrespondencia.h"
#include "comum/gravadores/iresultado.h"
#include "comum/gravadores/md/centidadebu.h"

namespace comum {

// md types used by the BU (layouts from funcs 1556, 2853, 2856, 3817, 3824, 5863, 5864, 10273):
namespace md {
struct CIdentificacaoVotavel {        // 8 bytes (func 5864)
    TCandidatoID codigo;              // +0  candidate number, party number (legenda) or answer number (consulta)
    TPartidoID partido;               // +4  u16; 0 for consultas
};
struct CVotosVotavel {                // 24 bytes (funcs 2856 without identification, 5863 with it)
    CVoto::ETipo tipo;                // +0  1 legenda, 2 nominal, 3 branco, 4 nulo, 8 nulo cargo sem candidato
    TCandidatoID codigo;              // +4
    TQtdEleitor quantidade;           // +8  u16
    std::optional<CIdentificacaoVotavel> identificacao;   // +12 (flag +20)
};
struct CVotosCargo {                  // 16 bytes
    TCargoID codigo;                  // +0  uebyte
    uebyte ordemImpressao;            // +1  1-based position of the cargo in CCargos
    std::vector<CVotosVotavel> votos; // +4
};
struct CResultadoVotacao {            // 20 bytes (ctor func 3824)
    CCargo::ETipo tipoCargo;          // +0  0 majoritário, 1 proporcional, 2 consulta
    TQtdEleitor qtdComparecimento;    // +4
    std::vector<CVotosCargo> votosCargos;   // +8
};
struct CResultadoVotacaoPorEleicao {  // 20 bytes
    TEleicaoID idEleicao;             // +0
    TQtdEleitor qtdAptosSecao;        // +4  (qtdEleitoresAptos = secao + TTE in the ASN.1)
    TQtdEleitor qtdAptosTTE;          // +6
    std::vector<CResultadoVotacao> resultados;   // +8
};
}  // namespace md

using TVetorVotosVotaveis = std::vector<md::CVotosVotavel>;

struct SQtdHabilitacoes {             // 3 x u16 (CBaseType<uint16_t, 0, 9999>)  name inferred
    TQtdEleitor semBiometria, porBiometria, porBiografia;
};

class CGravadorBU : public IGravador {                         // 304 bytes, vtable @1554140
public:
    CGravadorBU(TMunicipioID municipio, TZonaID zona, TLocalID local, TSecaoID secao, char fase,
                const api::CDateTime& dhGeracao, const api::CDateTime& dhEmissao,
                const md::estadoaplicacao::CDadoCorrespondencia& correspondencia,
                const std::vector<std::string>& historicoCodigosCarga, const SQtdeAptos& qtdAptos,
                const std::string& arquivoChave, const IRdv& rdv, TQtdEleitor comparecimento, bool urnaBiometrica,
                const SQtdHabilitacoes& habilitacoes, const api::CDateTime& dhInicioAquisicao,
                const api::CDateTime& dhFimAquisicao, std::map<TCargoID, uebyte> ordemCargos);   // inlined in 12098
    ~CGravadorBU() override;                                                     // func 5857 (slot 0), 11628 (slot 1)

    void GravaResultado(api::CFile& arquivo) const override;                     // func 11629 (slot 7, srcloc :484/:485)

    static void AcrescentaVotoVotavel(TVetorVotosVotaveis& votos, md::CVoto::ETipo tipo, TCandidatoID codigo,
                                      TQtdEleitor quantidade, const md::CCargo& cargo);   // func 3820 (srcloc :221)

private:
    std::vector<md::CVotosCargo> MontaVotosCargos(const std::vector<md::CCargo>& cargos) const;   // func 3819 (name inferred)
    std::vector<md::CResultadoVotacaoPorEleicao> MontaResultados() const;                          // inlined in 11629 (name inferred)
    md::CEntidadeBU MontaEntidadeBU(const md::CCabecalhoEntidade& cabecalho, const md::CUrna& urna,
                                    const std::vector<md::CResultadoVotacaoPorEleicao>& resultados) const;   // inlined (:636)
    std::vector<uebyte> LeChavePublica() const;                                   // inlined (:542, :552, :560)
    std::string LeSerialMV() const;                                               // inlined (name inferred)

    api::CDateTime m_dhGeracao;                                       // +40
    api::CDateTime m_dhEmissao;                                       // +52
    EUrnaFase m_fase;                                                 // +64  CGravadorUtil::ConverteFase(fase)
    md::ETipoArquivo m_tipoArquivo = md::ETipoArquivo('1');           // +68  votacaoUE
    md::estadoaplicacao::CDadoCorrespondencia m_correspondencia;      // +72  (96 bytes; carga, município/zona/seção)
    std::vector<std::string> m_historicoCodigosCarga;                 // +168
    SQtdeAptos m_qtdAptos;                                            // +180 map<abrangência, {aptos seção, aptos TTE}>
    bool m_urnaBiometrica;                                            // +192 -> detalhamentoComparecimento present
    TQtdEleitor m_comparecimento;                                     // +194
    const IRdv* m_rdv;                                                // +196 CRdvVota
    std::string m_arquivoChave;                                       // +200 "bu.pk1"
    bool m_permiteCifrar = true;                                      // +212 (ANDed with parâmetro criptografarBU)
    std::optional<md::CDadosBUVota> m_dadosVota;                      // +216 (flag +256): abertura, encerramento,
                                                                      //        optional desligamento do voto impresso
    std::optional<md::CTipoApuracaoSA> m_motivoUtilizacaoSA;          // +260 (flag +268): only for SA urnas
    std::optional<md::CDadosBUSA> m_dadosSA;                          // +272 (flag +280): junta, turma, urna origem
    std::map<TCargoID, uebyte> m_ordemCargos;                         // +284
    SQtdHabilitacoes m_habilitacoes;                                  // +296
};

}  // namespace comum

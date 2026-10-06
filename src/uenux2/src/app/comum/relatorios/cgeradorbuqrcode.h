// uenux2/src/app/comum/relatorios/cgeradorbuqrcode.h  (path inferred from cgeradorbuqrcode.cpp srclocs)
// Reconstructed from vota_web_wasm.wasm (unit u25; the payload generator itself, wasm 5603-5617, was
// reconstructed by unit u04 in cgeradorbuqrcode.u04-fragment.cpp and is explained in docs/bu/qrcode.md).
//
// comum::CGeradorBUQRCode — builds the text payloads of the QR codes printed at the end of the BU
// ("QRBU:i:n VRQR:6.0 ... HASH:... [ASSI:...]"). Abstract base (vtable @1575920):
//   [0] ~CGeradorBUQRCode  wasm 2251   [1] deleting dtor icf_tiny_vf1@325   [2] PreencheCabecalho  pure
//   [3] DadosEleicao  pure
// comum::CGeradorBUQRCodeVota : CGeradorBUQRCode (vtable @1575984): [0] wasm 11244, [1] wasm 11243,
//   [2] wasm 11242 (fills ORIG/MUNI/AGRE/LOCA/APTO.../DTEM), [3] wasm 11241 (DadosEleicao, "" for VOTA).
// Layout:
//   +0   vptr
//   +4   CCabecalhoQRCode m_cabecalho               (33 std::string, 396 bytes; dtor = comum_f2885)
//   +400 const CRdvVota& m_rdv
//   +404 std::map<md::ETipoAbrangencia, SQtdeAptos> m_aptos   (root at +408; destroy = comum_f1405)
//   --- CGeradorBUQRCodeVota ---
//   +416 TQtdVoto m_comparecimento   +418 bool m_origemRED   +419 bool m_incluiEmissao
//   +420 api::CDateTime m_dataEmissao                                          (432 bytes)
#pragma once

#include <map>
#include <string>
#include <vector>

#include "api/util/cdatetime.h"
#include "comum/dados/celeitores.h"          // SQtdeAptos
#include "comum/dados/crdvvota.h"
#include "comum/md/cabrangencia.h"           // md::ETipoAbrangencia
#include "comum/relatorios/ccabecalhoqrcodebuilder.h"

namespace comum {

struct SQRCodesBU {                                    // name inferred (u04)
    std::vector<std::string> conteudos;                // +0
    std::string assinatura;                            // +12 (hex; empty when not signed)
};

class CGeradorBUQRCode {
public:
    virtual ~CGeradorBUQRCode();                                          // wasm 2251
    SQRCodesBU GeraQRCodes(std::size_t tamanhoMaximo) const;              // wasm 5604 (u04)

protected:
    CGeradorBUQRCode(const CCabecalhoQRCode& cabecalho, const CRdvVota& rdv,
                     std::map<md::ETipoAbrangencia, SQtdeAptos> aptos);   // inlined in 5603 (u04)

    virtual void PreencheCabecalho(CCabecalhoQRCode& cabecalho) const = 0;   // slot 2
    virtual std::string DadosEleicao(TEleicaoID eleicao) const = 0;          // slot 3

    const SQtdeAptos& GetQtdAptos(md::ETipoAbrangencia abrangencia) const;   // srcloc :80 (inlined in 3696)
    std::string AptosCargo() const;                                          // wasm 3696   name inferred
    std::string TotaisVotosCargo() const;                                    // wasm 5606 (u04)
    std::string RetornaBlocoAssinado(const std::string& hash) const;         // srcloc :405 (inlined in 5604)

    CCabecalhoQRCode m_cabecalho;                                            // +4
    const CRdvVota& m_rdv;                                                   // +400
    std::map<md::ETipoAbrangencia, SQtdeAptos> m_aptos;                      // +404
};

class CGeradorBUQRCodeVota : public CGeradorBUQRCode {
public:
    CGeradorBUQRCodeVota(const CCabecalhoQRCode& cabecalho, TQtdVoto comparecimento,
                         const api::CDateTime& dataEmissao, bool origemRED = false,
                         bool incluiEmissao = false);                        // wasm 5603 (u04)
    ~CGeradorBUQRCodeVota() override;                                        // wasm 11244 (+ deleting 11243)

protected:
    void PreencheCabecalho(CCabecalhoQRCode& cabecalho) const override;      // wasm 11242
    std::string DadosEleicao(TEleicaoID eleicao) const override;             // wasm 11241

private:
    TQtdVoto m_comparecimento;       // +416
    bool m_origemRED;                // +418
    bool m_incluiEmissao;            // +419
    api::CDateTime m_dataEmissao;    // +420
};

} // namespace comum

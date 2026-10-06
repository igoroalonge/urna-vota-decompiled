// uenux2/src/app/comum/gravadores/cgravadorwsq.cpp
// Reconstructed from vota_web_wasm.wasm (unit u23). CompactaWsq (func 5821, "globToFileList") and the
// directory helpers are in gravadores/u12-foreign-fragments.cpp (unit u12); both parts belong to this file.
//
// CGravadorWSQ (88 bytes, vtable @1557300): packs the fingerprint images (WSQ) captured during the day into
// wsqbio.jez (voters enabled by biometrics, tipo 0), wsqman.jez (not enabled / manual, tipo 1) and
// wsqmes.jez (mesários, tipo 2). Only created on biometric urnas (CGravaResultado).
//   IResultado (+0..+39)
//   +40 api::CDateTime m_dhGeracao   +52 EUrnaFase m_fase   +56 std::string m_arquivoChave = "wsq.pk1" (unused here)
//   +68 int m_tipo (0/1/2)           +72 std::string m_versao = "10.23.0.1 - DESENVOLVIMENTO"
//   +84 bool m_gravaMV               (u07 passes !EhFaseTreinamento(): the MV copies are skipped in training)
// Unlike the other writers it overrides the COPY slots and leaves Grava/GravaMV/GravaResultado empty
// (slots 4, 5, 7 = no-op ICF bodies 218/425): the .jez is produced directly in the result directories.
#include "comum/gravadores/cgravadorwsq.h"

#include <filesystem>

#include "comum/cpath.h"
#include "comum/gravadores/cgravadorutil.h"

namespace comum {

// wasm func 3796. The tools used the name of the inlined ValidaTipoBiometria (srcloc cgravadorwsq.cpp:73).
CGravadorWSQ::CGravadorWSQ(const api::CDateTime& dhGeracao, TMunicipioID municipio, TZonaID zona, TLocalID local,
                           TSecaoID secao, char fase, EExtensaoArquivoResultado extensao, ESavdArquivoUE arquivoSavd,
                           int tipo, bool gravaMV)
    : IGravador(municipio, zona, local, secao, fase, extensao, arquivoSavd),          // func 1396
      m_dhGeracao(dhGeracao),
      m_fase(CGravadorUtil::ConverteFase(fase)),                                        // func 2274
      m_arquivoChave("wsq.pk1"),
      m_tipo(tipo),
      m_versao("10.23.0.1 - DESENVOLVIMENTO"),                                          // @326597
      m_gravaMV(gravaMV)
{
    ValidaTipoBiometria();
}

void CGravadorWSQ::ValidaTipoBiometria() const
{
    if (m_tipo >= 3)
        throw CUeComumGravadoresError(8651, "Argumento tipo de biometria inválido.");     // :73
}

// wasm func 11579 (slot 2, IGravador::CopiaParaResultado): result dir of the MI
void CGravadorWSQ::CopiaParaResultado() const
{
    CompactaWsq(false, std::filesystem::path(CPath::GetPathResult(EFlashOrigem::INTERNA)) / GetNome());   // f1274(0)
}

// wasm func 6041 (name inferred), shared by slots 3 and 6.
void CGravadorWSQ::CompactaNaMV(bool externo) const
{
    if (!m_gravaMV)
        return;
    CompactaWsq(externo, std::filesystem::path(CPath::GetPathResult(EFlashOrigem::EXTERNA)) / GetNome());  // f1274(1)
}

// wasm func 11577 (slot 3, CopiaParaMV): images of the MI packed into the MV result dir
void CGravadorWSQ::CopiaParaMV() const { CompactaNaMV(false); }

// wasm func 11578 (slot 6, CopiaResultadoParaMV): images of the MV packed into the MV result dir
void CGravadorWSQ::CopiaResultadoParaMV() const { CompactaNaMV(true); }

// wasm func 11576 (slot 0) / 11575 (slot 1, deleting): destroy m_versao (+72), m_arquivoChave (+56), IResultado.
CGravadorWSQ::~CGravadorWSQ() = default;

}  // namespace comum

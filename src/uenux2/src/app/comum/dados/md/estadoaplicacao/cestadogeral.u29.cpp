// FRAGMENT of uenux2/src/app/comum/dados/md/estadoaplicacao/cestadogeral.cpp (+ .h) reconstructed by unit u29
// from vota_web_wasm.wasm. The class (the md side of eg.bin, ModuloEstadoGeralUrna::EstadoGeralUrna) is
// reconstructed by units u05/u20; only its destructor and three std::vector helpers belong to u29.
//
// Layout (180 bytes, from the converter's constructor call, unit u05, and the builder of votaInit):
//   +0   EEstadoUrna m_estado            +4   TPEID m_idPE
//   +8   CDadoLocal m_local              (+8 std::string uf, +20 CLocalidadeEleitoral {município, zona, seção},
//                                          +28 tipo de local de votação)
//   +32  CDadoCarga m_carga              (+32 turno, +36/+40 tipo de urna T1/T2, +44 modelo, +48 fase)
//   +52  CAjusteDataHora m_ajuste        (8 bytes)
//   +60  CDadoCorrespondencia m_correspondencia   (96 bytes, destructor func 857)
//   +156 std::string m_versao            +168 std::vector<uebyte> m_hashVersoesPacotes
#include "comum/dados/md/estadoaplicacao/cestadogeral.h"

namespace comum::md::estadoaplicacao {

// wasm func 2793: the implicit destructor, members in reverse order:
//   ~vector (+168, func 520), ~string (+156), ~CDadoCorrespondencia (+60, func 857), ~string uf (+8).
// Callers: ~CAppInfoBuilder (5644), CAppInfoBuilder::SalvaGeral (10243), the builder constructor's unwind path.
CEstadoGeral::~CEstadoGeral() = default;

}  // namespace comum::md::estadoaplicacao

// Library instantiations for std::vector<CDadoCorrespondencia> (96-byte elements) that the tools attributed to
// the mock files (they are used by CEstadoGeralGap, whose first member is the vector of cargas - the "histórico
// de cargas" printed on the BU):
//   wasm func 1698  std::vector<CDadoCorrespondencia>::~vector()                 (= __destroy_vector{this}())
//   wasm func 5532  std::vector<CDadoCorrespondencia>::__destroy_vector::operator()()   (~elements, free)
//   wasm func 5294  std::__exception_guard_exceptions<vector::__destroy_vector>::~__exception_guard_exceptions()
//                   (rolls back a half-built vector copy if __completed_ (+4) is false)

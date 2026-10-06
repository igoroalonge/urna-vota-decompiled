// ecourna-lib/ecourna/app/dados/parametrizacaourna/cparametrosurna.cpp (path inferred)
// FRAGMENT written by unit u02 (the ecourna data classes belong to u14).
//
// ecourna::app::dados::CParametrosUrna: urna parameters read from ModuloParametrizacaoUrna
// (the scenario "...-pu.dat" file). 400+ bytes: a 100-byte block of scalars, three
// std::vector<SRotulo> (20-byte items {int64 id; std::string texto;}) at +100/+112/+124, and
// ~20 std::string members up to +392.

namespace ecourna::app::dados {

// wasm func 3777: implicit copy constructor CParametrosUrna(const CParametrosUrna&)
//   memcpy of the first 100 bytes, then member-wise copy of the vectors and strings.
//   Also called by wasm 9029 (std::pair<const K, CParametrosUrna> constructor).
CParametrosUrna::CParametrosUrna(const CParametrosUrna&) = default;

// wasm func 2267: implicit destructor ~CParametrosUrna() (frees the ~20 strings from +380 down and
//   the three vectors). Also called by ~CConfiguracaoEleicao (wasm 5792) and by
//   IConversorASN<ParametrosUrna, CParametrosUrna>::Deconverte (wasm 9171).
CParametrosUrna::~CParametrosUrna() = default;

} // namespace ecourna::app::dados

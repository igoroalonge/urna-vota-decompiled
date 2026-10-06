// ecourna-lib/ecourna/app/dados/asn/midias/cconversorinformacaomidia.cpp   (srcloc-attested file of unit u14)
// FRAGMENT written by unit u11: two template instantiations emitted for this file. Merge into the u14
// reconstruction. Reconstructed from vota_web_wasm.wasm.
//
// Context (u14): CConversorInformacaoMidia's vtable slot 3 (func 9179, currently named
// "DeconverteTipoMidia" after an inlined helper) converts InformacaoMidia (infomidia-*.dat) and turns the
// optional `aplicativos SEQUENCE OF Aplicativo` into a std::vector<CAplicativo> with a lambda like:
//
//     std::vector<CAplicativo> aplicativos;
//     std::transform(lista.begin(), lista.end(), std::back_inserter(aplicativos),
//                    [&conversorAplicativo](const ModuloInformacaoMidia::Aplicativo& aplicativo) {
//                        return conversorAplicativo.Deconverte(aplicativo);
//                    });
//
// wasm func 9176  std::transform<SEQUENCE_OF<Aplicativo>::const_iterator,
//                 std::back_insert_iterator<std::vector<CAplicativo>>, $lambda>
//                 The lambda = IConversorASN<Aplicativo, CAplicativo>::Deconverte, inlined (srcloc
//                 iconversorasn.hpp:66, record @1127392): validity check, then the converter's slot 3
//                 (func 9196 CConversorAplicativo). The tool therefore named 9176 "IConversorASN<Aplicativo,
//                 CAplicativo>::Deconverte". push_back fast path inlined (72-byte CAplicativo moved:
//                 +0 tipo, +8 std::optional<CAutenticacao> with flag +64).
// wasm func 9174  std::vector<CAplicativo>::__push_back_slow_path(CAplicativo&&)
//                 (growth max(2*cap, size+1), max_size 59 652 323, elements relocated one by one and the
//                 moved-from optional<CAutenticacao> hash vectors freed)

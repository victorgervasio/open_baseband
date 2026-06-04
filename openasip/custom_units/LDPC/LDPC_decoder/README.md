To fix linter warnings/errors:

1- Fixed include (relative references)
2- let b:ale_c_cc_options = '-D_GNU_SOURCE'
    -> ran it in nrLDPC_decoder.c
    -> This allowed ALE linter to find 'uint' definition
3- let g:ale_c_cc_options = '-D_GNU_SOURCE -mavx'
    -> ran it in nrLDPC_decoder.c
    -> This allowed ALE linter to find avx operations definitions

model diet.mod;
data diet.dat;
option solver mosek;
option mosek_options
'msk_ipar_optimizer = msk_optimizer_primal_simplex';
solve;
display Buy.sstatus;
solve;

model diet.mod;
data diet.dat;
option solver mosek;
option mosek_options 
'msk_ipar_optimizer=msk_optimizer_primal_simplex \
 msk_ipar_sim_max_iterations=100000';
solve;
quit;


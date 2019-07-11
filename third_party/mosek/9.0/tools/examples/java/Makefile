
#
# Compile and run MOSEK examples
#
examples.jar: blas_lapack.java callback.java portfolio_1_basic.java portfolio_2_frontier.java portfolio_3_impact.java portfolio_4_transcost.java portfolio_5_card.java cqo1.java ceo1.java pow1.java concurrent.java feasrepairex1.java lo1.java lo2.java logistic.java gp1.java milo1.java mioinitsol.java mico1.java modelLib.java opt_server_async.java opt_server_sync.java parallel.java parameters.java reoptimization.java qcqo1.java qo1.java response.java sdo1.java sensitivity.java simple.java solutionquality.java solvebasis.java solvelinear.java
	javac -d . -classpath ../../platform/linux64x86/bin/mosek.jar blas_lapack.java callback.java portfolio_1_basic.java portfolio_2_frontier.java portfolio_3_impact.java portfolio_4_transcost.java portfolio_5_card.java cqo1.java ceo1.java pow1.java concurrent.java feasrepairex1.java lo1.java lo2.java logistic.java gp1.java milo1.java mioinitsol.java mico1.java modelLib.java opt_server_async.java opt_server_sync.java parallel.java parameters.java reoptimization.java qcqo1.java qo1.java response.java sdo1.java sensitivity.java simple.java solutionquality.java solvebasis.java solvelinear.java
	jar cf examples.jar com

clean:
	rm -rf com/

test: examples.jar
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.blas_lapack
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.callback
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.portfolio_1_basic
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.portfolio_2_frontier
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.portfolio_3_impact
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.portfolio_4_transcost
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.portfolio_5_card
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.cqo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.ceo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.pow1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.concurrent
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.feasrepairex1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.lo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.lo2
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.logistic
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.gp1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.milo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.mioinitsol
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.mico1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.modelLib
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.opt_server_async
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.opt_server_sync
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.parallel
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.parameters
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.reoptimization
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.qcqo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.qo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.response
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.sdo1
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.sensitivity
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.simple
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.solutionquality
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.solvebasis
	java -cp examples.jar:../../platform/linux64x86/bin/mosek.jar com.mosek.example.solvelinear


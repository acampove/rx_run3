from .minimizers.baseminimizer import BaseMinimizer as BaseMinimizer, DefaultStrategy as DefaultStrategy, minimize_supports as minimize_supports
from .minimizers.ipopt import Ipyopt as Ipyopt
from .minimizers.minimizer_lm import LevenbergMarquardt as LevenbergMarquardt
from .minimizers.minimizer_minuit import Minuit as Minuit
from .minimizers.minimizer_nlopt import NLoptBOBYQA as NLoptBOBYQA, NLoptBaseMinimizer as NLoptBaseMinimizer, NLoptCCSAQ as NLoptCCSAQ, NLoptCOBYLA as NLoptCOBYLA, NLoptESCH as NLoptESCH, NLoptISRES as NLoptISRES, NLoptLBFGS as NLoptLBFGS, NLoptMLSL as NLoptMLSL, NLoptMMA as NLoptMMA, NLoptSLSQP as NLoptSLSQP, NLoptShiftVar as NLoptShiftVar, NLoptStoGO as NLoptStoGO, NLoptSubplex as NLoptSubplex, NLoptTruncNewton as NLoptTruncNewton
from .minimizers.minimizers_scipy import ScipyBFGS as ScipyBFGS, ScipyBaseMinimizer as ScipyBaseMinimizer, ScipyCOBYLA as ScipyCOBYLA, ScipyDogleg as ScipyDogleg, ScipyLBFGSB as ScipyLBFGSB, ScipyNelderMead as ScipyNelderMead, ScipyNewtonCG as ScipyNewtonCG, ScipyPowell as ScipyPowell, ScipySLSQP as ScipySLSQP, ScipyTruncNC as ScipyTruncNC, ScipyTrustConstr as ScipyTrustConstr, ScipyTrustKrylov as ScipyTrustKrylov, ScipyTrustNCG as ScipyTrustNCG
from .minimizers.optimizers_tf import Adam as Adam, WrapOptimizer as WrapOptimizer
from .minimizers.strategy import DefaultToyStrategy as DefaultToyStrategy, PushbackStrategy as PushbackStrategy
from .minimizers.termination import EDM as EDM

__all__ = ['EDM', 'Adam', 'BaseMinimizer', 'DefaultStrategy', 'DefaultToyStrategy', 'Ipyopt', 'LevenbergMarquardt', 'LevenbergMarquardt', 'Minuit', 'NLoptBOBYQA', 'NLoptBOBYQAV1', 'NLoptBaseMinimizer', 'NLoptBaseMinimizerV1', 'NLoptCCSAQ', 'NLoptCCSAQV1', 'NLoptCOBYLA', 'NLoptCOBYLAV1', 'NLoptESCH', 'NLoptESCHV1', 'NLoptISRES', 'NLoptISRESV1', 'NLoptLBFGS', 'NLoptLBFGSV1', 'NLoptMLSL', 'NLoptMLSLV1', 'NLoptMMA', 'NLoptMMAV1', 'NLoptSLSQP', 'NLoptSLSQPV1', 'NLoptShiftVar', 'NLoptShiftVarV1', 'NLoptStoGO', 'NLoptStoGOV1', 'NLoptSubplex', 'NLoptSubplexV1', 'NLoptTruncNewton', 'NLoptTruncNewtonV1', 'PushbackStrategy', 'ScipyBFGS', 'ScipyBaseMinimizer', 'ScipyBaseMinimizerV1', 'ScipyCOBYLA', 'ScipyDogleg', 'ScipyLBFGSB', 'ScipyLBFGSBV1', 'ScipyNelderMead', 'ScipyNelderMeadV1', 'ScipyNewtonCG', 'ScipyNewtonCGV1', 'ScipyPowell', 'ScipyPowellV1', 'ScipySLSQP', 'ScipySLSQPV1', 'ScipyTruncNC', 'ScipyTruncNCV1', 'ScipyTrustConstr', 'ScipyTrustConstrV1', 'ScipyTrustKrylov', 'ScipyTrustNCG', 'WrapOptimizer', 'minimize_supports']

ScipyTrustConstrV1 = ScipyTrustConstr
ScipyTrustNCGV1 = ScipyTrustNCG
ScipyTrustKrylovV1 = ScipyTrustKrylov
ScipyDoglegV1 = ScipyDogleg
ScipyCOBYLAV1 = ScipyCOBYLA
ScipyLBFGSBV1 = ScipyLBFGSB
ScipyPowellV1 = ScipyPowell
ScipySLSQPV1 = ScipySLSQP
ScipyNewtonCGV1 = ScipyNewtonCG
ScipyTruncNCV1 = ScipyTruncNC
ScipyNelderMeadV1 = ScipyNelderMead
NLoptLBFGSV1 = NLoptLBFGS
NLoptTruncNewtonV1 = NLoptTruncNewton
NLoptSLSQPV1 = NLoptSLSQP
NLoptMMAV1 = NLoptMMA
NLoptCCSAQV1 = NLoptCCSAQ
NLoptShiftVarV1 = NLoptShiftVar
NLoptMLSLV1 = NLoptMLSL
NLoptStoGOV1 = NLoptStoGO
NLoptESCHV1 = NLoptESCH
NLoptISRESV1 = NLoptISRES
NLoptSubplexV1 = NLoptSubplex
NLoptBOBYQAV1 = NLoptBOBYQA
NLoptCOBYLAV1 = NLoptCOBYLA
IpyoptV1 = Ipyopt
ScipyBaseMinimizerV1 = ScipyBaseMinimizer
NLoptBaseMinimizerV1 = NLoptBaseMinimizer
BaseMinimizerV1 = BaseMinimizer

'''
Module holding CXCalculator
'''
import random

from typing        import Final
from dmu           import LogStore
from rx_common     import Project, Trigger, Sample
from rx_common     import Channel, Qsq
from uncertainties import Variable, ufloat

from rx_efficiencies.efficiency_calculator import EfficiencyCalculator

log=LogStore.add_logger('rx_efficiencies:cxcalculator')
# ----------------------
class CXCalculator:
    '''
    Class meant to calculate double ratio of efficiencies
    '''
    # ----------------------
    def __init__(
        self, 
        project : Project,
        qsq     : Qsq) -> None:
        '''
        Parameters
        -------------
        project: rk or rkst
        qsq    : q2 bin, e.g. low, central, high
        '''
        self._project = project
        self._qsq     = qsq

        random.seed(111)
        self._scale : Final[float] = random.uniform(0.5, 2.0)
    # ----------------------
    def _trigger_from_channel(self, channel : Channel) -> Trigger:
        '''
        Parameters
        -------------
        channel: E.g. ee or mm

        Returns
        -------------
        Trigger associated to channel, given project already passed in initializer
        '''
        if self._project == Project.rk   and channel == Channel.ee:
            return Trigger.rk_ee_os

        if self._project == Project.rk   and channel == Channel.mm:
            return Trigger.rk_mm_os

        if self._project == Project.rkst and channel == Channel.ee:
            return Trigger.rkst_ee_os

        if self._project == Project.rkst and channel == Channel.mm:
            return Trigger.rkst_mm_os

        raise ValueError(f'Cannot pick trigger for channel/project: {channel}/{self._project}')
    # ----------------------
    def _get_sample(
        self, 
        q2bin   : Qsq, 
        channel : Channel) -> Sample:
        '''
        Parameters
        -------------
        q2bin:  E.g. central
        channel: E.g. ee

        Returns
        -------------
        MC sample associated to combinatoion of chnnel project and q2bin (resonant sample for resonant bin)
        '''
        if self._project == Project.rk:
            if q2bin == Qsq.jpsi and channel == Channel.ee:
                return Sample.bpkpjpsiee

            if q2bin == Qsq.jpsi and channel == Channel.mm:
                return Sample.bpkpjpsimm

            if q2bin == Qsq.psi2 and channel == Channel.ee:
                return Sample.bpkppsi2ee

            if q2bin == Qsq.psi2 and channel == Channel.mm:
                return Sample.bpkppsi2mm

            if channel == Channel.ee:
                return Sample.bpkpee

            if channel == Channel.mm:
                return Sample.bpkpmm

        if self._project == Project.rkst:
            if q2bin == Qsq.jpsi and channel == Channel.ee:
                return Sample.bdkstkpijpsiee

            if q2bin == Qsq.jpsi and channel == Channel.mm:
                return Sample.bdkstkpijpsimm

            if q2bin == Qsq.psi2 and channel == Channel.ee:
                return Sample.bdkstkpipsi2ee

            if q2bin == Qsq.psi2 and channel == Channel.mm:
                return Sample.bdkstkpipsi2mm

            if channel == Channel.ee:
                return Sample.bdkstkpiee

            if channel == Channel.mm:
                return Sample.bdkstkpimm

        raise ValueError(f'Invalid q2bin/channel: {q2bin}/{channel}')
    # ----------------------
    def _get_efficiency(self, channel : Channel, qsq : Qsq) -> Variable:
        '''
        Parameters
        -------------
        channel: Either ee or mm
        qsq    : q2 bin, e.g. central

        Returns
        -------------
        Random variable symbolizing efficiency
        '''
        trigger = self._trigger_from_channel(channel = channel)
        sample  = self._get_sample(q2bin = qsq, channel = channel)

        ecal    = EfficiencyCalculator(q2bin = qsq, trigger = trigger, sample = sample)
        val,err = ecal.get_efficiency()

        return ufloat(val, err)
    # ----------------------
    def calculate(self) -> tuple[float,float]:
        '''
        Returns
        -------------
        Tuple with value of cx and its error
        '''
        eff_ee_rare = self._get_efficiency(channel = Channel.ee, qsq = self._qsq)
        eff_mm_rare = self._get_efficiency(channel = Channel.mm, qsq = self._qsq)

        rat = eff_ee_rare / eff_mm_rare # type: ignore
        val = rat.nominal_value * self._scale 
        err = rat.std_dev

        return val, err 
# ----------------------

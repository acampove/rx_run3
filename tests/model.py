from q2_syst.model import model as q2model

from logzero import logger as log

#-------------------------------
def get_sim_par():
    d_sim_par = {}
    d_sim_par['mu_1'] = [100, 0]
    d_sim_par['mu_2'] = [100, 0]
    d_sim_par['mu_3'] = [100, 0]

    d_sim_par['sg_1'] = [10, 0]
    d_sim_par['sg_2'] = [10, 0]
    d_sim_par['sg_3'] = [10, 0]

    return d_sim_par
#-------------------------------
def test_1():
    obj=q2model()
    pdf=obj.get_pdf(is_signal=False, split_by_nspd=False)

    log.info(f'Passed test_1')
#-------------------------------
def test_2():
    obj=q2model()
    pdf=obj.get_pdf(is_signal=True, split_by_nspd=False)

    log.info(f'Passed test_2')
#-------------------------------
def test_3():
    d_sim_par = get_sim_par()

    obj=q2model(d_sim_par=d_sim_par)
    pdf=obj.get_pdf(is_signal=False, split_by_nspd=True)

    log.info(f'Passed test_3')
#-------------------------------
if __name__ == '__main__':
    test_1()
    test_2()
    test_3()


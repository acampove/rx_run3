'''
Module holding cv_classifier class
'''

from sklearn.ensemble        import GradientBoostingClassifier

# ---------------------------------------
class CVClassifier(GradientBoostingClassifier):
    # pylint: disable = too-many-ancestors, abstract-method
    '''
    Derived class meant to implement features needed for cross-validation 
    '''
    def __init__(self, **kwargs):
        super().__init__(**kwargs)
# ---------------------------------------

'''
Module with utility functions for reading and writting files
'''

# -----------------------
def _format(line : str) -> str:
    line_lstr = line.lstrip()
    if line_lstr.startswith('-'):
        ind=line.index('-')
        return line[:ind-1] + '   -' + line[ind+1:]

    return line
# -----------------------
def reformat_yaml(path : str) -> None:
    '''
    Picks up path to yaml file and improves formatting
    Needed because PyYaml does not indent lists
    '''

    with open(path, encoding='utf-8') as ifile:
        l_line = ifile.read().splitlines()

    l_line = [ _format(line) for line in l_line ]

    text = '\n'.join(l_line)
    with open(path, 'w', encoding='utf-8') as ofile:
        ofile.write(text)
# -----------------------

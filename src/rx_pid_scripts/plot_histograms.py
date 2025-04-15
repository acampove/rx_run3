'''
Script used to plot histograms made by PIDCalib2
'''
import os
import pickle
import argparse

import numpy             as np
import matplotlib
import matplotlib.pyplot as plt

matplotlib.use('Agg')

#fancy = True
fancy = False
if fancy:
    figsize= (12, 12)
    fontsize = 20
    #plt.style.use('/disk/lhcb_data/matzeni/AmplitudeFit-Bd2Kstll-DeltaWilsonCoeff/AmplitudeFit/work/scripts/matplotlib_LHCb.mplstyle')
else:
    figsize = (8, 8)
    fontsize=15

def _plot_boosthistogram_1d(hist, label=None):
    # Extract bin edges and values for plotting
    bin_edges = hist.axes[0].edges
    bin_values = hist.view()

    values = bin_values['value']
    variances = bin_values['variance']
    errors = np.sqrt(variances)

    c = (bin_edges[:-1] + bin_edges[1:])/2
    w = (-bin_edges[:-1] + bin_edges[1:])/2

    # Plot the histogram
    plt.errorbar(c, values, xerr=w, yerr=errors, capsize=4, fmt='o', label=label)

def _plot_fit_histogram_1d(hist, label=None):
    # Extract bin edges and values for plotting
    values = hist['efficiency_value']
    errors = hist['efficiency_error']

    c = hist['bin_centers']
    w = hist['bin_widths']

    # Plot the histogram
    plt.errorbar(c, values, xerr=w, yerr=errors, capsize=4, fmt='o', label=label)

def plot_boosthistogram_1d(fn, figure_dict=None):

    if figure_dict is None:
        figure_dict = {"xlabel": "Value", 'ylabel': "Counts", "legend": False,
                       "outputname":"test", "outputfold":'./plots', "label":None}

    with open(fn, 'rb') as file:
        hist = pickle.load(file)

    plt.close('all')
    _plot_boosthistogram_1d(hist, label=figure_dict['label'])

    plt.xlabel(figure_dict['xlabel'])
    plt.ylabel(figure_dict['ylabel'])
    plt.ylim(0.2, 1.1)
    outputfold = figure_dict['outputfold']
    os.system(f"mkdir -p {outputfold}")
    print(f"Saving figure in {outputfold}")
    plt.savefig(f"{outputfold}/{figure_dict['outputname']}")

def plot_list_of_boosthistogram_1d(fn_list, figure_dict=None, label_list=None):
    if label_list is None:
        label_list = [None for _ in fn_list]

    if figure_dict is None:
        figure_dict = {"xlabel": "Value", 'ylabel': "Counts", "legend": False, "title":None, "outputname":"test", "outputfold":'./plots', "label":None}
    plt.close('all')
    for fn, label in zip(fn_list, label_list):
        with open(fn, 'rb') as file:
            hist = pickle.load(file)

        _plot_boosthistogram_1d(hist, label=label)
    plt.title(figure_dict['title'])
    plt.xlabel(figure_dict['xlabel'])
    plt.ylabel(figure_dict['ylabel'])
    if figure_dict['legend']:
        plt.legend(frameon=False)
    outputfold = figure_dict['outputfold']
    os.system(f"mkdir -p {outputfold}")
    print(f"Saving figure in {outputfold}")
    plt.savefig(f"{outputfold}/{figure_dict['outputname']}")

def plot_boosthistogram_2d(fn, figure_dict=None, size=2):

    if figure_dict is None:
        figure_dict = {"xlabel": "Value", 'ylabel': "Counts", "legend": False,"title":None,
                       "outputname":"test", "outputfold":'./plots', "label":None}

    with open(fn, 'rb') as file:
        hist = pickle.load(file)
    plt.close('all')
    x_edges = hist.axes[0].edges
    y_edges = hist.axes[1].edges
    bin_values = hist.view()

    counts  = bin_values['value']
    variances = bin_values['variance']
    errors = np.sqrt(variances)

    # Step 4: Plot using matplotlib's pcolormesh
    X, Y = np.meshgrid(x_edges, y_edges)
    plt.pcolormesh(X, Y, counts.T, shading='auto', vmin=0.2, vmax=1.)  # Transpose counts to match the axes
    plt.colorbar(label='Efficiency')

    plt.xlabel(figure_dict['xlabel'])
    plt.title(figure_dict['title'])
    plt.ylabel(figure_dict['ylabel'])
    outputfold = figure_dict['outputfold']
    os.system(f"mkdir -p {outputfold}")

    for i in range(len(x_edges)-1):
        for j in range(len(y_edges)-1):
            x_center = (x_edges[i] + x_edges[i+1]) / 2
            y_center = (y_edges[j] + y_edges[j+1]) / 2
            plt.text(x_center, y_center, f'{counts[i][j]:.2f}\n$\pm$ {errors[i][j]:.2f}', ha='center', va='center', color='white', fontdict={'size': size})
            plt.text(x_center, y_center*0.9, f'{i}-{j}', ha='center', va='center', color='orange', fontdict={'size': size})

    print(f"Saving figure in {outputfold}")
    plt.savefig(f"{outputfold}/{figure_dict['outputname']}")

def get_fancy_xlabel(var):
    fancy_dict = {"P":r"$p(\mu)$ [MeV/c]",
                  "muprobe_P":r"$p(\mu)$ [MeV/c]",
                  "PT":r"$p_T(\mu)$ [MeV/c]",
                  "ETA":r"$\eta(\mu)$",
                  "PHI":r"$\phi(\mu)$",
                  "nPVs":r"nPVs",
                  "nFTClusters":r"nFTClusters",
                  }
    return fancy_dict[var]


def _parse_args():
    parser = argparse.ArgumentParser(description="A simple program that demonstrates argparse usage")
    parser.add_argument('--pid-cut', required=False, help='Path to the input file', dest="pid_cut", action='append')
    parser.add_argument('--output-dir', required=False, help='Path to the input file', dest="input_dir")
    parser.add_argument('--bin-var', required=False, help='Path to the input file', dest="bin_var", action='append')
    parser.add_argument('--sample', required=False, help='Path to the input file', dest="year")
    parser.add_argument('--samples-file', required=False, help='Path to the input file', dest="samples_file")
    parser.add_argument('--magnet', required=False, help='Path to the input file', dest="magnet")
    parser.add_argument('--particle', required=False, help='Path to the input file', dest="particle")
    parser.add_argument('--binning-file', required=False, help='Path to the input file')

    args         = parser.parse_args()
    pid_cut_list = args.pid_cut
    input_dir    = args.input_dir
    bin_var      = args.bin_var
    year         = args.year
    samples_file = args.samples_file
    particle     = args.particle
    magnet = args.magnet
    binning_file = args.binning_file

# ------------------------------------
def main():
    '''
    start here
    '''
    output_dir = os.path.dirname(samples_file)

    if len(bin_var)<2:
        bin_var = bin_var[0]
        for pid_cut in pid_cut_list:
            pid_cut = pid_cut.replace(" ","")
            fn = f'{input_dir}/effhists-{year}-{magnet}-{particle}-{pid_cut}-{bin_var}.pkl'
            figure_dict = {"xlabel": get_fancy_xlabel(bin_var), 'ylabel': "Efficiency", "legend": False,
                        "outputname":f"effhists-{year}-{magnet}-{particle}-{pid_cut}-{bin_var}.pdf",
                        "outputfold":f'{output_dir}/std/{particle}/', "label":None,"title": pid_cut}
            plot_boosthistogram_1d(fn, figure_dict=figure_dict)
    elif len(bin_var)==2:
        joined_bin_var = ".".join(bin_var)
        for pid_cut in pid_cut_list:
            pid_cut = pid_cut.replace(" ","")
            fn = f'{input_dir}/effhists-{year}-{magnet}-{particle}-{pid_cut}-{joined_bin_var}.pkl'
            figure_dict = {"xlabel": get_fancy_xlabel(bin_var[0]),
                            'ylabel': get_fancy_xlabel(bin_var[1]),
                            "legend": False,
                            "title": pid_cut,
                        "outputname":f"effhists-{year}-{magnet}-{particle}-{pid_cut}-{joined_bin_var}.pdf",
                        "outputfold":f'{output_dir}/std/{particle}/', "label":None}
            plot_boosthistogram_2d(fn, figure_dict=figure_dict)
# ------------------------------------
if __name__ == "__main__":
    main()

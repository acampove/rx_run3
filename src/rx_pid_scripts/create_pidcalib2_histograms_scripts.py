import os, sys
import yaml, json
import glob
import uproot as ur
import numpy as np

# paths, to be centralized
pidcalib2_path='/afs/cern.ch/work/m/matzeni/pidcalib2/src/'
eos_dir='/eos/lhcb/grid/prod/lhcb/'
pwd='/afs/cern.ch/work/m/matzeni/ewp-rjpsi-2024/corrections/pid/'

def create_new_root_with_ones(input_file, output_file, branch_name="ones", treename=None):
    # Open the existing ROOT file
    with ur.open(input_file) as file:
        # Get the first tree in the file (adjust if necessary)
        if isinstance(treename, list):
            tree_list = [file[_treename] for _treename in treename]
        elif isinstance(treename, str):
            tree_list = [file[treename]]
        else:
            tree_list = [file[file.keys()[0]]]
            
        ones_list = []
        for tree in tree_list:
            # Get the number of entries in the tree
            num_entries = tree.num_entries
            
            # Create a new branch filled with ones
            ones_list.append(np.ones(num_entries, dtype=np.float32))  # Change dtype if needed
            
        # Write the new ROOT file
        print(f"Saving into {output_file}")
        with ur.recreate(output_file) as new_file:
            for _treename, ones_branch in zip(treename, ones_list):
                new_file[_treename] = {branch_name: ones_branch}
    

def produce_fake_sweight_tuples(updated_location, fn_list, treename):
    outputfold = f'{updated_location}/sweights/'
    os.system(f"mkdir -p {outputfold}")
    for i,ifn in enumerate(fn_list):
        #/eos/lhcb//grid/prod/lhcb//anaprod/lhcb/MC/2024/MC.ROOT/00235871/0000/00235871_00000001_1.mc.root
        #00235871_00000001_1.mc_sweights.root
        if os.path.isfile(f"{outputfold}/{os.path.basename(ifn).replace(".root","_sweights.root")}"):
            print("File {} already exist, explicitely remove to reproduce it!".format(f"{outputfold}/{os.path.basename(ifn).replace(".root","_sweights.root")}"))
        else:
            create_new_root_with_ones(ifn, f"{outputfold}/{os.path.basename(ifn).replace(".root","_sweights.root")}", branch_name="sweight", treename=treename)
    return outputfold

def get_ttree_for_particle(particle):
    particle_dict = {'K':["Hlt2PID_DstToD0Pi_D0ToKPi/DecayTree"],
                     'Pi':["Hlt2PID_DstToD0Pi_D0ToKPi/DecayTree"],
                     'Mu': ["Hlt2PID_JpsiToMuMupTagged_Detached/DecayTree", "Hlt2PID_JpsiToMuMumTagged_Detached/DecayTree"],
                     'Mum': ["Hlt2PID_JpsiToMuMupTagged_Detached/DecayTree"],
                     'Mup': ["Hlt2PID_JpsiToMuMumTagged_Detached/DecayTree"],
                     }
    return particle_dict[particle]

def get_prefix_for_particle(particle):
    particle_dict = {'K':"K",
                     'Pi':"pi", 
                     'Mu':"muprobe",
                     'Mum':"muprobe",'Mup':"muprobe"}
    return particle_dict[particle]

def get_mc_dictionary_for_pidcalib2(particle, year, mag, sample, fn_list, sel_string, sweight_dir, particle_prefix):
    mc_dict_to_save = {
    f"{sample}-{"MagUp" if mag == 'up' else "MagDown"}-{particle}": {
        "files": fn_list,
        "cuts": [
                sel_string
        ],
        "sweight_dir": sweight_dir,
        "tuple_names": {
            particle: [itree.replace("/DecayTree","") for itree in get_ttree_for_particle(particle)]
        },
        "probe_prefix": particle_prefix
    }
    }
    return mc_dict_to_save
 
    
def write_configuration(result, outputfile, format='yaml'):
    if format == 'yaml':
        import yaml
        with open(outputfile, 'w') as file:
            yaml.dump(result, file, default_flow_style=False)
    elif format == 'json':
        import json
        with open(outputfile, 'w') as file:
            json.dump(result, file, indent=4)
            
def load_configuration(outputfile, format='yaml'):
    if format == 'yaml':
        import yaml
        with open(outputfile, 'r') as file:
            #yaml.dump(result, file, default_flow_style=False)
            return yaml.load(file, Loader=yaml.Loader)
            #return yaml.safe_load(file) #, Loader=yaml.Loader)
    elif format == 'json':
        import json
        with open(outputfile, 'r') as file:
            #json.dump(result, file, indent=4)
            return json.load(file)


def get_pidcalib2_file_location(data_type, channel, year, mag, block):
    if data_type == 'Data':
        pass 
    elif data_type == "MC":
        sample_dict ={  
                        "JpsiToMuMu": {2024: {
                                        "down": {
                                                #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w37_39_magdown_sim10d_24142001_incl_jpsi_mm_eq_dpc_tuple&ver=v1r2266
                                                "block7": f'{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263355/0000/00263355_0*_1.tuple.root',
                                                },
                                        "up": {
                                                #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w31_34_magup_sim10d_24142001_incl_jpsi_mm_eq_dpc_tuple&ver=v1r2266
                                                "block1_v0":f"{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263353/0000/00263353_0*_1.tuple.root",
                                                "block8":f"{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263353/0000/00263353_0*_1.tuple.root",
                                                }
                                            }
                                       },
                        # for the D0 I am using the same sample, because I could not find anything better
                        "DstToD0Pi_D0ToKPi": {2024: {
                                                    "down": {
                                                            #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w31_34_magup_sim10d_27163003_dst_d0pi_kpi_eq_dpc_tuple&ver=v1r2266
                                                            "block7": f'{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263357/0000/00263357_*_1.tuple.root',
                                                            },
                                                    "up": {
                                                            #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w31_34_magup_sim10d_27163003_dst_d0pi_kpi_eq_dpc_tuple&ver=v1r2266
                                                            "block1_v1": f'{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263357/0000/00263357_*_1.tuple.root',
                                                            #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w31_34_magup_sim10d_27163003_dst_d0pi_kpi_eq_dpc_tuple&ver=v1r2266
                                                            "block8": f'{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00263357/0000/00263357_*_1.tuple.root',
                                                            }
                                                    },
                                              },
        }
        '''
        sample_dict =  {  
                            # these files are taken from the cached tuples of pidcalib-sweight
                            # https://lhcb-analysis-productions.web.cern.ch/productions/?wg=rd&analysis=rd_ap_2024&dset=mc_24_w31_34_hlt1bug_magup_sim10d_24142001_incl_jpsi_mm_eq_dpc_tuple&ver=v1r2041                        
                            "JpsiToMuMu": {2024: {"up": f"{eos_dir}/anaprod/lhcb/MC/2024/TUPLE.ROOT/00256468/0000/00256468_0*_1.tuple.root"}},
                            #https://lhcb-analysis-productions.web.cern.ch/productions/?wg=pid&analysis=mc_tupling&dset=magup_mc_dstptod0pip_d0tokmpip_hlt1%2C2%2Ctuple&ver=v1r1586
                            "DstToD0Pi_D0ToKPi": {2024: {"up":f'{eos_dir}/anaprod/lhcb/MC/2024/MC.ROOT/00235871/0000/00235871_0*_1.mc.root'}},
                }
        '''
    return glob.glob(sample_dict[channel][year][mag][block])

def get_standard_dict_for_data():
    std_json_file = pidcalib2_path+"/pidcalib2/data/samples.json"
    std_data_dict = load_configuration(std_json_file, format='json')
    return std_data_dict

def get_mc_selection(data_type, channel, year, mag):
    #This cuts are at the moment hard coded, might want a bit more freedom here
    mc_sel_dict = {"MC": {"JpsiToMuMu": "(mutag_MUONLLBG <0) & (muprobe_INMUON==1) & ((muprobe_Hlt1TrackMVADecision_TIS==1) | (muprobe_Hlt1TwoTrackMVADecision_TIS==1)) & ((Jpsi_M>2930) & (Jpsi_M<3270)) & ((Jpsi_TRUEID==443)|(Jpsi_TRUEID==-443)) & ((muprobe_TRUEID==13)|(muprobe_TRUEID==-13)) & ((mutag_TRUEID==13)|(mutag_TRUEID==-13)) & ((mutag_MC_MOTHER_ID==443)|(mutag_MC_MOTHER_ID==-443)) & ((muprobe_MC_MOTHER_ID==443)|(muprobe_MC_MOTHER_ID==-443))",
                          "DstToD0Pi_D0ToKPi": "(Dst_BKGCAT==0) & (D_M > 1825) & (D_M < 1910) & ((Dst_M - D_M) > 141) & ((Dst_M - D_M) < 152)", #TODO: Need to find a way to include this selection for MC as well & ((D_WM_KK_manual < (1864.84 - 25.0)) | (D_WM_KK_manual > (1864.84 + 25.0))) & ((D_WM_PIK_manual < (1864.84 - 25.0)) | (D_WM_PIK_manual > (1864.84 + 25.0)))',",
                          }}
    return mc_sel_dict[data_type][channel]

def check_if_configuration_is_acceptable(data_type, year, mag, sample, acceptable_list):
    acceptable = False
    for iacc_dict in acceptable_list:
        if (('polarities' in iacc_dict.keys()) and ('samples' in iacc_dict.keys())):
            if ((mag in iacc_dict['polarities']) and (sample in iacc_dict['samples'])):
                acceptable = True
    return acceptable

if __name__ == "__main__":
  # load the data of interest
    import argparse
    parser = argparse.ArgumentParser(description="A simple program that demonstrates argparse usage")
    parser.add_argument('--yaml', required=False, help='Path to the input file', default=None)
    parser.add_argument('--user', required=False, help='Path to the input file', default="matzeni")
    # Add the dry-run flag
    parser.add_argument('--dry-run', action='store_true', help='Enable dry-run mode (default: False)')    
    parser.add_argument('--binning-file', required=False, help='Path to the input file')
    parser.add_argument('--save-in-root', action='store_true', help='Save also in root format')    
    parser.add_argument('--plot', action='store_true', help='Save also in root format')    
    parser.add_argument('--plot_only', action='store_true', help='Save also in root format')    
    
    args = parser.parse_args()
    yaml_file = args.yaml
    user = args.user
    dry_run = args.dry_run
    save_in_root = args.save_in_root
    plot = args.plot
    plot_only = args.plot_only
    binning_file = args.binning_file

    # read yaml configuration file
    hist_dict = load_configuration(yaml_file, format='yaml')
    
    for particle, particle_dict in hist_dict['particles'].items():
        for data_type in particle_dict['data_types']:
            for year in particle_dict['years']:
                for mag in particle_dict['polarities']:
                    for sample in particle_dict['samples']:
                        
                        #
                        if 'acceptable_config' in particle_dict.keys():
                            check = check_if_configuration_is_acceptable(data_type, year, mag, sample, particle_dict['acceptable_config'])
                            if not check:
                                print(f"Configuration {(data_type, year, mag, sample, particle_dict['acceptable_config'])} not acceptable, skipping!")
                                continue
                            
                        channel =  particle_dict['channel']
                        bin_var_list = particle_dict['bin-vars']
                        pid_cut_list = particle_dict['pid-cuts']

                        
                        if 'output-dir' in particle_dict.keys():
                            output_dir = particle_dict['output-dir']
                        else:
                            output_dir = None
                        if 'particle-prefix' in particle_dict.keys():
                            particle_prefix = particle_dict['particle-prefix']
                        else:
                            particle_prefix = get_prefix_for_particle(particle)
                        if 'output-dir-fake-sweights' in particle_dict.keys():
                            output_dir_fake_sweights = particle_dict['output-dir-fake-sweights'] + f"{data_type}/{year}/{mag}/{channel}/{particle}/"

                        
                        if data_type == "MC":
                            fn_list = get_pidcalib2_file_location(data_type, channel, year, mag, sample)
                            # for simulation no real sweights are needed, simply create a file with sweight column equal to one
                            sweight_dir = produce_fake_sweight_tuples(output_dir_fake_sweights, fn_list, get_ttree_for_particle(particle))
                            sel_string = get_mc_selection(data_type, channel, year, mag)
                            pidcalib2_config = get_mc_dictionary_for_pidcalib2(particle, year, mag, sample, fn_list, sel_string, sweight_dir, particle_prefix)
                        else:
                            pidcalib2_config = get_standard_dict_for_data()

                        # write json file needed to pass for the production of histograms
                        if output_dir is None:
                            output_dir = f'{pwd}/json/{data_type}/{year}/{mag}/{channel}/{particle}/'
                        os.system(f'mkdir -p {output_dir}')
                        outputfile = output_dir + f"{data_type.lower()}-samples.json"
                        write_configuration(pidcalib2_config, outputfile, format='json')
                        
                        # produce the command in pidcalib2 to get the desired efficiency histograms
                        command = "#!/bin/bash\n"
                        command_hist = "lb-conda pidcalib python -m pidcalib2.make_eff_hists"
                        command_hist += " --magnet="+mag
                        command_hist += " --sample="+sample
                        command_hist += " --samples-file="+outputfile
                        command_hist += " --particle="+particle
                        command_hist += " --output-dir="+output_dir
                        
                        if binning_file:
                            command_hist += " --binning-file="+binning_file
                            binning_name = os.path.basename(binning_file).replace(".json","")
                        else:
                            binning_name = 'std'
                            
                        for ibinvar in bin_var_list:
                            command_hist += " --bin-var='{}'".format(ibinvar)
                        for ipid in pid_cut_list:
                            command_hist += " --pid-cut='{}'".format(ipid)
                            
                        if "prior-cuts" in particle_dict.keys():
                            for ipriorcut in particle_dict['prior-cuts']:
                                command_hist += " --cut='{}'".format(ipriorcut)
                            
                        command += command_hist
                            
                        if save_in_root:
                            pkl_list = glob.glob(f"{output_dir}/*pkl")
                            for ipkl in pkl_list:
                                command += f'\nlb-conda pidcalib python -m pidcalib2.pklhisto2root "{output_dir}/{ipkl}"'
                        if plot:
                            command += f'\n'+command_hist.replace("-m pidcalib2.make_eff_hists",f"{pwd}/plot_histograms.py")
                        if plot_only:
                            os.system(command_hist.replace("-m pidcalib2.make_eff_hists","plot_histograms.py"))
                            
                        # Save 
                        output_dir = f'{pwd}/scripts/bash_scripts/{data_type}/{year}/{mag}/{channel}/{particle}/{sample}/{"_".join(bin_var_list)}/'
                        os.system(f"mkdir -p {output_dir}")
                        output_file = f'{output_dir}/run.sh'
                        print(f"Writing bash script: {output_file}")
                        with open(output_file, "w") as file:
                            file.write(command + "\n")
                            
                        condorfold = f'{pwd}/scripts/condor_scripts/{data_type}/{year}/{mag}/{channel}/{particle}/{sample}/{"_".join(bin_var_list)}/'
                        os.system(f'mkdir -p {condorfold}/log/')
                        os.system(f'mkdir -p {condorfold}/output/')
                        os.system(f'mkdir -p {condorfold}/error/')

                        script_condor_sub=f'{condorfold}/options.submit'
                        os.system(f"cp {pwd}/options.submit {script_condor_sub}")
                        os.system('sed -i "s|__SCRIPT__|{}|g" {}'.format(output_file, script_condor_sub))
                        os.system('sed -i "s|__BASEFOLDLOG__|{}|g" {}'.format(condorfold, script_condor_sub))
                        os.system('sed -i "s|__USER__|{}|g" {}'.format(user, script_condor_sub))

                        # Submit the job
                        if dry_run:
                            print(f"Submit with 'condor_submit {script_condor_sub}'")
                        else:
                            os.system(f"condor_submit {script_condor_sub}")
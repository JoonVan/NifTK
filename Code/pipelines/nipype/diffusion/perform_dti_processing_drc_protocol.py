#! /usr/bin/env python

import nipype.interfaces.utility        as niu            
import nipype.interfaces.io             as nio     
import nipype.pipeline.engine           as pe
import nipype.interfaces.dcm2nii        as mricron
import nipype.interfaces.niftyreg       as niftyreg
import nipype.interfaces.niftyseg       as niftyseg
import diffusion_mri_processing         as dmri
from midas2dicom import Midas2Dicom
import argparse
import os

mni_template = os.path.join(os.environ['FSLDIR'], 'data', 'standard', 'MNI152_T1_2mm.nii.gz')
mni_template_mask = os.path.join(os.environ['FSLDIR'], 'data', 'standard', 'MNI152_T1_2mm_brain_mask_dil.nii.gz')

def find_and_merge_dwi_data (input_bvals, input_bvecs, input_files):

    import os, glob, sys, errno
    import nipype.interfaces.fsl as fsl
    
    def merge_vector_files(input_files):
        import numpy as np
        import os
        result = np.array([])
        files_base, files_ext = os.path.splitext(os.path.basename(input_files[0]))
        for f in input_files:
            if result.size == 0:
                result = np.loadtxt(f)
            else:
                result = np.hstack((result, np.loadtxt(f)))
        output_file = os.path.abspath(files_base + '_merged' + files_ext)
        np.savetxt(output_file, result, fmt = '%.3f')
        return output_file
    
    input_path = os.path.dirname(input_files[0])
    dwis_files = []

    if len(input_bvals) == 0 or len(input_bvecs) == 0:
        print 'I/O Could not any diffusion based images in ', input_path, ', exiting.'
        sys.exit(errno.EIO)

    for bvals_file in input_bvals:
        dwi_base, _ = os.path.splitext(bvals_file)
        dwi_file = dwi_base + '.nii.gz'
        if not os.path.exists(dwi_file):
            dwi_file = dwi_base + '.nii'
        if not os.path.exists(dwi_file):
            print 'I/O The DWI file with base ', dwi_base, ' does not exist, exiting.'
            sys.exit(errno.EIO)
        dwis_files.append(dwi_file)

    merger = fsl.Merge(dimension = 't')
    merger.inputs.in_files = dwis_files
    res = merger.run()
    dwis = res.outputs.merged_file
    bvals = merge_vector_files(input_bvals)
    bvecs = merge_vector_files(input_bvecs)

    fms = glob.glob(input_path + os.sep + '*fieldmap*.nii*')
    if len(fms) == 0:
        print 'I/O Could not find any field map image in ', input_path, ', exiting.'
        sys.exit(errno.EIO)
    if len(fms) < 2:
        print 'I/O Field Map error: either the magnitude or phase is missing in ', input_path, ', exiting.'
        sys.exit(errno.EIO)
    if len(fms) > 2:
        print 'I/O Field Map warning: there are more field map images than expected in ', input_path, ', \nAssuming the first two are relevant...'
    fmmag = fms[0]
    fmph  = fms[1]

    t1s = glob.glob(input_path + os.sep + 'o*MPRAGE*.nii*')
    if len(t1s) == 0:
        t1s = glob.glob(input_path + os.sep + '*MPRAGE*.nii*')
    if len(t1s) == 0:
        print 'I/O Could not find any MPRAGE image in ', input_path, ', exiting.'
        sys.exit(errno.EIO)
    if len(t1s) > 1:
        print 'I/O warning: there is more than 1 MPRAGE image in ', input_path, ', \nAssuming the first one is relevant...'    
    t1 = t1s[0]

    return dwis, bvals, bvecs, fmmag, fmph, t1

help_message = \
'Perform Diffusion Model Fitting with pre-processing steps. \n\n' + \
'Mandatory Input is the 4/5 MIDAS code from which the DWIs, bval bvecs \n' + \
'as well as a T1 image are extracted for reference space. \n\n' + \
'The Field maps are provided so susceptibility correction is applied. \n' + \
'Use the --model option to control which diffusion model to use (tensor or noddi)' 

model_choices = ['tensor', 'noddi', 'both']

parser = argparse.ArgumentParser(description=help_message)

parser.add_argument('-m', '--midas_code',
                   dest='midas_code',
                   narg='+',
                   help='MIDAS code of the subject image')

parser.add_argument('--model',
                    dest='model',
                    metavar='model',
                    help='Diffusion Model to use, choices are ' + str(model_choices) + ' default: tensor',
                    required=False,
                    choices = model_choices,
                    default = model_choices[0])
parser.add_argument('-o', '--output',
                    dest='output',
                    metavar='output',
                    help='Result directory where the output data is to be stored',
                    required=False,
                    default='results')
parser.add_argument('-r', '--resample-t1',
                    dest='resample_t1',
                    metavar='resample_t1',
                    help='Resample the outputs in the T1 space',
                    required=False,
                    type=int,
                    default=0)

args = parser.parse_args()

result_dir = os.path.abspath(args.output)

if not os.path.exists(result_dir):
    os.mkdir(result_dir)


r = dmri.create_diffusion_mri_processing_workflow(name = 'dmri_workflow', 
                                                  resample_in_t1 = args.resample_t1, 
                                                  log_data = True,
                                                  correct_susceptibility = True,
                                                  dwi_interp_type = 'CUB',
                                                  t1_mask_provided = True,
                                                  ref_b0_provided = False,
                                                  model = args.model)
r.base_dir = os.getcwd()


infosource = pe.Node(niu.IdentityInterface(fields = ['subject_id']),
                     name = 'infosource')
infosource.iterables = ('subject_id', args.midas_code)

midas2dicom = pe.Node(Midas2Dicom(), name='m2d')
database_paths = ['/var/lib/midas/data/fidelity/images/ims-study/',
                  '/var/lib/midas/data/ppadti/images/ims-study/']
midas2dicom.inputs.midas_dirs = database_paths

dg = pe.Node(nio.DataGrabber(outfields = ['dicom_files']), name='dg')
dg.inputs.template = '*'
dg.inputs.sort_filelist = False

dcm2nii = pe.Node(interface = mricron.Dcm2nii(), 
                  name = 'dcm2nii')
dcm2nii.inputs.args = '-d n'
dcm2nii.inputs.gzip_output = True
dcm2nii.inputs.anonymize = False
dcm2nii.inputs.reorient = True
dcm2nii.inputs.reorient_and_crop = False

find_and_merge_dwis = pe.Node(interface = niu.Function(input_names = ['input_bvals', 'input_bvecs', 'input_files'],
                                                       output_names = ['dwis', 'bvals', 'bvecs', 'fieldmapmag', 'fieldmapphase', 't1'],
                                                       function = find_and_merge_dwi_data),
                              name = 'find_and_merge_dwis')

mni_to_input = pe.Node(interface=niftyreg.RegAladin(), name='mni_to_input')
mni_to_input.inputs.flo_file = mni_template

mask_resample  = pe.Node(interface = niftyreg.RegResample(), name = 'mask_resample')
mask_resample.inputs.inter_val = 'NN'
mask_resample.inputs.flo_file = mni_template_mask

r.connect(infosource, 'subject_id', midas2dicom, 'midas_code')
r.connect(midas2dicom, 'dicom_dir', dg, 'base_directory')
r.connect(dg, 'dicom_files', dcm2nii, 'source_names')
r.connect(dcm2nii, 'converted_files', find_and_merge_dwis, 'input_files')
r.connect(dcm2nii, 'bvals', find_and_merge_dwis, 'input_bvals')
r.connect(dcm2nii, 'bvecs', find_and_merge_dwis, 'input_bvecs')
r.connect(find_and_merge_dwis, 't1', mni_to_input, 'ref_file')
r.connect(find_and_merge_dwis, 't1', mask_resample, 'ref_file')
r.connect(mni_to_input, 'aff_file', mask_resample, 'aff_file')
r.connect(mask_resample, 'res_file', r.get_node('input_node'), 'in_t1_mask')
r.connect(find_and_merge_dwis, 'dwis', r.get_node('input_node'), 'in_dwi_4d_file')
r.connect(find_and_merge_dwis, 'bvals', r.get_node('input_node'), 'in_bval_file')
r.connect(find_and_merge_dwis, 'bvecs', r.get_node('input_node'), 'in_bvec_file')
r.connect(find_and_merge_dwis, 'fieldmapmag', r.get_node('input_node'), 'in_fm_magnitude_file')
r.connect(find_and_merge_dwis, 'fieldmapphase', r.get_node('input_node'), 'in_fm_phase_file')
r.connect(find_and_merge_dwis, 't1', r.get_node('input_node'), 'in_t1_file')

mask_eroder = pe.Node(interface = niftyseg.BinaryMaths(), 
                      name = 'mask_eroder')
mask_eroder.inputs.operation = 'ero'
mask_eroder.inputs.operand_value = 3

ds = pe.Node(nio.DataSink(), name='ds')
ds.inputs.base_directory = result_dir
ds.inputs.parameterization = False

r.connect(mask_resample, 'res_file', mask_eroder, 'in_file')

if (args.model == 'tensor' or args.model == 'both'):
    
    multiplicater_fa = pe.Node(interface = niftyseg.BinaryMaths(), 
                               name = 'multiplicater_fa')
    multiplicater_fa.inputs.operation = 'mul'
    multiplicater_md = pe.Node(interface = niftyseg.BinaryMaths(), 
                               name = 'multiplicater_md')
    multiplicater_md.inputs.operation = 'mul'
    multiplicater_v1 = pe.Node(interface = niftyseg.BinaryMaths(), 
                               name = 'multiplicater_v1')
    multiplicater_v1.inputs.operation = 'mul'
    multiplicater_colfa = pe.Node(interface = niftyseg.BinaryMaths(), 
                                  name = 'multiplicater_colfa')
    multiplicater_colfa.inputs.operation = 'mul'
    
    r.connect(mask_eroder, 'out_file', multiplicater_fa, 'operand_file')
    r.connect(mask_eroder, 'out_file', multiplicater_md, 'operand_file')
    r.connect(mask_eroder, 'out_file', multiplicater_v1, 'operand_file')
    r.connect(mask_eroder, 'out_file', multiplicater_colfa, 'operand_file')
    r.connect(r.get_node('output_node'), 'FA', multiplicater_fa, 'in_file')
    r.connect(r.get_node('output_node'), 'MD', multiplicater_md, 'in_file')
    r.connect(r.get_node('output_node'), 'V1', multiplicater_v1, 'in_file')
    r.connect(r.get_node('output_node'), 'COL_FA', multiplicater_colfa, 'in_file')
    
    r.connect(r.get_node('output_node'), 'tensor', ds, '@tensors')
    r.connect(multiplicater_fa, 'out_file', ds, '@fa')
    r.connect(multiplicater_md, 'out_file', ds, '@md')
    r.connect(multiplicater_colfa, 'out_file', ds, '@colfa')
    r.connect(multiplicater_v1, 'out_file', ds, '@v1')
    r.connect(r.get_node('output_node'), 'predicted_image_tensor', ds, '@img_tensor')
    r.connect(r.get_node('output_node'), 'residual_image_tensor', ds, '@res_tensor')

if (args.model == 'noddi' or args.model == 'both'):

    r.connect(r.get_node('output_node'), 'mcmap', ds, '@mcmap')
    r.connect(r.get_node('output_node'), 'predicted_image_noddi', ds, '@img_noddi')
    r.connect(r.get_node('output_node'), 'residual_image_noddi', ds, '@res_noddi')

r.connect(r.get_node('output_node'), 'dwis', ds, '@dwis')
r.connect(r.get_node('output_node'), 'T1toB0_transformation', ds, '@transformation')
r.connect(r.get_node('output_node'), 'average_b0', ds, '@b0')

#r.connect(r.get_node('output_node'), 'transformations', ds, 'transformations')

r.write_graph(graph2use = 'colored')

qsubargs='-l h_rt=00:05:00 -l tmem=1.8G -l h_vmem=1.8G -l vf=2.8G -l s_stack=10240 -j y -b y -S /bin/csh -V'
#r.run(plugin='SGE',       plugin_args={'qsub_args': qsubargs})
r.run(plugin='MultiProc')


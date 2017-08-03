from distutils.core import setup, Extension

module1 = Extension('SIRsimPyModule',
                    sources = ['SIRsimPyModule.cpp', '../src/SIRlib.cpp'],
                    include_dirs = ['../SIRlib/include/SIRlib/','../../SimulationLib/SimulationLib/include/SimulationLib','../../StatisticalDistributionsLib/include/StatisticalDistributionsLib'])

setup (name = 'PackageName',
       version = '',
       description = 'This is a SIRsimPyModule package',
       package_dir={ '': '/Users/nlin/Documents/LEPH/SIRlib/TestSIRlib' },
       depends={"../SIRlib/include/SIRlib/SIRlib.h"},
       ext_modules = [module1])

from distutils.core import setup, Extension

module1 = Extension('SIRsimPyModule',
                    sources = ['SIRsimPyModule.cpp', 'SIRSimRunner.cpp'],
                    include_dirs = ['../SIRlib/include/SIRlib/','../../SimulationLib/SimulationLib/include/SimulationLib','../../StatisticalDistributionsLib/include/StatisticalDistributionsLib', './'],
                    libraries = ['SIRlib', 'SimulationLib', 'StatisticalDistributionsLib'],
                    library_dirs = ['/usr/local/lib/SimulationLib-0.2/','/usr/local/lib/SIRlib-0.1/', '/usr/local/lib/StatisticalDistributionsLib-0.2/'],
                    extra_compile_args=['-std=c++14', '-v', '-mmacosx-version-min=10.9']
                    )

setup (name = 'PackageName',
       version = '',
       description = 'This is a SIRsimPyModule package',
       package_dir={ '': '/Users/nlin/Documents/LEPH/SIRlib/TestSIRlib' },
       ext_modules = [module1])

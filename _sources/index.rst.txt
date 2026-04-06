.. Brainvox documentation master file, created by
   sphinx-quickstart on Thu Apr  2 14:08:45 2026.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Brainvox |version| Documentation
================================

.. toctree::
   :maxdepth: 1
   :hidden:

   brainvox
   tal_programs
   cvio


Overview
--------
Included in this package is the source code to the Brainvox tool described [Frank1997]_:

.. [Frank1997] Frank RJ, Damasio H, Grabowski TJ, "Brainvox: An Interactive, Multimodal, 
   Visualization and Analysis System for Neuroanatomical Imaging," NeuroImage, 5:13-30, 1997.

| It also includes the supporting set of command line tools known collectively as the 'tal' 
| programs and various other utility applications and source code that folks may find useful.


Building
--------
The core sources are designed to be built using the Python SCons and Sphinx modules. 

Setup
^^^^^
A Python virtual environment is used to install the modules needed to run SCons and
Sphinx:


.. code::

   python -m virtualenv venv
   .\venv\Scripts\activate.ps1
   python -m pip install -r requirements.txt


Binaries
^^^^^^^^
The binary applications can be built using the following commands:

.. code::

   scons --clean
   scons -j 10


Subsequent builds need just use the `scons` command, the `--clean` option is used to
force a clean rebuild.  The executables will be placed in the `bin` directory when `scons`
finishes.

Documentation
^^^^^^^^^^^^^
The documentation can be built using the sphinx command line tool.  To generate html 
from the source code:

.. code::

   cd docs
   sphinx-build -a -E -b html source build


The `-a -E` options clean the `build` directory before generating the html content.

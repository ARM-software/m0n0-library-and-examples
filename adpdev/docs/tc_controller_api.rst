.. _chap-testcase-controller:

Testcase Controller
*******************

The M0N0S2 class instantiates an instance of the ``TestcaseController`` class and exposes it through the ``tcs`` property. 

The ``tcs`` object uses the ``testcase-list.csv`` file in the passed build directory (if provided) to create a table defining how to run the testcases included in the binary. It provides methods for running the testcases and extracting information from them. 


Reference
#########

.. automodule:: testcase_control
   :members:
   :undoc-members:
   :show-inheritance:

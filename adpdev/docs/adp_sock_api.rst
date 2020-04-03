.. _chap-adp-socket:

ADP Socket
***********

Handles the ADP interface with the M0N0 system, allowing read to/writing from the memory map, flashing of binaries to memory and sending/receiving STDIN/STDOUT. 

It also outputs several log files, for example (default filenames):

``adp_transactions.log``
    Contains all of the transactions written to and received from the M0N0 system via ADP
``adp_transactions.log-stripped.log``
    Same as ``adp_transactions.log`` but removing all ``<`` and ``>`` characters to make reading of STDOUT (in command mode) easier

Note that the default log file names can be changed, for example, when connected to multiple chips simultaneously. 

The command results and STDOUT received via ADP is constantly being read in a separate thread to prevent the system from locking up, see :ref:`chap-read-buffer`. 

Reference
#########

.. automodule:: adp_socket
   :members:
   :show-inheritance:

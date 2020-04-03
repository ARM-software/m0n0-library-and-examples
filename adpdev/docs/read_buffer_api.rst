.. _chap-read-buffer:

Read Buffer
***********

A ``ReadBuffer`` object is instantiated in the ``ADP_Conn`` to handling the constant reading of the received ADP data (including STDOUT) from M0N0. 

The data received from M0N0 is constantly read in a separate thread and placed in software buffers that can be created, read, and flushed independently, and configured to either store all data received or only STDOUT (printf). 

Reference
#########

.. automodule:: read_buffer
   :members:
   :show-inheritance:

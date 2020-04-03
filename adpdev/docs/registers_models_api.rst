.. _chap-registers_models:

Registers Models
****************

M0N0 system registers can be read/written from/to using the instances of the ``Registers_Model`` class available through the ``M0N0S2`` class (see :ref:`chap-testchip`). 

A ``Registers_Model`` object is instantiated using a register definition file (in YAML format). The ``Registers_Model`` object can calculate bit-group masks and also keep a running model of the current register values for checking/debugging and for writing to specific bit-groups of write-only registers (such as the PCSM). 

The ``Registers_Model`` class is agnostic to the device read/write interfaces and does not have any knowledge of the ADP interface. It allows custom read/write drivers to be set, which are callback functions used to read/write from/to the physical device. 
For example, the M0N0S2 chip sets "dummy" drivers initially for testing of reading/writing registers and replaces these dummy drivers with ADP drivers when connecting to the ADP interface. 
While the models support multiple drivers for both read/write, M0N0 only sets single, default, read/write drivers. 
All M0N0 registers set the ``memory_read`` method of the ``adp_sock`` instance for reading and the ``memory_write`` method of the ``adp_sock`` instance for writing (except if the register is read-only). An exception of the ``pcsm_regs`` which has the private ``_adp_to_pcsm_write`` function set as the write driver (which itself uses the ``spi_regs`` ``Registers_Model`` instance). 

In addition to reading and writing from/to the device, ``Registers_Model`` instances allow information about the register definition or the current model state to be extracted or exported. 

Reference
#########

.. automodule:: registers_model
   :members:
   :undoc-members:
   :show-inheritance:

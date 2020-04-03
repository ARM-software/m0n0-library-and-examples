.. _chap-testchip:

Testchip
********

The system chip behaviour is encapsulated in the ``M0N0S2`` class, which is derived from the more generic ``TestChip`` class. 

There is also a static ``setup_chip`` utility function that instantiate a ``M0N0S2`` object using a dictionary of parameters (e.g. from command-line arguments). 

Access to system registers is available though instances of the ``Registers_Model`` class (see :ref:`chap-registers_models`) exposed through the following properties:

* ``ctrl_regs`` (read/write)
* ``status_regs`` (read-only)
* ``spi_regs`` (read/write)
* ``gpio_regs`` (read/write)
* ``pcsm_regs`` (write-only, also exposed through ``pcsm``)

All ADP connection is handled though an instance of the ``ADP_Conn`` class (see :ref:`chap-adp-socket`) exposed through the ``adp_sock`` property. 
The register objects above (with the exception of the ``pcsm_regs``) have a function for reading/writing via ADP passed to them as a "read_driver" and a "write_driver" so that they can interface with the registers. 

.. note:: The register objects should be used to read/write system registers, rather than using the ``adp_sock`` directly. 

Memory map information, such as the address and size of memory regions, is available through the ``mem_map`` property (instances of the ``MemoryMap`` class, see :ref:`chap-memory-map`). 

Information on testcases compiled into the embedded software (if a ``testcase-list.csv`` file is provided next to the binary in the build directory) and methods for running them are exposed through the ``tcs`` property (instance of the ``TestcaseController`` class, see :ref:`chap-testcase-controller`).  

Reference
#########

.. automodule:: testchip
   :members:
   :undoc-members:
   :show-inheritance:

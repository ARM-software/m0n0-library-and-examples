# Generating Software Reference Documentation

Much of the library source code contains DocStrings which can be read by [Doxygen](http://www.doxygen.nl/manual/docblocks.html)

This documentation can be compiled as a PDF (via LaTeX) or HTML. 


## Installation

1. Download and Install [Doxygen](http://www.doxygen.nl/download.html)
2. (If a PDF output is desired) Install LaTeX (e.g. install [TexLive](https://www.tug.org/texlive/) or [MacTex](https://www.tug.org/mactex/) for MacOS. 

## Document Compilation

From the `docs` directory:

```
doxygen Doxyfile
```

This creates two directories:

* `latex`
* `html`

The HTML documentation can be viewed by opening the `html/index.html` file in a web browser.

To compile the PDF, go into the `latex` directory and run `make` (requires LaTeX installation). 

The created PDF is called `refman.pdf`. 

Note that the `ADPDev` software has a separate `docs` directory within the `adpdev` directory.


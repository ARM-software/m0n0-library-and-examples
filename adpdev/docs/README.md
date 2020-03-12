# Generating ADPDev Reference Documentation

Much of the ADPDev source code contains DocStrings which can be read by [Sphinx](https://www.sphinx-doc.org/en/master/)

This documentation can be compiled as a PDF (via LaTeX) or HTML. 

## Installation

1. Download and Install [Sphinx](https://www.sphinx-doc.org/en/master/usage/installation.html) - e.g. one option is though `conda` as used for running the ADPDev code. 
2. (If a PDF output is desired) Install LaTeX (e.g. install [TexLive](https://www.tug.org/texlive/) or [MacTex](https://www.tug.org/mactex/) for MacOS. 

## Document Compilation

**Note:** Ensure your python environment (i.e. `conda`) has been activated in the console you use to build this. 

For example:
```
conda activate adpenv
```

### HTML Output

From the `docs` directory:

```
make html
```

The resulting `_build/html/index.html` can be opened in a web browser. 

### PDF Output

From the `docs` directory:

```
make latex
```

To build the latex:
```
cd _build/latex
make
```

There should now be PDF: `_build/latex/adpdev.pdf`

Or, alternatively, from the `docs` directory run:
```
make latexpdf
```
to create the LaTeX source and compile it in one go. 



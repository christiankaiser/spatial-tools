r.percolation
=============



WHAT DOES r.percolation DO ?
----------------------------
It computes the spatial clusters in a raster map, based on percolation.




HOW TO USE IT ?
---------------
r.percolation is a command line utility written in C.
Call the program with the -h flag for more information on the syntax and
available options.




HOW TO COMPILE THE PROGRAM FOR MY OS ?
--------------------------------------
r.percolation should compile with almost every recent ANSI C compiler.
It needs to be linked against the GDAL library (version 1.5 or later should 
be fine). For more information about GDAL see http://www.gdal.org.



LICENSE
-------
r.percolation is released under the GNU General Public License. Therefore, you
are free to use this software in your projects as long as you release it under
the same license. Use in closed source software is prohibited.



AUTHOR
------
Christian Kaiser, University of Lausanne
Please send any comments or bug reports to christian@361degres.ch.



ACKNOWLEDGEMENTS
----------------
This work has been supported by the Swiss National Foundation, through the
projects "Urbanization Regime and Environmental Impact: Analysis and Modelling
of Urban Patters, Clustering and Methamorphoses" (no 100012-113506, see
www.clusterville.org for more information) and "GeoKernels": Kernel-Based
Methods for Geo- and Environmental Sciences, phase 2 (no 200020-121835, see
www.geokernels.org for more information).






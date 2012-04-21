#!/usr/bin/env bash

./v.lines.to.rast.py --input flows.txt --output flows.img --xmin 450000 --xmax 850000 --ymin 75000 --ymax 300000 --res 1000 --weighted

/*
 
 This file is part of r.groupcells
 
 Version:	1.0
 Date:		22.5.2009
 Author:	Christian Kaiser, christian.kaiser@unil.ch
 
 Copyright (C) 2009 Christian Kaiser.
 
 r.groupcells is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 2, or (at your option) any
 later version.
 
 r.groupcells is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received a copy of the GNU General Public License
 along with Octave; see the file COPYING.  If not, write to the Free
 Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 02110-1301, USA.
 
 */


int groupcells (char *iraster, 
				char *oraster, 
				char *oformat, 
				int r, 
				char *statistic);




char* arrayConvertFloat64ToChar(double *data, int len);
short* arrayConvertFloat64ToInt16(double *data, int len);
unsigned short* arrayConvertFloat64ToUInt16(double *data, int len);
long* arrayConvertFloat64ToInt32(double *data, int len);
unsigned long* arrayConvertFloat64ToUInt32(double *data, int len);
float* arrayConvertFloat64ToFloat32(double *data, int len);


// koalatoppm, a program that converts Koala Painter (C64) pictures to
// the portable pixmap (ppm) format
// Copyright © 1998 Peter Karlsson
//
// $Id$
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include <stdio.h>

// Struct defining Koala pictures
typedef struct
{
	unsigned char	loadaddress[2];		// highbit, lowbit
	unsigned char	image[8000];		// pixmap image
	unsigned char	colour1[1000];		// first colourmap (colour 1 and 2)
	unsigned char	colour2[1000];		// second colourmap (colour 3)
	unsigned char	background;			// background colour
} koala_t;

// Pixel masks
const unsigned char pixelmask[4] =
{ 0xc0, 0x30, 0x0c, 0x03 };

// Pixel displacement
const unsigned char pixeldisplacement[4] =
{ 6, 4, 2, 0 };

// Struct used to define colour mapping
typedef struct
{
	int		r, g, b;			// Colourmap entry
} colour_t;

// RGB colour codes for the C64 colours. These are borrowed from Vice.
const colour_t c64colours[16] =
{
	{   0,   0,   0 },	// Black
	{ 255, 255, 255 },	// White
	{ 189,  24,  33 },	// Red
	{  49, 231, 198 },	// Cyan
	{ 181,  24, 231 },	// Purple
	{  24, 214,  24 },	// Green
	{  33,  24, 173 },	// Blue
	{ 222, 247,   8 },	// Yellow
	{ 189,  66,   0 },	// Orange
	{ 107,  49,   0 },	// Brown
	{ 255,  74,  82 },	// Light red
	{  66,  66,  66 },	// Gray 1
	{ 115, 115, 107 },	// Gray 2
	{  90, 255,  90 },	// Light green
	{  90,  82, 255 },	// Light blue
	{ 165, 165, 165 }	// Gray 3
};

// Function headers
int convert(const char *fname1, const char *fname2, int expand);

// main
int main(int argc, char *argv[])
{
	int expand = 0;

	if (argc < 1 || argc > 4)
	{
		printf("Usage: %s [+] [infile [outfile]]\n", argv[0]);
		printf("           ^-expand picture to 320x200\n");
		return 1;
	}

	if (argc > 1 && !strcmp(argv[1], "+"))	expand = 1;
	
	if (1 == argc - expand)
		return convert(NULL, NULL, expand);
	else if (2 == argc - expand)
		return convert(argv[1 + expand], NULL, expand);
	else if (3 == argc - expand)
		return convert(argv[1 + expand], argv[2 + expand], expand);
	else
		return 1; // Shouldn't happen!
}

// Convertion function
int convert(const char *fname1, const char *fname2, int expand)
{
	FILE	*input, *output;
	koala_t	image;
	int		x, y, pixel, index, colourindex, mask, r, g, b;

	if (fname1)
	{
		// Open input file
		input = fopen(fname1, "rb");
		if (!input)	return 1;
		
		// Check file size
		fseek(input, 0, SEEK_END);
		if (10003 != ftell(input))
		{
			fprintf(stderr, "Input file size mismatch.\n");
			return 1;
		}
		rewind(input);
	}
	else
		input = stdin;

	if (fname2)
	{
		// Open output file
		output = fopen(fname2, "wb");
		if (!output)	return 1;
	}
	else
		output = stdout;

	// Load the Koala image
	if (10003 != fread(&image, 1, sizeof(image), input))
	{
		fprintf(stderr, "Input ended prematurely\n");
		return 1;
	}
	
	// Close the file
	fclose(input);

	if (0 != image.loadaddress[0] || 0x60 != image.loadaddress[1])
	{
		fprintf(stderr, "Load address mismatch. Amica picture?\n");
	}
	
	// Create the PPM output

	// Header
	fprintf(output, "P6\n# Created by koala2ppm\n%d 200 255\n",
			(expand) ? 320 : 160);

	// Image
	for (y = 0; y < 200; y ++)
	{
		for (x = 0; x < 160; x ++)
		{
			// Get value of pixel at (x,y)
			index = (x / 4) * 8 + (y % 8) + (y / 8) * 320;
			colourindex = (x / 4) + (y / 8) * 40;
			pixel = (image.image[index] & pixelmask[x % 4]) >>
				    pixeldisplacement[x % 4];

			// Retrieve RGB values
			switch (pixel)
			{
			case 0: // Background
				r = c64colours[image.background].r;
				g = c64colours[image.background].g;
				b = c64colours[image.background].b;
				break;
				
			case 1: // Colour 1
				r = c64colours[image.colour1[colourindex] >> 4].r;
				g = c64colours[image.colour1[colourindex] >> 4].g;
				b = c64colours[image.colour1[colourindex] >> 4].b;
				break;
				
			case 2: // Colour 2
				r = c64colours[image.colour1[colourindex] & 0xf].r;
				g = c64colours[image.colour1[colourindex] & 0xf].g;
				b = c64colours[image.colour1[colourindex] & 0xf].b;
				break;
				
			case 3: // Colour 3
				r = c64colours[image.colour2[colourindex] & 0xf].r;
				g = c64colours[image.colour2[colourindex] & 0xf].g;
				b = c64colours[image.colour2[colourindex] & 0xf].b;
				break;

			default:
				fprintf(stderr, "Internal error\n");
				break;
			};

			// Print to PPM file
			fputc(r, output);
			fputc(g, output);
			fputc(b, output);

			if (expand)
			{
				fputc(r, output);
				fputc(g, output);
				fputc(b, output);
			}
		}
	}
	
	fclose(output);
}

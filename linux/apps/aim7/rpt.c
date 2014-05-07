#ifndef	lint
static char sccs_id[] = { " @(#) rpt.c:1.7 1/22/96 16:16:56" };
#endif

/****************************************************************
**                                                             **
**    Copyright (c) 1996 - 2001 Caldera International, Inc.    **
**                    All Rights Reserved.                     **
**                                                             **
** This program is free software; you can redistribute it      **
** and/or modify it under the terms of the GNU General Public  **
** License as published by the Free Software Foundation;       **
** either version 2 of the License, or (at your option) any    **
** later version.                                              **
**                                                             **
** This program is distributed in the hope that it will be     **
** useful, but WITHOUT ANY WARRANTY; without even the implied  **
** warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR     **
** PURPOSE. See the GNU General Public License for more        **
** details.                                                    **
**                                                             **
** You should have received a copy of the GNU General Public   **
** License along with this program; if not, write to the Free  **
** Software Foundation, Inc., 59 Temple Place, Suite 330,      **
** Boston, MA  02111-1307  USA                                 **
**                                                             **
****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define NUM_HEADERS (4)			/* number of header lines */
#define LINE_SIZE (128)			/* default line size */
#define VERTICAL_SPACE_BETWEEN_GRAPHS (1.75)
#define BOTTOM_SPACE (1.25)
#define LEFT_MARGIN (1.5)
#define GRAPH_HEIGHT (1.25)
#define GRAPH_WIDTH (6.0)
#define normal_format "%.0f"
#define bold_font  "Times-Bold"
#define normal_font  "Times-Roman"

typedef struct _data_point {
	struct _data_point *next;
	double task;
	double task_rate;
	double jti;
	double real;
	double cpu;
	double jobs_sec_user;
} Data_point;

Data_point *data;

typedef struct _vector {
	double x, y;
} Vector;

char
 *progname,				/* name of program */
 *strdup(),				/* never quite declared when you need it */

 *header[5];				/* top few lines of the program */

char *graph_names[] = {
	"Jobs/Minute",
	"Job Timing Index",
	"Real Time (sec)",
	"CPU Time (sec)",
	"Jobs/Second/Task",
};

int
  data_points, line_count;

char *ps_init[] = {
	"%! PS-Adobe-2.0",
	"/inch",
	"	{ 72 mul }",
	"	def",
	"%",
	"% beginning (initialization)",
	"%",
};

#define NUM_PS_INIT (sizeof(ps_init) / sizeof(ps_init[0]))
void
draw_line(FILE * f,
	  double x1,
	  double y1,
	  double x2,
	  double y2)
{
	fprintf(f, "%g inch %g inch moveto %g inch %g inch lineto stroke\n",
		x1, y1, x2, y2);
}

void
center_line(FILE * f,
	    char *line,
	    double y)
{
	fprintf(f, " (%s)\n", line);
	fprintf(f, "	/Times-Bold findfont 20 scalefont setfont\n");
	fprintf(f,
		"	dup				%% duplicate string\n");
	fprintf(f, "	stringwidth pop			%% get width\n");
	fprintf(f, "	2 div				%% divide by 2\n");
	fprintf(f,
		"	4.25 inch exch sub		%% subtract from middle\n");
	fprintf(f, "	%g inch moveto			%% go to y value\n",
		y);
	fprintf(f,
		"	show				%% print value on stack\n");
}

void
print_at(FILE * f,
	 char *line,
	 double x,
	 double y,
	 int fontsize)
{
	fprintf(f, " (%s)\n", line);
	fprintf(f, "	/Times-Bold findfont %d scalefont setfont\n",
		fontsize);
	fprintf(f, "	%g inch %g inch moveto		%% go to y value\n", x,
		y);
	fprintf(f,
		"	show				%% print value on stack\n");
}

void
print_centered(FILE * f,
	       char *line,
	       double x,
	       double y,
	       int fontsize)
{
	fprintf(f, " (%s)\n", line);
	fprintf(f, "	/Times-Bold findfont %d scalefont setfont\n",
		fontsize);
	fprintf(f,
		"	dup				%% duplicate string\n");
	fprintf(f, "	stringwidth pop			%% get width\n");
	fprintf(f, "	2 div				%% divide by 2\n");
	fprintf(f,
		"	%g inch exch sub		%% subtract from middle\n",
		x);
	fprintf(f, "	%g inch moveto			%% go to y value\n",
		y);
	fprintf(f,
		"	show				%% print value on stack\n");
}

void
print_right(FILE * f,
	    char *line,
	    double x,
	    double y,
	    int fontsize)
{
	fprintf(f, " (%s)\n", line);
	fprintf(f, "	/Times-Bold findfont %d scalefont setfont\n",
		fontsize);
	fprintf(f,
		"	dup				%% duplicate string\n");
	fprintf(f, "	stringwidth pop			%% get width\n");
	fprintf(f, "	%g inch exch sub %g inch moveto	%% go to y value\n", x,
		y);
	fprintf(f,
		"	show				%% print value on stack\n");
}

void
get_line(FILE * f,
	 char *line)
{
	char *p = line;

	line_count++;
	if (feof(f)) {
		fprintf(stderr, "Unexpected EOF on input file at line %d.\n",
			line_count);
		exit(1);
	}
	fgets(line, LINE_SIZE, f);
	while (*p != '\0') {		/* while not at end of string */
		if (*p == '\n') {	/* if we find an \n */
			*p = '\0';	/* make it a space */
			break;
		}
		p++;			/* and move to next one */
	}
}

void
set_xscale(double *val)
{
	double temp = *val;
	int
	  i, pow = 0;
	static double rounded[] = { 2.0, 5.0, 10.0 };

	while (temp > 10.0) {
		temp /= 10.0;
		pow++;
	}
	while (temp < 1.0) {
		temp *= 10.0;
		pow--;
	}
	for (i = 0; i < sizeof (rounded) / sizeof (rounded[i]); i++) {
		if (temp < rounded[i]) {
			temp = rounded[i];
			break;
		}
	}
	while (pow > 0) {
		temp *= 10.0;
		pow--;
	}
	while (pow < 0) {
		temp /= 10.0;
		pow++;
	}
	*val = temp;
}

void
set_yscale(double *val)
{
	double temp = *val;
	int
	  i, pow = 0;
	static double rounded[] = { 2.5, 5.0, 10.0 };

	while (temp > 10.0) {
		temp /= 10.0;
		pow++;
	}
	while (temp < 1.0) {
		temp *= 10.0;
		pow--;
	}
	for (i = 0; i < sizeof (rounded) / sizeof (rounded[i]); i++) {
		if (temp < rounded[i]) {
			temp = rounded[i];
			break;
		}
	}
	while (pow > 0) {
		temp *= 10.0;
		pow--;
	}
	while (pow < 0) {
		temp /= 10.0;
		pow++;
	}
	*val = temp;
}

int
main(int argc,
     char *argv[])
{
	int
	  graph, i;			/* loop variable */

	FILE *output,			/* output (print) file */
	 *input;			/* input file */
	char
	 *p, *q, totals[60], titles[80], buffer[LINE_SIZE];	/* holds one line */
	Data_point *t;
	Vector *v;
	double

	 
		peak,
		sustain,
		temp,
		temp2,
		temp3, temp4, total, x[3], y[3], area, minjti, cross_over;


	/*
	 * Step 1: check command line parameters
	 */
	progname = argv[0];		/* save program name */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s input-file output-file\n",
			progname);
		exit(1);
	}
	/*
	 * Step 2: Open the input file, create output file
	 */
	input = fopen(argv[1], "r");	/* open input file */
	if (input == NULL) {		/* if we can't read it */
		fprintf(stderr, "%s: Unable to open input file %s\n",	/* talk to human */
			progname, argv[1]);
		exit(1);		/* and die */
	}
	output = fopen(argv[2], "w");	/* create output file */
	if (output == NULL) {		/* if we can't read it */
		fprintf(stderr, "%s: Unable to create output file %s\n",	/* talk to human */
			progname, argv[2]);
		exit(1);		/* and die */
	}
	/*
	 * Step 3: Pull out header records
	 */
	get_line(input, buffer);	/* read in a line */
	get_line(input, buffer);	/* read in a line */
	for (p = buffer, i = 0; i < 4; i++) {
		q = p;
		while (*p != '\t' && *p != '\0')
			p++;
		*p++ = '\0';
		header[i] = strdup(q);
	}
	get_line(input, buffer);	/* read in a line */
	get_line(input, buffer);	/* read in a line */
	/*
	 * Step 4: Fetch data values
	 */
	while (!feof(input)) {		/* while not at end of file */
		t = (Data_point *) malloc(sizeof (Data_point));
		get_line(input, buffer);	/* read in a line */
		if (t == NULL) {
			fprintf(stderr,
				"%s: Unable to allocate memory for line %d.\n",
				progname, line_count);
			exit(1);
		}
		i = sscanf(buffer, "%lf%lf%lf%lf%lf%lf",
			   &t->task, &t->task_rate, &t->jti,
			   &t->real, &t->cpu, &t->jobs_sec_user);
		if (i != 6) {
			fprintf(stderr, "%s: Syntax error on line %d.\n",
				progname, line_count);
			exit(1);
		}
		t->next = data;
		data = t;
		data_points++;
	}
	fclose(input);

	/*
	 * Step 4.5: Calculate sustained and find peak and minimum jti   12/95
	 */

	/*
	 * find peak    
	 */

	peak = 0.0;
	minjti = 100.0;
	t = data;
	for (i = 0; i < data_points; i++) {
		temp = t->task_rate;
		temp2 = t->jti;
		if (temp > peak)
			peak = temp;
		if (temp2 < minjti && i > 1)
			minjti = temp2;
		t = t->next;
	}
	fprintf(stderr, "Peak value is:   %.1f\n", peak);
	fprintf(stderr, "Minimum JTI value is:   %.0f\n", minjti);

	/*
	 * look for cross-over  
	 */
	/*
	 * crossover point can be determined by the last two set of data points.
	 * *  cross_over = [(X1 * Y2) - (X2 * Y1)] / (X1 - Y1 - X2 +Y2)
	 * *  we need the crossover point to calculate the sustained performance
	 * *  correctly
	 */


	t = data;
	for (i = 0; i < 3; i++) {
		x[i] = t->task;
		y[i] = t->task_rate;
		t = t->next;
	}

	cross_over =
		((x[1] * y[2]) - (x[2] * y[1])) / (x[1] - y[1] - x[2] + y[2]);
	fprintf(stderr, "Cross Over value is:   %.1f\n", cross_over);

	/*
	 * to calculate sustained value, calculate the area under the curve
	 * *  using the trapezoidal rule.
	 */

	total = 0.0;
	temp3 = 0.0;
	temp4 = 0.0;
	t = data;
	for (i = 0; i < data_points; i++) {
		temp2 = t->task;
		temp = t->task_rate;
		if (i > 1) {
			if (temp4 < cross_over) {
				area = fabs(((temp + temp3) / 2) * (temp2 -
								    temp4));
				total = total + area;
			} else {
				area = fabs(((cross_over +
					      temp) / 2) * (cross_over -
							    temp2));
				total = total + area;
			}
		}
		temp3 = temp;
		temp4 = temp2;
		t = t->next;
	}

	sustain = sqrt(total);

	fprintf(stderr, "Sustained value is:   %.1f\n", sustain);

	/*
	 * Step 5: Prime for postscript output
	 */
	for (i = 0; i < NUM_PS_INIT; i++)
		fprintf(output, "%s\n", ps_init[i]);
	v = (Vector *) malloc(sizeof (Vector) * data_points);

	/*
	 * Step 6: Print each graph
	 */
	for (graph = 0; graph < 5; graph++) {
		double

		 
			max_x = 0.0, max_y = 0.0,
			base_x, base_y, factor_x, factor_y, delta_x, delta_y;
		t = data;
		for (i = 0; i < data_points; i++) {
			v[i].x = t->task;
			switch (graph) {
			case 0:
				v[i].y = t->task_rate;
				break;
			case 1:
				v[i].y = t->jti;
				break;
			case 2:
				v[i].y = t->real;
				break;
			case 3:
				v[i].y = t->cpu;
				break;
			case 4:
				v[i].y = t->jobs_sec_user;
				break;
			default:
				fprintf(stderr, "%s: Internal error. i = %d\n",
					progname, graph);
				exit(1);
			}
			if (max_y < v[i].y)
				max_y = v[i].y;
			if (max_x < v[i].x)
				max_x = v[i].x;
			t = t->next;
		}

		set_xscale(&max_x);
		set_yscale(&max_y);
		delta_x = 1.0 / max_x;
		delta_y = 1.0 / max_y;
		factor_x = delta_x * GRAPH_WIDTH;
		factor_y = delta_y * GRAPH_HEIGHT;
		base_y = BOTTOM_SPACE + graph * VERTICAL_SPACE_BETWEEN_GRAPHS;
		base_x = LEFT_MARGIN;
		print_centered(output, graph_names[graph], 4.5,
			       base_y + GRAPH_HEIGHT + 0.05, 12);
		fprintf(output, "0.1 setlinewidth\n");
		fprintf(output, "\t%g inch %g inch moveto\n",
			base_x + v[0].x * factor_x,
			base_y + v[0].y * factor_y);
		fprintf(output, "\t%g inch %g inch moveto\n",
			base_x + v[0].x * factor_x, base_y);
		for (i = 1; i < data_points; i++) {
			double

			 
				x = base_x + v[i].x * factor_x,
				y = base_y + v[i].y * factor_y;
			fprintf(output, "\t%g inch %g inch lineto\n", x, y);
		}
		fprintf(output, "\t%g inch %g inch lineto\n",
			base_x + v[data_points - 1].x * factor_x, base_y);
		fprintf(output,
			"closepath gsave 0.75 setgray fill grestore stroke\n");
		fprintf(output, "\tstroke\n");
		fprintf(output, "3 setlinewidth\n");
		draw_line(output, base_x, base_y - 0.1, base_x,
			  base_y + GRAPH_HEIGHT + 0.1);
		draw_line(output, base_x - 0.1, base_y,
			  base_x + GRAPH_WIDTH + 0.1, base_y);
		fprintf(output, "1 setlinewidth\n");
		for (i = 1; i <= 10; i++) {
			char buffer[20];

			sprintf(buffer, "%g", (double)i * max_x / 10.0);
			draw_line(output,
				  base_x +
				  factor_x * (max_x / 10.0) * (double)i,
				  base_y,
				  base_x +
				  factor_x * (max_x / 10.0) * (double)i,
				  base_y - 0.1);
			print_centered(output, buffer,
				       base_x +
				       factor_x * (max_x / 10.0) * (double)i,
				       base_y - 0.25, 10);
		}
		for (i = 1; i <= 4; i++) {
			char buffer[20];
			double y =
				base_y + factor_y * (max_y / 4.0) * (double)i;
			sprintf(buffer, "%g", (double)i * max_y / 4.0);
			draw_line(output, base_x - 0.1, y,
				  base_x + GRAPH_WIDTH, y);
			print_right(output, buffer, base_x - 0.25, y, 10);
		}
	}

	sprintf(titles, "%s     %s", header[2], header[3]);
	sprintf(totals, "Peak = %.1f   Sustained = %.1f    Minimum JTI = %.0f",
		peak, sustain, minjti);

	print_centered(output, header[0], 4.5, 10.25, 24);
	print_centered(output, titles, 4.5, 10.0, 16);
	print_centered(output, totals, 4.5, 9.75, 12);	/*  add peak, sustained, jti */
	print_centered(output, "Loads", 4.5, BOTTOM_SPACE - 0.5, 16);
	print_centered(output,
		       "Copyright (c) 1996 - 2001 Caldera International, Inc.",
		       4.0, 0.50, 8);
	print_centered(output, "All Rights Reserved.", 5.8, 0.50, 8);
	print_centered(output, "These results are not certified.", 4.5, 0.35,
		       8);
	fprintf(output, "showpage\n");
	fclose(output);
	exit(0);
}

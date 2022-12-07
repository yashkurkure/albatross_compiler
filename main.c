#include <stdio.h>
#include <stdlib.h>
#include "ast.h"
#include "symbol.h"
#include "y.tab.h"
#include "semantic_analysis_symbols.h"
#include "semantic_analysis_types.h"
#include "frames.h"
#include "transform.h"
#include "mips_ast.h"

extern FILE *yyin;
extern int lexer();

program p;

static void * g1, *g2, *g3;

int main(int argc, char **argv) {
 char * fname;
 char * outname;
 if (argc!=3) {fprintf(stderr,"usage: albatrosscc <in.albatross> <out.mips>\n"); return 1;}

 fname=argv[1];
 outname=argv[2];

 p.variables = NULL;
 p.functions = NULL;
 p.statements = NULL;

 yyin = fopen(fname,"r");
 if (!yyin) { fprintf(stderr, "cannot open %s\n", fname); return 1; }


 int ret = yyparse();

 // Map from S_Symbol to ty_node
 S_table globals_types = S_empty();

 // Map from S_Symbol to ty_node
 S_table functions_ret_types = S_empty();

 // Map from S_Symbol to fun_frame
 S_table functions_frames = S_empty();

 // symbol analysis
 symbolResolution(&p, globals_types, functions_ret_types, functions_frames);

 // type checking
 typeCheck(&p, globals_types, functions_ret_types, functions_frames);

 // compute frames
 frames(&p, globals_types, functions_ret_types, functions_frames);

 // transform AST as needed
 transform(&p, globals_types, functions_ret_types, functions_frames);

 // generate assembly
 FILE * out = fopen(outname, "w");
 mips_ast(&p, globals_types, functions_ret_types, functions_frames, out);
 fclose(out);

 // The library that defines the symbol tables does not provide a destructor to free all memory
 // HACK: Let's put our variables on globals so the leak sanitizer is happy most of the time
 g1 = globals_types;
 g2 = functions_ret_types;
 g3 = functions_frames;


 return ret;
}


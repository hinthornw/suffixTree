# suffixTree
My C++ implementation of a generalized suffix tree with longest overlap search (in progress). Additionally includes a sample set of sequences from the Enterobacteria phage lambda genome. ph is used to parse the FASTA file, find the errors in the sequence (in this case inserted from a bacterium sequence) and 

Utilized Ukkonen's algorithm for suffix trees and a variant of the stable marriage problem to connect the mixed up gene sequences.

Uses g++ to build. 

Commands:

prompt> make ph
prompt> ./ph

Prints out the test script, timing, etc. Takes about 5 minutes in its current form to complete.

# suffixTree
My C++ implementation of a generalized suffix tree with longest overlap search (in progress). Additionally includes a sample set of sequences from the Enterobacteria phage lambda genome. ph is used to parse the FASTA file, find the errors in the sequence (in this case inserted from a bacterium sequence) and 

Utilizes Ukkonen's algorithm (Referenced and improved (removed memory links, fixed some logic) Anurag Singh's <a href="http://www.geeksforgeeks.org/ukkonens-suffix-tree-construction-part-1/">algorithms</a>) for suffix trees and a variant of the <a href="https://en.wikipedia.org/wiki/Stable_marriage_problem">stable marriage problem</a> to connect the mixed up gene sequences.

Thank you to <a href= "http://phosphorus.com">Phosphorous</a> for access to the data.

Uses g++ to build. 

Commands:


    $>make ph
    $>./ph
    Parsing lambda_scramble.fa 
    ...

Prints out the test script, timing, etc. Takes about 5 minutes on my machine in its current form to complete.

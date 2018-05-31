# My Tally
__Description__: mini application used for counting election votes and exporting result graphs and statistics.

### Compilation
There is a Makefile available for automatic compilation:
```sh
$ make
```
After compilation several executables are being created. The executable you should run is `mytally`.
### Execution
The program can be executed in the following way:
```sh
$./mytally -i <input_file> -o <output_file> -l <numOfSMs> -d <depth> -p <percentile>
```
* __input_file__: (necessary) file containing votes in the following format: `candidate_name election_center_code validity`.
* __output_file__: (optional) file where results will be written.
* __numOfSMs__: (necessary) number of SplitterMerger or Sorter processes to be forked in each level.
* __depth__: (necessary) defines the number of levels to be created in the process tree.
* __percentile__:(optional) the percentile of to be used to print top results.

### Processes created
While running the program creates `depth` levels of processes. Level 0 is consisted of `root` process. `root` creates `numOfSMs` `SplitterMerger` processes. Each one of them creates `numOfSMs` `SplitterMerger` processes and so on until we reach the last level where `Sorter` processes are being forked. So the process tree looks like this:
```sh
depth=3
lv0     lv1     lv2     lv3
root
│   
└───SplitterMerger1
│       │ 
│       └───SplitterMerger1.1
│       │           Sorter1.1.1
│       │           Sorter1.1.2
│       │           Sorter1.1.3
│       └───SplitterMerger1.2
│       │           Sorter1.2.1
│       │           Sorter1.2.2
│       │           Sorter1.2.3
│       └───SplitterMerger1.3
│                   Sorter1.3.1
│                   Sorter1.3.2
│                   Sorter1.3.3
└───SplitterMerger2
│       │ 
│       └───SplitterMerger2.1
│       │           Sorter2.1.1
│       │           Sorter2.1.2
│       │           Sorter2.1.3
│       └───SplitterMerger2.2
│       │           Sorter2.2.1
│       │           Sorter2.2.2
│       │           Sorter2.2.3
│       └───SplitterMe2ger2.3
│                   Sorter2.3.1
│                   Sorter2.3.2
│                   Sorter2.3.3
└───SplitterMerger3
        │ 
        └───SplitterMerger3.1
        │           Sorter3.1.1
        │           Sorter3.1.2
        │           Sorter3.1.3
        └───SplitterMerger3.2
        │           Sorter3.2.1
        │           Sorter3.2.2
        │           Sorter3.2.3
        └───SplitterMe2ger3.3
                    Sorter3.3.1
                    Sorter3.3.2
                    Sorter3.3.3

```                  
### Outputs
After exiting the program creates `results.jpg` and `resultspercenter.jpg` files with result graphs, created by gnuplot. A gnuplot script is also generated with name `plot1.p`. Finally, files `ResultsPerCandidate.txt` and `ResultsPerElectionCenter.txt` are being created, containing the results per Candidate or Election Center.
### Input File
There is an input file available with name `textfile` containing sample votes.

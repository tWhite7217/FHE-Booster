# FHE-Booster

FHE-Booster is a framework for reducing the execution time of fully homomorphic encryption programs via smart bootstrapping placement. While fully functional in its current form, FHE-Booster is still a work in progress. This repository is meant to host the framework as it existed upon submission of my thesis. Any commits to master moving forward will be accompanied by an update to this README. Such updates may not happen at all and are likely to only be bug fixes, not new features. A new repository will be created for the updated version of FHE-Booster.

### How to cite this work
The initial paper describing our framework can be accessed [here](https://doi.org/10.1109/HOST55118.2023.10132930); you can cite it as follows:
```
@INPROCEEDINGS{WGYT_FHEBooster_HOST23,
  author={White, Tommy and Gouert, Charles and Yang, Chengmo and Tsoutsos, Nektarios Georgios},
  booktitle={2023 IEEE International Symposium on Hardware Oriented Security and Trust (HOST)}, 
  title={FHE-Booster: Accelerating Fully Homomorphic Execution with Fine-tuned Bootstrapping Scheduling}, 
  year={2023},
  volume={},
  number={},
  pages={293-303},
  doi={10.1109/HOST55118.2023.10132930}
}
```

The more detailed and up-to-date thesis is [here](https://github.com/tWhite7217/FHE-Booster-Thesis); you can cite it as follows:
```
@mastersthesis{White_FHE_Thesis_2023,
  author  = "Tommy White",
  title   = "Scheduling General Purpose Encrypted Computation on Multicore Platform",
  school  = "University of Delaware",
  year    = "2023",
  note    = "Available at \url{https://github.com/tWhite7217/FHE-Booster-Thesis}"
}
```

## Installation

FHE-Booster was built on a Windows machine mostly using Windows Subsystem for Linux (WSL). Although untested, the framework should work on a Windows (w/ or w/o WSL), Linux, or Mac device. There are a number of bash scripts, but they are not necessary for using FHE-Booster. 

The dependencies are a C++ compiler capable of compiling C++20 code (we use both g++-10 and clang++-11), LINDO Lingo Software (if you would like to use the provided integer programming models), flexc++, make, and cmake.

Once the dependencies are installed and the repository has been cloned, running the following commands in the top directory will build the most important tools of FHE-Booster.

```
make all
export CXX=clang++-11
cmake -S CPP_code/execution_engine/ -B CPP_code/execution_engine/build/
make -C CPP_code/execution_engine/build/
```

## Usage

### Creating an FHE task graph

In order to use FHE-Booster, you must have an FHE program on which to perform optimizations. Since all FHE programs can take the form of a directed acyclic task graph, we use the following format. This format is not optimized and is likely to change in the future.

```
1,SET
2,SET
3,SET
~
1,ADD,k1,k2
2,SUB,c1,k3
3,MUL,k2,c2
4,ADD,c3
```

The first section list the input ciphertexts. The first must have an ID of 1, and each following ciphertext should increment the previous ID by one.

The second sections lists the operations, again starting from 1 and incrementing by one with each operation. On each line, after the ID, the operation type is listed. The supported operations are ADD, SUB, and MUL. Next, the inputs are listed. There can be either one or two. An input starting with a 'k' is referring to the initial inputs listed in the first section, while an input starting with a 'c' refers to the output of a previous operation. Note that the operations must be listed in topological order, so a 'c' input cannot have a higher ID than the operation being described.

Writing such a file by hand could be tedious. We have developed some tools to partially automate the process. Specifically the dag_generator.cppoe
rvol /h files in the CPP_code/benchmark_generation directory. We hope to replace these tools with fully automated compilation in the future.

Furthermore, random graphs can be generated using the random_graph_generator.cpp/h files in the CPP_code directory.

### Framework Steps

Now that you have a FHE task graph, the following steps can be performed.

1. Generate "bootstrap segments" using CPP_code/bootstrap_segments_generator.out
2. Create a bootstrap set in one of the following ways.
   a. Use the FHE_Model_min_bootstrapping.lng model using LINGO.
   b. Use the score-based method with CPP_code/boostrap_set_selector.out
3. Optionally convert the generated bootstrap sets to the selective forwarding sets using CPP_code/complete_to_selective_converter.out (recommended)
4. Create a schedule from the FHE task graph and some bootstrap set using CPP_code/list_scheduler.out
5. Run the generated schedule with CPP_code/execution_engine/build/execution_engine

Creating more detailed/helpful instructions here will be a consideration for future work. For now, some of these binaries have their own documentation inside the associated .h or .hpp file.

## Future Work

Although it can be used as is and some may find it useful, FHE-Booster is currently more of a proof of concept than a fully-featured production-ready framework. Therefore, the following features are intended for future implementation, in some FHE-Booster 2.0.
<!-- that will be hosted on a separate repository. -->

- Dynamic (rather than static) placement of bootstrap operations, which should greatly increase the compatability of FHE-Booster, since it is currently limited to smaller programs.
- An updated input file format, in preparation for a dynamic approach.
- Compilation from a higher-level language.
- Optimizations beyond bootstrap placement, likely as part of the compilation step.
- In-depth documentation.
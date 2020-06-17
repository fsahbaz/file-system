# COMP 304 Project 3: Space Allocation Methods

## Student Name: Furkan Sahbaz

## Student ID: 60124

C++ programs to simulate file systems that utilize contiguous and linked allocation methods were implemented for this project. All parts work according to the given instructions.

Each program takes the I/O file (file that contain the sequence of operations to be performed) as an input after running. Example executions for each program are done as following:

```bash
make contiguous
./contiguous.out
File name: input_x_x_x_x_x.txt
```

```bash
make linked
./linked.out
File name: input_x_x_x_x_x.txt
```

which will run the simulation for the given block size, i.e. the number right after "input" in the text file names. The block size information is extracted from each input file name, where it is directly utilized in each program. Both programs continue by parsing the given I/O files, and performing each operation right after it is parsed from the corresponding line. After the operations are complete, the simulation duration along with information on the successful and failed file creations/extensions are printed. Note that the input files should be in the same directory as the program outputs.

## Contiguous Allocation

In contiguous allocation, each creation operation is done by the first fit algorithm, in which files are placed in the first sufficient contiguous space (either a hole or at the end of already-allocated files). Initially, after the file information is extracted (file ID, file size), the method looks for a space to place the file. When there is enough space in the directory, but there isn't any hole in the file-to-be-allocated's size due to external fragmentation, the directory is defragmented by compaction (each file is shifted to lower indices). After a hole of the correct size is found, or compaction has been done, the file is added and the directory table is updated with the file ID and the starting index of the file. Similarly file extensions are directly done if enough contiguous space has been found after the file; however, if there isn't enough space after the end of the file (but there's enough space for the extension amount), the directory is defragmented again. The amount of space left after the end of file was considered in this part; if the extension amount exceeds the end of the directory after appending to the end of the file, the directory is compacted to the lower indices first. Each empty block is then brought to indices after the updated end of the file. If the appended file's content does not exceed the directory size, the latter case is directly performed. After a sufficiently large hole is opened after the end of the file, the extension contents are added one by one, and (if there are any) the upward-shifted files' starting indices are updated. Access and shrink operations are done rather simpler in this method, as they require calculation of the indices to return, or the indices until which the content is to be deleted. The files are shrunk until there's 1 block left, if the shrinking amount is greater than the file size in blocks.

## Linked Allocation

In linked allocation, a file allocation table (FAT), that simply contains the index of the next block for each block in the directory, was utilized along with the directory and directory table. In order to store the FAT in the directory, 4 bytes for each block was reserved since each block will have a FAT entry, and the number of available blocks is updated accordingly. This time, files are placed in the directory in a way that, file contents are placed in the first empty block observed in the directory; there's no need to be contiguous. After each placement, the next empty block is found and the FAT is updated accordingly. Extension is done similarly, where the first empty block that is observed in the directory is utilized, and the next block information is updated accordingly in the FAT. On the other hand, this method requires more work in terms of shrinking a file and accessing a given offset, In order to shrink a file, starting from its starting index, each block is iterated until there's enough blocks to delete for the given amount, until the end of the file. After the required point is achieved, each block is deleted until the end of the file is reached. Again, if the given amount is greater than the file size, the file is shrunk until there's only 1 block left. Accessing is done similarly as well, where each block is iterated, starting from the file's starting index, until the desired offset is reached.

### Performance Related Questions

Below table demonstrates the performance of each allocations method with various I/O files that are aimed for different block sizes, and contain different numbers of create, extend, shrink, and access operations:

Note: The simulation times were calculated by averaging 5 consecutive runs for each input and allocation type.

```bash
Case 1: input_8_600_5_5_0.txt (mostly creation extension, and access operations, no shrinking)

| Allocation Method | Simulation Time (ms) | Successful Creations | Failed Creations | Successful Extensions | Failed Extensions |
__________________________________________________________________________________________________________________________________
|    Contiguous     |	       1515        |	        385       |        215	     |          354          |         201       |
|    Linked         |	    	 20        |	        204       |        396       |          193          |         362       |

Case 2: input_1024_200_5_9_9.txt (mostly creation, shrinking, and extension operations, fewer access operations)

| Allocation Method | Simulation Time (ms) | Successful Creations | Failed Creations | Successful Extensions | Failed Extensions |
__________________________________________________________________________________________________________________________________
|    Contiguous     |	       148644      |	        186       |        14	    |          1225          |         171       |
|    Linked         |	    	 58        |	        190       |        10       |          1240          |         156       |

Case 3: input_1024_200_9_0_0.txt (only creation and access operations, no shrinking or extension)

| Allocation Method | Simulation Time (ms) | Successful Creations | Failed Creations | Successful Extensions | Failed Extensions |
__________________________________________________________________________________________________________________________________
|    Contiguous     |	      995          |	        120       |        80	     |          0            |         0         |
|    Linked         |	      1222         |	        121       |        79        |          0            |         0         |

Case 4: input_1024_200_9_0_9.txt (mostly access and shrink operations after creation, no extensions)

| Allocation Method | Simulation Time (ms) | Successful Creations | Failed Creations | Successful Extensions | Failed Extensions |
__________________________________________________________________________________________________________________________________
|    Contiguous     |	       14          |	        200       |        0	     |          0            |         0         |
|    Linked         |	       15          |	        200       |        0         |          0            |         0         |

Case 5: input_2048_600_5_5_0.txt (mostly creation extension, and access operations, no shrinking)

| Allocation Method | Simulation Time (ms) | Successful Creations | Failed Creations | Successful Extensions | Failed Extensions |
__________________________________________________________________________________________________________________________________
|    Contiguous     |	       1318        |	        387       |        213	     |          354          |         181       |
|    Linked         |	    	 33        |	        387       |        213       |          357          |         178       |

```
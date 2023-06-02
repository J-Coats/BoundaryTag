# BoundaryTag
Implements the contiguous memory mangagement scheme using the boundary-tag method.

A free block contains the following fields. Each field is an (integer) element of the array that is being managed.

    Size field on both ends of the block. The size indicates the number of bytes in the block, including the space used for bookkeeping.
    A link to the next free block (Next Link).
    A link to the previous free block (Prev Link).

-Size |	Prev Link |	Next Link |	Unused memory |	-Size

There is a linked list of free blocks from which you allocate memory. As the user frees the memory that has allocated, you need to make sure to coalesce the adjacent free blocks. This is why you need a size field on either side of each block.

An allocated block contains only the size fields.

Size |	Actual bytes being allocated |	Size


In order to test: 
run make
./boundaryTagApp.x

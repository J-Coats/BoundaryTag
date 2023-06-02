/**
 * Author: Jared Coats
 * Assignment: Project 3 -- Boundary Tag Memory Management
 * Class: CS 450
 * Date: 04/21/2023
 *
 */


#include "BoundaryTag.hpp"
#include <cstdio>
#include <cstdlib>



BoundaryTag::BoundaryTag() {

    // Initialize the free index to the beginning of the memory array
    freeIdx = 0;

    // Set up the first memory block as free

    memory[0] = -SIZE * BYTES_PER_WORD;
    memory[1] = -1;
    memory[2] = SIZE;
    memory[SIZE - 1] = -SIZE * BYTES_PER_WORD;
}

void BoundaryTag::start() {
    iterIdx = 0;
}

void* BoundaryTag::next() {

    // make sure iterIdx is null if it goes out of bounds
    if (iterIdx >= SIZE) {
        return nullptr;
    }

    int *ptr =  &memory[iterIdx];
    iterIdx += abs(*ptr) / BYTES_PER_WORD;

    if (*ptr < 0)
        return ptr + 3;
    return ptr + 1;
}

int BoundaryTag::size(void* ptr) {
    // Return the size of the block

    int * temp = static_cast<int *>(ptr);
    if (isFree(ptr)) {
        return abs(temp[-3]);
    }

    return abs(temp[-1]);
}

bool BoundaryTag::isFree(void* ptrToMem) {
    // Returns true if block is free, false if allocated

    int * addressOfCurrentFreeBlock = &memory[freeIdx];


    while (addressOfCurrentFreeBlock != 0)  {
        int sizeOfCurrentBlock = abs(*addressOfCurrentFreeBlock) / BYTES_PER_WORD;
        int * endAddressOfCurrentFreeBlock = addressOfCurrentFreeBlock + sizeOfCurrentBlock;
        if (addressOfCurrentFreeBlock < ptrToMem && ptrToMem < endAddressOfCurrentFreeBlock)
            return true;

        else {
            if (addressOfCurrentFreeBlock[2] == SIZE)
                addressOfCurrentFreeBlock = nullptr;
            else{
                addressOfCurrentFreeBlock = &memory[addressOfCurrentFreeBlock[2]];
            }

        }
    }

    return false;
}


void* BoundaryTag::allocate( int numBytes ) {

    if (freeIdx >= SIZE)
        return nullptr;


    int remainder = numBytes % BYTES_PER_WORD;

    if (remainder != 0) {
        numBytes += BYTES_PER_WORD - remainder;
    }
    numBytes += BYTES_PER_WORD * 2;

    // if it at this point numBytes is smaller than 16, we must set it to 16.
    // This will be the case if numBytes is passed a value of 4 or below.
    if (numBytes < BYTES_PER_WORD * FREE_OVERHEAD)
        numBytes = BYTES_PER_WORD * FREE_OVERHEAD;

    int *ptr =  &memory[freeIdx];    // point to the first free block

    bool found = false;
    bool isFirstFreeBlock = true; //used for setting freeIdx later

    while (!found && ptr != 0)  {

        if (abs(*ptr) >= numBytes) {
            // we found a block that is sufficient space for numBytes
            found = true;
        }
        else {
            // else follow the next pointer of the current free block
            if (ptr[2] == SIZE)
                ptr = nullptr;
            else{
                ptr = &memory[ptr[2]];
                isFirstFreeBlock = false;
            }

        }
    }

    // if there was no free blocks found
    if (!found)
        return nullptr;


    int sizeOfFreeBlock = abs(ptr[0]);
    int remainingSize = sizeOfFreeBlock - numBytes;

    // check if left over-size, if any, is big enough to cut or not
    if (remainingSize <= BYTES_PER_WORD * FREE_OVERHEAD) {
        numBytes += remainingSize;
        remainingSize = 0;
    }

    int numInts = numBytes / BYTES_PER_WORD;
    int sizeOfFreeInInteger = sizeOfFreeBlock / BYTES_PER_WORD;

    int prev = ptr[1];
    int next = ptr[2];
    ptr[0] = numBytes;
    ptr[numInts - 1] = numBytes;

    if (remainingSize > 0) {
        // performs the cutting of the block with the remainder space
        ptr[numInts] = -remainingSize;
        ptr[sizeOfFreeInInteger - 1] = -remainingSize;
        ptr[numInts + 1] = prev;
        ptr[numInts + 2] = next;

        // we have to rearrange the previous pointers next, and the next pointers previous
        if (prev != -1)
            memory[prev + 2] = memory[prev + 2] + numInts;
        if (next != SIZE)
            memory[next + 1] = memory[next + 1] + numInts;

        if (isFirstFreeBlock) {
            freeIdx += numInts;
        }
    }
    else {

        int * prevPtr;
        int * nextPtr;
        if (ptr[1] == -1 ) {
            prevPtr = nullptr;
        }
        else{
            prevPtr = &memory[ptr[1]];
        }
        if (ptr[2] == SIZE)
            nextPtr = nullptr;
        else
            nextPtr = &memory[ptr[2]];

        if (prevPtr != nullptr)
            prevPtr[2] = ptr[2];
        if (nextPtr != nullptr)
            nextPtr[1] = ptr[1];

        if (isFirstFreeBlock) {
            freeIdx = ptr[2];
        }
    }

    return ptr + 1;


}

void BoundaryTag::free(void* ptrToMem) {
    // Given a space in memory, free the allocated memory at the location.
    // Four cases for combining adjacent free blocks after freeing: surrounded by no open memory blocks
    // one on the left, one on the right, or both

    bool leftFree = false, rightFree = false;
    int rightBlockSize, leftBlockSize;

    int * castedPtrToMem = static_cast<int *> (ptrToMem ) - 1;

    int sizeOfAllocatedBlock = abs(*castedPtrToMem);
    int integerSizeOfAllocatedBlock = sizeOfAllocatedBlock / BYTES_PER_WORD;
    int * addressOfStart = &memory[0];
    int * addressOfEnd = &memory[SIZE - 1];

    // check to see if the block to the left exists and if it is free or not
    if (addressOfStart < castedPtrToMem) {
        leftBlockSize = castedPtrToMem[-1];
        if (leftBlockSize < 0)
            leftFree = true;
    }

    // do the same thing for the block to the right
    if (addressOfEnd > &castedPtrToMem[integerSizeOfAllocatedBlock - 1]) {
        rightBlockSize = castedPtrToMem[integerSizeOfAllocatedBlock];
        if (rightBlockSize < 0)
            rightFree = true;
    }

    // set index of the current block we're trying to free
    int indexOfCurrent = castedPtrToMem - addressOfStart;

    //  -- Now start the four cases --

    // Case 1: no surrounding free blocks
    if (!leftFree && !rightFree){

        castedPtrToMem[1] = -1;
        castedPtrToMem[0] = -sizeOfAllocatedBlock;
        castedPtrToMem[integerSizeOfAllocatedBlock - 1] = -sizeOfAllocatedBlock;
        castedPtrToMem[2] = freeIdx;

        if (freeIdx <= SIZE){
            memory[freeIdx + 1] = indexOfCurrent;
        }

        freeIdx = indexOfCurrent;

    }


    // Case 2: Only the right Block is free
    if (!leftFree && rightFree) {

        int prev = castedPtrToMem[integerSizeOfAllocatedBlock + 1];
        int next = castedPtrToMem[integerSizeOfAllocatedBlock + 2];

        int totalSize = sizeOfAllocatedBlock + abs(rightBlockSize);
        castedPtrToMem[0] = -totalSize;
        castedPtrToMem[totalSize / BYTES_PER_WORD - 1 ] = -totalSize;

        // have to change the previous' next pointer and the next's previous pointer
        if (prev != -1)
            memory[prev + 2] = memory[prev + 2] - integerSizeOfAllocatedBlock;
        if (next != SIZE)
            memory[next + 1] = memory[next + 1] - integerSizeOfAllocatedBlock;

        // set the previous and the next pointers
        castedPtrToMem[1] = castedPtrToMem[integerSizeOfAllocatedBlock + 1];
        castedPtrToMem[2] = castedPtrToMem[integerSizeOfAllocatedBlock + 2];

        int indexOfRightBlock = indexOfCurrent + integerSizeOfAllocatedBlock;
        if (freeIdx == indexOfRightBlock) {
            freeIdx = indexOfCurrent;
        }


    }


    // Case 3: Only the left block is free
    if (leftFree && !rightFree) {

        // we simply need to update the size tags in this case and we just
        // keep the existing pointers for prev and next of the left block
        int totalSize = sizeOfAllocatedBlock + abs(leftBlockSize);
        castedPtrToMem[leftBlockSize / BYTES_PER_WORD] = -totalSize;
        castedPtrToMem[integerSizeOfAllocatedBlock - 1] = -totalSize;

    }

    // Case 4: Both sides are free
    if (leftFree && rightFree) {

        int totalSize = sizeOfAllocatedBlock + abs(leftBlockSize) + abs(rightBlockSize);

        // set the size tag of the left block to the total size of all three blocks
        castedPtrToMem[leftBlockSize / BYTES_PER_WORD] = -totalSize;
        // set the end size tag of all three blocks to the total size now
        castedPtrToMem[(sizeOfAllocatedBlock + abs(rightBlockSize)) / BYTES_PER_WORD - 1] = -totalSize;

        // go through updating all the pointers
        int previousOfRightBlock = castedPtrToMem[integerSizeOfAllocatedBlock + 1];
        int nextOfRightBlock = castedPtrToMem[integerSizeOfAllocatedBlock + 2];

        if (previousOfRightBlock != -1){
            memory[previousOfRightBlock + 2] = nextOfRightBlock;
            memory[nextOfRightBlock + 1] = previousOfRightBlock;
        }


        else {
            freeIdx = nextOfRightBlock;
            memory[freeIdx + 1] = -1;
        }


    }


}



#include "Dx12Private.h"

#if GPU_DX12

GPU_Dx12DescriptorHeap* GPU_CreateDx12DescriptorHeap(ID3D12Device2* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, u32 capacity, MEM_Allocator allocator)
{
    MSR_ASSERT(capacity && "capacity must be non-zero");

    static_assert(sizeof(GPU_Dx12DescriptorHeap) % alignof(GPU_Dx12DescriptorHeap) == 0, "GPU_Dx12DescriptorHeap must be properly aligned");
    static_assert(alignof(GPU_Dx12DescriptorHeap) >= alignof(u32), "GPU_Dx12DescriptorHeap must be aligned to at least the alignment of u32");

    u32 allowedHoleCount = (capacity / 2) + 1; // allow for half the capacity to be holes, plus one extra for safety
    size_t toAllocate = sizeof(GPU_Dx12DescriptorHeap) + (allowedHoleCount * sizeof(u32));

    rawptr allocated = MEM_Allocate(allocator, true, toAllocate, alignof(GPU_Dx12DescriptorHeap));
    MSR_ASSERT(allocated && "Failed to allocate memory for GPU_Dx12DescriptorHeap");

    GPU_Dx12DescriptorHeap* output = (GPU_Dx12DescriptorHeap*) allocated;
    u32* holesBuffer = (u32*) ((u8*) allocated + sizeof(GPU_Dx12DescriptorHeap));
    *output = GPU_Dx12DescriptorHeap
    {
        .mutex     = SYN_CreateMutex(),
        .allocator = allocator,
        .actual    = nil,
        .type      = heapType,
        .capacity  = capacity,
        .count     = 0,
        .stride    = device->GetDescriptorHandleIncrementSize(heapType),
        .holes     =
        {
            .data      = holesBuffer,
            .count     = 0,
            .capacity  = allowedHoleCount,
            .allocator = {.procedure = nil, .data = nil},
        },
    };

    D3D12_DESCRIPTOR_HEAP_DESC heapDesc =
    {
        .Type           = heapType,
        .NumDescriptors = capacity,
        .Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
    };

    GPU_DX12_CHECKED_CALL(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&(output->actual))));

    return output;
}

void GPU_DestroyDx12DescriptorHeap(GPU_Dx12DescriptorHeap* heap)
{
    if (!heap)
        return;

    SYN_LockMutex(&(heap->mutex));
    {
        COL_ClearList(&(heap->holes));
        if (heap->actual)
        {
            heap->actual->Release();
            heap->actual = nil;
        }
    }
    SYN_UnlockMutex(&(heap->mutex));

    SYN_DestroyMutex(&(heap->mutex));
    heap->mutex = SYN_Mutex { };

    MEM_Deallocate(heap->allocator, heap);
}

GPU_Dx12DescriptorData GPU_AllocateFromDx12DescriptorHeap(GPU_Dx12DescriptorHeap* heap)
{
    GPU_Dx12DescriptorData output = {0};

    SYN_LockMutex(&(heap->mutex));
    {
        u32 idx;

        // allocate from one of the holes if available
        // otherwise, allocate a new fresh index
        if (heap->holes.count > 0) idx = heap->holes.data[--heap->holes.count];
        else if (heap->count < heap->capacity) idx = heap->count++;
        else
        {
            MSR_ASSERT(false && "No more descriptors available in the heap");
            SYN_UnlockMutex(&(heap->mutex));
            return output;
        }

        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = heap->actual->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += idx * heap->stride;

        output = GPU_Dx12DescriptorData
        {
            .owningHeap = heap,
            .index      = idx,
            .cpuHandle  = cpuHandle,
        };
    }
    SYN_UnlockMutex(&(heap->mutex));

    return output;
}

void GPU_DestroyDx12DescriptorData(GPU_Dx12DescriptorData* data)
{
    if (!data || !data->owningHeap)
        return;

    GPU_Dx12DescriptorHeap* heap = data->owningHeap;

    SYN_LockMutex(&(heap->mutex));
    {
        MSR_ASSERT(data->index < heap->capacity && "descriptor index out of bounds");

        // if the index is the last one allocated, we can just decrement the count
        // otherwise, we add the index to the holes list for reuse
        if (data->index == heap->count - 1) heap->count--;
        else  COL_AppendToList(&(heap->holes), data->index);

        // now we will iterate the holes list multiple times to remove any holes that are
        // at the end of the heap, since we can just decrement the count for those
        b8 somethingRemoved = false;
        do
        {
            somethingRemoved = false;
            // go in reverse to avoid issues with shifting indices
            for (isize i = heap->holes.count - 1; i >= 0; i--)
            {
                if (heap->holes.data[i] == heap->count - 1)
                {
                    COL_RemoveIdxFromList(&(heap->holes), i);
                    heap->count--;
                    somethingRemoved = true;
                }
            }
        } while (somethingRemoved);
    }
    SYN_UnlockMutex(&(heap->mutex));

    *data = GPU_Dx12DescriptorData { };
}

#endif

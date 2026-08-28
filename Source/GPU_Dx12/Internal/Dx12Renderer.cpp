#include "Dx12Private.h"

#if GPU_DX12

void GPU_Dx12Create(GPU_Instance* outBaseInstance, GPU_InstanceCfg cfg)
{
    MSR_ASSERT(!!outBaseInstance && "outBaseInstance can't be null");
    outBaseInstance->base.type = GPU_GfxAPIType_Dx12;

    GPU_Dx12Instance* output = GPU_ToDx12Instance(outBaseInstance);
    MSR_ASSERT(output && "output must not be null");

    output->appHandle = cfg.appHandle;

    // app name
    {
        utf8str nameStrToUse = cfg.appName;
        if (sizeof(output->buffers.appName) < (usize) cfg.appName.count)
            nameStrToUse = STR_SubString(cfg.appName, 0, sizeof(output->buffers.appName));

        MEM_Copy(output->buffers.appName, nameStrToUse.data, (usize) nameStrToUse.count);
        output->appName.data = output->buffers.appName;
        output->appName.count = nameStrToUse.count;
    }

    // debug layer
    u32 dxgiFactoryFlags = 0;
    if (MSR_DBG)
    {
        GPU_DX12_CHECKED_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&(output->dbgController))));
        output->dbgController->EnableDebugLayer();

        ID3D12Debug1* dbgController1 = nil;
        if (SUCCEEDED(output->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController1))))
        {
            dbgController1->SetEnableGPUBasedValidation(true);
            dbgController1->Release();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    // factory
    GPU_DX12_CHECKED_CALL(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&(output->dxgiFactory))));

    // adapter
    {
        IDXGIAdapter1* hwAdapter = nil;
        for (u32 i = 0; output->dxgiFactory->EnumAdapters1(i, &hwAdapter) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC1 desc;
            GPU_DX12_CHECKED_CALL(hwAdapter->GetDesc1(&desc));

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                hwAdapter->Release();
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(hwAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device2), nil)))
            {
                output->adapter = hwAdapter;
                break;
            }

            hwAdapter->Release();
        }
    }
    MSR_ASSERT(output->adapter && "No compatible DirectX 12 adapter found");

    // device
    GPU_DX12_CHECKED_CALL(D3D12CreateDevice(output->adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&(output->device))));
    GPU_DX12_SET_OBJ_DEBUG_NAME(output, output->device, "%.device", FMT(output->appName));

    // info queue setup
    {
        ID3D12InfoQueue* infoQueue = nil;
        if (SUCCEEDED(output->device->QueryInterface(IID_PPV_ARGS(&infoQueue))))
        {
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
            infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);

            // hide some annoying warnings, like clearing render target with a different value than initialised from
            // the day that clearing a render target with a different colour than the "optimised" one becomes my bottleneck,
            // i'll leave game development forever and get a boring lifeless finance job or something

            D3D12_MESSAGE_ID hide[] =
            {
                D3D12_MESSAGE_ID_CREATEDEVICE_DEBUG_LAYER_STARTUP_OPTIONS,
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            };

            D3D12_INFO_QUEUE_FILTER filter =
            {
                .DenyList =
                {
                    .NumIDs = (u32) (sizeof(hide) / sizeof(hide[0])),
                    .pIDList = hide,
                },
            };
            GPU_DX12_CHECKED_CALL(infoQueue->AddStorageFilterEntries(&filter));

            ID3D12InfoQueue1* iq1 = nil;
            if (SUCCEEDED(output->device->QueryInterface(IID_PPV_ARGS(&iq1))))
            {
                DWORD cookie = 0;
                GPU_DX12_CHECKED_CALL(iq1->RegisterMessageCallback(GPU_GetDx12DebugCallback(), D3D12_MESSAGE_CALLBACK_FLAG_NONE, nil, &cookie));
                output->dbgCallbackCookie = cookie;

                iq1->Release();
            }

            infoQueue->Release();
        }
    }

    // queue
    {
        D3D12_COMMAND_QUEUE_DESC queueDesc =
        {
            .Type     = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
        };
        GPU_DX12_CHECKED_CALL(output->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&(output->cmdQueue))));
    }
    GPU_DX12_SET_OBJ_DEBUG_NAME(output, output->cmdQueue, "%.maincmdqueue", FMT(output->appName));

    // allocator
    {
        D3D12MA::ALLOCATOR_DESC allocDesc =
        {
            .pDevice  = output->device,
            .pAdapter = output->adapter,
        };
        GPU_DX12_CHECKED_CALL(D3D12MA::CreateAllocator(&allocDesc, &(output->d3d12maAllocator)));
    }
}

void GPU_Dx12WaitTillIdle(GPU_Instance* baseRenderer)
{
    const GPU_Dx12Instance* renderer = GPU_ToDx12Instance(baseRenderer);

    if (!renderer || !(renderer->device) || !(renderer->cmdQueue))
        return;

    ID3D12Fence* fence = nil;
    GPU_DX12_CHECKED_CALL(renderer->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
    GPU_DX12_CHECKED_CALL(renderer->cmdQueue->Signal(fence, 1));

    if (fence->GetCompletedValue() < 1)
    {
        HANDLE fenceEvt = CreateEventA(nil, FALSE, FALSE, nil);
        MSR_ASSERT(fenceEvt && "Failed to create fence event");
        GPU_DX12_CHECKED_CALL(fence->SetEventOnCompletion(1, fenceEvt));
        WaitForSingleObject(fenceEvt, INFINITE);
        CloseHandle(fenceEvt);
    }

    fence->Release();
}

void GPU_Dx12Destroy(GPU_Instance* baseRenderer)
{
    GPU_Dx12Instance* renderer = GPU_ToDx12Instance(baseRenderer);
    if (!renderer) return;

    GPU_Dx12WaitTillIdle(baseRenderer);

    if (renderer->d3d12maAllocator)
    {
        renderer->d3d12maAllocator->Release();
        renderer->d3d12maAllocator = nil;
    }

    if (renderer->cmdQueue)
    {
        renderer->cmdQueue->Release();
        renderer->cmdQueue = nil;
    }

    if (renderer->device)
    {
        ID3D12InfoQueue* iq = nil;
        if (SUCCEEDED(renderer->device->QueryInterface(IID_PPV_ARGS(&iq))))
        {
            ID3D12InfoQueue1* iq1 = nil;
            if (SUCCEEDED(iq->QueryInterface(IID_PPV_ARGS(&iq1))))
            {
                iq1->UnregisterMessageCallback(renderer->dbgCallbackCookie);
                iq1->Release();
            }

            iq->Release();
        }

        renderer->device->Release();
        renderer->device = nil;
    }

    if (renderer->adapter)
    {
        renderer->adapter->Release();
        renderer->adapter = nil;
    }

    if (renderer->dxgiFactory)
    {
        renderer->dxgiFactory->Release();
        renderer->dxgiFactory = nil;
    }

    if (renderer->dbgController)
    {
        ID3D12Debug1* dbgController1 = nil;
        if (SUCCEEDED(renderer->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController1))))
        {
            dbgController1->SetEnableGPUBasedValidation(false);
            dbgController1->Release();
        }

        ID3D12Debug4* dbgController4 = nil;
        if (SUCCEEDED(renderer->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController4))))
        {
            dbgController4->DisableDebugLayer();
            dbgController4->Release();
        }

        renderer->dbgController->Release();
        renderer->dbgController = nil;
    }

    if (MSR_DBG)
    {
        IDXGIDebug1* pDebug = nil;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            pDebug->Release();
        }
    }
}

#endif

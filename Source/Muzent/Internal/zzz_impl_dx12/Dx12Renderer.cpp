#include "Dx12Fns.h"
#if MZNT_DX12

static const DXGI_FORMAT k_MZNT_Internal_PreferredDx12ColourAttchFormat  = DXGI_FORMAT_R16G16B16A16_FLOAT;
static const DXGI_FORMAT k_MZNT_Internal_PreferredDx12DepthAttchFormat   = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

MZNT_DirectX12Renderer* MZNT_CreateRenderer_DirectX12(MZNT_RendererConfiguration config, PNSLR_Allocator tempAllocator)
{
    MZNT_DirectX12Renderer* output = PNSLR_New(MZNT_DirectX12Renderer, config.allocator, PNSLR_GET_LOC(), nil);
    if (!output) FORCE_DBG_TRAP;

    output->parent.type      = MZNT_RendererType_DirectX12;
    output->parent.allocator = config.allocator;
    output->parent.appHandle = config.appHandle;

    output->appName = PNSLR_CloneString(config.appName, output->parent.allocator);

    // debug layer
    u32 dxgiFactoryFlags = 0;
    if (PNSLR_DBG)
    {
        MZNT_INTERNAL_DX12_CHECKED_CALL(D3D12GetDebugInterface(IID_PPV_ARGS(&(output->dbgController))));
        output->dbgController->EnableDebugLayer();

        ID3D12Debug1* dbgController1;
        if (SUCCEEDED(output->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController1))))
        {
            dbgController1->SetEnableGPUBasedValidation(true);
            dbgController1->Release();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    // factory
    MZNT_INTERNAL_DX12_CHECKED_CALL(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&(output->dxgiFactory))));

    // adapter
    {
        IDXGIAdapter1* hwAdapter;
        for (u32 i = 0; output->dxgiFactory->EnumAdapters1(i, &hwAdapter) != DXGI_ERROR_NOT_FOUND; i++)
        {
            DXGI_ADAPTER_DESC1 desc;
            hwAdapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                hwAdapter->Release();
                continue;
            }

            if (SUCCEEDED(D3D12CreateDevice(hwAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), nil)))
            {
                output->adapter = hwAdapter;
                break;
            }
        }

        if (!output->adapter)
        {
            PNSLR_LogE(Panshilar::StringLiteral("No compatible DirectX 12 adapter found."), PNSLR_GET_LOC());
            FORCE_DBG_TRAP;
            return nil;
        }
    }

    // device
    MZNT_INTERNAL_DX12_CHECKED_CALL(D3D12CreateDevice(output->adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&(output->device))));

    MZNT_Internal_SetDx12ObjDebugName(output, output->device,
        PNSLR_StringLiteral("$.device"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)), tempAllocator);

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

            D3D12_MESSAGE_ID hide[] = {
                D3D12_MESSAGE_ID_CREATEDEVICE_DEBUG_LAYER_STARTUP_OPTIONS,
                D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
            };

            D3D12_INFO_QUEUE_FILTER filter = { };
            filter.DenyList.NumIDs = _countof(hide);
            filter.DenyList.pIDList = hide;

            infoQueue->AddStorageFilterEntries(&filter);

            ID3D12InfoQueue1* iq1 = nil;
            if (SUCCEEDED(output->device->QueryInterface(IID_PPV_ARGS(&iq1))))
            {
                DWORD cookie = 0;
                iq1->RegisterMessageCallback(MZNT_Internal_GetDx12DebugCallback(), D3D12_MESSAGE_CALLBACK_FLAG_NONE, nil, &cookie);
                output->dbgCallbackCookie = cookie;

                iq1->Release();
            }

            infoQueue->Release();
        }
    }

    // queue
    D3D12_COMMAND_QUEUE_DESC queueDesc = { };
    queueDesc.Type     = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    MZNT_INTERNAL_DX12_CHECKED_CALL(output->device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&(output->cmdQueue))));

    MZNT_Internal_SetDx12ObjDebugName(output, output->cmdQueue,
        PNSLR_StringLiteral("$.maincmdqueue"), PNSLR_FmtArgs(PNSLR_FmtString(output->appName)), tempAllocator);

    // allocator
    D3D12MA::ALLOCATOR_DESC allocDesc = { };
    allocDesc.pDevice = output->device;
    allocDesc.pAdapter = output->adapter;
    MZNT_INTERNAL_DX12_CHECKED_CALL(D3D12MA::CreateAllocator(&allocDesc, &(output->d3d12maAllocator)));

    return output;
}

b8 MZNT_WaitTillRendererIdle_DirectX12(const MZNT_DirectX12Renderer* renderer)
{
    if (!renderer || !(renderer->device) || !(renderer->cmdQueue)) return false;

    ID3D12Fence* fence = nil;
    MZNT_INTERNAL_DX12_CHECKED_CALL(renderer->device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    MZNT_INTERNAL_DX12_CHECKED_CALL(renderer->cmdQueue->Signal(fence, 1));

    if (fence->GetCompletedValue() < 1)
    {
        HANDLE fenceEvt = CreateEventA(nil, FALSE, FALSE, nil);
        MZNT_INTERNAL_DX12_CHECKED_CALL(fence->SetEventOnCompletion(1, fenceEvt));
        WaitForSingleObject(fenceEvt, INFINITE);
        CloseHandle(fenceEvt);
    }

    fence->Release();

    return true;
}

b8 MZNT_DestroyRenderer_DirectX12(MZNT_DirectX12Renderer* renderer, PNSLR_Allocator tempAllocator)
{
    if (!renderer) return false;

    MZNT_WaitTillRendererIdle_DirectX12(renderer);

    renderer->d3d12maAllocator->Release();

    renderer->cmdQueue->Release();

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

    renderer->adapter->Release();

    renderer->dxgiFactory->Release();

    if (renderer->dbgController)
    {
        ID3D12Debug1* dbgController1;
        if (SUCCEEDED(renderer->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController1))))
        {
            dbgController1->SetEnableGPUBasedValidation(false);
            dbgController1->Release();
        }

        ID3D12Debug4* dbgController4;
        if (SUCCEEDED(renderer->dbgController->QueryInterface(IID_PPV_ARGS(&dbgController4))))
        {
            dbgController4->DisableDebugLayer();
            dbgController4->Release();
        }

        renderer->dbgController->Release();
    }

    PNSLR_FreeString(renderer->appName, renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    PNSLR_Delete(renderer, renderer->parent.allocator, PNSLR_GET_LOC(), nil);

    if (PNSLR_DBG)
    {
        IDXGIDebug1* pDebug = nullptr;
        if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
        {
            pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
            pDebug->Release();
        }
    }

    return true;
}

#endif

#include <OvRender.h>


class Dx11Overlay : public OvOverlay
{
public:
    Dx11Overlay( IDXGISwapChain* SwapChain, OV_RENDER RenderFunction );

    // Standard functions
    VOID DrawText( WCHAR* Text );
    VOID DrawText( WCHAR* Text, DWORD Color );
    VOID DrawText( WCHAR* Text, int X, int Y, DWORD Color );
    VOID Begin();
    VOID End();
    VOID* GetDevice();
};


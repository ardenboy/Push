typedef VOID (*HK_IDXGISWAPCHAIN_PRESENT_CALLBACK)(
	IDXGISwapChain* SwapChain
	);

VOID 
HookDxgi( HK_IDXGISWAPCHAIN_PRESENT_CALLBACK IDXGISwapChain_PresentCallback );
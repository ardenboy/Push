#include "font.h"


class Dx11Font : public Font
{
public:
    Dx11Font(ID3D11Device *device, FONT_PROPERTIES* Properties);
    VOID Begin();
    VOID End();
    VOID BeginBatch( ID3D11ShaderResourceView* texSRV );
    VOID DrawBatch( UINT startSpriteIndex, UINT spriteCount );
    VOID EndBatch( );
    VOID DrawString();
    BOOLEAN InitD3D11Sprite( );
};
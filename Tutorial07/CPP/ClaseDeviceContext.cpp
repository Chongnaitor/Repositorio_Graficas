#include "..\Encabezados\ClaseDeviceContext.h"

#ifdef D3D11
void ClaseDeviceContext::Init(ID3D11DeviceContext* _context){

	g_pImmediateContextD3D11 = _context;
}
#endif

void ClaseDeviceContext::Update(){
}

void ClaseDeviceContext::Render(){
}

void ClaseDeviceContext::Delete(){
}

#include "UE4Helper.h"

UE4Helper::UE4Helper()
{
}


UE4Helper::~UE4Helper()
{
}

UMaterial* UE4Helper::LoadMaterial(const TCHAR* path, UObject* inOuter /*= NULL*/)
{
	return LoadContent<UMaterial>(path, inOuter);
}

UMaterialInstanceDynamic* UE4Helper::LoadAndCreateMaterialInstance(const TCHAR* path, UObject* inOuter /*= NULL*/)
{
	UMaterial* mat = LoadMaterial(path, inOuter);
	if (mat)
		return UMaterialInstanceDynamic::Create(mat, inOuter);
	return NULL;
}


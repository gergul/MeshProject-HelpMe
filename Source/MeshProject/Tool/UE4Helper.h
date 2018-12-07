#pragma once

class UE4Helper
{
private:
	UE4Helper();
	~UE4Helper();

public:
	template<class T>
	static T* AddComponent(AActor *actor, const FName& name = "")
	{
		T* comp = NewObject<T>(actor, name);
		actor->AddOwnedComponent(comp);
		comp->RegisterComponent();
		//comp->AttachTo(actor->RootComponent);
		comp->bUseAsyncCooking = true;

		return comp;
	}

	/*
	 * 加载资源
	 * path - 可在UEditor编辑器中加载这个资源，然后在鼠标的tip上则看到资源路径
	 */
	template<class T>
	static T* LoadContent(const TCHAR* path, UObject* inOuter = NULL)
	{
		return Cast<T>(StaticLoadObject(T::StaticClass(), inOuter, path ));
	}

	/*
	 * 加载材质
	 */
	static UMaterial* LoadMaterial(const TCHAR* path, UObject* inOuter = NULL);

	/*
	 * 加载并创建材质实例
	 */
	static UMaterialInstanceDynamic* LoadAndCreateMaterialInstance(const TCHAR* path, UObject* inOuter = NULL);
};




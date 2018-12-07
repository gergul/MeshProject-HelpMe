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
	 * ������Դ
	 * path - ����UEditor�༭���м��������Դ��Ȼ��������tip���򿴵���Դ·��
	 */
	template<class T>
	static T* LoadContent(const TCHAR* path, UObject* inOuter = NULL)
	{
		return Cast<T>(StaticLoadObject(T::StaticClass(), inOuter, path ));
	}

	/*
	 * ���ز���
	 */
	static UMaterial* LoadMaterial(const TCHAR* path, UObject* inOuter = NULL);

	/*
	 * ���ز���������ʵ��
	 */
	static UMaterialInstanceDynamic* LoadAndCreateMaterialInstance(const TCHAR* path, UObject* inOuter = NULL);
};




// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FSimpleFtpToolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	//�Զ���˵� �ļ���
	TSharedRef<FExtender> OnExtendContentBrowser(const TArray<FString>& NewPaths);
	void CreateSubMenuForContentBrowser(FMenuBuilder& MenuBuilder, TArray<FString> NewPaths);  //���ܴ����ã�ֻ���ڴ濽��

	//�Զ���˵� ��Դ
	TSharedRef<FExtender> OnExtendContentAssetBrowser(const TArray<FAssetData>& NewAssetPaths);
	void CreateSubMenuForAssetBrowser(FMenuBuilder& MenuBuilder, TArray<FString> NewPaths);  //���ܴ����ã�ֻ���ڴ濽��


private:

	void CreateToFTPServer();

	//�Զ���˵���ť����¼� (����ʵ���ļ���)
	void CreateInstanceFolder(TArray<FString> NewPaths);
	//�Զ���˵���ť����¼� ���ύ�ļ����µ���Դ��
	void SubmitSourceUnderTheFolder(TArray<FString> NewPaths);
	//Ϊ�ļ����µ���Դ���������ļ�
	void CheckNameAndGenerateDependencyFiles(TArray<FString> NewPaths);

	//�Զ���˵���ť����¼� ���ύѡ�е���Դ��
	void SubmitSelectedSource(TArray<FString> NewPaths);

private:
	TSharedPtr<class FUICommandList> PluginCommands;
};

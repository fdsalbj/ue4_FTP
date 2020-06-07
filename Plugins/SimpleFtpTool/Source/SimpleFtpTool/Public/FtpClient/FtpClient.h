// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "FtpConfig/FtpConfig.h"
#include "FtpCommon/FtpTypes.h" 
#include "FtpCommon/FtpMacro.h"
#include "SimpleHttpManager.h"



class SIMPLEFTPTOOL_API FtpClientManager
{

public:
	FtpClientManager();
	~FtpClientManager();
	static FtpClientManager* Get();
	static void Destroy();
	//��ʼ���ļ�Ŀ¼�ṹ
	void Initialize_Folder();
	//����ʵ���ļ���
	void CreateInstanceFolder(const FString& InstanceName);
	void ShowMessageBox(const TArray<FString>& NameNotValidFiles, const TArray<FInvalidDepInfo>& DepenNotValidFiles);

	//�ϴ�ͼƬ�Լ�˵����web������
	//1. ʵ���ϴ�
	//2. ������Դ�ϴ�
	//3. �������ļ����ϴ�
	bool UploadInstanceDescriptToWeb(const FString& InFolderPath, const TArray<FString>& ThirdFolders);

	bool UploadAssetsDescriptToWeb(const TArray<FString>& InAssetPaths);

	bool UploadThirdFolderDescriptToWeb(const TArray<FString>& InThirdPath);
	
public:
	/*********************************************************************/
	/*************************ftp�ͻ��˲����ӿ�****************************/
	/*********************************************************************/
	//����controlsocket
	bool FTP_CreateControlSocket(const FString& IP, const int32& port);
	//����ָ��
	bool FTP_SendCommand(const EFtpCommandType& cmdtype, const FString& Param);
	//��Ҫ�õ��������ӵ����NLST,LIST,RETR,STOR
	//�о��ļ��� (���·��)
	bool FTP_ListFile(const FString& serverPath, TArray<FString>& OutFiles, bool bIncludeFolder = true);
	//���ص����ļ� �涨�ļ�·����/���� �磺/Folder1/Folder2/adadd.txt localpath:��Ҫ�þ���·���磺E:/Game/Folder
	bool FTP_DownloadOneFile(const FString& serverFileName);
	//�����ļ�����������ļ� serverFolder:�� /asd				localpath:��Ҫ�þ���·���磺E:/Game/Folder
	bool FTP_DownloadFiles(const FString& serverFolder);
	//�ϴ������ļ�
	bool FTP_UploadOneFile(const FString& localFileName);
	//�ϴ��ļ�����������ļ�
	bool FTP_UploadFilesByFolder(const FString& InGamePath, TArray<FString>& NameNotValidFiles, TArray<FInvalidDepInfo>& DepenNotValidFiles);
	//����PackageName�ϴ���Դ
	bool FTP_UploadFilesByAsset(const TArray<FString>& InPackNames, TArray<FString>& NameNotValidFiles, TArray<FInvalidDepInfo>& DepenNotValidFiles);

	bool ftp_test(const FString& InFolderPath, const FString& URL);

public:
	//�о�·���µ������ļ�(����·��)
	bool GetAllFileFromLocalPath(const FString& localPath, TArray<FString>& AllFiles, bool bRecursively = true);
	//�����ļ���
	bool CreateDirByAsssetPath(const FString& InAssetFullPath);
	//ɾ���ļ���������������
	bool DeleteFileOrFolder(const FString& InDir);
	//��ⵥ���ļ����µ��ļ������Ƿ�Ϸ�(ֻ����../../Common_Material  ���� ../../Ins_Material �ļ��У�����������ļ������� ../../Instance/ProjA ��Ҫ���������ļ����г�����Ȼ��ÿ�����ļ��ж�ִ��һ���������)
	bool FileNameValidationOfOneFolder(TArray<FString>& NoValidFiles, const FString& InFullFolderPath);
	//Ѱ��һ����Դ����������
	void RecursiveFindDependence(const FString& InPackageName, TArray<FString>& AllDependence);
	//���һ����Դ�����������Ƿ�Ϸ�
	bool ValidationDependenceOfOneAsset(const FString& InGamePath, const FString& AssetPackName, const TArray<FString>& TheAssetDependence, FInvalidDepInfo& InvalidInfo, bool& bDepHasChanged, bool bAllNameValid);
	//���һ���ļ����µ�������Դ����������
	bool ValidationAllDependenceOfTheFolder(const FString& InGamePath, TArray<FInvalidDepInfo>& NotValidDependences, bool bAllNameValid = false);
	//�ϴ��ļ��Լ������ļ� ������Դ��PackageName����
	bool UploadDepenceAssetAndDepences(const TArray<FString>& InPackageNames);
	//�жϷ��������Ƿ���ڸ���Դ������SIZE ������ݷ��������ص���Ӧ���ж��ļ��Ƿ���ڣ�Ȼ���ٸ����û�������ѡ���Ƿ񸲸Ƿ������ļ�����һ����û��ʹ�õ�У���룩
	bool OverrideAssetOnServer(const FString& FileFullPath);
	//У����ԴУ����
	bool IsAssetValidCodeSame(const FString& FileFullPath);
	//У��ʵ��У����
	bool IsInstValidCodeSame(const FString& InstName);
	//���ʵ���Ƿ�������������Դ ����ʵ���ļ���·��
	void HasDepencyThirdAsset(const FString& InGamePath, TArray<FString>& ThirdPartyName);
	//�ύ�������ļ���
	void UploadThirdPartyFolder(const TArray<FString>& InFolders);
	//����ʵ���������ļ�
	bool DownloadDepenceAsset(const FString& InInstFolderPath);
	
private:
	//��ȡ����IP
	FString GetMylocalIPADDR();
	//���ܷ���˷��ص���Ϣ
	bool ReceiveData(FSocket* sock, FString& RecvMesg, TArray<uint8>& dataArray, bool bSleep = true);
	//���� dataSocket
	bool CreateDataSocket_PASV(int32 port2);
	//�����յ��Ķ���������ת����FString
	FString BinaryArrayToString(const TArray<uint8>& BinaryArray);
	//ת��ָ��
	FString SwitchCommand(const EFtpCommandType& cmdtype, const FString& Param);
	//�ӷ��������ص��ַ����л�ȡ�µĶ˿ں�
	int32 GetPASVPortFromString(const FString& RecvMesg);
	//����PASV�����ȡ���������صĶ˿ں�  ���������������ó�����Ϊ�˷����ȡ���������صĶ˿�
	int32 SendPASVCommand();
	//�жϴ����serverPath���ļ� ���� �ļ���
	EFileType JudgeserverPath(const FString& InserverPath);
	

private:
	//Debug
	void Print(const FString& Mesg, float Time = 100.f, FColor Color = FColor::Yellow);
	void Print(const TArray<uint8>& dataArray, float Time = 100.f, FColor Color = FColor::Purple);
	//ɾ��û���õ��������ļ�
	void DeleteUselessFile();

private:
	static FtpClientManager* ftpInstance;
	int32 ResponseCode;			//��������Ӧ��
	FSocket* controlSocket;		//�������� :һֱ��������ֱ��������������߷������ر�
	FSocket* dataSocket;		//�������� :�ڷ����ļ����������ϴ����أ�ʱ�������ӣ���ɲ�����Ͽ�����
	FIPv4Address ipAddr;
	
	//ί�� �ϴ���������Դ��
	FUploadThirdPartyDelegate UploadThirdPartyDelegate;

private:
	FString DataTypeIni;
	
};

#define FTP_INSTANCE FtpClientManager::Get()
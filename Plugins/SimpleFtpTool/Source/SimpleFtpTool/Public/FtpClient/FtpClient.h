// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "FtpConfig/FtpConfig.h"
#include "FtpCommon/FtpTypes.h" 
#include "FtpCommon/FtpMacro.h"



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
	
public:
	/*********************************************************************/
	/*************************ftp�ͻ��˲����ӿ�****************************/
	/*********************************************************************/
	//����controlsocket
	bool FTP_CreateControlSocket(FString IP = TEXT("127.0.0.1"), int32 port = 21);
	//����ָ��
	bool FTP_SendCommand(const EFtpCommandType& cmdtype, const FString& Param);
	//��Ҫ�õ��������ӵ����NLST,LIST,RETR,STOR
	//�о��ļ��� (���·��)
	bool FTP_ListFile(const FString& serverPath, TArray<FString>& OutFiles, bool bIncludeFolder = true);
	//���ص����ļ� �涨�ļ�·����/���� �磺/Folder1/Folder2/adadd.txt localpath:��Ҫ�þ���·���磺E:/Game/Folder
	bool FTP_DownloadOneFile(const FString& serverFileName, const FString& localSavePath);
	//�����ļ�����������ļ� serverFolder:�� /asd				localpath:��Ҫ�þ���·���磺E:/Game/Folder
	bool FTP_DownloadFiles(const FString& serverFolder, const FString& localSavePath);
	//�ϴ������ļ�
	bool FTP_UploadOneFile(const FString& localFileName);
	//�ϴ��ļ�����������ļ�
	bool FTP_UploadFilesByFolder(const FString& InGamePath, TArray<FString>& NameNotValidFiles, TArray<FString>& DepenNotValidFiles);
	//����PackageName�ϴ���Դ
	bool FTP_UploadFilesByAsset(const TArray<FString>& InPackNames, TArray<FString>& NameNotValidFiles, TArray<FString>& DepenNotValidFiles);

	bool ftp_test(const FString& InFolderPath);

private:
	//���ܷ���˷��ص���Ϣ
	bool ReceiveData(FSocket* sock, FString& RecvMesg, TArray<uint8>& dataArray, bool bSleep = true);
	//���� dataSocket
	bool CreateDataSocket_PASV(int32 port2);
	//�����յ��Ķ���������ת����FString
	FString BinaryArrayToString(TArray<uint8> BinaryArray);
	//ת��ָ��
	FString SwitchCommand(const EFtpCommandType& cmdtype, const FString& Param);
	//�ӷ��������ص��ַ����л�ȡ�µĶ˿ں�
	int32 GetPASVPortFromString(const FString& RecvMesg);
	//����PASV�����ȡ���������صĶ˿ں�  ���������������ó�����Ϊ�˷����ȡ���������صĶ˿�
	int32 SendPASVCommand();
	//�жϴ����serverPath���ļ� ���� �ļ���
	EFileType JudgeserverPath(const FString& InserverPath);
	//�о�·���µ������ļ�(����·��)
	bool GetAllFileFromLocalPath(const FString& localPath, TArray<FString>& AllFiles, bool bRecursively = true); 
	//�����ļ���
	bool CreateDirByAsssetPath(const FString& InAssetFullPath);
	//ɾ���ļ���������������
	bool DeleteFileOrFolder(const FString& InDir);
	//��ⵥ���ļ����µ��ļ������Ƿ�Ϸ�
	bool FileValidationOfOneFolder(TArray<FString>& NoValidFiles, const FString& InFullFolderPath);
	//Ѱ��һ����Դ����������
	void RecursiveFindDependence(const FString& InPackageName, TArray<FString>& AllDependence);
	//���һ����Դ�����������Ƿ�Ϸ�
	bool ValidationDependenceOfOneAsset(const FString& InGamePath, const FString& AssetPackName, const TArray<FString>& TheAssetDependence);
	//���һ���ļ����µ�������Դ����������
	bool ValidationAllDependenceOfTheFolder(const FString& InGamePath, TArray<FString>& NotValidDependences, bool bAllNameValid = false);
	//

private:
	//Debug
	void Print(const FString& Mesg, float Time = 100.f, FColor Color = FColor::Yellow);
	void Print(const TArray<uint8>& dataArray, float Time = 100.f, FColor Color = FColor::Purple);
	void ShowMessageBox(const TArray<FString>& NameNotValidFiles, const TArray<FString>& DepenNotValidFiles);
private:
	static FtpClientManager* ftpInstance;
	int32 ResponseCode;		//��������Ӧ��
	FSocket* controlSocket;		//�������� :һֱ��������ֱ��������������߷������ر�
	FSocket* dataSocket;		//�������ӣ� �ڷ����ļ����������ϴ����أ�ʱ�������ӣ���ɲ�����Ͽ�����
	FIPv4Address ipAddr;
	
private:
	FString DataTypeIni;
};

#define FTP_INSTANCE FtpClientManager::Get()

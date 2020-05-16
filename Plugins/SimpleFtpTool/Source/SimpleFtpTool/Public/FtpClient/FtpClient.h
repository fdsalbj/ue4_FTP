// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Networking.h"
#include "FtpConfig/FtpConfig.h"
#include "FtpClient/FtpTypes.h" 
#include "FtpClient/FtpMacro.h"



class SIMPLEFTPTOOL_API FtpClientManager
{

public:
	FtpClientManager();
	~FtpClientManager();
	static FtpClientManager* Get();
	static void Destroy();

private:
	//�������� :һֱ��������ֱ���������
	FSocket* controlSocket;
	//�������ӣ� �ڷ����ļ����������ϴ����أ�ʱ�������ӣ���ɲ�����Ͽ�����
	FSocket* dataSocket;
	FIPv4Address ipAddr;

public:
	
	/*********************************************************************/
	/*************************ftp�ͻ��˲����ӿ�****************************/
	/*********************************************************************/
	//����controlsocket
	bool FTP_CreateControlSocket(FString IP = TEXT("192.168.0.4"), int32 port = 21);
	//���ܷ���˷��ص���Ϣ
	bool FTP_ReceiveData(FString& RecvMesg, bool bSleep = true);
	//����ָ��
	bool FTP_SendCommand(const EFtpCommandType& cmdtype, const FString& Param);
	
	//���� dataSocket
	bool FTP_CreateDataSocket_PASV(int32 port2);
	
	
	//��Ҫ�õ��������ӵ����NLST,LIST,RETR,STOR
	bool FTP_StorFile(const FString& servetPath, const FString& localFilePath);

private:
	//�����յ�������ת����FString
	FString BinaryArrayToString(TArray<uint8> BinaryArray);
	//ת��ָ��
	FString SwitchCommand(const EFtpCommandType& cmdtype, const FString& Param);
	//�ӷ��������ص��ַ����л�ȡ�µĶ˿ں�
	int32 GetPASVPortFromString(const FString& RecvMesg);
	//����PASV�����ȡ���������صĶ˿ں�  ���������������ó�����Ϊ�˷����ȡ���������صĶ˿�
	int32 FTP_SendPASVCommand();
private:

	static FtpClientManager* ftpInstance;
	
};

#define FTP_INSTANCE FtpClientManager::Get()

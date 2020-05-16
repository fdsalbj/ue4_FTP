#include "FtpClient/FtpClient.h"
#include "Engine.h"


#if PLATFORM_WINDOWS
#pragma optimize("",off) 
#endif


void Print(const FString& Mesg, float Time = 100.f, FColor Color = FColor::Yellow)
{
	if (GEngine && GetDefault<UFtpConfig>()->bShowDebug)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Mesg);
	}
}

FtpClientManager* FtpClientManager::ftpInstance = nullptr;

FtpClientManager::FtpClientManager()
{ 
}

FtpClientManager::~FtpClientManager()
{
	if (controlSocket)
	{
		controlSocket->Close();
		controlSocket = nullptr;
	}
	if (dataSocket)
	{
		dataSocket->Close();
		dataSocket = nullptr;
	}
}

FtpClientManager* FtpClientManager::Get()
{
	if (!ftpInstance)
	{
		ftpInstance = new FtpClientManager();
	}
	return ftpInstance;
}

void FtpClientManager::Destroy()
{
	if (ftpInstance)
	{
		delete ftpInstance;
	}
	ftpInstance = nullptr;
}

FString FtpClientManager::BinaryArrayToString(TArray<uint8> BinaryArray)
{
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}

FString FtpClientManager::SwitchCommand(const EFtpCommandType& cmdtype, const FString& Param)
{
	FString cmd;

	switch (cmdtype)
	{
		//�ж���������
		case EFtpCommandType::ABOR:
			cmd = TEXT("ABOR ");
			cmd.Append(TEXT("\r\n"));
			break;
		//ָ���û���
		case EFtpCommandType::USER:
			cmd = TEXT("USER ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//��¼����
		case EFtpCommandType::PASS:
			cmd = TEXT("PASS ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�ӷ������Ϸ���ָ���ļ��Ĵ�С
		case EFtpCommandType::SIZE:
			cmd = TEXT("SIZE ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�ı乤��Ŀ¼
		case EFtpCommandType::CWD:
			cmd = TEXT("CWD ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//���� FTP �������ͻ��˼����Ķ˿ںţ��� FTP ��������������ģʽ���ӿͻ���
		case EFtpCommandType::PORT:
			cmd = TEXT("PORT ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�����ļ�
		case EFtpCommandType::RETR:
			cmd = TEXT("RETR ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�ϴ��ļ�
		case EFtpCommandType::STOR: 
			cmd = TEXT("STOR ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//ɾ���������ϵ�ָ���ļ�
		case EFtpCommandType::DELE:
			cmd = TEXT("DELE ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�г�ָ��Ŀ¼�µ�����
		case EFtpCommandType::NLST:
			cmd = TEXT("NLST ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//������ļ����г��ļ���Ϣ�������Ŀ¼���г��ļ��б�
		case EFtpCommandType::LIST:
			cmd = TEXT("LIST ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�ڷ������Ͻ���ָ��Ŀ¼
		case EFtpCommandType::MKD:
			cmd = TEXT("MKD ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//��ʾ��ǰ����Ŀ¼
		case EFtpCommandType::PWD:
			cmd = TEXT("PWD ");
			cmd.Append(TEXT("\r\n"));
			break;
		//�ڷ�����ɾ��ָ��Ŀ¼
		case EFtpCommandType::RMD:
			cmd = TEXT("RMD ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�����������ͣ�A=ASCII��E=EBCDIC��I=binary��
		case EFtpCommandType::TYPE:
			cmd = TEXT("TYPE ") + Param;
			cmd.Append(TEXT("\r\n"));
			break;
		//�˳���¼
		case EFtpCommandType::QUIT:
			cmd = TEXT("QUIT ");
			cmd.Append(TEXT("\r\n"));
			break;
		default:
			break;
	}
	return cmd;
}

int32 FtpClientManager::GetPASVPortFromString(const FString& RecvMesg)
{
	auto lambda = [](const FString& split, FString R_String, bool bRight)->FString
	{
		FString L, R;
		R_String.Split(split, &L, &R);
		if (bRight)
			return R;
		else
			return L;
	};
	int32 p1 = 0, p2 = 0;
	//227 Entering Passive Mode (192,168,0,4,255,245)
	//���ж���Ӧ���ǲ��ǵ���227
	FString LeftString = lambda(TEXT(" "), RecvMesg, false);
	int32 ResponseCode = FCString::Atoi(*LeftString);
	if (PASV_MODE != ResponseCode)
	{
		return -1;
	}
	FString RightString =lambda(TEXT("("), RecvMesg, true);
	for (int32 i = 0; i < 5; i++)
	{
		RightString = lambda(TEXT(","), RightString, true);
		if (3 == i)
		{
			p1 = FCString::Atoi(*RightString);
		}
		if (4 == i)
		{
			p2 = FCString::Atoi(*RightString);
		}
	}
	return (p1 * 256 + p2);
}

bool FtpClientManager::FTP_CreateControlSocket( FString IP,  int32 port)
{ 
	//ת��Ip��ַ
	FIPv4Address::Parse(IP, ipAddr);
	//����һ�� FInternetAddr ���IP�Ͷ˿�
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ipAddr.Value);
	addr->SetPort(port);
	//����controlSocket
	controlSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	//���ӷ�����
	if (controlSocket->Connect(*addr))
	{
		FString RecvMesg;
		TArray<uint8> RecvBinary;
		FTP_ReceiveData(RecvMesg);
		return true;
	}
	Print("controlSocket connect failed!");
	return false; 
}

bool FtpClientManager::FTP_ReceiveData(FString& RecvMesg , bool bSleep)
{
	if(bSleep)
		FPlatformProcess::Sleep(1.f);
	if (!controlSocket)
	{
		RecvMesg = TEXT("Error:control socket is nullptr.");
		Print(RecvMesg);
		return false;
	}
	TArray<uint8> RecvData;
	uint32 size;
	uint8 element = 0;
	while (controlSocket->HasPendingData(size))
	{
		RecvData.Init(element, FMath::Min(size, 65507u));
		int32 read = 0;
		controlSocket->Recv(RecvData.GetData(), RecvData.Num(), read);
	}
	if (RecvData.Num() <= 0)
	{
		RecvMesg = TEXT("Error:RecvData is empty.");
		Print(RecvMesg);
		return false;
	}
	const FString ReceivedUE4String = BinaryArrayToString(RecvData);
	RecvMesg = ReceivedUE4String;
	Print(RecvMesg);
	return true;
}

bool FtpClientManager::FTP_SendCommand(const EFtpCommandType& cmdtype, const FString& Param)
{
	FString strCommand = SwitchCommand(cmdtype, Param);
	TCHAR* serializedChar = strCommand.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar) + 1;
	int32 sent = 0;
	if (controlSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar),size,sent))
	{
		Print("SendCommand  " + Param + "  succeed!");
		FString RecvMesg;
		FTP_ReceiveData(RecvMesg);
		return true;
	}
	Print("SendCommand " + Param + " failed!");
	return false;
}

int32 FtpClientManager::FTP_SendPASVCommand()
{
	FString cmd_PASV = TEXT("PASV \r\n");
	TCHAR* serializedChar = cmd_PASV.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar) + 1;
	int32 sent = 0;
	if (controlSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent))
	{
		Print("set pasv mode.");
		FString RecvMesg;
		FTP_ReceiveData(RecvMesg);
		return GetPASVPortFromString(RecvMesg);
	}
	return -1;
}

bool FtpClientManager::FTP_CreateDataSocket_PASV(int32 port2)
{
	//������ñ���ģʽ��PASV��������PASV����������᷵��һ������<,h2,h3,h4,p1,p2>���Ҵ�һ���¶˿ڹ��ͻ���dataSocket���� h1.h2.h3.h4 ���������IP��ַ���¶˿�port = p1*256+p2
	//����һ��FInternetAddr�����IP�Ͷ˿�
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ipAddr.Value);
	addr->SetPort(port2);
	//����dataSocket
	dataSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);
	if (dataSocket->Connect(*addr))
	{
		return true;
	}
	Print("dataSocket connect failed!");
	if (dataSocket)
	{
		delete dataSocket;
		dataSocket = nullptr;
	}
	return false;
}

bool FtpClientManager::FTP_StorFile(const FString& servetPath, const FString& localFilePath)
{
	//�ȷ���PASVָ��
	int32 PasvPort = FTP_SendPASVCommand();
	//������������
	FTP_CreateDataSocket_PASV(PasvPort);
	//�����ļ�����...


	return false;
}

#if PLATFORM_WINDOWS
#pragma optimize("",on) 
#endif
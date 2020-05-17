#include "FtpClient/FtpClient.h"
#include "Engine.h"
#include "Misc/Paths.h"
//#include "Misc/App.h"
#include "Misc/FileHelper.h"
//#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include <fstream>


#if PLATFORM_WINDOWS
#pragma optimize("",off) 
#endif


//�����ļ�·������ȡ�ļ���С
int64_t getFileSize(const FString& InfilePath)
{
	std::string filePath = TCHAR_TO_UTF8(*InfilePath);
	std::fstream f(filePath, std::ios::in | std::ios::binary);
	f.seekg(0, f.end);
	int64_t filesize = f.tellg();
	f.close();
	return filesize;
}

void FtpClientManager::Print(const FString& Mesg, float Time, FColor Color)
{
	if (GEngine && GetDefault<UFtpConfig>()->bShowDebug)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Mesg);
	}
}

void FtpClientManager::Print(const TArray<uint8>& dataArray, float Time, FColor Color)
{
	if (GEngine && GetDefault<UFtpConfig>()->bShowDebug)
	{

		GEngine->AddOnScreenDebugMessage(-1, Time, Color, BinaryArrayToString(dataArray));
	}
}

FtpClientManager* FtpClientManager::ftpInstance = nullptr;

FtpClientManager::FtpClientManager()
{ 
	controlSocket=nullptr;
	dataSocket = nullptr;
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


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
//�������Ӧ�붼���ɿͻ��˿������ӽ���,�������ص����������������ӽ���
bool FtpClientManager::ReceiveData(FSocket* sock, FString& RecvMesg, TArray<uint8>& dataArray, bool bSleep)
{
	if (bSleep)
		FPlatformProcess::Sleep(GetDefault<UFtpConfig>()->sleeptime);
	if (!sock)
	{
		RecvMesg = TEXT("Error:sock is nullptr.");
		Print(RecvMesg);
		return false;
	}
	TArray<uint8> RecvData;
	uint32 size;
	uint8 element = 0;
	while (sock->HasPendingData(size))
	{
		RecvData.Init(element, FMath::Min(size, 65507u));
		int32 read = 0;
		sock->Recv(RecvData.GetData(), RecvData.Num(), read);
	}
	if (RecvData.Num() <= 0)
	{
		RecvMesg = TEXT("Error : RecvData is empty.");
		Print(RecvMesg);
		return false;
	}
	const FString ReceivedUE4String = BinaryArrayToString(RecvData);
	RecvMesg = ReceivedUE4String;
	dataArray = RecvData;
	if(sock == controlSocket)
	{
		//��ȡ��Ӧ��
		FString l,r;
		RecvMesg.Split(TEXT(" "), &l,&r);
		ResponseCode = FCString::Atoi(*l);
	}
	Print(RecvMesg);
	return true;
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
	int32 Local_ResponseCode = FCString::Atoi(*LeftString);
	if (PASV_MODE != Local_ResponseCode)
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

bool FtpClientManager::CreateDataSocket_PASV(int32 port2)
{
	//������ñ���ģʽ��PASV��������PASV����������᷵��һ������<h1,h2,h3,h4,p1,p2>���Ҵ�һ���¶˿ڹ��ͻ���dataSocket���� h1.h2.h3.h4 ���������IP��ַ���¶˿�port = p1*256+p2
	//����һ��FInternetAddr�����IP�Ͷ˿�
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ipAddr.Value);
	addr->SetPort(port2);
	//����dataSocket
	dataSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("data"), false);
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

//����PASV�����ȡ���������صĶ˿ں�  ���������������ó�����Ϊ�˷����ȡ���������صĶ˿�
int32 FtpClientManager::SendPASVCommand()
{
	FString cmd_PASV = TEXT("PASV \r\n");
	TCHAR* serializedChar = cmd_PASV.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar) + 1;
	int32 sent = 0;
	if (controlSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent))
	{
		Print("set pasv mode.");
		FString RecvMesg;
		TArray<uint8> RecvBinary;
		ReceiveData(controlSocket, RecvMesg, RecvBinary);
		return GetPASVPortFromString(RecvMesg);
	}
	return -1;
}

EFileType FtpClientManager::JudgeserverPath(const FString &InserverPath)
{
	FString Extension = FPaths::GetExtension(InserverPath,false);
	if (Extension.IsEmpty())
	{
		return EFileType::FOLDER;
	}
	return EFileType::FILE;
}

bool FtpClientManager::ListAllFileFromLocalPath(const FString& localPath, TArray<FString>& AllFiles, bool bRecursively)
{
	class FileVisitor : public IPlatformFile::FDirectoryVisitor
	{
	public:
		FileVisitor(TArray<FString>& InVisitFiles)
			:VisitFiles(InVisitFiles)
		{}
		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory)
		{
			if (!bIsDirectory)
			{
				VisitFiles.Add(FilenameOrDirectory);
			}
			return true;
		}
		TArray<FString>& VisitFiles;
	};
	FileVisitor Visitor(AllFiles);
	if(bRecursively)
		return FPlatformFileManager::Get().GetPlatformFile().IterateDirectoryRecursively(*localPath, Visitor);
	return FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*localPath, Visitor);
}

bool FtpClientManager::CreateDir(const FString& InDir)
{
	if(FTP_SendCommand(EFtpCommandType::MKD, InDir))
	{
		if(ERROR_DIRECTORY == ResponseCode)
			return false;
		return true;
	}
	return false;
}

bool FtpClientManager::DeleteFileOrFolder(const FString& InDir, bool bForce)
{
	bool bSuccessed = true;
	FString Extension = FPaths::GetExtension(InDir,false);
	TArray<FString> AllOutFiles;
	TArray<FString> RemoveArray;
	if(Extension.IsEmpty())
	{
		//�ļ���
		if(FTP_ListFile(InDir, AllOutFiles, true))
		{
			RemoveArray = AllOutFiles;
			if (bForce)
			{
				for(const auto& Temp : RemoveArray)
				{
					Extension = FPaths::GetExtension(Temp);
					if(!Extension.IsEmpty())
					{
						//��ɾ�������ļ���Ȼ���ٵݹ�ɾ�������ļ���
						FTP_SendCommand(EFtpCommandType::DELE, Temp);
						AllOutFiles.Remove(Temp);
					}
				}
				//�����ļ���
				RemoveArray = AllOutFiles;
				for (const auto& TempFolder : RemoveArray) //ɾ�����п���ɾ�����ļ���
				{
					if (FTP_SendCommand(EFtpCommandType::RMD, TempFolder))
					{
						if (ERROR_DIRECTORY != ResponseCode)
						{
							AllOutFiles.Remove(TempFolder);
						}
					}
				}
				//ʣ�����еĶ����޷�ɾ�����ļ��У������ݹ�
				DeleteFileOrFolder(InDir, bForce);
			}
			return true;
		}
		return false;
	}
	else
	{
		if (FTP_SendCommand(EFtpCommandType::DELE, InDir))
		{
			if (ERROR_DIRECTORY == ResponseCode)
				return false;
			return true;
		}
	}
	return bSuccessed;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool FtpClientManager::FTP_CreateControlSocket( FString IP,  int32 port)
{ 
	if(controlSocket)
	{
		return false;
	}
	//ת��Ip��ַ
	FIPv4Address::Parse(IP, ipAddr);
	//����һ�� FInternetAddr ���IP�Ͷ˿�
	TSharedPtr<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ipAddr.Value);
	addr->SetPort(port);
	//����controlSocket
	controlSocket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("control"), false);
	//���ӷ�����
	if (controlSocket->Connect(*addr))
	{
		FString RecvMesg;
		TArray<uint8> RecvBinary;
		ReceiveData(controlSocket, RecvMesg, RecvBinary);
		return true;
	}
	Print("controlSocket connect failed!");
	if(controlSocket)
	{
		controlSocket->Close();
		controlSocket = nullptr;
	}
	return false; 
}

//����������ɿ������ӷ��ͣ������ϴ����������������ӷ��ͣ����ϴ��ļ���STOR �����ɿ������ӷ��ͣ��ļ��������������ӷ��ͣ�
bool FtpClientManager::FTP_SendCommand(const EFtpCommandType& cmdtype, const FString& Param)
{
	FString strCommand = SwitchCommand(cmdtype, Param);
	TCHAR* serializedChar = strCommand.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar) + 1;
	int32 sent = 0;
	if (controlSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent))
	{
		Print("SendCommand  " + Param + "  succeed!");
		FString RecvMesg;
		TArray<uint8> RecvBinary;
		ReceiveData(controlSocket, RecvMesg, RecvBinary);
		return true;
	}
	Print("SendCommand " + Param + " failed!");
	return false;
}

bool FtpClientManager::FTP_ListFile(const FString& serverPath, TArray<FString>& OutFiles, bool bIncludeFolder)
{
	//������ļ��Ͳ�����ִ��
	FString Extension = FPaths::GetExtension(serverPath,false);
	if(!Extension.IsEmpty())
	{
		OutFiles.Add(serverPath);
		return true;
	}
	FString Mesg;
	TArray<uint8> RecvBinary;
	//�ȷ���PASVָ��
	int32 PasvPort = SendPASVCommand();
	//������������
	if (false == CreateDataSocket_PASV(PasvPort))
	{
		if (dataSocket)
		{
			dataSocket->Close();
			dataSocket = nullptr;
		}
		return false;
	}
	if (false == FTP_SendCommand(EFtpCommandType::LIST, serverPath))
	{
		if (dataSocket)
		{
			dataSocket->Close();
			dataSocket = nullptr;
		}
		return false;
	}
	ReceiveData(dataSocket, Mesg, RecvBinary);
	if (dataSocket)
	{
		dataSocket->Close();
		dataSocket = nullptr;
	}
	////δ�ҵ��ļ��и����ļ��У����ݿ��ƽ��յ��Ĵ�����Ϣ��һ���ģ�������Ҫ�ÿ��������յ��ķ�����Ϣ�ж�
	if(Mesg.Contains("Error"))
	{
		if((LIST_SUCCEED == ResponseCode) && bIncludeFolder)
		{
			//ִ�е������ʾ���б�Ϊ���б�,ֱ�����
			OutFiles.Add(serverPath);
			return true;
		}
		return false;
	}
	//��ʼ�ݹ�
	//  -rw - r--r-- 1 ftp ftp             16 May 12 23:42 11111.txt
	//	- rw - r--r-- 1 ftp ftp            57 May 12 23:43 2222222.txt
	//	drwxr - xr - x 1 ftp ftp           0 May 16 23:54 EE
	TArray<FString> RecvStringArr;		//������������ص�ԭʼ����
	FString TemptxtFile = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()) / TEXT("Templist.txt");
	FFileHelper::SaveStringToFile(Mesg,*(TemptxtFile));
	FFileHelper::LoadFileToStringArray(RecvStringArr,*TemptxtFile);
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	PlatformFile.DeleteFile(*TemptxtFile);
	auto getfilename = [](const FString& split, const FString& SplitString)->FString
	{
		FString l,r;
		SplitString.Split(split,&l,&r);
		return r;
	};
	/*
	* 11111.txt
	* 2222222.txt
	* EE
	*/
	TArray<FString> FileNames; 
	for(const auto& temp : RecvStringArr)
	{
		if(!temp.Contains(":"))
			continue;
		FString TempSplit,Filename;
		TempSplit = getfilename(TEXT(":"), temp);
		Filename = getfilename(TEXT(" "), TempSplit);
		FileNames.Add(Filename);
	}
	//�ļ������������ļ��м����ݹ�
	for (const auto& TempFile:FileNames)
	{
		Extension = FPaths::GetExtension(TempFile,false);
		if(!Extension.IsEmpty())
		{
			OutFiles.Add(serverPath / TempFile);
		}
		else
		{
			FTP_ListFile(serverPath / TempFile, OutFiles, bIncludeFolder);
		}
	}
	return true;
}

bool FtpClientManager::FTP_DownloadOneFile(const FString& serverFileName, const FString& localSavePath)
{
	bool bSuccessed = true;
	FString Mesg;
	TArray<uint8> RecvBinary;
	FString FileSavName = localSavePath / serverFileName;
	//�ȷ���PASVָ��
	int32 PasvPort = SendPASVCommand();
	//������������
	if(false == CreateDataSocket_PASV(PasvPort))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	//�����ļ�����   ����ͳɹ�����û������ļ�������550
	if((false == FTP_SendCommand(EFtpCommandType::RETR, serverFileName)) || (ERROR_DIRECTORY == ResponseCode))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	ReceiveData(dataSocket, Mesg, RecvBinary); 
	if(FFileHelper::SaveArrayToFile(RecvBinary, *FileSavName))
	{
		Print("Download succeed!",100.f,FColor::Purple);
		bSuccessed = true;
	}
	else
	{
		Print("Download " + serverFileName +"failed!", 100.f, FColor::Red);
		bSuccessed = false;
	}
_Program_Endl:
	if(dataSocket)
	{
		dataSocket->Close();
		dataSocket = nullptr;
	}
	return bSuccessed;
}

bool FtpClientManager::FTP_DownloadFiles(const FString& serverFolder, const FString& localSavePath)
{
	EFileType fileType = JudgeserverPath(serverFolder);
	bool bSuccessed = false;
	TArray<FString> FileArr; 
	switch (fileType)
	{
	case EFileType::FOLDER:
		if(FTP_ListFile(serverFolder, FileArr,false))
		{
			for (const auto& Tempfilaname : FileArr)
			{
				bSuccessed = FTP_DownloadOneFile(Tempfilaname,localSavePath);
				if (!bSuccessed)
					return false;
			}
		}
		break;
	case EFileType::FILE:
		bSuccessed = FTP_DownloadOneFile(serverFolder, localSavePath);
		break;
	}
	return bSuccessed;
}

bool FtpClientManager::FTP_UploadOneFile(const FString& localFileName)
{
	bool bSuccessed = true;
	FString FilaName = FPaths::GetCleanFilename(localFileName);
	FString SendStr;
	TCHAR* serializedChar;
	int32 size = 0;
	int32 SendByte = 0;
	//�������ӷ����ļ�����:�Ȱ��ļ�ת���ɶ��������ݣ���ͨ��datasocket����  
	if(FFileHelper::LoadFileToString(SendStr,*localFileName))
	{
		serializedChar = SendStr.GetCharArray().GetData();
		size = (int32)getFileSize(localFileName);
	}
	else
	{
		return false;
	}
	//�ȷ���PASVָ��
	int32 PasvPort = SendPASVCommand();
	//������������
	if (false == CreateDataSocket_PASV(PasvPort))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	//�������� ��������
	if (false == FTP_SendCommand(EFtpCommandType::STOR, FilaName))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	//���ݿ��� ��������
	bSuccessed = dataSocket->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, SendByte);
_Program_Endl:
	if (dataSocket)
	{
		dataSocket->Close();
		dataSocket = nullptr;

		FString Mesg;
		TArray<uint8> RecvBinary;
		//���շ���˷����� 226 Successfully transferred "/ceshi.uasset" ��Ϣ  ������Ϣ�����ݿ��ƶϿ���ʱ�����˲Żᷢ��
		ReceiveData(controlSocket, Mesg, RecvBinary);
	}
	return bSuccessed;
}

bool FtpClientManager::FTP_UploadFiles(const FString& localPath)
{
	bool bSucceed = false;
	TArray<FString> localFiles;  //����·���µ������ļ�
	EFileType filetype = JudgeserverPath(localPath);
	switch (filetype)
	{
	case EFileType::FOLDER:
		if(ListAllFileFromLocalPath(localPath,localFiles))
		{
			for (const auto& Tempfilename : localFiles)
			{
				bSucceed = FTP_UploadOneFile(Tempfilename);
				if(!bSucceed)
					return false;
			}
		}
		break;
	case EFileType::FILE:
		bSucceed = FTP_UploadOneFile(localPath);
		break;
	}
	return bSucceed;
}

#if PLATFORM_WINDOWS
#pragma optimize("",on) 
#endif
#include "FtpClient/FtpClient.h"
#include "Engine.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "HAL/FileManager.h"
#include "HAL/PlatformFilemanager.h"
#include <fstream>
#include "AssetRegistryModule.h"
#include "Misc/MessageDialog.h"


#if WITH_EDITOR
#if PLATFORM_WINDOWS
#pragma optimize("",off) 
#endif
#endif


//�ж� ��Դ �Ƿ����ڵ�������Դ
bool IsThirdPartyAsset(const FString& InAssetpath)
{
	//������ ��/Com_�� ����/Ins_��
	if (!InAssetpath.Contains(TEXT("/Com_")) && !InAssetpath.Contains(TEXT("/Ins_")))
		return true;
	else
		return false;
}

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
	if (GEngine && GetDefault<UFtpConfig>()->bShowServerMesg)
	{
		GEngine->AddOnScreenDebugMessage(-1, Time, Color, Mesg);
	}
}

void FtpClientManager::Print(const TArray<uint8>& dataArray, float Time, FColor Color)
{
	if (GEngine && GetDefault<UFtpConfig>()->bShowServerMesg)
	{

		GEngine->AddOnScreenDebugMessage(-1, Time, Color, BinaryArrayToString(dataArray));
	}
}

void FtpClientManager::DeleteUselessFile()
{
	FString ProjContent = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	TArray<FString> AllFiles;
	FString Suffix = TEXT("*") + GetDefault<UFtpConfig>()->Suffix;
	IFileManager::Get().FindFilesRecursive(AllFiles, *ProjContent, *Suffix, true, false);
	for (const auto& temp : AllFiles)
	{
		FString Copyfile = temp.Replace(*(GetDefault<UFtpConfig>()->Suffix), TEXT(".uasset"));
		if (!IFileManager::Get().FileExists(*Copyfile))
		{
			FString FileName = FPaths::GetCleanFilename(Copyfile);  // xxx.uasset
			Copyfile.RemoveFromEnd(TEXT("/") + FileName);
			FileName.RemoveFromEnd(TEXT(".uasset"));  // xxx
			FString LastFolderName = FPaths::GetCleanFilename(Copyfile);
			if(!FileName.Equals(LastFolderName)) //�ж��ǲ���ʵ�������ļ�
				IFileManager::Get().Delete(*temp);
		}
	}
	
}

void FtpClientManager::ShowMessageBox(const TArray<FString>& NameNotValidFiles, const TArray<FInvalidDepInfo>& DepenNotValidFiles)
{
	if (NameNotValidFiles.Num())
	{
		FString AllStr;
		for (const auto& TEmpNameNotValid : NameNotValidFiles)
		{
			AllStr += TEmpNameNotValid + TEXT("\n");
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The following asset name is not valid:\n\n" + AllStr));
	}
	if (DepenNotValidFiles.Num())
	{
		FString AllStr;
		for (const auto& TEmpDepenNotValid : DepenNotValidFiles)
		{
			FString OneInvalidStr = TEXT("\n") + TEmpDepenNotValid.DepInvalidAssetName + TEXT(" :\n");
			for(const auto& temp : TEmpDepenNotValid.InvalidDepAsset)
			{
				OneInvalidStr += temp + TEXT("\n");
			}
			AllStr += OneInvalidStr + TEXT("\n");
		}
		FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("The following asset has invalid dependences:\n" + AllStr + "\nyou can try to copy the invalid dependences to the common folder or your instance folder."));
	}	
}

bool FtpClientManager::UploadInstanceDescriptToWeb(const FString& InFolderPath, const TArray<FString>& ThirdFolders)
{
	FWebSendData WebData;
	TArray<uint8> ImageData;
	FString ImageBase64;
	FString Description = GetDefault<UFtpConfig>()->InstanceDescription;
	FString WebUrl = GetDefault<UFtpConfig>()->WebURL;	
	FString InstanceIconPath = GetDefault<UFtpConfig>()->InstanceIcon.FilePath;
	FString Extension = FPaths::GetExtension(InstanceIconPath, false);
	FFileHelper::LoadFileToArray(ImageData, *InstanceIconPath);
	ImageBase64 = FBase64::Encode(ImageData);
	FString Preffix = TEXT("data:image/") + Extension + TEXT(";base64,");
	FString InsFolderName = InFolderPath;
	InsFolderName.RemoveFromStart(TEXT("/Game/"));
	WebData.describe = Description;
	WebData.name = InsFolderName;  // Instance/ProjA
	WebData.filePath = InsFolderName;
	WebData.file = Preffix + ImageBase64;
	FString Json = WebData.ConvertToString();

	FString Tips;
	if(InstanceIconPath.IsEmpty() || Description.IsEmpty())
	{
		Tips = TEXT("You have some Instance Description that you didn't fill in.");
		FMessageDialog::Open(EAppMsgType::Ok,FText::FromString(Tips));
		return false;
	}
	else
	{
		Tips = TEXT("Have you set up the instance submit description for this submission? \"No\" to stop upload and set up these information in project setting.");
		EAppReturnType::Type returntype;
		returntype = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Tips));
		if(returntype != EAppReturnType::Type::Yes)
			return false;
	}

	if (ThirdFolders.Num())
	{
		FString Str;
		for (const auto& temp : ThirdFolders)
		{
			Str += temp + TEXT("\n");
		}
		Tips = TEXT("you have referenced third-party resource:\n") + Str +TEXT("Have you set up the submit description for these third-party submission?\n \"No\" to stop upload and set up these information in project setting.");
		EAppReturnType::Type returntype;
		returntype = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(Tips));
		if (returntype != EAppReturnType::Type::Yes)
			return false;

		bool bCompleted;
		TArray<FSubmitThirdPartyInfo> ThirdPartyInfo = GetDefault<UFtpConfig>()->ThirdPartyDescriptions;
		for (const auto& temp : ThirdFolders)
		{
			bCompleted = false;
			for (const auto& tempthird : ThirdPartyInfo)
			{
				if (temp.Equals(tempthird.ThirdPartyName))
					bCompleted = true;
			}
			if (!bCompleted)
			{
				Tips = TEXT("please set the third-party folder name correctly in the project settings ");
				FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Tips));
				return false;
			}
		}
	}
	UploadThirdFolderDescriptToWeb(ThirdFolders);
	return HTTP_INSTANCE->PostIconAndDesc(WebUrl, Json);
}

bool FtpClientManager::UploadThirdFolderDescriptToWeb(const TArray<FString>& InThirdPath)
{
	//�����������Դ�ļ�����
	for (const auto& tempfolder : InThirdPath)
	{
		for (const auto& tempDescrip : GetDefault<UFtpConfig>()->ThirdPartyDescriptions)
		{
			if (tempfolder.Equals(tempDescrip.ThirdPartyName))
			{
				FWebSendData WebData;
				TArray<uint8> ImageData;
				FString ImageBase64;
				FString Description = tempDescrip.ThirdPartyDescription;
				FString WebUrl = GetDefault<UFtpConfig>()->WebURL;
				FString ThirdPartyIcon = tempDescrip.IconPath.FilePath;
				FString Extension = FPaths::GetExtension(ThirdPartyIcon, false);
				FFileHelper::LoadFileToArray(ImageData, *ThirdPartyIcon);
				ImageBase64 = FBase64::Encode(ImageData);
				FString Preffix = TEXT("data:image/") + Extension + TEXT(";base64,");
				WebData.describe = Description;
				WebData.name = tempfolder;
				WebData.filePath = tempfolder;
				WebData.file = Preffix + ImageBase64;
				FString Json = WebData.ConvertToString();
				HTTP_INSTANCE->PostIconAndDesc(WebUrl, Json);
			}
		}
	}
	return true;
}

FtpClientManager* FtpClientManager::ftpInstance = nullptr;

FtpClientManager::FtpClientManager()
{ 
	controlSocket = nullptr;
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

void FtpClientManager::Initialize_Folder()
{
	FString ProjContentFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	DataTypeIni = FPaths::ConvertRelativePathToFull(FPaths::ProjectConfigDir() / TEXT("DefaultDataTypeConfig.ini"));
	//�����ļ���
	FString CommonSKM = ProjContentFullPath + TEXT("Com_SkeletalMesh");
	FString CommonSTM = ProjContentFullPath + TEXT("Com_StaticMesh");
	FString CommonMAT = ProjContentFullPath + TEXT("Com_Material");
	FString CommonTEX = ProjContentFullPath + TEXT("Com_Texture");
	FString CommonANIM = ProjContentFullPath + TEXT("Com_Animation");
	FString CommonMAP = ProjContentFullPath + TEXT("Map");

	//��Ŀʵ��
	FString Instance = ProjContentFullPath + TEXT("Instance");

	IPlatformFile& filePlatform = FPlatformFileManager::Get().GetPlatformFile();
	if(!filePlatform.FileExists(*DataTypeIni))
		FFileHelper::SaveStringToFile(TEXT(""),*DataTypeIni);
	if(!filePlatform.DirectoryExists(*CommonSKM))
		filePlatform.CreateDirectory(*CommonSKM);
	if (!filePlatform.DirectoryExists(*CommonSTM))
		filePlatform.CreateDirectory(*CommonSTM);
	if (!filePlatform.DirectoryExists(*CommonMAT))
		filePlatform.CreateDirectory(*CommonMAT);
	if (!filePlatform.DirectoryExists(*CommonTEX))
		filePlatform.CreateDirectory(*CommonTEX);
	if (!filePlatform.DirectoryExists(*CommonANIM))
		filePlatform.CreateDirectory(*CommonANIM);
	if (!filePlatform.DirectoryExists(*CommonMAP))
		filePlatform.CreateDirectory(*CommonMAP);
	if (!filePlatform.DirectoryExists(*Instance))
		filePlatform.CreateDirectory(*Instance);
}

void FtpClientManager::CreateInstanceFolder(const FString& InstanceName)
{
	FString ProjContentFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	FString ProjectInstancePath = ProjContentFullPath +TEXT("Instance/") + InstanceName;
	FString InstanceSKM = ProjectInstancePath + TEXT("/Ins_SkeletalMesh");
	FString InstanceSTM = ProjectInstancePath + TEXT("/Ins_StaticMesh");
	FString InstanceMAT = ProjectInstancePath + TEXT("/Ins_Material");
	FString InstanceTEX = ProjectInstancePath + TEXT("/Ins_Texture");
	FString InstanceANI = ProjectInstancePath + TEXT("/Ins_Animation");
	IPlatformFile& filePlatform = FPlatformFileManager::Get().GetPlatformFile();

	if (!filePlatform.DirectoryExists(*ProjectInstancePath))
		filePlatform.CreateDirectory(*ProjectInstancePath);
	if (!filePlatform.DirectoryExists(*InstanceSKM))
		filePlatform.CreateDirectory(*InstanceSKM);
	if (!filePlatform.DirectoryExists(*InstanceSTM))
		filePlatform.CreateDirectory(*InstanceSTM);
	if (!filePlatform.DirectoryExists(*InstanceMAT))
		filePlatform.CreateDirectory(*InstanceMAT);
	if (!filePlatform.DirectoryExists(*InstanceTEX))
		filePlatform.CreateDirectory(*InstanceTEX);
	if (!filePlatform.DirectoryExists(*InstanceANI))
		filePlatform.CreateDirectory(*InstanceANI);
}

FString FtpClientManager::GetMylocalIPADDR()
{
	FString Myip("NONE");
	bool bCanBind;
	TSharedPtr<FInternetAddr> localIP = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);
	if(localIP->IsValid())
	{
		Myip = localIP->ToString(false);
	}
	return Myip;
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
		RecvData.Init(element, FMath::Min(size, 65536u));
		int32 read = 0;
		sock->Recv(RecvData.GetData(), RecvData.Num(), read);
		dataArray += RecvData;
	}
	if (dataArray.Num() <= 0)
	{
		RecvMesg = TEXT("Error : RecvData is empty.");
		Print(RecvMesg);
		return false;
	}
	const FString ReceivedUE4String = BinaryArrayToString(dataArray);
	RecvMesg = ReceivedUE4String;
	//dataArray = RecvData;
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

FString FtpClientManager::BinaryArrayToString(const TArray<uint8>& BinaryArray)
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

bool FtpClientManager::GetAllFileFromLocalPath(const FString& localPath, TArray<FString>& AllFiles, bool bRecursively)
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

bool FtpClientManager::CreateDirByAsssetPath(const FString& InAssetFullPath)
{
	FString Dir = InAssetFullPath;
	FString AssetName = FPaths::GetCleanFilename(InAssetFullPath);
	FString FullProjPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	if (Dir.RemoveFromStart(FullProjPath))
	{
		Dir.RemoveFromEnd(AssetName);
	}
	if(FTP_SendCommand(EFtpCommandType::MKD, Dir))
	{
		if(FTP_SendCommand(EFtpCommandType::CWD, Dir))
		{
			if (ERROR_DIRECTORY == ResponseCode)
				return false;
			return true;
		}
	}
	return false;
}

bool FtpClientManager::DeleteFileOrFolder(const FString& InDir)
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
			for (const auto& Temp : RemoveArray)
			{
				Extension = FPaths::GetExtension(Temp);
				if (!Extension.IsEmpty())
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
			DeleteFileOrFolder(InDir);
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

bool FtpClientManager::FileNameValidationOfOneFolder(TArray<FString>& NoValidFiles, const FString& InFullFolderPath)
{
	bool bAllValid = true;
	auto GetFolderType = [](FString InPath)->EFolderType
	{
		InPath.RemoveFromEnd(TEXT("/"));
		FString FolderName = FPaths::GetCleanFilename(InPath);
		if (FolderName.Contains(TEXT("_Texture")))
		{
			return EFolderType::TEXTURE;
		}
		else if (FolderName.Contains(TEXT("_Material")))
		{
			return EFolderType::MATERIAL;
		}
		else if (FolderName.Contains(TEXT("_Animation")))
		{
			return EFolderType::ANIMATION;
		}
		else if (FolderName.Contains(TEXT("_SkletalMesh")))
		{
			return EFolderType::SKLETALMESH;
		}
		else if (FolderName.Contains(TEXT("_StaticMesh")))
		{
			return EFolderType::STATICMESH;
		}
		else
		{
			return EFolderType::ERROR_FOLDER;
		} 
	};

	TArray<FString> AllFilePaths;  //�����ļ����������ļ��ľ���·��
	TArray<FString> AllFileNames;  //�����ļ����������ļ���

	TArray<FString> numArr1; //���������������жϱ���Ƿ����ظ�  
	TArray<FString> numArr2;

	FString JsonStr;
	FDataInfoList InfoList;
	FFileHelper::LoadFileToString(JsonStr, *DataTypeIni);
	JsonStr.ToUpperInline();
	if (!SimpleFtpDataType::ConvertStringToStruct(JsonStr, InfoList))
		return false;

	GetAllFileFromLocalPath(InFullFolderPath, AllFilePaths, true);
	EFolderType type = GetFolderType(InFullFolderPath);
	for (const auto& TempPath : AllFilePaths)
	{
		//   D:/Work/UE_4.22/UnrealProjects/HotUpdate/Content/Instance/Mat_Wood_0_Description.uasset
		FString Extension = FPaths::GetExtension(TempPath, true);  // .uasset
		if (!Extension.Equals(TEXT(".uasset")))
		{
			continue;
		}
		FString FileName = FPaths::GetCleanFilename(TempPath);     //Mat_Wood_0_Description.uasset
		FileName.RemoveFromEnd(Extension);						   //Mat_Wood_0_Description
		AllFileNames.Add(FileName);
	}
	if (0 == AllFileNames.Num())
		return true;

	switch (type)
	{
	case EFolderType::ANIMATION:
		//��ʼ�ж��ļ������Ƿ�Ϸ�
		NAME_VALIDATION_FOLDER("ANIM")
		break;
	case EFolderType::MATERIAL:
		//��ʼ�ж��ļ������Ƿ�Ϸ�
		NAME_VALIDATION_FOLDER("MAT")
		break;
	case EFolderType::SKLETALMESH:
		//��ʼ�ж��ļ������Ƿ�Ϸ�
		NAME_VALIDATION_FOLDER("SKM")
		break;
	case EFolderType::STATICMESH:
		//��ʼ�ж��ļ������Ƿ�Ϸ�
		NAME_VALIDATION_FOLDER("STM")
		break;
	case EFolderType::TEXTURE:
		//��ʼ�ж��ļ������Ƿ�Ϸ�
		NAME_VALIDATION_FOLDER("TEX")
		/*for (const auto& TempName : AllFileNames)
		{
			FString UperFileName = TempName.ToUpper(); 
			numArr1.Add(TempName); 
			TArray<FString> partArr; 
			UperFileName.ParseIntoArray(partArr, TEXT("_"), false); 
			if (partArr.Num() != 4)
			{
				bAllValid = false; 
				NoValidFiles.Add(TempName); 
				numArr2.Add(FGuid::NewGuid().ToString());
				continue;
			}
			if (!partArr[0].Equals(TEXT("TEX")))
			{
				bAllValid = false;
				NoValidFiles.Add(TempName);
				numArr2.Add(FGuid::NewGuid().ToString());
				continue;
			}
			bool bCorrect = false;
			for (const auto& TempAeestType : InfoList.DATATYPRARR)
			{
				if (!(partArr[1].Equals(TempAeestType.TYPENAME)) && !(partArr[1].Equals(TempAeestType.TYPEABBR)))
				{
					continue;
				}
				else
				{
					bCorrect = true;
					break;
				}
			}
			if (!bCorrect)
			{
				bAllValid = false;
				NoValidFiles.Add(TempName);
				numArr2.Add(FGuid::NewGuid().ToString());
				continue;
			}
			numArr2.AddUnique(partArr[2]);
			if (numArr1.Num() != numArr2.Num())
			{
				bAllValid = false;
				NoValidFiles.Add(TempName);
				numArr2.Add(FGuid::NewGuid().ToString());
			}
		}*/
		break;
	default:
		bAllValid = false;
		break;
	}
	return bAllValid;
}

// InPackageName �� /Game/Instance/Ins_Material/Mat_Wood_1_sad
void FtpClientManager::RecursiveFindDependence(const FString& InPackageName, TArray<FString>& AllDependence)
{
	//�������
	//AllDependence.Add(InPackageName);
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	TArray<FName> OutDependencies;
	AssetRegistryModule.GetDependencies(*InPackageName, OutDependencies, EAssetRegistryDependencyType::Packages);
	for (auto& TempDepen : OutDependencies)
	{
		if (TempDepen.IsValid())
		{
			FString TempDepenStr = TempDepen.ToString();
			if (!AllDependence.Contains(TempDepenStr))
			{
				AllDependence.AddUnique(TempDepenStr);
				RecursiveFindDependence(TempDepenStr, AllDependence);
			}
		}
	}
}

bool FtpClientManager::ValidationDependenceOfOneAsset(const FString& InGamePath, const FString& AssetPackName, const TArray<FString>& TheAssetDependence, FInvalidDepInfo& InvalidInfo, bool& bDepHasChanged, bool bAllNameValid)
{
	bool bValid = true;
	bDepHasChanged = false;
	//������Դ�������
	if (InGamePath.Contains(TEXT("Com_")))
	{
		for (const auto& TempDep : TheAssetDependence)
		{
			//������Դ������������ʵ�� �Լ�������
			if(!TempDep.Contains("/Com_"))  // || TempDep.Contains(TEXT("/Engine/")) || TempDep.Contains(TEXT("/Script/")) || IsThirdPartyAsset(TempDep))
			{
				bValid = false;
				InvalidInfo.DepInvalidAssetName = AssetPackName;
				InvalidInfo.InvalidDepAsset.Add(TempDep);
			}
		}
	}
	//ʵ��
	else if(InGamePath.Contains(TEXT("/Instance/")))
	{
		//ʵ����Դ�������
		FString InstName = FPaths::GetCleanFilename(InGamePath);
		for (const auto& TempDep : TheAssetDependence)
		{
			if (TempDep.Contains("/Engine/") || TempDep.Contains(TEXT("/Script/")))
			{
				bValid = false;
				InvalidInfo.DepInvalidAssetName = AssetPackName;
				InvalidInfo.InvalidDepAsset.Add(TempDep);
			}
			if (!TempDep.Contains(InstName))
			{
				//ɸѡ����������ʵ����Դ���ļ�
				if (TempDep.Contains("/Instance/"))
				{
					bValid = false;
					InvalidInfo.DepInvalidAssetName = AssetPackName;
					InvalidInfo.InvalidDepAsset.Add(TempDep);
				}
			}
		}
	}
	//������ 
	else
	{
		for (const auto& TempDep : TheAssetDependence)
		{
			if (TempDep.Contains("/Com_") || TempDep.Contains(TEXT("/Ins_")))
			{
				bValid = false;
				InvalidInfo.DepInvalidAssetName = AssetPackName;
				InvalidInfo.InvalidDepAsset.Add(TempDep);
			}
		}
	}
	//���������ļ�  Json��ʽ������Ϊ �����ļ����� ÿһ��������Դ����һ��32λУ����  ������ڲ��Ϸ�������Ҳ�����������ļ�
	if (bAllNameValid)
	{
		auto ConvertTimeToStr = [](const FDateTime& DataTime) ->FString
		{
			FString Str;
			Str.Append(FString::FromInt(DataTime.GetYear()) + TEXT("/"));
			Str.Append(FString::FromInt(DataTime.GetMonth()) + TEXT("/"));
			Str.Append(FString::FromInt(DataTime.GetDay()) + TEXT("_"));
			Str.Append(FString::FromInt(DataTime.GetHour() + 8) + TEXT(":"));
			Str.Append(FString::FromInt(DataTime.GetMinute()) + TEXT(":"));
			Str.Append(FString::FromInt(DataTime.GetSecond()) + TEXT(":"));
			Str.Append(FString::FromInt(DataTime.GetMillisecond()));
			return Str;
		};
		FString FileName = AssetPackName;
		FString UAssetName;
		FString contentName = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
		if (FileName.ReplaceInline(TEXT("/Game/"), *contentName))
		{
			FileName.Append(GetDefault<UFtpConfig>()->Suffix);
			UAssetName = FileName.Replace(*(GetDefault<UFtpConfig>()->Suffix), TEXT(".uasset")); 
			FDependenList depenlist;
			FString Json;
			if (IFileManager::Get().FileExists(*FileName))
			{
				FDateTime DataTime1 = IFileManager::Get().GetTimeStamp(*UAssetName);
				FString ModifyTime = ConvertTimeToStr(DataTime1);
				FFileHelper::LoadFileToString(Json, *FileName);
				//������Ϊ���ж���Դ��������û�з����ı䣬û�ı�Ͳ���������У���룬�����´���.dep�ļ�
				if (SimpleFtpDataType::ConvertStringToStruct(Json, depenlist))
				{
					if (!ModifyTime.Equals(depenlist.LastModifyTime))
					{
							GENERATE_DEP_FILE();
					}
				}
			}
			else
			{
					GENERATE_DEP_FILE();
			}
		}
	}
	return bValid;
}

//��ߴ����·���������·��   �磺/Game/Instance/ProjA    �ҵ����������� PackageName
bool FtpClientManager::ValidationAllDependenceOfTheFolder(const FString& InGamePath, TArray<FInvalidDepInfo>& NotValidDependences, bool bAllNameValid)
{
	bool bAllDependenceValid = true;
	bool bDepHasChanged = false;
	FString CopyTemp = InGamePath;
	TArray<FString> AllDependences;
	//�Ȱ����·��ת���ɾ���·�� 
	//�Ƴ�  /Game/
	if (CopyTemp.RemoveFromStart(TEXT("/Game/")))
	{
		CopyTemp = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / CopyTemp);
		//�ļ����µ���Դ·��
		TArray<FString> UassetSource;
		TArray<FString> ContentAssetPaths;
		IFileManager::Get().FindFilesRecursive(ContentAssetPaths, *CopyTemp, TEXT("*"), true, false);
		for (const auto& temp : ContentAssetPaths)
		{
			if (temp.Contains(".uasset"))
				UassetSource.Add(temp);
		}
		for (const auto& Tempasset : UassetSource)
		{
			TArray<FString> TheAssetDependence;
			FString PackageName = FPackageName::FilenameToLongPackageName(Tempasset);
			RecursiveFindDependence(PackageName, TheAssetDependence);
			for (const auto& temp : TheAssetDependence)
			{
				AllDependences.AddUnique(temp);
			}
			//������Դ�������Ƿ�Ϸ�
			FInvalidDepInfo InvalidInfo;
			bool blocalVar;
			if (!ValidationDependenceOfOneAsset(InGamePath, PackageName, TheAssetDependence, InvalidInfo, blocalVar, bAllNameValid))
			{
				NotValidDependences.Add(InvalidInfo);
				bAllDependenceValid = false;
			}
			if (blocalVar)
				bDepHasChanged = true;
		}
	}
	else
		bAllDependenceValid = false;
	
	//����ϴ�����ʵ���ļ��У����ʵ�����������õ���common�ļ�����Դ��PackageName �Լ� ���õ��ĵ�������Դ������Դ��������  ���浽 ProjA.dep  
	{
		if (InGamePath.Contains(TEXT("/Instance/")))
		{
			FString InstName = FPaths::GetCleanFilename(InGamePath);
			FString InstConfigName = CopyTemp / InstName + GetDefault<UFtpConfig>()->Suffix;
			FInstanceInfo InstInfo;
			FString Json;
		
			if (IFileManager::Get().FileExists(*InstConfigName))
			{
				if (bDepHasChanged)  //����ļ����µ�������Դ�����ö�û�з����ı䣬��ô�����ļ��������õĹ�����Դ�͵�������Դ�϶�û�䣬��ô�Ͳ���Ҫ�޸�У����
				{
					GENERATE_INST_FILE();
				}
			}
			else
			{
				GENERATE_INST_FILE();
			}
		}
	}
	return bAllDependenceValid;
}

//�ϴ��ļ��Լ������ļ� ������Դ��PackageName����
bool FtpClientManager::UploadDepenceAssetAndDepences(const TArray<FString>& InPackageNames)
{
	for (const auto& pakname : InPackageNames)
	{
		FString FileName = pakname;
		if (FileName.RemoveFromStart(TEXT("/Game/")))
		{
			FileName = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / FileName);
			FileName.Append(".uasset");
			if (!FTP_UploadOneFile(FileName))
				return false;
			FString Suffix = GetDefault<UFtpConfig>()->Suffix;
			FileName.ReplaceInline(TEXT(".uasset"), *Suffix);
			if (IFileManager::Get().FileExists(*FileName))		 //������ϴ�����Դ����������Դû�����������ļ�
				if (!FTP_UploadOneFile(FileName))
					return false;
		}
	}
	return true;
}

bool FtpClientManager::OverrideAssetOnServer(const FString& FileFullPath)
{
	bool bUpload = true;
	FString FileNameOnServer = FileFullPath;
	if (FileNameOnServer.Contains(TEXT("Com_")))   //ֻ�й����ļ����µ���Դ����Ҫһ��һ����⣬�������ʵ���ļ��еĻ�ֻ��Ҫ��֤ ʵ��.dep �ļ���У����
	{
		FileNameOnServer.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()));
		FTP_SendCommand(EFtpCommandType::SIZE, FileNameOnServer);
		if (FILE_EXIST == ResponseCode)
		{
			//�ļ����� ������ʾ�Ƿ񸲸Ƿ�����
			EAppReturnType::Type returntype;
			FString OptionStr = TEXT("the file  '") + FileNameOnServer + TEXT("'  is already exist on server, override it?");
			returntype = FMessageDialog::Open(EAppMsgType::YesNo, FText::FromString(OptionStr));
			switch (returntype)
			{
			case EAppReturnType::Type::Yes:
				bUpload = true;
				break;
			case EAppReturnType::Type::No:
				bUpload = false;
				break;
			default:
				bUpload = false;
				break;
			}
		}
	}
	//������ ֱ���ϴ�
	return bUpload;
}

bool FtpClientManager::IsAssetValidCodeSame(const FString& FileFullPath)
{
	FString LocalValidCode;
	FString ServerValidCode;
	FString DepLocalFullPath;
	FString DepServerPath;
	FString Json;
	FDependenList DepInfo;
	//�����ļ����µ���Դ�Ż�У�������Դ��У����
	if (FileFullPath.Contains(TEXT("/Com_")))
	{
		if (FileFullPath.Contains(TEXT(":")))
		{
			//�ϴ� ������Դ����·��
			DepLocalFullPath = FileFullPath;
			DepLocalFullPath.ReplaceInline(TEXT(".uasset"), *(GetDefault<UFtpConfig>()->Suffix));
			DepServerPath = DepLocalFullPath;
			DepServerPath.RemoveFromStart(FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()));
			GET_ASSET_VALIDCODE()
		}
		else
		{
			//����
			// Com_Material/Mat_wood_0_sad.uasset
			DepServerPath = FileFullPath.Replace(TEXT(".uasset"), *(GetDefault<UFtpConfig>()->Suffix));
			DepLocalFullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + DepServerPath;
			GET_ASSET_VALIDCODE()
		}
	}	
	return (LocalValidCode.Equals(ServerValidCode)) && !(LocalValidCode.IsEmpty());
}

bool FtpClientManager::IsInstValidCodeSame(const FString& InstName)
{
	FString LocalValidCode;
	FString ServerValidCode;
	FString localfilename;
	FString serverfilename;
	FString clearnname = FPaths::GetCleanFilename(InstName);
	FString Json;
	FInstanceInfo instInfo;
	if (InstName.Contains(TEXT("/Game/")))
	{
		//�ϴ�  ������� /Game/Instance/ProjA
		//ת���ɾ���·������ȡProjA.dep
		localfilename = InstName;
		serverfilename = InstName / clearnname + GetDefault<UFtpConfig>()->Suffix;
		serverfilename.RemoveFromStart(TEXT("/Game/"));
		FString ProjContent = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
		localfilename.ReplaceInline(TEXT("/Game/"), *ProjContent);
		localfilename += TEXT("/") + clearnname + GetDefault<UFtpConfig>()->Suffix;
		GET_INST_VALIDCODE()
	}
	else
	{
		//���� ������� Instance/ProjA
		serverfilename = InstName / clearnname + GetDefault<UFtpConfig>()->Suffix;
		localfilename = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + InstName / clearnname + GetDefault<UFtpConfig>()->Suffix;
		GET_INST_VALIDCODE()
	}
	//У���벻Ϊ�գ��������
	return (LocalValidCode.Equals(ServerValidCode)) && !(LocalValidCode.IsEmpty());
}

void FtpClientManager::HasDepencyThirdAsset(const FString& InGamePath, TArray<FString>& ThirdPartyName)
{
	if (InGamePath.Contains(TEXT("/Com_")))
		return;
	FString FullPath = InGamePath;
	FString FolderName = FPaths::GetCleanFilename(InGamePath);
	if (FullPath.RemoveFromStart(TEXT("/Game/")))
	{
		FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / FullPath);
	}
	FString InstDepFileName = FullPath / FolderName + GetDefault<UFtpConfig>()->Suffix;
	FString Json;
	FInstanceInfo Instinfo;
	FFileHelper::LoadFileToString(Json, *InstDepFileName);
	if (SimpleFtpDataType::ConvertStringToStruct(Json, Instinfo))
	{
		if (Instinfo.ThirdPartyAssetPackageName.Num())
		{
			//��ʾ ���ύ���� ���� ������������Դ
			EAppReturnType::Type returntype;
			FString Str = TEXT("It is detected that you have referenced a third-party resource, upload all(Yes) or only upload dependency(No)?");
			returntype = FMessageDialog::Open(EAppMsgType::Type::YesNo, FText::FromString(Str));
			switch (returntype)
			{
			case EAppReturnType::No:
				Instinfo.UploadAllAsset = false;
				break;
			case EAppReturnType::Yes:
				Instinfo.UploadAllAsset = true;
				break;
			default:
				Instinfo.UploadAllAsset = false;
				break;
			}
		}
	}

	{
		//�����ϴ��ߵ�ѡ��
		Json.Empty();
		SimpleFtpDataType::ConvertStructToString(Instinfo, Json);
		FFileHelper::SaveStringToFile(Json, *InstDepFileName);
	}
	for (const auto& temp : Instinfo.ThirdPartyAssetPackageName)
	{
		TArray<FString> FolderLevel;
		temp.ParseIntoArray(FolderLevel, TEXT("/"));
		ThirdPartyName.AddUnique(FolderLevel[1]);
	}
}

void FtpClientManager::UploadThirdPartyFolder(const TArray<FString>& InFolders)
{
	for (const auto& tempfolder : InFolders)
	{
		FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir()) + tempfolder;
		TArray<FString> ThirdPartyAsset;
		IFileManager::Get().FindFilesRecursive(ThirdPartyAsset, *FullPath, TEXT("*"), true, false);
		for (const auto& temp : ThirdPartyAsset)
		{
			FTP_UploadOneFile(temp);
		}
	}
}  


/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
bool FtpClientManager::FTP_CreateControlSocket(const FString& IP, const int32& port)
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
	FPlatformProcess::Sleep(GetDefault<UFtpConfig>()->sleeptime);
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
		SplitString.Split(split, &l, &r);
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

bool FtpClientManager::FTP_DownloadOneFile(const FString& serverFileName)
{
	bool bSuccessed = true;
	FString Mesg;
	TArray<uint8> RecvBinary;
	FString FileSaveName = GetDefault<UFtpConfig>()->DownloadPath.Path + serverFileName;
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
	if(FFileHelper::SaveArrayToFile(RecvBinary, *FileSaveName))
	{
		Print("Download succeed!",100.f,FColor::Purple);
		bSuccessed = true;
	}
	else
	{
		Print("Download " + serverFileName + " failed!", 100.f, FColor::Red);
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

bool FtpClientManager::FTP_DownloadFiles(const FString& serverFolder)
{
	EFileType fileType = JudgeserverPath(serverFolder);
	bool bSuccessed = false;
	TArray<FString> FileArr; 
	switch (fileType)
	{
	case EFileType::FOLDER:
		if(FTP_ListFile(serverFolder, FileArr, false))
		{
			for (const auto& Tempfilename : FileArr)
			{
				bSuccessed = FTP_DownloadOneFile(Tempfilename);
				if (!bSuccessed)
					return false;
			}
		}
break;
	case EFileType::FILE:
		bSuccessed = FTP_DownloadOneFile(serverFolder);
		break;
	}
	return bSuccessed;
}

bool FtpClientManager::FTP_UploadOneFile(const FString& localFileName)
{
	bool bSuccessed = true;
	FString FileName = FPaths::GetCleanFilename(localFileName);
	int32 size = 0;
	int32 SendByte = 0;
	//�������ӷ����ļ�����:�Ȱ��ļ�ת���ɶ��������ݣ���ͨ��datasocket����
	TArray<uint8> sendData;
	if (!FFileHelper::LoadFileToArray(sendData, *localFileName))
		return false;
	//������Դ��Ӧ�ļ���.�����õ�ǰĿ¼
	if (!CreateDirByAsssetPath(localFileName))
		return false;
	//�ȷ���PASVָ��
	int32 PasvPort = SendPASVCommand();
	//������������
	if (false == CreateDataSocket_PASV(PasvPort))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	//�������� ��������
	if (false == FTP_SendCommand(EFtpCommandType::STOR, FileName))
	{
		bSuccessed = false;
		goto _Program_Endl;
	}
	//���ݿ��� ��������
	bSuccessed = dataSocket->Send(sendData.GetData(), sendData.Num(), SendByte);
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
	FTP_SendCommand(EFtpCommandType::CWD, TEXT("/"));
	return bSuccessed;
}

//InGamePath��/Game/Instance/ProjA
//InGamePath��/Game/Com_
bool FtpClientManager::FTP_UploadFilesByFolder(const FString& InGamePath, TArray<FString>& NameNotValidFiles, TArray<FInvalidDepInfo>& DepenNotValidFiles)
{
	// �����·����/Game/Instance/ProjA
	// ��ת���ɾ���·��
	FString FullPath = InGamePath;
	FString FolderName = FPaths::GetCleanFilename(InGamePath);
	if (FullPath.RemoveFromStart(TEXT("/Game/")))
	{
		FullPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / FullPath);
	}
	bool bAllValid = true;
	TArray<FString> ChildrenFolders;
	IFileManager::Get().FindFilesRecursive(ChildrenFolders, *FullPath, TEXT("*"), false, true);
	if (ChildrenFolders.Num())
	{
		for (const auto& tempchild : ChildrenFolders)
		{
			if (tempchild.Contains(TEXT("Com_")) || tempchild.Contains(TEXT("Ins_")))
			{
				if (!FileNameValidationOfOneFolder(NameNotValidFiles, tempchild))
					bAllValid = false;
			}
		}
	}
	else
	{
		if (!FileNameValidationOfOneFolder(NameNotValidFiles, FullPath))
			bAllValid = false;
	}
	//����ļ����� ��ʾ���Ϸ�����Դ����
	if (!ValidationAllDependenceOfTheFolder(InGamePath, DepenNotValidFiles, bAllValid))
	{
		bAllValid = false;
	}
	if (!bAllValid)
	{
		ShowMessageBox(NameNotValidFiles, DepenNotValidFiles);
		return false;
	}

	TArray<FString> ThirdPartyName;
	if(InGamePath.Contains(TEXT("/Instance/")))
	{	
		HasDepencyThirdAsset(InGamePath, ThirdPartyName);
		if (ThirdPartyName.Num())
		{
			UploadThirdFolderDescriptToWeb(ThirdPartyName);
			UploadThirdPartyDelegate = FUploadThirdPartyDelegate::CreateRaw(this, &FtpClientManager::UploadThirdPartyFolder);
		}
		//�ύʵ������
		if(!UploadInstanceDescriptToWeb(InGamePath, ThirdPartyName))
			return false;
	}
	else
	{
		//�ύ�����ļ�������

	}
	DeleteUselessFile();	
	TArray<FString> localFiles;  //����·���µ������ļ� �������ɵ������ļ�
	TArray<FString> uploadfiles;  //��Ҫ�ϴ����ļ� �����ʵ���Ļ� �Ͳ����ϴ�������Դ�������ļ��������ļ��еĻ�����Ҫ�ϴ���Դ�������ļ�
	TArray<FString> depenfile;
	if (GetAllFileFromLocalPath(FullPath, localFiles))
	{
		for (const auto& Tempfilename : localFiles)
		{
			if (Tempfilename.Contains(TEXT("Ins_")) && Tempfilename.Contains(GetDefault<UFtpConfig>()->Suffix))
			{
				//ʵ���ļ����µ���Դ�����ļ����ϴ�
				continue;
			}
			uploadfiles.Add(Tempfilename);
		}
		//�ϴ���Դ �Լ������ļ�
		for (const auto& Tempfilename : uploadfiles)
		{
			if (Tempfilename.Contains(GetDefault<UFtpConfig>()->Suffix))
				depenfile.Add(Tempfilename);
			if (!FTP_UploadOneFile(Tempfilename))
				return false;
		}
		//�ҳ�������Դ��PackageName
		TArray<FString> DepenAssetPackName;
		for (const auto& tempdepenfile : depenfile)		//�ϴ����ǹ����ļ��еĻ� ������ȡ������Դ��.dep�ļ����ϴ�ʵ���ļ��еĻ�����ֻ���ȡ  ʵ��.dep
		{
			//�������ļ���Json����ʽ��ת���ɽṹ��
			FString Json;
			FDependenList depenlist;
			FInstanceInfo InstInfo;
			FFileHelper::LoadFileToString(Json, *tempdepenfile);
			if (SimpleFtpDataType::ConvertStringToStruct(Json, depenlist))
			{
				for (const auto& tempdep : depenlist.DepenArr)
				{
					if(!tempdep.DepenAssetPackName.Contains(FolderName))  //��ֹ�ظ��ϴ�
						DepenAssetPackName.AddUnique(tempdep.DepenAssetPackName);
				}
			}
			else if (SimpleFtpDataType::ConvertStringToStruct(Json, InstInfo))
			{
				for (const auto& tempdep : InstInfo.CommonAssetPackageName)
				{
					DepenAssetPackName.AddUnique(tempdep);
				}
			}
		}
		//�ϴ�������Դ,�Լ�������Դ�������ļ�
		if (!UploadDepenceAssetAndDepences(DepenAssetPackName))
			return false;
		UploadThirdPartyDelegate.ExecuteIfBound(ThirdPartyName);
		return true;
	}
	return false;
}

//ֻ�й�����Դ�����ϴ�������Դ
bool FtpClientManager::FTP_UploadFilesByAsset(const TArray<FString>& InPackNames, TArray<FString>& NameNotValidFiles, TArray<FInvalidDepInfo>& DepenNotValidFiles)
{
	bool bAllValid = true; 
	bool bDepHasChanged = false;
	auto GetFolderType = [](FString FolderName)->EFolderType
	{
		if (FolderName.Contains(TEXT("_Texture")))
		{
			return EFolderType::TEXTURE;
		}
		else if (FolderName.Contains(TEXT("_Material")))
		{
			return EFolderType::MATERIAL;
		}
		else if (FolderName.Contains(TEXT("_Animation")))
		{
			return EFolderType::ANIMATION;
		}
		else if (FolderName.Contains(TEXT("_SkeletalMesh")))
		{
			return EFolderType::SKLETALMESH;
		}
		else if (FolderName.Contains(TEXT("_StaticMesh")))
		{
			return EFolderType::STATICMESH;
		}
		else
		{
			return EFolderType::ERROR_FOLDER;
		}
	};
	TArray<FString> numArr1; //���������������жϱ���Ƿ����ظ�  
	TArray<FString> numArr2;

	FString JsonStr;
	FDataInfoList InfoList;
	FFileHelper::LoadFileToString(JsonStr, *DataTypeIni);
	JsonStr.ToUpperInline();
	if (!SimpleFtpDataType::ConvertStringToStruct(JsonStr, InfoList))
		return false;
	//�������
	for (auto pakname : InPackNames)
	{
		FString AssetName, AssetFolderPath;
		pakname.Split(TEXT("/"), &AssetFolderPath, &AssetName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		FString FolderName = FPaths::GetCleanFilename(AssetFolderPath);
		EFolderType type = GetFolderType(FolderName);
		FString UperFileName;
		TArray<FString> partArr;
		bool bCorrect = false;
		switch (type)
		{
		case EFolderType::ANIMATION:
			NAME_VALIDATION_ASSET("ANIM");
				/*UperFileName = AssetName.ToUpper(); 
				numArr1.Add(AssetName); 
				UperFileName.ParseIntoArray(partArr, TEXT("_"), false); 
				if (partArr.Num() != 4)
				{
					bAllValid = false; 
					NameNotValidFiles.Add(AssetName); 
					numArr2.Add(FGuid::NewGuid().ToString()); 
					continue; 
				}
				if (!partArr[0].Equals(TEXT("")))
				{
					bAllValid = false;
					NameNotValidFiles.Add(AssetName);
					numArr2.Add(FGuid::NewGuid().ToString());
					continue;
				}
				for (const auto& TempAeestType : InfoList.DATATYPRARR)
				{
					if (!(partArr[1].Equals(TempAeestType.TYPEABBR)) && !(partArr[1].Equals(TempAeestType.TYPENAME)))
					{
						continue;
					}
					else
					{
						bCorrect = true;
						continue;
					}
				}
				if (!bCorrect)
				{
					bAllValid = false;
					NameNotValidFiles.Add(AssetName);
					numArr2.Add(FGuid::NewGuid().ToString());
					continue;
				}
				numArr2.AddUnique(partArr[2]);
				if (numArr1.Num() != numArr2.Num())
				{
					bAllValid = false;
					NameNotValidFiles.Add(AssetName);
					numArr2.Add(FGuid::NewGuid().ToString());
				}
				break;*/
		case EFolderType::MATERIAL:
			//��ʼ�ж��ļ������Ƿ�Ϸ�
			NAME_VALIDATION_ASSET("MAT");
		case EFolderType::SKLETALMESH:
			//��ʼ�ж��ļ������Ƿ�Ϸ�
			NAME_VALIDATION_ASSET("SKM");
		case EFolderType::STATICMESH:
			//��ʼ�ж��ļ������Ƿ�Ϸ�
			NAME_VALIDATION_ASSET("STM");
		case EFolderType::TEXTURE:
			//��ʼ�ж��ļ������Ƿ�Ϸ�
			NAME_VALIDATION_ASSET("TEX");
		default:
			bAllValid = false;
			break;
		}
	}
	//�������
	for (const auto& pakname : InPackNames)
	{
		FString PakPath = pakname;
		TArray<FString> OneAssetDependence;
		RecursiveFindDependence(pakname, OneAssetDependence);

		//��ȡ����Դ���ڵ��ļ���
		FString AssetName = FPaths::GetCleanFilename(pakname);
		if (PakPath.RemoveFromEnd(TEXT("/") + AssetName))
		{
			FInvalidDepInfo InvalidInfo;
			if (!ValidationDependenceOfOneAsset(PakPath, pakname, OneAssetDependence, InvalidInfo, bDepHasChanged, bAllValid))
			{
				DepenNotValidFiles.Add(InvalidInfo);
				bAllValid = false;
			}
		}
	}
	if (!bAllValid)
	{
		ShowMessageBox(NameNotValidFiles, DepenNotValidFiles);
		return false;
	}
	//��ʼ�ϴ��ļ�,���ҵ����кϷ�����
	TArray<FString> PackNames = InPackNames;
	for (const auto& pakname : InPackNames)
	{
		//��ȡ�ļ�����
		FString FolderName = FPaths::GetCleanFilename(pakname);
		FolderName.RemoveFromEnd(TEXT("/") + FolderName);

		FString FileName = pakname;
		if (FileName.RemoveFromStart(TEXT("/Game/")))
		{
			FileName = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / FileName);
			FileName.Append(GetDefault<UFtpConfig>()->Suffix);		//�õ������ļ�
			FString Json;
			FDependenList depenlist;
			FFileHelper::LoadFileToString(Json, *FileName);
			if (SimpleFtpDataType::ConvertStringToStruct(Json, depenlist))
			{
				for (const auto& tempdep : depenlist.DepenArr)
				{
					if(!tempdep.DepenAssetPackName.Contains(FolderName))
						PackNames.AddUnique(tempdep.DepenAssetPackName);
				}
			}
		}
	}
	//�ϴ�������Դ,�Լ�������Դ�������ļ�
	return UploadDepenceAssetAndDepences(PackNames);
}

bool FtpClientManager::ftp_test(const FString& InFolderPath, const FString& URL)
{
	FString ip = GetMylocalIPADDR();
	TArray<FString> third;
	bool b = UploadInstanceDescriptToWeb(InFolderPath, third);
	return b;
	FWebSendData webdata;
	webdata.file = "file";
	webdata.describe = "describe";
	webdata.filePath = "filePath";
	webdata.name = "name";
	FString Json = webdata.ConvertToString();
	return false;
}

#if WITH_EDITOR
#if PLATFORM_WINDOWS
#pragma optimize("",on)
#endif
#endif

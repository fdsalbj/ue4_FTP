// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "FtpClient/FtpClient.h"
#include "ue4_FTPGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class UE4_FTP_API Aue4_FTPGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:

	virtual void BeginPlay() override;

public:

	UFUNCTION(BlueprintCallable,Category = "FTP")
		bool FTP_CreateControlSocket(const FString& ip, int32 port);

	UFUNCTION(BlueprintCallable, Category = "FTP")
		bool FTP_SendCommand(EFtpCommandType type, FString Param);

	UFUNCTION(BlueprintCallable, Category = "FTP")
		bool FTP_DownloadFile(FString ServerFilePath, FString SavePath);

	UFUNCTION(BlueprintCallable, Category = "FTP")
		bool FTP_DownloadFiles(const FString& serverFolder, const FString& localSavePath);

	UFUNCTION(BlueprintCallable, Category = "FTP")
		bool FTP_List(const FString& serverPath, TArray<FString>& OutFiles);

	UFUNCTION(BlueprintCallable, Category = "FTP")
		bool FTP_UploadFiles(const FString& localPath, TArray<FString>& NotValidFiles, TArray<FString>& DepeNotValidFiles);



	/**
	* ���Ժ���
	*/
	UFUNCTION(BlueprintCallable, Category = "FTP|Test")
		bool TEST_Function(const FString& InFolderPath , FString URL);

	UFUNCTION(BlueprintCallable, Category = "FTP|Test")
		bool MoveFile(const FString& to, const FString& from, bool bReplace);

	UFUNCTION(BlueprintCallable, Category = "FTP|Test")
		int32 CopyFile(const FString& to, const FString& from, bool bReplace);

	UFUNCTION(BlueprintCallable, Category = "FTP|Test")
		void convertbase64(FString localpath, FString savepath);
	
	UFUNCTION(BlueprintCallable, Category="Base64")
		FString EnCode(FString FilePath);

	UFUNCTION(BlueprintCallable, Category = "Base64")
		bool Decode(FString Source, TArray<uint8>& Dest);

	UFUNCTION(BlueprintCallable, Category = "Base64")
		bool SaveArrayToFile(const TArray<uint8>& Data, FString localpath);

	UFUNCTION(BlueprintCallable, Category = "Base64")
		void LoadFileToArray(TArray<uint8>& Data, const FString& localpath);
};

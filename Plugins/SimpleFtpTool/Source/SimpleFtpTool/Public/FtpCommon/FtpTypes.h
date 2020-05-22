// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "FtpTypes.generated.h"


UENUM(BlueprintType)
enum class EFtpCommandType : uint8
{
	//ABOR the connection
	ABOR = 0,
	//user name
	USER,
	//password
	PASS,
	//return the specific file size
	SIZE,
	//change the work dir
	CWD,
	//zhudongmoshi
	PORT,
	//download file
	RETR,
	//upload file
	STOR,
	//delete specific file
	DELE,
	//list content of specific dir
	NLST,
	//list the file or dictionary
	LIST,		
	//create folder
	MKD,		
	//show current work dir
	PWD,		
	//delete specific dir
	RMD,		
	//set data typr(A=ASCII, E=EBCDIC, I=binary)
	TYPE,		
	//quit
	QUIT		

};

UENUM(BlueprintType)
enum class EFileType : uint8
{
	FOLDER = 0,
	FILE
};

UENUM(BlueprintType)
enum class EFolderType : uint8
{
	MATERIAL = 0,
	TEXTURE,
	ANIMATION,
	STATICMESH,
	SKLETALMESH,
	ERROR_FOLDER
};




USTRUCT(BlueprintType)
struct FDataTypeInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TYPENAME;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TYPEABBR;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString TYPEDESCRIPTION;

};

USTRUCT(BlueprintType)
struct FDataInfoList
{
	GENERATED_USTRUCT_BODY()


	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString FILEEDESCRIPTION;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FDataTypeInfo> DATATYPRARR;

};

USTRUCT(BlueprintType)
struct FDependenceInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString DepenAssetPackName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		FString ValidCode;
	
};

USTRUCT(BlueprintType)
struct FDependenList
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString SourceAssetName;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		TArray<FDependenceInfo> DepenArr;
};


//实例配置文件信息
USTRUCT(BlueprintType)
struct FInstanceInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		FString InstValidCode;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		TArray<FString> CommonAssetPackageName;

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
		TArray<FString> ThirdPartyAssetPackageName;
};


namespace SimpleDataType
{
	void ConvertStructToString(const FDataInfoList& TypeArr, FString& Json);
	bool ConvertStringToStruct(const FString& Json, FDataInfoList& TypeArr);

	void ConvertStructToString(const FDependenList& DepenArr, FString& Json);
	bool ConvertStringToStruct(const FString& Json, FDependenList& DepenArr);

	void ConvertStructToString(const FInstanceInfo& InstInfo, FString& Json);
	bool ConvertStringToStruct(const FString& Json, FInstanceInfo& InstInfo);

}
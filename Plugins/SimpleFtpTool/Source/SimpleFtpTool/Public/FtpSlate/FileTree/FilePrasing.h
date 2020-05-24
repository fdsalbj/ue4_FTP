// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

namespace SimpleFtpFile
{
	enum class EFileType :uint8
	{
		FILE,
		FOLDER,
	};

	struct FFileList
	{
		//���Filename���ļ���ô����������һ��Packagename = /Game/SS/A
		//���Filename���ļ�����ô����������һ���ļ������� ���� Game
		FString Filename;
		TArray<FFileList> Children;

		FORCEINLINE friend bool operator==(const FFileList& A, const FFileList& B)
		{
			return A.Filename == B.Filename;
		}

		EFileType GetFileType()const;
	};

	void FilesParsing(const TArray<FString>& Filenames, FFileList& List);
}
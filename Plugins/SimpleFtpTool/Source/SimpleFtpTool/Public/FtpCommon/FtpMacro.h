// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#define PASV_MODE 227   //�ɹ�����Ϊ����ģʽ
#define LIST_SUCCEED 150  //�����ݿ���ͨ���ɹ�
#define ERROR_DIRECTORY 550  //LISTʧ�� û��·��
#define SUCCEED_TRANSFER 226  //�ϴ������سɹ�


//#define NAME_VALIDATION(Prefix) \    
//for(const auto& TempName : AllFileNames) \
//{  \
//	numArr1.Add(TempName); \
//	TArray<FString> partArr; \
//	TempName.ParseIntoArray(partArr, TEXT("_"), false); \
//	if (!partArr[0].Equals(TEXT(##Prefix))) \  
//	{  \
//		bAllValid = false;  \
//		NoValidFiles.Add(TempName); \
//		continue; \
//	} \
//	for (const auto& TempAeestType : AssetTypes) \
//	{ \
//		FString l, r; \
//		TempAeestType.Split(TEXT(":"), &l, &r); \
//		if ((partArr[1] == l) || (partArr[1] == r)) \
//		{ \
//			continue; \
//		} \
//		bAllValid = false; \
//		NoValidFiles.Add(TempName); \
//	}  \
//	numArr2.AddUnique(partArr[2]); \ 
//	if (numArr1.Num() != numArr2.Num()) \
//	{  \
//		bAllValid = false; \
//		NoValidFiles.Add(TempName); \
//		numArr2.Add(FGuid::NewGuid().ToString()); \
//	}  \
//}
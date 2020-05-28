// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HttpRequest/RequestInterface.h"
#include "HttpCommon/SimpleHttpType.h"

namespace SimpleHttpRequest
{
	struct SIMPLEHTTPTOOL_API FPutObjectRequest : IHTTPClientRequest
	{
		//URL  content String������洢����   ��FString��ʽ
		FPutObjectRequest(const FString& URL, const FString& ContentString);
		//���ֽ���ʽ�ϴ�
		FPutObjectRequest(const FString& URL, const TArray<uint8>& ContentString);
	};

	struct SIMPLEHTTPTOOL_API FGetObjectRequest : IHTTPClientRequest
	{
		FGetObjectRequest(const FString& URL);
	};

	struct SIMPLEHTTPTOOL_API FPostObjectRequest : IHTTPClientRequest
	{
		FPostObjectRequest(const FString& URL, const FString& SendMesg);
		FPostObjectRequest(const FString& URL, const TArray<uint8>& SendMesg);
	};

	struct SIMPLEHTTPTOOL_API FDeleteObjectRequest : IHTTPClientRequest
	{
		FDeleteObjectRequest(const FString& URL);
	};

}
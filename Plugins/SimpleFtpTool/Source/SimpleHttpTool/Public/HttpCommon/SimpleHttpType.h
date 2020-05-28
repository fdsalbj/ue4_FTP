// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Json.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


DECLARE_DELEGATE_ThreeParams(FSimpleCompleteDelegate, FHttpRequestPtr, FHttpResponsePtr, bool);
DECLARE_DELEGATE_ThreeParams(FSimpleSingleRequestProgressDelegate, FHttpRequestPtr, int32, int32);
DECLARE_DELEGATE_ThreeParams(FSimpleSingleRequestHeaderReceivedDelegate, FHttpRequestPtr, const FString&, const FString&);

struct SIMPLEHTTPTOOL_API FHttpDelegate
{
	FHttpDelegate()
		:SimpleCompleteDelegate(nullptr),
		 SimpleSingleRequestProgressDelegate(nullptr),
		 SimpleSingleRequestHeaderReceivedDelegate(nullptr)
	{}
	
	FSimpleCompleteDelegate SimpleCompleteDelegate;
	FSimpleSingleRequestProgressDelegate SimpleSingleRequestProgressDelegate;
	FSimpleSingleRequestHeaderReceivedDelegate SimpleSingleRequestHeaderReceivedDelegate;
};

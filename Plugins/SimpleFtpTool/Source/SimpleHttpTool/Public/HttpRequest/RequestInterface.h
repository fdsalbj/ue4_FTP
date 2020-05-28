// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

namespace SimpleHttpRequest
{
	struct SIMPLEHTTPTOOL_API IHTTPClientRequest
	{
		friend struct FHttpClient;
		IHTTPClientRequest();

		IHTTPClientRequest& operator<<(const FHttpRequestCompleteDelegate& SimpleDelegateInstance)
		{
			HttpRequest->OnProcessRequestComplete() = SimpleDelegateInstance;
			return *this;
		}

		IHTTPClientRequest& operator<<(const FHttpRequestProgressDelegate& SimpleDelegateInstance)
		{
			HttpRequest->OnRequestProgress() = SimpleDelegateInstance;
			return *this;
		}

		IHTTPClientRequest& operator<<(const FHttpRequestHeaderReceivedDelegate& SimpleDelegateInstance)
		{
			HttpRequest->OnHeaderReceived() = SimpleDelegateInstance;
			return *this;
		}

	protected:
		//ִ������
		bool ProcessRequest();
		//ȡ������
		void CancelRequest();
	protected:
		TSharedPtr<class IHttpRequest> HttpRequest;

	};
}
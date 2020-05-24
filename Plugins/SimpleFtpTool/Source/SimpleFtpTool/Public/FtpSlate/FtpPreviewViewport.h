#pragma once
#include "SEditorViewport.h"

class SIMPLEFTPTOOL_API SimpleFtpPreviewViewport : public SEditorViewport
{
public:
	SLATE_BEGIN_ARGS(SimpleFtpPreviewViewport) {}

	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient();

private:
	TSharedPtr<class FAdvancedPreviewScene> PreviewScene;
	class USkyLightComponent* Skylight;
	class UAtmosphericFogComponent* AtmosphericFog;
	TSharedPtr<class FPakViewEditorViewportClient> EditorViewportClient;
};
#pragma once

#include "CoreMinimal.h"
#include "InteractionPromptTypes.generated.h"

USTRUCT(BlueprintType)
struct INTERACTIONSYSTEMPLUGIN_API FInteractionPromptData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    FText PrimaryText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    FText SecondaryText;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    FName InputActionName = NAME_None;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    bool bIsWarning = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    float AutoHideTime = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Interaction|Prompt")
    int32 Priority = 10;

    UPROPERTY(BlueprintReadOnly, Category = "Interaction|Prompt")
    AActor* SourceActor = nullptr;
};
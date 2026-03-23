#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "InteractionPromptTypes.h"
#include "InteractionNotificationSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractionPromptUpdated, FInteractionPromptData, PromptData);

UCLASS(Blueprintable, BlueprintType)
class INTERACTIONSYSTEMPLUGIN_API UInteractionNotificationSubsystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnInteractionPromptUpdated OnInteractionPromptUpdated;

    UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
    void UpdateInteractionPrompt(const FInteractionPromptData& Data);

    UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
    void ClearInteractionPrompt();

    UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
    void ShowTemporaryNotification(const FText& Message, float Duration = 3.0f, bool bIsWarning = false);
};
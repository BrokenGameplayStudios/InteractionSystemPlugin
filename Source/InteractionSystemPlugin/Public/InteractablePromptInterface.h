#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractionPromptTypes.h"
#include "InteractablePromptInterface.generated.h"

UINTERFACE(Blueprintable, BlueprintType)
class INTERACTIONSYSTEMPLUGIN_API UInteractablePromptInterface : public UInterface
{
    GENERATED_BODY()
};

class INTERACTIONSYSTEMPLUGIN_API IInteractablePromptInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Prompt")
    FInteractionPromptData GetInteractionPromptData();

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interaction|Prompt")
    bool ShouldShowPrompt() const;

    virtual FInteractionPromptData GetInteractionPromptData_Implementation() { return FInteractionPromptData(); }
    virtual bool ShouldShowPrompt_Implementation() const { return true; }
};
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPromptTypes.h"
#include "InteractionPromptWidget.generated.h"

UCLASS(Abstract, BlueprintType)
class INTERACTIONSYSTEMPLUGIN_API UInteractionPromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction|UI")
    void BP_OnPromptUpdated(const FInteractionPromptData& Data);

    UFUNCTION(BlueprintCallable, Category = "Interaction|UI")
    void UpdateKeyIcon(FName ActionName);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UFUNCTION()
    void OnSubsystemPromptUpdated(FInteractionPromptData Data);

    UPROPERTY(BlueprintReadOnly, Category = "Interaction|UI")
    FInteractionPromptData CurrentData;
};
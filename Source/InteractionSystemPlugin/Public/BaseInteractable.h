#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableComponent.h"
#include "BaseInteractable.generated.h"

UCLASS()
class INTERACTIONSYSTEMPLUGIN_API ABaseInteractable : public AActor
{
    GENERATED_BODY()

public:
    ABaseInteractable();

    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
    UInteractableComponent* InteractableComp;

    UPROPERTY(ReplicatedUsing = OnRep_IsActive, BlueprintReadOnly, Category = "Interaction State")
    bool bIsActive = false;

    UFUNCTION(BlueprintPure, Category = "Interaction State")
    bool GetIsActive() const;

    UFUNCTION(BlueprintCallable, Category = "Interaction State")
    void SetIsActive(bool NewState);

    /** Runs BP_UpdateVisuals on BOTH server and clients */
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void ForceUpdateVisuals();

    UFUNCTION()
    void OnRep_IsActive();

    UFUNCTION()
    void HandleInteractComplete(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void ToggleState();
    virtual void ToggleState_Implementation();

    UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
    void BP_UpdateVisuals();
};
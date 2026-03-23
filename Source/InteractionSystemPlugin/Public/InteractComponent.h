// UInteractComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

class UInteractableComponent;
class APawn;
class ACharacter;
class APlayerController;

// Event fired when the focused interactable changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFocusedInteractableChanged, AActor*, NewFocused, AActor*, OldFocused);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEMPLUGIN_API UInteractComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Initialize cached references (call from BP on PossessedBy)
    UFUNCTION(BlueprintCallable, Category = "Interaction|Setup")
    void Initialize(APlayerController* InPlayerController);

    // Maximum distance for interaction trace
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float MaxInteractionDistance = 300.f;

    // Socket to start trace from (e.g. "head" or "eyes")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName TraceSocketName = NAME_None;

    // Collision channel used for LOS trace
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    TEnumAsByte<ECollisionChannel> InteractionTraceChannel = ECC_Visibility;

    // Global flag to temporarily disable ALL interactions (set by any actor)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bInteractionEnabled = true;

    // Fired when the best interactable in view changes
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnFocusedInteractableChanged OnFocusedInteractableChanged;

    // Called when player presses interact key
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractPressed();

    // Called when player releases interact key
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractReleased();

    // Returns true if there is clear LOS from socket/camera to target
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool HasLineOfSight(AActor* Target) const;

    // All interactables currently visible to the player
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TArray<AActor*> VisibleInteractables;

    // Server RPC to start interaction on the target
    UFUNCTION(Server, Reliable)
    void Server_StartInteraction(UInteractableComponent* TargetComponent);

    // Server RPC to cancel ongoing interaction
    UFUNCTION(Server, Reliable)
    void Server_CancelInteraction(UInteractableComponent* TargetComponent);

    // Enable or disable all interactions (call from any actor)
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void SetInteractionEnabled(bool bEnabled);

protected:
    // Current interactable we are holding/using
    TWeakObjectPtr<UInteractableComponent> CurrentInteractableComponent;

    // Timer used for continuous LOS checks during hold
    FTimerHandle InteractionCheckTimer;

    // Periodic check to see if LOS is still valid
    void PerformLOSCheck();

    // Radius to find nearby interactables
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 350.0f;

    // How often we run detection (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionInterval = 0.1f;

    // Max angle from center for gaze priority
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float MaxAngleForCenter = 45.0f;

    float LastDetectionTime = 0.0f;
    TArray<AActor*> NearbyInteractables;
    AActor* FocusedInteractable;

    void DetectNearbyInteractables();
    void UpdateVisibleInteractables();
    AActor* GetBestInteractable() const;

    UPROPERTY(Transient)
    APawn* CachedPawnOwner;

    UPROPERTY(Transient)
    ACharacter* CachedCharacterOwner;

    UPROPERTY(Transient)
    APlayerController* CachedPlayerController;

    bool bInitialized = false;

private:
    // Returns the start location for the LOS trace (socket or camera)
    FVector GetTraceStartLocation() const;
};
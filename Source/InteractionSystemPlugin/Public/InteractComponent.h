// InteractComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

class UInteractableComponent;
class APawn;
class ACharacter;
class APlayerController;
class USkeletalMeshComponent;

// Delegate
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFocusedInteractableChanged, AActor*, NewFocused, AActor*, OldFocused);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEMPLUGIN_API UInteractComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Explicit initialization function to pass references (call from BP, e.g., on PossessedBy)
    UFUNCTION(BlueprintCallable, Category = "Interaction|Setup")
    void Initialize(APlayerController* InPlayerController);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float MaxInteractionDistance = 200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    FName TraceSocketName = NAME_None;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnFocusedInteractableChanged OnFocusedInteractableChanged;

    // Input handlers
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractPressed();

    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void InteractReleased();

    // Function to check line of sight to a specific interactable
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    bool HasLineOfSight(AActor* Target) const;

    // Exposed for BP access (e.g., to send messages to visible interactables)
    UPROPERTY(BlueprintReadOnly, Category = "Interaction")
    TArray<AActor*> VisibleInteractables;

    // Server RPC for starting interaction (called from client to route to target)
    UFUNCTION(Server, Reliable)
    void Server_StartInteraction(UInteractableComponent* TargetComponent);

    // Server RPC for canceling interaction
    UFUNCTION(Server, Reliable)
    void Server_CancelInteraction(UInteractableComponent* TargetComponent);

protected:
    // Current interactable being targeted
    TWeakObjectPtr<UInteractableComponent> CurrentInteractableComponent;

    // Timer for ongoing LOS check during interaction
    FTimerHandle InteractionCheckTimer;

    // Periodic LOS check method
    void PerformLOSCheck();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionRadius = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float DetectionInterval = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float MaxAngleForCenter = 30.0f; // Degrees from center for priority cone

    float LastDetectionTime = 0.0f;
    TArray<AActor*> NearbyInteractables;
    AActor* FocusedInteractable;

    // Scans for interactables within sphere.
    void DetectNearbyInteractables();

    // Filters nearby to those with LOS.
    void UpdateVisibleInteractables();

    AActor* GetBestInteractable() const; // Considers center angle and distance on visible ones

    // Cached references for efficiency
    UPROPERTY(Transient)
    APawn* CachedPawnOwner;

    UPROPERTY(Transient)
    ACharacter* CachedCharacterOwner;

    UPROPERTY(Transient)
    APlayerController* CachedPlayerController;

    // Flag to track if Initialized
    bool bInitialized = false;
};
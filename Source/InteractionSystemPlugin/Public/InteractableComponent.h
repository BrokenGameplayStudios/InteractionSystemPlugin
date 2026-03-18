// InteractableComponent.h
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

// Delegates
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractStart, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractComplete, AActor*, Interactor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteractProgress, AActor*, Interactor, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractCancel, AActor*, Interactor);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class INTERACTIONSYSTEMPLUGIN_API UInteractableComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInteractableComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // Config Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Interaction")
    bool bIsInteractable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRequiresHold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction", meta = (EditCondition = "bRequiresHold"))
    float HoldTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bRepeatable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    bool bAutoInteractOnOverlap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
    float InteractionRadius;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractStart OnInteractStart;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractComplete OnInteractComplete;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractProgress OnInteractProgress;

    UPROPERTY(BlueprintAssignable, Category = "Interaction")
    FOnInteractCancel OnInteractCancel;

    // Start the interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void StartInteraction(AActor* Interactor);

    // Complete the interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CompleteInteraction(AActor* Interactor);

    // Cancel the interaction
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void CancelInteraction(AActor* Interactor);

    // Server RPC for requesting interaction
    UFUNCTION(Server, Reliable)
    void Server_RequestInteract(AActor* Interactor);

    // Server RPC for canceling hold
    UFUNCTION(Server, Reliable)
    void Server_CancelInteract(AActor* Interactor);

    // Multicast RPCs for replicating events to clients
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnInteractStart(AActor* Interactor);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnInteractComplete(AActor* Interactor);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnInteractProgress(AActor* Interactor, float Progress);

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnInteractCancel(AActor* Interactor);

protected:
    FTimerHandle HoldTimer;
    UPROPERTY(Replicated)
    AActor* CurrentInteractor;
    float HoldStartTime;

    // Replicated progress for client UI.
    UPROPERTY(ReplicatedUsing = OnRep_HoldProgress)
    float CurrentHoldProgress;

    UFUNCTION()
    void OnRep_HoldProgress();

    // Internal hold complete handler
    void OnHoldComplete();
};
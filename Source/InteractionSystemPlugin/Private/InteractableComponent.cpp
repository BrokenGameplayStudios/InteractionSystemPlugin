// InteractableComponent.cpp
#include "InteractableComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "GameFramework/Actor.h"

UInteractableComponent::UInteractableComponent()
{
    bIsInteractable = true;
    bRequiresHold = false;
    HoldTime = 2.0f;
    bRepeatable = true;
    bAutoInteractOnOverlap = false;
    InteractionRadius = 200.0f;
    HoldStartTime = 0.0f;
    CurrentHoldProgress = 0.0f;
    PrimaryComponentTick.bCanEverTick = true; // Enable tick for progress
}

void UInteractableComponent::BeginPlay()
{
    Super::BeginPlay();

}

void UInteractableComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentInteractor && bRequiresHold && GetOwner()->HasAuthority())
    {
        float Elapsed = GetWorld()->GetTimeSeconds() - HoldStartTime;
        float NewProgress = FMath::Clamp(Elapsed / HoldTime, 0.0f, 1.0f);
        if (NewProgress != CurrentHoldProgress)
        {
            CurrentHoldProgress = NewProgress;
        }
    }
}

void UInteractableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UInteractableComponent, bIsInteractable);
    DOREPLIFETIME(UInteractableComponent, CurrentInteractor);
    DOREPLIFETIME(UInteractableComponent, CurrentHoldProgress);
}

void UInteractableComponent::OnRep_HoldProgress()
{
    if (CurrentInteractor)
    {
        OnInteractProgress.Broadcast(CurrentInteractor, CurrentHoldProgress);
    }
}

void UInteractableComponent::StartInteraction(AActor* Interactor)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_RequestInteract(Interactor);
        return;
    }

    if (!bIsInteractable || !Interactor) return;

    CurrentInteractor = Interactor;
    HoldStartTime = GetWorld()->GetTimeSeconds();
    CurrentHoldProgress = 0.0f;
    Multicast_OnInteractStart(Interactor);
    Multicast_OnInteractProgress(Interactor, CurrentHoldProgress);

    if (bRequiresHold)
    {
        GetWorld()->GetTimerManager().SetTimer(HoldTimer, this, &UInteractableComponent::OnHoldComplete, HoldTime, false);
    }
    else
    {
        CompleteInteraction(Interactor);
    }
}

void UInteractableComponent::CompleteInteraction(AActor* Interactor)
{
    if (!GetOwner()->HasAuthority())
    {
        return;
    }

    if (CurrentInteractor == Interactor)
    {
        Multicast_OnInteractComplete(Interactor);
        if (!bRepeatable)
        {
            bIsInteractable = false;
        }
        CurrentInteractor = nullptr;
        CurrentHoldProgress = 0.0f;
        GetWorld()->GetTimerManager().ClearTimer(HoldTimer);
    }
}

void UInteractableComponent::CancelInteraction(AActor* Interactor)
{
    if (!GetOwner()->HasAuthority())
    {
        Server_CancelInteract(Interactor);
        return;
    }

    if (CurrentInteractor == Interactor)
    {
        GetWorld()->GetTimerManager().ClearTimer(HoldTimer);
        Multicast_OnInteractCancel(Interactor);
        CurrentInteractor = nullptr;
        CurrentHoldProgress = 0.0f;
    }
}

void UInteractableComponent::Server_RequestInteract_Implementation(AActor* Interactor)
{
    StartInteraction(Interactor);
}

void UInteractableComponent::Server_CancelInteract_Implementation(AActor* Interactor)
{
    CancelInteraction(Interactor);
}

void UInteractableComponent::Multicast_OnInteractStart_Implementation(AActor* Interactor)
{
    OnInteractStart.Broadcast(Interactor);
}

void UInteractableComponent::Multicast_OnInteractComplete_Implementation(AActor* Interactor)
{
    OnInteractComplete.Broadcast(Interactor);
}

void UInteractableComponent::Multicast_OnInteractProgress_Implementation(AActor* Interactor, float Progress)
{
    OnInteractProgress.Broadcast(Interactor, Progress);
}

void UInteractableComponent::Multicast_OnInteractCancel_Implementation(AActor* Interactor)
{
    OnInteractCancel.Broadcast(Interactor);
}

void UInteractableComponent::OnHoldComplete()
{
    if (CurrentInteractor)
    {
        CompleteInteraction(CurrentInteractor);
    }
}
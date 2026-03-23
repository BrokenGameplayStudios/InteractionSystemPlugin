#include "BaseInteractable.h"
#include "Net/UnrealNetwork.h"

ABaseInteractable::ABaseInteractable()
{
    bReplicates = true;
    InteractableComp = CreateDefaultSubobject<UInteractableComponent>(TEXT("InteractableComp"));
}

void ABaseInteractable::BeginPlay()
{
    Super::BeginPlay();
    if (InteractableComp)
    {
        InteractableComp->OnInteractComplete.AddDynamic(this, &ABaseInteractable::HandleInteractComplete);
    }
}

void ABaseInteractable::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseInteractable, bIsActive);
}

bool ABaseInteractable::GetIsActive() const { return bIsActive; }

void ABaseInteractable::SetIsActive(bool NewState)
{
    if (!HasAuthority()) return;
    if (bIsActive != NewState)
    {
        bIsActive = NewState;
        ForceUpdateVisuals();
    }
}

void ABaseInteractable::ForceUpdateVisuals()
{
    BP_UpdateVisuals();
}

void ABaseInteractable::HandleInteractComplete(AActor* Interactor)
{
    if (!HasAuthority()) return;
    ToggleState();
}

void ABaseInteractable::ToggleState_Implementation()
{
    SetIsActive(!GetIsActive());   // Default toggle
}

void ABaseInteractable::OnRep_IsActive()
{
    ForceUpdateVisuals();          // Clients
}
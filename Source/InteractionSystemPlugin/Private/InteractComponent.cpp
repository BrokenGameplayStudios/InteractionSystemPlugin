// InteractComponent.cpp
#include "InteractComponent.h"
#include "InteractableComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"  // For Debugging
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"
#include "Net/UnrealNetwork.h"  // For net mode logging
#include "GameFramework/Pawn.h"  // For APawn

UInteractComponent::UInteractComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    CachedPawnOwner = nullptr;
    CachedCharacterOwner = nullptr;
    CachedPlayerController = nullptr;
}

void UInteractComponent::BeginPlay()
{
    Super::BeginPlay();

    // Fallback cache
    AActor* OwnerActor = GetOwner();
    if (OwnerActor)
    {
        CachedPawnOwner = Cast<APawn>(OwnerActor);
        CachedCharacterOwner = Cast<ACharacter>(OwnerActor);
        if (CachedPawnOwner)
        {
            CachedPlayerController = Cast<APlayerController>(CachedPawnOwner->GetController());
        }
    }

    if (!CachedPawnOwner || !CachedPlayerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("UInteractComponent fallback cache incomplete for owner %s. Call Initialize from BP."), *GetOwner()->GetName());
    }
    else
    {
        bInitialized = true;
    }
}

void UInteractComponent::Initialize(APlayerController* InPlayerController)
{
    CachedPlayerController = InPlayerController;
    if (CachedPlayerController)
    {
        bInitialized = true;
        UE_LOG(LogTemp, Log, TEXT("UInteractComponent initialized with Controller for owner %s"), *GetOwner()->GetName());

        // Recache pawn if needed
        AActor* OwnerActor = GetOwner();
        CachedPawnOwner = Cast<APawn>(OwnerActor);
        CachedCharacterOwner = Cast<ACharacter>(OwnerActor);
    }
}

void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled())
    {
        return;
    }

    if (GetWorld()->GetTimeSeconds() - LastDetectionTime >= DetectionInterval)
    {
        DetectNearbyInteractables();
        UpdateVisibleInteractables();
        AActor* NewFocused = GetBestInteractable();
        if (NewFocused != FocusedInteractable)
        {
            OnFocusedInteractableChanged.Broadcast(NewFocused, FocusedInteractable);
            FocusedInteractable = NewFocused;
        }
        LastDetectionTime = GetWorld()->GetTimeSeconds();
    }
}

void UInteractComponent::InteractPressed()
{
    if (!bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return;

    if (FocusedInteractable)
    {
        UInteractableComponent* InteractableComp = FocusedInteractable->FindComponentByClass<UInteractableComponent>();
        if (InteractableComp && HasLineOfSight(FocusedInteractable))
        {
            CurrentInteractableComponent = InteractableComp;
            Server_StartInteraction(InteractableComp);
            GetWorld()->GetTimerManager().SetTimer(InteractionCheckTimer, this, &UInteractComponent::PerformLOSCheck, 0.2f, true);
        }
    }
}

void UInteractComponent::InteractReleased()
{
    if (!bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return;

    if (CurrentInteractableComponent.IsValid())
    {
        Server_CancelInteraction(CurrentInteractableComponent.Get());
        GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
        CurrentInteractableComponent.Reset();
    }
}

void UInteractComponent::Server_StartInteraction_Implementation(UInteractableComponent* TargetComponent)
{
    if (TargetComponent)
    {
        TargetComponent->StartInteraction(GetOwner());
    }
}

void UInteractComponent::Server_CancelInteraction_Implementation(UInteractableComponent* TargetComponent)
{
    if (TargetComponent)
    {
        TargetComponent->CancelInteraction(GetOwner());
    }
}

bool UInteractComponent::HasLineOfSight(AActor* Target) const
{
    if (!bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return false;

    if (!Target) return false;

    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return false;

    FVector TraceStart = OwnerActor->GetActorLocation();
    FRotator TraceRotation = OwnerActor->GetActorRotation();

    USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(OwnerActor->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
    if (Mesh && TraceSocketName != NAME_None)
    {
        TraceStart = Mesh->GetSocketLocation(TraceSocketName);
        TraceRotation = Mesh->GetSocketRotation(TraceSocketName);
    }

    if (!CachedPlayerController || !CachedPlayerController->PlayerCameraManager) return false;
    FVector CameraForward = CachedPlayerController->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();

    FVector TraceEnd = TraceStart + (CameraForward * MaxInteractionDistance);

    FHitResult HitResult;
    ECollisionChannel TraceChannel = ECC_Visibility;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerActor);

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, TraceChannel, Params);

    return bHit && HitResult.GetActor() == Target;
}

void UInteractComponent::PerformLOSCheck()
{
    if (!CurrentInteractableComponent.IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
        return;
    }

    AActor* TargetActor = CurrentInteractableComponent->GetOwner();
    if (!HasLineOfSight(TargetActor))
    {
        Server_CancelInteraction(CurrentInteractableComponent.Get());
        GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
        CurrentInteractableComponent.Reset();
    }
}

void UInteractComponent::DetectNearbyInteractables()
{
    NearbyInteractables.Empty();

    FVector Location = GetOwner()->GetActorLocation();
    TArray<FHitResult> HitResults;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    UKismetSystemLibrary::SphereTraceMultiForObjects(GetWorld(), Location, Location, DetectionRadius,
        ObjectTypes, false, { GetOwner() }, EDrawDebugTrace::None,
        HitResults, true);

    for (const FHitResult& Hit : HitResults)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor && HitActor != GetOwner())
        {
            UInteractableComponent* Comp = HitActor->FindComponentByClass<UInteractableComponent>();
            if (Comp && Comp->bIsInteractable)
            {
                NearbyInteractables.AddUnique(HitActor);
            }
        }
    }
}

void UInteractComponent::UpdateVisibleInteractables()
{
    VisibleInteractables.Empty();
    for (AActor* Candidate : NearbyInteractables)
    {
        if (HasLineOfSight(Candidate))
        {
            VisibleInteractables.Add(Candidate);
        }
    }
}

AActor* UInteractComponent::GetBestInteractable() const
{
    AActor* Best = nullptr;
    float BestScore = -1.0f;

    FVector ViewLocation;
    FRotator ViewRotation;
    if (CachedCharacterOwner)
    {
        CachedCharacterOwner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
    }
    else if (CachedPawnOwner)
    {
        CachedPawnOwner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
    }
    else
    {
        ViewLocation = GetOwner()->GetActorLocation();
        ViewRotation = GetOwner()->GetActorRotation();
    }
    FVector Forward = ViewRotation.Vector();

    for (AActor* Candidate : VisibleInteractables)
    {
        FVector DirToCandidate = (Candidate->GetActorLocation() - ViewLocation).GetSafeNormal();
        float Dot = FVector::DotProduct(Forward, DirToCandidate);
        float Angle = FMath::Acos(Dot) * (180.0f / PI);
        if (Angle > MaxAngleForCenter) continue;

        float Distance = FVector::Dist(ViewLocation, Candidate->GetActorLocation());
        float Score = Dot / (Distance + 1.0f);
        if (Score > BestScore)
        {
            BestScore = Score;
            Best = Candidate;
        }
    }
    return Best;
}
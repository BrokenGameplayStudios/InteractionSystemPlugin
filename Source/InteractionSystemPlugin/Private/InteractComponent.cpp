// UInteractComponent.cpp

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
#include "CollisionQueryParams.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

UInteractComponent::UInteractComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UInteractComponent::BeginPlay()
{
    Super::BeginPlay();

    AActor* OwnerActor = GetOwner();
    if (OwnerActor)
    {
        CachedPawnOwner = Cast<APawn>(OwnerActor);
        CachedCharacterOwner = Cast<ACharacter>(OwnerActor);
        if (CachedPawnOwner)
            CachedPlayerController = Cast<APlayerController>(CachedPawnOwner->GetController());
    }

    if (CachedPawnOwner && CachedPlayerController)
        bInitialized = true;
}

void UInteractComponent::Initialize(APlayerController* InPlayerController)
{
    CachedPlayerController = InPlayerController;
    if (CachedPlayerController) bInitialized = true;
}

FVector UInteractComponent::GetTraceStartLocation() const
{
    if (TraceSocketName != NAME_None && CachedPawnOwner)
    {
        USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(CachedPawnOwner->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
        if (Mesh)
            return Mesh->GetSocketLocation(TraceSocketName);
    }

    if (CachedPlayerController && CachedPlayerController->PlayerCameraManager)
        return CachedPlayerController->PlayerCameraManager->GetCameraLocation();

    return CachedPawnOwner ? CachedPawnOwner->GetActorLocation() : FVector::ZeroVector;
}

bool UInteractComponent::HasLineOfSight(AActor* Target) const
{
    if (!Target || !bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return false;

    FVector TraceStart = GetTraceStartLocation();
    if (!CachedPlayerController || !CachedPlayerController->PlayerCameraManager) return false;

    FVector Forward = CachedPlayerController->PlayerCameraManager->GetCameraRotation().Vector().GetSafeNormal();

    TraceStart += Forward * 25.0f;

    FVector TraceEnd = TraceStart + (Forward * MaxInteractionDistance);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());
    Params.bTraceComplex = true;

    if (USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(CachedPawnOwner->GetComponentByClass(USkeletalMeshComponent::StaticClass())))
        Params.AddIgnoredComponent(Mesh);

    return GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, InteractionTraceChannel, Params) &&
        HitResult.GetActor() == Target;
}

void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return;

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
    if (!bInteractionEnabled || !bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return;

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
    if (!bInteractionEnabled || !bInitialized || !CachedPawnOwner || !CachedPawnOwner->IsLocallyControlled()) return;

    if (CurrentInteractableComponent.IsValid())
    {
        Server_CancelInteraction(CurrentInteractableComponent.Get());
        GetWorld()->GetTimerManager().ClearTimer(InteractionCheckTimer);
        CurrentInteractableComponent.Reset();
    }
}

void UInteractComponent::SetInteractionEnabled(bool bEnabled)
{
    bInteractionEnabled = bEnabled;
}

void UInteractComponent::Server_StartInteraction_Implementation(UInteractableComponent* TargetComponent)
{
    if (TargetComponent) TargetComponent->StartInteraction(GetOwner());
}

void UInteractComponent::Server_CancelInteraction_Implementation(UInteractableComponent* TargetComponent)
{
    if (TargetComponent) TargetComponent->CancelInteraction(GetOwner());
}

void UInteractComponent::PerformLOSCheck()
{
    if (!CurrentInteractableComponent.IsValid()) return;

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
        ObjectTypes, false, { GetOwner() }, EDrawDebugTrace::None, HitResults, true);

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
            VisibleInteractables.Add(Candidate);
    }
}

AActor* UInteractComponent::GetBestInteractable() const
{
    AActor* Best = nullptr;
    float BestScore = -1.0f;

    FVector ViewLocation;
    FRotator ViewRotation;
    if (CachedPlayerController && CachedPlayerController->PlayerCameraManager)
        CachedPlayerController->PlayerCameraManager->GetCameraViewPoint(ViewLocation, ViewRotation);
    else if (CachedCharacterOwner)
        CachedCharacterOwner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
    else if (CachedPawnOwner)
        CachedPawnOwner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
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
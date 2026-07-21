#include "NPC_DinosaurHerdBehavior.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurHerdBehavior::UNPC_DinosaurHerdBehavior()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
}

void UNPC_DinosaurHerdBehavior::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeHerd();
    SetHerdState(ENPC_HerdState::Grazing);
}

void UNPC_DinosaurHerdBehavior::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    StateTimer += DeltaTime;
    CalculateHerdCenter();

    switch (CurrentState)
    {
        case ENPC_HerdState::Grazing:
            UpdateGrazingBehavior(DeltaTime);
            break;
        case ENPC_HerdState::Moving:
            UpdateMovingBehavior(DeltaTime);
            break;
        case ENPC_HerdState::Alert:
            UpdateAlertBehavior(DeltaTime);
            break;
        case ENPC_HerdState::Fleeing:
            UpdateFleeingBehavior(DeltaTime);
            break;
        case ENPC_HerdState::Drinking:
            UpdateDrinkingBehavior(DeltaTime);
            break;
    }

    // Check for danger every tick
    if (CurrentState != ENPC_HerdState::Fleeing && DetectDanger())
    {
        SetHerdState(ENPC_HerdState::Alert);
    }

    UpdateHerdFormation();
}

void UNPC_DinosaurHerdBehavior::InitializeHerd()
{
    HerdMembers.Empty();
    FindNearbyDinosaurs();
    
    if (HerdMembers.Num() > 0)
    {
        // Set first member as leader
        HerdMembers[0].bIsLeader = true;
        HerdCenterLocation = GetOwner()->GetActorLocation();
    }
}

void UNPC_DinosaurHerdBehavior::AddHerdMember(APawn* NewMember)
{
    if (!NewMember || HerdMembers.Num() >= MaxHerdSize)
    {
        return;
    }

    FNPC_HerdMember NewHerdMember;
    NewHerdMember.DinosaurPawn = NewMember;
    NewHerdMember.RelativePosition = NewMember->GetActorLocation() - HerdCenterLocation;
    NewHerdMember.DistanceFromLeader = FVector::Dist(NewMember->GetActorLocation(), HerdCenterLocation);
    NewHerdMember.bIsLeader = (HerdMembers.Num() == 0);

    HerdMembers.Add(NewHerdMember);
}

void UNPC_DinosaurHerdBehavior::RemoveHerdMember(APawn* Member)
{
    HerdMembers.RemoveAll([Member](const FNPC_HerdMember& HerdMember)
    {
        return HerdMember.DinosaurPawn == Member;
    });
}

void UNPC_DinosaurHerdBehavior::SetHerdState(ENPC_HerdState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    StateTimer = 0.0f;

    switch (NewState)
    {
        case ENPC_HerdState::Grazing:
            break;
        case ENPC_HerdState::Moving:
            // Set random target location within movement range
            TargetLocation = HerdCenterLocation + FVector(
                FMath::RandRange(-HerdRadius, HerdRadius),
                FMath::RandRange(-HerdRadius, HerdRadius),
                0.0f
            );
            break;
        case ENPC_HerdState::Alert:
            break;
        case ENPC_HerdState::Fleeing:
            TargetLocation = HerdCenterLocation + GetFleeDirection() * HerdRadius * 2.0f;
            break;
        case ENPC_HerdState::Drinking:
            TargetLocation = FindNearestWaterSource();
            break;
    }
}

bool UNPC_DinosaurHerdBehavior::DetectDanger()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    // Look for player or predators
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            // Check if it's a player or predator (simplified check)
            if (Pawn->IsPlayerControlled() || Pawn->GetName().Contains("TRex") || Pawn->GetName().Contains("Raptor"))
            {
                float Distance = FVector::Dist(HerdCenterLocation, Pawn->GetActorLocation());
                if (Distance < DangerDetectionRadius)
                {
                    return true;
                }
            }
        }
    }

    return false;
}

void UNPC_DinosaurHerdBehavior::UpdateHerdFormation()
{
    APawn* Leader = GetHerdLeader();
    if (!Leader)
    {
        return;
    }

    FVector LeaderLocation = Leader->GetActorLocation();

    for (int32 i = 0; i < HerdMembers.Num(); i++)
    {
        if (HerdMembers[i].bIsLeader || !HerdMembers[i].DinosaurPawn)
        {
            continue;
        }

        // Calculate desired position in formation
        float Angle = (2.0f * PI * i) / HerdMembers.Num();
        float FormationRadius = 500.0f;
        
        FVector DesiredPosition = LeaderLocation + FVector(
            FMath::Cos(Angle) * FormationRadius,
            FMath::Sin(Angle) * FormationRadius,
            0.0f
        );

        // Move towards desired position (simplified movement)
        FVector CurrentLocation = HerdMembers[i].DinosaurPawn->GetActorLocation();
        FVector Direction = (DesiredPosition - CurrentLocation).GetSafeNormal();
        
        // Update relative position
        HerdMembers[i].RelativePosition = DesiredPosition - LeaderLocation;
        HerdMembers[i].DistanceFromLeader = FVector::Dist(CurrentLocation, LeaderLocation);
    }
}

FVector UNPC_DinosaurHerdBehavior::GetFleeDirection()
{
    // Find the average direction away from all threats
    UWorld* World = GetWorld();
    if (!World)
    {
        return FVector::ForwardVector;
    }

    FVector FleeDirection = FVector::ZeroVector;
    int32 ThreatCount = 0;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (Pawn->IsPlayerControlled() || Pawn->GetName().Contains("TRex") || Pawn->GetName().Contains("Raptor"))
            {
                float Distance = FVector::Dist(HerdCenterLocation, Pawn->GetActorLocation());
                if (Distance < DangerDetectionRadius)
                {
                    FVector DirectionFromThreat = (HerdCenterLocation - Pawn->GetActorLocation()).GetSafeNormal();
                    FleeDirection += DirectionFromThreat;
                    ThreatCount++;
                }
            }
        }
    }

    if (ThreatCount > 0)
    {
        FleeDirection /= ThreatCount;
        return FleeDirection.GetSafeNormal();
    }

    // Default flee direction if no specific threat found
    return FVector::ForwardVector;
}

void UNPC_DinosaurHerdBehavior::UpdateGrazingBehavior(float DeltaTime)
{
    // Stay in grazing state for specified duration
    if (StateTimer > GrazingDuration)
    {
        // Randomly decide next action
        if (FMath::RandRange(0.0f, 1.0f) < 0.3f && IsNearWater())
        {
            SetHerdState(ENPC_HerdState::Drinking);
        }
        else
        {
            SetHerdState(ENPC_HerdState::Moving);
        }
    }
}

void UNPC_DinosaurHerdBehavior::UpdateMovingBehavior(float DeltaTime)
{
    MoveHerdToTarget(DeltaTime);

    // Check if reached target or timeout
    float DistanceToTarget = FVector::Dist(HerdCenterLocation, TargetLocation);
    if (DistanceToTarget < 200.0f || StateTimer > 20.0f)
    {
        SetHerdState(ENPC_HerdState::Grazing);
    }
}

void UNPC_DinosaurHerdBehavior::UpdateAlertBehavior(float DeltaTime)
{
    // Stay alert for a short time, then decide to flee or return to grazing
    if (StateTimer > 3.0f)
    {
        if (DetectDanger())
        {
            SetHerdState(ENPC_HerdState::Fleeing);
        }
        else
        {
            SetHerdState(ENPC_HerdState::Grazing);
        }
    }
}

void UNPC_DinosaurHerdBehavior::UpdateFleeingBehavior(float DeltaTime)
{
    MoveHerdToTarget(DeltaTime);

    // Stop fleeing when far enough from danger
    if (StateTimer > 10.0f && !DetectDanger())
    {
        SetHerdState(ENPC_HerdState::Grazing);
    }
}

void UNPC_DinosaurHerdBehavior::UpdateDrinkingBehavior(float DeltaTime)
{
    // Drink for a while then return to grazing
    if (StateTimer > 15.0f)
    {
        SetHerdState(ENPC_HerdState::Grazing);
    }
}

void UNPC_DinosaurHerdBehavior::FindNearbyDinosaurs()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            // Check if it's a herbivore dinosaur (simplified check by name)
            if (Pawn->GetName().Contains("Triceratops") || 
                Pawn->GetName().Contains("Brachiosaurus") || 
                Pawn->GetName().Contains("Parasaurolophus"))
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Pawn->GetActorLocation());
                if (Distance < HerdRadius && HerdMembers.Num() < MaxHerdSize)
                {
                    AddHerdMember(Pawn);
                }
            }
        }
    }
}

void UNPC_DinosaurHerdBehavior::CalculateHerdCenter()
{
    if (HerdMembers.Num() == 0)
    {
        HerdCenterLocation = GetOwner()->GetActorLocation();
        return;
    }

    FVector TotalLocation = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (const FNPC_HerdMember& Member : HerdMembers)
    {
        if (Member.DinosaurPawn)
        {
            TotalLocation += Member.DinosaurPawn->GetActorLocation();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        HerdCenterLocation = TotalLocation / ValidMembers;
    }
}

void UNPC_DinosaurHerdBehavior::MoveHerdToTarget(float DeltaTime)
{
    APawn* Leader = GetHerdLeader();
    if (!Leader)
    {
        return;
    }

    FVector Direction = (TargetLocation - HerdCenterLocation).GetSafeNormal();
    float Speed = (CurrentState == ENPC_HerdState::Fleeing) ? FleeSpeed : MovementSpeed;
    
    // Move leader towards target (simplified movement)
    FVector NewLocation = Leader->GetActorLocation() + Direction * Speed * DeltaTime;
    Leader->SetActorLocation(NewLocation);
}

APawn* UNPC_DinosaurHerdBehavior::GetHerdLeader()
{
    for (const FNPC_HerdMember& Member : HerdMembers)
    {
        if (Member.bIsLeader && Member.DinosaurPawn)
        {
            return Member.DinosaurPawn;
        }
    }
    return nullptr;
}

bool UNPC_DinosaurHerdBehavior::IsNearWater()
{
    // Simplified water detection - in real implementation would check for water bodies
    return FMath::RandRange(0.0f, 1.0f) < 0.2f;
}

FVector UNPC_DinosaurHerdBehavior::FindNearestWaterSource()
{
    // Simplified - return a random location that could be water
    return HerdCenterLocation + FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        -50.0f
    );
}
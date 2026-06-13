#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    CurrentState = ECombat_TacticalState::Idle;
    CurrentTarget = nullptr;
    StateChangeTimer = 0.0f;
    LastAttackTime = 0.0f;
    LastKnownTargetLocation = FVector::ZeroVector;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tactical profile based on dinosaur type
    if (TacticalProfile.DinosaurType == ECombat_DinosaurType::Apex)
    {
        TacticalProfile.Aggression = FMath::FRandRange(0.8f, 1.0f);
        TacticalProfile.Intelligence = FMath::FRandRange(0.7f, 0.9f);
        TacticalProfile.AttackRange = 600.0f;
    }
    else if (TacticalProfile.DinosaurType == ECombat_DinosaurType::PackHunter)
    {
        TacticalProfile.PackCoordination = FMath::FRandRange(0.6f, 0.9f);
        TacticalProfile.Intelligence = FMath::FRandRange(0.5f, 0.8f);
        TacticalProfile.AttackRange = 400.0f;
    }
    else if (TacticalProfile.DinosaurType == ECombat_DinosaurType::Herbivore)
    {
        TacticalProfile.Aggression = FMath::FRandRange(0.2f, 0.5f);
        TacticalProfile.FleeThreshold = 0.4f;
    }
    
    // Find nearby pack members
    if (TacticalProfile.DinosaurType == ECombat_DinosaurType::PackHunter)
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor != GetOwner() && FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation()) < 2000.0f)
            {
                UCombat_TacticalAI* OtherAI = Actor->FindComponentByClass<UCombat_TacticalAI>();
                if (OtherAI && OtherAI->TacticalProfile.DinosaurType == ECombat_DinosaurType::PackHunter)
                {
                    PackMembers.Add(Actor);
                }
            }
        }
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateStateTimer(DeltaTime);
    UpdateTargetTracking();
    ProcessTacticalLogic();
    UpdateTacticalDecision(DeltaTime);
    
    if (TacticalProfile.DinosaurType == ECombat_DinosaurType::PackHunter)
    {
        HandlePackBehavior();
    }
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        // State-specific initialization
        switch (NewState)
        {
            case ECombat_TacticalState::Attacking:
                LastAttackTime = GetWorld()->GetTimeSeconds();
                break;
            case ECombat_TacticalState::Circling:
                // Calculate circling position
                break;
            case ECombat_TacticalState::Ambushing:
                // Find ambush position
                break;
        }
    }
}

void UCombat_TacticalAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        
        // Inform pack members of new target
        if (TacticalProfile.DinosaurType == ECombat_DinosaurType::PackHunter)
        {
            CoordinateWithPack();
        }
    }
}

void UCombat_TacticalAI::UpdateTacticalDecision(float DeltaTime)
{
    if (!CurrentTarget)
    {
        // Look for targets
        TArray<AActor*> PotentialTargets;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PotentialTargets);
        
        float ClosestDistance = TacticalProfile.AttackRange * 2.0f;
        AActor* ClosestTarget = nullptr;
        
        for (AActor* Actor : PotentialTargets)
        {
            if (Actor != GetOwner())
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), GetOwner()->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    ClosestDistance = Distance;
                    ClosestTarget = Actor;
                }
            }
        }
        
        if (ClosestTarget && ClosestDistance < TacticalProfile.AttackRange)
        {
            SetTarget(ClosestTarget);
            SetTacticalState(ECombat_TacticalState::Stalking);
        }
        return;
    }
    
    // Target exists - make tactical decisions
    float DistanceToTarget = FVector::Dist(CurrentTarget->GetActorLocation(), GetOwner()->GetActorLocation());
    
    // Check if should flee
    if (ShouldFlee())
    {
        SetTacticalState(ECombat_TacticalState::Fleeing);
        return;
    }
    
    // Tactical state machine
    switch (CurrentState)
    {
        case ECombat_TacticalState::Idle:
            if (DistanceToTarget < TacticalProfile.AttackRange)
            {
                SetTacticalState(ECombat_TacticalState::Stalking);
            }
            break;
            
        case ECombat_TacticalState::Stalking:
            if (DistanceToTarget < TacticalProfile.AttackRange * 0.5f)
            {
                if (TacticalProfile.Intelligence > 0.6f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
                {
                    SetTacticalState(ECombat_TacticalState::Circling);
                }
                else
                {
                    SetTacticalState(ECombat_TacticalState::Attacking);
                }
            }
            break;
            
        case ECombat_TacticalState::Circling:
            if (StateChangeTimer > 3.0f)
            {
                SetTacticalState(ECombat_TacticalState::Attacking);
            }
            break;
            
        case ECombat_TacticalState::Attacking:
            if (StateChangeTimer > 2.0f)
            {
                if (TacticalProfile.Intelligence > 0.5f)
                {
                    SetTacticalState(ECombat_TacticalState::Retreating);
                }
                else
                {
                    SetTacticalState(ECombat_TacticalState::Stalking);
                }
            }
            break;
            
        case ECombat_TacticalState::Retreating:
            if (StateChangeTimer > 1.5f)
            {
                SetTacticalState(ECombat_TacticalState::Circling);
            }
            break;
    }
}

void UCombat_TacticalAI::ExecuteAttackPattern()
{
    if (!CurrentTarget) return;
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    
    // Move towards target for attack
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Apply attack movement
    FVector NewLocation = OwnerLocation + Direction * 300.0f * GetWorld()->GetDeltaSeconds();
    OwnerPawn->SetActorLocation(NewLocation);
    
    // Face target
    FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(OwnerLocation, TargetLocation);
    OwnerPawn->SetActorRotation(FMath::RInterpTo(OwnerPawn->GetActorRotation(), LookRotation, GetWorld()->GetDeltaSeconds(), 2.0f));
}

void UCombat_TacticalAI::ExecuteFlankingManeuver()
{
    if (!CurrentTarget) return;
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    
    // Calculate flanking position (90 degrees offset)
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
    
    FVector FlankPosition = TargetLocation + FlankDirection * TacticalProfile.CirclingDistance;
    FVector MoveDirection = (FlankPosition - OwnerLocation).GetSafeNormal();
    
    FVector NewLocation = OwnerLocation + MoveDirection * 200.0f * GetWorld()->GetDeltaSeconds();
    OwnerPawn->SetActorLocation(NewLocation);
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    if (PackMembers.Num() == 0) return;
    
    // Share target information with pack
    for (AActor* PackMember : PackMembers)
    {
        if (IsValid(PackMember))
        {
            UCombat_TacticalAI* PackAI = PackMember->FindComponentByClass<UCombat_TacticalAI>();
            if (PackAI && !PackAI->CurrentTarget && CurrentTarget)
            {
                PackAI->SetTarget(CurrentTarget);
            }
        }
    }
}

bool UCombat_TacticalAI::IsTargetInRange() const
{
    if (!CurrentTarget) return false;
    
    float Distance = FVector::Dist(CurrentTarget->GetActorLocation(), GetOwner()->GetActorLocation());
    return Distance <= TacticalProfile.AttackRange;
}

bool UCombat_TacticalAI::ShouldFlee() const
{
    // Simple flee logic based on health and aggression
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return false;
    
    // Placeholder health check - would integrate with actual health system
    float HealthRatio = 1.0f; // Would get from health component
    
    return HealthRatio < TacticalProfile.FleeThreshold && TacticalProfile.Aggression < 0.6f;
}

FVector UCombat_TacticalAI::CalculateOptimalPosition() const
{
    if (!CurrentTarget) return GetOwner()->GetActorLocation();
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate position based on current state
    switch (CurrentState)
    {
        case ECombat_TacticalState::Circling:
        {
            FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
            FVector CircleDirection = FVector::CrossProduct(ToTarget, FVector::UpVector).GetSafeNormal();
            return TargetLocation + CircleDirection * TacticalProfile.CirclingDistance;
        }
        
        case ECombat_TacticalState::Ambushing:
        {
            // Position behind cover near target
            return TargetLocation + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
        }
        
        default:
            return TargetLocation;
    }
}

void UCombat_TacticalAI::UpdateStateTimer(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
}

void UCombat_TacticalAI::ProcessTacticalLogic()
{
    switch (CurrentState)
    {
        case ECombat_TacticalState::Attacking:
            ExecuteAttackPattern();
            break;
            
        case ECombat_TacticalState::Circling:
            ExecuteFlankingManeuver();
            break;
            
        case ECombat_TacticalState::Fleeing:
        {
            APawn* OwnerPawn = Cast<APawn>(GetOwner());
            if (OwnerPawn && CurrentTarget)
            {
                FVector FleeDirection = (OwnerPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
                FVector NewLocation = OwnerPawn->GetActorLocation() + FleeDirection * 400.0f * GetWorld()->GetDeltaSeconds();
                OwnerPawn->SetActorLocation(NewLocation);
            }
            break;
        }
    }
}

void UCombat_TacticalAI::HandlePackBehavior()
{
    if (PackMembers.Num() == 0) return;
    
    // Clean up invalid pack members
    PackMembers.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
    
    // Coordinate pack tactics
    if (CurrentTarget && CurrentState == ECombat_TacticalState::Attacking)
    {
        // Assign different roles to pack members
        for (int32 i = 0; i < PackMembers.Num(); i++)
        {
            UCombat_TacticalAI* PackAI = PackMembers[i]->FindComponentByClass<UCombat_TacticalAI>();
            if (PackAI)
            {
                if (i % 2 == 0)
                {
                    PackAI->SetTacticalState(ECombat_TacticalState::Circling);
                }
                else
                {
                    PackAI->SetTacticalState(ECombat_TacticalState::Attacking);
                }
            }
        }
    }
}

void UCombat_TacticalAI::UpdateTargetTracking()
{
    if (CurrentTarget)
    {
        if (IsValid(CurrentTarget))
        {
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        }
        else
        {
            CurrentTarget = nullptr;
            SetTacticalState(ECombat_TacticalState::Idle);
        }
    }
}
#include "Combat_TacticalAI.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default values
    CurrentState = ECombat_CombatState::Idle;
    TacticalRole = ECombat_TacticalRole::Scout;
    AggressionLevel = 0.7f;
    CoordinationRadius = 1000.0f;
    StateTimer = 0.0f;
    LastCoordinationTime = 0.0f;
    bIsPackLeader = false;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Store initial position for reference
    if (GetOwner())
    {
        InitialPosition = GetOwner()->GetActorLocation();
    }
    
    // Try to find and join nearby pack members
    UWorld* World = GetWorld();
    if (World)
    {
        // Delayed pack formation to allow all components to initialize
        FTimerHandle TimerHandle;
        World->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            CoordinateWithPack();
        }, 2.0f, false);
    }
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    StateTimer += DeltaTime;
    
    // Update combat state based on current situation
    UpdateCombatState();
    
    // Execute state-specific behavior
    switch (CurrentState)
    {
        case ECombat_CombatState::Idle:
            ProcessIdleState();
            break;
        case ECombat_CombatState::Hunting:
            ProcessHuntingState();
            break;
        case ECombat_CombatState::Attacking:
            ProcessAttackingState();
            break;
        case ECombat_CombatState::Defending:
            ProcessDefendingState();
            break;
        case ECombat_CombatState::Fleeing:
            ProcessFleeingState();
            break;
        case ECombat_CombatState::Circling:
            ProcessCirclingState();
            break;
    }
    
    // Coordinate with pack members periodically
    if (StateTimer - LastCoordinationTime > 1.0f)
    {
        CoordinateWithPack();
        LastCoordinationTime = StateTimer;
    }
}

void UCombat_TacticalAI::SetCombatTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        
        // Inform pack members of new target
        for (UCombat_TacticalAI* PackMember : PackMembers)
        {
            if (PackMember && PackMember != this)
            {
                PackMember->SetCombatTarget(NewTarget);
            }
        }
    }
}

void UCombat_TacticalAI::JoinPack(UCombat_TacticalAI* PackLeader)
{
    if (!PackLeader || PackLeader == this)
        return;
    
    // Add this AI to the pack leader's members
    PackLeader->PackMembers.AddUnique(this);
    
    // Copy pack members from leader
    PackMembers = PackLeader->PackMembers;
    
    // Assign tactical role based on pack size
    AssignTacticalRoles();
}

void UCombat_TacticalAI::ExecuteTacticalManeuver()
{
    if (!GetOwner() || !CurrentTarget)
        return;
    
    FVector TargetLocation = FVector::ZeroVector;
    
    switch (TacticalRole)
    {
        case ECombat_TacticalRole::Alpha:
            // Alpha leads direct assault
            TargetLocation = CurrentTarget->GetActorLocation();
            break;
            
        case ECombat_TacticalRole::Flanker:
            // Flankers move to sides
            TargetLocation = GetFlankingPosition();
            break;
            
        case ECombat_TacticalRole::Distractor:
            // Distractors circle and harass
            TargetLocation = GetCirclingPosition();
            break;
            
        case ECombat_TacticalRole::Defender:
            // Defenders hold position
            TargetLocation = InitialPosition;
            break;
            
        case ECombat_TacticalRole::Scout:
            // Scouts maintain distance and observe
            TargetLocation = CurrentTarget->GetActorLocation() + 
                           (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal() * 600.0f;
            break;
    }
    
    // Move towards tactical position (simplified - in real implementation would use AI movement)
    if (TargetLocation != FVector::ZeroVector)
    {
        FVector Direction = (TargetLocation - GetOwner()->GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetOwner()->GetActorLocation() + Direction * 100.0f * GetWorld()->GetDeltaSeconds();
        GetOwner()->SetActorLocation(NewLocation);
    }
}

bool UCombat_TacticalAI::IsInAttackRange() const
{
    if (!CurrentTarget || !GetOwner())
        return false;
    
    float Distance = GetDistanceToTarget();
    return Distance <= TacticalData.AttackRange;
}

FVector UCombat_TacticalAI::GetFlankingPosition() const
{
    if (!CurrentTarget || !GetOwner())
        return FVector::ZeroVector;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate perpendicular position for flanking
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    return TargetLocation + FlankDirection * TacticalData.FlankingDistance;
}

void UCombat_TacticalAI::UpdateCombatState()
{
    if (!GetOwner())
        return;
    
    // State transition logic
    ECombat_CombatState NewState = CurrentState;
    
    if (CurrentTarget)
    {
        float DistanceToTarget = GetDistanceToTarget();
        bool CanSee = CanSeeTarget();
        
        if (ShouldRetreat())
        {
            NewState = ECombat_CombatState::Fleeing;
        }
        else if (IsInAttackRange() && CanSee)
        {
            NewState = ECombat_CombatState::Attacking;
        }
        else if (DistanceToTarget < TacticalData.CirclingRadius)
        {
            NewState = ECombat_CombatState::Circling;
        }
        else if (CanSee)
        {
            NewState = ECombat_CombatState::Hunting;
        }
        else
        {
            NewState = ECombat_CombatState::Idle;
        }
    }
    else
    {
        NewState = ECombat_CombatState::Idle;
    }
    
    // Reset state timer on state change
    if (NewState != CurrentState)
    {
        StateTimer = 0.0f;
        CurrentState = NewState;
    }
}

void UCombat_TacticalAI::ProcessIdleState()
{
    // Look for targets or return to initial position
    if (!CurrentTarget)
    {
        // Simple target acquisition - find player or other actors
        UWorld* World = GetWorld();
        if (World)
        {
            APawn* Player = World->GetFirstPlayerController()->GetPawn();
            if (Player)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
                if (Distance < CoordinationRadius)
                {
                    SetCombatTarget(Player);
                }
            }
        }
    }
}

void UCombat_TacticalAI::ProcessHuntingState()
{
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
        ExecuteTacticalManeuver();
    }
}

void UCombat_TacticalAI::ProcessAttackingState()
{
    // Execute attack behavior
    ExecuteTacticalManeuver();
    
    // Simple attack logic - in real implementation would trigger animations/damage
    if (StateTimer > 2.0f) // Attack cooldown
    {
        StateTimer = 0.0f;
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, 
                FString::Printf(TEXT("%s attacks!"), *GetOwner()->GetName()));
        }
    }
}

void UCombat_TacticalAI::ProcessDefendingState()
{
    // Hold defensive position
    FVector DefensivePosition = InitialPosition;
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    if (FVector::Dist(CurrentLocation, DefensivePosition) > 100.0f)
    {
        FVector Direction = (DefensivePosition - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * 50.0f * GetWorld()->GetDeltaSeconds();
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UCombat_TacticalAI::ProcessFleeingState()
{
    if (CurrentTarget)
    {
        // Move away from target
        FVector FleeDirection = (GetOwner()->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetOwner()->GetActorLocation() + FleeDirection * 200.0f * GetWorld()->GetDeltaSeconds();
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UCombat_TacticalAI::ProcessCirclingState()
{
    FVector CirclePosition = GetCirclingPosition();
    if (CirclePosition != FVector::ZeroVector)
    {
        FVector Direction = (CirclePosition - GetOwner()->GetActorLocation()).GetSafeNormal();
        FVector NewLocation = GetOwner()->GetActorLocation() + Direction * 150.0f * GetWorld()->GetDeltaSeconds();
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    if (!GetOwner())
        return;
    
    // Find nearby pack members
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, GetOwner()->GetClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner())
            continue;
        
        UCombat_TacticalAI* OtherAI = Actor->FindComponentByClass<UCombat_TacticalAI>();
        if (OtherAI)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= CoordinationRadius)
            {
                PackMembers.AddUnique(OtherAI);
                OtherAI->PackMembers.AddUnique(this);
            }
        }
    }
    
    // Assign roles if we have pack members
    if (PackMembers.Num() > 0)
    {
        AssignTacticalRoles();
    }
}

void UCombat_TacticalAI::AssignTacticalRoles()
{
    if (PackMembers.Num() == 0)
    {
        TacticalRole = ECombat_TacticalRole::Scout;
        return;
    }
    
    // Simple role assignment based on pack position
    int32 MyIndex = PackMembers.IndexOfByKey(this);
    if (MyIndex == INDEX_NONE)
        MyIndex = 0;
    
    switch (MyIndex % 5)
    {
        case 0:
            TacticalRole = ECombat_TacticalRole::Alpha;
            bIsPackLeader = true;
            break;
        case 1:
        case 2:
            TacticalRole = ECombat_TacticalRole::Flanker;
            break;
        case 3:
            TacticalRole = ECombat_TacticalRole::Distractor;
            break;
        case 4:
            TacticalRole = ECombat_TacticalRole::Scout;
            break;
    }
}

UCombat_TacticalAI* UCombat_TacticalAI::FindPackLeader()
{
    for (UCombat_TacticalAI* Member : PackMembers)
    {
        if (Member && Member->bIsPackLeader)
        {
            return Member;
        }
    }
    return nullptr;
}

float UCombat_TacticalAI::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return FLT_MAX;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool UCombat_TacticalAI::CanSeeTarget() const
{
    if (!CurrentTarget || !GetOwner())
        return false;
    
    // Simple line-of-sight check (in real implementation would use proper ray casting)
    float Distance = GetDistanceToTarget();
    return Distance <= CoordinationRadius;
}

FVector UCombat_TacticalAI::GetCirclingPosition() const
{
    if (!CurrentTarget || !GetOwner())
        return FVector::ZeroVector;
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    float Angle = StateTimer * 45.0f; // Circle at 45 degrees per second
    
    FVector CircleOffset = FVector(
        FMath::Cos(FMath::DegreesToRadians(Angle)) * TacticalData.CirclingRadius,
        FMath::Sin(FMath::DegreesToRadians(Angle)) * TacticalData.CirclingRadius,
        0.0f
    );
    
    return TargetLocation + CircleOffset;
}

bool UCombat_TacticalAI::ShouldRetreat() const
{
    // Simple retreat logic based on pack size and aggression
    if (PackMembers.Num() < 2 && AggressionLevel < 0.5f)
    {
        return true;
    }
    
    // Retreat if target is too powerful (simplified check)
    if (CurrentTarget)
    {
        float TargetScale = CurrentTarget->GetActorScale3D().X;
        float MyScale = GetOwner()->GetActorScale3D().X;
        
        if (TargetScale > MyScale * 2.0f && PackMembers.Num() < 3)
        {
            return true;
        }
    }
    
    return false;
}
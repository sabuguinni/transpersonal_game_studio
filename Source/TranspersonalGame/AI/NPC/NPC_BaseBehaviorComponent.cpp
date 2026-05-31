#include "NPC_BaseBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"

UNPC_BaseBehaviorComponent::UNPC_BaseBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = ENPC_BehaviorState::Idle;
    StateChangeTimer = 0.0f;
    WanderRadius = 2000.0f;
    PlayerDetectionRange = 1500.0f;
    FleeDistance = 3000.0f;
    
    bHasDailyRoutine = true;
    DailyRoutineStartHour = 8.0f;
    DailyRoutineEndHour = 18.0f;
    
    CachedPlayerActor = nullptr;
    InitialLocation = FVector::ZeroVector;
}

void UNPC_BaseBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    InitialLocation = GetOwner()->GetActorLocation();
    
    // Cache player actor for performance
    CachedPlayerActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    
    // Start with idle state
    SetBehaviorState(ENPC_BehaviorState::Idle);
}

void UNPC_BaseBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateBehavior(DeltaTime);
    
    // Update memory about player
    if (CachedPlayerActor)
    {
        UpdateMemory(CachedPlayerActor);
    }
    
    // Execute daily routine if enabled
    if (bHasDailyRoutine)
    {
        ExecuteDailyRoutine();
    }
}

void UNPC_BaseBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateChangeTimer = 0.0f;
        
        // Clear existing timer
        GetWorld()->GetTimerManager().ClearTimer(StateTimerHandle);
        
        // Set new timer based on state
        float TimerDuration = 5.0f; // Default
        switch (NewState)
        {
            case ENPC_BehaviorState::Idle:
                TimerDuration = FMath::RandRange(3.0f, 8.0f);
                break;
            case ENPC_BehaviorState::Wandering:
                TimerDuration = FMath::RandRange(5.0f, 15.0f);
                break;
            case ENPC_BehaviorState::Interacting:
                TimerDuration = FMath::RandRange(2.0f, 6.0f);
                break;
            case ENPC_BehaviorState::Fleeing:
                TimerDuration = FMath::RandRange(8.0f, 12.0f);
                break;
            case ENPC_BehaviorState::Sleeping:
                TimerDuration = FMath::RandRange(20.0f, 40.0f);
                break;
            case ENPC_BehaviorState::Working:
                TimerDuration = FMath::RandRange(10.0f, 25.0f);
                break;
        }
        
        GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &UNPC_BaseBehaviorComponent::OnStateTimerExpired, TimerDuration, false);
    }
}

void UNPC_BaseBehaviorComponent::UpdateBehavior(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
    
    // Check for player interaction
    if (IsPlayerNearby() && CachedPlayerActor)
    {
        ReactToPlayer(CachedPlayerActor);
    }
    
    // State-specific behavior updates
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            // Randomly transition to wandering
            if (StateChangeTimer > 5.0f && FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                SetBehaviorState(ENPC_BehaviorState::Wandering);
            }
            break;
            
        case ENPC_BehaviorState::Wandering:
            // Move towards random location within wander radius
            if (StateChangeTimer > 2.0f)
            {
                FVector RandomDirection = FMath::VRand();
                RandomDirection.Z = 0.0f; // Keep on ground
                RandomDirection.Normalize();
                
                FVector TargetLocation = InitialLocation + (RandomDirection * FMath::RandRange(0.0f, WanderRadius));
                
                // Simple movement towards target (in real implementation, use AI movement)
                FVector CurrentLocation = GetOwner()->GetActorLocation();
                FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
                
                // Log movement for debugging
                UE_LOG(LogTemp, Log, TEXT("NPC %s wandering towards %s"), *GetOwner()->GetName(), *TargetLocation.ToString());
            }
            break;
            
        case ENPC_BehaviorState::Fleeing:
            // Move away from player
            if (CachedPlayerActor)
            {
                FVector PlayerLocation = CachedPlayerActor->GetActorLocation();
                FVector NPCLocation = GetOwner()->GetActorLocation();
                FVector FleeDirection = (NPCLocation - PlayerLocation).GetSafeNormal();
                
                // Update fear level
                BehaviorMemory.FearLevel = FMath::Min(BehaviorMemory.FearLevel + DeltaTime * 0.5f, 1.0f);
                
                UE_LOG(LogTemp, Log, TEXT("NPC %s fleeing from player, fear level: %f"), *GetOwner()->GetName(), BehaviorMemory.FearLevel);
            }
            break;
            
        default:
            break;
    }
}

bool UNPC_BaseBehaviorComponent::IsPlayerNearby()
{
    if (!CachedPlayerActor)
        return false;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CachedPlayerActor->GetActorLocation());
    return Distance <= PlayerDetectionRange;
}

void UNPC_BaseBehaviorComponent::ReactToPlayer(AActor* PlayerActor)
{
    if (!PlayerActor)
        return;
        
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    // If player is too close and NPC is fearful, flee
    if (Distance < FleeDistance && BehaviorMemory.FearLevel > 0.3f)
    {
        if (CurrentState != ENPC_BehaviorState::Fleeing)
        {
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
            UE_LOG(LogTemp, Log, TEXT("NPC %s starting to flee from player"), *GetOwner()->GetName());
        }
    }
    // If player is nearby but not threatening, interact
    else if (Distance < PlayerDetectionRange * 0.5f && !BehaviorMemory.bPlayerIsHostile)
    {
        if (CurrentState != ENPC_BehaviorState::Interacting)
        {
            SetBehaviorState(ENPC_BehaviorState::Interacting);
            UE_LOG(LogTemp, Log, TEXT("NPC %s starting interaction with player"), *GetOwner()->GetName());
        }
    }
}

void UNPC_BaseBehaviorComponent::UpdateMemory(AActor* PlayerActor)
{
    if (!PlayerActor)
        return;
        
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    // Update last known player location if nearby
    if (Distance <= PlayerDetectionRange)
    {
        BehaviorMemory.LastKnownPlayerLocation = PlayerActor->GetActorLocation();
        BehaviorMemory.LastPlayerInteractionTime = CurrentTime;
    }
    
    // Decay fear over time if player is not nearby
    if (Distance > PlayerDetectionRange)
    {
        BehaviorMemory.FearLevel = FMath::Max(BehaviorMemory.FearLevel - GetWorld()->GetDeltaSeconds() * 0.1f, 0.0f);
    }
    
    // Check for hostile behavior (simplified - in real game, check for weapons, aggressive actions)
    if (Distance < 500.0f && CurrentState == ENPC_BehaviorState::Fleeing)
    {
        BehaviorMemory.bPlayerIsHostile = true;
        BehaviorMemory.FearLevel = 1.0f;
    }
}

void UNPC_BaseBehaviorComponent::ExecuteDailyRoutine()
{
    // Get current time of day (simplified - in real game, get from day/night cycle system)
    float CurrentHour = FMath::Fmod(GetWorld()->GetTimeSeconds() / 3600.0f, 24.0f); // Assuming 1 hour = 3600 seconds in game
    
    // Work during day hours
    if (CurrentHour >= DailyRoutineStartHour && CurrentHour <= DailyRoutineEndHour)
    {
        if (CurrentState == ENPC_BehaviorState::Sleeping)
        {
            SetBehaviorState(ENPC_BehaviorState::Working);
            UE_LOG(LogTemp, Log, TEXT("NPC %s waking up for daily routine"), *GetOwner()->GetName());
        }
    }
    // Sleep during night hours
    else
    {
        if (CurrentState != ENPC_BehaviorState::Sleeping && CurrentState != ENPC_BehaviorState::Fleeing)
        {
            SetBehaviorState(ENPC_BehaviorState::Sleeping);
            UE_LOG(LogTemp, Log, TEXT("NPC %s going to sleep"), *GetOwner()->GetName());
        }
    }
}

void UNPC_BaseBehaviorComponent::OnStateTimerExpired()
{
    // Transition to next logical state
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Idle:
            SetBehaviorState(ENPC_BehaviorState::Wandering);
            break;
        case ENPC_BehaviorState::Wandering:
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;
        case ENPC_BehaviorState::Interacting:
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;
        case ENPC_BehaviorState::Fleeing:
            SetBehaviorState(ENPC_BehaviorState::Idle);
            break;
        case ENPC_BehaviorState::Working:
            // Continue working during day hours
            break;
        case ENPC_BehaviorState::Sleeping:
            // Continue sleeping during night hours
            break;
    }
}
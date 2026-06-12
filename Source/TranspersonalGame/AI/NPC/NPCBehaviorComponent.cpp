#include "NPCBehaviorComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with default idle state
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    StateChangeTimer = 0.0f;
    
    // Add default daily schedule if none exists
    if (DailySchedule.Num() == 0)
    {
        FNPC_DailySchedule MorningWork;
        MorningWork.StartHour = 6.0f;
        MorningWork.EndHour = 12.0f;
        MorningWork.ActivityType = ENPC_BehaviorState::Working;
        MorningWork.Priority = 1.0f;
        DailySchedule.Add(MorningWork);
        
        FNPC_DailySchedule AfternoonRest;
        AfternoonRest.StartHour = 12.0f;
        AfternoonRest.EndHour = 14.0f;
        AfternoonRest.ActivityType = ENPC_BehaviorState::Resting;
        AfternoonRest.Priority = 0.8f;
        DailySchedule.Add(AfternoonRest);
        
        FNPC_DailySchedule EveningPatrol;
        EveningPatrol.StartHour = 14.0f;
        EveningPatrol.EndHour = 18.0f;
        EveningPatrol.ActivityType = ENPC_BehaviorState::Patrolling;
        EveningPatrol.Priority = 1.2f;
        DailySchedule.Add(EveningPatrol);
    }
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    // Update memory decay
    UpdateMemoryDecay(DeltaTime);
    
    // Update state timer
    StateChangeTimer += DeltaTime;
    
    // Execute current behavior
    ExecuteCurrentBehavior(DeltaTime);
    
    // Check for player proximity and react accordingly
    if (IsPlayerInSight())
    {
        float DistanceToPlayer = GetDistanceToPlayer();
        if (DistanceToPlayer < FleeDistance && CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
        {
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
        }
        else if (DistanceToPlayer < SocialDistance && CurrentBehaviorState == ENPC_BehaviorState::Idle)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = 0.0f;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior state to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
               static_cast<int32>(NewState));
    }
}

void UNPC_BehaviorComponent::AddScheduleEntry(const FNPC_DailySchedule& ScheduleEntry)
{
    DailySchedule.Add(ScheduleEntry);
    
    // Sort schedule by start time
    DailySchedule.Sort([](const FNPC_DailySchedule& A, const FNPC_DailySchedule& B) {
        return A.StartHour < B.StartHour;
    });
}

void UNPC_BehaviorComponent::UpdateSchedule(float CurrentTimeOfDay)
{
    // Find appropriate schedule entry for current time
    for (int32 i = 0; i < DailySchedule.Num(); i++)
    {
        const FNPC_DailySchedule& Schedule = DailySchedule[i];
        if (CurrentTimeOfDay >= Schedule.StartHour && CurrentTimeOfDay < Schedule.EndHour)
        {
            CurrentScheduleIndex = i;
            
            // Only change behavior if not in emergency state
            if (CurrentBehaviorState != ENPC_BehaviorState::Fleeing && 
                CurrentBehaviorState != ENPC_BehaviorState::Investigating)
            {
                SetBehaviorState(Schedule.ActivityType);
            }
            break;
        }
    }
}

void UNPC_BehaviorComponent::RememberActor(AActor* Actor, float Importance, bool bIsHostile)
{
    if (!Actor)
        return;
    
    // Check if actor is already in memory
    for (FNPC_Memory& Memory : MemoryBank)
    {
        if (Memory.RememberedActor == Actor)
        {
            // Update existing memory
            Memory.LastKnownLocation = Actor->GetActorLocation();
            Memory.Timestamp = GetWorld()->GetTimeSeconds();
            Memory.Importance = FMath::Max(Memory.Importance, Importance);
            Memory.bIsHostile = Memory.bIsHostile || bIsHostile;
            return;
        }
    }
    
    // Add new memory
    FNPC_Memory NewMemory;
    NewMemory.RememberedActor = Actor;
    NewMemory.LastKnownLocation = Actor->GetActorLocation();
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.Importance = Importance;
    NewMemory.bIsHostile = bIsHostile;
    
    MemoryBank.Add(NewMemory);
    
    // Remove oldest memories if we exceed the limit
    if (MemoryBank.Num() > MaxMemories)
    {
        // Sort by importance and timestamp, remove least important
        MemoryBank.Sort([](const FNPC_Memory& A, const FNPC_Memory& B) {
            return A.Importance > B.Importance;
        });
        MemoryBank.RemoveAt(MemoryBank.Num() - 1);
    }
}

FNPC_Memory UNPC_BehaviorComponent::GetMemoryOfActor(AActor* Actor)
{
    for (const FNPC_Memory& Memory : MemoryBank)
    {
        if (Memory.RememberedActor == Actor)
        {
            return Memory;
        }
    }
    
    // Return empty memory if not found
    return FNPC_Memory();
}

void UNPC_BehaviorComponent::ForgetActor(AActor* Actor)
{
    MemoryBank.RemoveAll([Actor](const FNPC_Memory& Memory) {
        return Memory.RememberedActor == Actor;
    });
}

void UNPC_BehaviorComponent::OnPlayerSighted(AActor* Player)
{
    if (Player)
    {
        RememberActor(Player, 2.0f, false);
        
        // React based on current state and distance
        float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        
        if (Distance < FleeDistance)
        {
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
        }
        else if (CurrentBehaviorState == ENPC_BehaviorState::Idle || 
                 CurrentBehaviorState == ENPC_BehaviorState::Patrolling)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
        }
    }
}

void UNPC_BehaviorComponent::OnDangerDetected(AActor* DangerSource)
{
    if (DangerSource)
    {
        RememberActor(DangerSource, 3.0f, true);
        SetBehaviorState(ENPC_BehaviorState::Fleeing);
    }
}

void UNPC_BehaviorComponent::OnSocialInteraction(AActor* OtherNPC)
{
    if (OtherNPC && CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
    {
        RememberActor(OtherNPC, 1.5f, false);
        SetBehaviorState(ENPC_BehaviorState::Socializing);
    }
}

float UNPC_BehaviorComponent::GetDistanceToPlayer() const
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            return FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
        }
    }
    return 99999.0f;
}

bool UNPC_BehaviorComponent::IsPlayerInSight() const
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
            if (Distance <= SightRange)
            {
                // Simple line of sight check
                FHitResult HitResult;
                FVector Start = GetOwner()->GetActorLocation();
                FVector End = PlayerPawn->GetActorLocation();
                
                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    HitResult,
                    Start,
                    End,
                    ECollisionChannel::ECC_Visibility
                );
                
                return !bHit || HitResult.GetActor() == PlayerPawn;
            }
        }
    }
    return false;
}

void UNPC_BehaviorComponent::UpdateMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryBank.Num() - 1; i >= 0; i--)
    {
        FNPC_Memory& Memory = MemoryBank[i];
        
        // Decay importance over time
        float TimeSinceMemory = CurrentTime - Memory.Timestamp;
        Memory.Importance -= MemoryDecayRate * DeltaTime * (TimeSinceMemory / 60.0f); // Decay faster for older memories
        
        // Remove memories that have decayed too much or reference null actors
        if (Memory.Importance <= 0.0f || !IsValid(Memory.RememberedActor))
        {
            MemoryBank.RemoveAt(i);
        }
    }
}

void UNPC_BehaviorComponent::ExecuteCurrentBehavior(float DeltaTime)
{
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Idle:
            // Occasionally switch to patrolling if idle for too long
            if (StateChangeTimer > 10.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrolling);
            }
            break;
            
        case ENPC_BehaviorState::Patrolling:
            // Simple patrol behavior - could be enhanced with actual movement
            if (StateChangeTimer > 20.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        case ENPC_BehaviorState::Investigating:
            // Investigation timeout
            if (StateChangeTimer > 8.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        case ENPC_BehaviorState::Fleeing:
            // Stop fleeing after some time if no longer in danger
            if (StateChangeTimer > 5.0f && GetDistanceToPlayer() > FleeDistance * 1.5f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        case ENPC_BehaviorState::Socializing:
            // Social interaction timeout
            if (StateChangeTimer > 15.0f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        case ENPC_BehaviorState::Working:
            // Work behavior - stay in this state during work hours
            break;
            
        case ENPC_BehaviorState::Resting:
            // Rest behavior - stay in this state during rest hours
            break;
    }
}

FVector UNPC_BehaviorComponent::GetRandomPatrolPoint()
{
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.5f, PatrolRadius);
    return OwnerLocation + (RandomDirection * RandomDistance);
}

AActor* UNPC_BehaviorComponent::FindNearestNPC()
{
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    AActor* NearestNPC = nullptr;
    float NearestDistance = SocialDistance;
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor != GetOwner() && Actor != UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestNPC = Actor;
            }
        }
    }
    
    return NearestNPC;
}
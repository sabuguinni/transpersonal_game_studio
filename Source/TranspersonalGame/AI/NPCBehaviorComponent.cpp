#include "NPCBehaviorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

UNPCBehaviorComponent::UNPCBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default values
    CurrentState = ENPC_BehaviorState::Idle;
    PersonalityType = ENPC_PersonalityType::Cautious;
    
    Aggressiveness = 0.3f;
    Curiosity = 0.5f;
    Sociability = 0.4f;
    
    MaxShortTermMemories = 10;
    MaxLongTermMemories = 50;
    
    CurrentPatrolIndex = 0;
    PatrolRadius = 1000.0f;
    PatrolSpeed = 150.0f;
    
    SightRange = 1500.0f;
    HearingRange = 800.0f;
    AlertnessLevel = 0.0f;
    
    DailyRoutineTimer = 0.0f;
    RoutineChangeInterval = 300.0f; // 5 minutes
}

void UNPCBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize patrol route
    InitializePatrolRoute();
    
    // Set up timers
    GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, &UNPCBehaviorComponent::UpdateBehavior, 1.0f, true);
    GetWorld()->GetTimerManager().SetTimer(MemoryProcessTimer, this, &UNPCBehaviorComponent::ConsolidateMemories, 30.0f, true);
}

void UNPCBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update daily routine
    UpdateDailyRoutine(DeltaTime);
    
    // Process memories
    ProcessMemories(DeltaTime);
    
    // Update alertness based on environment
    if (DetectPlayer())
    {
        AlertnessLevel = FMath::Clamp(AlertnessLevel + DeltaTime * 0.5f, 0.0f, 1.0f);
    }
    else
    {
        AlertnessLevel = FMath::Clamp(AlertnessLevel - DeltaTime * 0.2f, 0.0f, 1.0f);
    }
}

void UNPCBehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Log state change for debugging
        UE_LOG(LogTemp, Warning, TEXT("NPC %s changed state to %d"), 
               *GetOwner()->GetName(), 
               static_cast<int32>(NewState));
        
        // Add memory of state change
        AddMemory(GetOwner()->GetActorLocation(), 
                 FString::Printf(TEXT("StateChange_%d"), static_cast<int32>(NewState)), 
                 0.3f);
    }
}

void UNPCBehaviorComponent::AddMemory(FVector Location, FString EventType, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    ShortTermMemory.Add(NewMemory);
    
    // Limit short-term memory size
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        ShortTermMemory.RemoveAt(0);
    }
}

void UNPCBehaviorComponent::ProcessMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay memory importance over time
    for (FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        float TimeSinceEvent = CurrentTime - Memory.Timestamp;
        float DecayRate = 0.1f; // Memories decay at 10% per minute
        Memory.Importance = FMath::Max(0.0f, Memory.Importance - (DecayRate * TimeSinceEvent / 60.0f));
    }
    
    // Remove very weak memories
    ShortTermMemory.RemoveAll([](const FNPC_MemoryEntry& Memory) {
        return Memory.Importance < 0.05f;
    });
}

void UNPCBehaviorComponent::InitializePatrolRoute()
{
    if (PatrolPoints.Num() == 0)
    {
        // Generate random patrol points around the NPC's starting location
        FVector StartLocation = GetOwner()->GetActorLocation();
        
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector PatrolPoint = StartLocation + FVector(
                FMath::Cos(Angle) * PatrolRadius,
                FMath::Sin(Angle) * PatrolRadius,
                0.0f
            );
            PatrolPoints.Add(PatrolPoint);
        }
    }
}

FVector UNPCBehaviorComponent::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
    {
        return GetOwner()->GetActorLocation();
    }
    
    CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    return PatrolPoints[CurrentPatrolIndex];
}

bool UNPCBehaviorComponent::DetectPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return false;
    }
    
    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    
    // Check if player is within sight range
    if (DistanceToPlayer <= SightRange)
    {
        // Add memory of player sighting
        AddMemory(PlayerPawn->GetActorLocation(), TEXT("PlayerSighting"), 0.8f);
        return true;
    }
    
    return false;
}

void UNPCBehaviorComponent::UpdateDailyRoutine(float DeltaTime)
{
    DailyRoutineTimer += DeltaTime;
    
    if (DailyRoutineTimer >= RoutineChangeInterval)
    {
        DailyRoutineTimer = 0.0f;
        
        // Change behavior based on personality and current state
        switch (PersonalityType)
        {
            case ENPC_PersonalityType::Cautious:
                if (AlertnessLevel > 0.5f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Fleeing);
                }
                else
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                break;
                
            case ENPC_PersonalityType::Curious:
                if (ShortTermMemory.Num() > 0)
                {
                    SetBehaviorState(ENPC_BehaviorState::Investigating);
                }
                else
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                break;
                
            case ENPC_PersonalityType::Social:
                SetBehaviorState(ENPC_BehaviorState::Socializing);
                break;
                
            case ENPC_PersonalityType::Aggressive:
                if (AlertnessLevel > 0.3f)
                {
                    SetBehaviorState(ENPC_BehaviorState::Investigating);
                }
                else
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
                break;
                
            default:
                SetBehaviorState(ENPC_BehaviorState::Idle);
                break;
        }
    }
}

void UNPCBehaviorComponent::UpdateBehavior()
{
    // This function is called every second to update NPC behavior
    switch (CurrentState)
    {
        case ENPC_BehaviorState::Patrolling:
            // Move towards next patrol point
            if (PatrolPoints.Num() > 0)
            {
                FVector CurrentLocation = GetOwner()->GetActorLocation();
                FVector TargetLocation = PatrolPoints[CurrentPatrolIndex];
                float DistanceToTarget = FVector::Dist(CurrentLocation, TargetLocation);
                
                if (DistanceToTarget < 100.0f) // Reached patrol point
                {
                    GetNextPatrolPoint();
                }
            }
            break;
            
        case ENPC_BehaviorState::Investigating:
            // Investigate the most recent important memory
            if (ShortTermMemory.Num() > 0)
            {
                FNPC_MemoryEntry* MostImportant = nullptr;
                float HighestImportance = 0.0f;
                
                for (FNPC_MemoryEntry& Memory : ShortTermMemory)
                {
                    if (Memory.Importance > HighestImportance)
                    {
                        HighestImportance = Memory.Importance;
                        MostImportant = &Memory;
                    }
                }
                
                if (MostImportant)
                {
                    // Move towards the memory location
                    float DistanceToMemory = FVector::Dist(GetOwner()->GetActorLocation(), MostImportant->Location);
                    if (DistanceToMemory < 150.0f)
                    {
                        // Finished investigating
                        SetBehaviorState(ENPC_BehaviorState::Idle);
                    }
                }
            }
            break;
            
        case ENPC_BehaviorState::Fleeing:
            // Move away from threats
            if (AlertnessLevel < 0.2f)
            {
                SetBehaviorState(ENPC_BehaviorState::Idle);
            }
            break;
            
        default:
            // Idle behavior - occasionally switch to patrolling
            if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
            {
                SetBehaviorState(ENPC_BehaviorState::Patrolling);
            }
            break;
    }
}

void UNPCBehaviorComponent::ConsolidateMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Move important short-term memories to long-term
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        FNPC_MemoryEntry& Memory = ShortTermMemory[i];
        float MemoryAge = CurrentTime - Memory.Timestamp;
        
        // Memories older than 5 minutes with high importance become long-term
        if (MemoryAge > 300.0f && Memory.Importance > 0.6f)
        {
            LongTermMemory.Add(Memory);
            ShortTermMemory.RemoveAt(i);
            
            // Limit long-term memory size
            if (LongTermMemory.Num() > MaxLongTermMemories)
            {
                LongTermMemory.RemoveAt(0);
            }
        }
    }
}

float UNPCBehaviorComponent::CalculateMemoryImportance(const FNPC_MemoryEntry& Memory, float CurrentTime)
{
    float TimeSinceEvent = CurrentTime - Memory.Timestamp;
    float BaseImportance = Memory.Importance;
    
    // Importance decays over time, but some events maintain importance longer
    if (Memory.EventType.Contains(TEXT("PlayerSighting")))
    {
        return BaseImportance * FMath::Exp(-TimeSinceEvent / 600.0f); // 10-minute half-life
    }
    else if (Memory.EventType.Contains(TEXT("Danger")))
    {
        return BaseImportance * FMath::Exp(-TimeSinceEvent / 1200.0f); // 20-minute half-life
    }
    else
    {
        return BaseImportance * FMath::Exp(-TimeSinceEvent / 300.0f); // 5-minute half-life
    }
}
#include "NPC_BehaviorComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentState = ENPC_BehaviorState::Idle;
    AlertRadius = 1500.0f;
    FleeRadius = 800.0f;
    MovementSpeed = 300.0f;
    CurrentPatrolIndex = 0;
    StateChangeTime = 0.0f;
    LastBehaviorUpdate = 0.0f;
    CurrentTarget = nullptr;

    // Initialize memory
    NPCMemory = FNPC_Memory();
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set up behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(BehaviorUpdateTimer, this, &UNPC_BehaviorComponent::UpdateBehaviorLogic, 1.0f, true);
        World->GetTimerManager().SetTimer(MemoryDecayTimer, this, &UNPC_BehaviorComponent::DecayMemory, 5.0f, true);
    }

    // Initialize patrol points around spawn location
    if (AActor* Owner = GetOwner())
    {
        FVector SpawnLocation = Owner->GetActorLocation();
        NPCMemory.PatrolPoints.Add(SpawnLocation + FVector(500, 0, 0));
        NPCMemory.PatrolPoints.Add(SpawnLocation + FVector(0, 500, 0));
        NPCMemory.PatrolPoints.Add(SpawnLocation + FVector(-500, 0, 0));
        NPCMemory.PatrolPoints.Add(SpawnLocation + FVector(0, -500, 0));
    }

    UE_LOG(LogTemp, Warning, TEXT("NPC Behavior Component initialized for %s"), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    LastBehaviorUpdate += DeltaTime;
    NPCMemory.TimeSinceThreatSeen += DeltaTime;

    // Process current behavior state
    switch (CurrentState)
    {
    case ENPC_BehaviorState::Idle:
        ProcessIdleBehavior(DeltaTime);
        break;
    case ENPC_BehaviorState::Patrolling:
        ProcessPatrolBehavior(DeltaTime);
        break;
    case ENPC_BehaviorState::Alert:
        ProcessAlertBehavior(DeltaTime);
        break;
    case ENPC_BehaviorState::Fleeing:
        ProcessFleeingBehavior(DeltaTime);
        break;
    default:
        break;
    }
}

void UNPC_BehaviorComponent::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_BehaviorState PreviousState = CurrentState;
        CurrentState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();

        UE_LOG(LogTemp, Log, TEXT("NPC %s changed state from %d to %d"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"), 
               (int32)PreviousState, (int32)NewState);

        // State-specific initialization
        switch (NewState)
        {
        case ENPC_BehaviorState::Alert:
            BroadcastAlert(TEXT("Threat detected! All units be ready!"));
            break;
        case ENPC_BehaviorState::Fleeing:
            BroadcastAlert(TEXT("Danger! All units retreat to safe distance!"));
            break;
        case ENPC_BehaviorState::Patrolling:
            CurrentPatrolIndex = 0;
            break;
        default:
            break;
        }
    }
}

void UNPC_BehaviorComponent::DetectThreat(AActor* ThreatActor, float ThreatLevel)
{
    if (ThreatActor && ThreatLevel > NPCMemory.ThreatLevel)
    {
        UpdateMemory(ThreatActor->GetActorLocation(), ThreatLevel);
        
        if (ThreatLevel > 0.7f)
        {
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
        }
        else if (ThreatLevel > 0.3f)
        {
            SetBehaviorState(ENPC_BehaviorState::Alert);
        }

        UE_LOG(LogTemp, Warning, TEXT("NPC %s detected threat %s with level %.2f"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *ThreatActor->GetName(), ThreatLevel);
    }
}

void UNPC_BehaviorComponent::UpdateMemory(FVector ThreatLocation, float ThreatLevel)
{
    NPCMemory.LastKnownThreatLocation = ThreatLocation;
    NPCMemory.ThreatLevel = FMath::Max(NPCMemory.ThreatLevel, ThreatLevel);
    NPCMemory.TimeSinceThreatSeen = 0.0f;
}

void UNPC_BehaviorComponent::AddTribeMember(AActor* NewMember)
{
    if (NewMember && !TribeMembers.Contains(NewMember))
    {
        TribeMembers.Add(NewMember);
        UE_LOG(LogTemp, Log, TEXT("Added %s to tribe. Total members: %d"), 
               *NewMember->GetName(), TribeMembers.Num());
    }
}

void UNPC_BehaviorComponent::BroadcastAlert(const FString& AlertMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("NPC ALERT: %s"), *AlertMessage);

    // Notify nearby tribe members
    for (AActor* Member : TribeMembers)
    {
        if (Member && Member != GetOwner())
        {
            if (UNPC_BehaviorComponent* MemberBehavior = Member->FindComponentByClass<UNPC_BehaviorComponent>())
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Member->GetActorLocation());
                if (Distance < AlertRadius)
                {
                    MemberBehavior->SetBehaviorState(ENPC_BehaviorState::Alert);
                    MemberBehavior->UpdateMemory(NPCMemory.LastKnownThreatLocation, NPCMemory.ThreatLevel * 0.8f);
                }
            }
        }
    }
}

void UNPC_BehaviorComponent::ProcessIdleBehavior(float DeltaTime)
{
    // Check if should start patrolling
    if (LastBehaviorUpdate > 5.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Patrolling);
    }
}

void UNPC_BehaviorComponent::ProcessPatrolBehavior(float DeltaTime)
{
    if (NPCMemory.PatrolPoints.Num() > 0 && GetOwner())
    {
        FVector CurrentLocation = GetOwner()->GetActorLocation();
        FVector TargetPoint = NPCMemory.PatrolPoints[CurrentPatrolIndex];
        
        float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPoint);
        
        if (DistanceToTarget < 100.0f)
        {
            // Reached patrol point, move to next
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % NPCMemory.PatrolPoints.Num();
        }
    }
}

void UNPC_BehaviorComponent::ProcessAlertBehavior(float DeltaTime)
{
    // Return to idle if no threat for a while
    if (NPCMemory.TimeSinceThreatSeen > 10.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}

void UNPC_BehaviorComponent::ProcessFleeingBehavior(float DeltaTime)
{
    // Return to alert state after fleeing for a while
    if (NPCMemory.TimeSinceThreatSeen > 8.0f)
    {
        SetBehaviorState(ENPC_BehaviorState::Alert);
    }
}

void UNPC_BehaviorComponent::UpdateBehaviorLogic()
{
    if (!GetOwner()) return;

    // Scan for threats in the area
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), FoundActors);

    float HighestThreat = 0.0f;
    AActor* ThreatActor = nullptr;

    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != GetOwner())
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
            
            // Check if this is a potential threat (player or predator)
            if (Distance < AlertRadius)
            {
                float ThreatLevel = 0.0f;
                
                // Calculate threat based on distance and actor type
                if (Actor->GetName().Contains(TEXT("Character")) || Actor->GetName().Contains(TEXT("Player")))
                {
                    ThreatLevel = 1.0f - (Distance / AlertRadius);
                }
                else if (Actor->GetName().Contains(TEXT("TRex")) || Actor->GetName().Contains(TEXT("Raptor")))
                {
                    ThreatLevel = 1.5f - (Distance / AlertRadius);
                }

                if (ThreatLevel > HighestThreat)
                {
                    HighestThreat = ThreatLevel;
                    ThreatActor = Actor;
                }
            }
        }
    }

    // React to highest threat
    if (ThreatActor && HighestThreat > 0.2f)
    {
        DetectThreat(ThreatActor, HighestThreat);
    }
}

void UNPC_BehaviorComponent::DecayMemory()
{
    // Gradually reduce threat level over time
    NPCMemory.ThreatLevel = FMath::Max(0.0f, NPCMemory.ThreatLevel - 0.1f);
    
    // Return to idle if threat level is very low
    if (NPCMemory.ThreatLevel < 0.1f && CurrentState == ENPC_BehaviorState::Alert)
    {
        SetBehaviorState(ENPC_BehaviorState::Idle);
    }
}
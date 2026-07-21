#include "NPCBehaviorSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UNPC_BehaviorSystem::UNPC_BehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentState = ENPC_BehaviorState::Idle;
    PersonalityType = ENPC_Personality::Cautious;
    AwarenessRadius = 1500.0f;
    FleeRadius = 800.0f;
    SocialRadius = 500.0f;

    MaxMemories = 20;
    MemoryDecayRate = 0.1f;

    CurrentPatrolIndex = 0;
    PatrolSpeed = 200.0f;
    bIsPatrolling = false;
    CurrentTarget = nullptr;
    LastBehaviorUpdate = 0.0f;
    LastMemoryProcess = 0.0f;
}

void UNPC_BehaviorSystem::BeginPlay()
{
    Super::BeginPlay();

    // Store home location
    if (GetOwner())
    {
        HomeLocation = GetOwner()->GetActorLocation();
    }

    // Set up default patrol points in a circle around home
    if (PatrolPoints.Num() == 0)
    {
        for (int32 i = 0; i < 4; i++)
        {
            float Angle = (i * 90.0f) * PI / 180.0f;
            FVector PatrolPoint = HomeLocation + FVector(FMath::Cos(Angle) * 800.0f, FMath::Sin(Angle) * 800.0f, 0.0f);
            PatrolPoints.Add(PatrolPoint);
        }
    }

    // Start behavior systems
    GetWorld()->GetTimerManager().SetTimer(BehaviorUpdateTimer, [this]() { UpdateBehavior(0.5f); }, 0.5f, true);
    GetWorld()->GetTimerManager().SetTimer(MemoryProcessTimer, [this]() { ProcessMemories(1.0f); }, 1.0f, true);
    GetWorld()->GetTimerManager().SetTimer(EnvironmentScanTimer, [this]() { ScanEnvironment(); }, 0.2f, true);

    UE_LOG(LogTemp, Log, TEXT("NPC Behavior System initialized for %s"), *GetOwner()->GetName());
}

void UNPC_BehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsPatrolling)
    {
        UpdatePatrol(DeltaTime);
    }
}

void UNPC_BehaviorSystem::SetBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_BehaviorState PreviousState = CurrentState;
        CurrentState = NewState;

        // Log state change
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed state from %d to %d"), 
               *GetOwner()->GetName(), (int32)PreviousState, (int32)NewState);

        // Handle state-specific initialization
        switch (NewState)
        {
        case ENPC_BehaviorState::Patrolling:
            StartPatrol();
            break;
        case ENPC_BehaviorState::Fleeing:
            StopPatrol();
            break;
        case ENPC_BehaviorState::Resting:
            StopPatrol();
            break;
        default:
            break;
        }
    }
}

void UNPC_BehaviorSystem::AddMemory(FVector Location, FString EventType, float Importance)
{
    FNPC_Memory NewMemory;
    NewMemory.LastSeenLocation = Location;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();

    Memories.Add(NewMemory);

    // Remove oldest memory if we exceed max
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }

    UE_LOG(LogTemp, Log, TEXT("NPC %s added memory: %s at %s"), 
           *GetOwner()->GetName(), *EventType, *Location.ToString());
}

void UNPC_BehaviorSystem::UpdateRelationship(AActor* Target, float AffinityChange, float TrustChange, float FearChange)
{
    if (!Target) return;

    // Find existing relationship or create new one
    FNPC_Relationship* ExistingRelationship = nullptr;
    for (FNPC_Relationship& Relationship : Relationships)
    {
        if (Relationship.TargetActor == Target)
        {
            ExistingRelationship = &Relationship;
            break;
        }
    }

    if (!ExistingRelationship)
    {
        FNPC_Relationship NewRelationship;
        NewRelationship.TargetActor = Target;
        Relationships.Add(NewRelationship);
        ExistingRelationship = &Relationships.Last();
    }

    // Update relationship values
    ExistingRelationship->Affinity = FMath::Clamp(ExistingRelationship->Affinity + AffinityChange, -100.0f, 100.0f);
    ExistingRelationship->Trust = FMath::Clamp(ExistingRelationship->Trust + TrustChange, -100.0f, 100.0f);
    ExistingRelationship->Fear = FMath::Clamp(ExistingRelationship->Fear + FearChange, 0.0f, 100.0f);
    ExistingRelationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
}

AActor* UNPC_BehaviorSystem::GetNearestThreat()
{
    AActor* NearestThreat = nullptr;
    float NearestDistance = AwarenessRadius;

    if (!GetOwner()) return nullptr;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Check for dinosaurs
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != GetOwner())
        {
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains("trex") || ActorName.Contains("raptor") || ActorName.Contains("carno"))
            {
                float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
                if (Distance < NearestDistance)
                {
                    NearestDistance = Distance;
                    NearestThreat = Actor;
                }
            }
        }
    }

    return NearestThreat;
}

AActor* UNPC_BehaviorSystem::GetNearestFriend()
{
    AActor* NearestFriend = nullptr;
    float NearestDistance = SocialRadius;

    if (!GetOwner()) return nullptr;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Check relationships for friendly NPCs
    for (const FNPC_Relationship& Relationship : Relationships)
    {
        if (Relationship.TargetActor && Relationship.Affinity > 20.0f)
        {
            float Distance = FVector::Dist(OwnerLocation, Relationship.TargetActor->GetActorLocation());
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestFriend = Relationship.TargetActor;
            }
        }
    }

    return NearestFriend;
}

void UNPC_BehaviorSystem::StartPatrol()
{
    if (PatrolPoints.Num() > 0)
    {
        bIsPatrolling = true;
        CurrentPatrolIndex = 0;
        UE_LOG(LogTemp, Log, TEXT("NPC %s started patrolling"), *GetOwner()->GetName());
    }
}

void UNPC_BehaviorSystem::StopPatrol()
{
    bIsPatrolling = false;
    UE_LOG(LogTemp, Log, TEXT("NPC %s stopped patrolling"), *GetOwner()->GetName());
}

void UNPC_BehaviorSystem::UpdateBehavior(float DeltaTime)
{
    if (!GetOwner()) return;

    LastBehaviorUpdate = GetWorld()->GetTimeSeconds();

    // Check for immediate threats
    AActor* Threat = GetNearestThreat();
    if (Threat)
    {
        float ThreatDistance = FVector::Dist(GetOwner()->GetActorLocation(), Threat->GetActorLocation());
        
        if (ThreatDistance < FleeRadius)
        {
            SetBehaviorState(ENPC_BehaviorState::Fleeing);
            AddMemory(Threat->GetActorLocation(), TEXT("Threat Encountered"), 8.0f);
            return;
        }
        else if (ThreatDistance < AwarenessRadius)
        {
            SetBehaviorState(ENPC_BehaviorState::Investigating);
            AddMemory(Threat->GetActorLocation(), TEXT("Threat Spotted"), 5.0f);
            return;
        }
    }

    // Check for player interaction
    ReactToPlayer();

    // Default behavior based on personality
    if (CurrentState == ENPC_BehaviorState::Idle)
    {
        switch (PersonalityType)
        {
        case ENPC_Personality::Curious:
            if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
            {
                SetBehaviorState(ENPC_BehaviorState::Investigating);
            }
            break;
        case ENPC_Personality::Social:
            {
                AActor* Friend = GetNearestFriend();
                if (Friend)
                {
                    SetBehaviorState(ENPC_BehaviorState::Socializing);
                }
                else
                {
                    SetBehaviorState(ENPC_BehaviorState::Patrolling);
                }
            }
            break;
        default:
            SetBehaviorState(ENPC_BehaviorState::Patrolling);
            break;
        }
    }
}

void UNPC_BehaviorSystem::ProcessMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    // Decay memories over time
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        float MemoryAge = CurrentTime - Memories[i].Timestamp;
        Memories[i].Importance -= MemoryDecayRate * MemoryAge;

        // Remove memories that have decayed too much
        if (Memories[i].Importance <= 0.0f)
        {
            Memories.RemoveAt(i);
        }
    }

    LastMemoryProcess = CurrentTime;
}

void UNPC_BehaviorSystem::UpdatePatrol(float DeltaTime)
{
    if (!GetOwner() || PatrolPoints.Num() == 0) return;

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetPoint = PatrolPoints[CurrentPatrolIndex];

    // Move towards current patrol point
    FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + (Direction * PatrolSpeed * DeltaTime);

    // Check if we've reached the patrol point
    if (FVector::Dist(CurrentLocation, TargetPoint) < 100.0f)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
        AddMemory(TargetPoint, TEXT("Patrol Point Reached"), 2.0f);
    }

    // Move the actor (simplified - in real implementation would use movement component)
    GetOwner()->SetActorLocation(NewLocation);
}

void UNPC_BehaviorSystem::ScanEnvironment()
{
    if (!GetOwner()) return;

    // This would typically use line traces or overlap checks
    // For now, just log that we're scanning
    if (CurrentState == ENPC_BehaviorState::Investigating)
    {
        UE_LOG(LogTemp, VeryVerbose, TEXT("NPC %s scanning environment"), *GetOwner()->GetName());
    }
}

void UNPC_BehaviorSystem::ReactToPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn || !GetOwner()) return;

    float PlayerDistance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());

    if (PlayerDistance < AwarenessRadius)
    {
        // Update relationship with player based on personality
        float AffinityChange = 0.0f;
        float FearChange = 0.0f;

        switch (PersonalityType)
        {
        case ENPC_Personality::Social:
            AffinityChange = 1.0f;
            break;
        case ENPC_Personality::Cautious:
            FearChange = 2.0f;
            break;
        case ENPC_Personality::Aggressive:
            AffinityChange = -1.0f;
            break;
        default:
            break;
        }

        UpdateRelationship(PlayerPawn, AffinityChange, 0.0f, FearChange);
        AddMemory(PlayerPawn->GetActorLocation(), TEXT("Player Spotted"), 4.0f);
    }
}

void UNPC_BehaviorSystem::ReactToDinosaurs()
{
    // This would handle specific reactions to different dinosaur types
    // Implementation would depend on dinosaur AI system
}
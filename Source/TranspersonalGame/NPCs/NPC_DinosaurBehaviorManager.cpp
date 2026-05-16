#include "NPC_DinosaurBehaviorManager.h"
#include "DinosaurBase.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AI/NavigationSystemBase.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorManager::UNPC_DinosaurBehaviorManager()
{
}

void UNPC_DinosaurBehaviorManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("NPC Dinosaur Behavior Manager initialized"));
    
    // Start behavior update timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            BehaviorUpdateTimer,
            this,
            &UNPC_DinosaurBehaviorManager::UpdateAllDinosaurs,
            0.5f,
            true
        );
    }
}

void UNPC_DinosaurBehaviorManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(BehaviorUpdateTimer);
    }
    
    DinosaurStates.Empty();
    DinosaurBehaviorData.Empty();
    RegisteredDinosaurs.Empty();
    
    Super::Deinitialize();
}

void UNPC_DinosaurBehaviorManager::RegisterDinosaur(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || RegisteredDinosaurs.Contains(Dinosaur))
    {
        return;
    }
    
    RegisteredDinosaurs.Add(Dinosaur);
    
    // Initialize default state
    FNPC_DinosaurState DefaultState;
    DinosaurStates.Add(Dinosaur, DefaultState);
    
    // Initialize default behavior data
    FNPC_DinosaurBehaviorData DefaultBehavior;
    DinosaurBehaviorData.Add(Dinosaur, DefaultBehavior);
    
    UE_LOG(LogTemp, Warning, TEXT("Registered dinosaur: %s"), *Dinosaur->GetName());
}

void UNPC_DinosaurBehaviorManager::UnregisterDinosaur(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }
    
    RegisteredDinosaurs.Remove(Dinosaur);
    DinosaurStates.Remove(Dinosaur);
    DinosaurBehaviorData.Remove(Dinosaur);
    
    UE_LOG(LogTemp, Warning, TEXT("Unregistered dinosaur: %s"), *Dinosaur->GetName());
}

void UNPC_DinosaurBehaviorManager::UpdateDinosaurBehavior(ADinosaurBase* Dinosaur, float DeltaTime)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    ProcessDinosaurAI(Dinosaur);
    UpdateDinosaurStats(Dinosaur, DeltaTime);
}

void UNPC_DinosaurBehaviorManager::SetDinosaurState(ADinosaurBase* Dinosaur, const FNPC_DinosaurState& NewState)
{
    if (!Dinosaur)
    {
        return;
    }
    
    DinosaurStates.Add(Dinosaur, NewState);
}

FNPC_DinosaurState UNPC_DinosaurBehaviorManager::GetDinosaurState(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return FNPC_DinosaurState();
    }
    
    return DinosaurStates[Dinosaur];
}

void UNPC_DinosaurBehaviorManager::NotifyPlayerSeen(ADinosaurBase* Dinosaur, const FVector& PlayerLocation)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    State.LastKnownPlayerLocation = PlayerLocation;
    State.TimeSincePlayerSeen = 0.0f;
    State.bIsHunting = true;
    State.bIsPatrolling = false;
    State.Aggression = FMath::Min(State.Aggression + 10.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s spotted player at %s"), 
           *Dinosaur->GetName(), *PlayerLocation.ToString());
}

void UNPC_DinosaurBehaviorManager::NotifyPlayerHeard(ADinosaurBase* Dinosaur, const FVector& SoundLocation)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    State.LastKnownPlayerLocation = SoundLocation;
    State.Aggression = FMath::Min(State.Aggression + 5.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur %s heard player at %s"), 
           *Dinosaur->GetName(), *SoundLocation.ToString());
}

TArray<ADinosaurBase*> UNPC_DinosaurBehaviorManager::GetNearbyDinosaurs(ADinosaurBase* Dinosaur, float Radius)
{
    TArray<ADinosaurBase*> NearbyDinosaurs;
    
    if (!Dinosaur)
    {
        return NearbyDinosaurs;
    }
    
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    for (ADinosaurBase* OtherDinosaur : RegisteredDinosaurs)
    {
        if (OtherDinosaur && OtherDinosaur != Dinosaur)
        {
            float Distance = FVector::Dist(DinosaurLocation, OtherDinosaur->GetActorLocation());
            if (Distance <= Radius)
            {
                NearbyDinosaurs.Add(OtherDinosaur);
            }
        }
    }
    
    return NearbyDinosaurs;
}

void UNPC_DinosaurBehaviorManager::CoordinatePackBehavior(const TArray<ADinosaurBase*>& PackMembers)
{
    if (PackMembers.Num() < 2)
    {
        return;
    }
    
    // Find pack leader (highest aggression)
    ADinosaurBase* PackLeader = nullptr;
    float HighestAggression = 0.0f;
    
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && DinosaurStates.Contains(Member))
        {
            float Aggression = DinosaurStates[Member].Aggression;
            if (Aggression > HighestAggression)
            {
                HighestAggression = Aggression;
                PackLeader = Member;
            }
        }
    }
    
    if (!PackLeader)
    {
        return;
    }
    
    // Coordinate pack behavior based on leader's state
    FNPC_DinosaurState& LeaderState = DinosaurStates[PackLeader];
    
    for (ADinosaurBase* Member : PackMembers)
    {
        if (Member && Member != PackLeader && DinosaurStates.Contains(Member))
        {
            FNPC_DinosaurState& MemberState = DinosaurStates[Member];
            
            // Follow leader's hunting state
            if (LeaderState.bIsHunting)
            {
                MemberState.bIsHunting = true;
                MemberState.bIsPatrolling = false;
                MemberState.LastKnownPlayerLocation = LeaderState.LastKnownPlayerLocation;
            }
        }
    }
}

void UNPC_DinosaurBehaviorManager::StartBehaviorTree(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }
    
    // Implementation would start behavior tree execution
    UE_LOG(LogTemp, Warning, TEXT("Starting behavior tree for dinosaur: %s"), *Dinosaur->GetName());
}

void UNPC_DinosaurBehaviorManager::StopBehaviorTree(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur)
    {
        return;
    }
    
    // Implementation would stop behavior tree execution
    UE_LOG(LogTemp, Warning, TEXT("Stopping behavior tree for dinosaur: %s"), *Dinosaur->GetName());
}

void UNPC_DinosaurBehaviorManager::UpdateAllDinosaurs()
{
    for (ADinosaurBase* Dinosaur : RegisteredDinosaurs)
    {
        if (Dinosaur && IsValid(Dinosaur))
        {
            UpdateDinosaurBehavior(Dinosaur, 0.5f);
        }
    }
}

void UNPC_DinosaurBehaviorManager::ProcessDinosaurAI(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    
    // Check for player
    if (UWorld* World = GetWorld())
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0);
        if (Player)
        {
            FVector PlayerLocation = Player->GetActorLocation();
            FVector DinosaurLocation = Dinosaur->GetActorLocation();
            float DistanceToPlayer = FVector::Dist(DinosaurLocation, PlayerLocation);
            
            FNPC_DinosaurBehaviorData& BehaviorData = DinosaurBehaviorData[Dinosaur];
            
            // Check if player is in sight range
            if (DistanceToPlayer <= BehaviorData.SightRange && CanSeePlayer(Dinosaur, PlayerLocation))
            {
                NotifyPlayerSeen(Dinosaur, PlayerLocation);
            }
        }
    }
    
    // Update behavior based on current state
    if (State.bIsHunting)
    {
        HandleHuntingBehavior(Dinosaur);
    }
    else if (State.bIsResting)
    {
        HandleRestingBehavior(Dinosaur);
    }
    else if (State.bIsPatrolling)
    {
        HandlePatrolBehavior(Dinosaur);
    }
}

void UNPC_DinosaurBehaviorManager::HandleHuntingBehavior(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    FNPC_DinosaurBehaviorData& BehaviorData = DinosaurBehaviorData[Dinosaur];
    
    // Move towards last known player location
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    FVector TargetLocation = State.LastKnownPlayerLocation;
    float DistanceToTarget = FVector::Dist(DinosaurLocation, TargetLocation);
    
    if (DistanceToTarget > BehaviorData.AttackDistance)
    {
        // Move towards target
        FVector Direction = (TargetLocation - DinosaurLocation).GetSafeNormal();
        FVector NewLocation = DinosaurLocation + (Direction * BehaviorData.ChaseSpeed * 0.5f);
        Dinosaur->SetActorLocation(NewLocation);
    }
    
    // Check if should stop hunting
    State.TimeSincePlayerSeen += 0.5f;
    if (State.TimeSincePlayerSeen > 10.0f)
    {
        State.bIsHunting = false;
        State.bIsPatrolling = true;
        State.Aggression = FMath::Max(State.Aggression - 5.0f, 0.0f);
    }
}

void UNPC_DinosaurBehaviorManager::HandlePatrolBehavior(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    FNPC_DinosaurBehaviorData& BehaviorData = DinosaurBehaviorData[Dinosaur];
    
    // Simple patrol logic - move to random points within patrol radius
    FVector PatrolPoint = GetRandomPatrolPoint(Dinosaur);
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    FVector Direction = (PatrolPoint - DinosaurLocation).GetSafeNormal();
    FVector NewLocation = DinosaurLocation + (Direction * BehaviorData.MovementSpeed * 0.5f);
    
    Dinosaur->SetActorLocation(NewLocation);
}

void UNPC_DinosaurBehaviorManager::HandleRestingBehavior(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    
    // Restore stamina while resting
    State.Stamina = FMath::Min(State.Stamina + 10.0f, 100.0f);
    
    // Check if should stop resting
    if (State.Stamina >= 80.0f)
    {
        State.bIsResting = false;
        State.bIsPatrolling = true;
    }
}

void UNPC_DinosaurBehaviorManager::UpdateDinosaurStats(ADinosaurBase* Dinosaur, float DeltaTime)
{
    if (!Dinosaur || !DinosaurStates.Contains(Dinosaur))
    {
        return;
    }
    
    FNPC_DinosaurState& State = DinosaurStates[Dinosaur];
    
    // Update hunger and thirst over time
    State.Hunger = FMath::Max(State.Hunger - (DeltaTime * 0.5f), 0.0f);
    State.Thirst = FMath::Max(State.Thirst - (DeltaTime * 0.7f), 0.0f);
    
    // Update stamina based on activity
    if (State.bIsHunting)
    {
        State.Stamina = FMath::Max(State.Stamina - (DeltaTime * 2.0f), 0.0f);
    }
    else if (!State.bIsResting)
    {
        State.Stamina = FMath::Max(State.Stamina - (DeltaTime * 0.5f), 0.0f);
    }
    
    // Check if should start resting
    if (State.Stamina <= 20.0f && !State.bIsResting)
    {
        State.bIsResting = true;
        State.bIsHunting = false;
        State.bIsPatrolling = false;
    }
    
    // Reduce aggression over time
    State.Aggression = FMath::Max(State.Aggression - (DeltaTime * 0.5f), 0.0f);
}

bool UNPC_DinosaurBehaviorManager::CanSeePlayer(ADinosaurBase* Dinosaur, const FVector& PlayerLocation)
{
    if (!Dinosaur)
    {
        return false;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    FVector DinosaurEyeLocation = DinosaurLocation + FVector(0, 0, 150); // Approximate eye height
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Dinosaur);
    
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        DinosaurEyeLocation,
        PlayerLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstacle hit
}

bool UNPC_DinosaurBehaviorManager::IsPlayerInRange(ADinosaurBase* Dinosaur, const FVector& PlayerLocation, float Range)
{
    if (!Dinosaur)
    {
        return false;
    }
    
    float Distance = FVector::Dist(Dinosaur->GetActorLocation(), PlayerLocation);
    return Distance <= Range;
}

FVector UNPC_DinosaurBehaviorManager::GetRandomPatrolPoint(ADinosaurBase* Dinosaur)
{
    if (!Dinosaur || !DinosaurBehaviorData.Contains(Dinosaur))
    {
        return FVector::ZeroVector;
    }
    
    FNPC_DinosaurBehaviorData& BehaviorData = DinosaurBehaviorData[Dinosaur];
    FVector DinosaurLocation = Dinosaur->GetActorLocation();
    
    // Generate random point within patrol radius
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, BehaviorData.PatrolRadius);
    
    FVector RandomOffset = FVector(
        FMath::Cos(Angle) * Distance,
        FMath::Sin(Angle) * Distance,
        0.0f
    );
    
    return DinosaurLocation + RandomOffset;
}
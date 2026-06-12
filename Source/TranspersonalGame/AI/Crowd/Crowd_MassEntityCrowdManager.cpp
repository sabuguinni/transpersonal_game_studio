#include "Crowd_MassEntityCrowdManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "MassEntitySubsystem.h"
#include "MassSpawningSubsystem.h"
#include "MassMovementFragments.h"
#include "MassCommonFragments.h"
#include "MassEntityTemplateRegistry.h"

UCrowd_MassEntityCrowdManager::UCrowd_MassEntityCrowdManager()
    : MassEntitySubsystem(nullptr)
    , MassSpawningSubsystem(nullptr)
    , CrowdDensity(1.0f)
    , UpdateFrequency(0.1f)
    , MaxCrowdMembers(1000)
    , AvoidanceRadius(100.0f)
    , GlobalBehaviorMode(0)
    , LastUpdateTime(0.0f)
    , bIsInitialized(false)
{
}

void UCrowd_MassEntityCrowdManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UCrowd_MassEntityCrowdManager::Initialize"));
    
    InitializeCrowdSystem();
}

void UCrowd_MassEntityCrowdManager::Deinitialize()
{
    if (MassEntitySubsystem)
    {
        // Clean up Mass Entity resources
        MassEntitySubsystem = nullptr;
    }
    
    if (MassSpawningSubsystem)
    {
        MassSpawningSubsystem = nullptr;
    }
    
    CrowdMembers.Empty();
    CrowdZones.Empty();
    bIsInitialized = false;
    
    Super::Deinitialize();
}

bool UCrowd_MassEntityCrowdManager::ShouldCreateSubsystem(UObject* Outer) const
{
    return Super::ShouldCreateSubsystem(Outer);
}

void UCrowd_MassEntityCrowdManager::InitializeCrowdSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UCrowd_MassEntityCrowdManager: No valid world"));
        return;
    }
    
    // Initialize Mass Entity subsystems
    InitializeMassEntitySystem();
    
    // Create default crowd zones
    FCrowd_CrowdZone GatheringZone;
    GatheringZone.CenterLocation = FVector(1500.0f, 1500.0f, 100.0f);
    GatheringZone.Radius = 400.0f;
    GatheringZone.MaxCapacity = 30;
    GatheringZone.ZoneName = TEXT("GatheringZone");
    CrowdZones.Add(GatheringZone);
    
    FCrowd_CrowdZone MarketZone;
    MarketZone.CenterLocation = FVector(2500.0f, 2500.0f, 100.0f);
    MarketZone.Radius = 600.0f;
    MarketZone.MaxCapacity = 50;
    MarketZone.ZoneName = TEXT("MarketZone");
    CrowdZones.Add(MarketZone);
    
    FCrowd_CrowdZone RestZone;
    RestZone.CenterLocation = FVector(1000.0f, 2000.0f, 100.0f);
    RestZone.Radius = 300.0f;
    RestZone.MaxCapacity = 20;
    RestZone.ZoneName = TEXT("RestZone");
    CrowdZones.Add(RestZone);
    
    // Create Mass Entity archetype for crowd
    CreateCrowdArchetype();
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd system initialized with %d zones"), CrowdZones.Num());
}

void UCrowd_MassEntityCrowdManager::InitializeMassEntitySystem()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get Mass Entity subsystem
    MassEntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Entity subsystem not available, using fallback"));
        return;
    }
    
    // Get Mass Spawning subsystem
    MassSpawningSubsystem = World->GetSubsystem<UMassSpawningSubsystem>();
    if (!MassSpawningSubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mass Spawning subsystem not available"));
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Mass Entity subsystems initialized"));
}

void UCrowd_MassEntityCrowdManager::CreateCrowdArchetype()
{
    if (!MassEntitySubsystem)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create archetype: Mass Entity subsystem not available"));
        return;
    }
    
    // Create archetype with basic fragments for crowd simulation
    // This would normally use Mass Entity's archetype system
    // For now, we'll use a simplified approach
    
    UE_LOG(LogTemp, Warning, TEXT("Crowd archetype creation - using simplified system"));
}

void UCrowd_MassEntityCrowdManager::SpawnCrowdMembers(int32 Count, FVector SpawnCenter, float SpawnRadius)
{
    if (!bIsInitialized)
    {
        InitializeCrowdSystem();
    }
    
    int32 ActualCount = FMath::Min(Count, MaxCrowdMembers - CrowdMembers.Num());
    
    for (int32 i = 0; i < ActualCount; i++)
    {
        FCrowd_CrowdMemberData NewMember;
        
        // Random spawn location within radius
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, SpawnRadius);
        
        NewMember.CurrentLocation = SpawnCenter + FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            0.0f
        );
        
        // Assign random target from crowd zones
        if (CrowdZones.Num() > 0)
        {
            int32 RandomZoneIndex = FMath::RandRange(0, CrowdZones.Num() - 1);
            NewMember.TargetLocation = CrowdZones[RandomZoneIndex].CenterLocation;
        }
        else
        {
            NewMember.TargetLocation = NewMember.CurrentLocation;
        }
        
        NewMember.MovementSpeed = FMath::RandRange(100.0f, 200.0f);
        NewMember.BehaviorState = FMath::RandRange(0, 2); // 0=Wander, 1=GoToZone, 2=Rest
        NewMember.StateTimer = 0.0f;
        
        CrowdMembers.Add(NewMember);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d crowd members (total: %d)"), ActualCount, CrowdMembers.Num());
}

void UCrowd_MassEntityCrowdManager::UpdateCrowdBehavior(float DeltaTime)
{
    if (!bIsInitialized || CrowdMembers.Num() == 0)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime < UpdateFrequency)
    {
        return;
    }
    
    LastUpdateTime = 0.0f;
    
    UpdateCrowdMovement(DeltaTime);
    ProcessCrowdZones();
    HandleCrowdCollisions();
    
    if (MassEntitySubsystem)
    {
        ProcessCrowdEntities();
    }
}

void UCrowd_MassEntityCrowdManager::UpdateCrowdMovement(float DeltaTime)
{
    for (FCrowd_CrowdMemberData& Member : CrowdMembers)
    {
        Member.StateTimer += DeltaTime;
        
        // Update behavior state
        switch (Member.BehaviorState)
        {
            case 0: // Wander
                if (Member.StateTimer > 3.0f)
                {
                    // Find new random target
                    FVector RandomOffset = FVector(
                        FMath::RandRange(-500.0f, 500.0f),
                        FMath::RandRange(-500.0f, 500.0f),
                        0.0f
                    );
                    Member.TargetLocation = Member.CurrentLocation + RandomOffset;
                    Member.StateTimer = 0.0f;
                }
                break;
                
            case 1: // Go to zone
                if (FVector::Dist(Member.CurrentLocation, Member.TargetLocation) < 50.0f)
                {
                    Member.BehaviorState = 2; // Switch to rest
                    Member.StateTimer = 0.0f;
                }
                break;
                
            case 2: // Rest
                if (Member.StateTimer > 5.0f)
                {
                    Member.BehaviorState = 0; // Switch to wander
                    Member.StateTimer = 0.0f;
                }
                break;
        }
        
        // Move towards target
        if (Member.BehaviorState != 2) // Not resting
        {
            FVector Direction = (Member.TargetLocation - Member.CurrentLocation).GetSafeNormal();
            float MoveDistance = Member.MovementSpeed * DeltaTime;
            Member.CurrentLocation += Direction * MoveDistance;
        }
    }
}

void UCrowd_MassEntityCrowdManager::ProcessCrowdZones()
{
    // Reset zone occupancy
    for (FCrowd_CrowdZone& Zone : CrowdZones)
    {
        Zone.CurrentOccupancy = 0;
    }
    
    // Count members in each zone
    for (const FCrowd_CrowdMemberData& Member : CrowdMembers)
    {
        for (FCrowd_CrowdZone& Zone : CrowdZones)
        {
            if (IsLocationInCrowdZone(Member.CurrentLocation, Zone))
            {
                Zone.CurrentOccupancy++;
            }
        }
    }
}

void UCrowd_MassEntityCrowdManager::HandleCrowdCollisions()
{
    // Simple avoidance between crowd members
    for (int32 i = 0; i < CrowdMembers.Num(); i++)
    {
        for (int32 j = i + 1; j < CrowdMembers.Num(); j++)
        {
            FCrowd_CrowdMemberData& MemberA = CrowdMembers[i];
            FCrowd_CrowdMemberData& MemberB = CrowdMembers[j];
            
            float Distance = FVector::Dist(MemberA.CurrentLocation, MemberB.CurrentLocation);
            if (Distance < AvoidanceRadius && Distance > 0.0f)
            {
                // Push apart
                FVector Direction = (MemberA.CurrentLocation - MemberB.CurrentLocation).GetSafeNormal();
                float PushForce = (AvoidanceRadius - Distance) * 0.5f;
                
                MemberA.CurrentLocation += Direction * PushForce;
                MemberB.CurrentLocation -= Direction * PushForce;
            }
        }
    }
}

void UCrowd_MassEntityCrowdManager::SetCrowdDensity(float NewDensity)
{
    CrowdDensity = FMath::Clamp(NewDensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd density set to %f"), CrowdDensity);
}

void UCrowd_MassEntityCrowdManager::AddCrowdZone(const FCrowd_CrowdZone& NewZone)
{
    CrowdZones.Add(NewZone);
    UE_LOG(LogTemp, Warning, TEXT("Added crowd zone: %s"), *NewZone.ZoneName);
}

void UCrowd_MassEntityCrowdManager::RemoveCrowdZone(const FString& ZoneName)
{
    CrowdZones.RemoveAll([&ZoneName](const FCrowd_CrowdZone& Zone) {
        return Zone.ZoneName == ZoneName;
    });
    UE_LOG(LogTemp, Warning, TEXT("Removed crowd zone: %s"), *ZoneName);
}

void UCrowd_MassEntityCrowdManager::CreateMassEntityArchetype()
{
    CreateCrowdArchetype();
}

void UCrowd_MassEntityCrowdManager::ProcessCrowdEntities()
{
    if (!MassEntitySubsystem)
    {
        return;
    }
    
    // Process Mass Entity crowd simulation
    // This would integrate with Mass Entity's processing system
    UE_LOG(LogTemp, VeryVerbose, TEXT("Processing %d crowd entities"), CrowdMembers.Num());
}

int32 UCrowd_MassEntityCrowdManager::GetActiveCrowdMemberCount() const
{
    return CrowdMembers.Num();
}

void UCrowd_MassEntityCrowdManager::SetGlobalCrowdBehavior(int32 BehaviorMode)
{
    GlobalBehaviorMode = BehaviorMode;
    
    // Update all crowd members to new behavior
    for (FCrowd_CrowdMemberData& Member : CrowdMembers)
    {
        Member.BehaviorState = BehaviorMode;
        Member.StateTimer = 0.0f;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Global crowd behavior set to %d"), BehaviorMode);
}

void UCrowd_MassEntityCrowdManager::TriggerCrowdEvent(const FString& EventName, FVector EventLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd event triggered: %s at %s"), *EventName, *EventLocation.ToString());
    
    // React to events (panic, gathering, etc.)
    if (EventName == TEXT("Panic"))
    {
        for (FCrowd_CrowdMemberData& Member : CrowdMembers)
        {
            // Run away from event location
            FVector AwayDirection = (Member.CurrentLocation - EventLocation).GetSafeNormal();
            Member.TargetLocation = Member.CurrentLocation + AwayDirection * 1000.0f;
            Member.BehaviorState = 1; // Go to target
            Member.MovementSpeed = 300.0f; // Faster movement
        }
    }
    else if (EventName == TEXT("Gathering"))
    {
        for (FCrowd_CrowdMemberData& Member : CrowdMembers)
        {
            // Move towards event location
            Member.TargetLocation = EventLocation;
            Member.BehaviorState = 1; // Go to target
        }
    }
}

void UCrowd_MassEntityCrowdManager::SetCrowdAvoidanceRadius(float NewRadius)
{
    AvoidanceRadius = FMath::Max(NewRadius, 10.0f);
    UE_LOG(LogTemp, Warning, TEXT("Crowd avoidance radius set to %f"), AvoidanceRadius);
}

FVector UCrowd_MassEntityCrowdManager::FindNearestCrowdZone(const FVector& Location) const
{
    if (CrowdZones.Num() == 0)
    {
        return Location;
    }
    
    float MinDistance = FLT_MAX;
    FVector NearestZoneLocation = Location;
    
    for (const FCrowd_CrowdZone& Zone : CrowdZones)
    {
        float Distance = FVector::Dist(Location, Zone.CenterLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestZoneLocation = Zone.CenterLocation;
        }
    }
    
    return NearestZoneLocation;
}

bool UCrowd_MassEntityCrowdManager::IsLocationInCrowdZone(const FVector& Location, const FCrowd_CrowdZone& Zone) const
{
    float Distance = FVector::Dist(Location, Zone.CenterLocation);
    return Distance <= Zone.Radius;
}
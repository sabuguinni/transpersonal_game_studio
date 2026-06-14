#include "Crowd_BehaviorManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"

ACrowd_BehaviorManager::ACrowd_BehaviorManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    BehaviorUpdateInterval = 0.5f;
    AlertRadius = 2000.0f;
    FleeDistance = 1500.0f;
    LastBehaviorUpdate = 0.0f;
    
    // Initialize default behavior zones
    FCrowd_BehaviorZone GatheringZone;
    GatheringZone.ZoneName = TEXT("GatheringZone");
    GatheringZone.DefaultBehavior = ECrowd_BehaviorState::Gathering;
    GatheringZone.ZoneCenter = FVector(-2000, -2000, 100);
    GatheringZone.ZoneRadius = 1000.0f;
    GatheringZone.MaxOccupants = 30;
    BehaviorZones.Add(GatheringZone);
    
    FCrowd_BehaviorZone RestingZone;
    RestingZone.ZoneName = TEXT("RestingZone");
    RestingZone.DefaultBehavior = ECrowd_BehaviorState::Resting;
    RestingZone.ZoneCenter = FVector(2000, 2000, 100);
    RestingZone.ZoneRadius = 800.0f;
    RestingZone.MaxOccupants = 20;
    BehaviorZones.Add(RestingZone);
    
    FCrowd_BehaviorZone AlertZone;
    AlertZone.ZoneName = TEXT("AlertZone");
    AlertZone.DefaultBehavior = ECrowd_BehaviorState::Alert;
    AlertZone.ZoneCenter = FVector(0, -3000, 100);
    AlertZone.ZoneRadius = 1200.0f;
    AlertZone.MaxOccupants = 50;
    BehaviorZones.Add(AlertZone);
}

void ACrowd_BehaviorManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all crowd entities in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("CrowdEntity")))
        {
            TrackedActors.Add(Actor);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdBehaviorManager: Tracking %d crowd entities"), TrackedActors.Num());
}

void ACrowd_BehaviorManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastBehaviorUpdate += DeltaTime;
    if (LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        UpdateBehaviorStates();
        ProcessZoneOccupancy();
        LastBehaviorUpdate = 0.0f;
    }
}

ECrowd_BehaviorState ACrowd_BehaviorManager::GetBehaviorForLocation(const FVector& Location)
{
    for (const FCrowd_BehaviorZone& Zone : BehaviorZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius)
        {
            return Zone.DefaultBehavior;
        }
    }
    
    return ECrowd_BehaviorState::Wandering;
}

void ACrowd_BehaviorManager::TriggerGlobalAlert(const FVector& ThreatLocation)
{
    for (AActor* Actor : TrackedActors)
    {
        if (!Actor) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), ThreatLocation);
        if (Distance <= AlertRadius)
        {
            // Set actor to flee behavior
            FVector FleeDirection = (Actor->GetActorLocation() - ThreatLocation).GetSafeNormal();
            FVector FleeTarget = Actor->GetActorLocation() + FleeDirection * FleeDistance;
            
            // Apply flee behavior (would integrate with movement component)
            UE_LOG(LogTemp, Warning, TEXT("Actor %s fleeing from threat"), *Actor->GetName());
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Global alert triggered at %s"), *ThreatLocation.ToString());
}

void ACrowd_BehaviorManager::AddBehaviorZone(const FCrowd_BehaviorZone& NewZone)
{
    BehaviorZones.Add(NewZone);
    UE_LOG(LogTemp, Warning, TEXT("Added behavior zone: %s"), *NewZone.ZoneName);
}

void ACrowd_BehaviorManager::RemoveBehaviorZone(const FString& ZoneName)
{
    BehaviorZones.RemoveAll([ZoneName](const FCrowd_BehaviorZone& Zone)
    {
        return Zone.ZoneName == ZoneName;
    });
    UE_LOG(LogTemp, Warning, TEXT("Removed behavior zone: %s"), *ZoneName);
}

TArray<AActor*> ACrowd_BehaviorManager::GetActorsInZone(const FString& ZoneName)
{
    TArray<AActor*> ActorsInZone;
    FCrowd_BehaviorZone* Zone = FindZoneByName(ZoneName);
    
    if (!Zone) return ActorsInZone;
    
    for (AActor* Actor : TrackedActors)
    {
        if (!Actor) continue;
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), Zone->ZoneCenter);
        if (Distance <= Zone->ZoneRadius)
        {
            ActorsInZone.Add(Actor);
        }
    }
    
    return ActorsInZone;
}

void ACrowd_BehaviorManager::UpdateBehaviorStates()
{
    for (AActor* Actor : TrackedActors)
    {
        if (!Actor) continue;
        
        ECrowd_BehaviorState NewBehavior = GetBehaviorForLocation(Actor->GetActorLocation());
        
        // Apply behavior-specific logic
        switch (NewBehavior)
        {
            case ECrowd_BehaviorState::Gathering:
                // Move towards zone center
                break;
            case ECrowd_BehaviorState::Resting:
                // Reduce movement speed
                break;
            case ECrowd_BehaviorState::Alert:
                // Increase awareness radius
                break;
            case ECrowd_BehaviorState::Wandering:
            default:
                // Random movement
                break;
        }
    }
}

void ACrowd_BehaviorManager::ProcessZoneOccupancy()
{
    for (FCrowd_BehaviorZone& Zone : BehaviorZones)
    {
        TArray<AActor*> ActorsInZone = GetActorsInZone(Zone.ZoneName);
        
        if (ActorsInZone.Num() > Zone.MaxOccupants)
        {
            // Zone overcrowded - trigger dispersal
            UE_LOG(LogTemp, Warning, TEXT("Zone %s overcrowded: %d/%d"), 
                   *Zone.ZoneName, ActorsInZone.Num(), Zone.MaxOccupants);
        }
    }
}

FCrowd_BehaviorZone* ACrowd_BehaviorManager::FindZoneByName(const FString& ZoneName)
{
    for (FCrowd_BehaviorZone& Zone : BehaviorZones)
    {
        if (Zone.ZoneName == ZoneName)
        {
            return &Zone;
        }
    }
    return nullptr;
}
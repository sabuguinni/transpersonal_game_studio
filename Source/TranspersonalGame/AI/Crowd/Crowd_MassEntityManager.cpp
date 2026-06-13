#include "Crowd_MassEntityManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/TargetPoint.h"
#include "GameFramework/Character.h"
#include "DrawDebugHelpers.h"

ACrowd_MassEntityManager::ACrowd_MassEntityManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update every 100ms

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    
    // Create detection sphere
    DetectionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionSphere"));
    DetectionSphere->SetupAttachment(RootComponent);
    DetectionSphere->SetSphereRadius(EntityConfig.SpawnRadius);

    // Set default values
    EntityConfig.MaxEntities = 500;
    EntityConfig.SpawnRadius = 1500.0f;
    EntityConfig.MovementSpeed = 150.0f;
    EntityConfig.LODDistance0 = 500.0f;
    EntityConfig.LODDistance1 = 1500.0f;
    EntityConfig.LODDistance2 = 3000.0f;

    // Initialize default waypoint network
    WaypointNetwork.Add(FVector(1000, 1000, 100));   // Market area
    WaypointNetwork.Add(FVector(-1000, 1000, 100));  // Residential
    WaypointNetwork.Add(FVector(1000, -1000, 100));  // Gathering point
    WaypointNetwork.Add(FVector(-1000, -1000, 100)); // Work area
    WaypointNetwork.Add(FVector(0, 2000, 100));      // Central plaza

    // Initialize default behavior zones
    FCrowd_BehaviorZone MarketZone;
    MarketZone.ZoneName = TEXT("BusyMarket");
    MarketZone.ZoneCenter = FVector(1000, 1000, 50);
    MarketZone.ZoneRadius = 800.0f;
    MarketZone.BehaviorType = ECrowdBehaviorType::Gathering;
    MarketZone.DensityMultiplier = 2.0f;
    BehaviorZones.Add(MarketZone);

    FCrowd_BehaviorZone ResidentialZone;
    ResidentialZone.ZoneName = TEXT("QuietResidential");
    ResidentialZone.ZoneCenter = FVector(-1000, 1000, 50);
    ResidentialZone.ZoneRadius = 600.0f;
    ResidentialZone.BehaviorType = ECrowdBehaviorType::Wandering;
    ResidentialZone.DensityMultiplier = 0.5f;
    BehaviorZones.Add(ResidentialZone);

    FCrowd_BehaviorZone WorkZone;
    WorkZone.ZoneName = TEXT("WorkArea");
    WorkZone.ZoneCenter = FVector(-1000, -1000, 50);
    WorkZone.ZoneRadius = 700.0f;
    WorkZone.BehaviorType = ECrowdBehaviorType::Working;
    WorkZone.DensityMultiplier = 1.5f;
    BehaviorZones.Add(WorkZone);

    // Initialize default panic responses
    FCrowd_PanicResponse DinosaurPanic;
    DinosaurPanic.TriggerLocation = FVector(500, 500, 100);
    DinosaurPanic.TriggerRadius = 800.0f;
    DinosaurPanic.PanicDuration = 45.0f;
    DinosaurPanic.EvacuationSpeed = 400.0f;
    DinosaurPanic.EvacuationTarget = FVector(0, -2000, 100);
    PanicResponses.Add(DinosaurPanic);

    FCrowd_PanicResponse CombatPanic;
    CombatPanic.TriggerLocation = FVector(-500, -500, 100);
    CombatPanic.TriggerRadius = 600.0f;
    CombatPanic.PanicDuration = 30.0f;
    CombatPanic.EvacuationSpeed = 350.0f;
    CombatPanic.EvacuationTarget = FVector(0, -2000, 100);
    PanicResponses.Add(CombatPanic);
}

void ACrowd_MassEntityManager::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bSystemInitialized)
    {
        InitializeCrowdSystem();
    }
}

void ACrowd_MassEntityManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bSystemInitialized)
    {
        return;
    }

    // Update LOD system based on player location
    if (bEnableLODSystem)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            UpdateCrowdLOD(PlayerPawn->GetActorLocation());
        }
    }

    // Check for panic triggers
    if (bEnablePanicSystem)
    {
        // This would be triggered by external events like combat or dinosaur encounters
        // For now, we just maintain the system structure
    }

    LastUpdateTime += DeltaTime;
}

void ACrowd_MassEntityManager::InitializeCrowdSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Initializing crowd system"));

    CreateWaypointNetwork();
    CreateBehaviorZones();
    SpawnCrowdEntities(EntityConfig.MaxEntities);

    bSystemInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: System initialized with %d entities"), SpawnedEntities.Num());
}

void ACrowd_MassEntityManager::SpawnCrowdEntities(int32 Count)
{
    if (!GetWorld())
    {
        return;
    }

    // Clear existing entities
    for (AActor* Entity : SpawnedEntities)
    {
        if (IsValid(Entity))
        {
            Entity->Destroy();
        }
    }
    SpawnedEntities.Empty();

    // Spawn new entities using basic actors as placeholders
    // In a full implementation, these would be Mass Entity instances
    for (int32 i = 0; i < Count; ++i)
    {
        FVector SpawnLocation = GetActorLocation() + FVector(
            FMath::RandRange(-EntityConfig.SpawnRadius, EntityConfig.SpawnRadius),
            FMath::RandRange(-EntityConfig.SpawnRadius, EntityConfig.SpawnRadius),
            100.0f
        );

        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AActor* CrowdEntity = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (CrowdEntity)
        {
            CrowdEntity->SetActorLabel(FString::Printf(TEXT("CrowdEntity_%d"), i));
            SpawnedEntities.Add(CrowdEntity);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Spawned %d crowd entities"), SpawnedEntities.Num());
}

void ACrowd_MassEntityManager::UpdateCrowdLOD(const FVector& ViewerLocation)
{
    if (SpawnedEntities.Num() == 0)
    {
        return;
    }

    int32 LOD0Count = 0, LOD1Count = 0, LOD2Count = 0, CulledCount = 0;

    for (AActor* Entity : SpawnedEntities)
    {
        if (!IsValid(Entity))
        {
            continue;
        }

        float Distance = FVector::Dist(ViewerLocation, Entity->GetActorLocation());

        if (Distance <= EntityConfig.LODDistance0)
        {
            // LOD0: Full detail
            Entity->SetActorHiddenInGame(false);
            Entity->SetActorTickEnabled(true);
            LOD0Count++;
        }
        else if (Distance <= EntityConfig.LODDistance1)
        {
            // LOD1: Reduced detail
            Entity->SetActorHiddenInGame(false);
            Entity->SetActorTickEnabled(false);
            LOD1Count++;
        }
        else if (Distance <= EntityConfig.LODDistance2)
        {
            // LOD2: Minimal detail
            Entity->SetActorHiddenInGame(false);
            Entity->SetActorTickEnabled(false);
            LOD2Count++;
        }
        else
        {
            // Culled: Hidden
            Entity->SetActorHiddenInGame(true);
            Entity->SetActorTickEnabled(false);
            CulledCount++;
        }
    }

    // Debug output every 5 seconds
    if (FMath::Fmod(LastUpdateTime, 5.0f) < 0.1f)
    {
        UE_LOG(LogTemp, Log, TEXT("Crowd LOD: LOD0=%d, LOD1=%d, LOD2=%d, Culled=%d"), 
               LOD0Count, LOD1Count, LOD2Count, CulledCount);
    }
}

void ACrowd_MassEntityManager::TriggerPanicResponse(const FVector& TriggerLocation, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Panic response triggered at %s"), *TriggerLocation.ToString());

    // Find entities within panic radius
    TArray<AActor*> PanickedEntities;
    for (AActor* Entity : SpawnedEntities)
    {
        if (IsValid(Entity))
        {
            float Distance = FVector::Dist(TriggerLocation, Entity->GetActorLocation());
            if (Distance <= Radius)
            {
                PanickedEntities.Add(Entity);
            }
        }
    }

    // Apply panic behavior (move away from trigger)
    for (AActor* Entity : PanickedEntities)
    {
        FVector AwayDirection = (Entity->GetActorLocation() - TriggerLocation).GetSafeNormal();
        FVector PanicTarget = Entity->GetActorLocation() + (AwayDirection * 1000.0f);
        
        // In a full implementation, this would set the entity's movement target
        // For now, we just log the panic response
        UE_LOG(LogTemp, Log, TEXT("Entity %s panicking, moving to %s"), 
               *Entity->GetName(), *PanicTarget.ToString());
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: %d entities affected by panic"), PanickedEntities.Num());
}

void ACrowd_MassEntityManager::CreateWaypointNetwork()
{
    if (!GetWorld())
    {
        return;
    }

    // Clear existing waypoints
    for (AActor* Waypoint : WaypointActors)
    {
        if (IsValid(Waypoint))
        {
            Waypoint->Destroy();
        }
    }
    WaypointActors.Empty();

    // Create waypoint actors
    for (int32 i = 0; i < WaypointNetwork.Num(); ++i)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        ATargetPoint* Waypoint = GetWorld()->SpawnActor<ATargetPoint>(
            ATargetPoint::StaticClass(), 
            WaypointNetwork[i], 
            FRotator::ZeroRotator, 
            SpawnParams
        );

        if (Waypoint)
        {
            Waypoint->SetActorLabel(FString::Printf(TEXT("CrowdWaypoint_%d"), i + 1));
            WaypointActors.Add(Waypoint);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Created %d waypoints"), WaypointActors.Num());
}

void ACrowd_MassEntityManager::CreateBehaviorZones()
{
    if (!GetWorld())
    {
        return;
    }

    // Clear existing zones
    for (AActor* Zone : ZoneActors)
    {
        if (IsValid(Zone))
        {
            Zone->Destroy();
        }
    }
    ZoneActors.Empty();

    // Create zone visualization actors
    for (const FCrowd_BehaviorZone& Zone : BehaviorZones)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = this;

        AActor* ZoneActor = GetWorld()->SpawnActor<AActor>(
            AActor::StaticClass(), 
            Zone.ZoneCenter, 
            FRotator::ZeroRotator, 
            SpawnParams
        );

        if (ZoneActor)
        {
            ZoneActor->SetActorLabel(FString::Printf(TEXT("CrowdZone_%s"), *Zone.ZoneName));
            ZoneActor->SetActorScale3D(FVector(Zone.ZoneRadius / 100.0f, Zone.ZoneRadius / 100.0f, 1.0f));
            ZoneActors.Add(ZoneActor);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Crowd_MassEntityManager: Created %d behavior zones"), ZoneActors.Num());
}

void ACrowd_MassEntityManager::ValidateCrowdSystem()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Crowd System Validation ==="));
    UE_LOG(LogTemp, Warning, TEXT("Max Entities: %d"), EntityConfig.MaxEntities);
    UE_LOG(LogTemp, Warning, TEXT("Spawn Radius: %.1f"), EntityConfig.SpawnRadius);
    UE_LOG(LogTemp, Warning, TEXT("Active Entities: %d"), SpawnedEntities.Num());
    UE_LOG(LogTemp, Warning, TEXT("Waypoints: %d"), WaypointActors.Num());
    UE_LOG(LogTemp, Warning, TEXT("Behavior Zones: %d"), BehaviorZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("Panic Responses: %d"), PanicResponses.Num());
    UE_LOG(LogTemp, Warning, TEXT("System Initialized: %s"), bSystemInitialized ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Warning, TEXT("LOD System: %s"), bEnableLODSystem ? TEXT("Enabled") : TEXT("Disabled"));
    UE_LOG(LogTemp, Warning, TEXT("Panic System: %s"), bEnablePanicSystem ? TEXT("Enabled") : TEXT("Disabled"));
}

int32 ACrowd_MassEntityManager::GetActiveCrowdCount() const
{
    int32 ActiveCount = 0;
    for (const AActor* Entity : SpawnedEntities)
    {
        if (IsValid(Entity) && !Entity->IsActorBeingDestroyed())
        {
            ActiveCount++;
        }
    }
    return ActiveCount;
}

float ACrowd_MassEntityManager::GetCrowdDensity(const FVector& Location, float Radius) const
{
    int32 EntitiesInRadius = 0;
    for (const AActor* Entity : SpawnedEntities)
    {
        if (IsValid(Entity))
        {
            float Distance = FVector::Dist(Location, Entity->GetActorLocation());
            if (Distance <= Radius)
            {
                EntitiesInRadius++;
            }
        }
    }

    float Area = PI * Radius * Radius;
    return EntitiesInRadius / Area; // Entities per square unit
}

ECrowdBehaviorType ACrowd_MassEntityManager::GetZoneBehaviorType(const FVector& Location) const
{
    for (const FCrowd_BehaviorZone& Zone : BehaviorZones)
    {
        float Distance = FVector::Dist(Location, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius)
        {
            return Zone.BehaviorType;
        }
    }
    return ECrowdBehaviorType::Wandering; // Default behavior
}
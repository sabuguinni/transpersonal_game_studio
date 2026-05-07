#include "World_PerformanceOptimizedBiomes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

AWorld_PerformanceOptimizedBiomes::AWorld_PerformanceOptimizedBiomes()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Performance defaults
    MaxActorsPerFrame = 5;
    MaxTotalActors = 200;
    VegetationCullingDistance = 5000.0f;
    
    // Initialize performance tracking
    CurrentActorCount = 0;
    LastFrameTime = 0.0f;
    bPerformanceLimitReached = false;
    PerformanceCheckTimer = 0.0f;
    
    // Initialize default vegetation zones
    FWorld_VegetationZone ForestZone;
    ForestZone.ZoneName = TEXT("Forest_Core");
    ForestZone.Center = FVector(2000.0f, 2000.0f, 100.0f);
    ForestZone.Radius = 800.0f;
    ForestZone.MaxDensity = 15;
    ForestZone.BiomeType = EWorld_BiomeType::Forest;
    VegetationZones.Add(ForestZone);
    
    FWorld_VegetationZone PlainsZone;
    PlainsZone.ZoneName = TEXT("Plains_Edge");
    PlainsZone.Center = FVector(-1500.0f, 1000.0f, 50.0f);
    PlainsZone.Radius = 600.0f;
    PlainsZone.MaxDensity = 10;
    PlainsZone.BiomeType = EWorld_BiomeType::Grassland;
    VegetationZones.Add(PlainsZone);
    
    // Initialize default weather zones
    FWorld_WeatherZone StormZone;
    StormZone.ZoneName = TEXT("Storm_Center");
    StormZone.Location = FVector(3000.0f, 0.0f, 500.0f);
    StormZone.WeatherType = EWorld_WeatherType::Storm;
    StormZone.Intensity = 0.8f;
    StormZone.EffectRadius = 2500.0f;
    WeatherZones.Add(StormZone);
    
    FWorld_WeatherZone FogZone;
    FogZone.ZoneName = TEXT("Mist_Valley");
    FogZone.Location = FVector(-2000.0f, -1000.0f, 100.0f);
    FogZone.WeatherType = EWorld_WeatherType::Fog;
    FogZone.Intensity = 0.6f;
    FogZone.EffectRadius = 1500.0f;
    WeatherZones.Add(FogZone);
}

void AWorld_PerformanceOptimizedBiomes::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimized Biomes system initialized"));
    
    // Initialize vegetation zones with performance limits
    for (const FWorld_VegetationZone& Zone : VegetationZones)
    {
        if (CanSpawnMoreActors())
        {
            SpawnVegetationBatch(Zone, FMath::Min(Zone.MaxDensity, MaxActorsPerFrame));
        }
    }
    
    // Create weather zone markers
    for (const FWorld_WeatherZone& Zone : WeatherZones)
    {
        CreateWeatherZone(Zone);
    }
    
    UpdatePerformanceMetrics();
}

void AWorld_PerformanceOptimizedBiomes::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastFrameTime = DeltaTime;
    PerformanceCheckTimer += DeltaTime;
    
    // Performance check every 2 seconds
    if (PerformanceCheckTimer >= 2.0f)
    {
        UpdatePerformanceMetrics();
        ManageActorLOD();
        PerformanceCheckTimer = 0.0f;
    }
    
    // Update weather effects every frame
    UpdateWeatherEffects();
}

void AWorld_PerformanceOptimizedBiomes::CreateVegetationZone(const FWorld_VegetationZone& ZoneData)
{
    if (!CanSpawnMoreActors())
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot create vegetation zone - performance limit reached"));
        return;
    }
    
    VegetationZones.Add(ZoneData);
    SpawnVegetationBatch(ZoneData, FMath::Min(ZoneData.MaxDensity, MaxActorsPerFrame));
    
    UE_LOG(LogTemp, Log, TEXT("Created vegetation zone: %s"), *ZoneData.ZoneName);
}

void AWorld_PerformanceOptimizedBiomes::CreateWeatherZone(const FWorld_WeatherZone& ZoneData)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Spawn weather marker actor
    AActor* WeatherMarker = World->SpawnActor<AActor>(AActor::StaticClass(), ZoneData.Location, FRotator::ZeroRotator);
    if (WeatherMarker)
    {
        WeatherMarker->SetActorLabel(FString::Printf(TEXT("WeatherZone_%s"), *ZoneData.ZoneName));
        WeatherMarkerActors.Add(WeatherMarker);
        
        UE_LOG(LogTemp, Log, TEXT("Created weather zone: %s"), *ZoneData.ZoneName);
    }
}

void AWorld_PerformanceOptimizedBiomes::SpawnVegetationBatch(const FWorld_VegetationZone& Zone, int32 BatchSize)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 SpawnedCount = 0;
    
    for (int32 i = 0; i < BatchSize && CanSpawnMoreActors(); i++)
    {
        // Generate random position within zone
        float Angle = FMath::RandRange(0.0f, 2.0f * PI);
        float Distance = FMath::RandRange(0.0f, Zone.Radius);
        
        FVector SpawnLocation;
        SpawnLocation.X = Zone.Center.X + Distance * FMath::Cos(Angle);
        SpawnLocation.Y = Zone.Center.Y + Distance * FMath::Sin(Angle);
        SpawnLocation.Z = Zone.Center.Z + FMath::RandRange(-20.0f, 40.0f);
        
        // Spawn tree actor
        AStaticMeshActor* TreeActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            SpawnLocation,
            FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f)
        );
        
        if (TreeActor)
        {
            TreeActor->SetActorLabel(FString::Printf(TEXT("Tree_%s_%d"), *Zone.ZoneName, i));
            
            // Set basic tree mesh
            UStaticMeshComponent* MeshComp = TreeActor->GetStaticMeshComponent();
            if (MeshComp)
            {
                UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder"));
                if (CylinderMesh)
                {
                    MeshComp->SetStaticMesh(CylinderMesh);
                    TreeActor->SetActorScale3D(FVector(0.3f, 0.3f, 2.5f));
                }
            }
            
            SpawnedVegetationActors.Add(TreeActor);
            SpawnedCount++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %d vegetation actors in zone %s"), SpawnedCount, *Zone.ZoneName);
}

void AWorld_PerformanceOptimizedBiomes::OptimizeVegetationDensity()
{
    if (LastFrameTime > 0.033f) // If frame time > 33ms (below 30 FPS)
    {
        bPerformanceLimitReached = true;
        
        // Reduce vegetation density by removing distant actors
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            CullDistantVegetation(PlayerPawn->GetActorLocation());
        }
    }
    else
    {
        bPerformanceLimitReached = false;
    }
}

void AWorld_PerformanceOptimizedBiomes::CullDistantVegetation(const FVector& PlayerLocation)
{
    TArray<AActor*> ActorsToCull;
    
    for (AActor* VegetationActor : SpawnedVegetationActors)
    {
        if (VegetationActor && IsValid(VegetationActor))
        {
            float Distance = FVector::Dist(VegetationActor->GetActorLocation(), PlayerLocation);
            if (Distance > VegetationCullingDistance)
            {
                ActorsToCull.Add(VegetationActor);
            }
        }
    }
    
    // Remove distant actors
    for (AActor* ActorToCull : ActorsToCull)
    {
        SpawnedVegetationActors.Remove(ActorToCull);
        ActorToCull->Destroy();
    }
    
    if (ActorsToCull.Num() > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("Culled %d distant vegetation actors for performance"), ActorsToCull.Num());
    }
}

int32 AWorld_PerformanceOptimizedBiomes::GetCurrentActorCount()
{
    // Clean up invalid actors
    SpawnedVegetationActors.RemoveAll([](AActor* Actor) {
        return !IsValid(Actor);
    });
    
    CurrentActorCount = SpawnedVegetationActors.Num() + WeatherMarkerActors.Num();
    return CurrentActorCount;
}

void AWorld_PerformanceOptimizedBiomes::UpdateWeatherEffects()
{
    // Update weather zone effects based on intensity and type
    for (const FWorld_WeatherZone& Zone : WeatherZones)
    {
        // Weather effects would be implemented here
        // For now, just log the active weather zones
        if (Zone.Intensity > 0.5f)
        {
            // High intensity weather zone is active
        }
    }
}

bool AWorld_PerformanceOptimizedBiomes::CanSpawnMoreActors() const
{
    return CurrentActorCount < MaxTotalActors && !bPerformanceLimitReached;
}

EWorld_BiomeType AWorld_PerformanceOptimizedBiomes::GetBiomeAtLocation(const FVector& Location) const
{
    for (const FWorld_VegetationZone& Zone : VegetationZones)
    {
        if (IsLocationInVegetationZone(Location, Zone))
        {
            return Zone.BiomeType;
        }
    }
    
    return EWorld_BiomeType::Desert; // Default biome
}

void AWorld_PerformanceOptimizedBiomes::UpdatePerformanceMetrics()
{
    CurrentActorCount = GetCurrentActorCount();
    OptimizeVegetationDensity();
    
    UE_LOG(LogTemp, Log, TEXT("Performance Metrics - Actors: %d/%d, Frame Time: %fms, Performance Limited: %s"),
        CurrentActorCount, MaxTotalActors, LastFrameTime * 1000.0f, bPerformanceLimitReached ? TEXT("Yes") : TEXT("No"));
}

void AWorld_PerformanceOptimizedBiomes::ManageActorLOD()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (AActor* VegetationActor : SpawnedVegetationActors)
    {
        if (VegetationActor && IsValid(VegetationActor))
        {
            float Distance = CalculateDistanceToPlayer(VegetationActor->GetActorLocation());
            
            // Simple LOD management based on distance
            if (Distance > VegetationCullingDistance * 0.5f)
            {
                // Reduce detail for distant actors
                VegetationActor->SetActorHiddenInGame(Distance > VegetationCullingDistance);
            }
            else
            {
                VegetationActor->SetActorHiddenInGame(false);
            }
        }
    }
}

bool AWorld_PerformanceOptimizedBiomes::IsLocationInVegetationZone(const FVector& Location, const FWorld_VegetationZone& Zone) const
{
    float Distance = FVector::Dist(Location, Zone.Center);
    return Distance <= Zone.Radius;
}

float AWorld_PerformanceOptimizedBiomes::CalculateDistanceToPlayer(const FVector& ActorLocation) const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        return FVector::Dist(ActorLocation, PlayerPawn->GetActorLocation());
    }
    return 0.0f;
}
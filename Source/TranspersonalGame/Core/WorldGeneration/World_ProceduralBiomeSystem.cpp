#include "World_ProceduralBiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"

UWorld_ProceduralBiomeSystem::UWorld_ProceduralBiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
    bAutoPopulateBiomes = true;
    PopulationUpdateInterval = 30.0f;
    MaxActorsPerBiome = 500;
    CullingDistance = 2000000.0f; // 20km
    MaxSpawnsPerFrame = 5;
    CurrentSpawnIndex = 0;
    LastUpdateTime = 0.0f;
}

void UWorld_ProceduralBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    SetupDefaultBiomes();
    
    if (bAutoPopulateBiomes)
    {
        InitializeBiomes();
        
        // Start population update timer
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().SetTimer(
                PopulationTimerHandle,
                this,
                &UWorld_ProceduralBiomeSystem::UpdatePopulation,
                PopulationUpdateInterval,
                true
            );
        }
    }
}

void UWorld_ProceduralBiomeSystem::SetupDefaultBiomes()
{
    BiomeConfigs.Empty();
    
    // Savana biome
    FWorld_BiomeConfig Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    Savana.Radius = 1000000.0f;
    Savana.MaxDinosaurs = 15;
    Savana.MaxTerrainFeatures = 100;
    Savana.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"));
    Savana.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"));
    Savana.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops"));
    Savana.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Cube"));
    Savana.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Cylinder"));
    BiomeConfigs.Add(Savana);
    
    // Floresta biome
    FWorld_BiomeConfig Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    Floresta.Radius = 800000.0f;
    Floresta.MaxDinosaurs = 12;
    Floresta.MaxTerrainFeatures = 150;
    Floresta.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus"));
    Floresta.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Parasaurolophus/Mesh/SKM_Parasaurolophus_Mesh"));
    Floresta.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Cylinder"));
    Floresta.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Sphere"));
    BiomeConfigs.Add(Floresta);
    
    // Pantano biome
    FWorld_BiomeConfig Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    Pantano.Radius = 600000.0f;
    Pantano.MaxDinosaurs = 8;
    Pantano.MaxTerrainFeatures = 80;
    Pantano.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh"));
    Pantano.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Protoceratops/Mesh/SKM_Protoceratops_Skin"));
    Pantano.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Plane"));
    BiomeConfigs.Add(Pantano);
    
    // Deserto biome
    FWorld_BiomeConfig Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    Deserto.Radius = 900000.0f;
    Deserto.MaxDinosaurs = 6;
    Deserto.MaxTerrainFeatures = 60;
    Deserto.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Pachycephalo/Mesh/SKM_Pachycephalo"));
    Deserto.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Cube"));
    BiomeConfigs.Add(Deserto);
    
    // Montanha biome
    FWorld_BiomeConfig Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 100.0f);
    Montanha.Radius = 700000.0f;
    Montanha.MaxDinosaurs = 10;
    Montanha.MaxTerrainFeatures = 120;
    Montanha.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Tsintaosaurus/Mesh/SKM_Tsintaosaurus_Mesh"));
    Montanha.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Sphere"));
    Montanha.TerrainAssetPaths.Add(TEXT("/Engine/BasicShapes/Cube"));
    BiomeConfigs.Add(Montanha);
}

void UWorld_ProceduralBiomeSystem::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Initializing %d biomes"), BiomeConfigs.Num());
    
    for (const FWorld_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Biome %s initialized at location %s"), 
               *BiomeConfig.BiomeName, 
               *BiomeConfig.CenterLocation.ToString());
    }
}

void UWorld_ProceduralBiomeSystem::PopulateBiome(const FString& BiomeName)
{
    FWorld_BiomeConfig* BiomeConfig = BiomeConfigs.FindByPredicate([BiomeName](const FWorld_BiomeConfig& Config)
    {
        return Config.BiomeName == BiomeName;
    });
    
    if (!BiomeConfig)
    {
        UE_LOG(LogTemp, Error, TEXT("Biome %s not found"), *BiomeName);
        return;
    }
    
    int32 CurrentPopulation = GetBiomePopulation(BiomeName);
    int32 TargetPopulation = BiomeConfig->MaxDinosaurs + BiomeConfig->MaxTerrainFeatures;
    
    if (CurrentPopulation >= TargetPopulation)
    {
        return;
    }
    
    // Spawn dinosaurs
    int32 DinosaurCount = GetActorsInBiome(BiomeName).Num();
    if (DinosaurCount < BiomeConfig->MaxDinosaurs && BiomeConfig->DinosaurAssetPaths.Num() > 0)
    {
        int32 SpawnsNeeded = FMath::Min(MaxSpawnsPerFrame, BiomeConfig->MaxDinosaurs - DinosaurCount);
        
        for (int32 i = 0; i < SpawnsNeeded; i++)
        {
            FString AssetPath = BiomeConfig->DinosaurAssetPaths[FMath::RandRange(0, BiomeConfig->DinosaurAssetPaths.Num() - 1)];
            SpawnActorInBiome(BiomeName, AssetPath);
        }
    }
    
    // Spawn terrain features
    int32 TerrainCount = GetActorsInBiome(BiomeName).Num() - DinosaurCount;
    if (TerrainCount < BiomeConfig->MaxTerrainFeatures && BiomeConfig->TerrainAssetPaths.Num() > 0)
    {
        int32 SpawnsNeeded = FMath::Min(MaxSpawnsPerFrame, BiomeConfig->MaxTerrainFeatures - TerrainCount);
        
        for (int32 i = 0; i < SpawnsNeeded; i++)
        {
            FString AssetPath = BiomeConfig->TerrainAssetPaths[FMath::RandRange(0, BiomeConfig->TerrainAssetPaths.Num() - 1)];
            FVector Offset = FVector(0.0f, 0.0f, -50.0f); // Terrain features slightly below ground
            SpawnActorInBiome(BiomeName, AssetPath, Offset);
        }
    }
}

void UWorld_ProceduralBiomeSystem::PopulateAllBiomes()
{
    for (const FWorld_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        PopulateBiome(BiomeConfig.BiomeName);
    }
}

int32 UWorld_ProceduralBiomeSystem::GetBiomePopulation(const FString& BiomeName) const
{
    int32 Count = 0;
    for (const FWorld_SpawnedActor& SpawnedActor : SpawnedActors)
    {
        if (SpawnedActor.BiomeName == BiomeName && SpawnedActor.ActorRef.IsValid())
        {
            Count++;
        }
    }
    return Count;
}

FWorld_BiomeConfig UWorld_ProceduralBiomeSystem::GetBiomeConfig(const FString& BiomeName) const
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeName == BiomeName)
        {
            return Config;
        }
    }
    return FWorld_BiomeConfig();
}

TArray<AActor*> UWorld_ProceduralBiomeSystem::GetActorsInBiome(const FString& BiomeName) const
{
    TArray<AActor*> BiomeActors;
    
    for (const FWorld_SpawnedActor& SpawnedActor : SpawnedActors)
    {
        if (SpawnedActor.BiomeName == BiomeName && SpawnedActor.ActorRef.IsValid())
        {
            if (AActor* Actor = SpawnedActor.ActorRef.Get())
            {
                BiomeActors.Add(Actor);
            }
        }
    }
    
    return BiomeActors;
}

AActor* UWorld_ProceduralBiomeSystem::SpawnActorInBiome(const FString& BiomeName, const FString& AssetPath, const FVector& Offset)
{
    FWorld_BiomeConfig BiomeConfig = GetBiomeConfig(BiomeName);
    if (BiomeConfig.BiomeName.IsEmpty())
    {
        return nullptr;
    }
    
    FVector SpawnLocation = GetRandomLocationInBiome(BiomeConfig) + Offset;
    
    if (!IsLocationValid(SpawnLocation, BiomeName))
    {
        return nullptr;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Try to load the asset
    UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
    if (!Asset)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load asset: %s"), *AssetPath);
        return nullptr;
    }
    
    // Spawn static mesh actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AStaticMeshActor* SpawnedActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);
    if (!SpawnedActor)
    {
        return nullptr;
    }
    
    // Set the mesh
    if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(Asset))
    {
        SpawnedActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
    }
    
    // Set random scale for variety
    float Scale = FMath::RandRange(0.8f, 1.5f);
    SpawnedActor->SetActorScale3D(FVector(Scale));
    
    // Set label
    FString ActorType = AssetPath.Contains(TEXT("Dinosaur")) ? TEXT("Dinosaur") : TEXT("Terrain");
    SpawnedActor->SetActorLabel(FString::Printf(TEXT("%s_%s_%d"), *ActorType, *BiomeName, SpawnedActors.Num()));
    
    // Register the spawned actor
    RegisterSpawnedActor(SpawnedActor, BiomeName, ActorType);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned %s in %s at %s"), *ActorType, *BiomeName, *SpawnLocation.ToString());
    
    return SpawnedActor;
}

void UWorld_ProceduralBiomeSystem::ClearBiome(const FString& BiomeName)
{
    TArray<AActor*> ActorsToDestroy = GetActorsInBiome(BiomeName);
    
    for (AActor* Actor : ActorsToDestroy)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    
    // Remove from spawned actors list
    SpawnedActors.RemoveAll([BiomeName](const FWorld_SpawnedActor& SpawnedActor)
    {
        return SpawnedActor.BiomeName == BiomeName;
    });
    
    UE_LOG(LogTemp, Warning, TEXT("Cleared biome %s - destroyed %d actors"), *BiomeName, ActorsToDestroy.Num());
}

void UWorld_ProceduralBiomeSystem::ClearAllBiomes()
{
    for (const FWorld_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        ClearBiome(BiomeConfig.BiomeName);
    }
}

void UWorld_ProceduralBiomeSystem::UpdateActorCulling()
{
    if (!GetWorld())
    {
        return;
    }
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FWorld_SpawnedActor& SpawnedActor : SpawnedActors)
    {
        if (AActor* Actor = SpawnedActor.ActorRef.Get())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            bool bShouldBeVisible = Distance <= CullingDistance;
            
            Actor->SetActorHiddenInGame(!bShouldBeVisible);
            Actor->SetActorEnableCollision(bShouldBeVisible);
        }
    }
}

void UWorld_ProceduralBiomeSystem::OptimizeBiomePerformance()
{
    CleanupInvalidActors();
    UpdateActorCulling();
    
    // Log performance stats
    int32 TotalActors = SpawnedActors.Num();
    int32 ValidActors = 0;
    
    for (const FWorld_SpawnedActor& SpawnedActor : SpawnedActors)
    {
        if (SpawnedActor.ActorRef.IsValid())
        {
            ValidActors++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomeSystem Performance: %d/%d valid actors"), ValidActors, TotalActors);
}

void UWorld_ProceduralBiomeSystem::EditorPopulateBiomes()
{
    PopulateAllBiomes();
}

void UWorld_ProceduralBiomeSystem::EditorClearBiomes()
{
    ClearAllBiomes();
}

void UWorld_ProceduralBiomeSystem::EditorValidateBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME VALIDATION ==="));
    
    for (const FWorld_BiomeConfig& BiomeConfig : BiomeConfigs)
    {
        int32 Population = GetBiomePopulation(BiomeConfig.BiomeName);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d/%d actors"), 
               *BiomeConfig.BiomeName, 
               Population, 
               BiomeConfig.MaxDinosaurs + BiomeConfig.MaxTerrainFeatures);
    }
    
    OptimizeBiomePerformance();
}

void UWorld_ProceduralBiomeSystem::UpdatePopulation()
{
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    LastUpdateTime = CurrentTime;
    
    // Update one biome per tick to spread the load
    if (BiomeConfigs.IsValidIndex(CurrentSpawnIndex))
    {
        PopulateBiome(BiomeConfigs[CurrentSpawnIndex].BiomeName);
        CurrentSpawnIndex = (CurrentSpawnIndex + 1) % BiomeConfigs.Num();
    }
    
    // Periodic optimization
    if (FMath::RandRange(0, 10) == 0)
    {
        OptimizeBiomePerformance();
    }
}

FVector UWorld_ProceduralBiomeSystem::GetRandomLocationInBiome(const FWorld_BiomeConfig& BiomeConfig) const
{
    float RandomRadius = FMath::RandRange(0.0f, BiomeConfig.Radius * 0.8f);
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    
    FVector Offset;
    Offset.X = RandomRadius * FMath::Cos(RandomAngle);
    Offset.Y = RandomRadius * FMath::Sin(RandomAngle);
    Offset.Z = 0.0f;
    
    return BiomeConfig.CenterLocation + Offset;
}

bool UWorld_ProceduralBiomeSystem::IsLocationValid(const FVector& Location, const FString& BiomeName) const
{
    // Basic validation - could be expanded with terrain checks
    return true;
}

void UWorld_ProceduralBiomeSystem::RegisterSpawnedActor(AActor* Actor, const FString& BiomeName, const FString& ActorType)
{
    if (!Actor)
    {
        return;
    }
    
    FWorld_SpawnedActor NewSpawnedActor;
    NewSpawnedActor.ActorRef = Actor;
    NewSpawnedActor.BiomeName = BiomeName;
    NewSpawnedActor.ActorType = ActorType;
    NewSpawnedActor.SpawnLocation = Actor->GetActorLocation();
    NewSpawnedActor.SpawnTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    SpawnedActors.Add(NewSpawnedActor);
}

void UWorld_ProceduralBiomeSystem::CleanupInvalidActors()
{
    SpawnedActors.RemoveAll([](const FWorld_SpawnedActor& SpawnedActor)
    {
        return !SpawnedActor.ActorRef.IsValid();
    });
}
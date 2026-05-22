#include "World_ProceduralBiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"

AWorld_ProceduralBiomeSystem::AWorld_ProceduralBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Performance settings
    MaxActiveActors = 500;
    UpdateInterval = 2.0f;
    CullingDistance = 50000.0f;
    
    // Runtime state
    TotalSpawnedCount = 0;
    LastUpdateTime = 0.0f;
    CurrentSpawnIndex = 0;
    bIsPopulating = false;
    
    // Initialize default biomes
    InitializeDefaultBiomes();
}

void AWorld_ProceduralBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: BeginPlay - Initializing biome system"));
    
    // Start biome population after a short delay
    FTimerHandle PopulationTimer;
    GetWorld()->GetTimerManager().SetTimer(PopulationTimer, this, &AWorld_ProceduralBiomeSystem::PopulateAllBiomes, 3.0f, false);
}

void AWorld_ProceduralBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastUpdateTime += DeltaTime;
    
    // Periodic optimization
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateActorDistances();
        OptimizeActorsByDistance();
        LastUpdateTime = 0.0f;
    }
}

void AWorld_ProceduralBiomeSystem::InitializeDefaultBiomes()
{
    BiomeDefinitions.Empty();
    
    // Savana biome
    FWorld_BiomeDefinition Savana;
    Savana.BiomeName = TEXT("Savana");
    Savana.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    Savana.Radius = 15000.0f;
    Savana.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"));
    Savana.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops"));
    Savana.VegetationTypes.Add(TEXT("SavanaTree"));
    Savana.VegetationTypes.Add(TEXT("SavanaGrass"));
    Savana.DinosaurDensity = 0.15f;
    Savana.VegetationDensity = 0.3f;
    BiomeDefinitions.Add(Savana);
    
    // Floresta biome
    FWorld_BiomeDefinition Floresta;
    Floresta.BiomeName = TEXT("Floresta");
    Floresta.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    Floresta.Radius = 18000.0f;
    Floresta.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus"));
    Floresta.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Parasaurolophus/Mesh/SKM_Parasaurolophus_Mesh"));
    Floresta.VegetationTypes.Add(TEXT("ForestTree"));
    Floresta.VegetationTypes.Add(TEXT("Fern"));
    Floresta.DinosaurDensity = 0.12f;
    Floresta.VegetationDensity = 0.8f;
    BiomeDefinitions.Add(Floresta);
    
    // Deserto biome
    FWorld_BiomeDefinition Deserto;
    Deserto.BiomeName = TEXT("Deserto");
    Deserto.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    Deserto.Radius = 12000.0f;
    Deserto.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh"));
    Deserto.VegetationTypes.Add(TEXT("Cactus"));
    Deserto.VegetationTypes.Add(TEXT("DeadTree"));
    Deserto.DinosaurDensity = 0.08f;
    Deserto.VegetationDensity = 0.2f;
    BiomeDefinitions.Add(Deserto);
    
    // Pantano biome
    FWorld_BiomeDefinition Pantano;
    Pantano.BiomeName = TEXT("Pantano");
    Pantano.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    Pantano.Radius = 14000.0f;
    Pantano.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"));
    Pantano.VegetationTypes.Add(TEXT("SwampTree"));
    Pantano.VegetationTypes.Add(TEXT("Moss"));
    Pantano.DinosaurDensity = 0.2f;
    Pantano.VegetationDensity = 0.6f;
    BiomeDefinitions.Add(Pantano);
    
    // Montanha biome
    FWorld_BiomeDefinition Montanha;
    Montanha.BiomeName = TEXT("Montanha");
    Montanha.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    Montanha.Radius = 16000.0f;
    Montanha.DinosaurAssetPaths.Add(TEXT("/Game/Dinosaur_Pack/Pachycephalo/Mesh/SKM_Pachycephalo"));
    Montanha.VegetationTypes.Add(TEXT("PineTree"));
    Montanha.VegetationTypes.Add(TEXT("Rock"));
    Montanha.DinosaurDensity = 0.1f;
    Montanha.VegetationDensity = 0.4f;
    BiomeDefinitions.Add(Montanha);
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Initialized %d biomes"), BiomeDefinitions.Num());
}

void AWorld_ProceduralBiomeSystem::PopulateAllBiomes()
{
    if (bIsPopulating)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Already populating, skipping"));
        return;
    }
    
    bIsPopulating = true;
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Starting biome population"));
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        PopulateBiome(Biome);
    }
    
    bIsPopulating = false;
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Biome population complete - %d actors spawned"), TotalSpawnedCount);
}

void AWorld_ProceduralBiomeSystem::PopulateBiome(const FWorld_BiomeDefinition& BiomeData)
{
    if (TotalSpawnedCount >= MaxActiveActors)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Max actors reached, skipping biome %s"), *BiomeData.BiomeName);
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Populating biome %s"), *BiomeData.BiomeName);
    
    // Calculate spawn counts based on density and performance limits
    int32 MaxDinosInBiome = FMath::Min(5, MaxActiveActors - TotalSpawnedCount);
    int32 MaxVegetationInBiome = FMath::Min(15, MaxActiveActors - TotalSpawnedCount - MaxDinosInBiome);
    
    // Spawn dinosaurs
    for (int32 i = 0; i < MaxDinosInBiome && i < BiomeData.DinosaurAssetPaths.Num(); ++i)
    {
        if (TotalSpawnedCount >= MaxActiveActors) break;
        
        const FString& DinoPath = BiomeData.DinosaurAssetPaths[i % BiomeData.DinosaurAssetPaths.Num()];
        FVector SpawnLocation = GenerateRandomLocationInBiome(BiomeData);
        
        AActor* DinoActor = SpawnDinosaurInBiome(DinoPath, SpawnLocation, BiomeData.BiomeName);
        if (DinoActor)
        {
            UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Spawned dinosaur %s in %s"), *DinoActor->GetName(), *BiomeData.BiomeName);
        }
    }
    
    // Spawn vegetation clusters
    for (int32 i = 0; i < MaxVegetationInBiome; ++i)
    {
        if (TotalSpawnedCount >= MaxActiveActors) break;
        
        FVector SpawnLocation = GenerateRandomLocationInBiome(BiomeData);
        const FString& VegType = BiomeData.VegetationTypes.Num() > 0 ? 
            BiomeData.VegetationTypes[i % BiomeData.VegetationTypes.Num()] : TEXT("DefaultVegetation");
        
        AActor* VegActor = SpawnVegetationInBiome(VegType, SpawnLocation, BiomeData.BiomeName);
        if (VegActor)
        {
            UE_LOG(LogTemp, Log, TEXT("ProceduralBiomeSystem: Spawned vegetation in %s"), *BiomeData.BiomeName);
        }
    }
}

AActor* AWorld_ProceduralBiomeSystem::SpawnDinosaurInBiome(const FString& AssetPath, const FVector& Location, const FString& BiomeName)
{
    if (!GetWorld()) return nullptr;
    
    // Try to load the dinosaur mesh asset
    UObject* LoadedAsset = nullptr;
    
    // Load asset using soft object path
    FSoftObjectPath SoftPath(AssetPath);
    LoadedAsset = SoftPath.TryLoad();
    
    if (!LoadedAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: Failed to load dinosaur asset: %s"), *AssetPath);
        return nullptr;
    }
    
    // Spawn StaticMeshActor and set the mesh
    AStaticMeshActor* MeshActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (MeshActor && MeshActor->GetStaticMeshComponent())
    {
        if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(LoadedAsset))
        {
            MeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
        }
        
        // Set actor label
        FString DinoType = AssetPath;
        DinoType = DinoType.Replace(TEXT("/Game/Dinosaur_Pack/"), TEXT(""));
        DinoType = DinoType.Replace(TEXT("/Mesh/SKM_"), TEXT(""));
        DinoType = DinoType.Replace(TEXT("_Skin"), TEXT(""));
        DinoType = DinoType.Replace(TEXT("_Mesh"), TEXT(""));
        
        FString ActorLabel = FString::Printf(TEXT("%s_%s_%03d"), *DinoType, *BiomeName, TotalSpawnedCount);
        MeshActor->SetActorLabel(ActorLabel);
        
        // Register the spawned actor
        RegisterSpawnedActor(MeshActor, BiomeName, TEXT("Dinosaur"), Location);
        
        return MeshActor;
    }
    
    return nullptr;
}

AActor* AWorld_ProceduralBiomeSystem::SpawnVegetationInBiome(const FString& VegetationType, const FVector& Location, const FString& BiomeName)
{
    if (!GetWorld()) return nullptr;
    
    // Spawn basic vegetation using StaticMeshActor
    AStaticMeshActor* VegActor = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (VegActor)
    {
        // Set random scale for variety
        float Scale = FMath::RandRange(0.5f, 2.0f);
        VegActor->SetActorScale3D(FVector(Scale, Scale, Scale));
        
        FString ActorLabel = FString::Printf(TEXT("%s_%s_%03d"), *VegetationType, *BiomeName, TotalSpawnedCount);
        VegActor->SetActorLabel(ActorLabel);
        
        // Register the spawned actor
        RegisterSpawnedActor(VegActor, BiomeName, TEXT("Vegetation"), Location);
        
        return VegActor;
    }
    
    return nullptr;
}

void AWorld_ProceduralBiomeSystem::OptimizeActorsByDistance()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    for (FWorld_SpawnedActorInfo& ActorInfo : SpawnedActors)
    {
        if (!ActorInfo.SpawnedActor || !IsValid(ActorInfo.SpawnedActor)) continue;
        
        float Distance = FVector::Dist(PlayerLocation, ActorInfo.SpawnedActor->GetActorLocation());
        ActorInfo.DistanceFromPlayer = Distance;
        
        // Simple LOD based on distance
        if (Distance > CullingDistance * 0.8f)
        {
            // Very far - minimal detail
            ActorInfo.SpawnedActor->SetActorHiddenInGame(true);
        }
        else if (Distance > CullingDistance * 0.5f)
        {
            // Far - reduced detail
            ActorInfo.SpawnedActor->SetActorHiddenInGame(false);
            ActorInfo.SpawnedActor->SetActorTickEnabled(false);
        }
        else
        {
            // Near - full detail
            ActorInfo.SpawnedActor->SetActorHiddenInGame(false);
            ActorInfo.SpawnedActor->SetActorTickEnabled(true);
        }
    }
}

void AWorld_ProceduralBiomeSystem::CullDistantActors()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    for (int32 i = SpawnedActors.Num() - 1; i >= 0; --i)
    {
        FWorld_SpawnedActorInfo& ActorInfo = SpawnedActors[i];
        
        if (!ActorInfo.SpawnedActor || !IsValid(ActorInfo.SpawnedActor))
        {
            SpawnedActors.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, ActorInfo.SpawnedActor->GetActorLocation());
        
        if (Distance > CullingDistance)
        {
            ActorInfo.SpawnedActor->Destroy();
            SpawnedActors.RemoveAt(i);
            TotalSpawnedCount--;
        }
    }
}

FVector AWorld_ProceduralBiomeSystem::GetPlayerLocation() const
{
    if (GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            return PC->GetPawn()->GetActorLocation();
        }
    }
    return FVector::ZeroVector;
}

FWorld_BiomeDefinition* AWorld_ProceduralBiomeSystem::GetBiomeAtLocation(const FVector& Location)
{
    for (FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        float Distance = FVector::Dist2D(Location, Biome.CenterLocation);
        if (Distance <= Biome.Radius)
        {
            return &Biome;
        }
    }
    return nullptr;
}

TArray<AActor*> AWorld_ProceduralBiomeSystem::GetActorsInBiome(const FString& BiomeName) const
{
    TArray<AActor*> BiomeActors;
    
    for (const FWorld_SpawnedActorInfo& ActorInfo : SpawnedActors)
    {
        if (ActorInfo.BiomeName == BiomeName && ActorInfo.SpawnedActor && IsValid(ActorInfo.SpawnedActor))
        {
            BiomeActors.Add(ActorInfo.SpawnedActor);
        }
    }
    
    return BiomeActors;
}

void AWorld_ProceduralBiomeSystem::PrintBiomeStatistics() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME STATISTICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Spawned Actors: %d"), TotalSpawnedCount);
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        TArray<AActor*> BiomeActors = GetActorsInBiome(Biome.BiomeName);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d actors"), *Biome.BiomeName, BiomeActors.Num());
    }
}

void AWorld_ProceduralBiomeSystem::ClearAllSpawnedActors()
{
    for (const FWorld_SpawnedActorInfo& ActorInfo : SpawnedActors)
    {
        if (ActorInfo.SpawnedActor && IsValid(ActorInfo.SpawnedActor))
        {
            ActorInfo.SpawnedActor->Destroy();
        }
    }
    
    SpawnedActors.Empty();
    TotalSpawnedCount = 0;
    
    UE_LOG(LogTemp, Warning, TEXT("ProceduralBiomeSystem: All spawned actors cleared"));
}

FVector AWorld_ProceduralBiomeSystem::GenerateRandomLocationInBiome(const FWorld_BiomeDefinition& Biome) const
{
    float Angle = FMath::RandRange(0.0f, 2.0f * PI);
    float Distance = FMath::RandRange(0.0f, Biome.Radius * 0.8f);
    
    float X = Biome.CenterLocation.X + Distance * FMath::Cos(Angle);
    float Y = Biome.CenterLocation.Y + Distance * FMath::Sin(Angle);
    float Z = 100.0f; // Ground level
    
    return FVector(X, Y, Z);
}

bool AWorld_ProceduralBiomeSystem::IsLocationValidForSpawn(const FVector& Location) const
{
    // Basic validation - can be expanded with terrain checks
    return true;
}

void AWorld_ProceduralBiomeSystem::RegisterSpawnedActor(AActor* Actor, const FString& BiomeName, const FString& ActorType, const FVector& Location)
{
    if (!Actor) return;
    
    FWorld_SpawnedActorInfo NewInfo;
    NewInfo.SpawnedActor = Actor;
    NewInfo.BiomeName = BiomeName;
    NewInfo.ActorType = ActorType;
    NewInfo.SpawnLocation = Location;
    NewInfo.DistanceFromPlayer = 0.0f;
    
    SpawnedActors.Add(NewInfo);
    TotalSpawnedCount++;
}

void AWorld_ProceduralBiomeSystem::UpdateActorDistances()
{
    FVector PlayerLocation = GetPlayerLocation();
    
    for (FWorld_SpawnedActorInfo& ActorInfo : SpawnedActors)
    {
        if (ActorInfo.SpawnedActor && IsValid(ActorInfo.SpawnedActor))
        {
            ActorInfo.DistanceFromPlayer = FVector::Dist(PlayerLocation, ActorInfo.SpawnedActor->GetActorLocation());
        }
    }
}
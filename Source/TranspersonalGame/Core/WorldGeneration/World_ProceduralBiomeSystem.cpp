#include "World_ProceduralBiomeSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"

AWorld_ProceduralBiomeSystem::AWorld_ProceduralBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    SetupDefaultBiomeConfigs();
    LoadBiomeAssets();
}

void AWorld_ProceduralBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: BeginPlay - Initializing biome system"));
    
    // Delay initialization to allow world to fully load
    FTimerHandle InitTimer;
    GetWorld()->GetTimerManager().SetTimer(InitTimer, this, &AWorld_ProceduralBiomeSystem::InitializeBiomes, 2.0f, false);
}

void AWorld_ProceduralBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bIsGenerating || bGenerationComplete)
    {
        return;
    }

    // Performance-budgeted generation
    if (bUsePerformanceBudget && GetWorld()->GetTimeSeconds() - LastGenerationTime < 0.1f)
    {
        return;
    }

    // Generate actors in batches
    int32 ActorsSpawnedThisFrame = 0;
    while (ActorsSpawnedThisFrame < MaxActorsPerFrame && CurrentGenerationIndex < BiomeConfigs.Num())
    {
        FWorld_BiomeConfig& CurrentBiome = BiomeConfigs[CurrentGenerationIndex];
        
        // Count existing actors in this biome
        int32 ExistingActors = 0;
        for (AActor* Actor : GeneratedActors)
        {
            if (IsValid(Actor))
            {
                FVector ActorLoc = Actor->GetActorLocation();
                float Distance = FVector::Dist2D(ActorLoc, CurrentBiome.CenterLocation);
                if (Distance <= CurrentBiome.Radius)
                {
                    ExistingActors++;
                }
            }
        }

        if (ExistingActors < CurrentBiome.TargetActorCount)
        {
            PopulateBiomeWithAssets(CurrentBiome);
            ActorsSpawnedThisFrame++;
        }
        else
        {
            CurrentGenerationIndex++;
        }
    }

    if (CurrentGenerationIndex >= BiomeConfigs.Num())
    {
        bGenerationComplete = true;
        bIsGenerating = false;
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: Generation complete! Total actors: %d"), TotalGeneratedActors);
    }

    LastGenerationTime = GetWorld()->GetTimeSeconds();
}

void AWorld_ProceduralBiomeSystem::InitializeBiomes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: Starting biome initialization"));
    
    bIsGenerating = true;
    bGenerationComplete = false;
    CurrentGenerationIndex = 0;
    TotalGeneratedActors = 0;

    // Clear existing generated actors
    for (AActor* Actor : GeneratedActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    GeneratedActors.Empty();

    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: Biome initialization started - %d biomes to generate"), BiomeConfigs.Num());
}

void AWorld_ProceduralBiomeSystem::GenerateBiome(EWorld_BiomeType BiomeType)
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            PopulateBiomeWithAssets(Config);
            break;
        }
    }
}

void AWorld_ProceduralBiomeSystem::PopulateBiomeWithAssets(const FWorld_BiomeConfig& BiomeConfig)
{
    if (!BiomeAssets.Contains(BiomeConfig.BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: No assets found for biome type"));
        return;
    }

    const TArray<FWorld_ProceduralAsset>& Assets = BiomeAssets[BiomeConfig.BiomeType];
    if (Assets.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: Asset array empty for biome"));
        return;
    }

    // Generate random location in biome
    FVector SpawnLocation = GetRandomLocationInBiome(BiomeConfig);
    
    // Select random asset based on weights
    float TotalWeight = 0.0f;
    for (const FWorld_ProceduralAsset& Asset : Assets)
    {
        TotalWeight += Asset.SpawnWeight;
    }

    if (TotalWeight <= 0.0f)
    {
        return;
    }

    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float CurrentWeight = 0.0f;
    
    for (const FWorld_ProceduralAsset& Asset : Assets)
    {
        CurrentWeight += Asset.SpawnWeight;
        if (RandomValue <= CurrentWeight)
        {
            AActor* SpawnedActor = SpawnProceduralAsset(Asset, SpawnLocation, BiomeConfig.BiomeType);
            if (SpawnedActor)
            {
                GeneratedActors.Add(SpawnedActor);
                TotalGeneratedActors++;
            }
            break;
        }
    }
}

FVector AWorld_ProceduralBiomeSystem::GetRandomLocationInBiome(const FWorld_BiomeConfig& BiomeConfig)
{
    // Generate random point within biome radius
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, BiomeConfig.Radius * 0.8f); // Keep within 80% of radius
    
    FVector RandomOffset;
    RandomOffset.X = FMath::Cos(RandomAngle) * RandomDistance;
    RandomOffset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    RandomOffset.Z = 0.0f;

    FVector SpawnLocation = BiomeConfig.CenterLocation + RandomOffset;
    
    // Adjust Z based on terrain height
    SpawnLocation.Z = GetTerrainHeight(SpawnLocation);
    
    return SpawnLocation;
}

AActor* AWorld_ProceduralBiomeSystem::SpawnProceduralAsset(const FWorld_ProceduralAsset& Asset, const FVector& Location, EWorld_BiomeType BiomeType)
{
    if (!Asset.AssetMesh.IsValid())
    {
        // Try to load the asset
        UStaticMesh* LoadedMesh = Asset.AssetMesh.LoadSynchronous();
        if (!LoadedMesh)
        {
            return nullptr;
        }
    }

    // Spawn static mesh actor
    AStaticMeshActor* SpawnedActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (!SpawnedActor)
    {
        return nullptr;
    }

    // Configure the static mesh component
    UStaticMeshComponent* MeshComp = SpawnedActor->GetStaticMeshComponent();
    if (MeshComp)
    {
        MeshComp->SetStaticMesh(Asset.AssetMesh.Get());
        
        // Apply random scale
        FVector Scale = FVector::OneVector;
        Scale.X = FMath::RandRange(Asset.ScaleRange.X, Asset.ScaleRange.Y);
        Scale.Y = Scale.X; // Uniform XY scaling
        Scale.Z = FMath::RandRange(Asset.ScaleRange.X * Asset.ScaleRange.Z, Asset.ScaleRange.Y * Asset.ScaleRange.Z);
        SpawnedActor->SetActorScale3D(Scale);
        
        // Apply random rotation if enabled
        if (Asset.bRandomRotation)
        {
            FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
            SpawnedActor->SetActorRotation(RandomRotation);
        }
    }

    // Set actor label for debugging
    FString BiomeName = UEnum::GetValueAsString(BiomeType);
    FString ActorLabel = FString::Printf(TEXT("%s_Asset_%d"), *BiomeName, TotalGeneratedActors);
    SpawnedActor->SetActorLabel(ActorLabel);

    return SpawnedActor;
}

void AWorld_ProceduralBiomeSystem::ClearBiome(EWorld_BiomeType BiomeType)
{
    for (int32 i = GeneratedActors.Num() - 1; i >= 0; i--)
    {
        AActor* Actor = GeneratedActors[i];
        if (IsValid(Actor))
        {
            // Check if actor is in the specified biome
            for (const FWorld_BiomeConfig& Config : BiomeConfigs)
            {
                if (Config.BiomeType == BiomeType)
                {
                    float Distance = FVector::Dist2D(Actor->GetActorLocation(), Config.CenterLocation);
                    if (Distance <= Config.Radius)
                    {
                        Actor->Destroy();
                        GeneratedActors.RemoveAt(i);
                        TotalGeneratedActors--;
                        break;
                    }
                }
            }
        }
        else
        {
            GeneratedActors.RemoveAt(i);
        }
    }
}

void AWorld_ProceduralBiomeSystem::SetPerformanceBudget(int32 MaxActors, float UpdateRadius)
{
    MaxActorsPerFrame = FMath::Clamp(MaxActors, 1, 50);
    GenerationRadius = FMath::Clamp(UpdateRadius, 10000.0f, 100000.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralBiomeSystem: Performance budget updated - MaxActors: %d, Radius: %.0f"), 
           MaxActorsPerFrame, GenerationRadius);
}

void AWorld_ProceduralBiomeSystem::DebugBiomeStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME SYSTEM STATUS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Generated Actors: %d"), TotalGeneratedActors);
    UE_LOG(LogTemp, Warning, TEXT("Generation Complete: %s"), bGenerationComplete ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("Currently Generating: %s"), bIsGenerating ? TEXT("YES") : TEXT("NO"));
    
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        int32 BiomeActorCount = 0;
        for (AActor* Actor : GeneratedActors)
        {
            if (IsValid(Actor))
            {
                float Distance = FVector::Dist2D(Actor->GetActorLocation(), Config.CenterLocation);
                if (Distance <= Config.Radius)
                {
                    BiomeActorCount++;
                }
            }
        }
        
        FString BiomeName = UEnum::GetValueAsString(Config.BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s: %d/%d actors (%.1f%% complete)"), 
               *BiomeName, BiomeActorCount, Config.TargetActorCount, 
               (float)BiomeActorCount / Config.TargetActorCount * 100.0f);
    }
}

void AWorld_ProceduralBiomeSystem::SetupDefaultBiomeConfigs()
{
    BiomeConfigs.Empty();

    // Savana biome
    FWorld_BiomeConfig SavanaConfig;
    SavanaConfig.BiomeType = EWorld_BiomeType::Savana;
    SavanaConfig.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaConfig.Radius = 10000.0f;
    SavanaConfig.TargetActorCount = 500;
    SavanaConfig.VegetationDensity = 0.4f;
    SavanaConfig.RockDensity = 0.2f;
    SavanaConfig.TerrainRoughness = 0.3f;
    BiomeConfigs.Add(SavanaConfig);

    // Pantano biome
    FWorld_BiomeConfig PantanoConfig;
    PantanoConfig.BiomeType = EWorld_BiomeType::Pantano;
    PantanoConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoConfig.Radius = 10000.0f;
    PantanoConfig.TargetActorCount = 500;
    PantanoConfig.VegetationDensity = 0.9f;
    PantanoConfig.RockDensity = 0.1f;
    PantanoConfig.TerrainRoughness = 0.2f;
    BiomeConfigs.Add(PantanoConfig);

    // Floresta biome
    FWorld_BiomeConfig FlorestaConfig;
    FlorestaConfig.BiomeType = EWorld_BiomeType::Floresta;
    FlorestaConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaConfig.Radius = 10000.0f;
    FlorestaConfig.TargetActorCount = 500;
    FlorestaConfig.VegetationDensity = 0.8f;
    FlorestaConfig.RockDensity = 0.3f;
    FlorestaConfig.TerrainRoughness = 0.4f;
    BiomeConfigs.Add(FlorestaConfig);

    // Deserto biome
    FWorld_BiomeConfig DesertoConfig;
    DesertoConfig.BiomeType = EWorld_BiomeType::Deserto;
    DesertoConfig.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoConfig.Radius = 10000.0f;
    DesertoConfig.TargetActorCount = 500;
    DesertoConfig.VegetationDensity = 0.1f;
    DesertoConfig.RockDensity = 0.6f;
    DesertoConfig.TerrainRoughness = 0.7f;
    BiomeConfigs.Add(DesertoConfig);

    // Montanha biome
    FWorld_BiomeConfig MontanhaConfig;
    MontanhaConfig.BiomeType = EWorld_BiomeType::Montanha;
    MontanhaConfig.CenterLocation = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaConfig.Radius = 10000.0f;
    MontanhaConfig.TargetActorCount = 500;
    MontanhaConfig.VegetationDensity = 0.3f;
    MontanhaConfig.RockDensity = 0.8f;
    MontanhaConfig.TerrainRoughness = 0.9f;
    BiomeConfigs.Add(MontanhaConfig);
}

void AWorld_ProceduralBiomeSystem::LoadBiomeAssets()
{
    BiomeAssets.Empty();

    // Create default procedural assets for each biome
    // These will be replaced with real assets when available
    
    for (EWorld_BiomeType BiomeType : TEnumRange<EWorld_BiomeType>())
    {
        TArray<FWorld_ProceduralAsset> Assets;
        
        // Create placeholder asset entries
        FWorld_ProceduralAsset DefaultAsset;
        DefaultAsset.SpawnWeight = 1.0f;
        DefaultAsset.ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        DefaultAsset.bRandomRotation = true;
        
        Assets.Add(DefaultAsset);
        BiomeAssets.Add(BiomeType, Assets);
    }
}

bool AWorld_ProceduralBiomeSystem::IsLocationValid(const FVector& Location, const FWorld_BiomeConfig& BiomeConfig)
{
    // Check if location is within biome radius
    float Distance = FVector::Dist2D(Location, BiomeConfig.CenterLocation);
    if (Distance > BiomeConfig.Radius)
    {
        return false;
    }

    // Check for overlaps with existing actors
    for (AActor* ExistingActor : GeneratedActors)
    {
        if (IsValid(ExistingActor))
        {
            float ExistingDistance = FVector::Dist(Location, ExistingActor->GetActorLocation());
            if (ExistingDistance < 500.0f) // Minimum 5m spacing
            {
                return false;
            }
        }
    }

    return true;
}

float AWorld_ProceduralBiomeSystem::GetTerrainHeight(const FVector& Location)
{
    // Simple terrain height calculation
    // This should be replaced with actual landscape height sampling
    float BaseHeight = 100.0f;
    float NoiseHeight = FMath::PerlinNoise2D(FVector2D(Location.X * 0.0001f, Location.Y * 0.0001f)) * 200.0f;
    return BaseHeight + NoiseHeight;
}
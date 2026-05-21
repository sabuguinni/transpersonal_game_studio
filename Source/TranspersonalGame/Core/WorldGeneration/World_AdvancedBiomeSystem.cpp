#include "World_AdvancedBiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"
#include "UObject/ConstructorHelpers.h"

AWorld_AdvancedBiomeSystem::AWorld_AdvancedBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Performance defaults
    MaxActorsPerFrame = 50;
    SpawnDelay = 0.1f;
    bIsGenerating = false;
    CurrentBiomeIndex = 0;

    CreateDefaultBiomeConfigurations();
}

void AWorld_AdvancedBiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: BeginPlay - Ready for biome generation"));
}

void AWorld_AdvancedBiomeSystem::CreateDefaultBiomeConfigurations()
{
    BiomeConfigurations.Empty();

    // Savana Biome (Center of world)
    FWorld_BiomeConfiguration SavanaConfig;
    SavanaConfig.BiomeType = EBiomeType::Savana;
    SavanaConfig.CenterLocation = FVector(0.0f, 0.0f, 100.0f);
    SavanaConfig.BiomeRadius = 25000.0f;
    SavanaConfig.VegetationDensity = 300;
    SavanaConfig.RockDensity = 150;
    SavanaConfig.DinosaurCount = 20;
    SavanaConfig.TemperatureRange = 35.0f;
    SavanaConfig.HumidityLevel = 0.3f;
    SavanaConfig.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Trex/Mesh/SKM_Trex_Skin"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin"),
        TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops")
    };
    BiomeConfigurations.Add(SavanaConfig);

    // Forest Biome
    FWorld_BiomeConfiguration ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Floresta;
    ForestConfig.CenterLocation = FVector(-45000.0f, 40000.0f, 100.0f);
    ForestConfig.BiomeRadius = 30000.0f;
    ForestConfig.VegetationDensity = 800;
    ForestConfig.RockDensity = 100;
    ForestConfig.DinosaurCount = 15;
    ForestConfig.TemperatureRange = 25.0f;
    ForestConfig.HumidityLevel = 0.8f;
    ForestConfig.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus"),
        TEXT("/Game/Dinosaur_Pack/Parasaurolophus/Mesh/SKM_Parasaurolophus_Mesh"),
        TEXT("/Game/Dinosaur_Pack/Protoceratops/Mesh/SKM_Protoceratops_Skin")
    };
    BiomeConfigurations.Add(ForestConfig);

    // Desert Biome
    FWorld_BiomeConfiguration DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Deserto;
    DesertConfig.CenterLocation = FVector(55000.0f, 0.0f, 100.0f);
    DesertConfig.BiomeRadius = 28000.0f;
    DesertConfig.VegetationDensity = 100;
    DesertConfig.RockDensity = 300;
    DesertConfig.DinosaurCount = 12;
    DesertConfig.TemperatureRange = 45.0f;
    DesertConfig.HumidityLevel = 0.1f;
    DesertConfig.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh"),
        TEXT("/Game/Dinosaur_Pack/Pachycephalo/Mesh/SKM_Pachycephalo")
    };
    BiomeConfigurations.Add(DesertConfig);

    // Swamp Biome
    FWorld_BiomeConfiguration SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Pantano;
    SwampConfig.CenterLocation = FVector(-50000.0f, -45000.0f, 100.0f);
    SwampConfig.BiomeRadius = 22000.0f;
    SwampConfig.VegetationDensity = 600;
    SwampConfig.RockDensity = 80;
    SwampConfig.DinosaurCount = 18;
    SwampConfig.TemperatureRange = 28.0f;
    SwampConfig.HumidityLevel = 0.9f;
    SwampConfig.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Tsintaosaurus/Mesh/SKM_Tsintaosaurus_Mesh"),
        TEXT("/Game/Dinosaur_Pack/Velociraptor/Mesh/SKM_Velociraptor_Skin")
    };
    BiomeConfigurations.Add(SwampConfig);

    // Mountain Biome
    FWorld_BiomeConfiguration MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Montanha;
    MountainConfig.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    MountainConfig.BiomeRadius = 20000.0f;
    MountainConfig.VegetationDensity = 200;
    MountainConfig.RockDensity = 500;
    MountainConfig.DinosaurCount = 8;
    MountainConfig.TemperatureRange = 15.0f;
    MountainConfig.HumidityLevel = 0.4f;
    MountainConfig.DinosaurAssets = {
        TEXT("/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops")
    };
    BiomeConfigurations.Add(MountainConfig);

    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Created %d default biome configurations"), BiomeConfigurations.Num());
}

void AWorld_AdvancedBiomeSystem::InitializeAllBiomes()
{
    if (bIsGenerating)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Already generating biomes"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Starting generation of %d biomes"), BiomeConfigurations.Num());
    
    bIsGenerating = true;
    CurrentBiomeIndex = 0;
    SpawnResults.Empty();

    // Start generation timer
    GetWorldTimerManager().SetTimer(GenerationTimerHandle, this, &AWorld_AdvancedBiomeSystem::ContinueGeneration, SpawnDelay, true);
}

void AWorld_AdvancedBiomeSystem::ContinueGeneration()
{
    if (CurrentBiomeIndex >= BiomeConfigurations.Num())
    {
        // Generation complete
        bIsGenerating = false;
        GetWorldTimerManager().ClearTimer(GenerationTimerHandle);
        
        UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: All biomes generated successfully"));
        LogBiomeStatus();
        return;
    }

    // Generate current biome
    GenerateBiome(BiomeConfigurations[CurrentBiomeIndex]);
    CurrentBiomeIndex++;
}

void AWorld_AdvancedBiomeSystem::GenerateBiome(const FWorld_BiomeConfiguration& Config)
{
    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Generating biome at (%f, %f, %f)"), 
           Config.CenterLocation.X, Config.CenterLocation.Y, Config.CenterLocation.Z);

    FWorld_SpawnResult Result = PopulateBiomeArea(Config);
    SpawnResults.Add(Result);

    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Biome complete - Vegetation: %d, Rocks: %d, Dinosaurs: %d"), 
           Result.VegetationSpawned, Result.RocksSpawned, Result.DinosaursSpawned);
}

FWorld_SpawnResult AWorld_AdvancedBiomeSystem::PopulateBiomeArea(const FWorld_BiomeConfiguration& Config)
{
    FWorld_SpawnResult Result;
    double StartTime = FPlatformTime::Seconds();

    // Spawn vegetation
    SpawnVegetationInArea(Config.CenterLocation, Config.BiomeRadius, Config.VegetationAssets, Config.VegetationDensity);
    Result.VegetationSpawned = Config.VegetationDensity;

    // Spawn rocks
    SpawnRocksInArea(Config.CenterLocation, Config.BiomeRadius, Config.RockAssets, Config.RockDensity);
    Result.RocksSpawned = Config.RockDensity;

    // Spawn dinosaurs
    SpawnDinosaursInArea(Config.CenterLocation, Config.BiomeRadius, Config.DinosaurAssets, Config.DinosaurCount);
    Result.DinosaursSpawned = Config.DinosaurCount;

    Result.ExecutionTime = FPlatformTime::Seconds() - StartTime;
    Result.bSuccess = true;

    return Result;
}

void AWorld_AdvancedBiomeSystem::SpawnVegetationInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count)
{
    if (Assets.Num() == 0) return;

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Center, Radius);
        if (!IsLocationValid(SpawnLocation)) continue;

        FString AssetPath = Assets[FMath::RandRange(0, Assets.Num() - 1)];
        FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        AActor* SpawnedActor = SpawnActorFromAssetPath(AssetPath, SpawnLocation, RandomRotation);
        if (SpawnedActor)
        {
            SpawnedActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%d"), *UEnum::GetValueAsString(EBiomeType::Savana), i));
        }
    }
}

void AWorld_AdvancedBiomeSystem::SpawnRocksInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count)
{
    if (Assets.Num() == 0) return;

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Center, Radius);
        if (!IsLocationValid(SpawnLocation)) continue;

        FString AssetPath = Assets[FMath::RandRange(0, Assets.Num() - 1)];
        FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        AActor* SpawnedActor = SpawnActorFromAssetPath(AssetPath, SpawnLocation, RandomRotation);
        if (SpawnedActor)
        {
            SpawnedActor->SetActorLabel(FString::Printf(TEXT("Rock_%d"), i));
        }
    }
}

void AWorld_AdvancedBiomeSystem::SpawnDinosaursInArea(const FVector& Center, float Radius, const TArray<FString>& Assets, int32 Count)
{
    if (Assets.Num() == 0) return;

    for (int32 i = 0; i < Count; i++)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Center, Radius);
        SpawnLocation.Z += 50.0f; // Elevate dinosaurs slightly
        
        if (!IsLocationValid(SpawnLocation)) continue;

        FString AssetPath = Assets[FMath::RandRange(0, Assets.Num() - 1)];
        FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        AActor* SpawnedActor = SpawnActorFromAssetPath(AssetPath, SpawnLocation, RandomRotation);
        if (SpawnedActor)
        {
            SpawnedActor->SetActorLabel(FString::Printf(TEXT("Dinosaur_%d"), i));
        }
    }
}

FVector AWorld_AdvancedBiomeSystem::GetRandomLocationInRadius(const FVector& Center, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset;
    Offset.X = FMath::Cos(RandomAngle) * RandomDistance;
    Offset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    Offset.Z = 0.0f;
    
    return Center + Offset;
}

bool AWorld_AdvancedBiomeSystem::IsLocationValid(const FVector& Location)
{
    // Basic validation - could be expanded with terrain height checks
    return true;
}

AActor* AWorld_AdvancedBiomeSystem::SpawnActorFromAssetPath(const FString& AssetPath, const FVector& Location, const FRotator& Rotation)
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    // Try to load the asset
    UObject* LoadedAsset = LoadObject<UObject>(nullptr, *AssetPath);
    if (!LoadedAsset)
    {
        UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Failed to load asset: %s"), *AssetPath);
        return nullptr;
    }

    // Check if it's a static mesh
    UStaticMesh* StaticMesh = Cast<UStaticMesh>(LoadedAsset);
    if (StaticMesh)
    {
        AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (MeshActor && MeshActor->GetStaticMeshComponent())
        {
            MeshActor->GetStaticMeshComponent()->SetStaticMesh(StaticMesh);
            return MeshActor;
        }
    }

    return nullptr;
}

void AWorld_AdvancedBiomeSystem::SetupDefaultBiomes()
{
    CreateDefaultBiomeConfigurations();
    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Default biomes setup complete"));
}

void AWorld_AdvancedBiomeSystem::ClearAllBiomeActors()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> ActorsToDestroy;
    
    // Find all actors with biome-related labels
    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (Actor && (Actor->GetActorLabel().Contains(TEXT("Vegetation_")) || 
                     Actor->GetActorLabel().Contains(TEXT("Rock_")) || 
                     Actor->GetActorLabel().Contains(TEXT("Dinosaur_"))))
        {
            ActorsToDestroy.Add(Actor);
        }
    }

    for (AActor* Actor : ActorsToDestroy)
    {
        Actor->Destroy();
    }

    UE_LOG(LogTemp, Warning, TEXT("AdvancedBiomeSystem: Cleared %d biome actors"), ActorsToDestroy.Num());
}

void AWorld_AdvancedBiomeSystem::LogBiomeStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== BIOME GENERATION SUMMARY ==="));
    
    int32 TotalVegetation = 0;
    int32 TotalRocks = 0;
    int32 TotalDinosaurs = 0;
    float TotalTime = 0.0f;

    for (int32 i = 0; i < SpawnResults.Num(); i++)
    {
        const FWorld_SpawnResult& Result = SpawnResults[i];
        TotalVegetation += Result.VegetationSpawned;
        TotalRocks += Result.RocksSpawned;
        TotalDinosaurs += Result.DinosaursSpawned;
        TotalTime += Result.ExecutionTime;

        UE_LOG(LogTemp, Warning, TEXT("Biome %d: Veg=%d, Rocks=%d, Dinos=%d, Time=%.2fs"), 
               i, Result.VegetationSpawned, Result.RocksSpawned, Result.DinosaursSpawned, Result.ExecutionTime);
    }

    UE_LOG(LogTemp, Warning, TEXT("TOTALS: Vegetation=%d, Rocks=%d, Dinosaurs=%d, Time=%.2fs"), 
           TotalVegetation, TotalRocks, TotalDinosaurs, TotalTime);
    UE_LOG(LogTemp, Warning, TEXT("=== END BIOME SUMMARY ==="));
}
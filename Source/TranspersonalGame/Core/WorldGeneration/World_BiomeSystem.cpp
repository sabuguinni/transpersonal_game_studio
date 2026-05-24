#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"

AWorld_BiomeSystem::AWorld_BiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default settings
    WorldSize = 20000.0f;
    BiomeTransitionSmoothness = 5;
    VegetationSpawnRadius = 1000.0f;
    MaxVegetationPerBiome = 50;
    bAutoSpawnVegetation = true;
    bBiomesInitialized = false;

    // Setup default biomes
    SetupDefaultBiomes();
}

void AWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    if (!bBiomesInitialized)
    {
        InitializeBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: BeginPlay completed"));
}

void AWorld_BiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Optional: Update biome effects over time
    // Currently passive system, but could add dynamic weather, seasonal changes, etc.
}

void AWorld_BiomeSystem::SetupDefaultBiomes()
{
    BiomeConfigs.Empty();

    // Swamp Biome
    FWorld_BiomeConfig SwampConfig;
    SwampConfig.BiomeType = EWorld_BiomeType::Swamp;
    SwampConfig.AmbientColor = FLinearColor(0.4f, 0.6f, 0.3f, 1.0f); // Greenish tint
    SwampConfig.Temperature = 25.0f;
    SwampConfig.Humidity = 0.9f;
    SwampConfig.VegetationDensity = 1.5f;
    BiomeConfigs.Add(SwampConfig);

    // Forest Biome
    FWorld_BiomeConfig ForestConfig;
    ForestConfig.BiomeType = EWorld_BiomeType::Forest;
    ForestConfig.AmbientColor = FLinearColor(0.6f, 0.8f, 0.4f, 1.0f); // Green tint
    ForestConfig.Temperature = 18.0f;
    ForestConfig.Humidity = 0.7f;
    ForestConfig.VegetationDensity = 2.0f;
    BiomeConfigs.Add(ForestConfig);

    // Savanna Biome
    FWorld_BiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EWorld_BiomeType::Savanna;
    SavannaConfig.AmbientColor = FLinearColor(1.0f, 0.9f, 0.6f, 1.0f); // Golden tint
    SavannaConfig.Temperature = 28.0f;
    SavannaConfig.Humidity = 0.4f;
    SavannaConfig.VegetationDensity = 0.6f;
    BiomeConfigs.Add(SavannaConfig);

    // Desert Biome
    FWorld_BiomeConfig DesertConfig;
    DesertConfig.BiomeType = EWorld_BiomeType::Desert;
    DesertConfig.AmbientColor = FLinearColor(1.0f, 0.8f, 0.5f, 1.0f); // Sandy tint
    DesertConfig.Temperature = 35.0f;
    DesertConfig.Humidity = 0.1f;
    DesertConfig.VegetationDensity = 0.2f;
    BiomeConfigs.Add(DesertConfig);

    // Snowy Mountain Biome
    FWorld_BiomeConfig MountainConfig;
    MountainConfig.BiomeType = EWorld_BiomeType::SnowyMountain;
    MountainConfig.AmbientColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f); // Bluish tint
    MountainConfig.Temperature = -5.0f;
    MountainConfig.Humidity = 0.3f;
    MountainConfig.VegetationDensity = 0.3f;
    BiomeConfigs.Add(MountainConfig);

    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Setup %d default biome configurations"), BiomeConfigs.Num());
}

void AWorld_BiomeSystem::InitializeBiomes()
{
    if (bBiomesInitialized)
    {
        return;
    }

    // Create default biome zones if none exist
    if (BiomeZones.Num() == 0)
    {
        // Central Forest
        CreateBiomeZone(FVector(0, 0, 0), 3000.0f, EWorld_BiomeType::Forest);
        
        // Swamp to the north
        CreateBiomeZone(FVector(0, 8000, 0), 4000.0f, EWorld_BiomeType::Swamp);
        
        // Savanna to the south
        CreateBiomeZone(FVector(0, -8000, 0), 4000.0f, EWorld_BiomeType::Savanna);
        
        // Desert to the east
        CreateBiomeZone(FVector(8000, 0, 0), 3500.0f, EWorld_BiomeType::Desert);
        
        // Snowy mountains to the west
        CreateBiomeZone(FVector(-8000, 0, 0), 3500.0f, EWorld_BiomeType::SnowyMountain);
    }

    // Spawn initial vegetation if enabled
    if (bAutoSpawnVegetation)
    {
        for (const FWorld_BiomeZone& Zone : BiomeZones)
        {
            SpawnVegetationInBiome(Zone.BiomeType, Zone.Center, Zone.Radius * 0.8f);
        }
    }

    bBiomesInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Biomes initialized with %d zones"), BiomeZones.Num());
}

EWorld_BiomeType AWorld_BiomeSystem::GetBiomeAtLocation(const FVector& Location) const
{
    if (BiomeZones.Num() == 0)
    {
        return EWorld_BiomeType::Forest; // Default fallback
    }

    float MaxInfluence = 0.0f;
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::Forest;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Influence = CalculateBiomeInfluence(Location, Zone);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Zone.BiomeType;
        }
    }

    return DominantBiome;
}

FWorld_BiomeConfig AWorld_BiomeSystem::GetBiomeConfig(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeConfig& Config : BiomeConfigs)
    {
        if (Config.BiomeType == BiomeType)
        {
            return Config;
        }
    }

    // Return default forest config if not found
    FWorld_BiomeConfig DefaultConfig;
    DefaultConfig.BiomeType = EWorld_BiomeType::Forest;
    return DefaultConfig;
}

void AWorld_BiomeSystem::SpawnVegetationInBiome(EWorld_BiomeType BiomeType, const FVector& Center, float Radius)
{
    FWorld_BiomeConfig Config = GetBiomeConfig(BiomeType);
    
    int32 VegetationCount = FMath::RoundToInt(MaxVegetationPerBiome * Config.VegetationDensity);
    
    for (int32 i = 0; i < VegetationCount; i++)
    {
        FVector SpawnLocation = GetRandomLocationInRadius(Center, Radius);
        FRotator SpawnRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
        
        // For now, spawn basic cube meshes as vegetation placeholders
        // In production, this would use the Config.VegetationMeshes array
        UWorld* World = GetWorld();
        if (World)
        {
            AStaticMeshActor* VegetationActor = World->SpawnActor<AStaticMeshActor>(SpawnLocation, SpawnRotation);
            if (VegetationActor)
            {
                // Set a basic cube mesh as placeholder
                UStaticMeshComponent* MeshComp = VegetationActor->GetStaticMeshComponent();
                if (MeshComp)
                {
                    // Load basic cube mesh
                    UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
                    if (CubeMesh)
                    {
                        MeshComp->SetStaticMesh(CubeMesh);
                        
                        // Scale based on biome type
                        FVector Scale = FVector(1.0f);
                        switch (BiomeType)
                        {
                            case EWorld_BiomeType::Forest:
                                Scale = FVector(0.5f, 0.5f, 2.0f); // Tall trees
                                break;
                            case EWorld_BiomeType::Swamp:
                                Scale = FVector(0.3f, 0.3f, 1.5f); // Swamp trees
                                break;
                            case EWorld_BiomeType::Savanna:
                                Scale = FVector(0.2f, 0.2f, 0.5f); // Grass/bushes
                                break;
                            case EWorld_BiomeType::Desert:
                                Scale = FVector(0.1f, 0.1f, 1.0f); // Cacti
                                break;
                            case EWorld_BiomeType::SnowyMountain:
                                Scale = FVector(0.4f, 0.4f, 1.8f); // Pine trees
                                break;
                        }
                        VegetationActor->SetActorScale3D(Scale);
                        
                        // Set name for identification
                        FString BiomeName = UEnum::GetValueAsString(BiomeType);
                        VegetationActor->SetActorLabel(FString::Printf(TEXT("%s_Vegetation_%d"), *BiomeName, i));
                        
                        SpawnedVegetation.Add(VegetationActor);
                    }
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Spawned %d vegetation items for %s biome"), 
           VegetationCount, *UEnum::GetValueAsString(BiomeType));
}

void AWorld_BiomeSystem::CreateBiomeZone(const FVector& Center, float Radius, EWorld_BiomeType BiomeType)
{
    FWorld_BiomeZone NewZone;
    NewZone.Center = Center;
    NewZone.Radius = Radius;
    NewZone.BiomeType = BiomeType;
    NewZone.Influence = 1.0f;
    
    BiomeZones.Add(NewZone);
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Created biome zone %s at %s with radius %f"), 
           *UEnum::GetValueAsString(BiomeType), *Center.ToString(), Radius);
}

void AWorld_BiomeSystem::RegenerateBiomes()
{
    ClearAllVegetation();
    bBiomesInitialized = false;
    InitializeBiomes();
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Biomes regenerated"));
}

void AWorld_BiomeSystem::ClearAllVegetation()
{
    for (AActor* VegActor : SpawnedVegetation)
    {
        if (IsValid(VegActor))
        {
            VegActor->Destroy();
        }
    }
    SpawnedVegetation.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("World_BiomeSystem: Cleared all vegetation"));
}

float AWorld_BiomeSystem::GetTemperatureAtLocation(const FVector& Location) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FWorld_BiomeConfig Config = GetBiomeConfig(BiomeType);
    return Config.Temperature;
}

float AWorld_BiomeSystem::GetHumidityAtLocation(const FVector& Location) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FWorld_BiomeConfig Config = GetBiomeConfig(BiomeType);
    return Config.Humidity;
}

FLinearColor AWorld_BiomeSystem::GetAmbientColorAtLocation(const FVector& Location) const
{
    EWorld_BiomeType BiomeType = GetBiomeAtLocation(Location);
    FWorld_BiomeConfig Config = GetBiomeConfig(BiomeType);
    return Config.AmbientColor;
}

FVector AWorld_BiomeSystem::GetRandomLocationInRadius(const FVector& Center, float Radius) const
{
    float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
    float RandomDistance = FMath::RandRange(0.0f, Radius);
    
    FVector Offset;
    Offset.X = FMath::Cos(RandomAngle) * RandomDistance;
    Offset.Y = FMath::Sin(RandomAngle) * RandomDistance;
    Offset.Z = 0.0f; // Keep on ground level
    
    return Center + Offset;
}

float AWorld_BiomeSystem::CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeZone& Zone) const
{
    float Distance = FVector::Dist2D(Location, Zone.Center);
    
    if (Distance >= Zone.Radius)
    {
        return 0.0f;
    }
    
    // Linear falloff from center to edge
    float NormalizedDistance = Distance / Zone.Radius;
    float Influence = (1.0f - NormalizedDistance) * Zone.Influence;
    
    return FMath::Clamp(Influence, 0.0f, 1.0f);
}
#include "VegetationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "TranspersonalGame/SharedTypes.h"

DEFINE_LOG_CATEGORY(LogVegetationSystem);

UVegetationSystem::UVegetationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Tick every 5 seconds for performance
    
    // Initialize default vegetation parameters
    VegetationDensity = 0.5f;
    MaxVegetationDistance = 50000.0f;
    BiomeBlendRadius = 5000.0f;
    
    // Initialize biome vegetation configs
    InitializeBiomeConfigs();
}

void UVegetationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogVegetationSystem, Log, TEXT("VegetationSystem: BeginPlay started"));
    
    // Initialize vegetation placement
    if (GetWorld())
    {
        GenerateInitialVegetation();
    }
}

void UVegetationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Performance-aware vegetation streaming
    UpdateVegetationLOD();
}

void UVegetationSystem::InitializeBiomeConfigs()
{
    // PANTANO (Swamp) - Dense, wet vegetation
    FVegetationBiomeConfig SwampConfig;
    SwampConfig.BiomeType = EBiomeType::Swamp;
    SwampConfig.TreeDensity = 0.8f;
    SwampConfig.UndergrowthDensity = 0.9f;
    SwampConfig.GrassDensity = 0.6f;
    SwampConfig.MaxTreeHeight = 2500.0f;
    SwampConfig.MinTreeHeight = 800.0f;
    SwampConfig.TreeScaleVariation = 0.3f;
    BiomeConfigs.Add(EBiomeType::Swamp, SwampConfig);
    
    // FLORESTA (Forest) - Tall, dense trees
    FVegetationBiomeConfig ForestConfig;
    ForestConfig.BiomeType = EBiomeType::Forest;
    ForestConfig.TreeDensity = 0.7f;
    ForestConfig.UndergrowthDensity = 0.8f;
    ForestConfig.GrassDensity = 0.5f;
    ForestConfig.MaxTreeHeight = 4000.0f;
    ForestConfig.MinTreeHeight = 1500.0f;
    ForestConfig.TreeScaleVariation = 0.4f;
    BiomeConfigs.Add(EBiomeType::Forest, ForestConfig);
    
    // SAVANA (Savanna) - Sparse trees, dense grass
    FVegetationBiomeConfig SavannaConfig;
    SavannaConfig.BiomeType = EBiomeType::Savanna;
    SavannaConfig.TreeDensity = 0.3f;
    SavannaConfig.UndergrowthDensity = 0.4f;
    SavannaConfig.GrassDensity = 0.8f;
    SavannaConfig.MaxTreeHeight = 3000.0f;
    SavannaConfig.MinTreeHeight = 1200.0f;
    SavannaConfig.TreeScaleVariation = 0.2f;
    BiomeConfigs.Add(EBiomeType::Savanna, SavannaConfig);
    
    // DESERTO (Desert) - Very sparse vegetation
    FVegetationBiomeConfig DesertConfig;
    DesertConfig.BiomeType = EBiomeType::Desert;
    DesertConfig.TreeDensity = 0.1f;
    DesertConfig.UndergrowthDensity = 0.2f;
    DesertConfig.GrassDensity = 0.1f;
    DesertConfig.MaxTreeHeight = 1500.0f;
    DesertConfig.MinTreeHeight = 500.0f;
    DesertConfig.TreeScaleVariation = 0.5f;
    BiomeConfigs.Add(EBiomeType::Desert, DesertConfig);
    
    // MONTANHA (Mountain) - Hardy, low vegetation
    FVegetationBiomeConfig MountainConfig;
    MountainConfig.BiomeType = EBiomeType::Mountain;
    MountainConfig.TreeDensity = 0.4f;
    MountainConfig.UndergrowthDensity = 0.3f;
    MountainConfig.GrassDensity = 0.2f;
    MountainConfig.MaxTreeHeight = 2000.0f;
    MountainConfig.MinTreeHeight = 600.0f;
    MountainConfig.TreeScaleVariation = 0.3f;
    BiomeConfigs.Add(EBiomeType::Mountain, MountainConfig);
}

void UVegetationSystem::GenerateInitialVegetation()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogVegetationSystem, Error, TEXT("VegetationSystem: No valid world found"));
        return;
    }
    
    UE_LOG(LogVegetationSystem, Log, TEXT("VegetationSystem: Generating initial vegetation"));
    
    // Generate vegetation for each biome zone
    GenerateVegetationForBiome(EBiomeType::Swamp, FVector(-50000, -45000, 0), 25000.0f);
    GenerateVegetationForBiome(EBiomeType::Forest, FVector(-45000, 40000, 0), 30000.0f);
    GenerateVegetationForBiome(EBiomeType::Savanna, FVector(0, 0, 0), 20000.0f);
    GenerateVegetationForBiome(EBiomeType::Desert, FVector(55000, 0, 0), 25000.0f);
    GenerateVegetationForBiome(EBiomeType::Mountain, FVector(40000, 50000, 500), 30000.0f);
}

void UVegetationSystem::GenerateVegetationForBiome(EBiomeType BiomeType, const FVector& BiomeCenter, float BiomeRadius)
{
    const FVegetationBiomeConfig* Config = BiomeConfigs.Find(BiomeType);
    if (!Config)
    {
        UE_LOG(LogVegetationSystem, Warning, TEXT("VegetationSystem: No config found for biome type %d"), (int32)BiomeType);
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Calculate vegetation count based on biome size and density
    int32 TreeCount = FMath::RoundToInt(Config->TreeDensity * BiomeRadius * 0.01f);
    int32 UndergrowthCount = FMath::RoundToInt(Config->UndergrowthDensity * BiomeRadius * 0.02f);
    int32 GrassCount = FMath::RoundToInt(Config->GrassDensity * BiomeRadius * 0.05f);
    
    UE_LOG(LogVegetationSystem, Log, TEXT("VegetationSystem: Generating %d trees, %d undergrowth, %d grass for biome %d"), 
           TreeCount, UndergrowthCount, GrassCount, (int32)BiomeType);
    
    // Generate trees
    for (int32 i = 0; i < TreeCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnVegetationActor(EVegetationType::Tree, SpawnLocation, *Config);
    }
    
    // Generate undergrowth
    for (int32 i = 0; i < UndergrowthCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnVegetationActor(EVegetationType::Undergrowth, SpawnLocation, *Config);
    }
    
    // Generate grass patches
    for (int32 i = 0; i < GrassCount; i++)
    {
        FVector RandomOffset = FVector(
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            FMath::RandRange(-BiomeRadius, BiomeRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnVegetationActor(EVegetationType::Grass, SpawnLocation, *Config);
    }
}

void UVegetationSystem::SpawnVegetationActor(EVegetationType VegType, const FVector& Location, const FVegetationBiomeConfig& Config)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create vegetation actor
    AActor* VegActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
    if (!VegActor)
    {
        return;
    }
    
    // Add static mesh component
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(VegActor);
    VegActor->SetRootComponent(MeshComp);
    
    // Set random scale based on vegetation type and biome config
    float Scale = 1.0f;
    if (VegType == EVegetationType::Tree)
    {
        float HeightRange = Config.MaxTreeHeight - Config.MinTreeHeight;
        float RandomHeight = Config.MinTreeHeight + FMath::RandRange(0.0f, HeightRange);
        Scale = RandomHeight / 100.0f; // Normalize to reasonable scale
        
        // Apply scale variation
        Scale *= FMath::RandRange(1.0f - Config.TreeScaleVariation, 1.0f + Config.TreeScaleVariation);
    }
    else if (VegType == EVegetationType::Undergrowth)
    {
        Scale = FMath::RandRange(0.5f, 1.5f);
    }
    else if (VegType == EVegetationType::Grass)
    {
        Scale = FMath::RandRange(0.3f, 0.8f);
    }
    
    VegActor->SetActorScale3D(FVector(Scale));
    
    // Set random rotation
    FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    VegActor->SetActorRotation(RandomRotation);
    
    // Tag the actor for later management
    FString BiomeTag = FString::Printf(TEXT("Vegetation_%s_%s"), 
                                       *UEnum::GetValueAsString(Config.BiomeType),
                                       *UEnum::GetValueAsString(VegType));
    VegActor->Tags.Add(FName(*BiomeTag));
    
    // Store reference for LOD management
    VegetationActors.Add(VegActor);
    
    UE_LOG(LogVegetationSystem, VeryVerbose, TEXT("VegetationSystem: Spawned %s at %s with scale %f"), 
           *BiomeTag, *Location.ToString(), Scale);
}

void UVegetationSystem::UpdateVegetationLOD()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Update LOD for all vegetation actors
    for (int32 i = VegetationActors.Num() - 1; i >= 0; i--)
    {
        AActor* VegActor = VegetationActors[i];
        if (!IsValid(VegActor))
        {
            VegetationActors.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(PlayerLocation, VegActor->GetActorLocation());
        
        // Simple LOD system - hide distant vegetation
        bool bShouldBeVisible = Distance <= MaxVegetationDistance;
        VegActor->SetActorHiddenInGame(!bShouldBeVisible);
        
        // Disable collision for very distant vegetation
        bool bShouldHaveCollision = Distance <= (MaxVegetationDistance * 0.5f);
        VegActor->SetActorEnableCollision(bShouldHaveCollision);
    }
}

EBiomeType UVegetationSystem::GetBiomeAtLocation(const FVector& Location) const
{
    // Simple biome detection based on world coordinates
    // This should eventually be replaced with proper biome data from WorldGeneration
    
    if (Location.X < -25000 && Location.Y < -15000)
    {
        return EBiomeType::Swamp;
    }
    else if (Location.X < -15000 && Location.Y > 15000)
    {
        return EBiomeType::Forest;
    }
    else if (FMath::Abs(Location.X) < 20000 && FMath::Abs(Location.Y) < 20000)
    {
        return EBiomeType::Savanna;
    }
    else if (Location.X > 25000 && FMath::Abs(Location.Y) < 30000)
    {
        return EBiomeType::Desert;
    }
    else if (Location.X > 15000 && Location.Y > 20000)
    {
        return EBiomeType::Mountain;
    }
    
    return EBiomeType::Savanna; // Default fallback
}

void UVegetationSystem::ClearVegetationInRadius(const FVector& Center, float Radius)
{
    for (int32 i = VegetationActors.Num() - 1; i >= 0; i--)
    {
        AActor* VegActor = VegetationActors[i];
        if (!IsValid(VegActor))
        {
            VegetationActors.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(Center, VegActor->GetActorLocation());
        if (Distance <= Radius)
        {
            VegActor->Destroy();
            VegetationActors.RemoveAt(i);
        }
    }
}

void UVegetationSystem::RegenerateVegetationInRadius(const FVector& Center, float Radius)
{
    // Clear existing vegetation first
    ClearVegetationInRadius(Center, Radius);
    
    // Determine biome type at center
    EBiomeType BiomeType = GetBiomeAtLocation(Center);
    
    // Regenerate vegetation for this area
    GenerateVegetationForBiome(BiomeType, Center, Radius);
}
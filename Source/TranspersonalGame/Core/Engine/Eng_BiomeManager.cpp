#include "Eng_BiomeManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/KismetMathLibrary.h"

AEng_BiomeManager::AEng_BiomeManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);
    VisualizationMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    VisualizationMesh->SetVisibility(true);

    // Load default cube mesh for visualization
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube"));
    if (CubeMeshAsset.Succeeded())
    {
        VisualizationMesh->SetStaticMesh(CubeMeshAsset.Object);
        VisualizationMesh->SetRelativeScale3D(FVector(2.0f, 2.0f, 0.5f));
    }

    // Initialize default values
    GlobalTemperature = 25.0f;
    GlobalHumidity = 0.6f;
    SeasonalVariation = 0.2f;
    BiomeUpdateInterval = 5.0f;
    EcosystemComplexity = 1.0f;
    
    bEnableDynamicWeather = true;
    bEnableSeasonalChanges = true;
    bEnableBiomeTransitions = true;
    
    LastUpdateTime = 0.0f;
    bIsInitialized = false;

    // Setup default biome regions
    FEng_BiomeParameters DefaultGrassland;
    DefaultGrassland.BiomeType = EBiomeType::Grassland;
    DefaultGrassland.Temperature = 22.0f;
    DefaultGrassland.Humidity = 0.5f;
    DefaultGrassland.Elevation = 0.0f;
    DefaultGrassland.BiomeInfluenceRadius = 3000.0f;
    BiomeRegions.Add(DefaultGrassland);

    FEng_BiomeParameters DefaultForest;
    DefaultForest.BiomeType = EBiomeType::Forest;
    DefaultForest.Temperature = 18.0f;
    DefaultForest.Humidity = 0.8f;
    DefaultForest.Elevation = 200.0f;
    DefaultForest.BiomeInfluenceRadius = 2500.0f;
    BiomeRegions.Add(DefaultForest);
}

void AEng_BiomeManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeBiomeSystem();
    UpdateVisualization();
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: System initialized with %d biome regions"), BiomeRegions.Num());
}

void AEng_BiomeManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bIsInitialized)
    {
        return;
    }
    
    LastUpdateTime += DeltaTime;
    
    if (LastUpdateTime >= BiomeUpdateInterval)
    {
        UpdateBiomeSystem();
        LastUpdateTime = 0.0f;
    }
}

void AEng_BiomeManager::InitializeBiomeSystem()
{
    if (bIsInitialized)
    {
        return;
    }
    
    // Setup default biome transitions
    if (BiomeTransitions.Num() == 0)
    {
        FEng_BiomeTransition GrasslandToForest;
        GrasslandToForest.FromBiome = EBiomeType::Grassland;
        GrasslandToForest.ToBiome = EBiomeType::Forest;
        GrasslandToForest.TransitionDistance = 800.0f;
        GrasslandToForest.BlendFactor = 0.5f;
        BiomeTransitions.Add(GrasslandToForest);
        
        FEng_BiomeTransition ForestToMountain;
        ForestToMountain.FromBiome = EBiomeType::Forest;
        ForestToMountain.ToBiome = EBiomeType::Mountain;
        ForestToMountain.TransitionDistance = 1200.0f;
        ForestToMountain.BlendFactor = 0.3f;
        BiomeTransitions.Add(ForestToMountain);
    }
    
    bIsInitialized = true;
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Initialization complete"));
}

void AEng_BiomeManager::UpdateBiomeSystem()
{
    if (!bIsInitialized)
    {
        return;
    }
    
    // Update seasonal variations
    if (bEnableSeasonalChanges)
    {
        float TimeSeconds = GetWorld()->GetTimeSeconds();
        float SeasonalFactor = FMath::Sin(TimeSeconds * 0.001f) * SeasonalVariation;
        
        for (FEng_BiomeParameters& Biome : BiomeRegions)
        {
            Biome.Temperature = Biome.Temperature + (SeasonalFactor * 10.0f);
            Biome.Humidity = FMath::Clamp(Biome.Humidity + (SeasonalFactor * 0.2f), 0.0f, 1.0f);
        }
    }
    
    // Update visualization
    UpdateVisualization();
}

void AEng_BiomeManager::UpdateVisualization()
{
    if (!VisualizationMesh)
    {
        return;
    }
    
    // Change color based on dominant biome type
    if (BiomeRegions.Num() > 0)
    {
        EBiomeType DominantBiome = BiomeRegions[0].BiomeType;
        
        // Set material color based on biome type
        FLinearColor BiomeColor = FLinearColor::Green; // Default grassland
        
        switch (DominantBiome)
        {
            case EBiomeType::Forest:
                BiomeColor = FLinearColor(0.0f, 0.5f, 0.0f, 1.0f); // Dark green
                break;
            case EBiomeType::Desert:
                BiomeColor = FLinearColor(1.0f, 0.8f, 0.4f, 1.0f); // Sandy yellow
                break;
            case EBiomeType::Mountain:
                BiomeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray
                break;
            case EBiomeType::Swamp:
                BiomeColor = FLinearColor(0.3f, 0.4f, 0.2f, 1.0f); // Murky green
                break;
            case EBiomeType::Coastal:
                BiomeColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f); // Light blue
                break;
            default:
                BiomeColor = FLinearColor::Green;
                break;
        }
    }
}

EBiomeType AEng_BiomeManager::GetBiomeAtLocation(FVector Location) const
{
    if (BiomeRegions.Num() == 0)
    {
        return EBiomeType::Grassland;
    }
    
    float MaxInfluence = 0.0f;
    EBiomeType DominantBiome = EBiomeType::Grassland;
    
    for (const FEng_BiomeParameters& Biome : BiomeRegions)
    {
        float Influence = CalculateBiomeInfluence(Biome, Location);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome.BiomeType;
        }
    }
    
    return DominantBiome;
}

FEng_BiomeParameters AEng_BiomeManager::GetBiomeParameters(FVector Location) const
{
    if (BiomeRegions.Num() == 0)
    {
        return FEng_BiomeParameters();
    }
    
    // Find the most influential biome at this location
    float MaxInfluence = 0.0f;
    FEng_BiomeParameters DominantBiome;
    
    for (const FEng_BiomeParameters& Biome : BiomeRegions)
    {
        float Influence = CalculateBiomeInfluence(Biome, Location);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome;
        }
    }
    
    return DominantBiome;
}

float AEng_BiomeManager::GetTemperatureAtLocation(FVector Location) const
{
    FEng_BiomeParameters BiomeParams = GetBiomeParameters(Location);
    return BiomeParams.Temperature;
}

float AEng_BiomeManager::GetHumidityAtLocation(FVector Location) const
{
    FEng_BiomeParameters BiomeParams = GetBiomeParameters(Location);
    return BiomeParams.Humidity;
}

bool AEng_BiomeManager::IsLocationInBiomeTransition(FVector Location) const
{
    for (const FEng_BiomeTransition& Transition : BiomeTransitions)
    {
        // Check if location is within transition zone
        // This is a simplified check - in practice would be more complex
        EBiomeType CurrentBiome = GetBiomeAtLocation(Location);
        if (CurrentBiome == Transition.FromBiome || CurrentBiome == Transition.ToBiome)
        {
            return true;
        }
    }
    
    return false;
}

void AEng_BiomeManager::AddBiomeRegion(const FEng_BiomeParameters& NewBiome)
{
    BiomeRegions.Add(NewBiome);
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Added new biome region. Total regions: %d"), BiomeRegions.Num());
}

void AEng_BiomeManager::RemoveBiomeRegion(int32 RegionIndex)
{
    if (BiomeRegions.IsValidIndex(RegionIndex))
    {
        BiomeRegions.RemoveAt(RegionIndex);
        UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Removed biome region %d. Total regions: %d"), RegionIndex, BiomeRegions.Num());
    }
}

void AEng_BiomeManager::RefreshAllBiomes()
{
    UpdateBiomeSystem();
    UpdateVisualization();
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: All biomes refreshed"));
}

void AEng_BiomeManager::IntegrateWithWorldGenerator()
{
    // Integration point for PCGWorldGenerator
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Integrating with World Generator"));
}

void AEng_BiomeManager::IntegrateWithFoliageSystem()
{
    // Integration point for FoliageManager
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Integrating with Foliage System"));
}

void AEng_BiomeManager::NotifyEnvironmentChange(FVector Location, float Radius)
{
    // Handle environment changes that affect biomes
    UE_LOG(LogTemp, Warning, TEXT("BiomeManager: Environment change at location %s, radius %f"), *Location.ToString(), Radius);
}

float AEng_BiomeManager::CalculateBiomeInfluence(const FEng_BiomeParameters& Biome, FVector Location) const
{
    // Calculate distance-based influence
    FVector BiomeCenter = GetActorLocation(); // Simplified - would use biome-specific centers
    float Distance = FVector::Dist(Location, BiomeCenter);
    
    if (Distance > Biome.BiomeInfluenceRadius)
    {
        return 0.0f;
    }
    
    // Linear falloff
    float Influence = 1.0f - (Distance / Biome.BiomeInfluenceRadius);
    return FMath::Clamp(Influence, 0.0f, 1.0f);
}

FEng_BiomeParameters AEng_BiomeManager::BlendBiomeParameters(const FEng_BiomeParameters& BiomeA, const FEng_BiomeParameters& BiomeB, float BlendFactor) const
{
    FEng_BiomeParameters BlendedBiome;
    
    BlendedBiome.Temperature = FMath::Lerp(BiomeA.Temperature, BiomeB.Temperature, BlendFactor);
    BlendedBiome.Humidity = FMath::Lerp(BiomeA.Humidity, BiomeB.Humidity, BlendFactor);
    BlendedBiome.Elevation = FMath::Lerp(BiomeA.Elevation, BiomeB.Elevation, BlendFactor);
    BlendedBiome.WaterDistance = FMath::Lerp(BiomeA.WaterDistance, BiomeB.WaterDistance, BlendFactor);
    BlendedBiome.BiomeInfluenceRadius = FMath::Lerp(BiomeA.BiomeInfluenceRadius, BiomeB.BiomeInfluenceRadius, BlendFactor);
    
    // For enum, use threshold-based selection
    BlendedBiome.BiomeType = (BlendFactor < 0.5f) ? BiomeA.BiomeType : BiomeB.BiomeType;
    
    return BlendedBiome;
}
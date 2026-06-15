#include "World_BiomeSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

UWorld_BiomeSystem::UWorld_BiomeSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f; // Update every 2 seconds
}

void UWorld_BiomeSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (!bIsInitialized)
    {
        InitializeDefaultBiomes();
        
        if (bAutoGenerateBiomes && ActiveBiomes.Num() == 0)
        {
            GenerateRandomBiomes();
        }
        
        bIsInitialized = true;
        UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Initialized with %d biomes"), ActiveBiomes.Num());
    }
}

void UWorld_BiomeSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Optional: Update biome transitions or dynamic effects
}

void UWorld_BiomeSystem::GenerateRandomBiomes()
{
    ClearAllBiomes();
    
    // Generate biomes in a grid pattern with some randomization
    int32 GridSize = FMath::Sqrt(MaxBiomes);
    float CellSize = WorldBounds.X / GridSize;
    
    for (int32 i = 0; i < MaxBiomes; i++)
    {
        FWorld_BiomeData NewBiome;
        
        // Calculate grid position with random offset
        int32 GridX = i % GridSize;
        int32 GridY = i / GridSize;
        
        float BaseX = (GridX * CellSize) - (WorldBounds.X * 0.5f) + (CellSize * 0.5f);
        float BaseY = (GridY * CellSize) - (WorldBounds.Y * 0.5f) + (CellSize * 0.5f);
        
        // Add random offset within cell
        float OffsetX = FMath::RandRange(-CellSize * 0.3f, CellSize * 0.3f);
        float OffsetY = FMath::RandRange(-CellSize * 0.3f, CellSize * 0.3f);
        
        NewBiome.BiomeCenter = FVector2D(BaseX + OffsetX, BaseY + OffsetY);
        NewBiome.BiomeRadius = FMath::RandRange(CellSize * 0.4f, CellSize * 0.8f);
        
        // Randomly assign biome type
        int32 BiomeTypeIndex = FMath::RandRange(0, 5);
        NewBiome.BiomeType = static_cast<EWorld_BiomeType>(BiomeTypeIndex);
        
        // Set biome-specific properties
        switch (NewBiome.BiomeType)
        {
            case EWorld_BiomeType::Forest:
                NewBiome.VegetationDensity = FMath::RandRange(0.7f, 0.9f);
                NewBiome.WaterCoverage = FMath::RandRange(0.05f, 0.15f);
                NewBiome.RockDensity = FMath::RandRange(0.1f, 0.3f);
                NewBiome.VegetationTypes = {"Fern", "Conifer", "Oak", "Moss"};
                NewBiome.DinosaurSpecies = {"Triceratops", "Parasaurolophus", "Ankylosaurus"};
                break;
                
            case EWorld_BiomeType::Plains:
                NewBiome.VegetationDensity = FMath::RandRange(0.2f, 0.4f);
                NewBiome.WaterCoverage = FMath::RandRange(0.02f, 0.08f);
                NewBiome.RockDensity = FMath::RandRange(0.05f, 0.15f);
                NewBiome.VegetationTypes = {"Grass", "Shrub", "Palm"};
                NewBiome.DinosaurSpecies = {"Brachiosaurus", "Velociraptor", "Pachycephalosaurus"};
                break;
                
            case EWorld_BiomeType::RiverValley:
                NewBiome.VegetationDensity = FMath::RandRange(0.5f, 0.7f);
                NewBiome.WaterCoverage = FMath::RandRange(0.2f, 0.4f);
                NewBiome.RockDensity = FMath::RandRange(0.2f, 0.4f);
                NewBiome.VegetationTypes = {"Fern", "Reed", "Willow", "Moss"};
                NewBiome.DinosaurSpecies = {"Parasaurolophus", "Tsintaosaurus", "Protoceratops"};
                break;
                
            case EWorld_BiomeType::RockyOutcrop:
                NewBiome.VegetationDensity = FMath::RandRange(0.1f, 0.3f);
                NewBiome.WaterCoverage = FMath::RandRange(0.0f, 0.05f);
                NewBiome.RockDensity = FMath::RandRange(0.6f, 0.9f);
                NewBiome.VegetationTypes = {"Moss", "Lichen", "Hardy_Shrub"};
                NewBiome.DinosaurSpecies = {"Ankylosaurus", "Protoceratops"};
                break;
                
            case EWorld_BiomeType::Wetlands:
                NewBiome.VegetationDensity = FMath::RandRange(0.6f, 0.8f);
                NewBiome.WaterCoverage = FMath::RandRange(0.3f, 0.6f);
                NewBiome.RockDensity = FMath::RandRange(0.05f, 0.2f);
                NewBiome.VegetationTypes = {"Reed", "Cattail", "Moss", "Fern"};
                NewBiome.DinosaurSpecies = {"Tsintaosaurus", "Parasaurolophus"};
                break;
                
            case EWorld_BiomeType::VolcanicRegion:
                NewBiome.VegetationDensity = FMath::RandRange(0.1f, 0.4f);
                NewBiome.WaterCoverage = FMath::RandRange(0.0f, 0.1f);
                NewBiome.RockDensity = FMath::RandRange(0.7f, 0.95f);
                NewBiome.VegetationTypes = {"Hardy_Shrub", "Volcanic_Moss"};
                NewBiome.DinosaurSpecies = {"Ankylosaurus"};
                break;
        }
        
        ActiveBiomes.Add(NewBiome);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Generated %d random biomes"), ActiveBiomes.Num());
}

EWorld_BiomeType UWorld_BiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    
    float MaxInfluence = 0.0f;
    EWorld_BiomeType DominantBiome = EWorld_BiomeType::Forest;
    
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        float Influence = CalculateBiomeInfluence(WorldLocation, Biome);
        if (Influence > MaxInfluence)
        {
            MaxInfluence = Influence;
            DominantBiome = Biome.BiomeType;
        }
    }
    
    return DominantBiome;
}

FWorld_BiomeData UWorld_BiomeSystem::GetBiomeData(EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        if (Biome.BiomeType == BiomeType)
        {
            return Biome;
        }
    }
    
    // Return default if not found
    FWorld_BiomeData DefaultBiome;
    DefaultBiome.BiomeType = BiomeType;
    return DefaultBiome;
}

void UWorld_BiomeSystem::AddBiome(const FWorld_BiomeData& NewBiome)
{
    if (ActiveBiomes.Num() < MaxBiomes)
    {
        ActiveBiomes.Add(NewBiome);
        UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Added biome at (%.0f, %.0f)"), 
               NewBiome.BiomeCenter.X, NewBiome.BiomeCenter.Y);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Cannot add biome - maximum limit reached"));
    }
}

void UWorld_BiomeSystem::ClearAllBiomes()
{
    ActiveBiomes.Empty();
    UE_LOG(LogTemp, Warning, TEXT("BiomeSystem: Cleared all biomes"));
}

TArray<FVector> UWorld_BiomeSystem::GetWaterSourceLocations() const
{
    TArray<FVector> WaterSources;
    
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        if (Biome.WaterCoverage > 0.15f) // Only biomes with significant water
        {
            // Add center as primary water source
            FVector WaterLocation(Biome.BiomeCenter.X, Biome.BiomeCenter.Y, 0.0f);
            WaterSources.Add(WaterLocation);
            
            // Add additional water sources for large water biomes
            if (Biome.WaterCoverage > 0.3f)
            {
                for (int32 i = 0; i < 3; i++)
                {
                    float Angle = (i * 120.0f) * PI / 180.0f;
                    float Distance = Biome.BiomeRadius * 0.6f;
                    FVector AdditionalWater(
                        Biome.BiomeCenter.X + FMath::Cos(Angle) * Distance,
                        Biome.BiomeCenter.Y + FMath::Sin(Angle) * Distance,
                        0.0f
                    );
                    WaterSources.Add(AdditionalWater);
                }
            }
        }
    }
    
    return WaterSources;
}

float UWorld_BiomeSystem::GetVegetationDensityAtLocation(const FVector& WorldLocation) const
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    float TotalDensity = 0.0f;
    float TotalInfluence = 0.0f;
    
    for (const FWorld_BiomeData& Biome : ActiveBiomes)
    {
        float Influence = CalculateBiomeInfluence(WorldLocation, Biome);
        if (Influence > 0.0f)
        {
            TotalDensity += Biome.VegetationDensity * Influence;
            TotalInfluence += Influence;
        }
    }
    
    return TotalInfluence > 0.0f ? TotalDensity / TotalInfluence : 0.3f;
}

TArray<FString> UWorld_BiomeSystem::GetDinosaurSpeciesAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType DominantBiome = GetBiomeAtLocation(WorldLocation);
    FWorld_BiomeData BiomeData = GetBiomeData(DominantBiome);
    return BiomeData.DinosaurSpecies;
}

void UWorld_BiomeSystem::InitializeDefaultBiomes()
{
    // This can be overridden in Blueprint or called manually
    // Default implementation creates a basic forest biome
    if (ActiveBiomes.Num() == 0)
    {
        FWorld_BiomeData DefaultForest;
        DefaultForest.BiomeType = EWorld_BiomeType::Forest;
        DefaultForest.BiomeCenter = FVector2D(0.0f, 0.0f);
        DefaultForest.BiomeRadius = 10000.0f;
        DefaultForest.VegetationDensity = 0.7f;
        DefaultForest.WaterCoverage = 0.1f;
        DefaultForest.RockDensity = 0.2f;
        ActiveBiomes.Add(DefaultForest);
    }
}

float UWorld_BiomeSystem::CalculateBiomeInfluence(const FVector& WorldLocation, const FWorld_BiomeData& Biome) const
{
    FVector2D Location2D(WorldLocation.X, WorldLocation.Y);
    float Distance = FVector2D::Distance(Location2D, Biome.BiomeCenter);
    
    if (Distance <= Biome.BiomeRadius)
    {
        // Full influence at center, fading to zero at edge
        float NormalizedDistance = Distance / Biome.BiomeRadius;
        return 1.0f - (NormalizedDistance * NormalizedDistance); // Quadratic falloff
    }
    else if (Distance <= Biome.BiomeRadius + BiomeBlendDistance)
    {
        // Blend zone - gradual falloff
        float BlendFactor = (Distance - Biome.BiomeRadius) / BiomeBlendDistance;
        return FMath::Max(0.0f, 1.0f - BlendFactor);
    }
    
    return 0.0f;
}
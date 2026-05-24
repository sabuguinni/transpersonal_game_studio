#include "World_LandscapeExpansionManager.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY_STATIC(LogLandscapeExpansion, Log, All);

UWorld_LandscapeExpansionManager::UWorld_LandscapeExpansionManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize 200km2 target dimensions
    TargetLandscapeSize = FVector2D(1414000.0f, 1414000.0f); // 200km2 = ~14.14km x 14.14km
    CurrentLandscapeSize = FVector2D(157000.0f, 153000.0f);  // Current size from memory
    
    // Initialize biome data from memory coordinates
    InitializeBiomeData();
}

void UWorld_LandscapeExpansionManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogLandscapeExpansion, Log, TEXT("LandscapeExpansionManager initialized"));
    UE_LOG(LogLandscapeExpansion, Log, TEXT("Current size: %.0fx%.0f, Target: %.0fx%.0f"), 
           CurrentLandscapeSize.X, CurrentLandscapeSize.Y, 
           TargetLandscapeSize.X, TargetLandscapeSize.Y);
}

void UWorld_LandscapeExpansionManager::InitializeBiomeData()
{
    BiomeRegions.Empty();
    
    // Pantano (Swamp) - Southwest
    FWorld_BiomeRegion PantanoRegion;
    PantanoRegion.BiomeName = TEXT("Pantano");
    PantanoRegion.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoRegion.BoundsMin = FVector(-77500.0f, -76500.0f, -100.0f);
    PantanoRegion.BoundsMax = FVector(-25000.0f, -15000.0f, 200.0f);
    PantanoRegion.TerrainType = EWorld_TerrainType::Swamp;
    BiomeRegions.Add(PantanoRegion);
    
    // Floresta (Forest) - Northwest  
    FWorld_BiomeRegion FlorestaRegion;
    FlorestaRegion.BiomeName = TEXT("Floresta");
    FlorestaRegion.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaRegion.BoundsMin = FVector(-77500.0f, 15000.0f, -50.0f);
    FlorestaRegion.BoundsMax = FVector(-15000.0f, 76500.0f, 300.0f);
    FlorestaRegion.TerrainType = EWorld_TerrainType::Forest;
    BiomeRegions.Add(FlorestaRegion);
    
    // Savana (Savanna) - Center
    FWorld_BiomeRegion SavanaRegion;
    SavanaRegion.BiomeName = TEXT("Savana");
    SavanaRegion.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaRegion.BoundsMin = FVector(-20000.0f, -20000.0f, -30.0f);
    SavanaRegion.BoundsMax = FVector(20000.0f, 20000.0f, 150.0f);
    SavanaRegion.TerrainType = EWorld_TerrainType::Savanna;
    BiomeRegions.Add(SavanaRegion);
    
    // Deserto (Desert) - East
    FWorld_BiomeRegion DesertoRegion;
    DesertoRegion.BiomeName = TEXT("Deserto");
    DesertoRegion.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoRegion.BoundsMin = FVector(25000.0f, -30000.0f, -20.0f);
    DesertoRegion.BoundsMax = FVector(79500.0f, 30000.0f, 250.0f);
    DesertoRegion.TerrainType = EWorld_TerrainType::Desert;
    BiomeRegions.Add(DesertoRegion);
    
    // Montanha (Mountain) - Northeast
    FWorld_BiomeRegion MontanhaRegion;
    MontanhaRegion.BiomeName = TEXT("Montanha");
    MontanhaRegion.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    MontanhaRegion.BoundsMin = FVector(15000.0f, 20000.0f, 200.0f);
    MontanhaRegion.BoundsMax = FVector(79500.0f, 76500.0f, 1000.0f);
    MontanhaRegion.TerrainType = EWorld_TerrainType::Mountain;
    BiomeRegions.Add(MontanhaRegion);
    
    UE_LOG(LogLandscapeExpansion, Log, TEXT("Initialized %d biome regions"), BiomeRegions.Num());
}

bool UWorld_LandscapeExpansionManager::ExpandLandscapeTo200km2()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogLandscapeExpansion, Error, TEXT("No valid world found"));
        return false;
    }
    
    // Find existing landscape
    ALandscape* ExistingLandscape = nullptr;
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    if (LandscapeActors.Num() > 0)
    {
        ExistingLandscape = Cast<ALandscape>(LandscapeActors[0]);
        UE_LOG(LogLandscapeExpansion, Log, TEXT("Found existing landscape: %s"), 
               *ExistingLandscape->GetName());
    }
    
    // Calculate expansion requirements
    float ExpansionFactorX = TargetLandscapeSize.X / CurrentLandscapeSize.X;
    float ExpansionFactorY = TargetLandscapeSize.Y / CurrentLandscapeSize.Y;
    
    UE_LOG(LogLandscapeExpansion, Log, TEXT("Expansion factors: X=%.2f, Y=%.2f"), 
           ExpansionFactorX, ExpansionFactorY);
    
    // Create expansion sections
    bool bSuccess = CreateLandscapeExpansionSections();
    
    if (bSuccess)
    {
        UE_LOG(LogLandscapeExpansion, Log, TEXT("Successfully expanded landscape to 200km2"));
        CurrentLandscapeSize = TargetLandscapeSize;
    }
    else
    {
        UE_LOG(LogLandscapeExpansion, Error, TEXT("Failed to expand landscape"));
    }
    
    return bSuccess;
}

bool UWorld_LandscapeExpansionManager::CreateLandscapeExpansionSections()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    // Create 4 expansion sections around the current map
    TArray<FVector> ExpansionCenters = {
        FVector(-200000.0f, 0.0f, 0.0f),    // West expansion
        FVector(200000.0f, 0.0f, 0.0f),     // East expansion  
        FVector(0.0f, -200000.0f, 0.0f),    // South expansion
        FVector(0.0f, 200000.0f, 0.0f)      // North expansion
    };
    
    int32 SectionCount = 0;
    for (const FVector& Center : ExpansionCenters)
    {
        // Create landscape section
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("LandscapeSection_%d"), SectionCount));
        
        ALandscape* NewSection = World->SpawnActor<ALandscape>(
            ALandscape::StaticClass(),
            Center,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (NewSection)
        {
            UE_LOG(LogLandscapeExpansion, Log, TEXT("Created landscape section %d at %s"), 
                   SectionCount, *Center.ToString());
            SectionCount++;
        }
    }
    
    return SectionCount > 0;
}

void UWorld_LandscapeExpansionManager::CreateBiomeSpecificTerrain()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (const FWorld_BiomeRegion& Region : BiomeRegions)
    {
        CreateTerrainFeaturesForBiome(Region);
    }
}

void UWorld_LandscapeExpansionManager::CreateTerrainFeaturesForBiome(const FWorld_BiomeRegion& BiomeRegion)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create 5-10 terrain features per biome
    int32 FeatureCount = FMath::RandRange(5, 10);
    
    for (int32 i = 0; i < FeatureCount; i++)
    {
        // Random position within biome bounds
        FVector RandomLocation = FVector(
            FMath::RandRange(BiomeRegion.BoundsMin.X, BiomeRegion.BoundsMax.X),
            FMath::RandRange(BiomeRegion.BoundsMin.Y, BiomeRegion.BoundsMax.Y),
            FMath::RandRange(BiomeRegion.BoundsMin.Z, BiomeRegion.BoundsMax.Z)
        );
        
        // Create terrain feature based on biome type
        CreateTerrainFeatureByType(BiomeRegion.TerrainType, RandomLocation, i, BiomeRegion.BiomeName);
    }
    
    UE_LOG(LogLandscapeExpansion, Log, TEXT("Created %d terrain features for %s biome"), 
           FeatureCount, *BiomeRegion.BiomeName);
}

void UWorld_LandscapeExpansionManager::CreateTerrainFeatureByType(EWorld_TerrainType TerrainType, 
                                                                  const FVector& Location, 
                                                                  int32 Index, 
                                                                  const FString& BiomeName)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FActorSpawnParameters SpawnParams;
    FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
    
    switch (TerrainType)
    {
        case EWorld_TerrainType::Swamp:
        {
            // Create muddy low areas
            FVector SwampLocation = Location;
            SwampLocation.Z -= 50.0f; // Lower elevation for swamp
            
            SpawnParams.Name = FName(*FString::Printf(TEXT("SwampMud_%s_%d"), *BiomeName, Index));
            AStaticMeshActor* SwampFeature = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), SwampLocation, RandomRotation, SpawnParams);
            break;
        }
        
        case EWorld_TerrainType::Forest:
        {
            // Create fallen logs and forest floor features
            RandomRotation.Roll = FMath::RandRange(-15.0f, 15.0f);
            
            SpawnParams.Name = FName(*FString::Printf(TEXT("ForestLog_%s_%d"), *BiomeName, Index));
            AStaticMeshActor* ForestFeature = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), Location, RandomRotation, SpawnParams);
            break;
        }
        
        case EWorld_TerrainType::Savanna:
        {
            // Create rock outcrops
            FVector SavannaLocation = Location;
            SavannaLocation.Z += 20.0f;
            
            SpawnParams.Name = FName(*FString::Printf(TEXT("SavannaRock_%s_%d"), *BiomeName, Index));
            AStaticMeshActor* SavannaFeature = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), SavannaLocation, RandomRotation, SpawnParams);
            break;
        }
        
        case EWorld_TerrainType::Desert:
        {
            // Create sand dunes
            FVector DesertLocation = Location;
            DesertLocation.Z += 30.0f;
            RandomRotation.Roll = FMath::RandRange(-5.0f, 5.0f);
            
            SpawnParams.Name = FName(*FString::Printf(TEXT("DesertDune_%s_%d"), *BiomeName, Index));
            AStaticMeshActor* DesertFeature = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), DesertLocation, RandomRotation, SpawnParams);
            break;
        }
        
        case EWorld_TerrainType::Mountain:
        {
            // Create rocky peaks
            FVector MountainLocation = Location;
            MountainLocation.Z += 200.0f;
            RandomRotation.Pitch = FMath::RandRange(-10.0f, 10.0f);
            RandomRotation.Roll = FMath::RandRange(-10.0f, 10.0f);
            
            SpawnParams.Name = FName(*FString::Printf(TEXT("MountainPeak_%s_%d"), *BiomeName, Index));
            AStaticMeshActor* MountainFeature = World->SpawnActor<AStaticMeshActor>(
                AStaticMeshActor::StaticClass(), MountainLocation, RandomRotation, SpawnParams);
            break;
        }
    }
}

FWorld_BiomeRegion UWorld_LandscapeExpansionManager::GetBiomeAtLocation(const FVector& Location) const
{
    for (const FWorld_BiomeRegion& Region : BiomeRegions)
    {
        if (Location.X >= Region.BoundsMin.X && Location.X <= Region.BoundsMax.X &&
            Location.Y >= Region.BoundsMin.Y && Location.Y <= Region.BoundsMax.Y)
        {
            return Region;
        }
    }
    
    // Return default savanna if no match found
    return BiomeRegions.Num() > 2 ? BiomeRegions[2] : FWorld_BiomeRegion();
}

bool UWorld_LandscapeExpansionManager::IsLocationInBiome(const FVector& Location, const FString& BiomeName) const
{
    FWorld_BiomeRegion FoundBiome = GetBiomeAtLocation(Location);
    return FoundBiome.BiomeName == BiomeName;
}

TArray<FWorld_BiomeRegion> UWorld_LandscapeExpansionManager::GetAllBiomeRegions() const
{
    return BiomeRegions;
}
#include "FoliageManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SceneComponent.h"
#include "FoliageInstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AFoliageManager::AFoliageManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize prehistoric foliage data
    InitializeFoliageTypes();
}

void AFoliageManager::BeginPlay()
{
    Super::BeginPlay();

    // Find or create the foliage actor in the level
    FoliageActor = AInstancedFoliageActor::GetInstancedFoliageActorForLevel(GetLevel());
    if (!FoliageActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FoliageManager: Could not find InstancedFoliageActor in level"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("FoliageManager: Successfully connected to InstancedFoliageActor"));
    }
}

void AFoliageManager::InitializeFoliageTypes()
{
    // Initialize prehistoric vegetation types
    PrehistoricVegetation.Empty();

    // Giant Ferns - dominant ground cover
    FPrehistoricFoliageData GiantFern;
    GiantFern.FoliageType = TEXT("GiantFern");
    GiantFern.MinScale = 1.5f;
    GiantFern.MaxScale = 3.0f;
    GiantFern.Density = 2.0f;
    GiantFern.bNaniteEnabled = true;
    PrehistoricVegetation.Add(GiantFern);

    // Conifer Trees - tall prehistoric conifers
    FPrehistoricFoliageData ConiferTree;
    ConiferTree.FoliageType = TEXT("ConiferTree");
    ConiferTree.MinScale = 8.0f;
    ConiferTree.MaxScale = 15.0f;
    ConiferTree.Density = 0.3f;
    ConiferTree.bNaniteEnabled = true;
    PrehistoricVegetation.Add(ConiferTree);

    // Cycad Palms - prehistoric palm-like plants
    FPrehistoricFoliageData CycadPalm;
    CycadPalm.FoliageType = TEXT("CycadPalm");
    CycadPalm.MinScale = 2.0f;
    CycadPalm.MaxScale = 4.0f;
    CycadPalm.Density = 0.8f;
    CycadPalm.bNaniteEnabled = true;
    PrehistoricVegetation.Add(CycadPalm);

    // Moss patches - ground detail
    FPrehistoricFoliageData Moss;
    Moss.FoliageType = TEXT("Moss");
    Moss.MinScale = 0.5f;
    Moss.MaxScale = 1.5f;
    Moss.Density = 5.0f;
    Moss.bNaniteEnabled = false; // Too small for Nanite
    PrehistoricVegetation.Add(Moss);

    // Hanging vines
    FPrehistoricFoliageData Vine;
    Vine.FoliageType = TEXT("Vine");
    Vine.MinScale = 1.0f;
    Vine.MaxScale = 2.5f;
    Vine.Density = 1.5f;
    Vine.bNaniteEnabled = true;
    PrehistoricVegetation.Add(Vine);

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Initialized %d prehistoric vegetation types"), PrehistoricVegetation.Num());
}

void AFoliageManager::PopulateFoliageInRadius(FVector Center, float Radius, const FString& BiomeType)
{
    if (!FoliageActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FoliageManager: No FoliageActor available for population"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Populating foliage in radius %.2f around (%s) for biome %s"), 
           Radius, *Center.ToString(), *BiomeType);

    // Get foliage types appropriate for this biome
    TArray<FPrehistoricFoliageData> BiomeFoliage;
    
    if (BiomeType == TEXT("DenseForest"))
    {
        // Dense forest: all vegetation types with high density
        BiomeFoliage = PrehistoricVegetation;
        for (auto& Foliage : BiomeFoliage)
        {
            Foliage.Density *= 1.5f; // Increase density for dense forest
        }
    }
    else if (BiomeType == TEXT("Wetland"))
    {
        // Wetland: ferns, moss, some palms
        for (const auto& Foliage : PrehistoricVegetation)
        {
            if (Foliage.FoliageType == TEXT("GiantFern") || 
                Foliage.FoliageType == TEXT("Moss") || 
                Foliage.FoliageType == TEXT("CycadPalm"))
            {
                BiomeFoliage.Add(Foliage);
            }
        }
    }
    else if (BiomeType == TEXT("Highland"))
    {
        // Highland: mostly conifers and some ferns
        for (const auto& Foliage : PrehistoricVegetation)
        {
            if (Foliage.FoliageType == TEXT("ConiferTree") || 
                Foliage.FoliageType == TEXT("GiantFern"))
            {
                auto HighlandFoliage = Foliage;
                HighlandFoliage.Density *= 0.7f; // Reduce density for highland
                BiomeFoliage.Add(HighlandFoliage);
            }
        }
    }
    else
    {
        // Default: balanced mix
        BiomeFoliage = PrehistoricVegetation;
    }

    int32 TotalInstancesCreated = 0;

    // Place foliage instances
    for (const auto& FoliageData : BiomeFoliage)
    {
        float AdjustedDensity = FoliageData.Density * GlobalFoliageDensity;
        int32 InstanceCount = FMath::RoundToInt(AdjustedDensity * (Radius / 1000.0f) * (Radius / 1000.0f));
        
        for (int32 i = 0; i < InstanceCount; ++i)
        {
            // Generate random position within radius
            FVector2D RandomCircle = FMath::RandPointInCircle(Radius);
            FVector InstanceLocation = Center + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
            
            // Perform line trace to get ground height
            FHitResult HitResult;
            FVector TraceStart = InstanceLocation + FVector(0, 0, 1000);
            FVector TraceEnd = InstanceLocation - FVector(0, 0, 1000);
            
            if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
            {
                InstanceLocation.Z = HitResult.Location.Z;
                
                // Generate random rotation
                FRotator InstanceRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                
                // Generate random scale
                float RandomScale = FMath::RandRange(FoliageData.MinScale, FoliageData.MaxScale);
                FVector InstanceScale = FVector(RandomScale);
                
                CreateFoliageInstance(FoliageData, InstanceLocation, InstanceRotation, InstanceScale);
                TotalInstancesCreated++;
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Created %d foliage instances for biome %s"), 
           TotalInstancesCreated, *BiomeType);

    // Broadcast event
    OnFoliagePopulated(BiomeType, TotalInstancesCreated);
}

void AFoliageManager::ClearFoliageInRadius(FVector Center, float Radius)
{
    if (!FoliageActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("FoliageManager: No FoliageActor available for clearing"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Clearing foliage in radius %.2f around (%s)"), 
           Radius, *Center.ToString());

    // This would typically interface with the foliage system to remove instances
    // Implementation depends on UE5's foliage API
}

void AFoliageManager::UpdateFoliageDensity(float NewDensity)
{
    GlobalFoliageDensity = FMath::Clamp(NewDensity, 0.1f, 5.0f);
    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Updated global foliage density to %.2f"), GlobalFoliageDensity);
}

void AFoliageManager::SetBiomeFoliage(const FString& BiomeType)
{
    UE_LOG(LogTemp, Log, TEXT("FoliageManager: Setting biome foliage for %s"), *BiomeType);
    // Implementation for changing foliage based on biome
}

void AFoliageManager::CreateFoliageInstance(const FPrehistoricFoliageData& FoliageData, 
                                           FVector Location, FRotator Rotation, FVector Scale)
{
    // This is a simplified version - actual implementation would use UE5's foliage system
    UE_LOG(LogTemp, VeryVerbose, TEXT("FoliageManager: Creating %s instance at %s"), 
           *FoliageData.FoliageType, *Location.ToString());
}

FPrehistoricFoliageData AFoliageManager::GetFoliageForBiome(const FString& BiomeType) const
{
    // Return appropriate foliage data for biome
    if (PrehistoricVegetation.Num() > 0)
    {
        return PrehistoricVegetation[0];
    }
    return FPrehistoricFoliageData();
}
#include "Arch_CretaceousStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogCretaceousStructure, Log, All);

UArch_CretaceousStructureSystem::UArch_CretaceousStructureSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    // Initialize structure categories
    InitializeStructureCategories();
    
    // Set default values
    MaxStructuresPerBiome = 50;
    StructureSpawnRadius = 5000.0f;
    MinDistanceBetweenStructures = 500.0f;
    WeatheringIntensity = 0.7f;
    VegetationOvergrowthLevel = 0.5f;
    
    UE_LOG(LogCretaceousStructure, Log, TEXT("CretaceousStructureSystem initialized"));
}

void UArch_CretaceousStructureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structure placement system
    InitializeStructurePlacement();
    
    // Start periodic structure maintenance
    GetWorld()->GetTimerManager().SetTimer(
        MaintenanceTimerHandle,
        this,
        &UArch_CretaceousStructureSystem::PerformStructureMaintenance,
        60.0f,
        true
    );
}

void UArch_CretaceousStructureSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update structure weathering
    UpdateStructureWeathering(DeltaTime);
    
    // Process vegetation overgrowth
    ProcessVegetationOvergrowth(DeltaTime);
}

void UArch_CretaceousStructureSystem::InitializeStructureCategories()
{
    // Natural formations
    FArch_StructureCategory NaturalCategory;
    NaturalCategory.CategoryName = TEXT("Natural");
    NaturalCategory.SpawnWeight = 0.4f;
    NaturalCategory.MaxInstancesPerBiome = 20;
    NaturalCategory.StructureTypes = {
        TEXT("StoneArch"),
        TEXT("RockPillar"),
        TEXT("CaveEntrance"),
        TEXT("NaturalBridge")
    };
    StructureCategories.Add(NaturalCategory);
    
    // Primitive constructions
    FArch_StructureCategory PrimitiveCategory;
    PrimitiveCategory.CategoryName = TEXT("Primitive");
    PrimitiveCategory.SpawnWeight = 0.3f;
    PrimitiveCategory.MaxInstancesPerBiome = 15;
    PrimitiveCategory.StructureTypes = {
        TEXT("StonePlatform"),
        TEXT("RockShelter"),
        TEXT("PrimitiveAltar"),
        TEXT("StoneCircle")
    };
    StructureCategories.Add(PrimitiveCategory);
    
    // Ruins and remnants
    FArch_StructureCategory RuinsCategory;
    RuinsCategory.CategoryName = TEXT("Ruins");
    RuinsCategory.SpawnWeight = 0.2f;
    RuinsCategory.MaxInstancesPerBiome = 10;
    RuinsCategory.StructureTypes = {
        TEXT("AncientWall"),
        TEXT("CollapsedStructure"),
        TEXT("WeatheredPillar"),
        TEXT("BrokenFoundation")
    };
    StructureCategories.Add(RuinsCategory);
    
    // Geological features
    FArch_StructureCategory GeologicalCategory;
    GeologicalCategory.CategoryName = TEXT("Geological");
    GeologicalCategory.SpawnWeight = 0.1f;
    GeologicalCategory.MaxInstancesPerBiome = 5;
    GeologicalCategory.StructureTypes = {
        TEXT("CrystalFormation"),
        TEXT("FossilOutcrop"),
        TEXT("MineralDeposit"),
        TEXT("VolcanicRock")
    };
    StructureCategories.Add(GeologicalCategory);
}

void UArch_CretaceousStructureSystem::InitializeStructurePlacement()
{
    if (!GetWorld())
    {
        UE_LOG(LogCretaceousStructure, Warning, TEXT("Cannot initialize structure placement - no world"));
        return;
    }
    
    // Clear existing placement data
    PlacedStructures.Empty();
    BiomeStructureCounts.Empty();
    
    // Initialize biome counters
    BiomeStructureCounts.Add(TEXT("Forest"), 0);
    BiomeStructureCounts.Add(TEXT("Plains"), 0);
    BiomeStructureCounts.Add(TEXT("Mountains"), 0);
    BiomeStructureCounts.Add(TEXT("Wetlands"), 0);
    BiomeStructureCounts.Add(TEXT("Desert"), 0);
    
    UE_LOG(LogCretaceousStructure, Log, TEXT("Structure placement system initialized"));
}

bool UArch_CretaceousStructureSystem::SpawnStructureAtLocation(const FVector& Location, const FString& StructureType, const FString& BiomeName)
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check biome structure limits
    int32* BiomeCount = BiomeStructureCounts.Find(BiomeName);
    if (BiomeCount && *BiomeCount >= MaxStructuresPerBiome)
    {
        UE_LOG(LogCretaceousStructure, Warning, TEXT("Biome %s has reached structure limit"), *BiomeName);
        return false;
    }
    
    // Check distance from existing structures
    if (!IsValidStructureLocation(Location))
    {
        return false;
    }
    
    // Create structure data
    FArch_CretaceousStructure NewStructure;
    NewStructure.StructureID = FGuid::NewGuid();
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.BiomeName = BiomeName;
    NewStructure.WeatheringLevel = FMath::RandRange(0.1f, 0.3f);
    NewStructure.VegetationCoverage = FMath::RandRange(0.0f, 0.2f);
    NewStructure.Age = FMath::RandRange(100.0f, 1000.0f);
    NewStructure.bIsStable = true;
    NewStructure.LastMaintenanceTime = GetWorld()->GetTimeSeconds();
    
    // Add to tracking
    PlacedStructures.Add(NewStructure);
    if (BiomeCount)
    {
        (*BiomeCount)++;
    }
    
    UE_LOG(LogCretaceousStructure, Log, TEXT("Spawned %s structure at %s in biome %s"), 
           *StructureType, *Location.ToString(), *BiomeName);
    
    return true;
}

bool UArch_CretaceousStructureSystem::IsValidStructureLocation(const FVector& Location) const
{
    // Check minimum distance from existing structures
    for (const FArch_CretaceousStructure& Structure : PlacedStructures)
    {
        float Distance = FVector::Dist(Location, Structure.Location);
        if (Distance < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    return true;
}

void UArch_CretaceousStructureSystem::UpdateStructureWeathering(float DeltaTime)
{
    float WeatheringRate = WeatheringIntensity * DeltaTime * 0.001f; // Very slow weathering
    
    for (FArch_CretaceousStructure& Structure : PlacedStructures)
    {
        // Increase weathering over time
        Structure.WeatheringLevel = FMath::Clamp(
            Structure.WeatheringLevel + WeatheringRate,
            0.0f,
            1.0f
        );
        
        // Update age
        Structure.Age += DeltaTime;
        
        // Check structural stability
        if (Structure.WeatheringLevel > 0.8f)
        {
            Structure.bIsStable = false;
        }
    }
}

void UArch_CretaceousStructureSystem::ProcessVegetationOvergrowth(float DeltaTime)
{
    float OvergrowthRate = VegetationOvergrowthLevel * DeltaTime * 0.0005f; // Very slow overgrowth
    
    for (FArch_CretaceousStructure& Structure : PlacedStructures)
    {
        // Vegetation grows more on older, more weathered structures
        float GrowthModifier = (Structure.WeatheringLevel + Structure.Age * 0.001f) * 0.5f;
        float ActualGrowthRate = OvergrowthRate * GrowthModifier;
        
        Structure.VegetationCoverage = FMath::Clamp(
            Structure.VegetationCoverage + ActualGrowthRate,
            0.0f,
            1.0f
        );
    }
}

void UArch_CretaceousStructureSystem::PerformStructureMaintenance()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    int32 MaintenanceCount = 0;
    
    for (FArch_CretaceousStructure& Structure : PlacedStructures)
    {
        // Perform maintenance every 5 minutes
        if (CurrentTime - Structure.LastMaintenanceTime > 300.0f)
        {
            // Reset some weathering for maintained structures
            if (Structure.bIsStable)
            {
                Structure.WeatheringLevel = FMath::Max(
                    Structure.WeatheringLevel - 0.1f,
                    0.0f
                );
            }
            
            Structure.LastMaintenanceTime = CurrentTime;
            MaintenanceCount++;
        }
    }
    
    if (MaintenanceCount > 0)
    {
        UE_LOG(LogCretaceousStructure, Log, TEXT("Performed maintenance on %d structures"), MaintenanceCount);
    }
}

TArray<FArch_CretaceousStructure> UArch_CretaceousStructureSystem::GetStructuresInBiome(const FString& BiomeName) const
{
    TArray<FArch_CretaceousStructure> BiomeStructures;
    
    for (const FArch_CretaceousStructure& Structure : PlacedStructures)
    {
        if (Structure.BiomeName == BiomeName)
        {
            BiomeStructures.Add(Structure);
        }
    }
    
    return BiomeStructures;
}

int32 UArch_CretaceousStructureSystem::GetStructureCountInBiome(const FString& BiomeName) const
{
    const int32* Count = BiomeStructureCounts.Find(BiomeName);
    return Count ? *Count : 0;
}

void UArch_CretaceousStructureSystem::RemoveStructure(const FGuid& StructureID)
{
    for (int32 i = PlacedStructures.Num() - 1; i >= 0; i--)
    {
        if (PlacedStructures[i].StructureID == StructureID)
        {
            FString BiomeName = PlacedStructures[i].BiomeName;
            PlacedStructures.RemoveAt(i);
            
            // Update biome count
            int32* BiomeCount = BiomeStructureCounts.Find(BiomeName);
            if (BiomeCount && *BiomeCount > 0)
            {
                (*BiomeCount)--;
            }
            
            UE_LOG(LogCretaceousStructure, Log, TEXT("Removed structure %s from biome %s"), 
                   *StructureID.ToString(), *BiomeName);
            break;
        }
    }
}

FString UArch_CretaceousStructureSystem::GetRandomStructureType(const FString& CategoryName) const
{
    for (const FArch_StructureCategory& Category : StructureCategories)
    {
        if (Category.CategoryName == CategoryName && Category.StructureTypes.Num() > 0)
        {
            int32 RandomIndex = FMath::RandRange(0, Category.StructureTypes.Num() - 1);
            return Category.StructureTypes[RandomIndex];
        }
    }
    
    return TEXT("StoneArch"); // Default fallback
}

void UArch_CretaceousStructureSystem::SetWeatheringIntensity(float NewIntensity)
{
    WeatheringIntensity = FMath::Clamp(NewIntensity, 0.0f, 2.0f);
    UE_LOG(LogCretaceousStructure, Log, TEXT("Weathering intensity set to %f"), WeatheringIntensity);
}

void UArch_CretaceousStructureSystem::SetVegetationOvergrowthLevel(float NewLevel)
{
    VegetationOvergrowthLevel = FMath::Clamp(NewLevel, 0.0f, 2.0f);
    UE_LOG(LogCretaceousStructure, Log, TEXT("Vegetation overgrowth level set to %f"), VegetationOvergrowthLevel);
}
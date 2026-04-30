#include "ArchitectureManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UArchitectureManager::UArchitectureManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 2.0f;
    
    MaxShelterDistance = 5000.0f;
    MaxSheltersPerArea = 3;
    
    RegisteredShelters.Empty();
}

void UArchitectureManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultShelters();
    
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Initialized with %d default shelters"), RegisteredShelters.Num());
}

void UArchitectureManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Debug draw registered shelters
    if (GetWorld() && GetWorld()->IsGameWorld())
    {
        for (const FArch_ShelterData& Shelter : RegisteredShelters)
        {
            FColor DebugColor = FColor::Green;
            switch (Shelter.ShelterType)
            {
                case EArch_ShelterType::Cave:
                    DebugColor = FColor::Blue;
                    break;
                case EArch_ShelterType::ElevatedPlatform:
                    DebugColor = FColor::Yellow;
                    break;
                case EArch_ShelterType::CliffDwelling:
                    DebugColor = FColor::Red;
                    break;
                case EArch_ShelterType::RockOverhang:
                    DebugColor = FColor::Orange;
                    break;
                default:
                    DebugColor = FColor::White;
                    break;
            }
            
            DrawDebugSphere(GetWorld(), Shelter.Location, 100.0f, 12, DebugColor, false, 2.5f);
        }
    }
}

void UArchitectureManager::RegisterShelter(const FArch_ShelterData& ShelterData)
{
    if (ShelterData.ShelterType == EArch_ShelterType::None)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Cannot register shelter with None type"));
        return;
    }
    
    // Check if we already have too many shelters in this area
    TArray<FArch_ShelterData> NearbyShelters = FindNearbyShelters(ShelterData.Location, 1000.0f);
    if (NearbyShelters.Num() >= MaxSheltersPerArea)
    {
        UE_LOG(LogTemp, Warning, TEXT("ArchitectureManager: Too many shelters in area, cannot register new one"));
        return;
    }
    
    RegisteredShelters.Add(ShelterData);
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Registered new shelter at location %s"), *ShelterData.Location.ToString());
}

TArray<FArch_ShelterData> UArchitectureManager::FindNearbyShelters(FVector PlayerLocation, float SearchRadius)
{
    TArray<FArch_ShelterData> NearbyShelters;
    
    for (const FArch_ShelterData& Shelter : RegisteredShelters)
    {
        float Distance = FVector::Dist(PlayerLocation, Shelter.Location);
        if (Distance <= SearchRadius)
        {
            NearbyShelters.Add(Shelter);
        }
    }
    
    return NearbyShelters;
}

FArch_ShelterData UArchitectureManager::GetBestShelterForLocation(FVector Location, float SearchRadius)
{
    TArray<FArch_ShelterData> NearbyShelters = FindNearbyShelters(Location, SearchRadius);
    
    FArch_ShelterData BestShelter;
    float BestScore = -1.0f;
    
    for (const FArch_ShelterData& Shelter : NearbyShelters)
    {
        float Distance = FVector::Dist(Location, Shelter.Location);
        float DistanceScore = 1.0f - (Distance / SearchRadius);
        float TotalScore = (Shelter.SafetyRating * 0.6f) + (DistanceScore * 0.4f);
        
        if (TotalScore > BestScore)
        {
            BestScore = TotalScore;
            BestShelter = Shelter;
        }
    }
    
    return BestShelter;
}

bool UArchitectureManager::CanBuildShelterAt(FVector Location, EArch_ShelterType ShelterType)
{
    // Check terrain suitability
    float TerrainSuitability = CalculateTerrainSuitability(Location, ShelterType);
    if (TerrainSuitability < 0.3f)
    {
        return false;
    }
    
    // Check distance from existing shelters
    TArray<FArch_ShelterData> NearbyShelters = FindNearbyShelters(Location, 500.0f);
    if (NearbyShelters.Num() > 0)
    {
        return false; // Too close to existing shelter
    }
    
    return IsLocationSuitable(Location, ShelterType);
}

void UArchitectureManager::SpawnShelterActor(const FArch_ShelterData& ShelterData)
{
    if (!GetWorld())
    {
        return;
    }
    
    // This would spawn actual shelter actors in the world
    // For now, just register the shelter data
    RegisterShelter(ShelterData);
    
    UE_LOG(LogTemp, Log, TEXT("ArchitectureManager: Spawned shelter actor of type %d at %s"), 
           (int32)ShelterData.ShelterType, *ShelterData.Location.ToString());
}

float UArchitectureManager::CalculateShelterSafety(const FArch_ShelterData& ShelterData, FVector ThreatLocation)
{
    float Distance = FVector::Dist(ShelterData.Location, ThreatLocation);
    float BaseSafety = ShelterData.SafetyRating;
    
    // Distance bonus - farther from threat is safer
    float DistanceBonus = FMath::Clamp(Distance / 1000.0f, 0.0f, 0.5f);
    
    // Shelter type bonus
    float TypeBonus = 0.0f;
    switch (ShelterData.ShelterType)
    {
        case EArch_ShelterType::Cave:
            TypeBonus = 0.3f; // Caves are very safe
            break;
        case EArch_ShelterType::CliffDwelling:
            TypeBonus = 0.25f; // High ground advantage
            break;
        case EArch_ShelterType::ElevatedPlatform:
            TypeBonus = 0.2f; // Height advantage
            break;
        case EArch_ShelterType::RockOverhang:
            TypeBonus = 0.15f; // Some protection
            break;
        default:
            TypeBonus = 0.0f;
            break;
    }
    
    return FMath::Clamp(BaseSafety + DistanceBonus + TypeBonus, 0.0f, 1.0f);
}

void UArchitectureManager::InitializeDefaultShelters()
{
    // Create some default shelters for testing
    FArch_ShelterData CaveShelter;
    CaveShelter.ShelterType = EArch_ShelterType::Cave;
    CaveShelter.Location = FVector(2000.0f, 1500.0f, 200.0f);
    CaveShelter.SafetyRating = 0.8f;
    CaveShelter.CapacityPersons = 4.0f;
    CaveShelter.bHasFirePit = true;
    CaveShelter.bHasWaterAccess = false;
    CaveShelter.Materials.Add(EArch_ConstructionMaterial::Stone);
    RegisteredShelters.Add(CaveShelter);
    
    FArch_ShelterData RockShelter;
    RockShelter.ShelterType = EArch_ShelterType::RockOverhang;
    RockShelter.Location = FVector(-1500.0f, 2000.0f, 150.0f);
    RockShelter.SafetyRating = 0.6f;
    RockShelter.CapacityPersons = 2.0f;
    RockShelter.bHasFirePit = true;
    RockShelter.bHasWaterAccess = true;
    RockShelter.Materials.Add(EArch_ConstructionMaterial::Stone);
    RockShelter.Materials.Add(EArch_ConstructionMaterial::AnimalHide);
    RegisteredShelters.Add(RockShelter);
    
    FArch_ShelterData TreeShelter;
    TreeShelter.ShelterType = EArch_ShelterType::ElevatedPlatform;
    TreeShelter.Location = FVector(0.0f, -2500.0f, 800.0f);
    TreeShelter.SafetyRating = 0.7f;
    TreeShelter.CapacityPersons = 3.0f;
    TreeShelter.bHasFirePit = false;
    TreeShelter.bHasWaterAccess = false;
    TreeShelter.Materials.Add(EArch_ConstructionMaterial::Wood);
    TreeShelter.Materials.Add(EArch_ConstructionMaterial::Vine);
    RegisteredShelters.Add(TreeShelter);
}

bool UArchitectureManager::IsLocationSuitable(FVector Location, EArch_ShelterType ShelterType)
{
    // Basic terrain checks
    switch (ShelterType)
    {
        case EArch_ShelterType::Cave:
            return Location.Z > 100.0f; // Caves need some elevation
        case EArch_ShelterType::ElevatedPlatform:
            return Location.Z > 500.0f; // Need height for tree platforms
        case EArch_ShelterType::CliffDwelling:
            return Location.Z > 300.0f; // Need cliff face
        case EArch_ShelterType::RockOverhang:
            return true; // Can be built almost anywhere
        default:
            return true;
    }
}

float UArchitectureManager::CalculateTerrainSuitability(FVector Location, EArch_ShelterType ShelterType)
{
    // Simple terrain suitability calculation
    // In a real implementation, this would check actual terrain data
    
    float BaseSuitability = 0.5f;
    
    // Height-based suitability
    if (ShelterType == EArch_ShelterType::ElevatedPlatform && Location.Z > 500.0f)
    {
        BaseSuitability += 0.3f;
    }
    else if (ShelterType == EArch_ShelterType::Cave && Location.Z > 200.0f)
    {
        BaseSuitability += 0.2f;
    }
    
    // Distance from origin (assuming safer areas are further from spawn)
    float DistanceFromOrigin = FVector::Dist(Location, FVector::ZeroVector);
    if (DistanceFromOrigin > 1000.0f)
    {
        BaseSuitability += 0.1f;
    }
    
    return FMath::Clamp(BaseSuitability, 0.0f, 1.0f);
}
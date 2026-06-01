#include "Arch_StructuralManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UArch_StructuralManager::UArch_StructuralManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f;
    
    MaxStructuresPerBiome = 25;
    StructureSpawnRadius = 15000.0f;
    MinDistanceBetweenStructures = 2000.0f;
    bAutoGenerateStructures = true;
    WeatheringRate = 0.1f;
}

void UArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    InitializeStructureSystem();
    
    if (bAutoGenerateStructures)
    {
        // Generate structures for each biome
        GenerateStructuresForBiome(EBiomeType::Savanna, 20);
        GenerateStructuresForBiome(EBiomeType::Forest, 25);
        GenerateStructuresForBiome(EBiomeType::Desert, 15);
        GenerateStructuresForBiome(EBiomeType::Swamp, 18);
        GenerateStructuresForBiome(EBiomeType::Mountain, 22);
    }
}

void UArch_StructuralManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Periodic weathering updates
    for (const FArch_StructureData& StructureData : ActiveStructures)
    {
        // Find structure actor and apply weathering
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (FVector::Dist(Actor->GetActorLocation(), StructureData.Location) < 100.0f)
            {
                UpdateStructureWeathering(Actor, WeatheringRate * DeltaTime);
                break;
            }
        }
    }
}

void UArch_StructuralManager::SpawnStructureAtLocation(EArch_StructureType StructureType, FVector Location, EBiomeType BiomeType)
{
    if (!IsValidStructureLocation(Location, BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid structure location"));
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Create structure actor
    AStaticMeshActor* StructureActor = World->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
    if (!StructureActor)
    {
        return;
    }
    
    // Set up mesh component
    UStaticMeshComponent* MeshComp = CreateStructureMesh(StructureType);
    if (MeshComp)
    {
        StructureActor->SetRootComponent(MeshComp);
        ApplyBiomeSpecificMaterials(MeshComp, BiomeType);
    }
    
    // Create structure data
    FArch_StructureData NewStructure;
    NewStructure.StructureType = StructureType;
    NewStructure.Location = Location;
    NewStructure.BiomeType = BiomeType;
    NewStructure.bIsPlayerAccessible = true;
    
    // Register structure
    RegisterStructure(StructureActor, NewStructure);
    
    UE_LOG(LogTemp, Log, TEXT("Spawned structure at location: %s"), *Location.ToString());
}

void UArch_StructuralManager::GenerateStructuresForBiome(EBiomeType BiomeType, int32 StructureCount)
{
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    
    for (int32 i = 0; i < StructureCount; ++i)
    {
        // Random location within biome
        FVector RandomOffset = FVector(
            FMath::RandRange(-StructureSpawnRadius, StructureSpawnRadius),
            FMath::RandRange(-StructureSpawnRadius, StructureSpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = BiomeCenter + RandomOffset;
        SpawnLocation.Z = 100.0f; // Ground level
        
        // Random structure type appropriate for biome
        EArch_StructureType StructureType;
        switch (BiomeType)
        {
            case EBiomeType::Mountain:
                StructureType = (FMath::RandBool()) ? EArch_StructureType::CaveEntrance : EArch_StructureType::CliffOverhang;
                break;
            case EBiomeType::Desert:
                StructureType = (FMath::RandBool()) ? EArch_StructureType::RockShelter : EArch_StructureType::NaturalArch;
                break;
            case EBiomeType::Forest:
                StructureType = EArch_StructureType::BoulderFormation;
                break;
            case EBiomeType::Swamp:
                StructureType = EArch_StructureType::StonePlatform;
                break;
            default:
                StructureType = EArch_StructureType::RockShelter;
                break;
        }
        
        SpawnStructureAtLocation(StructureType, SpawnLocation, BiomeType);
    }
}

TArray<AActor*> UArch_StructuralManager::GetStructuresInRadius(FVector Center, float Radius)
{
    TArray<AActor*> NearbyStructures;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AStaticMeshActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (FVector::Dist(Actor->GetActorLocation(), Center) <= Radius)
        {
            // Check if this is a structure we manage
            for (const FArch_StructureData& StructureData : ActiveStructures)
            {
                if (FVector::Dist(Actor->GetActorLocation(), StructureData.Location) < 100.0f)
                {
                    NearbyStructures.Add(Actor);
                    break;
                }
            }
        }
    }
    
    return NearbyStructures;
}

void UArch_StructuralManager::RemoveStructuresInArea(FVector Center, float Radius)
{
    TArray<AActor*> StructuresToRemove = GetStructuresInRadius(Center, Radius);
    
    for (AActor* Structure : StructuresToRemove)
    {
        // Remove from active structures list
        ActiveStructures.RemoveAll([Structure](const FArch_StructureData& Data) {
            return FVector::Dist(Structure->GetActorLocation(), Data.Location) < 100.0f;
        });
        
        // Destroy actor
        Structure->Destroy();
    }
}

bool UArch_StructuralManager::IsLocationSheltered(FVector Location)
{
    TArray<AActor*> NearbyStructures = GetStructuresInRadius(Location, 1000.0f);
    
    for (AActor* Structure : NearbyStructures)
    {
        // Simple shelter check - if structure is above the location
        if (Structure->GetActorLocation().Z > Location.Z + 200.0f)
        {
            float Distance = FVector::Dist2D(Structure->GetActorLocation(), Location);
            if (Distance < 500.0f)
            {
                return true;
            }
        }
    }
    
    return false;
}

AActor* UArch_StructuralManager::FindNearestShelter(FVector PlayerLocation, float SearchRadius)
{
    TArray<AActor*> NearbyStructures = GetStructuresInRadius(PlayerLocation, SearchRadius);
    AActor* NearestShelter = nullptr;
    float NearestDistance = SearchRadius;
    
    for (AActor* Structure : NearbyStructures)
    {
        float Distance = FVector::Dist(Structure->GetActorLocation(), PlayerLocation);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestShelter = Structure;
        }
    }
    
    return NearestShelter;
}

void UArch_StructuralManager::CreateEmergencyShelter(FVector Location, EBiomeType BiomeType)
{
    // Create a simple rock shelter for emergency situations
    SpawnStructureAtLocation(EArch_StructureType::RockShelter, Location, BiomeType);
    UE_LOG(LogTemp, Log, TEXT("Created emergency shelter at: %s"), *Location.ToString());
}

float UArch_StructuralManager::CalculateStructuralStability(AActor* Structure)
{
    if (!Structure)
    {
        return 0.0f;
    }
    
    // Simple stability calculation based on age and weather exposure
    float BaseStability = 100.0f;
    
    // Find structure data
    for (const FArch_StructureData& StructureData : ActiveStructures)
    {
        if (FVector::Dist(Structure->GetActorLocation(), StructureData.Location) < 100.0f)
        {
            // Reduce stability based on biome conditions
            switch (StructureData.BiomeType)
            {
                case EBiomeType::Swamp:
                    BaseStability *= 0.7f; // High moisture degrades structures
                    break;
                case EBiomeType::Desert:
                    BaseStability *= 0.8f; // Sand erosion
                    break;
                case EBiomeType::Mountain:
                    BaseStability *= 0.9f; // Cold and wind
                    break;
                default:
                    BaseStability *= 0.85f;
                    break;
            }
            break;
        }
    }
    
    return FMath::Clamp(BaseStability, 0.0f, 100.0f);
}

void UArch_StructuralManager::UpdateStructureWeathering(AActor* Structure, float WeatherIntensity)
{
    if (!Structure)
    {
        return;
    }
    
    // Apply visual weathering effects
    UStaticMeshComponent* MeshComp = Structure->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Gradually darken and add wear to materials
        // This would be expanded with actual material parameter updates
        UE_LOG(LogTemp, VeryVerbose, TEXT("Applying weathering to structure"));
    }
}

bool UArch_StructuralManager::CanStructureSupportWeight(AActor* Structure, float Weight)
{
    float Stability = CalculateStructuralStability(Structure);
    float MaxSupportWeight = Stability * 10.0f; // 1000kg at 100% stability
    
    return Weight <= MaxSupportWeight;
}

void UArch_StructuralManager::InitializeStructureSystem()
{
    ActiveStructures.Empty();
    UE_LOG(LogTemp, Log, TEXT("Architecture Structural Manager initialized"));
}

FVector UArch_StructuralManager::GetBiomeCenter(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            return FVector(0.0f, 0.0f, 100.0f);
        case EBiomeType::Swamp:
            return FVector(-50000.0f, -45000.0f, 100.0f);
        case EBiomeType::Forest:
            return FVector(-45000.0f, 40000.0f, 100.0f);
        case EBiomeType::Desert:
            return FVector(55000.0f, 0.0f, 100.0f);
        case EBiomeType::Mountain:
            return FVector(40000.0f, 50000.0f, 100.0f);
        default:
            return FVector::ZeroVector;
    }
}

UStaticMeshComponent* UArch_StructuralManager::CreateStructureMesh(EArch_StructureType StructureType)
{
    UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>();
    
    // In a real implementation, this would load appropriate static meshes
    // For now, we'll use basic shapes as placeholders
    switch (StructureType)
    {
        case EArch_StructureType::CaveEntrance:
            // Load cave entrance mesh
            break;
        case EArch_StructureType::RockShelter:
            // Load rock shelter mesh
            break;
        case EArch_StructureType::StonePlatform:
            // Load stone platform mesh
            break;
        default:
            break;
    }
    
    return MeshComp;
}

void UArch_StructuralManager::ApplyBiomeSpecificMaterials(UStaticMeshComponent* MeshComp, EBiomeType BiomeType)
{
    if (!MeshComp)
    {
        return;
    }
    
    // Apply materials based on biome
    // This would load actual material assets in a full implementation
    switch (BiomeType)
    {
        case EBiomeType::Swamp:
            // Apply mossy, wet materials
            break;
        case EBiomeType::Desert:
            // Apply sandy, weathered materials
            break;
        case EBiomeType::Mountain:
            // Apply cold, rocky materials
            break;
        default:
            // Apply default stone materials
            break;
    }
}

bool UArch_StructuralManager::IsValidStructureLocation(FVector Location, EBiomeType BiomeType)
{
    // Check minimum distance from other structures
    for (const FArch_StructureData& ExistingStructure : ActiveStructures)
    {
        if (FVector::Dist(Location, ExistingStructure.Location) < MinDistanceBetweenStructures)
        {
            return false;
        }
    }
    
    // Check if location is within biome bounds
    FVector BiomeCenter = GetBiomeCenter(BiomeType);
    float DistanceFromCenter = FVector::Dist2D(Location, BiomeCenter);
    
    return DistanceFromCenter <= StructureSpawnRadius;
}

void UArch_StructuralManager::RegisterStructure(AActor* Structure, const FArch_StructureData& StructureData)
{
    if (Structure)
    {
        ActiveStructures.Add(StructureData);
        Structure->SetActorLabel(FString::Printf(TEXT("Structure_%s_%d"), 
            *UEnum::GetValueAsString(StructureData.StructureType), 
            ActiveStructures.Num()));
    }
}
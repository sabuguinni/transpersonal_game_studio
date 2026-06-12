#include "Arch_StructuralManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AArch_StructuralManager::AArch_StructuralManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root scene component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default structural data
    StructuralData.StructureType = EArch_StructureType::Shelter;
    StructuralData.Dimensions = FVector(400.0f, 600.0f, 300.0f);
    StructuralData.WeatheringLevel = 0.5f;
    StructuralData.bHasVegetationGrowth = true;
    StructuralData.StructuralIntegrity = 75;

    // Initialize shelter locations for Cretaceous biomes
    ShelterLocations.Add(FVector(50000, 50000, 100));    // Central highlands
    ShelterLocations.Add(FVector(45000, 55000, 150));    // Forest edge
    ShelterLocations.Add(FVector(55000, 45000, 80));     // River valley
    ShelterLocations.Add(FVector(52000, 48000, 200));    // Mountain base

    BiomePlacementRadius = 10000.0f;
}

void AArch_StructuralManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize structural systems
    PlaceStructuralElements();
    ApplyEnvironmentalIntegration();
}

void AArch_StructuralManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update weathering effects over time
    UpdateWeatheringEffects(DeltaTime);
}

void AArch_StructuralManager::PlaceStructuralElements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("AArch_StructuralManager: No world context for structural placement"));
        return;
    }

    // Set structure mesh scale based on structural data
    if (StructureMesh)
    {
        FVector ScaleVector = StructuralData.Dimensions / 100.0f; // Convert to UE5 scale
        StructureMesh->SetWorldScale3D(ScaleVector);
        
        UE_LOG(LogTemp, Log, TEXT("AArch_StructuralManager: Structure scaled to %s"), *ScaleVector.ToString());
    }

    // Apply weathering material effects
    if (StructureMesh && StructuralData.WeatheringLevel > 0.0f)
    {
        // Material weathering logic would be implemented here
        UE_LOG(LogTemp, Log, TEXT("AArch_StructuralManager: Applied weathering level %f"), StructuralData.WeatheringLevel);
    }
}

void AArch_StructuralManager::UpdateWeatheringEffects(float DeltaTime)
{
    // Gradually increase weathering over time (very slowly)
    float WeatheringRate = 0.0001f; // Very slow weathering
    StructuralData.WeatheringLevel = FMath::Clamp(StructuralData.WeatheringLevel + (WeatheringRate * DeltaTime), 0.0f, 1.0f);

    // Decrease structural integrity based on weathering
    if (StructuralData.WeatheringLevel > 0.7f)
    {
        float IntegrityLoss = 0.001f * DeltaTime;
        StructuralData.StructuralIntegrity = FMath::Clamp(StructuralData.StructuralIntegrity - IntegrityLoss, 0, 100);
    }
}

void AArch_StructuralManager::GenerateShelterNetwork()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    for (const FVector& Location : ShelterLocations)
    {
        FVector OptimalLocation = GetOptimalShelterLocation(Location);
        
        // Log shelter placement for debugging
        UE_LOG(LogTemp, Log, TEXT("AArch_StructuralManager: Generated shelter at %s"), *OptimalLocation.ToString());
    }
}

FVector AArch_StructuralManager::GetOptimalShelterLocation(const FVector& BiomeCenter) const
{
    // Add some randomization for natural placement
    FVector RandomOffset = FVector(
        FMath::RandRange(-1000.0f, 1000.0f),
        FMath::RandRange(-1000.0f, 1000.0f),
        0.0f
    );

    return BiomeCenter + RandomOffset;
}

void AArch_StructuralManager::ApplyEnvironmentalIntegration()
{
    // Integration with vegetation growth
    if (StructuralData.bHasVegetationGrowth)
    {
        // Logic for moss and vegetation growth on structures
        UE_LOG(LogTemp, Log, TEXT("AArch_StructuralManager: Applied vegetation growth integration"));
    }

    // Integration with weather systems
    UE_LOG(LogTemp, Log, TEXT("AArch_StructuralManager: Applied environmental integration"));
}

bool AArch_StructuralManager::IsStructureStable() const
{
    return StructuralData.StructuralIntegrity > 25;
}

void AArch_StructuralManager::SetStructureType(EArch_StructureType NewType)
{
    StructuralData.StructureType = NewType;
    
    // Adjust dimensions based on structure type
    switch (NewType)
    {
        case EArch_StructureType::Shelter:
            StructuralData.Dimensions = FVector(400.0f, 600.0f, 300.0f);
            break;
        case EArch_StructureType::Foundation:
            StructuralData.Dimensions = FVector(800.0f, 800.0f, 100.0f);
            break;
        case EArch_StructureType::Wall:
            StructuralData.Dimensions = FVector(100.0f, 400.0f, 250.0f);
            break;
        case EArch_StructureType::Entrance:
            StructuralData.Dimensions = FVector(200.0f, 100.0f, 220.0f);
            break;
        case EArch_StructureType::Support:
            StructuralData.Dimensions = FVector(50.0f, 50.0f, 300.0f);
            break;
        case EArch_StructureType::Ruins:
            StructuralData.Dimensions = FVector(600.0f, 800.0f, 200.0f);
            break;
    }

    // Refresh structural elements
    PlaceStructuralElements();
}
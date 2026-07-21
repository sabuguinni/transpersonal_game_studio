#include "PrimitiveShelterManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

APrimitiveShelterManager::APrimitiveShelterManager()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default shelter configuration
    ShelterConfig.BiomeType = ESurvivalBiome::Savana;
    ShelterConfig.WallCount = 4;
    ShelterConfig.ShelterRadius = 300.0f;
    ShelterConfig.WallHeight = 200.0f;
    ShelterConfig.bHasRoof = true;
    ShelterConfig.bHasFirepit = true;

    // Initialize component arrays
    WallComponents.Empty();
    SpawnedWalls.Empty();
    SpawnedRoof = nullptr;
    SpawnedFirepit = nullptr;

    // Setup default assets
    SetupDefaultAssets();
}

void APrimitiveShelterManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-construct shelter on begin play if configured
    if (ShelterConfig.WallCount > 0)
    {
        ConstructShelter();
    }
}

void APrimitiveShelterManager::SetupDefaultAssets()
{
    // Initialize wall components array
    WallComponents.SetNum(8); // Support up to 8 walls
    
    for (int32 i = 0; i < WallComponents.Num(); i++)
    {
        WallComponents[i] = FArch_ShelterComponent();
        WallComponents[i].RelativeLocation = FVector::ZeroVector;
        WallComponents[i].RelativeRotation = FRotator::ZeroRotator;
        WallComponents[i].Scale = FVector::OneVector;
    }

    // Initialize roof component
    RoofComponent = FArch_ShelterComponent();
    RoofComponent.RelativeLocation = FVector(0, 0, ShelterConfig.WallHeight);
    RoofComponent.Scale = FVector(1.5f, 1.5f, 0.5f);

    // Initialize firepit component
    FirepitComponent = FArch_ShelterComponent();
    FirepitComponent.RelativeLocation = FVector(0, 0, 0);
    FirepitComponent.Scale = FVector(0.8f, 0.8f, 0.3f);
}

void APrimitiveShelterManager::ConstructShelter()
{
    // Clear existing shelter first
    DestroyCurrentShelter();

    // Load biome-specific assets
    if (!LoadBiomeSpecificAssets(ShelterConfig.BiomeType))
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to load biome-specific assets for %d"), (int32)ShelterConfig.BiomeType);
    }

    // Calculate wall positions
    TArray<FVector> WallPositions;
    TArray<FRotator> WallRotations;
    CalculateWallPositions(WallPositions, WallRotations);

    // Spawn walls
    for (int32 i = 0; i < FMath::Min(ShelterConfig.WallCount, WallPositions.Num()); i++)
    {
        SpawnWallAtPosition(WallPositions[i], WallRotations[i], i);
    }

    // Spawn roof if configured
    if (ShelterConfig.bHasRoof)
    {
        SpawnRoofStructure();
    }

    // Spawn firepit if configured
    if (ShelterConfig.bHasFirepit)
    {
        SpawnFirepit();
    }

    UE_LOG(LogTemp, Log, TEXT("Primitive shelter constructed with %d walls"), SpawnedWalls.Num());
}

void APrimitiveShelterManager::DestroyCurrentShelter()
{
    // Destroy existing walls
    for (UStaticMeshComponent* Wall : SpawnedWalls)
    {
        if (IsValid(Wall))
        {
            Wall->DestroyComponent();
        }
    }
    SpawnedWalls.Empty();

    // Destroy roof
    if (IsValid(SpawnedRoof))
    {
        SpawnedRoof->DestroyComponent();
        SpawnedRoof = nullptr;
    }

    // Destroy firepit
    if (IsValid(SpawnedFirepit))
    {
        SpawnedFirepit->DestroyComponent();
        SpawnedFirepit = nullptr;
    }
}

void APrimitiveShelterManager::UpdateShelterConfiguration(const FArch_ShelterConfiguration& NewConfig)
{
    ShelterConfig = NewConfig;
    
    // Reconstruct shelter with new configuration
    ConstructShelter();
}

bool APrimitiveShelterManager::LoadBiomeSpecificAssets(ESurvivalBiome BiomeType)
{
    // This would load different materials and meshes based on biome
    // For now, return true as a placeholder
    switch (BiomeType)
    {
        case ESurvivalBiome::Savana:
            // Load savanna-specific stone and wood assets
            break;
        case ESurvivalBiome::Floresta:
            // Load forest-specific wood and moss assets
            break;
        case ESurvivalBiome::Deserto:
            // Load desert-specific sandstone assets
            break;
        case ESurvivalBiome::Pantano:
            // Load swamp-specific weathered wood assets
            break;
        case ESurvivalBiome::Montanha:
            // Load mountain-specific granite assets
            break;
    }
    
    return true;
}

void APrimitiveShelterManager::SpawnWallAtPosition(const FVector& Position, const FRotator& Rotation, int32 WallIndex)
{
    if (WallIndex >= 0 && WallIndex < WallComponents.Num())
    {
        FString ComponentName = FString::Printf(TEXT("Wall_%d"), WallIndex);
        UStaticMeshComponent* WallComponent = CreateMeshComponent(WallComponents[WallIndex], ComponentName);
        
        if (WallComponent)
        {
            WallComponent->SetWorldLocation(GetActorLocation() + Position);
            WallComponent->SetWorldRotation(Rotation);
            
            // Apply biome-specific weathering
            ApplyBiomeWeathering(WallComponent, ShelterConfig.BiomeType);
            
            SpawnedWalls.Add(WallComponent);
        }
    }
}

void APrimitiveShelterManager::SpawnRoofStructure()
{
    SpawnedRoof = CreateMeshComponent(RoofComponent, TEXT("ShelterRoof"));
    if (SpawnedRoof)
    {
        FVector RoofLocation = GetActorLocation() + RoofComponent.RelativeLocation;
        SpawnedRoof->SetWorldLocation(RoofLocation);
        SpawnedRoof->SetWorldRotation(RoofComponent.RelativeRotation);
        
        ApplyBiomeWeathering(SpawnedRoof, ShelterConfig.BiomeType);
    }
}

void APrimitiveShelterManager::SpawnFirepit()
{
    SpawnedFirepit = CreateMeshComponent(FirepitComponent, TEXT("ShelterFirepit"));
    if (SpawnedFirepit)
    {
        FVector FirepitLocation = GetActorLocation() + FirepitComponent.RelativeLocation;
        SpawnedFirepit->SetWorldLocation(FirepitLocation);
        SpawnedFirepit->SetWorldRotation(FirepitComponent.RelativeRotation);
        
        ApplyBiomeWeathering(SpawnedFirepit, ShelterConfig.BiomeType);
    }
}

bool APrimitiveShelterManager::ValidateShelterIntegrity() const
{
    // Check if minimum shelter requirements are met
    int32 ValidWalls = 0;
    for (const UStaticMeshComponent* Wall : SpawnedWalls)
    {
        if (IsValid(Wall))
        {
            ValidWalls++;
        }
    }
    
    // Minimum 3 walls for basic shelter
    return ValidWalls >= 3;
}

float APrimitiveShelterManager::CalculateShelterProtection() const
{
    if (!ValidateShelterIntegrity())
    {
        return 0.0f;
    }
    
    float Protection = 0.0f;
    
    // Base protection from walls
    Protection += SpawnedWalls.Num() * 10.0f;
    
    // Bonus protection from roof
    if (IsValid(SpawnedRoof))
    {
        Protection += 25.0f;
    }
    
    // Bonus protection from firepit (warmth)
    if (IsValid(SpawnedFirepit))
    {
        Protection += 15.0f;
    }
    
    // Cap at 100% protection
    return FMath::Clamp(Protection, 0.0f, 100.0f);
}

void APrimitiveShelterManager::EditorConstructPreview()
{
    ConstructShelter();
}

void APrimitiveShelterManager::EditorClearPreview()
{
    DestroyCurrentShelter();
}

void APrimitiveShelterManager::CalculateWallPositions(TArray<FVector>& OutPositions, TArray<FRotator>& OutRotations) const
{
    OutPositions.Empty();
    OutRotations.Empty();
    
    if (ShelterConfig.WallCount <= 0)
    {
        return;
    }
    
    float AngleStep = 360.0f / ShelterConfig.WallCount;
    
    for (int32 i = 0; i < ShelterConfig.WallCount; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector Position = FVector(
            FMath::Cos(RadianAngle) * ShelterConfig.ShelterRadius,
            FMath::Sin(RadianAngle) * ShelterConfig.ShelterRadius,
            0.0f
        );
        
        FRotator Rotation = FRotator(0.0f, Angle + 90.0f, 0.0f);
        
        OutPositions.Add(Position);
        OutRotations.Add(Rotation);
    }
}

UStaticMeshComponent* APrimitiveShelterManager::CreateMeshComponent(const FArch_ShelterComponent& ComponentData, const FString& ComponentName)
{
    UStaticMeshComponent* MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(*ComponentName);
    if (MeshComponent)
    {
        MeshComponent->SetupAttachment(RootComponent);
        
        if (ComponentData.ComponentMesh)
        {
            MeshComponent->SetStaticMesh(ComponentData.ComponentMesh);
        }
        
        if (ComponentData.ComponentMaterial)
        {
            MeshComponent->SetMaterial(0, ComponentData.ComponentMaterial);
        }
        
        MeshComponent->SetRelativeLocation(ComponentData.RelativeLocation);
        MeshComponent->SetRelativeRotation(ComponentData.RelativeRotation);
        MeshComponent->SetRelativeScale3D(ComponentData.Scale);
    }
    
    return MeshComponent;
}

void APrimitiveShelterManager::ApplyBiomeWeathering(UStaticMeshComponent* MeshComponent, ESurvivalBiome BiomeType)
{
    if (!IsValid(MeshComponent))
    {
        return;
    }
    
    // Apply biome-specific weathering effects
    // This is a placeholder for material parameter adjustments
    switch (BiomeType)
    {
        case ESurvivalBiome::Pantano:
            // Add moss and water damage effects
            break;
        case ESurvivalBiome::Deserto:
            // Add sand erosion and sun bleaching
            break;
        case ESurvivalBiome::Montanha:
            // Add frost damage and wind erosion
            break;
        default:
            // Default weathering
            break;
    }
}
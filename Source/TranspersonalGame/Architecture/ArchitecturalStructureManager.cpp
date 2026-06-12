#include "ArchitecturalStructureManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

AArchitecturalStructureManager::AArchitecturalStructureManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create structure mesh component
    StructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    StructureMesh->SetupAttachment(RootComponent);

    // Initialize default values
    StructureData.StructureType = EArch_StructureType::Foundation;
    StructureData.Material = EArch_ConstructionMaterial::Stone;
    StructureData.StructuralIntegrity = 100.0f;
    StructureData.WeatherResistance = 80.0f;
    StructureData.ConstructionCost = 10;
    StructureData.bIsCompleted = false;

    SnapRadius = 100.0f;
    bCanBeDestroyed = true;
    MaxHealth = 200.0f;
    CurrentHealth = MaxHealth;

    // Initialize snap points for foundation
    SnapPoints.Add(FVector(0, 0, 0));
    SnapPoints.Add(FVector(400, 0, 0));
    SnapPoints.Add(FVector(0, 600, 0));
    SnapPoints.Add(FVector(400, 600, 0));
}

void AArchitecturalStructureManager::BeginPlay()
{
    Super::BeginPlay();
    
    UpdateMeshMaterial();
    
    if (StructureData.bIsCompleted)
    {
        OnStructureCompleted();
    }
}

void AArchitecturalStructureManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (StructureData.bIsCompleted)
    {
        ApplyWeatherEffects(DeltaTime);
        UpdateStructuralIntegrity();
    }
}

void AArchitecturalStructureManager::InitializeStructure(EArch_StructureType Type, EArch_ConstructionMaterial Material)
{
    StructureData.StructureType = Type;
    StructureData.Material = Material;

    // Set material-specific properties
    switch (Material)
    {
        case EArch_ConstructionMaterial::Stone:
            StructureData.WeatherResistance = 90.0f;
            StructureData.ConstructionCost = 15;
            MaxHealth = 300.0f;
            break;
        case EArch_ConstructionMaterial::Wood:
            StructureData.WeatherResistance = 60.0f;
            StructureData.ConstructionCost = 8;
            MaxHealth = 150.0f;
            break;
        case EArch_ConstructionMaterial::Clay:
            StructureData.WeatherResistance = 70.0f;
            StructureData.ConstructionCost = 10;
            MaxHealth = 180.0f;
            break;
        case EArch_ConstructionMaterial::Bone:
            StructureData.WeatherResistance = 50.0f;
            StructureData.ConstructionCost = 12;
            MaxHealth = 120.0f;
            break;
        case EArch_ConstructionMaterial::Hide:
            StructureData.WeatherResistance = 40.0f;
            StructureData.ConstructionCost = 5;
            MaxHealth = 80.0f;
            break;
        case EArch_ConstructionMaterial::Thatch:
            StructureData.WeatherResistance = 30.0f;
            StructureData.ConstructionCost = 3;
            MaxHealth = 60.0f;
            break;
    }

    CurrentHealth = MaxHealth;
    UpdateMeshMaterial();

    UE_LOG(LogTemp, Log, TEXT("Initialized %s structure with %s material"), 
           *UEnum::GetValueAsString(Type), 
           *UEnum::GetValueAsString(Material));
}

bool AArchitecturalStructureManager::CanSnapToLocation(const FVector& Location) const
{
    for (const FVector& SnapPoint : SnapPoints)
    {
        FVector WorldSnapPoint = GetActorTransform().TransformPosition(SnapPoint);
        float Distance = FVector::Dist(Location, WorldSnapPoint);
        if (Distance <= SnapRadius)
        {
            return true;
        }
    }
    return false;
}

FVector AArchitecturalStructureManager::GetNearestSnapPoint(const FVector& Location) const
{
    if (SnapPoints.Num() == 0)
    {
        return GetActorLocation();
    }

    FVector NearestPoint = GetActorTransform().TransformPosition(SnapPoints[0]);
    float MinDistance = FVector::Dist(Location, NearestPoint);

    for (int32 i = 1; i < SnapPoints.Num(); i++)
    {
        FVector WorldSnapPoint = GetActorTransform().TransformPosition(SnapPoints[i]);
        float Distance = FVector::Dist(Location, WorldSnapPoint);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestPoint = WorldSnapPoint;
        }
    }

    return NearestPoint;
}

void AArchitecturalStructureManager::CompleteConstruction()
{
    StructureData.bIsCompleted = true;
    StructureData.StructuralIntegrity = 100.0f;
    
    UpdateMeshMaterial();
    OnStructureCompleted();

    UE_LOG(LogTemp, Log, TEXT("Construction completed for %s structure"), 
           *UEnum::GetValueAsString(StructureData.StructureType));
}

void AArchitecturalStructureManager::TakeDamage(float DamageAmount)
{
    if (!bCanBeDestroyed)
    {
        return;
    }

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);
    StructureData.StructuralIntegrity = (CurrentHealth / MaxHealth) * 100.0f;

    OnStructureDamaged(DamageAmount);

    if (CurrentHealth <= 0.0f)
    {
        OnStructureDestroyed();
        Destroy();
    }

    UpdateMeshMaterial();
}

bool AArchitecturalStructureManager::IsStructureStable() const
{
    return StructureData.StructuralIntegrity > 25.0f && StructureData.bIsCompleted;
}

void AArchitecturalStructureManager::UpdateStructuralIntegrity()
{
    // Gradual degradation over time based on weather resistance
    float DegradationRate = (100.0f - StructureData.WeatherResistance) * 0.001f;
    StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - DegradationRate);

    // Update health based on structural integrity
    CurrentHealth = (StructureData.StructuralIntegrity / 100.0f) * MaxHealth;

    if (StructureData.StructuralIntegrity <= 0.0f)
    {
        OnStructureDestroyed();
        Destroy();
    }
}

void AArchitecturalStructureManager::ApplyWeatherEffects(float DeltaTime)
{
    // Weather effects reduce structural integrity over time
    // This would be enhanced with actual weather system integration
    float WeatherDamage = (1.0f - (StructureData.WeatherResistance / 100.0f)) * DeltaTime * 0.1f;
    StructureData.StructuralIntegrity = FMath::Max(0.0f, StructureData.StructuralIntegrity - WeatherDamage);
}

void AArchitecturalStructureManager::UpdateMeshMaterial()
{
    if (!StructureMesh)
    {
        return;
    }

    // This would be enhanced with actual material switching based on:
    // - Construction material type
    // - Structural integrity level
    // - Completion status
    
    // For now, just log the material update
    UE_LOG(LogTemp, Log, TEXT("Updated material for %s structure (Integrity: %.1f%%)"), 
           *UEnum::GetValueAsString(StructureData.StructureType),
           StructureData.StructuralIntegrity);
}
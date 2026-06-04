#include "Arch_CretaceousStructureSystem.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"

AArch_CretaceousStructure::AArch_CretaceousStructure()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create main structure mesh
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    MainStructureMesh->SetupAttachment(RootComponent);

    // Create support beams mesh
    SupportBeamsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SupportBeamsMesh"));
    SupportBeamsMesh->SetupAttachment(RootComponent);

    // Create roof mesh
    RoofMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofMesh"));
    RoofMesh->SetupAttachment(RootComponent);

    // Initialize default structure config
    StructureConfig.StructureType = EArch_StructureType::ShelterBasic;
    StructureConfig.PrimaryMaterial = EArch_BuildMaterial::Wood;
    StructureConfig.StructuralIntegrity = 100.0f;
    StructureConfig.WeatherResistance = 50.0f;
    StructureConfig.MaxOccupants = 2;
    StructureConfig.bProvidesShelter = true;
    StructureConfig.bProvidesStorage = false;
    StructureConfig.bProvidesWarmth = false;

    // Initialize state
    CurrentIntegrity = StructureConfig.StructuralIntegrity;
    DamageAccumulated = 0.0f;
    bIsOccupied = false;
    CurrentOccupants = 0;
}

void AArch_CretaceousStructure::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeStructureComponents();
    ApplyMaterialBasedProperties();
    UpdateStructureMeshes();
}

void AArch_CretaceousStructure::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply gradual weather damage over time
    if (CurrentIntegrity > 0.0f)
    {
        float WeatherDamageRate = (100.0f - StructureConfig.WeatherResistance) * 0.001f; // Base damage per second
        ApplyWeatherDamage(WeatherDamageRate * DeltaTime);
    }

    // Check for structure collapse
    if (CurrentIntegrity <= 0.0f && bIsOccupied)
    {
        SetOccupancy(0);
        OnStructureDestroyed();
    }
}

void AArch_CretaceousStructure::ApplyWeatherDamage(float DamageAmount)
{
    if (DamageAmount <= 0.0f) return;

    DamageAccumulated += DamageAmount;
    CurrentIntegrity = FMath::Max(0.0f, CurrentIntegrity - DamageAmount);

    // Update visual state based on damage
    if (CurrentIntegrity < StructureConfig.StructuralIntegrity * 0.5f)
    {
        // Structure is heavily damaged - could trigger visual effects
        UpdateStructureMeshes();
    }
}

void AArch_CretaceousStructure::RepairStructure(float RepairAmount)
{
    if (RepairAmount <= 0.0f) return;

    CurrentIntegrity = FMath::Min(StructureConfig.StructuralIntegrity, CurrentIntegrity + RepairAmount);
    DamageAccumulated = FMath::Max(0.0f, DamageAccumulated - RepairAmount);

    UpdateStructureMeshes();
}

bool AArch_CretaceousStructure::CanAccommodateOccupants(int32 NumOccupants) const
{
    if (CurrentIntegrity <= 0.0f) return false;
    if (NumOccupants <= 0) return true;
    
    return (CurrentOccupants + NumOccupants) <= StructureConfig.MaxOccupants;
}

void AArch_CretaceousStructure::SetOccupancy(int32 NumOccupants)
{
    int32 PreviousOccupants = CurrentOccupants;
    CurrentOccupants = FMath::Clamp(NumOccupants, 0, StructureConfig.MaxOccupants);
    bIsOccupied = (CurrentOccupants > 0);

    if (PreviousOccupants != CurrentOccupants)
    {
        OnOccupancyChanged(CurrentOccupants);
    }
}

float AArch_CretaceousStructure::GetShelterEffectiveness() const
{
    if (CurrentIntegrity <= 0.0f) return 0.0f;

    float BaseEffectiveness = 1.0f;
    
    // Reduce effectiveness based on damage
    float IntegrityRatio = CurrentIntegrity / StructureConfig.StructuralIntegrity;
    BaseEffectiveness *= IntegrityRatio;

    // Material-based effectiveness
    switch (StructureConfig.PrimaryMaterial)
    {
        case EArch_BuildMaterial::Stone:
            BaseEffectiveness *= 1.2f; // Stone provides better shelter
            break;
        case EArch_BuildMaterial::Wood:
            BaseEffectiveness *= 1.0f; // Wood is baseline
            break;
        case EArch_BuildMaterial::Mud:
            BaseEffectiveness *= 0.8f; // Mud is less effective
            break;
        case EArch_BuildMaterial::Hide:
            BaseEffectiveness *= 0.9f; // Hide is decent but not great
            break;
        case EArch_BuildMaterial::Thatch:
            BaseEffectiveness *= 0.7f; // Thatch is basic
            break;
        default:
            break;
    }

    // Weather resistance affects shelter effectiveness
    BaseEffectiveness *= (StructureConfig.WeatherResistance / 100.0f);

    return FMath::Clamp(BaseEffectiveness, 0.0f, 1.5f);
}

void AArch_CretaceousStructure::UpdateStructureMeshes()
{
    if (!MainStructureMesh) return;

    // Get appropriate mesh based on structure type and material
    UStaticMesh* NewMesh = GetMeshForStructureType();
    if (NewMesh && MainStructureMesh->GetStaticMesh() != NewMesh)
    {
        MainStructureMesh->SetStaticMesh(NewMesh);
    }

    // Update material based on primary material and damage state
    UStaticMesh* MaterialMesh = GetMeshForMaterial(StructureConfig.PrimaryMaterial);
    if (MaterialMesh && SupportBeamsMesh)
    {
        SupportBeamsMesh->SetStaticMesh(MaterialMesh);
    }

    // Adjust mesh opacity/color based on damage
    float IntegrityRatio = CurrentIntegrity / StructureConfig.StructuralIntegrity;
    if (IntegrityRatio < 0.5f)
    {
        // Structure is heavily damaged - could apply damaged material
        // This would typically involve material parameter collection updates
    }
}

void AArch_CretaceousStructure::InitializeStructureComponents()
{
    if (!MainStructureMesh || !SupportBeamsMesh || !RoofMesh) return;

    // Set collision for main structure
    MainStructureMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MainStructureMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    MainStructureMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    // Support beams typically don't need collision
    SupportBeamsMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // Roof might need collision for weather protection
    RoofMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    RoofMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
}

void AArch_CretaceousStructure::ApplyMaterialBasedProperties()
{
    // Adjust structure properties based on primary material
    switch (StructureConfig.PrimaryMaterial)
    {
        case EArch_BuildMaterial::Stone:
            StructureConfig.StructuralIntegrity = FMath::Max(StructureConfig.StructuralIntegrity, 150.0f);
            StructureConfig.WeatherResistance = FMath::Max(StructureConfig.WeatherResistance, 80.0f);
            break;
            
        case EArch_BuildMaterial::Wood:
            StructureConfig.StructuralIntegrity = FMath::Max(StructureConfig.StructuralIntegrity, 100.0f);
            StructureConfig.WeatherResistance = FMath::Max(StructureConfig.WeatherResistance, 60.0f);
            break;
            
        case EArch_BuildMaterial::Mud:
            StructureConfig.StructuralIntegrity = FMath::Max(StructureConfig.StructuralIntegrity, 70.0f);
            StructureConfig.WeatherResistance = FMath::Max(StructureConfig.WeatherResistance, 40.0f);
            break;
            
        case EArch_BuildMaterial::Hide:
            StructureConfig.StructuralIntegrity = FMath::Max(StructureConfig.StructuralIntegrity, 60.0f);
            StructureConfig.WeatherResistance = FMath::Max(StructureConfig.WeatherResistance, 70.0f);
            break;
            
        case EArch_BuildMaterial::Thatch:
            StructureConfig.StructuralIntegrity = FMath::Max(StructureConfig.StructuralIntegrity, 50.0f);
            StructureConfig.WeatherResistance = FMath::Max(StructureConfig.WeatherResistance, 30.0f);
            break;
            
        default:
            break;
    }

    // Update current integrity to match new structural integrity
    CurrentIntegrity = StructureConfig.StructuralIntegrity;
}

void AArch_CretaceousStructure::CalculateWeatherResistance()
{
    float BaseResistance = 50.0f;
    
    // Material contributes to weather resistance
    switch (StructureConfig.PrimaryMaterial)
    {
        case EArch_BuildMaterial::Stone:
            BaseResistance += 30.0f;
            break;
        case EArch_BuildMaterial::Wood:
            BaseResistance += 10.0f;
            break;
        case EArch_BuildMaterial::Hide:
            BaseResistance += 20.0f;
            break;
        default:
            break;
    }

    // Structure type affects weather resistance
    switch (StructureConfig.StructureType)
    {
        case EArch_StructureType::ShelterAdvanced:
            BaseResistance += 20.0f;
            break;
        case EArch_StructureType::CaveEntrance:
            BaseResistance += 40.0f; // Caves are naturally weather-resistant
            break;
        default:
            break;
    }

    StructureConfig.WeatherResistance = FMath::Clamp(BaseResistance, 0.0f, 100.0f);
}

UStaticMesh* AArch_CretaceousStructure::GetMeshForStructureType() const
{
    // In a real implementation, this would load specific meshes based on structure type
    // For now, return nullptr and let the designer set meshes manually
    // This would typically use LoadObject<UStaticMesh> with specific asset paths
    return nullptr;
}

UStaticMesh* AArch_CretaceousStructure::GetMeshForMaterial(EArch_BuildMaterial Material) const
{
    // In a real implementation, this would load material-specific meshes
    // For now, return nullptr and let the designer set meshes manually
    // This would typically use LoadObject<UStaticMesh> with specific asset paths
    return nullptr;
}
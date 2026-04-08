// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architecture Types Implementation
// Agent #07 - Architecture & Interior Agent

#include "ArchitectureTypes.h"
#include "Engine/Engine.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

UArchitectureDataAsset::UArchitectureDataAsset()
{
    // Initialize default architecture presets
    InitializeDefaultPresets();
}

void UArchitectureDataAsset::InitializeDefaultPresets()
{
    ArchitecturePresets.Empty();

    // SURVIVAL STRUCTURES
    FArchitectureSpec BasicShelterSpec;
    BasicShelterSpec.Type = EArchitectureType::BasicShelter;
    BasicShelterSpec.PrimaryMaterial = EConstructionMaterial::Wood;
    BasicShelterSpec.SecondaryMaterial = EConstructionMaterial::Leaves;
    BasicShelterSpec.Dimensions = FVector(400.0f, 300.0f, 250.0f);
    BasicShelterSpec.StructuralIntegrity = 0.6f;
    BasicShelterSpec.WeatherResistance = 0.4f;
    BasicShelterSpec.bIsDefensive = false;
    BasicShelterSpec.bIsAbandoned = false;
    BasicShelterSpec.InteriorSpaces = {EInteriorType::SleepingArea, EInteriorType::StorageArea};
    ArchitecturePresets.Add(BasicShelterSpec);

    FArchitectureSpec ReinforcedHutSpec;
    ReinforcedHutSpec.Type = EArchitectureType::ReinforcedHut;
    ReinforcedHutSpec.PrimaryMaterial = EConstructionMaterial::StoneAndWood;
    ReinforcedHutSpec.SecondaryMaterial = EConstructionMaterial::Mud;
    ReinforcedHutSpec.Dimensions = FVector(500.0f, 400.0f, 300.0f);
    ReinforcedHutSpec.StructuralIntegrity = 0.8f;
    ReinforcedHutSpec.WeatherResistance = 0.7f;
    ReinforcedHutSpec.bIsDefensive = true;
    ReinforcedHutSpec.bIsAbandoned = false;
    ReinforcedHutSpec.InteriorSpaces = {EInteriorType::SleepingArea, EInteriorType::CookingArea, EInteriorType::StorageArea, EInteriorType::WorkArea};
    ArchitecturePresets.Add(ReinforcedHutSpec);

    FArchitectureSpec WatchTowerSpec;
    WatchTowerSpec.Type = EArchitectureType::WatchTower;
    WatchTowerSpec.PrimaryMaterial = EConstructionMaterial::Stone;
    WatchTowerSpec.SecondaryMaterial = EConstructionMaterial::Wood;
    WatchTowerSpec.Dimensions = FVector(300.0f, 300.0f, 800.0f);
    WatchTowerSpec.StructuralIntegrity = 0.9f;
    WatchTowerSpec.WeatherResistance = 0.8f;
    WatchTowerSpec.bIsDefensive = true;
    WatchTowerSpec.bIsAbandoned = false;
    WatchTowerSpec.InteriorSpaces = {EInteriorType::LookoutPost, EInteriorType::StorageArea};
    ArchitecturePresets.Add(WatchTowerSpec);

    // DEFENSIVE STRUCTURES
    FArchitectureSpec PalisadeSpec;
    PalisadeSpec.Type = EArchitectureType::Palisade;
    PalisadeSpec.PrimaryMaterial = EConstructionMaterial::Wood;
    PalisadeSpec.SecondaryMaterial = EConstructionMaterial::Stone;
    PalisadeSpec.Dimensions = FVector(1000.0f, 50.0f, 400.0f);
    PalisadeSpec.StructuralIntegrity = 0.7f;
    PalisadeSpec.WeatherResistance = 0.5f;
    PalisadeSpec.bIsDefensive = true;
    PalisadeSpec.bIsAbandoned = false;
    ArchitecturePresets.Add(PalisadeSpec);

    // UTILITY STRUCTURES
    FArchitectureSpec FirePitSpec;
    FirePitSpec.Type = EArchitectureType::FirePit;
    FirePitSpec.PrimaryMaterial = EConstructionMaterial::Stone;
    FirePitSpec.SecondaryMaterial = EConstructionMaterial::Mud;
    FirePitSpec.Dimensions = FVector(200.0f, 200.0f, 50.0f);
    FirePitSpec.StructuralIntegrity = 1.0f;
    FirePitSpec.WeatherResistance = 0.9f;
    FirePitSpec.bIsDefensive = false;
    FirePitSpec.bIsAbandoned = false;
    FirePitSpec.InteriorSpaces = {EInteriorType::CookingArea};
    ArchitecturePresets.Add(FirePitSpec);

    // RUINS & ABANDONED
    FArchitectureSpec AbandonedCampSpec;
    AbandonedCampSpec.Type = EArchitectureType::AbandonedCamp;
    AbandonedCampSpec.PrimaryMaterial = EConstructionMaterial::Wood;
    AbandonedCampSpec.SecondaryMaterial = EConstructionMaterial::Leaves;
    AbandonedCampSpec.Dimensions = FVector(600.0f, 500.0f, 200.0f);
    AbandonedCampSpec.StructuralIntegrity = 0.3f;
    AbandonedCampSpec.WeatherResistance = 0.2f;
    AbandonedCampSpec.bIsDefensive = false;
    AbandonedCampSpec.bIsAbandoned = true;
    AbandonedCampSpec.InteriorSpaces = {EInteriorType::EmptyDwelling, EInteriorType::RansackedShelter};
    ArchitecturePresets.Add(AbandonedCampSpec);

    UE_LOG(LogTemp, Log, TEXT("ArchitectureDataAsset: Initialized %d default presets"), ArchitecturePresets.Num());
}

FArchitectureSpec UArchitectureDataAsset::GetPresetByType(EArchitectureType Type) const
{
    for (const FArchitectureSpec& Preset : ArchitecturePresets)
    {
        if (Preset.Type == Type)
        {
            return Preset;
        }
    }

    // Return default if not found
    UE_LOG(LogTemp, Warning, TEXT("ArchitectureDataAsset: Preset not found for type %d, returning default"), (int32)Type);
    return FArchitectureSpec();
}

TArray<FArchitectureSpec> UArchitectureDataAsset::GetPresetsByMaterial(EConstructionMaterial Material) const
{
    TArray<FArchitectureSpec> MatchingPresets;
    
    for (const FArchitectureSpec& Preset : ArchitecturePresets)
    {
        if (Preset.PrimaryMaterial == Material || Preset.SecondaryMaterial == Material)
        {
            MatchingPresets.Add(Preset);
        }
    }

    return MatchingPresets;
}

TArray<FArchitectureSpec> UArchitectureDataAsset::GetDefensiveStructures() const
{
    TArray<FArchitectureSpec> DefensiveStructures;
    
    for (const FArchitectureSpec& Preset : ArchitecturePresets)
    {
        if (Preset.bIsDefensive)
        {
            DefensiveStructures.Add(Preset);
        }
    }

    return DefensiveStructures;
}

TArray<FArchitectureSpec> UArchitectureDataAsset::GetAbandonedStructures() const
{
    TArray<FArchitectureSpec> AbandonedStructures;
    
    for (const FArchitectureSpec& Preset : ArchitecturePresets)
    {
        if (Preset.bIsAbandoned)
        {
            AbandonedStructures.Add(Preset);
        }
    }

    return AbandonedStructures;
}

UMaterialInterface* UArchitectureDataAsset::GetMaterialForType(EConstructionMaterial MaterialType) const
{
    if (const TSoftObjectPtr<UMaterialInterface>* FoundMaterial = MaterialMap.Find(MaterialType))
    {
        return FoundMaterial->LoadSynchronous();
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitectureDataAsset: Material not found for type %d"), (int32)MaterialType);
    return nullptr;
}

UStaticMesh* UArchitectureDataAsset::GetMeshForArchitectureType(EArchitectureType ArchType) const
{
    if (const TSoftObjectPtr<UStaticMesh>* FoundMesh = MeshMap.Find(ArchType))
    {
        return FoundMesh->LoadSynchronous();
    }

    UE_LOG(LogTemp, Warning, TEXT("ArchitectureDataAsset: Mesh not found for architecture type %d"), (int32)ArchType);
    return nullptr;
}

void UArchitectureDataAsset::ValidatePresets()
{
    int32 ValidPresets = 0;
    int32 InvalidPresets = 0;

    for (const FArchitectureSpec& Preset : ArchitecturePresets)
    {
        bool bIsValid = true;

        // Check dimensions
        if (Preset.Dimensions.X <= 0 || Preset.Dimensions.Y <= 0 || Preset.Dimensions.Z <= 0)
        {
            UE_LOG(LogTemp, Error, TEXT("ArchitectureDataAsset: Invalid dimensions for preset type %d"), (int32)Preset.Type);
            bIsValid = false;
        }

        // Check integrity values
        if (Preset.StructuralIntegrity < 0.0f || Preset.StructuralIntegrity > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("ArchitectureDataAsset: Invalid structural integrity for preset type %d"), (int32)Preset.Type);
            bIsValid = false;
        }

        if (Preset.WeatherResistance < 0.0f || Preset.WeatherResistance > 1.0f)
        {
            UE_LOG(LogTemp, Error, TEXT("ArchitectureDataAsset: Invalid weather resistance for preset type %d"), (int32)Preset.Type);
            bIsValid = false;
        }

        if (bIsValid)
        {
            ValidPresets++;
        }
        else
        {
            InvalidPresets++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ArchitectureDataAsset: Validation complete - %d valid, %d invalid presets"), ValidPresets, InvalidPresets);
}
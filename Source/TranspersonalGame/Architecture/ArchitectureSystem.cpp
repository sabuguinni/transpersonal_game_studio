// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architecture System Implementation
// Agent #07 - Architecture & Interior Agent

#include "ArchitectureSystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"

AArchitectureSystem::AArchitectureSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Create main structure mesh component
    MainStructureMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainStructureMesh"));
    RootComponent = MainStructureMesh;
    
    // Set default values
    StructureType = EArchitectureType::BasicShelter;
    Condition = EStructureCondition::Good;
    
    // Initialize environmental details
    EnvironmentalDetails.bHasExtinguishedFireplace = false;
    EnvironmentalDetails.bHasAbandonedTools = false;
    EnvironmentalDetails.bHasFoodRemains = false;
    EnvironmentalDetails.bHasFootprints = false;
    EnvironmentalDetails.bHasWallMarkings = false;
    EnvironmentalDetails.bHasInvasiveVegetation = false;
    EnvironmentalDetails.bHasAnimalDamage = false;
    EnvironmentalDetails.bHasForgottenItems = false;
    EnvironmentalDetails.AbandonmentLevel = 0.0f;
}

void AArchitectureSystem::BeginPlay()
{
    Super::BeginPlay();
    
    LoadStructureConfiguration();
    ApplyEnvironmentalDetails();
    ApplyWeatheringEffects();
}

void AArchitectureSystem::InitializeStructure(EArchitectureType Type, EStructureCondition InitialCondition)
{
    StructureType = Type;
    Condition = InitialCondition;
    
    LoadStructureConfiguration();
    SpawnDetailMeshes();
    ApplyEnvironmentalDetails();
    ApplyWeatheringEffects();
}

void AArchitectureSystem::ApplyEnvironmentalDetails()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Apply environmental storytelling based on details
    if (EnvironmentalDetails.bHasInvasiveVegetation)
    {
        // Spawn vegetation on structure
        // Implementation would spawn ivy, moss, etc.
    }
    
    if (EnvironmentalDetails.bHasAnimalDamage)
    {
        // Apply damage materials/decals
        // Implementation would add scratch marks, bite marks, etc.
    }
    
    if (EnvironmentalDetails.AbandonmentLevel > 0.5f)
    {
        // Apply abandonment effects
        // Implementation would add dust, cobwebs, decay, etc.
    }
}

void AArchitectureSystem::SetCondition(EStructureCondition NewCondition)
{
    Condition = NewCondition;
    ApplyWeatheringEffects();
}

bool AArchitectureSystem::CanPlayerEnter() const
{
    // Player can enter if structure is not completely collapsed
    return Condition != EStructureCondition::Collapsed;
}

float AArchitectureSystem::GetDefenseRating() const
{
    if (!ArchitectureConfigTable)
    {
        return 0.0f;
    }
    
    // Find configuration for current structure type
    FString ContextString;
    FArchitectureConfig* Config = ArchitectureConfigTable->FindRow<FArchitectureConfig>(
        FName(*FString::FromInt((int32)StructureType)), ContextString);
    
    if (Config)
    {
        // Adjust defense rating based on condition
        float ConditionMultiplier = 1.0f;
        switch (Condition)
        {
            case EStructureCondition::Perfect:
                ConditionMultiplier = 1.0f;
                break;
            case EStructureCondition::Good:
                ConditionMultiplier = 0.8f;
                break;
            case EStructureCondition::Damaged:
                ConditionMultiplier = 0.6f;
                break;
            case EStructureCondition::Ruined:
                ConditionMultiplier = 0.3f;
                break;
            case EStructureCondition::Collapsed:
                ConditionMultiplier = 0.0f;
                break;
        }
        
        return Config->DefenseRating * ConditionMultiplier;
    }
    
    return 0.0f;
}

void AArchitectureSystem::LoadStructureConfiguration()
{
    if (!ArchitectureConfigTable)
    {
        return;
    }
    
    FString ContextString;
    FArchitectureConfig* Config = ArchitectureConfigTable->FindRow<FArchitectureConfig>(
        FName(*FString::FromInt((int32)StructureType)), ContextString);
    
    if (Config && MainStructureMesh)
    {
        // Load primary mesh
        if (Config->PrimaryMesh.IsValid())
        {
            MainStructureMesh->SetStaticMesh(Config->PrimaryMesh.LoadSynchronous());
        }
    }
}

void AArchitectureSystem::SpawnDetailMeshes()
{
    if (!ArchitectureConfigTable)
    {
        return;
    }
    
    FString ContextString;
    FArchitectureConfig* Config = ArchitectureConfigTable->FindRow<FArchitectureConfig>(
        FName(*FString::FromInt((int32)StructureType)), ContextString);
    
    if (Config)
    {
        // Spawn detail meshes
        for (const TSoftObjectPtr<UStaticMesh>& DetailMesh : Config->DetailMeshes)
        {
            if (DetailMesh.IsValid())
            {
                UStaticMeshComponent* DetailComponent = CreateDefaultSubobject<UStaticMeshComponent>(
                    *FString::Printf(TEXT("DetailMesh_%d"), Config->DetailMeshes.Find(DetailMesh)));
                DetailComponent->SetStaticMesh(DetailMesh.LoadSynchronous());
                DetailComponent->AttachToComponent(MainStructureMesh, 
                    FAttachmentTransformRules::KeepRelativeTransform);
            }
        }
    }
}

void AArchitectureSystem::ApplyWeatheringEffects()
{
    if (!MainStructureMesh)
    {
        return;
    }
    
    // Create dynamic material instance for weathering effects
    UMaterialInterface* BaseMaterial = MainStructureMesh->GetMaterial(0);
    if (BaseMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
        
        // Apply condition-based weathering
        float WeatheringAmount = 0.0f;
        switch (Condition)
        {
            case EStructureCondition::Perfect:
                WeatheringAmount = 0.0f;
                break;
            case EStructureCondition::Good:
                WeatheringAmount = 0.2f;
                break;
            case EStructureCondition::Damaged:
                WeatheringAmount = 0.5f;
                break;
            case EStructureCondition::Ruined:
                WeatheringAmount = 0.8f;
                break;
            case EStructureCondition::Collapsed:
                WeatheringAmount = 1.0f;
                break;
        }
        
        // Set material parameters for weathering
        DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), WeatheringAmount);
        DynamicMaterial->SetScalarParameterValue(TEXT("AbandonmentLevel"), EnvironmentalDetails.AbandonmentLevel);
        
        MainStructureMesh->SetMaterial(0, DynamicMaterial);
    }
}
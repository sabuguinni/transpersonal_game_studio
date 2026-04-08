// Copyright Transpersonal Game Studio. All Rights Reserved.
// Architecture Subsystem Implementation
// Agent #07 - Architecture & Interior Agent

#include "ArchitectureSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "PCGComponent.h"
#include "PCGSubsystem.h"

UArchitectureSubsystem::UArchitectureSubsystem()
{
    // Initialize default values
}

void UArchitectureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Architecture Subsystem Initialized"));
    
    // Initialize performance tracking
    PerformanceStats.ActiveStructures = 0;
    PerformanceStats.RenderedTriangles = 0;
    PerformanceStats.MemoryUsageMB = 0.0f;
    PerformanceStats.LastUpdateTime = FDateTime::Now();
    
    // Load default architecture configurations
    LoadArchitectureConfigurations();
}

void UArchitectureSubsystem::Deinitialize()
{
    // Cleanup all structures
    ClearAllStructures();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture Subsystem Deinitialized"));
    
    Super::Deinitialize();
}

bool UArchitectureSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UArchitectureSubsystem::GenerateArchitecture(const FArchitectureGenerationSettings& Settings)
{
    UE_LOG(LogTemp, Log, TEXT("Generating Architecture with settings"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No valid world found for architecture generation"));
        return;
    }
    
    // Clear existing structures in the generation area if requested
    if (Settings.bClearExisting)
    {
        ClearRegion(Settings.GenerationBounds);
    }
    
    // Generate structures based on biome and terrain data
    GenerateStructuresInRegion(Settings);
    
    // Update performance statistics
    UpdatePerformanceStats();
    
    UE_LOG(LogTemp, Log, TEXT("Architecture generation completed. Active structures: %d"), 
           PerformanceStats.ActiveStructures);
}

void UArchitectureSubsystem::RefreshRegion(const FBox& WorldBounds)
{
    UE_LOG(LogTemp, Log, TEXT("Refreshing architecture in region"));
    
    // Find all structures in the region
    TArray<AActor*> StructuresInRegion;
    FindStructuresInBounds(WorldBounds, StructuresInRegion);
    
    // Refresh each structure's state
    for (AActor* Structure : StructuresInRegion)
    {
        if (Structure)
        {
            RefreshStructureState(Structure);
        }
    }
}

void UArchitectureSubsystem::ClearAllStructures()
{
    UE_LOG(LogTemp, Log, TEXT("Clearing all architecture structures"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all architecture actors and destroy them
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Architecture")))
        {
            Actor->Destroy();
        }
    }
    
    // Reset performance stats
    PerformanceStats.ActiveStructures = 0;
    PerformanceStats.RenderedTriangles = 0;
    PerformanceStats.MemoryUsageMB = 0.0f;
}

AActor* UArchitectureSubsystem::ConstructBuilding(const FBuildingConstructionData& ConstructionData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }
    
    // Spawn new building actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* NewBuilding = World->SpawnActor<AActor>(
        AActor::StaticClass(),
        ConstructionData.Location,
        ConstructionData.Rotation,
        SpawnParams
    );
    
    if (NewBuilding)
    {
        // Set building scale
        NewBuilding->SetActorScale3D(ConstructionData.Scale);
        
        // Add static mesh component
        UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(NewBuilding);
        NewBuilding->SetRootComponent(MeshComponent);
        
        // Load appropriate mesh based on building type
        LoadBuildingMesh(NewBuilding, ConstructionData.BuildingType);
        
        // Apply structural condition effects
        ApplyStructuralCondition(NewBuilding, ConstructionData.Condition);
        
        // Apply weathering based on age
        ApplyAgeWeathering(NewBuilding, ConstructionData.AgeInDays);
        
        // Generate interior if requested
        if (ConstructionData.bGenerateInterior)
        {
            FInteriorFurnishingData InteriorData;
            InteriorData.BuildingType = ConstructionData.BuildingType;
            InteriorData.bIsAbandoned = ConstructionData.bIsAbandoned;
            InteriorData.AbandonmentReason = ConstructionData.AbandonmentReason;
            
            FurnishInterior(NewBuilding, InteriorData);
        }
        
        // Register the building
        RegisterStructure(NewBuilding);
        
        UE_LOG(LogTemp, Log, TEXT("Constructed building at location: %s"), 
               *ConstructionData.Location.ToString());
    }
    
    return NewBuilding;
}

void UArchitectureSubsystem::FurnishInterior(AActor* Building, const FInteriorFurnishingData& FurnishingData)
{
    if (!Building)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Furnishing interior for building"));
    
    // Get building bounds for interior placement
    FBox BuildingBounds = Building->GetComponentsBoundingBox();
    FVector InteriorCenter = BuildingBounds.GetCenter();
    FVector InteriorExtent = BuildingBounds.GetExtent() * 0.8f; // Slightly smaller than building
    
    // Generate interior zones based on building type
    TArray<FInteriorZone> InteriorZones;
    GenerateInteriorZones(FurnishingData.BuildingType, InteriorCenter, InteriorExtent, InteriorZones);
    
    // Place furniture and props in each zone
    for (const FInteriorZone& Zone : InteriorZones)
    {
        PlaceFurnitureInZone(Building, Zone, FurnishingData);
    }
    
    // Add storytelling elements if not abandoned
    if (!FurnishingData.bIsAbandoned)
    {
        AddStorytellingElements(Building, FurnishingData);
    }
    else
    {
        AddAbandonmentElements(Building, FurnishingData);
    }
}

void UArchitectureSubsystem::ConvertToRuins(AActor* Building, const FRuinsConversionData& ConversionData)
{
    if (!Building)
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Converting building to ruins"));
    
    // Apply ruins materials
    ApplyRuinsMaterials(Building, ConversionData.RuinsStage);
    
    // Add vegetation overgrowth
    if (ConversionData.bAddVegetationOvergrowth)
    {
        AddVegetationOvergrowth(Building, ConversionData.OvergrowthIntensity);
    }
    
    // Add structural damage
    if (ConversionData.bAddStructuralDamage)
    {
        AddStructuralDamage(Building, ConversionData.DamageIntensity);
    }
    
    // Scatter debris around the structure
    if (ConversionData.bScatterDebris)
    {
        ScatterDebrisAroundStructure(Building, ConversionData.DebrisRadius);
    }
}

void UArchitectureSubsystem::ApplyWeathering(AActor* Structure, float WeatheringIntensity)
{
    if (!Structure)
    {
        return;
    }
    
    // Get all mesh components
    TArray<UStaticMeshComponent*> MeshComponents;
    Structure->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            // Create dynamic material instance for weathering
            UMaterialInterface* BaseMaterial = MeshComp->GetMaterial(0);
            if (BaseMaterial)
            {
                UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Structure);
                
                // Apply weathering parameters
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringAmount"), WeatheringIntensity);
                DynamicMaterial->SetScalarParameterValue(TEXT("DirtAccumulation"), WeatheringIntensity * 0.7f);
                DynamicMaterial->SetScalarParameterValue(TEXT("MossGrowth"), WeatheringIntensity * 0.5f);
                
                MeshComp->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void UArchitectureSubsystem::GenerateSettlement(const FSettlementData& SettlementData)
{
    UE_LOG(LogTemp, Log, TEXT("Generating settlement: %s"), *SettlementData.SettlementName);
    
    // Create settlement layout based on type
    TArray<FBuildingPlacement> BuildingPlacements;
    GenerateSettlementLayout(SettlementData, BuildingPlacements);
    
    // Construct each building in the settlement
    for (const FBuildingPlacement& Placement : BuildingPlacements)
    {
        FBuildingConstructionData ConstructionData;
        ConstructionData.BuildingType = Placement.BuildingType;
        ConstructionData.Location = Placement.Location;
        ConstructionData.Rotation = Placement.Rotation;
        ConstructionData.Scale = Placement.Scale;
        ConstructionData.Condition = Placement.Condition;
        ConstructionData.bGenerateInterior = true;
        ConstructionData.bIsAbandoned = SettlementData.bIsAbandoned;
        
        AActor* NewBuilding = ConstructBuilding(ConstructionData);
        if (NewBuilding)
        {
            // Tag building as part of this settlement
            NewBuilding->Tags.Add(FName(*FString::Printf(TEXT("Settlement_%s"), *SettlementData.SettlementName)));
        }
    }
    
    // Add settlement-wide features (paths, central fire pit, etc.)
    AddSettlementFeatures(SettlementData);
}

FArchitecturePerformanceStats UArchitectureSubsystem::GetPerformanceStats() const
{
    return PerformanceStats;
}

// Private helper methods

void UArchitectureSubsystem::LoadArchitectureConfigurations()
{
    // Load architecture configuration data tables
    // This would load from data assets in a full implementation
    UE_LOG(LogTemp, Log, TEXT("Loading architecture configurations"));
}

void UArchitectureSubsystem::GenerateStructuresInRegion(const FArchitectureGenerationSettings& Settings)
{
    // Implementation for generating structures based on PCG and biome data
    UE_LOG(LogTemp, Log, TEXT("Generating structures in region"));
}

void UArchitectureSubsystem::ClearRegion(const FBox& Bounds)
{
    TArray<AActor*> StructuresInRegion;
    FindStructuresInBounds(Bounds, StructuresInRegion);
    
    for (AActor* Structure : StructuresInRegion)
    {
        if (Structure)
        {
            Structure->Destroy();
        }
    }
}

void UArchitectureSubsystem::FindStructuresInBounds(const FBox& Bounds, TArray<AActor*>& OutStructures)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Bounds.IsInside(Actor->GetActorLocation()))
        {
            if (Actor->GetClass()->GetName().Contains(TEXT("Architecture")) ||
                Actor->Tags.Contains(TEXT("Structure")))
            {
                OutStructures.Add(Actor);
            }
        }
    }
}

void UArchitectureSubsystem::RefreshStructureState(AActor* Structure)
{
    if (!Structure)
    {
        return;
    }
    
    // Refresh LOD, materials, and other dynamic properties
    UE_LOG(LogTemp, VeryVerbose, TEXT("Refreshing structure state for: %s"), *Structure->GetName());
}

void UArchitectureSubsystem::LoadBuildingMesh(AActor* Building, EArchitectureType BuildingType)
{
    // Load appropriate static mesh based on building type
    // This would use a data table lookup in a full implementation
}

void UArchitectureSubsystem::ApplyStructuralCondition(AActor* Building, EStructuralCondition Condition)
{
    // Apply visual effects based on structural condition
}

void UArchitectureSubsystem::ApplyAgeWeathering(AActor* Building, float AgeInDays)
{
    float WeatheringAmount = FMath::Clamp(AgeInDays / 365.0f, 0.0f, 1.0f); // 1 year = full weathering
    ApplyWeathering(Building, WeatheringAmount);
}

void UArchitectureSubsystem::RegisterStructure(AActor* Structure)
{
    if (Structure)
    {
        PerformanceStats.ActiveStructures++;
        Structure->Tags.Add(TEXT("ManagedStructure"));
    }
}

void UArchitectureSubsystem::GenerateInteriorZones(EArchitectureType BuildingType, const FVector& Center, 
                                                 const FVector& Extent, TArray<FInteriorZone>& OutZones)
{
    // Generate interior zones based on building type
    // This would create functional areas like sleeping, cooking, storage, etc.
}

void UArchitectureSubsystem::PlaceFurnitureInZone(AActor* Building, const FInteriorZone& Zone, 
                                                const FInteriorFurnishingData& FurnishingData)
{
    // Place furniture and props appropriate for the zone type
}

void UArchitectureSubsystem::AddStorytellingElements(AActor* Building, const FInteriorFurnishingData& FurnishingData)
{
    // Add props and details that tell the story of who lived here
}

void UArchitectureSubsystem::AddAbandonmentElements(AActor* Building, const FInteriorFurnishingData& FurnishingData)
{
    // Add elements that show abandonment and decay
}

void UArchitectureSubsystem::ApplyRuinsMaterials(AActor* Building, ERuinsStage RuinsStage)
{
    // Apply materials appropriate for the ruins stage
}

void UArchitectureSubsystem::AddVegetationOvergrowth(AActor* Building, float OvergrowthIntensity)
{
    // Add vegetation that has grown over the structure
}

void UArchitectureSubsystem::AddStructuralDamage(AActor* Building, float DamageIntensity)
{
    // Add visual damage to the structure
}

void UArchitectureSubsystem::ScatterDebrisAroundStructure(AActor* Building, float DebrisRadius)
{
    // Scatter debris and rubble around the structure
}

void UArchitectureSubsystem::GenerateSettlementLayout(const FSettlementData& SettlementData, 
                                                    TArray<FBuildingPlacement>& OutPlacements)
{
    // Generate layout for settlement buildings
}

void UArchitectureSubsystem::AddSettlementFeatures(const FSettlementData& SettlementData)
{
    // Add paths, central areas, defensive features, etc.
}

void UArchitectureSubsystem::UpdatePerformanceStats()
{
    PerformanceStats.LastUpdateTime = FDateTime::Now();
    
    // Update triangle count and memory usage
    // This would calculate actual rendering statistics in a full implementation
}
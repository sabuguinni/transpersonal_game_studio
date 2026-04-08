// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "ArchitectureImplementationSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Landscape/Landscape.h"
#include "PCGComponent.h"
#include "PCGGraph.h"
#include "PCGData.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchitectureImplementation, Log, All);

UArchitectureImplementationSystem::UArchitectureImplementationSystem()
{
    // Constructor implementation
}

void UArchitectureImplementationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Architecture Implementation System Initialized"));
    
    // Initialize implementation stats
    ImplementationStats = FArchitectureImplementationStats();
    ImplementationStats.TotalStructuresImplemented = 0;
    ImplementationStats.TotalInteriorsGenerated = 0;
    ImplementationStats.TotalPerformanceOptimizations = 0;
    ImplementationStats.AverageImplementationTime = 0.0f;
}

void UArchitectureImplementationSystem::Deinitialize()
{
    // Clean up any active implementation tasks
    ClearAllImplementedStructures();
    
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Architecture Implementation System Deinitialized"));
    
    Super::Deinitialize();
}

bool UArchitectureImplementationSystem::ShouldCreateSubsystem(UObject* Outer) const
{
    // Only create in game worlds, not in editor preview worlds
    if (UWorld* World = Cast<UWorld>(Outer))
    {
        return World->IsGameWorld();
    }
    return false;
}

void UArchitectureImplementationSystem::ImplementArchitecture(const FArchitectureImplementationSettings& Settings)
{
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Starting Architecture Implementation"));
    
    double StartTime = FPlatformTime::Seconds();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogArchitectureImplementation, Error, TEXT("No valid world found for architecture implementation"));
        return;
    }
    
    // Clear existing structures if requested
    if (Settings.bClearExistingStructures)
    {
        ClearAllImplementedStructures();
    }
    
    // Implement structures based on settings
    for (const FStructureImplementationData& StructureData : Settings.StructuresToImplement)
    {
        FStructureConstructionData ConstructionData;
        ConstructionData.StructureType = StructureData.StructureType;
        ConstructionData.Location = StructureData.Location;
        ConstructionData.Rotation = StructureData.Rotation;
        ConstructionData.Scale = StructureData.Scale;
        ConstructionData.Integrity = StructureData.Integrity;
        ConstructionData.AgeInDays = StructureData.AgeInDays;
        ConstructionData.bAdaptToTerrain = StructureData.bAdaptToTerrain;
        ConstructionData.bGenerateInterior = StructureData.bGenerateInterior;
        ConstructionData.bIsAbandoned = StructureData.bIsAbandoned;
        ConstructionData.AbandonmentReason = StructureData.AbandonmentReason;
        
        AActor* ConstructedStructure = ConstructStructure(ConstructionData);
        if (ConstructedStructure)
        {
            ImplementedStructures.Add(ConstructedStructure);
            ImplementationStats.TotalStructuresImplemented++;
        }
    }
    
    // Apply performance optimizations
    if (Settings.bApplyPerformanceOptimizations)
    {
        OptimizeArchitecturePerformance(Settings.PerformanceSettings);
    }
    
    double EndTime = FPlatformTime::Seconds();
    ImplementationStats.AverageImplementationTime = (EndTime - StartTime);
    
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Architecture Implementation completed in %.2f seconds"), 
           ImplementationStats.AverageImplementationTime);
}

void UArchitectureImplementationSystem::RefreshArchitectureInRegion(const FBox& WorldBounds)
{
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Refreshing Architecture in Region"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Find all structures in the specified region
    TArray<AActor*> StructuresInRegion;
    for (AActor* Structure : ImplementedStructures)
    {
        if (Structure && WorldBounds.IsInside(Structure->GetActorLocation()))
        {
            StructuresInRegion.Add(Structure);
        }
    }
    
    // Refresh each structure
    for (AActor* Structure : StructuresInRegion)
    {
        // Re-apply weathering based on current conditions
        FWeatheringSettings WeatheringSettings;
        WeatheringSettings.WeatheringIntensity = 0.5f;
        WeatheringSettings.bApplyRainDamage = true;
        WeatheringSettings.bApplyWindErosion = true;
        WeatheringSettings.bApplyUVDamage = true;
        
        ApplyWeatheringToStructure(Structure, WeatheringSettings);
    }
}

void UArchitectureImplementationSystem::ClearAllImplementedStructures()
{
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Clearing all implemented structures"));
    
    for (AActor* Structure : ImplementedStructures)
    {
        if (Structure && IsValid(Structure))
        {
            Structure->Destroy();
        }
    }
    
    ImplementedStructures.Empty();
    ImplementationStats.TotalStructuresImplemented = 0;
}

AActor* UArchitectureImplementationSystem::ConstructStructure(const FStructureConstructionData& ConstructionData)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogArchitectureImplementation, Error, TEXT("No valid world for structure construction"));
        return nullptr;
    }
    
    // Create the structure actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* StructureActor = World->SpawnActor<AActor>(AActor::StaticClass(), 
                                                       ConstructionData.Location, 
                                                       ConstructionData.Rotation, 
                                                       SpawnParams);
    
    if (!StructureActor)
    {
        UE_LOG(LogArchitectureImplementation, Error, TEXT("Failed to spawn structure actor"));
        return nullptr;
    }
    
    // Set the structure scale
    StructureActor->SetActorScale3D(ConstructionData.Scale);
    
    // Add static mesh component based on structure type
    UStaticMeshComponent* MeshComponent = StructureActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StructureMesh"));
    if (MeshComponent)
    {
        StructureActor->SetRootComponent(MeshComponent);
        
        // Load appropriate mesh based on structure type
        UStaticMesh* StructureMesh = LoadStructureMeshForType(ConstructionData.StructureType);
        if (StructureMesh)
        {
            MeshComponent->SetStaticMesh(StructureMesh);
        }
        
        // Apply weathering materials based on integrity
        ApplyIntegrityMaterials(MeshComponent, ConstructionData.Integrity);
    }
    
    // Adapt to terrain if requested
    if (ConstructionData.bAdaptToTerrain)
    {
        FTerrainAdaptationSettings TerrainSettings;
        TerrainSettings.bAlignToSlope = true;
        TerrainSettings.bSinkIntoGround = true;
        TerrainSettings.SinkDepth = 10.0f;
        AdaptStructureToTerrain(StructureActor, TerrainSettings);
    }
    
    // Generate interior if requested
    if (ConstructionData.bGenerateInterior)
    {
        FInteriorImplementationData InteriorData;
        InteriorData.InteriorType = GetInteriorTypeForStructure(ConstructionData.StructureType);
        InteriorData.bIsAbandoned = ConstructionData.bIsAbandoned;
        InteriorData.AbandonmentReason = ConstructionData.AbandonmentReason;
        InteriorData.AgeInDays = ConstructionData.AgeInDays;
        
        ImplementInterior(StructureActor, InteriorData);
    }
    
    // Apply weathering based on age
    if (ConstructionData.AgeInDays > 0.0f)
    {
        SimulateDecayOverTime(StructureActor, ConstructionData.AgeInDays);
    }
    
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Successfully constructed structure of type %d at location %s"), 
           (int32)ConstructionData.StructureType, *ConstructionData.Location.ToString());
    
    return StructureActor;
}

void UArchitectureImplementationSystem::AdaptStructureToTerrain(AActor* Structure, const FTerrainAdaptationSettings& Settings)
{
    if (!Structure)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector StructureLocation = Structure->GetActorLocation();
    
    // Perform line trace to find ground
    FHitResult HitResult;
    FVector TraceStart = StructureLocation + FVector(0, 0, 1000.0f);
    FVector TraceEnd = StructureLocation - FVector(0, 0, 1000.0f);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    QueryParams.AddIgnoredActor(Structure);
    
    if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, QueryParams))
    {
        FVector GroundLocation = HitResult.Location;
        
        // Sink into ground if requested
        if (Settings.bSinkIntoGround)
        {
            GroundLocation.Z -= Settings.SinkDepth;
        }
        
        Structure->SetActorLocation(GroundLocation);
        
        // Align to slope if requested
        if (Settings.bAlignToSlope)
        {
            FVector GroundNormal = HitResult.Normal;
            FRotator AlignedRotation = UKismetMathLibrary::MakeRotFromZX(GroundNormal, Structure->GetActorForwardVector());
            Structure->SetActorRotation(AlignedRotation);
        }
    }
}

void UArchitectureImplementationSystem::ImplementInterior(AActor* Structure, const FInteriorImplementationData& InteriorData)
{
    if (!Structure)
    {
        return;
    }
    
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Implementing interior for structure"));
    
    // Create interior storytelling props
    FStorytellingPropsData PropsData;
    PropsData.StoryCategory = GetStorytellingCategoryForInterior(InteriorData);
    PropsData.bIsAbandoned = InteriorData.bIsAbandoned;
    PropsData.AbandonmentReason = InteriorData.AbandonmentReason;
    PropsData.PropDensity = InteriorData.PropDensity;
    
    PopulateInteriorWithStorytellingProps(Structure, PropsData);
    
    ImplementationStats.TotalInteriorsGenerated++;
}

void UArchitectureImplementationSystem::PopulateInteriorWithStorytellingProps(AActor* Structure, const FStorytellingPropsData& PropsData)
{
    if (!Structure)
    {
        return;
    }
    
    // Get structure bounds for prop placement
    FBox StructureBounds = Structure->GetComponentsBoundingBox();
    FVector StructureCenter = StructureBounds.GetCenter();
    FVector StructureExtent = StructureBounds.GetExtent();
    
    // Generate props based on story category
    TArray<FPropPlacementData> PropsToPlace = GeneratePropsForStoryCategory(PropsData.StoryCategory, PropsData);
    
    for (const FPropPlacementData& PropData : PropsToPlace)
    {
        // Calculate placement location within structure bounds
        FVector PropLocation = StructureCenter + FVector(
            FMath::RandRange(-StructureExtent.X * 0.8f, StructureExtent.X * 0.8f),
            FMath::RandRange(-StructureExtent.Y * 0.8f, StructureExtent.Y * 0.8f),
            -StructureExtent.Z * 0.9f  // Place on floor
        );
        
        // Spawn prop actor
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = Structure;
        
        AActor* PropActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), PropLocation, FRotator::ZeroRotator, SpawnParams);
        if (PropActor)
        {
            // Add mesh component for the prop
            UStaticMeshComponent* PropMesh = PropActor->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PropMesh"));
            if (PropMesh)
            {
                PropActor->SetRootComponent(PropMesh);
                
                // Load appropriate prop mesh
                UStaticMesh* Mesh = LoadPropMeshForType(PropData.PropType);
                if (Mesh)
                {
                    PropMesh->SetStaticMesh(Mesh);
                }
            }
        }
    }
}

void UArchitectureImplementationSystem::ApplyWeatheringToStructure(AActor* Structure, const FWeatheringSettings& WeatheringSettings)
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
        if (!MeshComp)
        {
            continue;
        }
        
        // Create dynamic material instance for weathering
        UMaterialInterface* OriginalMaterial = MeshComp->GetMaterial(0);
        if (OriginalMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(OriginalMaterial, MeshComp);
            if (DynamicMaterial)
            {
                // Apply weathering parameters
                DynamicMaterial->SetScalarParameterValue(TEXT("WeatheringIntensity"), WeatheringSettings.WeatheringIntensity);
                DynamicMaterial->SetScalarParameterValue(TEXT("RainDamage"), WeatheringSettings.bApplyRainDamage ? 1.0f : 0.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("WindErosion"), WeatheringSettings.bApplyWindErosion ? 1.0f : 0.0f);
                DynamicMaterial->SetScalarParameterValue(TEXT("UVDamage"), WeatheringSettings.bApplyUVDamage ? 1.0f : 0.0f);
                
                MeshComp->SetMaterial(0, DynamicMaterial);
            }
        }
    }
}

void UArchitectureImplementationSystem::SimulateDecayOverTime(AActor* Structure, float TimeInDays)
{
    if (!Structure || TimeInDays <= 0.0f)
    {
        return;
    }
    
    // Calculate decay intensity based on time
    float DecayIntensity = FMath::Clamp(TimeInDays / 365.0f, 0.0f, 1.0f); // Max decay after 1 year
    
    // Apply weathering based on decay
    FWeatheringSettings WeatheringSettings;
    WeatheringSettings.WeatheringIntensity = DecayIntensity;
    WeatheringSettings.bApplyRainDamage = true;
    WeatheringSettings.bApplyWindErosion = true;
    WeatheringSettings.bApplyUVDamage = true;
    
    ApplyWeatheringToStructure(Structure, WeatheringSettings);
}

void UArchitectureImplementationSystem::ImplementSettlement(const FSettlementImplementationData& SettlementData)
{
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Implementing settlement"));
    
    // Implementation for settlement creation
    for (const FStructureImplementationData& StructureData : SettlementData.Structures)
    {
        FStructureConstructionData ConstructionData;
        // Convert StructureImplementationData to StructureConstructionData
        ConstructionData.StructureType = StructureData.StructureType;
        ConstructionData.Location = StructureData.Location;
        ConstructionData.Rotation = StructureData.Rotation;
        ConstructionData.Scale = StructureData.Scale;
        
        AActor* Structure = ConstructStructure(ConstructionData);
        if (Structure)
        {
            ImplementedStructures.Add(Structure);
        }
    }
}

void UArchitectureImplementationSystem::OptimizeArchitecturePerformance(const FArchitecturePerformanceSettings& Settings)
{
    UE_LOG(LogArchitectureImplementation, Log, TEXT("Optimizing Architecture Performance"));
    
    // Apply LOD settings
    for (AActor* Structure : ImplementedStructures)
    {
        if (!Structure)
        {
            continue;
        }
        
        TArray<UStaticMeshComponent*> MeshComponents;
        Structure->GetComponents<UStaticMeshComponent>(MeshComponents);
        
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (MeshComp)
            {
                // Set LOD distances
                MeshComp->SetForcedLodModel(Settings.ForcedLODLevel);
                
                // Apply culling settings
                MeshComp->SetCullDistance(Settings.MaxDrawDistance);
            }
        }
    }
    
    ImplementationStats.TotalPerformanceOptimizations++;
}

FArchitectureImplementationStats UArchitectureImplementationSystem::GetImplementationStats() const
{
    return ImplementationStats;
}

// Helper functions implementation

UStaticMesh* UArchitectureImplementationSystem::LoadStructureMeshForType(EArchitectureType StructureType)
{
    // Implementation to load appropriate mesh based on structure type
    // This would typically load from a data table or asset registry
    return nullptr; // Placeholder
}

void UArchitectureImplementationSystem::ApplyIntegrityMaterials(UStaticMeshComponent* MeshComponent, EStructuralIntegrity Integrity)
{
    if (!MeshComponent)
    {
        return;
    }
    
    // Apply materials based on structural integrity
    // This would load different material variants for different integrity levels
}

EInteriorType UArchitectureImplementationSystem::GetInteriorTypeForStructure(EArchitectureType StructureType)
{
    // Map structure types to interior types
    switch (StructureType)
    {
        case EArchitectureType::BasicShelter:
            return EInteriorType::BasicLiving;
        case EArchitectureType::LongHouse:
            return EInteriorType::CommunalLiving;
        default:
            return EInteriorType::BasicLiving;
    }
}

EInteriorStoryCategory UArchitectureImplementationSystem::GetStorytellingCategoryForInterior(const FInteriorImplementationData& InteriorData)
{
    if (InteriorData.bIsAbandoned)
    {
        switch (InteriorData.AbandonmentReason)
        {
            case EAbandonmentReason::PredatorAttack:
                return EInteriorStoryCategory::ViolentAbandonement;
            case EAbandonmentReason::PlannedEvacuation:
                return EInteriorStoryCategory::GradualAbandonement;
            default:
                return EInteriorStoryCategory::HurriedException;
        }
    }
    
    return EInteriorStoryCategory::SleepingArrangements;
}

TArray<FPropPlacementData> UArchitectureImplementationSystem::GeneratePropsForStoryCategory(EInteriorStoryCategory Category, const FStorytellingPropsData& PropsData)
{
    TArray<FPropPlacementData> Props;
    
    // Generate props based on story category
    switch (Category)
    {
        case EInteriorStoryCategory::SleepingArrangements:
            // Add sleeping furs, bedrolls, etc.
            break;
        case EInteriorStoryCategory::FoodPreparation:
            // Add cooking tools, food remains, etc.
            break;
        case EInteriorStoryCategory::ViolentAbandonement:
            // Add scattered items, damage signs, etc.
            break;
        default:
            break;
    }
    
    return Props;
}

UStaticMesh* UArchitectureImplementationSystem::LoadPropMeshForType(EPropType PropType)
{
    // Load appropriate prop mesh based on type
    return nullptr; // Placeholder
}
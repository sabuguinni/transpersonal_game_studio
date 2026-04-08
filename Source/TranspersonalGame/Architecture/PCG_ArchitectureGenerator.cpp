// Architecture & Interior Agent — Transpersonal Game Studio
// PROD_JURASSIC_001 — PCG Architecture Generator Implementation
// Agent #07 — Architecture & Interior Agent

#include "PCG_ArchitectureGenerator.h"
#include "PCGContext.h"
#include "Data/PCGSpatialData.h"
#include "Data/PCGPointData.h"
#include "Helpers/PCGAsync.h"
#include "Helpers/PCGHelpers.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"

#define LOCTEXT_NAMESPACE "PCGArchitectureGenerator"

UPCGArchitectureSettings::UPCGArchitectureSettings()
{
    // Set default values that make sense for prehistoric survival architecture
}

TArray<FPCGPinProperties> UPCGArchitectureSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    
    // Terrain input from Environment Artist
    PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Spatial);
    
    // Water proximity data
    PinProperties.Emplace(TEXT("WaterProximity"), EPCGDataType::Spatial);
    
    // Vegetation density data
    PinProperties.Emplace(TEXT("VegetationDensity"), EPCGDataType::Spatial);
    
    return PinProperties;
}

TArray<FPCGPinProperties> UPCGArchitectureSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    
    // Architecture points with metadata
    PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
    
    // Interior layout points
    PinProperties.Emplace(TEXT("InteriorPoints"), EPCGDataType::Point);
    
    // Defensive structure points
    PinProperties.Emplace(TEXT("DefensivePoints"), EPCGDataType::Point);
    
    return PinProperties;
}

#if WITH_EDITOR
FText UPCGArchitectureSettings::GetNodeTooltipText() const
{
    return LOCTEXT("ArchitectureGeneratorTooltip", "Generates prehistoric survival architecture based on environmental conditions and narrative requirements");
}
#endif

FPCGElementPtr UPCGArchitectureSettings::CreateElement() const
{
    return MakeShared<FPCGArchitectureElement>();
}

UPCGNode* UPCGArchitectureSettings::CreateNode() const
{
    return NewObject<UPCGNode>();
}

FPCGContext* FPCGArchitectureElement::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
    FPCGContext* Context = new FPCGContext();
    Context->InputData = InputData;
    Context->SourceComponent = SourceComponent;
    Context->Node = Node;
    
    return Context;
}

bool FPCGArchitectureElement::ExecuteInternal(FPCGContext* Context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGArchitectureElement::Execute);
    
    const UPCGArchitectureSettings* Settings = Context->GetInputSettings<UPCGArchitectureSettings>();
    if (!Settings)
    {
        PCGE_LOG(Error, GraphAndLog, LOCTEXT("SettingsError", "Unable to retrieve settings"));
        return true;
    }

    // Generate architecture points
    if (!GenerateArchitecturePoints(Context, Settings))
    {
        PCGE_LOG(Error, GraphAndLog, LOCTEXT("GenerationError", "Failed to generate architecture points"));
        return true;
    }

    return true;
}

bool FPCGArchitectureElement::GenerateArchitecturePoints(FPCGContext* Context, const UPCGArchitectureSettings* Settings) const
{
    // Get input spatial data
    const FPCGDataCollection& InputData = Context->InputData;
    TArray<FPCGTaggedData> Inputs = InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
    
    if (Inputs.IsEmpty())
    {
        PCGE_LOG(Warning, GraphAndLog, LOCTEXT("NoInputData", "No input spatial data found"));
        return false;
    }

    // Create output point data
    UPCGPointData* OutputPointData = NewObject<UPCGPointData>();
    TArray<FPCGPoint>& OutputPoints = OutputPointData->GetMutablePoints();

    UPCGPointData* InteriorPointData = NewObject<UPCGPointData>();
    TArray<FPCGPoint>& InteriorPoints = InteriorPointData->GetMutablePoints();

    UPCGPointData* DefensivePointData = NewObject<UPCGPointData>();
    TArray<FPCGPoint>& DefensivePoints = DefensivePointData->GetMutablePoints();

    // Process each input spatial data
    for (const FPCGTaggedData& Input : Inputs)
    {
        const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
        if (!SpatialData)
        {
            continue;
        }

        // Sample points from spatial data for potential architecture locations
        const UPCGPointData* SampledPoints = SpatialData->ToPointData(Context);
        if (!SampledPoints)
        {
            continue;
        }

        // Filter and process points for architecture placement
        for (const FPCGPoint& Point : SampledPoints->GetPoints())
        {
            // Analyze environmental suitability
            float Suitability = AnalyzeEnvironmentalSuitability(Point.Transform.GetLocation(), Settings, Context);
            
            // Apply density filtering
            if (FMath::RandRange(0.0f, 1.0f) > Settings->StructureDensity * Suitability)
            {
                continue;
            }

            // Check minimum distance to existing structures
            if (!IsValidPlacementLocation(Point.Transform.GetLocation(), OutputPoints, Settings->MinDistanceBetweenStructures))
            {
                continue;
            }

            // Create architecture point
            FPCGPoint ArchPoint = Point;
            
            // Determine architecture type based on location
            EArchitectureType ArchType = DetermineArchitectureType(Point.Transform.GetLocation(), Settings, Context);
            EConstructionMaterial PrimaryMaterial = DeterminePrimaryMaterial(Point.Transform.GetLocation(), Context);
            
            // Set optimal orientation
            ArchPoint.Transform.SetRotation(CalculateOptimalOrientation(Point.Transform.GetLocation(), Context).Quaternion());
            
            // Add metadata
            ArchPoint.SetLocalAttribute(TEXT("ArchitectureType"), PCGMetadataHelpers::MakeAttribute((int32)ArchType));
            ArchPoint.SetLocalAttribute(TEXT("PrimaryMaterial"), PCGMetadataHelpers::MakeAttribute((int32)PrimaryMaterial));
            ArchPoint.SetLocalAttribute(TEXT("Suitability"), PCGMetadataHelpers::MakeAttribute(Suitability));
            
            // Apply narrative elements
            ApplyNarrativeElements(ArchPoint, Settings, Context);
            
            // Add to appropriate output
            OutputPoints.Add(ArchPoint);
            
            // Generate interior layout
            GenerateInteriorLayout(ArchPoint, ArchType, Settings);
            
            // Add to specialized outputs based on type
            if (ArchType == EArchitectureType::Palisade || ArchType == EArchitectureType::Barricade || 
                ArchType == EArchitectureType::TrapPit || ArchType == EArchitectureType::SpikeTrap)
            {
                DefensivePoints.Add(ArchPoint);
            }
        }
    }

    // Add outputs to context
    FPCGTaggedData& OutputTaggedData = Context->OutputData.TaggedData.Emplace_GetRef();
    OutputTaggedData.Data = OutputPointData;
    OutputTaggedData.Pin = PCGPinConstants::DefaultOutputLabel;

    FPCGTaggedData& InteriorTaggedData = Context->OutputData.TaggedData.Emplace_GetRef();
    InteriorTaggedData.Data = InteriorPointData;
    InteriorTaggedData.Pin = TEXT("InteriorPoints");

    FPCGTaggedData& DefensiveTaggedData = Context->OutputData.TaggedData.Emplace_GetRef();
    DefensiveTaggedData.Data = DefensivePointData;
    DefensiveTaggedData.Pin = TEXT("DefensivePoints");

    PCGE_LOG(Log, LogOnly, FText::Format(LOCTEXT("GenerationComplete", "Generated {0} architecture points"), OutputPoints.Num()));

    return true;
}

float FPCGArchitectureElement::AnalyzeEnvironmentalSuitability(const FVector& Location, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const
{
    float Suitability = 1.0f;
    
    // Analyze slope - flatter areas are better for construction
    // This would need landscape data access in a real implementation
    float Slope = 0.0f; // Placeholder
    Suitability *= FMath::Lerp(1.0f, 0.1f, FMath::Clamp(Slope / 45.0f, 0.0f, 1.0f));
    
    // Analyze elevation - higher ground is better for defense
    float Elevation = Location.Z;
    float ElevationFactor = FMath::Clamp(Elevation / 1000.0f, 0.0f, 1.0f);
    Suitability *= FMath::Lerp(0.5f, 1.0f, ElevationFactor) * Settings->ElevationInfluence;
    
    // Water proximity - closer to water is better for survival
    // This would analyze water proximity data in a real implementation
    float WaterDistance = 1000.0f; // Placeholder
    float WaterFactor = FMath::Clamp(1.0f - (WaterDistance / 2000.0f), 0.0f, 1.0f);
    Suitability *= FMath::Lerp(0.3f, 1.0f, WaterFactor) * Settings->WaterProximityInfluence;
    
    return FMath::Clamp(Suitability, 0.0f, 1.0f);
}

EArchitectureType FPCGArchitectureElement::DetermineArchitectureType(const FVector& Location, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const
{
    float RandomValue = FMath::RandRange(0.0f, 1.0f);
    float CumulativeWeight = 0.0f;
    
    // Shelter structures
    CumulativeWeight += Settings->ShelterWeight;
    if (RandomValue <= CumulativeWeight)
    {
        TArray<EArchitectureType> ShelterTypes = {
            EArchitectureType::BasicShelter,
            EArchitectureType::ReinforcedHut,
            EArchitectureType::WatchTower
        };
        return ShelterTypes[FMath::RandRange(0, ShelterTypes.Num() - 1)];
    }
    
    // Defensive structures
    CumulativeWeight += Settings->DefensiveWeight;
    if (RandomValue <= CumulativeWeight)
    {
        TArray<EArchitectureType> DefensiveTypes = {
            EArchitectureType::Palisade,
            EArchitectureType::Barricade,
            EArchitectureType::TrapPit,
            EArchitectureType::SpikeTrap
        };
        return DefensiveTypes[FMath::RandRange(0, DefensiveTypes.Num() - 1)];
    }
    
    // Utility structures
    CumulativeWeight += Settings->UtilityWeight;
    if (RandomValue <= CumulativeWeight)
    {
        TArray<EArchitectureType> UtilityTypes = {
            EArchitectureType::FirePit,
            EArchitectureType::WorkBench,
            EArchitectureType::DryingRack,
            EArchitectureType::WaterCollector,
            EArchitectureType::StoragePit
        };
        return UtilityTypes[FMath::RandRange(0, UtilityTypes.Num() - 1)];
    }
    
    // Abandoned structures
    TArray<EArchitectureType> AbandonedTypes = {
        EArchitectureType::AbandonedCamp,
        EArchitectureType::CollapsedShelter,
        EArchitectureType::BuriedStructure,
        EArchitectureType::AncientRuins
    };
    return AbandonedTypes[FMath::RandRange(0, AbandonedTypes.Num() - 1)];
}

EConstructionMaterial FPCGArchitectureElement::DeterminePrimaryMaterial(const FVector& Location, FPCGContext* Context) const
{
    // Determine material based on local environment
    // This would analyze biome data in a real implementation
    
    TArray<EConstructionMaterial> AvailableMaterials = {
        EConstructionMaterial::Wood,
        EConstructionMaterial::Stone,
        EConstructionMaterial::Mud,
        EConstructionMaterial::WoodAndMud
    };
    
    return AvailableMaterials[FMath::RandRange(0, AvailableMaterials.Num() - 1)];
}

void FPCGArchitectureElement::ApplyNarrativeElements(FPCGPoint& Point, const UPCGArchitectureSettings* Settings, FPCGContext* Context) const
{
    bool bIsAbandoned = FMath::RandRange(0.0f, 1.0f) < Settings->AbandonmentChance;
    bool bIsPartiallyDestroyed = FMath::RandRange(0.0f, 1.0f) < Settings->PartialDestructionChance;
    bool bIsOvergrown = FMath::RandRange(0.0f, 1.0f) < Settings->OvergrowthChance;
    
    Point.SetLocalAttribute(TEXT("IsAbandoned"), PCGMetadataHelpers::MakeAttribute(bIsAbandoned));
    Point.SetLocalAttribute(TEXT("IsPartiallyDestroyed"), PCGMetadataHelpers::MakeAttribute(bIsPartiallyDestroyed));
    Point.SetLocalAttribute(TEXT("IsOvergrown"), PCGMetadataHelpers::MakeAttribute(bIsOvergrown));
    
    // Reduce structural integrity for damaged structures
    float StructuralIntegrity = 1.0f;
    if (bIsAbandoned) StructuralIntegrity *= 0.6f;
    if (bIsPartiallyDestroyed) StructuralIntegrity *= 0.4f;
    if (bIsOvergrown) StructuralIntegrity *= 0.7f;
    
    Point.SetLocalAttribute(TEXT("StructuralIntegrity"), PCGMetadataHelpers::MakeAttribute(StructuralIntegrity));
}

void FPCGArchitectureElement::GenerateInteriorLayout(FPCGPoint& Point, EArchitectureType ArchType, const UPCGArchitectureSettings* Settings) const
{
    // Generate interior points based on architecture type
    // This is a simplified version - full implementation would create detailed interior layouts
    
    TArray<EInteriorType> PossibleInteriors;
    
    switch (ArchType)
    {
        case EArchitectureType::BasicShelter:
        case EArchitectureType::ReinforcedHut:
            PossibleInteriors = { EInteriorType::SleepingArea, EInteriorType::StorageArea };
            break;
            
        case EArchitectureType::WatchTower:
            PossibleInteriors = { EInteriorType::LookoutPost };
            break;
            
        case EArchitectureType::AbandonedCamp:
            PossibleInteriors = { EInteriorType::EmptyDwelling, EInteriorType::RansackedShelter };
            break;
            
        default:
            break;
    }
    
    // Store interior types as metadata
    FString InteriorTypesString;
    for (int32 i = 0; i < PossibleInteriors.Num(); ++i)
    {
        InteriorTypesString += FString::FromInt((int32)PossibleInteriors[i]);
        if (i < PossibleInteriors.Num() - 1)
        {
            InteriorTypesString += TEXT(",");
        }
    }
    
    Point.SetLocalAttribute(TEXT("InteriorTypes"), PCGMetadataHelpers::MakeAttribute(InteriorTypesString));
}

bool FPCGArchitectureElement::IsValidPlacementLocation(const FVector& Location, const TArray<FPCGPoint>& ExistingPoints, float MinDistance) const
{
    for (const FPCGPoint& ExistingPoint : ExistingPoints)
    {
        float Distance = FVector::Dist(Location, ExistingPoint.Transform.GetLocation());
        if (Distance < MinDistance)
        {
            return false;
        }
    }
    return true;
}

FRotator FPCGArchitectureElement::CalculateOptimalOrientation(const FVector& Location, FPCGContext* Context) const
{
    // Calculate optimal orientation based on:
    // 1. Slope direction (face away from steep slopes)
    // 2. Wind direction (opening away from prevailing winds)
    // 3. Sun direction (opening towards south for warmth)
    // 4. Threat direction (defensive structures face likely threat directions)
    
    // Simplified implementation - face random direction with slight bias towards south
    float Yaw = FMath::RandRange(-30.0f, 30.0f); // Bias towards south (0 degrees)
    
    return FRotator(0.0f, Yaw, 0.0f);
}

#undef LOCTEXT_NAMESPACE
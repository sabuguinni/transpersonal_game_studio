#include "PCGBuildingGenerator.h"
#include "PCGContext.h"
#include "Data/PCGPointData.h"
#include "Data/PCGSpatialData.h"
#include "Helpers/PCGHelpers.h"
#include "PCGComponent.h"
#include "Engine/World.h"
#include "Landscape.h"

UPCGBuildingGeneratorSettings::UPCGBuildingGeneratorSettings()
{
    BuildingDensity = EPCGBuildingDensity::Low;
    DistributionPattern = EPCGBuildingDistribution::Random;
    MinDistanceBetweenBuildings = 500.0f; // 5 meters
    MaxSlopeAngle = 15.0f;
    MinDistanceFromWater = 200.0f; // 2 meters
    bAvoidSteepTerrain = true;
    bPreferHighGround = false;
    bClusterAroundResources = true;
    AbandonmentProbability = 0.2f; // 20% chance of abandoned buildings
    DeteriorationVariation = 0.5f;

    // Initialize style probabilities for prehistoric buildings
    StyleProbabilities.Empty();
    StyleProbabilities.Add(TEXT("PrimitiveHut"), 0.4f);
    StyleProbabilities.Add(TEXT("PrehistoricCave"), 0.2f);
    StyleProbabilities.Add(TEXT("StoneCircle"), 0.1f);
    StyleProbabilities.Add(TEXT("TribalShelter"), 0.2f);
    StyleProbabilities.Add(TEXT("AncientRuins"), 0.05f);
    StyleProbabilities.Add(TEXT("BurialMound"), 0.05f);

    // Initialize purpose probabilities
    PurposeProbabilities.Empty();
    PurposeProbabilities.Add(TEXT("Shelter"), 0.5f);
    PurposeProbabilities.Add(TEXT("Storage"), 0.2f);
    PurposeProbabilities.Add(TEXT("Crafting"), 0.15f);
    PurposeProbabilities.Add(TEXT("Ritual"), 0.08f);
    PurposeProbabilities.Add(TEXT("Defense"), 0.05f);
    PurposeProbabilities.Add(TEXT("Burial"), 0.02f);
}

TArray<FPCGPinProperties> UPCGBuildingGeneratorSettings::InputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultInputLabel, EPCGDataType::Spatial);
    return PinProperties;
}

TArray<FPCGPinProperties> UPCGBuildingGeneratorSettings::OutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    PinProperties.Emplace(PCGPinConstants::DefaultOutputLabel, EPCGDataType::Point);
    return PinProperties;
}

UPCGNode* UPCGBuildingGeneratorSettings::CreateNode() const
{
    return NewObject<UPCGNode>();
}

FPCGElementPtr UPCGBuildingGeneratorSettings::CreateElement() const
{
    return MakeShared<FPCGBuildingGeneratorElement>();
}

FPCGContext* FPCGBuildingGeneratorElement::Initialize(const FPCGDataCollection& InputData, TWeakObjectPtr<UPCGComponent> SourceComponent, const UPCGNode* Node)
{
    FPCGContext* Context = new FPCGContext();
    Context->InputData = InputData;
    Context->SourceComponent = SourceComponent;
    Context->Node = Node;
    return Context;
}

bool FPCGBuildingGeneratorElement::ExecuteInternal(FPCGContext* Context) const
{
    TRACE_CPUPROFILER_EVENT_SCOPE(FPCGBuildingGeneratorElement::Execute);

    const UPCGBuildingGeneratorSettings* Settings = Context->GetInputSettings<UPCGBuildingGeneratorSettings>();
    check(Settings);

    // Get input spatial data
    TArray<FPCGTaggedData> Inputs = Context->InputData.GetInputsByPin(PCGPinConstants::DefaultInputLabel);
    if (Inputs.IsEmpty())
    {
        PCGE_LOG(Error, GraphAndLog, FTEXT("No input data provided"));
        return true;
    }

    // Process each input
    for (const FPCGTaggedData& Input : Inputs)
    {
        const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
        if (!SpatialData)
        {
            PCGE_LOG(Warning, GraphAndLog, FTEXT("Input is not spatial data, skipping"));
            continue;
        }

        // Create output point data
        UPCGPointData* OutputData = NewObject<UPCGPointData>();
        if (!OutputData)
        {
            PCGE_LOG(Error, GraphAndLog, FTEXT("Failed to create output point data"));
            continue;
        }

        // Generate building points
        if (GenerateBuildingPoints(Context, Settings, SpatialData, OutputData))
        {
            FPCGTaggedData& Output = Context->OutputData.TaggedData.Emplace_GetRef();
            Output.Data = OutputData;
            Output.Tags = Input.Tags;
        }
    }

    return true;
}

bool FPCGBuildingGeneratorElement::GenerateBuildingPoints(FPCGContext* Context, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData, UPCGPointData* OutputData) const
{
    if (!Settings || !SpatialData || !OutputData)
    {
        return false;
    }

    // Get spatial bounds
    FBox Bounds = SpatialData->GetBounds();
    float Area = (Bounds.Max.X - Bounds.Min.X) * (Bounds.Max.Y - Bounds.Min.Y);
    
    // Calculate number of buildings based on density
    int32 TargetBuildingCount = 0;
    switch (Settings->BuildingDensity)
    {
        case EPCGBuildingDensity::Sparse:
            TargetBuildingCount = FMath::RoundToInt(Area / 1000000.0f * 1.5f); // 1.5 per km²
            break;
        case EPCGBuildingDensity::Low:
            TargetBuildingCount = FMath::RoundToInt(Area / 1000000.0f * 4.0f); // 4 per km²
            break;
        case EPCGBuildingDensity::Medium:
            TargetBuildingCount = FMath::RoundToInt(Area / 1000000.0f * 8.0f); // 8 per km²
            break;
        case EPCGBuildingDensity::High:
            TargetBuildingCount = FMath::RoundToInt(Area / 1000000.0f * 13.0f); // 13 per km²
            break;
        case EPCGBuildingDensity::Dense:
            TargetBuildingCount = FMath::RoundToInt(Area / 1000000.0f * 20.0f); // 20 per km²
            break;
    }

    // Clamp to reasonable limits
    TargetBuildingCount = FMath::Clamp(TargetBuildingCount, 1, 100);

    PCGE_LOG(Log, LogOnly, FText::FromString(FString::Printf(TEXT("Generating %d buildings in area of %.2f km²"), 
             TargetBuildingCount, Area / 1000000.0f)));

    // Generate candidate locations
    TArray<FVector> CandidateLocations;
    TArray<FPCGPoint> OutputPoints;

    int32 AttemptCount = 0;
    const int32 MaxAttempts = TargetBuildingCount * 20; // Try 20x more locations than needed

    while (CandidateLocations.Num() < TargetBuildingCount && AttemptCount < MaxAttempts)
    {
        AttemptCount++;

        // Generate random location within bounds
        FVector TestLocation = FVector(
            FMath::RandRange(Bounds.Min.X, Bounds.Max.X),
            FMath::RandRange(Bounds.Min.Y, Bounds.Max.Y),
            0.0f
        );

        // Sample height from spatial data
        FVector SampledLocation = TestLocation;
        if (SpatialData->SamplePoint(FTransform(TestLocation), Bounds, SampledLocation, nullptr))
        {
            TestLocation = SampledLocation;
        }

        // Check if location is valid for building placement
        if (IsValidBuildingLocation(TestLocation, Settings, SpatialData))
        {
            // Check minimum distance from other buildings
            bool bTooClose = false;
            for (const FVector& ExistingLocation : CandidateLocations)
            {
                if (FVector::Dist(TestLocation, ExistingLocation) < Settings->MinDistanceBetweenBuildings)
                {
                    bTooClose = true;
                    break;
                }
            }

            if (!bTooClose)
            {
                CandidateLocations.Add(TestLocation);
            }
        }
    }

    // Apply distribution pattern
    ApplyDistributionPattern(CandidateLocations, Settings->DistributionPattern, SpatialData);

    // Create PCG points for each valid location
    for (const FVector& Location : CandidateLocations)
    {
        FPCGPoint Point;
        Point.Transform = FTransform(Location);
        Point.SetLocalBounds(FBox(FVector(-50.0f), FVector(50.0f))); // 1m x 1m bounds
        
        // Calculate location score for density
        float LocationScore = CalculateLocationScore(Location, Settings, SpatialData);
        Point.Density = FMath::Clamp(LocationScore, 0.1f, 1.0f);

        // Add building metadata
        Point.SetAttribute(TEXT("BuildingStyle"), SelectBuildingStyle(Settings));
        Point.SetAttribute(TEXT("BuildingPurpose"), SelectBuildingPurpose(Settings));
        Point.SetAttribute(TEXT("IsAbandoned"), FMath::RandRange(0.0f, 1.0f) < Settings->AbandonmentProbability);
        Point.SetAttribute(TEXT("DeteriorationLevel"), FMath::RandRange(0.1f, 0.8f) * Settings->DeteriorationVariation);
        Point.SetAttribute(TEXT("BuildingAge"), FMath::RandRange(10, 500)); // Age in years
        
        // Random rotation for natural placement
        FRotator RandomRotation = FRotator(0.0f, FMath::RandRange(0.0f, 360.0f), 0.0f);
        Point.Transform.SetRotation(RandomRotation.Quaternion());

        OutputPoints.Add(Point);
    }

    // Set output points
    OutputData->GetMutablePoints() = OutputPoints;

    PCGE_LOG(Log, LogOnly, FText::FromString(FString::Printf(TEXT("Successfully generated %d building points"), 
             OutputPoints.Num())));

    return true;
}

bool FPCGBuildingGeneratorElement::IsValidBuildingLocation(const FVector& Location, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData) const
{
    if (!Settings || !SpatialData)
    {
        return false;
    }

    // Check if location is within spatial data bounds
    if (!SpatialData->GetBounds().IsInside(Location))
    {
        return false;
    }

    // Check slope if avoiding steep terrain
    if (Settings->bAvoidSteepTerrain)
    {
        // Sample nearby points to calculate slope
        FVector LocationA = Location + FVector(100.0f, 0.0f, 0.0f);
        FVector LocationB = Location + FVector(0.0f, 100.0f, 0.0f);
        
        FVector SampledA, SampledB;
        FBox Bounds = SpatialData->GetBounds();
        
        if (SpatialData->SamplePoint(FTransform(LocationA), Bounds, SampledA, nullptr) &&
            SpatialData->SamplePoint(FTransform(LocationB), Bounds, SampledB, nullptr))
        {
            FVector Normal = FVector::CrossProduct(
                (SampledA - Location).GetSafeNormal(),
                (SampledB - Location).GetSafeNormal()
            ).GetSafeNormal();

            float SlopeAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
            
            if (SlopeAngle > Settings->MaxSlopeAngle)
            {
                return false;
            }
        }
    }

    // Additional validation could include:
    // - Water proximity checks
    // - Resource proximity checks
    // - Existing building proximity checks

    return true;
}

float FPCGBuildingGeneratorElement::CalculateLocationScore(const FVector& Location, const UPCGBuildingGeneratorSettings* Settings, const UPCGSpatialData* SpatialData) const
{
    float Score = 0.5f; // Base score

    if (!Settings || !SpatialData)
    {
        return Score;
    }

    // Prefer high ground if specified
    if (Settings->bPreferHighGround)
    {
        FBox Bounds = SpatialData->GetBounds();
        float RelativeHeight = (Location.Z - Bounds.Min.Z) / (Bounds.Max.Z - Bounds.Min.Z);
        Score += RelativeHeight * 0.3f;
    }

    // Cluster around resources if specified
    if (Settings->bClusterAroundResources)
    {
        // This would check proximity to resource nodes when available
        // For now, add some randomness to simulate resource clustering
        Score += FMath::RandRange(-0.2f, 0.2f);
    }

    return FMath::Clamp(Score, 0.0f, 1.0f);
}

FString FPCGBuildingGeneratorElement::SelectBuildingStyle(const UPCGBuildingGeneratorSettings* Settings) const
{
    if (!Settings || Settings->StyleProbabilities.IsEmpty())
    {
        return TEXT("PrimitiveHut");
    }

    // Weighted random selection
    float TotalWeight = 0.0f;
    for (const auto& StylePair : Settings->StyleProbabilities)
    {
        TotalWeight += StylePair.Value;
    }

    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float AccumulatedWeight = 0.0f;

    for (const auto& StylePair : Settings->StyleProbabilities)
    {
        AccumulatedWeight += StylePair.Value;
        if (RandomValue <= AccumulatedWeight)
        {
            return StylePair.Key;
        }
    }

    // Fallback
    return Settings->StyleProbabilities.begin()->Key;
}

FString FPCGBuildingGeneratorElement::SelectBuildingPurpose(const UPCGBuildingGeneratorSettings* Settings) const
{
    if (!Settings || Settings->PurposeProbabilities.IsEmpty())
    {
        return TEXT("Shelter");
    }

    // Weighted random selection
    float TotalWeight = 0.0f;
    for (const auto& PurposePair : Settings->PurposeProbabilities)
    {
        TotalWeight += PurposePair.Value;
    }

    float RandomValue = FMath::RandRange(0.0f, TotalWeight);
    float AccumulatedWeight = 0.0f;

    for (const auto& PurposePair : Settings->PurposeProbabilities)
    {
        AccumulatedWeight += PurposePair.Value;
        if (RandomValue <= AccumulatedWeight)
        {
            return PurposePair.Key;
        }
    }

    // Fallback
    return Settings->PurposeProbabilities.begin()->Key;
}

void FPCGBuildingGeneratorElement::ApplyDistributionPattern(TArray<FVector>& Locations, EPCGBuildingDistribution Pattern, const UPCGSpatialData* SpatialData) const
{
    switch (Pattern)
    {
        case EPCGBuildingDistribution::Clustered:
            CreateClusteredDistribution(Locations, SpatialData);
            break;
        case EPCGBuildingDistribution::Linear:
            CreateLinearDistribution(Locations, SpatialData);
            break;
        case EPCGBuildingDistribution::Defensive:
            CreateDefensiveDistribution(Locations, SpatialData);
            break;
        case EPCGBuildingDistribution::Random:
        default:
            // Already random, no modification needed
            break;
    }
}

void FPCGBuildingGeneratorElement::CreateClusteredDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const
{
    if (Locations.Num() < 3)
    {
        return; // Not enough locations to cluster
    }

    // Group locations into clusters
    int32 NumClusters = FMath::Max(1, Locations.Num() / 4); // Average 4 buildings per cluster
    
    // Simple clustering by proximity
    // In a more sophisticated implementation, this would use proper clustering algorithms
}

void FPCGBuildingGeneratorElement::CreateLinearDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const
{
    if (Locations.Num() < 2)
    {
        return;
    }

    // Arrange buildings along natural paths or ridges
    // This would integrate with path/road generation systems when available
}

void FPCGBuildingGeneratorElement::CreateDefensiveDistribution(TArray<FVector>& Locations, const UPCGSpatialData* SpatialData) const
{
    if (!SpatialData)
    {
        return;
    }

    // Prefer high ground and defensive positions
    FBox Bounds = SpatialData->GetBounds();
    
    // Sort locations by height (defensive advantage)
    Locations.Sort([](const FVector& A, const FVector& B) {
        return A.Z > B.Z; // Higher locations first
    });

    // Keep only the highest positions for defensive buildings
    int32 DefensiveCount = FMath::Max(1, Locations.Num() / 3);
    if (Locations.Num() > DefensiveCount)
    {
        Locations.RemoveAt(DefensiveCount, Locations.Num() - DefensiveCount);
    }
}
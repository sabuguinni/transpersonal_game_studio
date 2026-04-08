#include "PCGArchitectureGenerator.h"
#include "PCGContext.h"
#include "PCGPin.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

UPCGArchitectureNode::UPCGArchitectureNode()
{
    // Initialize default settlement layouts
    FSettlementLayout IsolatedLayout;
    IsolatedLayout.SettlementType = ESettlementType::IsolatedShelter;
    IsolatedLayout.RequiredBuildings = {EBuildingType::ShelterBasic};
    IsolatedLayout.OptionalBuildings = {EBuildingType::FirePit, EBuildingType::StorageHut};
    IsolatedLayout.MinRadius = 100.0f;
    IsolatedLayout.MaxRadius = 300.0f;
    IsolatedLayout.MinBuildings = 1;
    IsolatedLayout.MaxBuildings = 3;
    IsolatedLayout.bRequiresCentralFirePit = true;
    IsolatedLayout.SettlementStory = "A lone survivor's desperate attempt at shelter. The hastily built structure speaks of urgency and fear.";
    SettlementLayouts.Add(IsolatedLayout);

    FSettlementLayout SmallCampLayout;
    SmallCampLayout.SettlementType = ESettlementType::SmallCamp;
    SmallCampLayout.RequiredBuildings = {EBuildingType::ShelterBasic, EBuildingType::FirePit};
    SmallCampLayout.OptionalBuildings = {EBuildingType::StorageHut, EBuildingType::WorkshopBasic, EBuildingType::WaterCistern};
    SmallCampLayout.MinRadius = 200.0f;
    SmallCampLayout.MaxRadius = 500.0f;
    SmallCampLayout.MinBuildings = 2;
    SmallCampLayout.MaxBuildings = 5;
    SmallCampLayout.bRequiresCentralFirePit = true;
    SmallCampLayout.SettlementStory = "A small group tried to establish a permanent camp here. The organized layout suggests they stayed for weeks, maybe months.";
    SettlementLayouts.Add(SmallCampLayout);

    FSettlementLayout DefensiveLayout;
    DefensiveLayout.SettlementType = ESettlementType::DefensiveOutpost;
    DefensiveLayout.RequiredBuildings = {EBuildingType::Watchtower, EBuildingType::ShelterReinforced};
    DefensiveLayout.OptionalBuildings = {EBuildingType::DefensiveWall, EBuildingType::TrapPit, EBuildingType::StorageHut};
    DefensiveLayout.MinRadius = 300.0f;
    DefensiveLayout.MaxRadius = 800.0f;
    DefensiveLayout.MinBuildings = 2;
    DefensiveLayout.MaxBuildings = 6;
    DefensiveLayout.bRequiresDefensivePerimeter = true;
    DefensiveLayout.SettlementStory = "This outpost was built by someone who understood the dangers. Every structure is positioned for maximum visibility and defense.";
    SettlementLayouts.Add(DefensiveLayout);

    // Initialize spawn parameters
    SpawnParameters.AllowedSettlementTypes = {
        ESettlementType::IsolatedShelter,
        ESettlementType::SmallCamp,
        ESettlementType::DefensiveOutpost,
        ESettlementType::AbandonedSite
    };
}

TArray<FPCGPinProperties> UPCGArchitectureNode::GetInputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    
    FPCGPinProperties& TerrainInput = PinProperties.Emplace_GetRef();
    TerrainInput.Label = TEXT("Terrain Data");
    TerrainInput.AllowedTypes = EPCGDataType::Spatial;
    
    FPCGPinProperties& EnvironmentInput = PinProperties.Emplace_GetRef();
    EnvironmentInput.Label = TEXT("Environment Data");
    EnvironmentInput.AllowedTypes = EPCGDataType::Point;
    
    return PinProperties;
}

TArray<FPCGPinProperties> UPCGArchitectureNode::GetOutputPinProperties() const
{
    TArray<FPCGPinProperties> PinProperties;
    
    FPCGPinProperties& BuildingOutput = PinProperties.Emplace_GetRef();
    BuildingOutput.Label = TEXT("Building Points");
    BuildingOutput.AllowedTypes = EPCGDataType::Point;
    
    FPCGPinProperties& InteriorOutput = PinProperties.Emplace_GetRef();
    InteriorOutput.Label = TEXT("Interior Objects");
    InteriorOutput.AllowedTypes = EPCGDataType::Point;
    
    return PinProperties;
}

bool UPCGArchitectureNode::ExecuteInternal(FPCGContext* Context) const
{
    if (!ArchitectureTypology.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGArchitectureNode: No Architecture Typology assigned"));
        return false;
    }

    // Get input data
    const UPCGSpatialData* TerrainData = Context->InputData.GetSpatialData();
    if (!TerrainData)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGArchitectureNode: No terrain data provided"));
        return false;
    }

    TArray<FPCGPoint> BuildingPoints;
    TArray<FPCGPoint> InteriorPoints;

    // Sample potential building locations from terrain
    const FBox TerrainBounds = TerrainData->GetBounds();
    const float SampleDistance = SpawnParameters.MinDistanceBetweenBuildings;
    
    for (float X = TerrainBounds.Min.X; X < TerrainBounds.Max.X; X += SampleDistance)
    {
        for (float Y = TerrainBounds.Min.Y; Y < TerrainBounds.Max.Y; Y += SampleDistance)
        {
            FVector SampleLocation(X, Y, 0);
            
            // Sample terrain height
            FVector TerrainLocation;
            if (TerrainData->SamplePoint(FTransform(SampleLocation), TerrainLocation, nullptr))
            {
                SampleLocation.Z = TerrainLocation.Z;
                
                // Random chance to place settlement
                if (FMath::RandRange(0.0f, 1.0f) < SpawnParameters.SpawnDensity)
                {
                    // Choose random settlement type
                    if (SpawnParameters.AllowedSettlementTypes.Num() > 0)
                    {
                        int32 RandomIndex = FMath::RandRange(0, SpawnParameters.AllowedSettlementTypes.Num() - 1);
                        ESettlementType SettlementType = SpawnParameters.AllowedSettlementTypes[RandomIndex];
                        
                        GenerateSettlement(SampleLocation, SettlementType, BuildingPoints, Context);
                        
                        if (bGenerateInteriorObjects)
                        {
                            // Generate interior objects for buildings in this settlement
                            // This would be expanded to generate interior layouts
                        }
                    }
                }
            }
        }
    }

    // Create output data
    UPCGPointData* BuildingOutputData = NewObject<UPCGPointData>();
    BuildingOutputData->InitializeFromData(BuildingPoints);
    Context->OutputData.TaggedData.Emplace(BuildingOutputData, TEXT("Building Points"));

    if (bGenerateInteriorObjects && InteriorPoints.Num() > 0)
    {
        UPCGPointData* InteriorOutputData = NewObject<UPCGPointData>();
        InteriorOutputData->InitializeFromData(InteriorPoints);
        Context->OutputData.TaggedData.Emplace(InteriorOutputData, TEXT("Interior Objects"));
    }

    return true;
}

void UPCGArchitectureNode::GenerateSettlement(const FVector& Location, ESettlementType SettlementType, 
                                            TArray<FPCGPoint>& OutPoints, FPCGContext* Context) const
{
    FSettlementLayout Layout = GetSettlementLayout(SettlementType);
    
    // Determine if this should be an abandoned site
    bool bIsAbandoned = FMath::RandRange(0.0f, 1.0f) < SpawnParameters.AbandonedSiteProbability;
    
    // Place required buildings
    for (EBuildingType BuildingType : Layout.RequiredBuildings)
    {
        FVector BuildingLocation = Location + FVector(
            FMath::RandRange(-Layout.MaxRadius, Layout.MaxRadius),
            FMath::RandRange(-Layout.MaxRadius, Layout.MaxRadius),
            0
        );
        
        EBuildingCondition Condition = bIsAbandoned ? 
            (FMath::RandBool() ? EBuildingCondition::Ruined : EBuildingCondition::Overgrown) :
            EBuildingCondition::WellMaintained;
            
        PlaceBuilding(BuildingLocation, BuildingType, Condition, OutPoints);
    }
    
    // Place optional buildings
    int32 NumOptionalBuildings = FMath::RandRange(0, Layout.OptionalBuildings.Num());
    for (int32 i = 0; i < NumOptionalBuildings; i++)
    {
        if (Layout.OptionalBuildings.IsValidIndex(i))
        {
            FVector BuildingLocation = Location + FVector(
                FMath::RandRange(-Layout.MaxRadius, Layout.MaxRadius),
                FMath::RandRange(-Layout.MaxRadius, Layout.MaxRadius),
                0
            );
            
            EBuildingCondition Condition = bIsAbandoned ? 
                EBuildingCondition::Damaged : EBuildingCondition::Weathered;
                
            PlaceBuilding(BuildingLocation, Layout.OptionalBuildings[i], Condition, OutPoints);
        }
    }
    
    // Place central fire pit if required
    if (Layout.bRequiresCentralFirePit)
    {
        PlaceBuilding(Location, EBuildingType::FirePit, 
                     bIsAbandoned ? EBuildingCondition::Ruined : EBuildingCondition::Weathered, 
                     OutPoints);
    }
}

void UPCGArchitectureNode::PlaceBuilding(const FVector& Location, EBuildingType BuildingType, 
                                       EBuildingCondition Condition, TArray<FPCGPoint>& OutPoints) const
{
    FPCGPoint BuildingPoint;
    BuildingPoint.Transform = FTransform(Location);
    BuildingPoint.Density = 1.0f;
    
    // Set metadata for the building
    BuildingPoint.MetadataEntry = OutPoints.Num(); // Simple ID system
    
    // Store building type and condition as attributes
    // In a full implementation, this would use PCG's attribute system
    
    OutPoints.Add(BuildingPoint);
}

void UPCGArchitectureNode::GenerateInteriorLayout(const FVector& BuildingLocation, EBuildingType BuildingType,
                                                 TArray<FPCGPoint>& OutPoints, FPCGContext* Context) const
{
    // This would generate interior objects based on building type
    // For now, just add a placeholder point
    FPCGPoint InteriorPoint;
    InteriorPoint.Transform = FTransform(BuildingLocation + FVector(0, 0, 50));
    InteriorPoint.Density = 0.5f;
    OutPoints.Add(InteriorPoint);
}

bool UPCGArchitectureNode::IsLocationSuitable(const FVector& Location, EBuildingType BuildingType, 
                                             FPCGContext* Context) const
{
    // Check terrain slope, water access, etc.
    // This would integrate with environment data from previous agents
    return true; // Simplified for now
}

FSettlementLayout UPCGArchitectureNode::GetSettlementLayout(ESettlementType SettlementType) const
{
    for (const FSettlementLayout& Layout : SettlementLayouts)
    {
        if (Layout.SettlementType == SettlementType)
        {
            return Layout;
        }
    }
    
    // Return default layout if not found
    return SettlementLayouts.Num() > 0 ? SettlementLayouts[0] : FSettlementLayout();
}

// PCG Architecture Component Implementation

UPCGArchitectureComponent::UPCGArchitectureComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    bGenerateOnBeginPlay = true;
    bRegenerateOnEnvironmentChange = true;
}

void UPCGArchitectureComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bGenerateOnBeginPlay)
    {
        RegenerateArchitecture();
    }
}

void UPCGArchitectureComponent::RegenerateArchitecture()
{
    CleanupGeneratedBuildings();
    
    // Trigger PCG generation
    if (GetGraph())
    {
        Generate();
    }
}

TArray<AActor*> UPCGArchitectureComponent::GetBuildingsInRadius(const FVector& Center, float Radius) const
{
    TArray<AActor*> NearbyBuildings;
    
    for (AActor* Building : GeneratedBuildings)
    {
        if (IsValid(Building))
        {
            float Distance = FVector::Dist(Building->GetActorLocation(), Center);
            if (Distance <= Radius)
            {
                NearbyBuildings.Add(Building);
            }
        }
    }
    
    return NearbyBuildings;
}

AActor* UPCGArchitectureComponent::GetNearestBuilding(const FVector& Location, EBuildingType BuildingType) const
{
    AActor* NearestBuilding = nullptr;
    float NearestDistance = MAX_FLT;
    
    for (AActor* Building : GeneratedBuildings)
    {
        if (IsValid(Building))
        {
            float Distance = FVector::Dist(Building->GetActorLocation(), Location);
            if (Distance < NearestDistance)
            {
                // In a full implementation, we'd check the building type here
                NearestDistance = Distance;
                NearestBuilding = Building;
            }
        }
    }
    
    return NearestBuilding;
}

void UPCGArchitectureComponent::OnEnvironmentChanged()
{
    if (bRegenerateOnEnvironmentChange)
    {
        RegenerateArchitecture();
    }
}

void UPCGArchitectureComponent::CleanupGeneratedBuildings()
{
    for (AActor* Building : GeneratedBuildings)
    {
        if (IsValid(Building))
        {
            Building->Destroy();
        }
    }
    GeneratedBuildings.Empty();
}
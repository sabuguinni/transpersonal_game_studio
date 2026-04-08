// PCG_SettlementGenerator.cpp
// Procedural Content Generation for prehistoric settlements
// Architecture & Interior Agent #07 — Transpersonal Game Studio

#include "PCG_SettlementGenerator.h"
#include "PCGComponent.h"
#include "PCGContext.h"
#include "PCGParamData.h"
#include "Elements/PCGStaticMeshSpawner.h"
#include "Engine/StaticMesh.h"
#include "Math/UnrealMathUtility.h"

UPCG_SettlementGenerator::UPCG_SettlementGenerator()
{
    // Default settlement parameters
    SettlementRadius = 5000.0f; // 50 meters
    MinBuildingDistance = 800.0f; // 8 meters between buildings
    MaxBuildingDistance = 1500.0f; // 15 meters max distance
    DefensiveWallChance = 0.7f;
    CommunalAreaSize = 1000.0f; // 10 meters diameter
    
    // Building type probabilities
    BasicShelterChance = 0.4f;
    PermanentHouseChance = 0.3f;
    SemiSubterraneanChance = 0.2f;
    WatchtowerChance = 0.1f;
}

TArray<FPCGTaggedData> UPCG_SettlementGenerator::Process(FPCGContext* Context, const TArray<FPCGTaggedData>& InputData) const
{
    TArray<FPCGTaggedData> Outputs;
    
    for (const FPCGTaggedData& Input : InputData)
    {
        const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
        if (!SpatialData)
        {
            continue;
        }

        // Generate settlement layout
        TArray<FVector> BuildingPositions = GenerateSettlementLayout(SpatialData);
        
        // Create building spawns
        for (int32 i = 0; i < BuildingPositions.Num(); ++i)
        {
            FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
            UPCGPointData* PointData = NewObject<UPCGPointData>();
            
            // Determine building type based on position and random factors
            EBuildingType BuildingType = DetermineBuildingType(i, BuildingPositions.Num());
            
            // Create point for building spawn
            FPCGPoint BuildingPoint;
            BuildingPoint.Transform.SetLocation(BuildingPositions[i]);
            BuildingPoint.Transform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RandRange(0.0f, 360.0f))));
            BuildingPoint.Density = 1.0f;
            
            // Set building type as metadata
            BuildingPoint.SetLocalBounds(FBox(FVector(-400, -400, 0), FVector(400, 400, 300))); // 8x8x6 meters
            
            TArray<FPCGPoint>& Points = PointData->GetMutablePoints();
            Points.Add(BuildingPoint);
            
            Output.Data = PointData;
            Output.Tags.Add(GetBuildingTypeTag(BuildingType));
        }
        
        // Generate defensive perimeter if needed
        if (FMath::RandRange(0.0f, 1.0f) < DefensiveWallChance)
        {
            GenerateDefensivePerimeter(Input, Outputs);
        }
        
        // Generate communal areas
        GenerateCommunalAreas(Input, Outputs);
    }
    
    return Outputs;
}

TArray<FVector> UPCG_SettlementGenerator::GenerateSettlementLayout(const UPCGSpatialData* SpatialData) const
{
    TArray<FVector> Positions;
    FBox Bounds = SpatialData->GetBounds();
    FVector Center = Bounds.GetCenter();
    
    // Determine number of buildings based on area
    float Area = Bounds.GetSize().X * Bounds.GetSize().Y;
    int32 NumBuildings = FMath::Clamp(FMath::RoundToInt(Area / 10000000.0f), 3, 15); // 3-15 buildings
    
    // Generate positions in defensive cluster pattern
    for (int32 i = 0; i < NumBuildings; ++i)
    {
        FVector Position;
        int32 Attempts = 0;
        bool ValidPosition = false;
        
        while (!ValidPosition && Attempts < 50)
        {
            // Generate position in circular pattern around center
            float Angle = (2.0f * PI * i / NumBuildings) + FMath::RandRange(-0.5f, 0.5f);
            float Distance = FMath::RandRange(MinBuildingDistance, MaxBuildingDistance);
            
            Position = Center + FVector(
                FMath::Cos(Angle) * Distance,
                FMath::Sin(Angle) * Distance,
                0.0f
            );
            
            // Check if position is valid (within bounds and not too close to others)
            ValidPosition = IsValidBuildingPosition(Position, Positions, SpatialData);
            Attempts++;
        }
        
        if (ValidPosition)
        {
            Positions.Add(Position);
        }
    }
    
    return Positions;
}

bool UPCG_SettlementGenerator::IsValidBuildingPosition(const FVector& Position, const TArray<FVector>& ExistingPositions, const UPCGSpatialData* SpatialData) const
{
    // Check bounds
    if (!SpatialData->GetBounds().IsInside(Position))
    {
        return false;
    }
    
    // Check minimum distance to other buildings
    for (const FVector& ExistingPos : ExistingPositions)
    {
        if (FVector::Dist(Position, ExistingPos) < MinBuildingDistance)
        {
            return false;
        }
    }
    
    return true;
}

EBuildingType UPCG_SettlementGenerator::DetermineBuildingType(int32 BuildingIndex, int32 TotalBuildings) const
{
    // First building is always a watchtower if settlement is large enough
    if (BuildingIndex == 0 && TotalBuildings > 5)
    {
        return EBuildingType::Watchtower;
    }
    
    // Random distribution for other buildings
    float Random = FMath::RandRange(0.0f, 1.0f);
    
    if (Random < BasicShelterChance)
    {
        return EBuildingType::BasicShelter;
    }
    else if (Random < BasicShelterChance + PermanentHouseChance)
    {
        return EBuildingType::PermanentHouse;
    }
    else if (Random < BasicShelterChance + PermanentHouseChance + SemiSubterraneanChance)
    {
        return EBuildingType::SemiSubterranean;
    }
    else
    {
        return EBuildingType::CommunalStorage;
    }
}

FString UPCG_SettlementGenerator::GetBuildingTypeTag(EBuildingType BuildingType) const
{
    switch (BuildingType)
    {
        case EBuildingType::BasicShelter:
            return TEXT("BasicShelter");
        case EBuildingType::PermanentHouse:
            return TEXT("PermanentHouse");
        case EBuildingType::SemiSubterranean:
            return TEXT("SemiSubterranean");
        case EBuildingType::Watchtower:
            return TEXT("Watchtower");
        case EBuildingType::CommunalStorage:
            return TEXT("CommunalStorage");
        default:
            return TEXT("Unknown");
    }
}

void UPCG_SettlementGenerator::GenerateDefensivePerimeter(const FPCGTaggedData& Input, TArray<FPCGTaggedData>& Outputs) const
{
    const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
    if (!SpatialData)
    {
        return;
    }
    
    FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
    UPCGPointData* PointData = NewObject<UPCGPointData>();
    
    FBox Bounds = SpatialData->GetBounds();
    FVector Center = Bounds.GetCenter();
    float Radius = SettlementRadius * 1.2f; // Perimeter slightly outside settlement
    
    // Generate wall segments
    int32 NumSegments = 16; // Circular wall with 16 segments
    for (int32 i = 0; i < NumSegments; ++i)
    {
        float Angle = (2.0f * PI * i / NumSegments);
        FVector Position = Center + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        FPCGPoint WallPoint;
        WallPoint.Transform.SetLocation(Position);
        WallPoint.Transform.SetRotation(FQuat::MakeFromEuler(FVector(0, 0, FMath::RadiansToDegrees(Angle + PI/2))));
        WallPoint.Density = 1.0f;
        WallPoint.SetLocalBounds(FBox(FVector(-200, -50, 0), FVector(200, 50, 400))); // Wall segment bounds
        
        PointData->GetMutablePoints().Add(WallPoint);
    }
    
    Output.Data = PointData;
    Output.Tags.Add(TEXT("DefensiveWall"));
}

void UPCG_SettlementGenerator::GenerateCommunalAreas(const FPCGTaggedData& Input, TArray<FPCGTaggedData>& Outputs) const
{
    const UPCGSpatialData* SpatialData = Cast<UPCGSpatialData>(Input.Data);
    if (!SpatialData)
    {
        return;
    }
    
    FPCGTaggedData& Output = Outputs.Add_GetRef(Input);
    UPCGPointData* PointData = NewObject<UPCGPointData>();
    
    FBox Bounds = SpatialData->GetBounds();
    FVector Center = Bounds.GetCenter();
    
    // Central fire pit
    FPCGPoint FirePitPoint;
    FirePitPoint.Transform.SetLocation(Center);
    FirePitPoint.Transform.SetRotation(FQuat::Identity);
    FirePitPoint.Density = 1.0f;
    FirePitPoint.SetLocalBounds(FBox(FVector(-150, -150, 0), FVector(150, 150, 50))); // 3x3 meter fire pit
    
    PointData->GetMutablePoints().Add(FirePitPoint);
    
    Output.Data = PointData;
    Output.Tags.Add(TEXT("CommunalArea"));
}
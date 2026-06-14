#include "Core_TerrainPhysics.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/LandscapeComponent.h"
#include "Landscape/LandscapeInfo.h"
#include "Components/PrimitiveComponent.h"
#include "DrawDebugHelpers.h"

UCore_TerrainPhysics::UCore_TerrainPhysics()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    MaxFootprints = 500;
    FootprintLifetime = 300.0f;
    
    InitializeTerrainPhysicsDefaults();
}

void UCore_TerrainPhysics::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeTerrainPhysicsDefaults();
    
    UE_LOG(LogTemp, Warning, TEXT("Core_TerrainPhysics: Component initialized with %d terrain types"), TerrainPhysicsMap.Num());
}

void UCore_TerrainPhysics::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Clean up old footprints every tick
    ClearOldFootprints();
}

void UCore_TerrainPhysics::InitializeTerrainPhysicsDefaults()
{
    TerrainPhysicsMap.Empty();
    
    // Grass terrain
    FCore_TerrainPhysicsData GrassData;
    GrassData.SurfaceFriction = 0.7f;
    GrassData.Hardness = 0.6f;
    GrassData.Stability = 0.8f;
    GrassData.DeformationResistance = 0.5f;
    GrassData.WaterAbsorption = 0.4f;
    GrassData.TerrainType = ETerrainType::Grass;
    TerrainPhysicsMap.Add(ETerrainType::Grass, GrassData);
    
    // Rock terrain
    FCore_TerrainPhysicsData RockData;
    RockData.SurfaceFriction = 0.9f;
    RockData.Hardness = 1.0f;
    RockData.Stability = 1.0f;
    RockData.DeformationResistance = 1.0f;
    RockData.WaterAbsorption = 0.1f;
    RockData.TerrainType = ETerrainType::Rock;
    TerrainPhysicsMap.Add(ETerrainType::Rock, RockData);
    
    // Mud terrain
    FCore_TerrainPhysicsData MudData;
    MudData.SurfaceFriction = 0.3f;
    MudData.Hardness = 0.2f;
    MudData.Stability = 0.3f;
    MudData.DeformationResistance = 0.1f;
    MudData.WaterAbsorption = 0.9f;
    MudData.TerrainType = ETerrainType::Mud;
    TerrainPhysicsMap.Add(ETerrainType::Mud, MudData);
    
    // Sand terrain
    FCore_TerrainPhysicsData SandData;
    SandData.SurfaceFriction = 0.5f;
    SandData.Hardness = 0.4f;
    SandData.Stability = 0.5f;
    SandData.DeformationResistance = 0.3f;
    SandData.WaterAbsorption = 0.6f;
    SandData.TerrainType = ETerrainType::Sand;
    TerrainPhysicsMap.Add(ETerrainType::Sand, SandData);
    
    // Water terrain
    FCore_TerrainPhysicsData WaterData;
    WaterData.SurfaceFriction = 0.1f;
    WaterData.Hardness = 0.0f;
    WaterData.Stability = 0.0f;
    WaterData.DeformationResistance = 0.0f;
    WaterData.WaterAbsorption = 1.0f;
    WaterData.TerrainType = ETerrainType::Water;
    TerrainPhysicsMap.Add(ETerrainType::Water, WaterData);
}

FCore_TerrainPhysicsData UCore_TerrainPhysics::GetTerrainPhysicsAtLocation(const FVector& Location)
{
    ETerrainType DetectedType = DetectTerrainTypeAtLocation(Location);
    
    if (TerrainPhysicsMap.Contains(DetectedType))
    {
        return TerrainPhysicsMap[DetectedType];
    }
    
    // Default to grass if not found
    return TerrainPhysicsMap.Contains(ETerrainType::Grass) ? TerrainPhysicsMap[ETerrainType::Grass] : FCore_TerrainPhysicsData();
}

void UCore_TerrainPhysics::SetTerrainPhysicsData(ETerrainType TerrainType, const FCore_TerrainPhysicsData& PhysicsData)
{
    TerrainPhysicsMap.Add(TerrainType, PhysicsData);
}

float UCore_TerrainPhysics::CalculateMovementSpeedModifier(const FVector& Location, ECreatureSize CreatureSize)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    float BaseModifier = TerrainData.SurfaceFriction * TerrainData.Stability;
    
    // Slope influence
    float Slope = CalculateTerrainSlope(Location);
    float SlopeModifier = FMath::Clamp(1.0f - (Slope / 45.0f), 0.2f, 1.0f);
    
    // Creature size influence
    float SizeModifier = 1.0f;
    switch (CreatureSize)
    {
        case ECreatureSize::Small:
            SizeModifier = 1.2f; // Small creatures less affected by terrain
            break;
        case ECreatureSize::Medium:
            SizeModifier = 1.0f;
            break;
        case ECreatureSize::Large:
            SizeModifier = 0.8f; // Large creatures more affected
            break;
        case ECreatureSize::Massive:
            SizeModifier = 0.6f; // Massive creatures heavily affected
            break;
    }
    
    return BaseModifier * SlopeModifier * SizeModifier;
}

bool UCore_TerrainPhysics::CanCreateFootprint(const FVector& Location, ECreatureSize CreatureSize)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    
    // Can't create footprints on very hard surfaces or water
    if (TerrainData.DeformationResistance > 0.9f || TerrainData.TerrainType == ETerrainType::Water)
    {
        return false;
    }
    
    // Check if we're at max footprints
    if (ActiveFootprints.Num() >= MaxFootprints)
    {
        return false;
    }
    
    return true;
}

void UCore_TerrainPhysics::CreateFootprint(const FVector& Location, ECreatureSize CreatureSize, float Force)
{
    if (!CanCreateFootprint(Location, CreatureSize))
    {
        return;
    }
    
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    
    FCore_FootprintData NewFootprint;
    NewFootprint.Location = Location;
    NewFootprint.CreatureSize = CreatureSize;
    NewFootprint.Timestamp = GetWorld()->GetTimeSeconds();
    
    // Calculate footprint properties based on creature size and terrain
    switch (CreatureSize)
    {
        case ECreatureSize::Small:
            NewFootprint.Radius = 3.0f;
            break;
        case ECreatureSize::Medium:
            NewFootprint.Radius = 8.0f;
            break;
        case ECreatureSize::Large:
            NewFootprint.Radius = 15.0f;
            break;
        case ECreatureSize::Massive:
            NewFootprint.Radius = 25.0f;
            break;
    }
    
    // Depth based on force, terrain resistance, and creature size
    float BaseDepth = Force * (1.0f - TerrainData.DeformationResistance);
    NewFootprint.Depth = BaseDepth * (static_cast<float>(CreatureSize) + 1.0f);
    
    ActiveFootprints.Add(NewFootprint);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Created footprint at %s, depth %.2f, radius %.2f"), 
           *Location.ToString(), NewFootprint.Depth, NewFootprint.Radius);
}

void UCore_TerrainPhysics::ClearOldFootprints()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    ActiveFootprints.RemoveAll([CurrentTime, this](const FCore_FootprintData& Footprint)
    {
        return (CurrentTime - Footprint.Timestamp) > FootprintLifetime;
    });
}

TArray<FCore_FootprintData> UCore_TerrainPhysics::GetFootprintsInRadius(const FVector& Center, float Radius)
{
    TArray<FCore_FootprintData> NearbyFootprints;
    
    for (const FCore_FootprintData& Footprint : ActiveFootprints)
    {
        float Distance = FVector::Dist(Center, Footprint.Location);
        if (Distance <= Radius)
        {
            NearbyFootprints.Add(Footprint);
        }
    }
    
    return NearbyFootprints;
}

ETerrainType UCore_TerrainPhysics::DetectTerrainTypeAtLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return ETerrainType::Grass;
    }
    
    // Perform line trace to detect surface material
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        if (HitResult.GetComponent())
        {
            // Check for landscape component
            if (ULandscapeComponent* LandscapeComp = Cast<ULandscapeComponent>(HitResult.GetComponent()))
            {
                // For now, return based on height and slope
                float Height = HitResult.Location.Z;
                float Slope = CalculateTerrainSlope(HitResult.Location);
                
                if (Height < 0)
                {
                    return ETerrainType::Water;
                }
                else if (Slope > 30.0f)
                {
                    return ETerrainType::Rock;
                }
                else if (Height < 100)
                {
                    return ETerrainType::Mud;
                }
                else
                {
                    return ETerrainType::Grass;
                }
            }
        }
    }
    
    return ETerrainType::Grass;
}

float UCore_TerrainPhysics::CalculateTerrainSlope(const FVector& Location, float SampleRadius)
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    // Sample terrain height at multiple points around the location
    TArray<FVector> SamplePoints;
    SamplePoints.Add(Location + FVector(SampleRadius, 0, 0));
    SamplePoints.Add(Location + FVector(-SampleRadius, 0, 0));
    SamplePoints.Add(Location + FVector(0, SampleRadius, 0));
    SamplePoints.Add(Location + FVector(0, -SampleRadius, 0));
    
    float MaxHeightDiff = 0.0f;
    float CenterHeight = GetTerrainHeightAtLocation(Location);
    
    for (const FVector& SamplePoint : SamplePoints)
    {
        float SampleHeight = GetTerrainHeightAtLocation(SamplePoint);
        float HeightDiff = FMath::Abs(SampleHeight - CenterHeight);
        MaxHeightDiff = FMath::Max(MaxHeightDiff, HeightDiff);
    }
    
    // Convert to slope angle in degrees
    float SlopeAngle = FMath::RadiansToDegrees(FMath::Atan(MaxHeightDiff / SampleRadius));
    return SlopeAngle;
}

bool UCore_TerrainPhysics::IsLocationStable(const FVector& Location, float MinStability)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    float Slope = CalculateTerrainSlope(Location);
    
    // Stability decreases with slope
    float SlopeStabilityModifier = FMath::Clamp(1.0f - (Slope / 60.0f), 0.0f, 1.0f);
    float EffectiveStability = TerrainData.Stability * SlopeStabilityModifier;
    
    return EffectiveStability >= MinStability;
}

void UCore_TerrainPhysics::ApplyTerrainDeformation(const FVector& Location, float Force, float Radius)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    
    // Only deform if terrain allows it
    if (TerrainData.DeformationResistance < 0.9f)
    {
        float DeformationAmount = Force * (1.0f - TerrainData.DeformationResistance);
        
        UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Applied deformation at %s, force %.2f, amount %.2f"), 
               *Location.ToString(), Force, DeformationAmount);
        
        // In a full implementation, this would modify the landscape heightmap
        // For now, we just log the deformation
    }
}

void UCore_TerrainPhysics::UpdatePhysicsFromBiome(const FVector& Location)
{
    // This would integrate with the biome system to get biome-specific physics modifiers
    // For now, we use the basic terrain detection
    ETerrainType DetectedType = DetectTerrainTypeAtLocation(Location);
    
    UE_LOG(LogTemp, Log, TEXT("Core_TerrainPhysics: Updated physics from biome at %s, terrain type: %d"), 
           *Location.ToString(), static_cast<int32>(DetectedType));
}

float UCore_TerrainPhysics::GetBiomeInfluencedFriction(const FVector& Location)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    
    // Base friction modified by biome conditions
    float BaseFriction = TerrainData.SurfaceFriction;
    
    // In a full implementation, this would query the biome system for weather/humidity modifiers
    float BiomeModifier = 1.0f;
    
    return BaseFriction * BiomeModifier;
}

float UCore_TerrainPhysics::GetBiomeInfluencedHardness(const FVector& Location)
{
    FCore_TerrainPhysicsData TerrainData = GetTerrainPhysicsAtLocation(Location);
    
    // Base hardness modified by biome conditions
    float BaseHardness = TerrainData.Hardness;
    
    // In a full implementation, this would query the biome system for temperature/moisture modifiers
    float BiomeModifier = 1.0f;
    
    return BaseHardness * BiomeModifier;
}

void UCore_TerrainPhysics::CleanupFootprintArray()
{
    // Remove excess footprints if we're over the limit
    if (ActiveFootprints.Num() > MaxFootprints)
    {
        int32 ExcessCount = ActiveFootprints.Num() - MaxFootprints;
        ActiveFootprints.RemoveAt(0, ExcessCount);
    }
}

float UCore_TerrainPhysics::GetTerrainHeightAtLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return 0.0f;
    }
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 1000);
    FVector EndLocation = Location - FVector(0, 0, 1000);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = false;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Location.Z;
    }
    
    return Location.Z;
}

FVector UCore_TerrainPhysics::GetTerrainNormalAtLocation(const FVector& Location)
{
    if (!GetWorld())
    {
        return FVector::UpVector;
    }
    
    FHitResult HitResult;
    FVector StartLocation = Location + FVector(0, 0, 100);
    FVector EndLocation = Location - FVector(0, 0, 100);
    
    FCollisionQueryParams QueryParams;
    QueryParams.bTraceComplex = true;
    
    if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_WorldStatic, QueryParams))
    {
        return HitResult.Normal;
    }
    
    return FVector::UpVector;
}
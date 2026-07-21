#include "World_RiverSystemGenerator.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "Landscape.h"
#include "LandscapeComponent.h"
#include "LandscapeInfo.h"
#include "Materials/MaterialInstanceDynamic.h"

AWorld_RiverSystemGenerator::AWorld_RiverSystemGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create spline component for river path
    RiverSpline = CreateDefaultSubobject<USplineComponent>(TEXT("RiverSpline"));
    RiverSpline->SetupAttachment(RootComponent);
    RiverSpline->SetClosedLoop(false);

    // Create mesh component for river visualization
    RiverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiverMesh"));
    RiverMesh->SetupAttachment(RootComponent);

    // Initialize default values
    MaxTributaries = 3;
    MinSegmentLength = 1000.0f;
    MaxSegmentLength = 5000.0f;
    RiverMeandering = 0.3f;
    ErosionStrength = 1.0f;

    // Set default river network values
    RiverNetwork.TotalLength = 0.0f;
}

void AWorld_RiverSystemGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    if (RiverNetwork.MainRiver.Num() == 0)
    {
        // Generate a default river if none exists
        FVector StartPoint = GetActorLocation() + FVector(0, 0, 1000);
        FVector EndPoint = GetActorLocation() + FVector(10000, 5000, -500);
        GenerateRiverSystem(StartPoint, EndPoint, 8);
    }
}

void AWorld_RiverSystemGenerator::GenerateRiverSystem(FVector StartPoint, FVector EndPoint, int32 Segments)
{
    if (Segments < 2)
    {
        UE_LOG(LogTemp, Warning, TEXT("River system needs at least 2 segments"));
        return;
    }

    // Clear existing river data
    RiverNetwork.MainRiver.Empty();
    RiverNetwork.Tributaries.Empty();
    RiverSpline->ClearSplinePoints();

    // Set source and delta locations
    RiverNetwork.SourceLocation = StartPoint;
    RiverNetwork.DeltaLocation = EndPoint;

    // Calculate total distance
    float TotalDistance = FVector::Dist(StartPoint, EndPoint);
    RiverNetwork.TotalLength = TotalDistance;

    // Generate main river segments
    for (int32 i = 0; i < Segments; i++)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(Segments - 1);
        FVector BasePoint = FMath::Lerp(StartPoint, EndPoint, Alpha);
        
        // Add meandering offset
        FVector MeanderedPoint = CalculateMeanderingOffset(BasePoint, RiverMeandering);
        
        // Create river segment
        FWorld_RiverSegment Segment;
        Segment.StartLocation = (i == 0) ? StartPoint : RiverNetwork.MainRiver[i-1].EndLocation;
        Segment.EndLocation = MeanderedPoint;
        
        // Calculate width based on distance from source (wider downstream)
        float WidthMultiplier = FMath::Lerp(0.5f, 2.0f, Alpha);
        Segment.Width = 300.0f * WidthMultiplier;
        Segment.Depth = 50.0f * WidthMultiplier;
        Segment.FlowSpeed = FMath::Lerp(2.0f, 0.8f, Alpha); // Faster upstream
        
        // Determine biome based on elevation and location
        float Elevation = MeanderedPoint.Z;
        if (Elevation > 800.0f)
        {
            Segment.BiomeType = EBiomeType::Mountain;
        }
        else if (Elevation > 200.0f)
        {
            Segment.BiomeType = EBiomeType::Temperate;
        }
        else
        {
            Segment.BiomeType = EBiomeType::Wetland;
        }

        RiverNetwork.MainRiver.Add(Segment);
        
        // Add spline point
        RiverSpline->AddSplinePoint(MeanderedPoint, ESplineCoordinateSpace::World);
    }

    // Generate tributaries
    int32 TributaryCount = FMath::RandRange(1, MaxTributaries);
    for (int32 i = 0; i < TributaryCount; i++)
    {
        if (RiverNetwork.MainRiver.Num() > 2)
        {
            int32 BranchIndex = FMath::RandRange(1, RiverNetwork.MainRiver.Num() - 2);
            FVector BranchPoint = RiverNetwork.MainRiver[BranchIndex].EndLocation;
            
            // Generate tributary endpoint
            FVector TributaryDirection = FVector(
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(-1.0f, 1.0f),
                FMath::RandRange(0.2f, 0.8f)
            ).GetSafeNormal();
            
            FVector TributaryEnd = BranchPoint + TributaryDirection * FMath::RandRange(2000.0f, 8000.0f);
            TributaryEnd.Z += FMath::RandRange(200.0f, 600.0f); // Tributaries start higher
            
            AddTributary(BranchPoint, TributaryEnd, RiverNetwork.MainRiver[BranchIndex].Width * 0.6f);
        }
    }

    // Update spline and create mesh
    RiverSpline->UpdateSpline();
    GenerateSplinePoints();
    CreateRiverMesh();
    ApplyMaterials();

    UE_LOG(LogTemp, Log, TEXT("Generated river system with %d main segments and %d tributaries"), 
           RiverNetwork.MainRiver.Num(), RiverNetwork.Tributaries.Num());
}

void AWorld_RiverSystemGenerator::AddTributary(FVector BranchPoint, FVector EndPoint, float BranchWidth)
{
    // Create tributary segments (simplified - 3 segments per tributary)
    int32 TributarySegments = 3;
    for (int32 i = 0; i < TributarySegments; i++)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(TributarySegments - 1);
        FVector SegmentStart = (i == 0) ? EndPoint : RiverNetwork.Tributaries.Last().EndLocation;
        FVector SegmentEnd = FMath::Lerp(EndPoint, BranchPoint, Alpha);
        
        // Add some randomness to tributary path
        SegmentEnd += FVector(
            FMath::RandRange(-500.0f, 500.0f),
            FMath::RandRange(-500.0f, 500.0f),
            0.0f
        );

        FWorld_RiverSegment TributarySegment;
        TributarySegment.StartLocation = SegmentStart;
        TributarySegment.EndLocation = SegmentEnd;
        TributarySegment.Width = BranchWidth * FMath::Lerp(0.3f, 1.0f, 1.0f - Alpha);
        TributarySegment.Depth = TributarySegment.Width * 0.2f;
        TributarySegment.FlowSpeed = 1.5f;
        TributarySegment.BiomeType = EBiomeType::Temperate;

        RiverNetwork.Tributaries.Add(TributarySegment);
    }
}

void AWorld_RiverSystemGenerator::ApplyErosionToTerrain()
{
    // Get landscape reference
    UWorld* World = GetWorld();
    if (!World) return;

    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (!Landscape) continue;

        // Apply erosion along river path
        for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
        {
            FVector StartLoc = Segment.StartLocation;
            FVector EndLoc = Segment.EndLocation;
            
            // Sample points along segment for erosion
            int32 ErosionSamples = 10;
            for (int32 i = 0; i <= ErosionSamples; i++)
            {
                float Alpha = static_cast<float>(i) / static_cast<float>(ErosionSamples);
                FVector ErosionPoint = FMath::Lerp(StartLoc, EndLoc, Alpha);
                
                // Apply erosion effect (simplified - would need landscape editing API)
                float ErosionRadius = Segment.Width * 0.8f;
                float ErosionDepth = Segment.Depth * ErosionStrength;
                
                // Log erosion application for debugging
                UE_LOG(LogTemp, Log, TEXT("Applying erosion at %s with radius %f and depth %f"), 
                       *ErosionPoint.ToString(), ErosionRadius, ErosionDepth);
            }
        }
        break; // Only process first landscape found
    }
}

void AWorld_RiverSystemGenerator::SpawnRiverVegetation()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn vegetation along riverbanks
    for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
    {
        FVector StartLoc = Segment.StartLocation;
        FVector EndLoc = Segment.EndLocation;
        FVector Direction = (EndLoc - StartLoc).GetSafeNormal();
        FVector RightVector = FVector::CrossProduct(Direction, FVector::UpVector);
        
        // Sample points along segment
        int32 VegetationSamples = FMath::RandRange(5, 12);
        for (int32 i = 0; i <= VegetationSamples; i++)
        {
            float Alpha = static_cast<float>(i) / static_cast<float>(VegetationSamples);
            FVector BasePoint = FMath::Lerp(StartLoc, EndLoc, Alpha);
            
            // Spawn vegetation on both sides of river
            for (int32 Side = -1; Side <= 1; Side += 2)
            {
                FVector VegLocation = BasePoint + RightVector * Side * (Segment.Width * 0.8f + FMath::RandRange(100.0f, 500.0f));
                
                // Trace to ground
                FHitResult HitResult;
                FVector TraceStart = VegLocation + FVector(0, 0, 1000);
                FVector TraceEnd = VegLocation - FVector(0, 0, 1000);
                
                if (World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic))
                {
                    // Would spawn vegetation actor here
                    UE_LOG(LogTemp, Log, TEXT("Spawning river vegetation at %s"), *HitResult.Location.ToString());
                }
            }
        }
    }
}

void AWorld_RiverSystemGenerator::CreateRiverSounds()
{
    // Create ambient river sounds based on flow characteristics
    for (int32 i = 0; i < RiverNetwork.MainRiver.Num(); i++)
    {
        const FWorld_RiverSegment& Segment = RiverNetwork.MainRiver[i];
        
        // Determine sound type based on flow speed and width
        FString SoundType;
        if (Segment.FlowSpeed > 1.5f && Segment.Width < 400.0f)
        {
            SoundType = TEXT("FastStream");
        }
        else if (Segment.Width > 800.0f)
        {
            SoundType = TEXT("WideRiver");
        }
        else
        {
            SoundType = TEXT("GentleFlow");
        }
        
        FVector SoundLocation = FMath::Lerp(Segment.StartLocation, Segment.EndLocation, 0.5f);
        UE_LOG(LogTemp, Log, TEXT("Creating %s sound at %s"), *SoundType, *SoundLocation.ToString());
    }
}

void AWorld_RiverSystemGenerator::RegenerateRiver()
{
    if (RiverNetwork.SourceLocation != FVector::ZeroVector && RiverNetwork.DeltaLocation != FVector::ZeroVector)
    {
        GenerateRiverSystem(RiverNetwork.SourceLocation, RiverNetwork.DeltaLocation, 8);
        ApplyErosionToTerrain();
        SpawnRiverVegetation();
        CreateRiverSounds();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot regenerate river - source or delta location not set"));
    }
}

FWorld_RiverSegment AWorld_RiverSystemGenerator::GetNearestRiverSegment(FVector Location) const
{
    if (RiverNetwork.MainRiver.Num() == 0)
    {
        return FWorld_RiverSegment();
    }

    float MinDistance = MAX_FLT;
    int32 NearestIndex = 0;

    for (int32 i = 0; i < RiverNetwork.MainRiver.Num(); i++)
    {
        const FWorld_RiverSegment& Segment = RiverNetwork.MainRiver[i];
        FVector SegmentCenter = FMath::Lerp(Segment.StartLocation, Segment.EndLocation, 0.5f);
        float Distance = FVector::Dist(Location, SegmentCenter);
        
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestIndex = i;
        }
    }

    return RiverNetwork.MainRiver[NearestIndex];
}

bool AWorld_RiverSystemGenerator::IsLocationNearWater(FVector Location, float MaxDistance) const
{
    FWorld_RiverSegment NearestSegment = GetNearestRiverSegment(Location);
    if (NearestSegment.Width == 0.0f) return false;

    FVector SegmentCenter = FMath::Lerp(NearestSegment.StartLocation, NearestSegment.EndLocation, 0.5f);
    float Distance = FVector::Dist(Location, SegmentCenter);
    
    return Distance <= (MaxDistance + NearestSegment.Width * 0.5f);
}

float AWorld_RiverSystemGenerator::GetWaterDepthAtLocation(FVector Location) const
{
    if (!IsLocationNearWater(Location, 100.0f))
    {
        return 0.0f;
    }

    FWorld_RiverSegment NearestSegment = GetNearestRiverSegment(Location);
    FVector SegmentCenter = FMath::Lerp(NearestSegment.StartLocation, NearestSegment.EndLocation, 0.5f);
    float Distance = FVector::Dist2D(Location, SegmentCenter);
    
    // Calculate depth based on distance from river center
    float DistanceRatio = FMath::Clamp(Distance / (NearestSegment.Width * 0.5f), 0.0f, 1.0f);
    return NearestSegment.Depth * (1.0f - DistanceRatio);
}

FVector AWorld_RiverSystemGenerator::GetFlowDirectionAtLocation(FVector Location) const
{
    FWorld_RiverSegment NearestSegment = GetNearestRiverSegment(Location);
    if (NearestSegment.Width == 0.0f)
    {
        return FVector::ZeroVector;
    }

    return (NearestSegment.EndLocation - NearestSegment.StartLocation).GetSafeNormal();
}

void AWorld_RiverSystemGenerator::GenerateSplinePoints()
{
    if (RiverSpline && RiverNetwork.MainRiver.Num() > 0)
    {
        RiverSpline->ClearSplinePoints();
        
        for (int32 i = 0; i < RiverNetwork.MainRiver.Num(); i++)
        {
            const FWorld_RiverSegment& Segment = RiverNetwork.MainRiver[i];
            
            if (i == 0)
            {
                RiverSpline->AddSplinePoint(Segment.StartLocation, ESplineCoordinateSpace::World);
            }
            RiverSpline->AddSplinePoint(Segment.EndLocation, ESplineCoordinateSpace::World);
        }
        
        RiverSpline->UpdateSpline();
    }
}

void AWorld_RiverSystemGenerator::CreateRiverMesh()
{
    // Create procedural mesh for river (simplified implementation)
    if (RiverMesh && RiverSegmentMesh)
    {
        RiverMesh->SetStaticMesh(RiverSegmentMesh);
        
        // Scale mesh to match river dimensions
        if (RiverNetwork.MainRiver.Num() > 0)
        {
            float AvgWidth = 0.0f;
            for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
            {
                AvgWidth += Segment.Width;
            }
            AvgWidth /= RiverNetwork.MainRiver.Num();
            
            FVector MeshScale = FVector(RiverNetwork.TotalLength / 1000.0f, AvgWidth / 100.0f, 1.0f);
            RiverMesh->SetWorldScale3D(MeshScale);
        }
    }
}

void AWorld_RiverSystemGenerator::ApplyMaterials()
{
    if (RiverMesh && WaterMaterial)
    {
        UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(WaterMaterial, this);
        if (DynamicMaterial)
        {
            // Set material parameters based on river properties
            if (RiverNetwork.MainRiver.Num() > 0)
            {
                float AvgFlowSpeed = 0.0f;
                for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
                {
                    AvgFlowSpeed += Segment.FlowSpeed;
                }
                AvgFlowSpeed /= RiverNetwork.MainRiver.Num();
                
                DynamicMaterial->SetScalarParameterValue(TEXT("FlowSpeed"), AvgFlowSpeed);
                DynamicMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.7f);
            }
            
            RiverMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}

FVector AWorld_RiverSystemGenerator::CalculateMeanderingOffset(FVector BasePoint, float MeanderAmount)
{
    // Create natural river meandering using sine waves
    float NoiseX = FMath::Sin(BasePoint.Y * 0.001f) * MeanderAmount * 1000.0f;
    float NoiseY = FMath::Cos(BasePoint.X * 0.0008f) * MeanderAmount * 800.0f;
    
    return BasePoint + FVector(NoiseX, NoiseY, 0.0f);
}

void AWorld_RiverSystemGenerator::SpawnRiverRocks()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Spawn rocks in and around river
    for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
    {
        if (Segment.FlowSpeed > 1.2f) // Rocky areas in faster flowing sections
        {
            int32 RockCount = FMath::RandRange(2, 6);
            for (int32 i = 0; i < RockCount; i++)
            {
                FVector RockLocation = FMath::Lerp(Segment.StartLocation, Segment.EndLocation, FMath::RandRange(0.1f, 0.9f));
                RockLocation += FVector(
                    FMath::RandRange(-Segment.Width * 0.3f, Segment.Width * 0.3f),
                    FMath::RandRange(-Segment.Width * 0.3f, Segment.Width * 0.3f),
                    0.0f
                );
                
                UE_LOG(LogTemp, Log, TEXT("Spawning river rock at %s"), *RockLocation.ToString());
            }
        }
    }
}

void AWorld_RiverSystemGenerator::SpawnAquaticLife()
{
    // Spawn fish and aquatic creatures based on river characteristics
    for (const FWorld_RiverSegment& Segment : RiverNetwork.MainRiver)
    {
        if (Segment.Depth > 80.0f && Segment.FlowSpeed < 1.5f) // Deep, slow sections good for fish
        {
            FVector FishLocation = FMath::Lerp(Segment.StartLocation, Segment.EndLocation, 0.5f);
            FishLocation.Z -= Segment.Depth * 0.5f; // Place fish underwater
            
            UE_LOG(LogTemp, Log, TEXT("Spawning aquatic life at %s"), *FishLocation.ToString());
        }
    }
}
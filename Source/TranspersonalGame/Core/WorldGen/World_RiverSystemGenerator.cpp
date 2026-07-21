#include "World_RiverSystemGenerator.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AWorld_RiverSystemGenerator::AWorld_RiverSystemGenerator()
{
    PrimaryActorTick.bCanEverTick = false;

    // Initialize components
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    MainRiverSpline = CreateDefaultSubobject<USplineComponent>(TEXT("MainRiverSpline"));
    MainRiverSpline->SetupAttachment(RootComponent);

    RiverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RiverMesh"));
    RiverMesh->SetupAttachment(RootComponent);

    // Initialize default values
    RiverSeed = 12345;
    RiverLength = 10000.0f;
    RiverWidth = 500.0f;
    MeanderIntensity = 0.3f;
    TributaryCount = 3;
    ElevationInfluence = 0.7f;
    
    WaterTransparency = 0.6f;
    WaterColor = FLinearColor(0.2f, 0.4f, 0.8f, 0.6f);
    RiparianZoneWidth = 1000.0f;
    WildlifeSpawnRadius = 2000.0f;
    bEnableFishSpawning = true;

    // Initialize biome array
    RiverBiomes.Add(EBiomeType::Temperate);
    RiverBiomes.Add(EBiomeType::Tropical);
}

void AWorld_RiverSystemGenerator::BeginPlay()
{
    Super::BeginPlay();
    InitializeRiverComponents();
}

void AWorld_RiverSystemGenerator::InitializeRiverComponents()
{
    if (MainRiverSpline)
    {
        MainRiverSpline->ClearSplinePoints();
        MainRiverSpline->SetSplinePointType(0, ESplinePointType::CurveClamped);
    }

    if (RiverMesh && WaterMaterial)
    {
        RiverMesh->SetMaterial(0, WaterMaterial);
    }
}

void AWorld_RiverSystemGenerator::GenerateRiverSystem()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("World_RiverSystemGenerator: No valid world context"));
        return;
    }

    // Clear existing river
    ClearGeneratedRiver();

    // Set random seed
    FMath::RandInit(RiverSeed);

    // Generate main river from random start to end points
    FVector StartPoint = GetActorLocation() + FVector(FMath::RandRange(-2000.0f, 2000.0f), FMath::RandRange(-2000.0f, 2000.0f), 0.0f);
    FVector EndPoint = StartPoint + FVector(FMath::RandRange(5000.0f, 15000.0f), FMath::RandRange(-3000.0f, 3000.0f), FMath::RandRange(-500.0f, -100.0f));

    GenerateMainRiver(StartPoint, EndPoint);
    GenerateTributaries();
    CreateRiverMesh();
    ApplyWaterPhysics();
    SpawnRiparianVegetation();
    
    if (bEnableFishSpawning)
    {
        SpawnAquaticWildlife();
    }

    UE_LOG(LogTemp, Log, TEXT("World_RiverSystemGenerator: River system generated with %d points"), RiverPoints.Num());
}

void AWorld_RiverSystemGenerator::GenerateMainRiver(const FVector& StartPoint, const FVector& EndPoint)
{
    if (!MainRiverSpline)
    {
        return;
    }

    RiverPoints.Empty();
    MainRiverSpline->ClearSplinePoints();

    // Calculate number of segments based on river length
    float Distance = FVector::Dist(StartPoint, EndPoint);
    int32 NumSegments = FMath::Max(5, FMath::FloorToInt(Distance / 1000.0f));

    // Generate meandering river points
    RiverPoints.Add(StartPoint);
    
    for (int32 i = 1; i < NumSegments; i++)
    {
        float Alpha = static_cast<float>(i) / static_cast<float>(NumSegments);
        FVector BasePoint = FMath::Lerp(StartPoint, EndPoint, Alpha);
        
        // Add meander
        FVector MeanderPoint = CalculateMeanderPoint(StartPoint, EndPoint, MeanderIntensity, i);
        FVector FinalPoint = FMath::Lerp(BasePoint, MeanderPoint, MeanderIntensity);
        
        // Apply elevation influence
        FinalPoint.Z += FMath::Sin(Alpha * PI * 2.0f) * ElevationInfluence * 100.0f;
        
        RiverPoints.Add(FinalPoint);
    }
    
    RiverPoints.Add(EndPoint);

    // Update spline with generated points
    UpdateSplineFromPoints();

    // Create main river segments for the network
    GeneratedNetwork.MainRiver.Empty();
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        EBiomeType SegmentBiome = RiverBiomes.Num() > 0 ? RiverBiomes[FMath::RandRange(0, RiverBiomes.Num() - 1)] : EBiomeType::Temperate;
        FWorld_RiverSegment Segment = CreateRiverSegment(RiverPoints[i], RiverPoints[i + 1], RiverWidth, SegmentBiome);
        GeneratedNetwork.MainRiver.Add(Segment);
    }

    GeneratedNetwork.TotalLength = Distance;
}

void AWorld_RiverSystemGenerator::GenerateTributaries()
{
    if (RiverPoints.Num() < 3)
    {
        return;
    }

    GeneratedNetwork.Tributaries.Empty();

    for (int32 i = 0; i < TributaryCount; i++)
    {
        // Select random point along main river (not start or end)
        int32 MainPointIndex = FMath::RandRange(1, RiverPoints.Num() - 2);
        FVector TributaryStart = RiverPoints[MainPointIndex];
        
        // Generate tributary direction (perpendicular to main river flow)
        FVector MainFlow = (RiverPoints[MainPointIndex + 1] - RiverPoints[MainPointIndex - 1]).GetSafeNormal();
        FVector TributaryDirection = FVector::CrossProduct(MainFlow, FVector::UpVector).GetSafeNormal();
        
        // Randomize direction
        if (FMath::RandBool())
        {
            TributaryDirection *= -1.0f;
        }

        // Generate tributary end point
        float TributaryLength = FMath::RandRange(1000.0f, 3000.0f);
        FVector TributaryEnd = TributaryStart + (TributaryDirection * TributaryLength);
        TributaryEnd.Z += FMath::RandRange(50.0f, 200.0f); // Tributaries flow downhill

        // Create tributary segment
        EBiomeType TributaryBiome = RiverBiomes.Num() > 0 ? RiverBiomes[FMath::RandRange(0, RiverBiomes.Num() - 1)] : EBiomeType::Temperate;
        float TributaryWidth = RiverWidth * FMath::RandRange(0.3f, 0.7f);
        FWorld_RiverSegment TributarySegment = CreateRiverSegment(TributaryEnd, TributaryStart, TributaryWidth, TributaryBiome);
        
        GeneratedNetwork.Tributaries.Add(TributarySegment);
    }

    GeneratedNetwork.NetworkComplexity = 1 + TributaryCount;
}

void AWorld_RiverSystemGenerator::CreateRiverMesh()
{
    if (!RiverMesh || !MainRiverSpline)
    {
        return;
    }

    GenerateRiverMeshFromSpline();
    
    if (WaterMaterial)
    {
        RiverMesh->SetMaterial(0, WaterMaterial);
    }

    // Set water color and transparency
    if (UMaterialInstanceDynamic* DynamicMaterial = RiverMesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        DynamicMaterial->SetVectorParameterValue(TEXT("WaterColor"), WaterColor);
        DynamicMaterial->SetScalarParameterValue(TEXT("Transparency"), WaterTransparency);
    }
}

void AWorld_RiverSystemGenerator::SpawnRiparianVegetation()
{
    if (!GetWorld() || RiverPoints.Num() < 2)
    {
        return;
    }

    // Spawn vegetation along river banks
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        FVector RiverDirection = (RiverPoints[i + 1] - RiverPoints[i]).GetSafeNormal();
        FVector RightBank = FVector::CrossProduct(RiverDirection, FVector::UpVector).GetSafeNormal();
        
        // Spawn vegetation on both sides
        for (int32 Side = -1; Side <= 1; Side += 2)
        {
            for (float Distance = RiverWidth; Distance < RiparianZoneWidth; Distance += 200.0f)
            {
                FVector VegetationPos = RiverPoints[i] + (RightBank * Side * Distance);
                VegetationPos.Z += FMath::RandRange(-50.0f, 50.0f);
                
                // Spawn vegetation actor (placeholder for now)
                if (FMath::RandRange(0.0f, 1.0f) < 0.3f) // 30% chance to spawn
                {
                    // This would spawn actual vegetation actors in a full implementation
                    UE_LOG(LogTemp, Log, TEXT("Spawning riparian vegetation at: %s"), *VegetationPos.ToString());
                }
            }
        }
    }
}

void AWorld_RiverSystemGenerator::SpawnAquaticWildlife()
{
    if (!GetWorld() || RiverPoints.Num() < 2)
    {
        return;
    }

    // Spawn fish and aquatic creatures along the river
    for (int32 i = 0; i < RiverPoints.Num(); i += 3) // Every 3rd point
    {
        FVector SpawnLocation = RiverPoints[i];
        SpawnLocation.Z -= 50.0f; // Underwater

        // This would spawn actual fish actors in a full implementation
        UE_LOG(LogTemp, Log, TEXT("Spawning aquatic wildlife at: %s"), *SpawnLocation.ToString());
    }
}

void AWorld_RiverSystemGenerator::ApplyWaterPhysics()
{
    if (!RiverMesh)
    {
        return;
    }

    // Enable collision for water interaction
    RiverMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RiverMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldStatic);
    RiverMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    
    // Set up water physics properties
    RiverMesh->SetMassOverrideInKg(NAME_None, 1000.0f, true);
}

void AWorld_RiverSystemGenerator::ClearGeneratedRiver()
{
    // Clear spline points
    if (MainRiverSpline)
    {
        MainRiverSpline->ClearSplinePoints();
    }

    // Clear generated data
    RiverPoints.Empty();
    GeneratedNetwork.MainRiver.Empty();
    GeneratedNetwork.Tributaries.Empty();
    GeneratedNetwork.LakePositions.Empty();
    GeneratedNetwork.TotalLength = 0.0f;
    GeneratedNetwork.NetworkComplexity = 0;

    // Destroy spawned actors
    for (AActor* Actor : SpawnedRiverActors)
    {
        if (IsValid(Actor))
        {
            Actor->Destroy();
        }
    }
    SpawnedRiverActors.Empty();
}

float AWorld_RiverSystemGenerator::CalculateRiverFlow(const FVector& Point) const
{
    // Calculate flow speed based on elevation and distance from source
    if (RiverPoints.Num() < 2)
    {
        return 1.0f;
    }

    float NearestDistance = FLT_MAX;
    int32 NearestIndex = 0;
    
    for (int32 i = 0; i < RiverPoints.Num(); i++)
    {
        float Distance = FVector::Dist(Point, RiverPoints[i]);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestIndex = i;
        }
    }

    // Flow increases towards the end (downstream)
    float FlowFactor = static_cast<float>(NearestIndex) / static_cast<float>(RiverPoints.Num() - 1);
    return FMath::Lerp(0.5f, 2.0f, FlowFactor);
}

FVector AWorld_RiverSystemGenerator::GetNearestRiverPoint(const FVector& WorldPosition) const
{
    if (RiverPoints.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector NearestPoint = RiverPoints[0];
    float NearestDistance = FVector::Dist(WorldPosition, RiverPoints[0]);

    for (const FVector& Point : RiverPoints)
    {
        float Distance = FVector::Dist(WorldPosition, Point);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestPoint = Point;
        }
    }

    return NearestPoint;
}

bool AWorld_RiverSystemGenerator::IsPointInRiver(const FVector& WorldPosition, float Tolerance) const
{
    FVector NearestPoint = GetNearestRiverPoint(WorldPosition);
    float Distance = FVector::Dist(WorldPosition, NearestPoint);
    return Distance <= (RiverWidth * 0.5f + Tolerance);
}

EBiomeType AWorld_RiverSystemGenerator::GetRiverBiomeAtPoint(const FVector& WorldPosition) const
{
    // Find the nearest river segment and return its biome
    if (GeneratedNetwork.MainRiver.Num() == 0)
    {
        return EBiomeType::Temperate;
    }

    float NearestDistance = FLT_MAX;
    EBiomeType NearestBiome = EBiomeType::Temperate;

    for (const FWorld_RiverSegment& Segment : GeneratedNetwork.MainRiver)
    {
        FVector SegmentCenter = (Segment.StartPoint + Segment.EndPoint) * 0.5f;
        float Distance = FVector::Dist(WorldPosition, SegmentCenter);
        
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestBiome = Segment.BiomeType;
        }
    }

    return NearestBiome;
}

FVector AWorld_RiverSystemGenerator::CalculateMeanderPoint(const FVector& Start, const FVector& End, float MeanderFactor, int32 SegmentIndex)
{
    FVector Direction = (End - Start).GetSafeNormal();
    FVector Perpendicular = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    
    // Create sinusoidal meander pattern
    float MeanderOffset = FMath::Sin(SegmentIndex * 0.5f) * MeanderFactor * 1000.0f;
    
    return Start + (Perpendicular * MeanderOffset);
}

void AWorld_RiverSystemGenerator::UpdateSplineFromPoints()
{
    if (!MainRiverSpline || RiverPoints.Num() < 2)
    {
        return;
    }

    MainRiverSpline->ClearSplinePoints();
    
    for (int32 i = 0; i < RiverPoints.Num(); i++)
    {
        FVector LocalPoint = GetActorTransform().InverseTransformPosition(RiverPoints[i]);
        MainRiverSpline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local);
        MainRiverSpline->SetSplinePointType(i, ESplinePointType::CurveClamped);
    }

    MainRiverSpline->UpdateSpline();
}

void AWorld_RiverSystemGenerator::GenerateRiverMeshFromSpline()
{
    // This would generate a procedural mesh from the spline in a full implementation
    // For now, we'll use the existing static mesh if available
    if (RiverMeshAsset)
    {
        RiverMesh->SetStaticMesh(RiverMeshAsset);
    }
}

FWorld_RiverSegment AWorld_RiverSystemGenerator::CreateRiverSegment(const FVector& Start, const FVector& End, float Width, EBiomeType Biome)
{
    FWorld_RiverSegment Segment;
    Segment.StartPoint = Start;
    Segment.EndPoint = End;
    Segment.Width = Width;
    Segment.Depth = Width * 0.2f; // Depth is 20% of width
    Segment.FlowSpeed = CalculateRiverFlow(Start);
    Segment.BiomeType = Biome;
    
    return Segment;
}
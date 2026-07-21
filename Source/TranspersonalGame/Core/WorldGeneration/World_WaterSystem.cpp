#include "World_WaterSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AWorld_WaterSystem::AWorld_WaterSystem()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default values
    GlobalWaterLevel = 0.0f;
    bEnableWaterFlow = true;
    WaterQuality = 1.0f;

    // Try to load default water plane mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> WaterPlaneMeshAsset(TEXT("/Engine/BasicShapes/Plane"));
    if (WaterPlaneMeshAsset.Succeeded())
    {
        WaterPlaneMesh = WaterPlaneMeshAsset.Object;
    }

    InitializeDefaultWaterSystem();
}

void AWorld_WaterSystem::BeginPlay()
{
    Super::BeginPlay();
    
    if (RiverSegments.Num() == 0 && Lakes.Num() == 0)
    {
        GenerateRiverSystem();
        ConnectBiomesWithRivers();
    }
}

void AWorld_WaterSystem::InitializeDefaultWaterSystem()
{
    // Initialize default river connecting biomes
    FWorld_RiverSegment MainRiver;
    MainRiver.StartPoint = FVector(-20000, 0, 50);
    MainRiver.EndPoint = FVector(25000, -15000, 30);
    MainRiver.Width = 2000.0f;
    MainRiver.FlowSpeed = 100.0f;
    RiverSegments.Add(MainRiver);

    // Initialize default lakes for each biome
    FWorld_Lake ForestLake;
    ForestLake.Location = FVector(-45000, 35000, 20);
    ForestLake.Radius = 3000.0f;
    ForestLake.BiomeType = EBiomeType::Forest;
    ForestLake.LakeName = TEXT("Forest Lake");
    Lakes.Add(ForestLake);

    FWorld_Lake SavannaWaterhole;
    SavannaWaterhole.Location = FVector(0, -5000, 25);
    SavannaWaterhole.Radius = 2000.0f;
    SavannaWaterhole.BiomeType = EBiomeType::Savanna;
    SavannaWaterhole.LakeName = TEXT("Savanna Waterhole");
    Lakes.Add(SavannaWaterhole);

    FWorld_Lake DesertOasis;
    DesertOasis.Location = FVector(45000, -35000, 15);
    DesertOasis.Radius = 1500.0f;
    DesertOasis.BiomeType = EBiomeType::Desert;
    DesertOasis.LakeName = TEXT("Desert Oasis");
    Lakes.Add(DesertOasis);
}

void AWorld_WaterSystem::GenerateRiverSystem()
{
    CleanupExistingWater();

    // Create river segments
    for (const FWorld_RiverSegment& Segment : RiverSegments)
    {
        CreateRiverSegmentMesh(Segment);
    }

    // Create lakes
    for (const FWorld_Lake& Lake : Lakes)
    {
        CreateLakeMesh(Lake);
    }

    UE_LOG(LogTemp, Warning, TEXT("Water system generated: %d rivers, %d lakes"), RiverSegments.Num(), Lakes.Num());
}

void AWorld_WaterSystem::CreateLake(const FWorld_Lake& LakeData)
{
    Lakes.Add(LakeData);
    CreateLakeMesh(LakeData);
}

void AWorld_WaterSystem::ConnectBiomesWithRivers()
{
    // Create additional river segments connecting major biomes
    TArray<FVector> BiomeLocations = {
        FVector(-45000, 40000, 20),  // Forest
        FVector(0, 0, 50),           // Savanna
        FVector(50000, -40000, 15)   // Desert
    };

    for (int32 i = 0; i < BiomeLocations.Num() - 1; i++)
    {
        FWorld_RiverSegment ConnectingRiver;
        ConnectingRiver.StartPoint = BiomeLocations[i];
        ConnectingRiver.EndPoint = BiomeLocations[i + 1];
        ConnectingRiver.Width = 1500.0f;
        ConnectingRiver.FlowSpeed = 75.0f;

        RiverSegments.Add(ConnectingRiver);
        CreateRiverSegmentMesh(ConnectingRiver);
    }
}

void AWorld_WaterSystem::RegenerateWaterSystem()
{
    GenerateRiverSystem();
}

bool AWorld_WaterSystem::IsLocationNearWater(const FVector& Location, float MaxDistance) const
{
    // Check distance to lakes
    for (const FWorld_Lake& Lake : Lakes)
    {
        float Distance = FVector::Dist(Location, Lake.Location);
        if (Distance <= MaxDistance + Lake.Radius)
        {
            return true;
        }
    }

    // Check distance to rivers
    for (const FWorld_RiverSegment& Segment : RiverSegments)
    {
        FVector ClosestPoint = FMath::ClosestPointOnSegment(Location, Segment.StartPoint, Segment.EndPoint);
        float Distance = FVector::Dist(Location, ClosestPoint);
        if (Distance <= MaxDistance + Segment.Width * 0.5f)
        {
            return true;
        }
    }

    return false;
}

FVector AWorld_WaterSystem::GetNearestWaterSource(const FVector& Location) const
{
    FVector NearestWater = FVector::ZeroVector;
    float MinDistance = FLT_MAX;

    // Check lakes
    for (const FWorld_Lake& Lake : Lakes)
    {
        float Distance = FVector::Dist(Location, Lake.Location);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestWater = Lake.Location;
        }
    }

    // Check rivers
    for (const FWorld_RiverSegment& Segment : RiverSegments)
    {
        FVector ClosestPoint = FMath::ClosestPointOnSegment(Location, Segment.StartPoint, Segment.EndPoint);
        float Distance = FVector::Dist(Location, ClosestPoint);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NearestWater = ClosestPoint;
        }
    }

    return NearestWater;
}

float AWorld_WaterSystem::GetWaterDepthAtLocation(const FVector& Location) const
{
    // Check if location is in a lake
    for (const FWorld_Lake& Lake : Lakes)
    {
        float Distance = FVector::Dist2D(Location, Lake.Location);
        if (Distance <= Lake.Radius)
        {
            // Calculate depth based on distance from shore
            float DepthRatio = 1.0f - (Distance / Lake.Radius);
            return Lake.Depth * DepthRatio;
        }
    }

    // Check if location is in a river
    for (const FWorld_RiverSegment& Segment : RiverSegments)
    {
        FVector ClosestPoint = FMath::ClosestPointOnSegment(Location, Segment.StartPoint, Segment.EndPoint);
        float Distance = FVector::Dist2D(Location, ClosestPoint);
        if (Distance <= Segment.Width * 0.5f)
        {
            return Segment.Depth;
        }
    }

    return 0.0f;
}

void AWorld_WaterSystem::SetSeasonalWaterLevels(float SeasonMultiplier)
{
    GlobalWaterLevel = 50.0f * SeasonMultiplier;
    
    // Adjust lake levels
    for (FWorld_Lake& Lake : Lakes)
    {
        Lake.Depth = FMath::Max(50.0f, Lake.Depth * SeasonMultiplier);
    }

    // Adjust river flow
    for (FWorld_RiverSegment& Segment : RiverSegments)
    {
        Segment.FlowSpeed = FMath::Max(10.0f, Segment.FlowSpeed * SeasonMultiplier);
    }
}

void AWorld_WaterSystem::CreateRiverSegmentMesh(const FWorld_RiverSegment& Segment)
{
    if (!WaterPlaneMesh)
    {
        return;
    }

    UStaticMeshComponent* RiverMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("RiverMesh_%d"), WaterMeshComponents.Num())
    );
    
    if (RiverMesh)
    {
        RiverMesh->SetStaticMesh(WaterPlaneMesh);
        RiverMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

        // Position and scale the river segment
        FVector MidPoint = (Segment.StartPoint + Segment.EndPoint) * 0.5f;
        FVector Direction = (Segment.EndPoint - Segment.StartPoint).GetSafeNormal();
        float Length = FVector::Dist(Segment.StartPoint, Segment.EndPoint);

        RiverMesh->SetWorldLocation(MidPoint);
        RiverMesh->SetWorldScale3D(FVector(Length / 100.0f, Segment.Width / 100.0f, 1.0f));

        // Rotate to align with river direction
        FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
        RiverMesh->SetWorldRotation(Rotation);

        if (WaterMaterial)
        {
            RiverMesh->SetMaterial(0, WaterMaterial);
        }

        WaterMeshComponents.Add(RiverMesh);
    }
}

void AWorld_WaterSystem::CreateLakeMesh(const FWorld_Lake& Lake)
{
    if (!WaterPlaneMesh)
    {
        return;
    }

    UStaticMeshComponent* LakeMesh = CreateDefaultSubobject<UStaticMeshComponent>(
        *FString::Printf(TEXT("LakeMesh_%s"), *Lake.LakeName)
    );
    
    if (LakeMesh)
    {
        LakeMesh->SetStaticMesh(WaterPlaneMesh);
        LakeMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);

        LakeMesh->SetWorldLocation(Lake.Location);
        float Scale = Lake.Radius / 100.0f;
        LakeMesh->SetWorldScale3D(FVector(Scale, Scale, 1.0f));

        if (WaterMaterial)
        {
            LakeMesh->SetMaterial(0, WaterMaterial);
        }

        WaterMeshComponents.Add(LakeMesh);
    }
}

void AWorld_WaterSystem::UpdateWaterFlow()
{
    if (!bEnableWaterFlow)
    {
        return;
    }

    // Update water flow animations and effects
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (UStaticMeshComponent* WaterMesh : WaterMeshComponents)
    {
        if (WaterMesh && WaterMesh->GetMaterial(0))
        {
            // Apply time-based water flow effects
            // This would typically involve updating material parameters
        }
    }
}

void AWorld_WaterSystem::CleanupExistingWater()
{
    for (UStaticMeshComponent* WaterMesh : WaterMeshComponents)
    {
        if (WaterMesh)
        {
            WaterMesh->DestroyComponent();
        }
    }
    WaterMeshComponents.Empty();
}
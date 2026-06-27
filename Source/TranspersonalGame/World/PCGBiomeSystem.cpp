#include "PCGBiomeSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"

APCGBiomeSystem::APCGBiomeSystem()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.5f; // Tick every 0.5s for performance
}

void APCGBiomeSystem::BeginPlay()
{
    Super::BeginPlay();

    if (bAutoGenerateOnBeginPlay)
    {
        InitializeDefaultBiomes();
        InitializeDefaultRivers();
        GenerateBiomeZones();
        GenerateRiverNetwork();
    }
}

void APCGBiomeSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // Reserved for dynamic biome updates (weather influence, seasonal changes)
}

void APCGBiomeSystem::InitializeDefaultBiomes()
{
    BiomeZones.Empty();

    // River Delta biome — north, near water
    FWorld_BiomeZone RiverDelta;
    RiverDelta.BiomeType = EWorld_BiomeType::RiverDelta;
    RiverDelta.Center = FVector(0.0f, 2000.0f, 0.0f);
    RiverDelta.Radius = 3000.0f;
    RiverDelta.VegetationDensity = 0.8f;
    RiverDelta.WaterCoverage = 0.6f;
    RiverDelta.BiomeTint = FLinearColor(0.1f, 0.5f, 0.9f, 1.0f);
    BiomeZones.Add(RiverDelta);

    // Dense Forest biome — west
    FWorld_BiomeZone DenseForest;
    DenseForest.BiomeType = EWorld_BiomeType::DenseForest;
    DenseForest.Center = FVector(-2000.0f, -1000.0f, 0.0f);
    DenseForest.Radius = 2500.0f;
    DenseForest.VegetationDensity = 1.0f;
    DenseForest.WaterCoverage = 0.1f;
    DenseForest.BiomeTint = FLinearColor(0.1f, 0.7f, 0.1f, 1.0f);
    BiomeZones.Add(DenseForest);

    // Open Plains biome — east, large
    FWorld_BiomeZone OpenPlains;
    OpenPlains.BiomeType = EWorld_BiomeType::OpenPlains;
    OpenPlains.Center = FVector(2000.0f, -1000.0f, 0.0f);
    OpenPlains.Radius = 3500.0f;
    OpenPlains.VegetationDensity = 0.3f;
    OpenPlains.WaterCoverage = 0.0f;
    OpenPlains.BiomeTint = FLinearColor(0.8f, 0.7f, 0.2f, 1.0f);
    BiomeZones.Add(OpenPlains);

    // Rocky Highland biome — south, elevated
    FWorld_BiomeZone RockyHighland;
    RockyHighland.BiomeType = EWorld_BiomeType::RockyHighland;
    RockyHighland.Center = FVector(0.0f, -3000.0f, 300.0f);
    RockyHighland.Radius = 2000.0f;
    RockyHighland.VegetationDensity = 0.2f;
    RockyHighland.WaterCoverage = 0.0f;
    RockyHighland.BiomeTint = FLinearColor(0.6f, 0.5f, 0.3f, 1.0f);
    BiomeZones.Add(RockyHighland);
}

void APCGBiomeSystem::InitializeDefaultRivers()
{
    RiverSegments.Empty();

    // Main river channel — flows north to delta
    FWorld_RiverSegment MainRiver;
    MainRiver.StartPoint = FVector(0.0f, -500.0f, 5.0f);
    MainRiver.EndPoint = FVector(200.0f, 4500.0f, 3.0f);
    MainRiver.Width = 250.0f;
    MainRiver.Depth = 80.0f;
    MainRiver.FlowSpeed = 120.0f;
    RiverSegments.Add(MainRiver);

    // West tributary
    FWorld_RiverSegment WestTributary;
    WestTributary.StartPoint = FVector(-1500.0f, 500.0f, 8.0f);
    WestTributary.EndPoint = FVector(-300.0f, 1200.0f, 5.0f);
    WestTributary.Width = 120.0f;
    WestTributary.Depth = 40.0f;
    WestTributary.FlowSpeed = 80.0f;
    RiverSegments.Add(WestTributary);

    // East tributary
    FWorld_RiverSegment EastTributary;
    EastTributary.StartPoint = FVector(1200.0f, 800.0f, 8.0f);
    EastTributary.EndPoint = FVector(800.0f, 1500.0f, 5.0f);
    EastTributary.Width = 100.0f;
    EastTributary.Depth = 35.0f;
    EastTributary.FlowSpeed = 90.0f;
    RiverSegments.Add(EastTributary);
}

void APCGBiomeSystem::GenerateBiomeZones()
{
    // If no zones defined, initialize defaults
    if (BiomeZones.Num() == 0)
    {
        InitializeDefaultBiomes();
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Generated %d biome zones"), BiomeZones.Num());

    // Debug visualization in editor
#if WITH_EDITOR
    UWorld* World = GetWorld();
    if (World)
    {
        for (const FWorld_BiomeZone& Zone : BiomeZones)
        {
            FColor DebugColor = FColor(
                FMath::Clamp(FMath::RoundToInt(Zone.BiomeTint.R * 255), 0, 255),
                FMath::Clamp(FMath::RoundToInt(Zone.BiomeTint.G * 255), 0, 255),
                FMath::Clamp(FMath::RoundToInt(Zone.BiomeTint.B * 255), 0, 255)
            );
            DrawDebugSphere(World, Zone.Center, Zone.Radius, 16, DebugColor, false, 30.0f, 0, 5.0f);
        }
    }
#endif
}

void APCGBiomeSystem::GenerateRiverNetwork()
{
    if (RiverSegments.Num() == 0)
    {
        InitializeDefaultRivers();
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Generated %d river segments"), RiverSegments.Num());
}

EWorld_BiomeType APCGBiomeSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    EWorld_BiomeType ClosestBiome = EWorld_BiomeType::OpenPlains;
    float ClosestDistance = MAX_FLT;

    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Zone.Center);
        if (Distance < ClosestDistance)
        {
            ClosestDistance = Distance;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

float APCGBiomeSystem::GetBiomeBlendWeight(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const
{
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            float Distance = FVector::Dist2D(WorldLocation, Zone.Center);
            if (Distance >= Zone.Radius)
            {
                return 0.0f;
            }
            // Smooth blend at edges (last 20% of radius)
            float BlendStart = Zone.Radius * 0.8f;
            if (Distance < BlendStart)
            {
                return 1.0f;
            }
            float BlendRange = Zone.Radius - BlendStart;
            return 1.0f - ((Distance - BlendStart) / BlendRange);
        }
    }
    return 0.0f;
}

bool APCGBiomeSystem::IsLocationInRiver(const FVector& WorldLocation) const
{
    for (const FWorld_RiverSegment& Segment : RiverSegments)
    {
        // Project point onto river segment line
        FVector SegDir = Segment.EndPoint - Segment.StartPoint;
        float SegLength = SegDir.Size();
        if (SegLength < KINDA_SMALL_NUMBER) continue;

        FVector SegDirNorm = SegDir / SegLength;
        FVector ToPoint = WorldLocation - Segment.StartPoint;
        float Projection = FVector::DotProduct(ToPoint, SegDirNorm);

        // Clamp to segment
        Projection = FMath::Clamp(Projection, 0.0f, SegLength);
        FVector ClosestPoint = Segment.StartPoint + SegDirNorm * Projection;

        float DistToRiver = FVector::Dist2D(WorldLocation, ClosestPoint);
        if (DistToRiver <= Segment.Width * 0.5f)
        {
            return true;
        }
    }
    return false;
}

void APCGBiomeSystem::ScatterVegetationForBiome(EWorld_BiomeType BiomeType, int32 Count)
{
    // Find the biome zone
    const FWorld_BiomeZone* TargetZone = nullptr;
    for (const FWorld_BiomeZone& Zone : BiomeZones)
    {
        if (Zone.BiomeType == BiomeType)
        {
            TargetZone = &Zone;
            break;
        }
    }

    if (!TargetZone)
    {
        UE_LOG(LogTemp, Warning, TEXT("PCGBiomeSystem: BiomeType not found for vegetation scatter"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PCGBiomeSystem: Scattering %d vegetation items in biome at (%.0f, %.0f)"),
        Count, TargetZone->Center.X, TargetZone->Center.Y);

    // Actual spawning is handled via Python/Blueprint for editor workflow
    // Runtime spawning would use FoliageManager integration
}

float APCGBiomeSystem::CalculateDistanceToBiomeEdge(const FVector& Location, const FWorld_BiomeZone& Zone) const
{
    float DistToCenter = FVector::Dist2D(Location, Zone.Center);
    return Zone.Radius - DistToCenter;
}

#include "World_CretaceousLandscapeBuilder.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeProxy.h"
#include "Landscape/LandscapeInfo.h"
#include "LandscapeEditorObject.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AWorld_CretaceousLandscapeBuilder::AWorld_CretaceousLandscapeBuilder()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default configuration
    BaseTerrainHeight = 0.0f;
    NoiseScale = 0.01f;
    NoiseIntensity = 500.0f;
    RiverWidth = 300.0f;
    RiverDepth = 100.0f;
    MainLandscape = nullptr;
    
    // Setup default biome zones for Cretaceous period
    SetupBiomeZones();
}

void AWorld_CretaceousLandscapeBuilder::BeginPlay()
{
    Super::BeginPlay();
    
    // Find existing landscape in the level
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        MainLandscape = Cast<ALandscape>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeBuilder: Found existing landscape: %s"), 
               MainLandscape ? *MainLandscape->GetName() : TEXT("NULL"));
    }
}

bool AWorld_CretaceousLandscapeBuilder::CreateCretaceousLandscape()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeBuilder: No valid world context"));
        return false;
    }

    // Check if landscape already exists
    if (MainLandscape && IsValid(MainLandscape))
    {
        UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeBuilder: Landscape already exists, skipping creation"));
        return true;
    }

    // Create heightmap data
    const int32 HeightmapWidth = 1009; // Standard UE5 landscape size
    const int32 HeightmapHeight = 1009;
    TArray<uint16> HeightData;
    
    if (!GenerateHeightmapData(HeightData, HeightmapWidth, HeightmapHeight))
    {
        UE_LOG(LogTemp, Error, TEXT("CretaceousLandscapeBuilder: Failed to generate heightmap data"));
        return false;
    }

    // Apply river system to heightmap
    ApplyRiverToHeightmap(HeightData, HeightmapWidth, HeightmapHeight);

    UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeBuilder: Generated heightmap with %d points"), HeightData.Num());
    return true;
}

bool AWorld_CretaceousLandscapeBuilder::GenerateHeightmapData(TArray<uint16>& HeightData, int32 Width, int32 Height)
{
    HeightData.Empty();
    HeightData.Reserve(Width * Height);

    const float WorldScale = 10000.0f; // 10km world
    const float HeightScale = 1000.0f; // Max height variation

    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            // Convert to world coordinates
            float WorldX = (X / float(Width - 1) - 0.5f) * WorldScale;
            float WorldY = (Y / float(Height - 1) - 0.5f) * WorldScale;
            
            FVector2D Location(WorldX, WorldY);
            float Height = CalculateHeightAtLocation(Location);
            
            // Convert to heightmap value (0-65535)
            uint16 HeightValue = FloatToHeightmapValue(Height);
            HeightData.Add(HeightValue);
        }
    }

    return HeightData.Num() == (Width * Height);
}

void AWorld_CretaceousLandscapeBuilder::SetupBiomeZones()
{
    BiomeZones.Empty();

    // Swamp (Southwest)
    FWorld_BiomeZoneData SwampZone;
    SwampZone.BiomeType = EBiomeType::Swamp;
    SwampZone.ZoneCenter = FVector2D(-3000.0f, -3000.0f);
    SwampZone.ZoneRadius = 2500.0f;
    SwampZone.HeightVariation = 200.0f;
    BiomeZones.Add(SwampZone);

    // Forest (Northwest)
    FWorld_BiomeZoneData ForestZone;
    ForestZone.BiomeType = EBiomeType::Forest;
    ForestZone.ZoneCenter = FVector2D(-3000.0f, 3000.0f);
    ForestZone.ZoneRadius = 2500.0f;
    ForestZone.HeightVariation = 400.0f;
    BiomeZones.Add(ForestZone);

    // Plains (Center)
    FWorld_BiomeZoneData PlainsZone;
    PlainsZone.BiomeType = EBiomeType::Plains;
    PlainsZone.ZoneCenter = FVector2D(0.0f, 0.0f);
    PlainsZone.ZoneRadius = 2000.0f;
    PlainsZone.HeightVariation = 300.0f;
    BiomeZones.Add(PlainsZone);

    // Desert (East)
    FWorld_BiomeZoneData DesertZone;
    DesertZone.BiomeType = EBiomeType::Desert;
    DesertZone.ZoneCenter = FVector2D(4000.0f, 0.0f);
    DesertZone.ZoneRadius = 2000.0f;
    DesertZone.HeightVariation = 600.0f;
    BiomeZones.Add(DesertZone);

    // Mountains (Northeast)
    FWorld_BiomeZoneData MountainZone;
    MountainZone.BiomeType = EBiomeType::Mountains;
    MountainZone.ZoneCenter = FVector2D(3000.0f, 3000.0f);
    MountainZone.ZoneRadius = 2000.0f;
    MountainZone.HeightVariation = 1000.0f;
    BiomeZones.Add(MountainZone);

    // Generate river system
    GenerateRiverSystem();
}

EBiomeType AWorld_CretaceousLandscapeBuilder::GetBiomeAtLocation(FVector2D Location) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Plains;

    for (const FWorld_BiomeZoneData& Zone : BiomeZones)
    {
        float Distance = FVector2D::Distance(Location, Zone.ZoneCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Zone.BiomeType;
        }
    }

    return ClosestBiome;
}

float AWorld_CretaceousLandscapeBuilder::CalculateHeightAtLocation(FVector2D Location) const
{
    float BaseHeight = BaseTerrainHeight;
    
    // Find the biome zone for this location
    for (const FWorld_BiomeZoneData& Zone : BiomeZones)
    {
        float Distance = FVector2D::Distance(Location, Zone.ZoneCenter);
        if (Distance <= Zone.ZoneRadius)
        {
            // Calculate influence based on distance from zone center
            float Influence = 1.0f - (Distance / Zone.ZoneRadius);
            Influence = SmoothStep(0.0f, 1.0f, Influence);
            
            // Generate height based on biome type
            float BiomeHeight = 0.0f;
            switch (Zone.BiomeType)
            {
                case EBiomeType::Mountains:
                    BiomeHeight = Zone.HeightVariation * GeneratePerlinNoise(Location.X * 0.001f, Location.Y * 0.001f, 4.0f);
                    break;
                case EBiomeType::Swamp:
                    BiomeHeight = -Zone.HeightVariation * 0.3f + Zone.HeightVariation * 0.2f * GeneratePerlinNoise(Location.X * 0.002f, Location.Y * 0.002f, 2.0f);
                    break;
                case EBiomeType::Desert:
                    BiomeHeight = Zone.HeightVariation * 0.4f * GeneratePerlinNoise(Location.X * 0.0015f, Location.Y * 0.0015f, 3.0f);
                    break;
                case EBiomeType::Forest:
                    BiomeHeight = Zone.HeightVariation * 0.6f * GeneratePerlinNoise(Location.X * 0.0012f, Location.Y * 0.0012f, 3.5f);
                    break;
                case EBiomeType::Plains:
                default:
                    BiomeHeight = Zone.HeightVariation * 0.3f * GeneratePerlinNoise(Location.X * 0.002f, Location.Y * 0.002f, 2.5f);
                    break;
            }
            
            BaseHeight += BiomeHeight * Influence;
        }
    }

    // Check if near river (lower the terrain)
    if (IsLocationNearRiver(Location, RiverWidth))
    {
        float RiverDistance = FLT_MAX;
        for (int32 i = 0; i < RiverPath.Num() - 1; i++)
        {
            float SegmentDistance = FMath::PointDistToSegment(FVector(Location.X, Location.Y, 0.0f), 
                                                             FVector(RiverPath[i].X, RiverPath[i].Y, 0.0f),
                                                             FVector(RiverPath[i + 1].X, RiverPath[i + 1].Y, 0.0f)).Size();
            RiverDistance = FMath::Min(RiverDistance, SegmentDistance);
        }
        
        if (RiverDistance < RiverWidth)
        {
            float RiverInfluence = 1.0f - (RiverDistance / RiverWidth);
            BaseHeight -= RiverDepth * RiverInfluence;
        }
    }

    return BaseHeight;
}

void AWorld_CretaceousLandscapeBuilder::GenerateRiverSystem()
{
    RiverPath.Empty();
    
    // Main river flowing from mountains to swamp
    RiverPath.Add(FVector2D(2500.0f, 2500.0f));   // Start in mountains
    RiverPath.Add(FVector2D(1000.0f, 1500.0f));   // Flow through forest edge
    RiverPath.Add(FVector2D(0.0f, 500.0f));       // Through plains
    RiverPath.Add(FVector2D(-1500.0f, -500.0f));  // Towards swamp
    RiverPath.Add(FVector2D(-2500.0f, -2000.0f)); // End in swamp
}

bool AWorld_CretaceousLandscapeBuilder::IsLocationNearRiver(FVector2D Location, float Threshold) const
{
    for (int32 i = 0; i < RiverPath.Num() - 1; i++)
    {
        float Distance = FMath::PointDistToSegment(FVector(Location.X, Location.Y, 0.0f), 
                                                  FVector(RiverPath[i].X, RiverPath[i].Y, 0.0f),
                                                  FVector(RiverPath[i + 1].X, RiverPath[i + 1].Y, 0.0f)).Size();
        if (Distance <= Threshold)
        {
            return true;
        }
    }
    return false;
}

FVector AWorld_CretaceousLandscapeBuilder::GetLandscapeSize() const
{
    if (MainLandscape)
    {
        FVector Origin, BoxExtent;
        MainLandscape->GetActorBounds(false, Origin, BoxExtent);
        return BoxExtent * 2.0f;
    }
    return FVector(10000.0f, 10000.0f, 1000.0f); // Default 10km x 10km
}

bool AWorld_CretaceousLandscapeBuilder::IsLandscapeValid() const
{
    return MainLandscape && IsValid(MainLandscape);
}

void AWorld_CretaceousLandscapeBuilder::BuildLandscapeInEditor()
{
    CreateCretaceousLandscape();
}

void AWorld_CretaceousLandscapeBuilder::RegenerateBiomes()
{
    SetupBiomeZones();
    UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeBuilder: Regenerated %d biome zones"), BiomeZones.Num());
}

void AWorld_CretaceousLandscapeBuilder::ValidateLandscapeSetup()
{
    UE_LOG(LogTemp, Warning, TEXT("CretaceousLandscapeBuilder: Validating landscape setup..."));
    UE_LOG(LogTemp, Warning, TEXT("  - Biome zones: %d"), BiomeZones.Num());
    UE_LOG(LogTemp, Warning, TEXT("  - River path points: %d"), RiverPath.Num());
    UE_LOG(LogTemp, Warning, TEXT("  - Landscape valid: %s"), IsLandscapeValid() ? TEXT("Yes") : TEXT("No"));
    
    if (MainLandscape)
    {
        FVector Size = GetLandscapeSize();
        UE_LOG(LogTemp, Warning, TEXT("  - Landscape size: %.0f x %.0f x %.0f"), Size.X, Size.Y, Size.Z);
    }
}

float AWorld_CretaceousLandscapeBuilder::GeneratePerlinNoise(float X, float Y, float Scale) const
{
    // Simple Perlin-like noise implementation
    float Noise = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    
    for (int32 i = 0; i < 4; i++)
    {
        Noise += FMath::Sin(X * Frequency) * FMath::Cos(Y * Frequency) * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return FMath::Clamp(Noise, -1.0f, 1.0f);
}

float AWorld_CretaceousLandscapeBuilder::SmoothStep(float Edge0, float Edge1, float X) const
{
    float T = FMath::Clamp((X - Edge0) / (Edge1 - Edge0), 0.0f, 1.0f);
    return T * T * (3.0f - 2.0f * T);
}

uint16 AWorld_CretaceousLandscapeBuilder::FloatToHeightmapValue(float Height) const
{
    // Convert float height to 16-bit heightmap value
    float NormalizedHeight = (Height + 1000.0f) / 2000.0f; // Assume height range -1000 to +1000
    return FMath::Clamp(FMath::RoundToInt(NormalizedHeight * 65535.0f), 0, 65535);
}

void AWorld_CretaceousLandscapeBuilder::ApplyRiverToHeightmap(TArray<uint16>& HeightData, int32 Width, int32 Height)
{
    const float WorldScale = 10000.0f;
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            float WorldX = (X / float(Width - 1) - 0.5f) * WorldScale;
            float WorldY = (Y / float(Height - 1) - 0.5f) * WorldScale;
            FVector2D Location(WorldX, WorldY);
            
            if (IsLocationNearRiver(Location, RiverWidth))
            {
                int32 Index = Y * Width + X;
                if (HeightData.IsValidIndex(Index))
                {
                    // Lower the terrain for river
                    uint16 CurrentHeight = HeightData[Index];
                    uint16 RiverHeight = FloatToHeightmapValue(-RiverDepth);
                    HeightData[Index] = FMath::Min(CurrentHeight, RiverHeight);
                }
            }
        }
    }
}
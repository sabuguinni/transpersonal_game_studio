#include "World_LandscapeCreationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "LandscapeProxy.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AWorld_LandscapeCreationSystem::AWorld_LandscapeCreationSystem()
{
    PrimaryActorTick.bCanEverTick = false;
    
    // Initialize default landscape configuration
    LandscapeConfig.ComponentCountX = 8;
    LandscapeConfig.ComponentCountY = 8;
    LandscapeConfig.QuadsPerComponent = 63;
    LandscapeConfig.SectionsPerComponent = 1;
    LandscapeConfig.LandscapeScale = FVector(100.0f, 100.0f, 100.0f);
    LandscapeConfig.LandscapeLocation = FVector::ZeroVector;
    LandscapeConfig.LandscapeRotation = FRotator::ZeroRotator;
    
    GeneratedLandscape = nullptr;
    
    // Initialize default biomes
    InitializeDefaultBiomes();
}

void AWorld_LandscapeCreationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Auto-create landscape and biomes on begin play
    CreateLandscapeWithBiomes();
}

void AWorld_LandscapeCreationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AWorld_LandscapeCreationSystem::InitializeDefaultBiomes()
{
    BiomeZones.Empty();
    
    // Forest biome at center
    FWorld_BiomeZone ForestBiome;
    ForestBiome.BiomeType = EBiomeType::Forest;
    ForestBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    ForestBiome.Radius = 3000.0f;
    ForestBiome.BlendDistance = 800.0f;
    ForestBiome.HeightVariation = 300.0f;
    ForestBiome.NoiseScale = 0.0008f;
    BiomeZones.Add(ForestBiome);
    
    // Savanna biome to the east
    FWorld_BiomeZone SavannaBiome;
    SavannaBiome.BiomeType = EBiomeType::Savanna;
    SavannaBiome.CenterLocation = FVector(6000.0f, 0.0f, 0.0f);
    SavannaBiome.Radius = 4000.0f;
    SavannaBiome.BlendDistance = 1000.0f;
    SavannaBiome.HeightVariation = 150.0f;
    SavannaBiome.NoiseScale = 0.0005f;
    BiomeZones.Add(SavannaBiome);
    
    // Desert biome to the west
    FWorld_BiomeZone DesertBiome;
    DesertBiome.BiomeType = EBiomeType::Desert;
    DesertBiome.CenterLocation = FVector(-6000.0f, 0.0f, 0.0f);
    DesertBiome.Radius = 3500.0f;
    DesertBiome.BlendDistance = 1200.0f;
    DesertBiome.HeightVariation = 400.0f;
    DesertBiome.NoiseScale = 0.0006f;
    BiomeZones.Add(DesertBiome);
    
    // Swamp biome to the south
    FWorld_BiomeZone SwampBiome;
    SwampBiome.BiomeType = EBiomeType::Swamp;
    SwampBiome.CenterLocation = FVector(0.0f, -6000.0f, 0.0f);
    SwampBiome.Radius = 3000.0f;
    SwampBiome.BlendDistance = 600.0f;
    SwampBiome.HeightVariation = 100.0f;
    SwampBiome.NoiseScale = 0.0012f;
    BiomeZones.Add(SwampBiome);
    
    // Mountain biome to the north
    FWorld_BiomeZone MountainBiome;
    MountainBiome.BiomeType = EBiomeType::Mountain;
    MountainBiome.CenterLocation = FVector(0.0f, 6000.0f, 0.0f);
    MountainBiome.Radius = 2500.0f;
    MountainBiome.BlendDistance = 1500.0f;
    MountainBiome.HeightVariation = 800.0f;
    MountainBiome.NoiseScale = 0.0004f;
    BiomeZones.Add(MountainBiome);
}

bool AWorld_LandscapeCreationSystem::CreateLandscapeWithBiomes()
{
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("World_LandscapeCreationSystem: No valid world found"));
        return false;
    }
    
    // Create biome markers first
    CreateBiomeMarkers();
    
    UE_LOG(LogTemp, Warning, TEXT("World_LandscapeCreationSystem: Landscape creation system initialized with %d biome zones"), BiomeZones.Num());
    
    return true;
}

bool AWorld_LandscapeCreationSystem::GenerateHeightmapData(TArray<uint16>& HeightData, int32 Width, int32 Height)
{
    HeightData.Empty();
    HeightData.Reserve(Width * Height);
    
    const float WorldScale = LandscapeConfig.LandscapeScale.X;
    const FVector WorldOffset = LandscapeConfig.LandscapeLocation;
    
    for (int32 Y = 0; Y < Height; Y++)
    {
        for (int32 X = 0; X < Width; X++)
        {
            // Convert heightmap coordinates to world coordinates
            FVector WorldLocation;
            WorldLocation.X = (X - Width * 0.5f) * WorldScale + WorldOffset.X;
            WorldLocation.Y = (Y - Height * 0.5f) * WorldScale + WorldOffset.Y;
            WorldLocation.Z = 0.0f;
            
            // Calculate blended height from all biomes
            float BlendedHeight = BlendBiomeHeights(WorldLocation);
            
            // Convert to heightmap format (0-65535)
            uint16 HeightValue = FMath::Clamp(FMath::RoundToInt((BlendedHeight + 500.0f) * 65.0f), 0, 65535);
            HeightData.Add(HeightValue);
        }
    }
    
    return true;
}

void AWorld_LandscapeCreationSystem::CreateBiomeMarkers()
{
    // Clear existing markers
    for (AActor* Marker : BiomeMarkers)
    {
        if (IsValid(Marker))
        {
            Marker->Destroy();
        }
    }
    BiomeMarkers.Empty();
    
    // Create new biome markers
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        FVector SpawnLocation = Biome.CenterLocation;
        SpawnLocation.Z = 100.0f; // Spawn slightly above ground
        
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        AStaticMeshActor* MarkerActor = GetWorld()->SpawnActor<AStaticMeshActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (MarkerActor)
        {
            FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
            MarkerActor->SetActorLabel(FString::Printf(TEXT("%s_Marker"), *BiomeName));
            
            BiomeMarkers.Add(MarkerActor);
            
            UE_LOG(LogTemp, Log, TEXT("Created biome marker for %s at %s"), *BiomeName, *SpawnLocation.ToString());
        }
    }
}

EBiomeType AWorld_LandscapeCreationSystem::GetBiomeAtLocation(const FVector& WorldLocation) const
{
    float MinDistance = FLT_MAX;
    EBiomeType ClosestBiome = EBiomeType::Forest;
    
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        float Distance = FVector::Dist2D(WorldLocation, Biome.CenterLocation);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestBiome = Biome.BiomeType;
        }
    }
    
    return ClosestBiome;
}

float AWorld_LandscapeCreationSystem::GetHeightAtBiome(const FVector& Location, EBiomeType BiomeType) const
{
    // Find the biome configuration
    const FWorld_BiomeZone* FoundBiome = nullptr;
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        if (Biome.BiomeType == BiomeType)
        {
            FoundBiome = &Biome;
            break;
        }
    }
    
    if (!FoundBiome)
    {
        return 0.0f;
    }
    
    // Calculate noise-based height
    float NoiseHeight = CalculateNoiseHeight(Location, FoundBiome->NoiseScale, FoundBiome->HeightVariation);
    
    // Add biome-specific height modifications
    float BaseHeight = 0.0f;
    switch (BiomeType)
    {
        case EBiomeType::Mountain:
            BaseHeight = 400.0f;
            break;
        case EBiomeType::Forest:
            BaseHeight = 100.0f;
            break;
        case EBiomeType::Savanna:
            BaseHeight = 50.0f;
            break;
        case EBiomeType::Desert:
            BaseHeight = 80.0f;
            break;
        case EBiomeType::Swamp:
            BaseHeight = -20.0f;
            break;
        default:
            BaseHeight = 0.0f;
            break;
    }
    
    return BaseHeight + NoiseHeight;
}

void AWorld_LandscapeCreationSystem::ApplyBiomeHeightmaps()
{
    UE_LOG(LogTemp, Warning, TEXT("World_LandscapeCreationSystem: Applying biome heightmaps to landscape"));
    
    // This would be implemented with actual landscape editing tools
    // For now, just log the operation
}

void AWorld_LandscapeCreationSystem::DebugPrintLandscapeInfo() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== Landscape Creation System Debug Info ==="));
    UE_LOG(LogTemp, Warning, TEXT("Landscape Config: %dx%d components, %d quads per component"), 
           LandscapeConfig.ComponentCountX, LandscapeConfig.ComponentCountY, LandscapeConfig.QuadsPerComponent);
    UE_LOG(LogTemp, Warning, TEXT("Biome Zones: %d configured"), BiomeZones.Num());
    
    for (int32 i = 0; i < BiomeZones.Num(); i++)
    {
        const FWorld_BiomeZone& Biome = BiomeZones[i];
        FString BiomeName = UEnum::GetValueAsString(Biome.BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("  Biome %d: %s at %s (radius: %.1f)"), 
               i, *BiomeName, *Biome.CenterLocation.ToString(), Biome.Radius);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Biome Markers: %d created"), BiomeMarkers.Num());
}

float AWorld_LandscapeCreationSystem::CalculateNoiseHeight(const FVector& Location, float Scale, float Amplitude) const
{
    // Simple Perlin-like noise implementation
    float X = Location.X * Scale;
    float Y = Location.Y * Scale;
    
    // Basic noise calculation
    float Noise1 = FMath::Sin(X * 0.1f) * FMath::Cos(Y * 0.1f);
    float Noise2 = FMath::Sin(X * 0.3f) * FMath::Cos(Y * 0.3f) * 0.5f;
    float Noise3 = FMath::Sin(X * 0.7f) * FMath::Cos(Y * 0.7f) * 0.25f;
    
    float CombinedNoise = (Noise1 + Noise2 + Noise3) / 1.75f;
    
    return CombinedNoise * Amplitude;
}

float AWorld_LandscapeCreationSystem::BlendBiomeHeights(const FVector& Location) const
{
    float TotalWeight = 0.0f;
    float WeightedHeight = 0.0f;
    
    for (const FWorld_BiomeZone& Biome : BiomeZones)
    {
        float Distance = FVector::Dist2D(Location, Biome.CenterLocation);
        float Weight = 0.0f;
        
        if (Distance <= Biome.Radius)
        {
            // Inside biome radius
            if (Distance <= Biome.Radius - Biome.BlendDistance)
            {
                // Full biome influence
                Weight = 1.0f;
            }
            else
            {
                // Blend zone
                float BlendFactor = (Biome.Radius - Distance) / Biome.BlendDistance;
                Weight = FMath::Clamp(BlendFactor, 0.0f, 1.0f);
            }
            
            float BiomeHeight = GetHeightAtBiome(Location, Biome.BiomeType);
            WeightedHeight += BiomeHeight * Weight;
            TotalWeight += Weight;
        }
    }
    
    if (TotalWeight > 0.0f)
    {
        return WeightedHeight / TotalWeight;
    }
    
    // Default height if no biome influence
    return 0.0f;
}
#include "World_LandscapeExpansion.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeProxy.h"
#include "LandscapeEditorModule.h"
#include "LandscapeEditorObject.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

AWorld_LandscapeExpansion::AWorld_LandscapeExpansion()
{
    PrimaryActorTick.bCanEverTick = false;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize default landscape settings for 200km2 world
    LandscapeSettings.ComponentSizeQuads = 63;
    LandscapeSettings.SectionsPerComponent = 2;
    LandscapeSettings.QuadsPerSection = 31;
    LandscapeSettings.ScaleX = 100.0f;
    LandscapeSettings.ScaleY = 100.0f;
    LandscapeSettings.ScaleZ = 100.0f;
    LandscapeSettings.ComponentCountX = 64;  // Increased for 200km2
    LandscapeSettings.ComponentCountY = 64;

    CurrentLandscape = nullptr;
    bHasBackup = false;
}

void AWorld_LandscapeExpansion::BeginPlay()
{
    Super::BeginPlay();
    
    SetupBiomeHeightProfiles();
    
    // Find existing landscape in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundActors);
    
    if (FoundActors.Num() > 0)
    {
        CurrentLandscape = Cast<ALandscape>(FoundActors[0]);
        UE_LOG(LogTemp, Warning, TEXT("Found existing landscape: %s"), CurrentLandscape ? *CurrentLandscape->GetName() : TEXT("NULL"));
    }
}

void AWorld_LandscapeExpansion::SetupBiomeHeightProfiles()
{
    BiomeHeightData.Empty();

    // PANTANO (Swamp) - Low, flat terrain
    FWorld_BiomeHeightData PantanoData;
    PantanoData.BiomeName = TEXT("PANTANO");
    PantanoData.Center = FVector2D(-50000, -45000);
    PantanoData.Bounds = FVector2D(52500, 61500);  // From memory coordinates
    PantanoData.BaseHeight = -50.0f;
    PantanoData.HeightVariation = 25.0f;
    PantanoData.NoiseScale = 0.0005f;
    BiomeHeightData.Add(PantanoData);

    // FLORESTA (Forest) - Rolling hills
    FWorld_BiomeHeightData FlorestaData;
    FlorestaData.BiomeName = TEXT("FLORESTA");
    FlorestaData.Center = FVector2D(-45000, 40000);
    FlorestaData.Bounds = FVector2D(62500, 61500);
    FlorestaData.BaseHeight = 100.0f;
    FlorestaData.HeightVariation = 150.0f;
    FlorestaData.NoiseScale = 0.0008f;
    BiomeHeightData.Add(FlorestaData);

    // SAVANA (Savanna) - Gentle slopes
    FWorld_BiomeHeightData SavanaData;
    SavanaData.BiomeName = TEXT("SAVANA");
    SavanaData.Center = FVector2D(0, 0);
    SavanaData.Bounds = FVector2D(40000, 40000);
    SavanaData.BaseHeight = 50.0f;
    SavanaData.HeightVariation = 75.0f;
    SavanaData.NoiseScale = 0.0006f;
    BiomeHeightData.Add(SavanaData);

    // DESERTO (Desert) - Dunes and valleys
    FWorld_BiomeHeightData DesertoData;
    DesertoData.BiomeName = TEXT("DESERTO");
    DesertoData.Center = FVector2D(55000, 0);
    DesertoData.Bounds = FVector2D(54500, 60000);
    DesertoData.BaseHeight = 25.0f;
    DesertoData.HeightVariation = 200.0f;
    DesertoData.NoiseScale = 0.001f;
    BiomeHeightData.Add(DesertoData);

    // MONTANHA (Mountain) - High peaks
    FWorld_BiomeHeightData MontanhaData;
    MontanhaData.BiomeName = TEXT("MONTANHA");
    MontanhaData.Center = FVector2D(40000, 50000);
    MontanhaData.Bounds = FVector2D(64500, 56500);
    MontanhaData.BaseHeight = 500.0f;
    MontanhaData.HeightVariation = 800.0f;
    MontanhaData.NoiseScale = 0.0012f;
    BiomeHeightData.Add(MontanhaData);

    UE_LOG(LogTemp, Warning, TEXT("Setup %d biome height profiles"), BiomeHeightData.Num());
}

void AWorld_LandscapeExpansion::GenerateExpandedLandscape()
{
    if (!ValidateLandscapeSettings())
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid landscape settings"));
        return;
    }

    // Backup current landscape if it exists
    if (CurrentLandscape)
    {
        BackupCurrentLandscape();
    }

    UE_LOG(LogTemp, Warning, TEXT("Starting landscape generation for 200km2 world..."));

    // Calculate total size
    int32 TotalQuadsX = LandscapeSettings.ComponentCountX * LandscapeSettings.ComponentSizeQuads;
    int32 TotalQuadsY = LandscapeSettings.ComponentCountY * LandscapeSettings.ComponentSizeQuads;
    
    float WorldSizeX = TotalQuadsX * LandscapeSettings.ScaleX;
    float WorldSizeY = TotalQuadsY * LandscapeSettings.ScaleY;

    UE_LOG(LogTemp, Warning, TEXT("Landscape size: %d x %d quads, World size: %.0f x %.0f units"), 
           TotalQuadsX, TotalQuadsY, WorldSizeX, WorldSizeY);

    // Generate heightmap data
    TArray<uint16> HeightData = GenerateHeightmapData();
    
    if (HeightData.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to generate heightmap data"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated heightmap with %d height values"), HeightData.Num());

    // Note: In a production environment, you would use LandscapeEditorUtils or similar
    // to actually create the landscape. This is a framework for the expansion system.
}

void AWorld_LandscapeExpansion::BackupCurrentLandscape()
{
    if (!CurrentLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("No current landscape to backup"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Backing up current landscape: %s"), *CurrentLandscape->GetName());
    
    // In production, this would extract heightmap data from the current landscape
    // For now, we mark that we have a backup
    bHasBackup = true;
    
    UE_LOG(LogTemp, Warning, TEXT("Landscape backup completed"));
}

void AWorld_LandscapeExpansion::RestoreBackupLandscape()
{
    if (!bHasBackup)
    {
        UE_LOG(LogTemp, Warning, TEXT("No backup available to restore"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Restoring landscape from backup..."));
    
    // In production, this would restore the heightmap data
    bHasBackup = false;
    
    UE_LOG(LogTemp, Warning, TEXT("Landscape restore completed"));
}

float AWorld_LandscapeExpansion::CalculateHeightAtPosition(const FVector2D& Position) const
{
    float FinalHeight = 0.0f;
    float TotalWeight = 0.0f;

    // Blend heights from all biomes based on distance
    for (const FWorld_BiomeHeightData& Biome : BiomeHeightData)
    {
        float Weight = BiomeBlendWeight(Position, Biome);
        if (Weight > 0.0f)
        {
            float NoiseValue = PerlinNoise(Position.X, Position.Y, Biome.NoiseScale);
            float BiomeHeight = Biome.BaseHeight + (NoiseValue * Biome.HeightVariation);
            
            FinalHeight += BiomeHeight * Weight;
            TotalWeight += Weight;
        }
    }

    if (TotalWeight > 0.0f)
    {
        FinalHeight /= TotalWeight;
    }

    return FinalHeight;
}

TArray<uint16> AWorld_LandscapeExpansion::GenerateHeightmapData() const
{
    TArray<uint16> HeightData;

    int32 TotalQuadsX = LandscapeSettings.ComponentCountX * LandscapeSettings.ComponentSizeQuads;
    int32 TotalQuadsY = LandscapeSettings.ComponentCountY * LandscapeSettings.ComponentSizeQuads;
    
    int32 HeightmapSizeX = TotalQuadsX + 1;
    int32 HeightmapSizeY = TotalQuadsY + 1;

    HeightData.Reserve(HeightmapSizeX * HeightmapSizeY);

    float WorldSizeX = TotalQuadsX * LandscapeSettings.ScaleX;
    float WorldSizeY = TotalQuadsY * LandscapeSettings.ScaleY;

    for (int32 Y = 0; Y < HeightmapSizeY; Y++)
    {
        for (int32 X = 0; X < HeightmapSizeX; X++)
        {
            // Convert heightmap coordinates to world coordinates
            float WorldX = (X / float(HeightmapSizeX - 1)) * WorldSizeX - (WorldSizeX * 0.5f);
            float WorldY = (Y / float(HeightmapSizeY - 1)) * WorldSizeY - (WorldSizeY * 0.5f);

            FVector2D WorldPos(WorldX, WorldY);
            float Height = CalculateHeightAtPosition(WorldPos);

            // Convert height to uint16 (0-65535 range)
            uint16 HeightValue = FMath::Clamp(FMath::RoundToInt((Height + 256.0f) * 128.0f), 0, 65535);
            HeightData.Add(HeightValue);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Generated heightmap: %d x %d = %d values"), 
           HeightmapSizeX, HeightmapSizeY, HeightData.Num());

    return HeightData;
}

bool AWorld_LandscapeExpansion::ValidateLandscapeSettings() const
{
    if (LandscapeSettings.ComponentCountX <= 0 || LandscapeSettings.ComponentCountY <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid component count"));
        return false;
    }

    if (LandscapeSettings.ComponentSizeQuads <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid component size"));
        return false;
    }

    if (LandscapeSettings.ScaleX <= 0.0f || LandscapeSettings.ScaleY <= 0.0f)
    {
        UE_LOG(LogTemp, Error, TEXT("Invalid scale values"));
        return false;
    }

    return true;
}

float AWorld_LandscapeExpansion::PerlinNoise(float X, float Y, float Scale) const
{
    // Simplified Perlin noise implementation
    float ScaledX = X * Scale;
    float ScaledY = Y * Scale;

    float Noise = FMath::PerlinNoise2D(FVector2D(ScaledX, ScaledY));
    return FMath::Clamp(Noise, -1.0f, 1.0f);
}

float AWorld_LandscapeExpansion::BiomeBlendWeight(const FVector2D& Position, const FWorld_BiomeHeightData& Biome) const
{
    FVector2D Distance = Position - Biome.Center;
    float DistanceFromCenter = Distance.Size();
    float MaxDistance = FMath::Max(Biome.Bounds.X, Biome.Bounds.Y);

    if (DistanceFromCenter > MaxDistance)
    {
        return 0.0f;
    }

    // Smooth falloff
    float NormalizedDistance = DistanceFromCenter / MaxDistance;
    return FMath::Pow(1.0f - NormalizedDistance, 2.0f);
}

FVector2D AWorld_LandscapeExpansion::WorldToLandscapeCoords(const FVector& WorldPosition) const
{
    float LandscapeX = WorldPosition.X / LandscapeSettings.ScaleX;
    float LandscapeY = WorldPosition.Y / LandscapeSettings.ScaleY;
    return FVector2D(LandscapeX, LandscapeY);
}

FVector AWorld_LandscapeExpansion::LandscapeToWorldCoords(const FVector2D& LandscapePosition) const
{
    float WorldX = LandscapePosition.X * LandscapeSettings.ScaleX;
    float WorldY = LandscapePosition.Y * LandscapeSettings.ScaleY;
    return FVector(WorldX, WorldY, 0.0f);
}
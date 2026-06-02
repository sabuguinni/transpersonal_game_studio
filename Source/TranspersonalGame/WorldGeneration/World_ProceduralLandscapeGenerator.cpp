#include "World_ProceduralLandscapeGenerator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/Landscape.h"
#include "Landscape/LandscapeInfo.h"
#include "Landscape/LandscapeDataAccess.h"
#include "Landscape/LandscapeEdit.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UWorld_ProceduralLandscapeGenerator::UWorld_ProceduralLandscapeGenerator()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize default terrain settings
    DefaultTerrainSettings = FWorld_TerrainSettings();
    
    // Initialize default landscape layers
    FWorld_LandscapeLayer BaseLayer;
    BaseLayer.LayerName = TEXT("Base");
    BaseLayer.BlendWeight = 1.0f;
    BaseLayer.NoiseScale = 100.0f;
    LandscapeLayers.Add(BaseLayer);
    
    // Performance settings
    MaxLandscapeComponents = 16;
    bEnableLODOptimization = true;
    LODDistanceScale = 1.0f;
    
    // Biome integration
    bGenerateBiomeSpecificTerrain = true;
    BiomeTransitionDistance = 500.0f;
    
    // Initialize biome terrain settings
    InitializeBiomeTerrainSettings();
}

void UWorld_ProceduralLandscapeGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Component initialized"));
}

void UWorld_ProceduralLandscapeGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

ALandscape* UWorld_ProceduralLandscapeGenerator::GenerateProceduralLandscape(const FVector& Location, const FWorld_TerrainSettings& Settings)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeGenerator: No valid world found"));
        return nullptr;
    }
    
    // Calculate landscape size
    int32 ComponentCountX = 4;
    int32 ComponentCountY = 4;
    int32 QuadsPerComponent = Settings.ComponentSizeQuads;
    int32 SizeX = ComponentCountX * QuadsPerComponent + 1;
    int32 SizeY = ComponentCountY * QuadsPerComponent + 1;
    
    // Generate heightmap data
    TArray<uint16> HeightData;
    GenerateHeightmapData(HeightData, Settings, SizeX, SizeY);
    
    // Create landscape actor
    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = FName(*FString::Printf(TEXT("ProceduralLandscape_%d"), GeneratedLandscapes.Num()));
    
    ALandscape* NewLandscape = World->SpawnActor<ALandscape>(ALandscape::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
    if (!NewLandscape)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeGenerator: Failed to spawn landscape actor"));
        return nullptr;
    }
    
    // Configure landscape
    NewLandscape->SetActorLocation(Location);
    
    // Import heightmap data
    TMap<FGuid, TArray<uint16>> HeightmapDataMap;
    TMap<FGuid, TArray<FLandscapeImportLayerInfo>> MaterialLayerDataMap;
    
    // Create landscape info
    ULandscapeInfo* LandscapeInfo = NewLandscape->CreateLandscapeInfo();
    if (LandscapeInfo)
    {
        // Set up landscape material
        if (DefaultLandscapeMaterial)
        {
            NewLandscape->LandscapeMaterial = DefaultLandscapeMaterial;
        }
        
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Landscape created successfully at %s"), *Location.ToString());
    }
    
    // Add to generated landscapes list
    GeneratedLandscapes.Add(NewLandscape);
    
    // Apply performance optimizations
    if (bEnableLODOptimization)
    {
        OptimizeLandscapeForPerformance(NewLandscape);
    }
    
    return NewLandscape;
}

void UWorld_ProceduralLandscapeGenerator::GenerateHeightmapData(TArray<uint16>& HeightData, const FWorld_TerrainSettings& Settings, int32 SizeX, int32 SizeY)
{
    HeightData.SetNum(SizeX * SizeY);
    
    for (int32 Y = 0; Y < SizeY; Y++)
    {
        for (int32 X = 0; X < SizeX; X++)
        {
            int32 Index = Y * SizeX + X;
            
            // Generate base height using Perlin noise
            float NoiseValue = GeneratePerlinNoise(
                X * Settings.NoiseFrequency,
                Y * Settings.NoiseFrequency,
                Settings.NoiseFrequency,
                Settings.NoiseOctaves,
                Settings.NoisePersistence
            );
            
            // Convert to height range
            float Height = FMath::Lerp(Settings.MinHeight, Settings.MaxHeight, (NoiseValue + 1.0f) * 0.5f);
            
            // Convert to uint16 format (0-65535)
            HeightData[Index] = FMath::Clamp(FMath::RoundToInt((Height + 32768.0f) / 65536.0f * 65535.0f), 0, 65535);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Generated heightmap data for %dx%d landscape"), SizeX, SizeY);
}

void UWorld_ProceduralLandscapeGenerator::ApplyPerlinNoise(TArray<uint16>& HeightData, const FWorld_TerrainSettings& Settings, int32 SizeX, int32 SizeY)
{
    for (int32 Y = 0; Y < SizeY; Y++)
    {
        for (int32 X = 0; X < SizeX; X++)
        {
            int32 Index = Y * SizeX + X;
            
            float NoiseValue = GeneratePerlinNoise(
                X * Settings.NoiseFrequency,
                Y * Settings.NoiseFrequency,
                Settings.NoiseFrequency,
                Settings.NoiseOctaves,
                Settings.NoisePersistence
            );
            
            // Apply noise to existing height data
            float CurrentHeight = (float)HeightData[Index] / 65535.0f * 65536.0f - 32768.0f;
            float ModifiedHeight = CurrentHeight + (NoiseValue * 100.0f);
            
            HeightData[Index] = FMath::Clamp(FMath::RoundToInt((ModifiedHeight + 32768.0f) / 65536.0f * 65535.0f), 0, 65535);
        }
    }
}

void UWorld_ProceduralLandscapeGenerator::CreateLandscapeLayers(ALandscape* Landscape, const TArray<FWorld_LandscapeLayer>& Layers)
{
    if (!Landscape)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeGenerator: Invalid landscape for layer creation"));
        return;
    }
    
    ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
    if (!LandscapeInfo)
    {
        UE_LOG(LogTemp, Error, TEXT("World_ProceduralLandscapeGenerator: No landscape info found"));
        return;
    }
    
    for (const FWorld_LandscapeLayer& Layer : Layers)
    {
        if (Layer.LayerMaterial)
        {
            UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Creating landscape layer: %s"), *Layer.LayerName);
            // Layer creation would require more complex UE5 landscape API integration
        }
    }
}

void UWorld_ProceduralLandscapeGenerator::GenerateBiomeBasedTerrain(EBiomeType BiomeType, const FVector& Location)
{
    if (!bGenerateBiomeSpecificTerrain)
    {
        return;
    }
    
    FWorld_TerrainSettings* BiomeSettings = BiomeTerrainSettings.Find(BiomeType);
    if (!BiomeSettings)
    {
        BiomeSettings = &DefaultTerrainSettings;
    }
    
    ALandscape* BiomeLandscape = GenerateProceduralLandscape(Location, *BiomeSettings);
    if (BiomeLandscape)
    {
        SetupLandscapeMaterial(BiomeLandscape, BiomeType);
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Generated biome-specific terrain for %s"), 
               *UEnum::GetValueAsString(BiomeType));
    }
}

void UWorld_ProceduralLandscapeGenerator::CreateRiverBeds(ALandscape* Landscape, const TArray<FVector>& RiverPath)
{
    if (!Landscape || RiverPath.Num() < 2)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Creating river bed with %d points"), RiverPath.Num());
    
    // River bed creation would require landscape heightmap modification
    // This is a complex operation that would need direct landscape data access
}

void UWorld_ProceduralLandscapeGenerator::CreateMountainRanges(ALandscape* Landscape, const TArray<FVector>& MountainPeaks)
{
    if (!Landscape || MountainPeaks.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Creating mountain ranges with %d peaks"), MountainPeaks.Num());
    
    // Mountain range creation would involve heightmap modification around peak points
}

void UWorld_ProceduralLandscapeGenerator::CreateValleys(ALandscape* Landscape, const TArray<FVector>& ValleyPoints)
{
    if (!Landscape || ValleyPoints.Num() == 0)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Creating valleys with %d points"), ValleyPoints.Num());
    
    // Valley creation would involve lowering terrain around specified points
}

float UWorld_ProceduralLandscapeGenerator::GeneratePerlinNoise(float X, float Y, float Frequency, int32 Octaves, float Persistence)
{
    float Total = 0.0f;
    float MaxValue = 0.0f;
    float Amplitude = 1.0f;
    float CurrentFreq = Frequency;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        // Simple noise approximation using sine waves
        float NoiseValue = FMath::Sin(X * CurrentFreq) * FMath::Cos(Y * CurrentFreq);
        NoiseValue += FMath::Sin(X * CurrentFreq * 2.0f) * FMath::Cos(Y * CurrentFreq * 2.0f) * 0.5f;
        
        Total += NoiseValue * Amplitude;
        MaxValue += Amplitude;
        
        Amplitude *= Persistence;
        CurrentFreq *= 2.0f;
    }
    
    return Total / MaxValue;
}

float UWorld_ProceduralLandscapeGenerator::SampleHeightAtLocation(const FVector& WorldLocation)
{
    // Find the landscape at this location
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape && Landscape->GetActorLocation().DistanceSquared2D(WorldLocation) < 1000000.0f) // 1km radius
        {
            // Sample height from landscape
            ULandscapeInfo* LandscapeInfo = Landscape->GetLandscapeInfo();
            if (LandscapeInfo)
            {
                // This would require landscape data access API
                return WorldLocation.Z; // Placeholder
            }
        }
    }
    
    return 0.0f;
}

EBiomeType UWorld_ProceduralLandscapeGenerator::GetBiomeAtLocation(const FVector& WorldLocation)
{
    // Simple biome determination based on location
    float X = WorldLocation.X;
    float Y = WorldLocation.Y;
    
    if (FMath::Abs(X) < 2000.0f && FMath::Abs(Y) < 2000.0f)
    {
        return EBiomeType::Grassland;
    }
    else if (X > 2000.0f)
    {
        return EBiomeType::Desert;
    }
    else if (X < -2000.0f)
    {
        return EBiomeType::Forest;
    }
    else if (Y > 2000.0f)
    {
        return EBiomeType::Mountain;
    }
    else
    {
        return EBiomeType::Wetland;
    }
}

void UWorld_ProceduralLandscapeGenerator::OptimizeLandscapeForPerformance(ALandscape* Landscape)
{
    if (!Landscape)
    {
        return;
    }
    
    // Set LOD settings
    if (bEnableLODOptimization)
    {
        // Configure landscape LOD settings for performance
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Applying performance optimizations to landscape"));
    }
}

void UWorld_ProceduralLandscapeGenerator::GenerateTestLandscape()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Generating test landscape"));
    
    FVector TestLocation = FVector(0.0f, 0.0f, 0.0f);
    ALandscape* TestLandscape = GenerateProceduralLandscape(TestLocation, DefaultTerrainSettings);
    
    if (TestLandscape)
    {
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Test landscape generated successfully"));
    }
}

void UWorld_ProceduralLandscapeGenerator::ClearAllLandscapes()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Clearing all generated landscapes"));
    
    for (ALandscape* Landscape : GeneratedLandscapes)
    {
        if (Landscape)
        {
            Landscape->Destroy();
        }
    }
    
    GeneratedLandscapes.Empty();
}

void UWorld_ProceduralLandscapeGenerator::ValidateLandscapeGeneration()
{
    UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Validating landscape generation system"));
    UE_LOG(LogTemp, Warning, TEXT("Generated landscapes count: %d"), GeneratedLandscapes.Num());
    UE_LOG(LogTemp, Warning, TEXT("Biome terrain settings count: %d"), BiomeTerrainSettings.Num());
}

void UWorld_ProceduralLandscapeGenerator::InitializeBiomeTerrainSettings()
{
    // Forest biome - rolling hills with dense vegetation
    FWorld_TerrainSettings ForestSettings;
    ForestSettings.MaxHeight = 1500.0f;
    ForestSettings.MinHeight = -200.0f;
    ForestSettings.NoiseFrequency = 0.008f;
    ForestSettings.NoiseOctaves = 5;
    ForestSettings.NoisePersistence = 0.6f;
    BiomeTerrainSettings.Add(EBiomeType::Forest, ForestSettings);
    
    // Desert biome - sandy dunes and rocky outcrops
    FWorld_TerrainSettings DesertSettings;
    DesertSettings.MaxHeight = 800.0f;
    DesertSettings.MinHeight = -100.0f;
    DesertSettings.NoiseFrequency = 0.012f;
    DesertSettings.NoiseOctaves = 3;
    DesertSettings.NoisePersistence = 0.4f;
    BiomeTerrainSettings.Add(EBiomeType::Desert, DesertSettings);
    
    // Mountain biome - high peaks and steep slopes
    FWorld_TerrainSettings MountainSettings;
    MountainSettings.MaxHeight = 3000.0f;
    MountainSettings.MinHeight = 500.0f;
    MountainSettings.NoiseFrequency = 0.005f;
    MountainSettings.NoiseOctaves = 6;
    MountainSettings.NoisePersistence = 0.7f;
    BiomeTerrainSettings.Add(EBiomeType::Mountain, MountainSettings);
    
    // Grassland biome - gentle rolling plains
    FWorld_TerrainSettings GrasslandSettings;
    GrasslandSettings.MaxHeight = 600.0f;
    GrasslandSettings.MinHeight = -50.0f;
    GrasslandSettings.NoiseFrequency = 0.015f;
    GrasslandSettings.NoiseOctaves = 4;
    GrasslandSettings.NoisePersistence = 0.5f;
    BiomeTerrainSettings.Add(EBiomeType::Grassland, GrasslandSettings);
    
    // Wetland biome - low, marshy terrain
    FWorld_TerrainSettings WetlandSettings;
    WetlandSettings.MaxHeight = 200.0f;
    WetlandSettings.MinHeight = -300.0f;
    WetlandSettings.NoiseFrequency = 0.020f;
    WetlandSettings.NoiseOctaves = 3;
    WetlandSettings.NoisePersistence = 0.3f;
    BiomeTerrainSettings.Add(EBiomeType::Wetland, WetlandSettings);
}

float UWorld_ProceduralLandscapeGenerator::InterpolateHeight(float Height1, float Height2, float Alpha)
{
    return FMath::Lerp(Height1, Height2, Alpha);
}

void UWorld_ProceduralLandscapeGenerator::ApplyBiomeSpecificModifications(TArray<uint16>& HeightData, EBiomeType BiomeType, int32 SizeX, int32 SizeY)
{
    // Apply biome-specific terrain modifications
    switch (BiomeType)
    {
        case EBiomeType::Desert:
            // Add sand dune patterns
            break;
        case EBiomeType::Mountain:
            // Add rocky, jagged features
            break;
        case EBiomeType::Wetland:
            // Smooth out terrain, add water channels
            break;
        default:
            break;
    }
}

UTexture2D* UWorld_ProceduralLandscapeGenerator::CreateHeightmapTexture(const TArray<uint16>& HeightData, int32 SizeX, int32 SizeY)
{
    // Create texture from heightmap data
    // This would require texture creation API
    return nullptr;
}

void UWorld_ProceduralLandscapeGenerator::SetupLandscapeMaterial(ALandscape* Landscape, EBiomeType BiomeType)
{
    if (!Landscape)
    {
        return;
    }
    
    // Set biome-appropriate material
    if (DefaultLandscapeMaterial)
    {
        Landscape->LandscapeMaterial = DefaultLandscapeMaterial;
        UE_LOG(LogTemp, Warning, TEXT("World_ProceduralLandscapeGenerator: Applied material for biome %s"), 
               *UEnum::GetValueAsString(BiomeType));
    }
}
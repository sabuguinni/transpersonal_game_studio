#include "World_AdvancedTerrainSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Landscape/Classes/Landscape.h"
#include "Landscape/Classes/LandscapeComponent.h"
#include "Landscape/Classes/LandscapeInfo.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/UnrealMathUtility.h"
#include "DrawDebugHelpers.h"

UWorld_AdvancedTerrainSystem::UWorld_AdvancedTerrainSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
    
    // Initialize default values
    TerrainResolution = 1024;
    GlobalHeightScale = 1000.0f;
    TerrainScale = 100.0f;
    MaxLODLevel = 5;
    StreamingDistance = 50000.0f;
    
    // Performance settings
    bEnableLODOptimization = true;
    bEnableTerrainStreaming = true;
    LODDistanceScale = 1.0f;
    MaxVisibleComponents = 64;
    
    // Initialize terrain data arrays
    HeightData.Reserve(TerrainResolution * TerrainResolution);
    NormalData.Reserve(TerrainResolution * TerrainResolution);
    SlopeData.Reserve(TerrainResolution * TerrainResolution);
}

void UWorld_AdvancedTerrainSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: BeginPlay started"));
    
    // Setup biome terrain configurations
    SetupBiomeTerrainConfigs();
    
    // Find or create main landscape
    if (!MainLandscape)
    {
        TArray<AActor*> FoundLandscapes;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ALandscape::StaticClass(), FoundLandscapes);
        
        if (FoundLandscapes.Num() > 0)
        {
            MainLandscape = Cast<ALandscape>(FoundLandscapes[0]);
            UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Found existing landscape"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: No landscape found, will create procedural terrain"));
        }
    }
    
    // Initialize terrain generation
    GenerateAdvancedTerrain();
}

void UWorld_AdvancedTerrainSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnableLODOptimization)
    {
        OptimizeTerrainLOD();
    }
    
    if (bEnableTerrainStreaming)
    {
        UpdateTerrainStreaming();
    }
}

void UWorld_AdvancedTerrainSystem::GenerateAdvancedTerrain()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Generating advanced terrain"));
    
    // Generate terrain for each biome
    for (const FWorld_BiomeTerrainConfig& BiomeConfig : BiomeTerrainConfigs)
    {
        GenerateBiomeSpecificTerrain(BiomeConfig);
    }
    
    // Apply global terrain modifications
    CreateTerrainLayers();
    ApplyTerrainMaterials();
    BlendBiomeTransitions();
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Advanced terrain generation completed"));
}

void UWorld_AdvancedTerrainSystem::GenerateBiomeSpecificTerrain(const FWorld_BiomeTerrainConfig& BiomeConfig)
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Generating terrain for biome: %s"), *BiomeConfig.BiomeName);
    
    FVector BiomeCenter = BiomeConfig.BiomeCenter;
    float BiomeRadius = BiomeConfig.BiomeRadius;
    float HeightVariation = BiomeConfig.HeightVariation;
    
    // Generate height data based on biome type
    for (int32 X = 0; X < TerrainResolution; X++)
    {
        for (int32 Y = 0; Y < TerrainResolution; Y++)
        {
            float WorldX = (X - TerrainResolution * 0.5f) * TerrainScale;
            float WorldY = (Y - TerrainResolution * 0.5f) * TerrainScale;
            
            FVector WorldPos(WorldX, WorldY, 0.0f);
            float DistanceToBiome = FVector::Dist2D(WorldPos, BiomeCenter);
            
            if (DistanceToBiome <= BiomeRadius)
            {
                float BiomeInfluence = FMath::Clamp(1.0f - (DistanceToBiome / BiomeRadius), 0.0f, 1.0f);
                float Height = 0.0f;
                
                switch (BiomeConfig.PrimaryTerrainType)
                {
                    case EWorld_TerrainType::Plains:
                        Height = GeneratePerlinNoise(WorldX, WorldY, 2, 0.001f, 0.5f) * 200.0f;
                        break;
                    case EWorld_TerrainType::Hills:
                        Height = GeneratePerlinNoise(WorldX, WorldY, 4, 0.0008f, 0.6f) * 600.0f;
                        break;
                    case EWorld_TerrainType::Mountains:
                        Height = GenerateRidgedNoise(WorldX, WorldY, 6, 0.0005f) * 1500.0f;
                        break;
                    case EWorld_TerrainType::Valleys:
                        Height = -GeneratePerlinNoise(WorldX, WorldY, 3, 0.0012f, 0.4f) * 300.0f;
                        break;
                    case EWorld_TerrainType::Cliffs:
                        Height = GenerateBillowNoise(WorldX, WorldY, 5, 0.0006f) * 1200.0f;
                        break;
                    default:
                        Height = GeneratePerlinNoise(WorldX, WorldY, BiomeConfig.OctaveCount, BiomeConfig.NoiseScale, 0.5f) * HeightVariation;
                        break;
                }
                
                Height *= BiomeInfluence;
                
                int32 Index = Y * TerrainResolution + X;
                if (HeightData.IsValidIndex(Index))
                {
                    HeightData[Index] = FMath::Max(HeightData[Index], Height);
                }
                else if (Index >= 0 && Index < TerrainResolution * TerrainResolution)
                {
                    HeightData.SetNum(TerrainResolution * TerrainResolution);
                    HeightData[Index] = Height;
                }
            }
        }
    }
}

void UWorld_AdvancedTerrainSystem::CreateTerrainLayers()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Creating terrain layers"));
    
    // Calculate normals and slopes for layer blending
    CalculateNormals();
    CalculateSlopes();
    
    // Apply erosion for realistic terrain features
    ApplyErosion(5, 0.1f);
    SmoothTerrain(3, 0.05f);
}

void UWorld_AdvancedTerrainSystem::ApplyTerrainMaterials()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Applying terrain materials"));
    
    if (TerrainParameterCollection)
    {
        UMaterialParameterCollectionInstance* ParameterInstance = GetWorld()->GetParameterCollectionInstance(TerrainParameterCollection);
        if (ParameterInstance)
        {
            ParameterInstance->SetScalarParameterValue(FName("TerrainScale"), TerrainScale);
            ParameterInstance->SetScalarParameterValue(FName("HeightScale"), GlobalHeightScale);
            ParameterInstance->SetScalarParameterValue(FName("LODDistanceScale"), LODDistanceScale);
        }
    }
}

void UWorld_AdvancedTerrainSystem::ModifyTerrainHeight(FVector Location, float Radius, float Height, bool bAdditive)
{
    int32 CenterX = FMath::RoundToInt((Location.X / TerrainScale) + (TerrainResolution * 0.5f));
    int32 CenterY = FMath::RoundToInt((Location.Y / TerrainScale) + (TerrainResolution * 0.5f));
    int32 RadiusPixels = FMath::RoundToInt(Radius / TerrainScale);
    
    for (int32 X = CenterX - RadiusPixels; X <= CenterX + RadiusPixels; X++)
    {
        for (int32 Y = CenterY - RadiusPixels; Y <= CenterY + RadiusPixels; Y++)
        {
            if (X >= 0 && X < TerrainResolution && Y >= 0 && Y < TerrainResolution)
            {
                float Distance = FMath::Sqrt(FMath::Square(X - CenterX) + FMath::Square(Y - CenterY));
                if (Distance <= RadiusPixels)
                {
                    float Falloff = 1.0f - (Distance / RadiusPixels);
                    float ModifiedHeight = Height * Falloff;
                    
                    int32 Index = Y * TerrainResolution + X;
                    if (HeightData.IsValidIndex(Index))
                    {
                        if (bAdditive)
                        {
                            HeightData[Index] += ModifiedHeight;
                        }
                        else
                        {
                            HeightData[Index] = ModifiedHeight;
                        }
                    }
                }
            }
        }
    }
}

void UWorld_AdvancedTerrainSystem::CreateRiver(const TArray<FVector>& RiverPoints, float RiverWidth, float RiverDepth)
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Creating river with %d points"), RiverPoints.Num());
    
    for (int32 i = 0; i < RiverPoints.Num() - 1; i++)
    {
        FVector StartPoint = RiverPoints[i];
        FVector EndPoint = RiverPoints[i + 1];
        
        float SegmentLength = FVector::Dist(StartPoint, EndPoint);
        int32 Steps = FMath::Max(1, FMath::RoundToInt(SegmentLength / 100.0f));
        
        for (int32 Step = 0; Step <= Steps; Step++)
        {
            float Alpha = (float)Step / (float)Steps;
            FVector CurrentPoint = FMath::Lerp(StartPoint, EndPoint, Alpha);
            
            ModifyTerrainHeight(CurrentPoint, RiverWidth, -RiverDepth, true);
        }
    }
}

void UWorld_AdvancedTerrainSystem::CreateLake(FVector Center, float Radius, float Depth)
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Creating lake at (%f, %f, %f)"), Center.X, Center.Y, Center.Z);
    
    ModifyTerrainHeight(Center, Radius, -Depth, true);
    
    // Create smooth edges
    for (float EdgeRadius = Radius; EdgeRadius < Radius * 1.5f; EdgeRadius += 50.0f)
    {
        float EdgeDepth = -Depth * (1.0f - ((EdgeRadius - Radius) / (Radius * 0.5f)));
        ModifyTerrainHeight(Center, EdgeRadius, EdgeDepth * 0.3f, true);
    }
}

void UWorld_AdvancedTerrainSystem::CreateCanyon(FVector StartPoint, FVector EndPoint, float Width, float Depth)
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Creating canyon from (%f, %f) to (%f, %f)"), 
           StartPoint.X, StartPoint.Y, EndPoint.X, EndPoint.Y);
    
    float CanyonLength = FVector::Dist(StartPoint, EndPoint);
    int32 Steps = FMath::Max(1, FMath::RoundToInt(CanyonLength / 200.0f));
    
    for (int32 Step = 0; Step <= Steps; Step++)
    {
        float Alpha = (float)Step / (float)Steps;
        FVector CurrentPoint = FMath::Lerp(StartPoint, EndPoint, Alpha);
        
        // Create canyon walls with varying depth
        float CurrentDepth = Depth * (0.5f + 0.5f * FMath::Sin(Alpha * PI));
        ModifyTerrainHeight(CurrentPoint, Width, -CurrentDepth, true);
        
        // Create sloped walls
        ModifyTerrainHeight(CurrentPoint, Width * 1.5f, -CurrentDepth * 0.3f, true);
    }
}

float UWorld_AdvancedTerrainSystem::GetTerrainHeightAtLocation(FVector WorldLocation)
{
    int32 X = FMath::RoundToInt((WorldLocation.X / TerrainScale) + (TerrainResolution * 0.5f));
    int32 Y = FMath::RoundToInt((WorldLocation.Y / TerrainScale) + (TerrainResolution * 0.5f));
    
    if (X >= 0 && X < TerrainResolution && Y >= 0 && Y < TerrainResolution)
    {
        int32 Index = Y * TerrainResolution + X;
        if (HeightData.IsValidIndex(Index))
        {
            return HeightData[Index];
        }
    }
    
    return 0.0f;
}

FVector UWorld_AdvancedTerrainSystem::GetTerrainNormalAtLocation(FVector WorldLocation)
{
    int32 X = FMath::RoundToInt((WorldLocation.X / TerrainScale) + (TerrainResolution * 0.5f));
    int32 Y = FMath::RoundToInt((WorldLocation.Y / TerrainScale) + (TerrainResolution * 0.5f));
    
    if (X >= 0 && X < TerrainResolution && Y >= 0 && Y < TerrainResolution)
    {
        int32 Index = Y * TerrainResolution + X;
        if (NormalData.IsValidIndex(Index))
        {
            return NormalData[Index];
        }
    }
    
    return FVector::UpVector;
}

float UWorld_AdvancedTerrainSystem::GetTerrainSlopeAtLocation(FVector WorldLocation)
{
    int32 X = FMath::RoundToInt((WorldLocation.X / TerrainScale) + (TerrainResolution * 0.5f));
    int32 Y = FMath::RoundToInt((WorldLocation.Y / TerrainScale) + (TerrainResolution * 0.5f));
    
    if (X >= 0 && X < TerrainResolution && Y >= 0 && Y < TerrainResolution)
    {
        int32 Index = Y * TerrainResolution + X;
        if (SlopeData.IsValidIndex(Index))
        {
            return SlopeData[Index];
        }
    }
    
    return 0.0f;
}

EWorld_TerrainType UWorld_AdvancedTerrainSystem::GetTerrainTypeAtLocation(FVector WorldLocation)
{
    float Height = GetTerrainHeightAtLocation(WorldLocation);
    float Slope = GetTerrainSlopeAtLocation(WorldLocation);
    
    if (Height < -100.0f)
    {
        return EWorld_TerrainType::Valleys;
    }
    else if (Height > 800.0f)
    {
        return EWorld_TerrainType::Mountains;
    }
    else if (Slope > 45.0f)
    {
        return EWorld_TerrainType::Cliffs;
    }
    else if (Height > 300.0f && Height < 600.0f)
    {
        return EWorld_TerrainType::Hills;
    }
    else
    {
        return EWorld_TerrainType::Plains;
    }
}

void UWorld_AdvancedTerrainSystem::SetupBiomeTerrainConfigs()
{
    BiomeTerrainConfigs.Empty();
    
    // Savana (Plains with gentle hills)
    FWorld_BiomeTerrainConfig SavanaConfig;
    SavanaConfig.BiomeName = TEXT("Savana");
    SavanaConfig.PrimaryTerrainType = EWorld_TerrainType::Plains;
    SavanaConfig.BiomeCenter = FVector(0.0f, 0.0f, 0.0f);
    SavanaConfig.BiomeRadius = 30000.0f;
    SavanaConfig.HeightVariation = 300.0f;
    SavanaConfig.NoiseScale = 0.0008f;
    SavanaConfig.OctaveCount = 3;
    BiomeTerrainConfigs.Add(SavanaConfig);
    
    // Floresta (Hills with valleys)
    FWorld_BiomeTerrainConfig FlorestaConfig;
    FlorestaConfig.BiomeName = TEXT("Floresta");
    FlorestaConfig.PrimaryTerrainType = EWorld_TerrainType::Hills;
    FlorestaConfig.BiomeCenter = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaConfig.BiomeRadius = 25000.0f;
    FlorestaConfig.HeightVariation = 600.0f;
    FlorestaConfig.NoiseScale = 0.0006f;
    FlorestaConfig.OctaveCount = 4;
    BiomeTerrainConfigs.Add(FlorestaConfig);
    
    // Deserto (Plateaus and canyons)
    FWorld_BiomeTerrainConfig DesertoConfig;
    DesertoConfig.BiomeName = TEXT("Deserto");
    DesertoConfig.PrimaryTerrainType = EWorld_TerrainType::Plateaus;
    DesertoConfig.BiomeCenter = FVector(55000.0f, 0.0f, 0.0f);
    DesertoConfig.BiomeRadius = 28000.0f;
    DesertoConfig.HeightVariation = 400.0f;
    DesertoConfig.NoiseScale = 0.001f;
    DesertoConfig.OctaveCount = 2;
    BiomeTerrainConfigs.Add(DesertoConfig);
    
    // Montanha (Mountains and ridges)
    FWorld_BiomeTerrainConfig MontanhaConfig;
    MontanhaConfig.BiomeName = TEXT("Montanha");
    MontanhaConfig.PrimaryTerrainType = EWorld_TerrainType::Mountains;
    MontanhaConfig.BiomeCenter = FVector(40000.0f, 50000.0f, 0.0f);
    MontanhaConfig.BiomeRadius = 22000.0f;
    MontanhaConfig.HeightVariation = 1200.0f;
    MontanhaConfig.NoiseScale = 0.0004f;
    MontanhaConfig.OctaveCount = 6;
    BiomeTerrainConfigs.Add(MontanhaConfig);
    
    // Pantano (Valleys with water features)
    FWorld_BiomeTerrainConfig PantanoConfig;
    PantanoConfig.BiomeName = TEXT("Pantano");
    PantanoConfig.PrimaryTerrainType = EWorld_TerrainType::Valleys;
    PantanoConfig.BiomeCenter = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoConfig.BiomeRadius = 26000.0f;
    PantanoConfig.HeightVariation = 200.0f;
    PantanoConfig.NoiseScale = 0.0012f;
    PantanoConfig.OctaveCount = 2;
    BiomeTerrainConfigs.Add(PantanoConfig);
    
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Setup %d biome terrain configurations"), BiomeTerrainConfigs.Num());
}

void UWorld_AdvancedTerrainSystem::BlendBiomeTransitions()
{
    UE_LOG(LogTemp, Warning, TEXT("World_AdvancedTerrainSystem: Blending biome transitions"));
    
    // Apply smoothing between biome boundaries
    for (int32 i = 0; i < 3; i++)
    {
        SmoothTerrain(2, 0.02f);
    }
}

FWorld_BiomeTerrainConfig UWorld_AdvancedTerrainSystem::GetBiomeConfigAtLocation(FVector WorldLocation)
{
    float MinDistance = FLT_MAX;
    FWorld_BiomeTerrainConfig ClosestConfig;
    
    for (const FWorld_BiomeTerrainConfig& Config : BiomeTerrainConfigs)
    {
        float Distance = FVector::Dist2D(WorldLocation, Config.BiomeCenter);
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            ClosestConfig = Config;
        }
    }
    
    return ClosestConfig;
}

void UWorld_AdvancedTerrainSystem::OptimizeTerrainLOD()
{
    // LOD optimization logic here
    // This would be called each frame to adjust terrain detail based on camera distance
}

void UWorld_AdvancedTerrainSystem::UpdateTerrainStreaming()
{
    // Terrain streaming logic here
    // This would manage loading/unloading terrain chunks based on player position
}

void UWorld_AdvancedTerrainSystem::CullDistantTerrainDetails()
{
    // Distance culling logic here
    // This would hide distant terrain details to improve performance
}

float UWorld_AdvancedTerrainSystem::GeneratePerlinNoise(float X, float Y, int32 Octaves, float Scale, float Persistence)
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        Value += FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)) * Amplitude;
        Amplitude *= Persistence;
        Frequency *= 2.0f;
    }
    
    return Value;
}

float UWorld_AdvancedTerrainSystem::GenerateRidgedNoise(float X, float Y, int32 Octaves, float Scale)
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        float NoiseValue = FMath::Abs(FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)));
        NoiseValue = 1.0f - NoiseValue;
        Value += NoiseValue * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Value;
}

float UWorld_AdvancedTerrainSystem::GenerateBillowNoise(float X, float Y, int32 Octaves, float Scale)
{
    float Value = 0.0f;
    float Amplitude = 1.0f;
    float Frequency = Scale;
    
    for (int32 i = 0; i < Octaves; i++)
    {
        float NoiseValue = FMath::Abs(FMath::PerlinNoise2D(FVector2D(X * Frequency, Y * Frequency)));
        Value += NoiseValue * Amplitude;
        Amplitude *= 0.5f;
        Frequency *= 2.0f;
    }
    
    return Value;
}

void UWorld_AdvancedTerrainSystem::CalculateNormals()
{
    NormalData.SetNum(TerrainResolution * TerrainResolution);
    
    for (int32 X = 1; X < TerrainResolution - 1; X++)
    {
        for (int32 Y = 1; Y < TerrainResolution - 1; Y++)
        {
            int32 Index = Y * TerrainResolution + X;
            
            float HeightL = HeightData[(Y * TerrainResolution) + (X - 1)];
            float HeightR = HeightData[(Y * TerrainResolution) + (X + 1)];
            float HeightD = HeightData[((Y - 1) * TerrainResolution) + X];
            float HeightU = HeightData[((Y + 1) * TerrainResolution) + X];
            
            FVector Normal = FVector(HeightL - HeightR, HeightD - HeightU, 2.0f).GetSafeNormal();
            NormalData[Index] = Normal;
        }
    }
}

void UWorld_AdvancedTerrainSystem::CalculateSlopes()
{
    SlopeData.SetNum(TerrainResolution * TerrainResolution);
    
    for (int32 X = 0; X < TerrainResolution; X++)
    {
        for (int32 Y = 0; Y < TerrainResolution; Y++)
        {
            int32 Index = Y * TerrainResolution + X;
            
            if (NormalData.IsValidIndex(Index))
            {
                FVector Normal = NormalData[Index];
                float Slope = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(Normal, FVector::UpVector)));
                SlopeData[Index] = Slope;
            }
        }
    }
}

void UWorld_AdvancedTerrainSystem::ApplyErosion(int32 Iterations, float Strength)
{
    for (int32 Iter = 0; Iter < Iterations; Iter++)
    {
        TArray<float> NewHeightData = HeightData;
        
        for (int32 X = 1; X < TerrainResolution - 1; X++)
        {
            for (int32 Y = 1; Y < TerrainResolution - 1; Y++)
            {
                int32 Index = Y * TerrainResolution + X;
                float CurrentHeight = HeightData[Index];
                
                // Find steepest descent direction
                float MaxDiff = 0.0f;
                int32 BestX = X, BestY = Y;
                
                for (int32 OffsetX = -1; OffsetX <= 1; OffsetX++)
                {
                    for (int32 OffsetY = -1; OffsetY <= 1; OffsetY++)
                    {
                        if (OffsetX == 0 && OffsetY == 0) continue;
                        
                        int32 NeighborIndex = (Y + OffsetY) * TerrainResolution + (X + OffsetX);
                        float HeightDiff = CurrentHeight - HeightData[NeighborIndex];
                        
                        if (HeightDiff > MaxDiff)
                        {
                            MaxDiff = HeightDiff;
                            BestX = X + OffsetX;
                            BestY = Y + OffsetY;
                        }
                    }
                }
                
                // Apply erosion
                if (MaxDiff > 0.0f)
                {
                    float ErosionAmount = FMath::Min(MaxDiff, Strength);
                    NewHeightData[Index] -= ErosionAmount * 0.5f;
                    
                    int32 TargetIndex = BestY * TerrainResolution + BestX;
                    NewHeightData[TargetIndex] += ErosionAmount * 0.5f;
                }
            }
        }
        
        HeightData = NewHeightData;
    }
}

void UWorld_AdvancedTerrainSystem::SmoothTerrain(int32 Iterations, float Strength)
{
    for (int32 Iter = 0; Iter < Iterations; Iter++)
    {
        TArray<float> NewHeightData = HeightData;
        
        for (int32 X = 1; X < TerrainResolution - 1; X++)
        {
            for (int32 Y = 1; Y < TerrainResolution - 1; Y++)
            {
                int32 Index = Y * TerrainResolution + X;
                float CurrentHeight = HeightData[Index];
                
                // Calculate average of neighbors
                float NeighborSum = 0.0f;
                int32 NeighborCount = 0;
                
                for (int32 OffsetX = -1; OffsetX <= 1; OffsetX++)
                {
                    for (int32 OffsetY = -1; OffsetY <= 1; OffsetY++)
                    {
                        int32 NeighborIndex = (Y + OffsetY) * TerrainResolution + (X + OffsetX);
                        NeighborSum += HeightData[NeighborIndex];
                        NeighborCount++;
                    }
                }
                
                float NeighborAverage = NeighborSum / NeighborCount;
                NewHeightData[Index] = FMath::Lerp(CurrentHeight, NeighborAverage, Strength);
            }
        }
        
        HeightData = NewHeightData;
    }
}
#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Math/UnrealMathUtility.h"
#include "ProceduralWorldGenerator.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessZoneType : uint8
{
    Meditation     UMETA(DisplayName = "Meditation Zone"),
    Transformation UMETA(DisplayName = "Transformation Zone"),
    Reflection     UMETA(DisplayName = "Reflection Zone"),
    Integration    UMETA(DisplayName = "Integration Zone"),
    Transcendence  UMETA(DisplayName = "Transcendence Zone")
};

USTRUCT(BlueprintType)
struct FConsciousnessZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntensityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SymbolicElements;

    FConsciousnessZoneData()
    {
        ZoneType = EConsciousnessZoneType::Meditation;
        Location = FVector::ZeroVector;
        Radius = 1000.0f;
        IntensityLevel = 0.5f;
        AmbientColor = FLinearColor::White;
    }
};

USTRUCT(BlueprintType)
struct FBiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float NoiseScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HeightVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DominantColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SacredGeometryFrequency;

    FBiomeParameters()
    {
        NoiseScale = 0.01f;
        HeightVariation = 500.0f;
        VegetationDensity = 0.7f;
        DominantColor = FLinearColor::Green;
        SacredGeometryFrequency = 0.1f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UProceduralWorldGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UProceduralWorldGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Generation Functions
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(int32 Seed = -1);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateConsciousnessZones();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateSacredGeometry();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateSymbolicLandmarks();

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    FConsciousnessZoneData GetZoneDataAtLocation(FVector Location);

    // Consciousness-Based Generation
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void AdaptWorldToConsciousnessLevel(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void CreateTransformativePathways();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void SpawnMeditationCircles();

    // Biome Generation
    UFUNCTION(BlueprintCallable, Category = "Biomes")
    void GenerateBiome(FVector Center, float Radius, FBiomeParameters Parameters);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    float GetTerrainHeightAtLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Biomes")
    FLinearColor GetBiomeColorAtLocation(FVector Location);

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldSeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float WorldSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 NumConsciousnessZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float MinZoneDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    TArray<FConsciousnessZoneData> ConsciousnessZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TMap<EConsciousnessZoneType, FBiomeParameters> BiomeParametersMap;

    // Sacred Geometry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    TArray<FVector> SacredGeometryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    float GoldenRatioScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sacred Geometry")
    bool bUseFibonacciSpiral;

    // Generation State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bWorldGenerated;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentConsciousnessLevel;

private:
    // Internal Generation Methods
    void InitializeBiomeParameters();
    void GenerateTerrainNoise();
    void PlaceConsciousnessZones();
    void CreateSacredGeometryPattern(FVector Center, float Scale);
    void SpawnSymbolicElement(FVector Location, FString ElementType);
    
    // Noise Functions
    float PerlinNoise2D(float X, float Y, float Scale);
    float FractalNoise(float X, float Y, int32 Octaves, float Persistence);
    
    // Sacred Geometry Helpers
    FVector CalculateGoldenRatioPoint(FVector Center, float Angle, float Distance);
    TArray<FVector> GenerateFibonacciSpiral(FVector Center, int32 Points, float Scale);
    TArray<FVector> GenerateFlowerOfLife(FVector Center, float Radius);
    
    // Zone Management
    EConsciousnessZoneType DetermineZoneType(FVector Location);
    float CalculateZoneInfluence(FVector Location, FConsciousnessZoneData Zone);
    
    // Cached noise values for performance
    TMap<FIntPoint, float> NoiseCache;
    
    // Random stream for consistent generation
    FRandomStream RandomStream;
};
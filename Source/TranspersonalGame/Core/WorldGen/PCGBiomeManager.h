#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "PCGBiomeManager.generated.h"

// ============================================================
// PCGBiomeManager — Agent #5 Procedural World Generator
// Manages biome zones, vegetation distribution, and river system
// Uses HISM instancing for performance (1 draw call per mesh type)
// ============================================================

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Count       UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AverageElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;
};

USTRUCT(BlueprintType)
struct FWorld_VegetationConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    EWorld_BiomeType TargetBiome = EWorld_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 InstanceCount = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    FVector2D ScaleRange = FVector2D(0.8f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float PlacementRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAlignToSurface = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bRandomYaw = true;
};

USTRUCT(BlueprintType)
struct FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint = FVector(5000.0f, 0.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Depth = 150.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed = 200.0f;
};

UCLASS(ClassGroup = "WorldGen", meta = (DisplayName = "PCG Biome Manager"))
class TRANSPERSONALGAME_API UPCGBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPCGBiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ---- Biome Queries ----

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Biome")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Biome")
    float GetVegetationDensityAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Biome")
    float GetTemperatureAt(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Biome")
    float GetHumidityAt(const FVector& WorldLocation) const;

    // ---- World Generation ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen|Generation")
    void GenerateAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen|Generation")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen|Generation")
    void ClearGeneratedContent();

    UFUNCTION(BlueprintCallable, Category = "WorldGen|Generation")
    int32 GetTotalInstanceCount() const;

    // ---- River Queries ----

    UFUNCTION(BlueprintCallable, Category = "WorldGen|River")
    bool IsLocationNearRiver(const FVector& WorldLocation, float SearchRadius = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|River")
    FVector GetNearestRiverPoint(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "WorldGen|River")
    float GetRiverFlowSpeedAt(const FVector& WorldLocation) const;

    // ---- Debug ----

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "WorldGen|Debug")
    void LogBiomeReport() const;

    // ---- Configuration ----

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    int32 GenerationSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    float WorldScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    bool bAutoGenerateOnBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    TArray<FWorld_VegetationConfig> VegetationConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WorldGen|Config")
    TArray<FWorld_RiverSegment> RiverSegments;

private:
    void InitializeDefaultBiomes();
    void InitializeDefaultRivers();
    float CalculateBiomeBlend(const FVector& Location, const FWorld_BiomeZone& Zone) const;

    UPROPERTY()
    TArray<AActor*> GeneratedActors;

    int32 TotalInstanceCount = 0;
};

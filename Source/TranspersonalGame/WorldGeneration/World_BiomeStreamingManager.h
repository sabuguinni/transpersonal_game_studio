#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "World_BiomeStreamingManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Tundra      UMETA(DisplayName = "Tundra")
};

UENUM(BlueprintType)
enum class EWorld_StreamingState : uint8
{
    Unloaded    UMETA(DisplayName = "Unloaded"),
    Loading     UMETA(DisplayName = "Loading"),
    Loaded      UMETA(DisplayName = "Loaded"),
    Unloading   UMETA(DisplayName = "Unloading")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_StreamingState StreamingState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 DetailLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LastUpdateTime;

    FWorld_BiomeZone()
    {
        BiomeName = TEXT("DefaultBiome");
        BiomeType = EWorld_BiomeType::Forest;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        StreamingState = EWorld_StreamingState::Unloaded;
        DetailLevel = 2;
        LastUpdateTime = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_PerformanceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDetailLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float StreamingPriority;

    FWorld_PerformanceZone()
    {
        ZoneName = TEXT("DefaultZone");
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxDetailLevel = 3;
        StreamingPriority = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UWorld_BiomeStreamingManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UWorld_BiomeStreamingManager();

    // Subsystem overrides
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void RegisterBiomeZone(const FWorld_BiomeZone& BiomeZone);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UnregisterBiomeZone(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    void UpdateBiomeStreaming(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Streaming")
    TArray<FWorld_BiomeZone> GetActiveBiomes() const;

    // Performance integration
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterPerformanceZone(const FWorld_PerformanceZone& PerformanceZone);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceBasedStreaming(float CurrentFPS, float TargetFPS);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetRecommendedDetailLevel(const FVector& Location, float CurrentFPS) const;

    // Vegetation and terrain
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateVegetationForBiome(const FString& BiomeName, int32 VegetationCount);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateLandmarkAtLocation(const FVector& Location, const FString& LandmarkName);

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void CreateWaterSystem(const TArray<FVector>& WaterPoints);

    // Debug and utilities
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogBiomeStatus() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Streaming")
    TArray<FWorld_BiomeZone> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<FWorld_PerformanceZone> PerformanceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float MaxStreamingDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LastPerformanceUpdate;

private:
    void StreamInBiome(FWorld_BiomeZone& BiomeZone);
    void StreamOutBiome(FWorld_BiomeZone& BiomeZone);
    float CalculateStreamingPriority(const FWorld_BiomeZone& BiomeZone, const FVector& PlayerLocation) const;
    void AdjustDetailLevelForPerformance(FWorld_BiomeZone& BiomeZone, float CurrentFPS, float TargetFPS);
    AActor* SpawnVegetationActor(const FVector& Location, EWorld_BiomeType BiomeType);
    UStaticMesh* GetMeshForBiomeType(EWorld_BiomeType BiomeType) const;
};
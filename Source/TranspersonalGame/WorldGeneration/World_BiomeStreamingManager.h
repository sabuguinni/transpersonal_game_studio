#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "World_BiomeStreamingManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountain    UMETA(DisplayName = "Mountain")
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
    int32 MaxTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bPhysicsEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float StreamingDistance;

    FWorld_BiomeZone()
    {
        BiomeName = TEXT("DefaultBiome");
        BiomeType = EWorld_BiomeType::Plains;
        CenterLocation = FVector::ZeroVector;
        Radius = 1000.0f;
        MaxTerrainFeatures = 10;
        bPhysicsEnabled = true;
        StreamingDistance = 2000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_TerrainFeature
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FString FeatureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    AActor* FeatureActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EWorld_BiomeType OwnerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bIsPhysicsObject;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bCurrentlyStreamed;

    FWorld_TerrainFeature()
    {
        FeatureName = TEXT("UnknownFeature");
        FeatureActor = nullptr;
        OwnerBiome = EWorld_BiomeType::Plains;
        Location = FVector::ZeroVector;
        LODDistance = 1000.0f;
        bIsPhysicsObject = false;
        bCurrentlyStreamed = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeStreamingManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeStreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // === BIOME MANAGEMENT ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    TArray<FWorld_TerrainFeature> TerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    float StreamingUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System", meta = (AllowPrivateAccess = "true"))
    float MaxStreamingDistance;

    // === PHYSICS INTEGRATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    bool bEnablePhysicsOnTerrain;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics", meta = (AllowPrivateAccess = "true"))
    float PhysicsActivationDistance;

    // === PERFORMANCE MONITORING ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActiveTerrainFeatures;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float PerformanceCheckInterval;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 CurrentActiveFeatures;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float CurrentFrameTime;

private:
    float LastStreamingUpdate;
    float LastPerformanceCheck;
    APawn* PlayerPawn;

public:
    // === BIOME SYSTEM FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiomeZone(const FWorld_BiomeZone& NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeStreaming();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FWorld_TerrainFeature> GetFeaturesInBiome(EWorld_BiomeType BiomeType) const;

    // === TERRAIN GENERATION ===
    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void GenerateTerrainForBiome(const FWorld_BiomeZone& Biome);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    AActor* SpawnTerrainFeature(const FVector& Location, EWorld_BiomeType BiomeType, const FString& FeatureType);

    UFUNCTION(BlueprintCallable, Category = "Terrain Generation")
    void EnablePhysicsOnFeature(FWorld_TerrainFeature& Feature);

    // === STREAMING MANAGEMENT ===
    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void StreamInBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    void StreamOutBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Streaming")
    bool ShouldStreamBiome(const FWorld_BiomeZone& Biome, const FVector& PlayerLocation) const;

    // === PERFORMANCE OPTIMIZATION ===
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void MonitorFrameRate();

    // === DEBUG AND VALIDATION ===
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateBiomeSystem();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugDrawBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void PrintBiomeStatistics();
};

#include "World_BiomeStreamingManager.generated.h"
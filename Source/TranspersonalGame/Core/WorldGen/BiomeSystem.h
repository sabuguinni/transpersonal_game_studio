#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

// Biome type enum — unique prefix World_ to avoid conflicts
UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Rocky       UMETA(DisplayName = "Rocky Highlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    River       UMETA(DisplayName = "River"),
    None        UMETA(DisplayName = "None")
};

// Per-biome configuration data
USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor GroundColor = FLinearColor(0.2f, 0.2f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasWater = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");
};

// Runtime biome zone — tracks which actors belong to a biome
USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    FWorld_BiomeConfig Config;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    TArray<AActor*> OwnedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    bool bIsActive = true;
};

/**
 * ABiomeSystem — manages all biome zones in the world.
 * Spawned once in the level. Provides biome query API for other systems.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome System"))
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Query which biome a world position belongs to
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    // Get full config for a biome type
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool GetBiomeConfig(EWorld_BiomeType BiomeType, FWorld_BiomeConfig& OutConfig) const;

    // Get danger level at a world position (0-1)
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    // Get ambient temperature at a world position
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    // Check if a location has water nearby
    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsNearWater(const FVector& WorldLocation, float SearchRadius = 500.0f) const;

    // Initialize all biome zones from config array
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void InitializeBiomes();

    // Get all registered biome zones
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FWorld_BiomeZone> GetAllBiomeZones() const { return BiomeZones; }

    // Get count of active biomes
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetActiveBiomeCount() const;

protected:
    // Biome configuration array — edit in Blueprint or level
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    // Runtime biome zones (populated from BiomeConfigs on BeginPlay)
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    TArray<FWorld_BiomeZone> BiomeZones;

    // Whether biomes have been initialized
    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    bool bBiomesInitialized = false;

    // Tick interval for biome updates (seconds)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Performance")
    float BiomeUpdateInterval = 5.0f;

private:
    float TimeSinceLastUpdate = 0.0f;

    void SetupDefaultBiomeConfigs();
    void UpdateBiomeActorCounts();
};

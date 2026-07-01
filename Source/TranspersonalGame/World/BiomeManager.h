#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    None            UMETA(DisplayName = "None"),
    Jungle          UMETA(DisplayName = "Jungle"),
    Savanna         UMETA(DisplayName = "Savanna"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Coastal         UMETA(DisplayName = "Coastal"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.5f, 0.7f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");
};

USTRUCT(BlueprintType)
struct FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType FromBiome = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType ToBiome = EEng_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendAlpha = 0.0f;
};

/**
 * ABiomeManager — manages biome zones, transitions, and environmental properties
 * for the prehistoric survival world. Placed in the level as a singleton actor.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Biome Query ---

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns blended biome data for smooth transitions */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBlendedBiomeData(const FVector& WorldLocation) const;

    // --- Biome Registration ---

    /** Register a biome zone (called by PCG or level setup) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiomeZone(EEng_BiomeType BiomeType, const FVector& Center, float Radius);

    /** Clear all registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void ClearAllBiomeZones();

    // --- Biome Data Access ---

    /** Get static biome data for a given biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataForType(EEng_BiomeType BiomeType) const;

    /** Get number of registered biome zones */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    int32 GetBiomeZoneCount() const;

    // --- Environmental Application ---

    /** Apply biome atmosphere to the current level (fog, sky color, etc.) */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void ApplyBiomeAtmosphere(EEng_BiomeType BiomeType);

    /** Update atmosphere based on player location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateAtmosphereForPlayerLocation(const FVector& PlayerLocation);

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeTransitionBlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bEnableAtmosphereUpdates = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float AtmosphereUpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    EEng_BiomeType DefaultBiome = EEng_BiomeType::Jungle;

    // --- Debug ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Debug")
    bool bDrawBiomeZones = false;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome|Debug")
    void DebugDrawAllBiomeZones();

private:
    struct FBiomeZone
    {
        EEng_BiomeType BiomeType;
        FVector Center;
        float Radius;
    };

    TArray<FBiomeZone> RegisteredZones;
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataTable;
    float AtmosphereUpdateTimer = 0.0f;

    void InitializeBiomeDataTable();
    float CalculateBlendAlpha(const FVector& Location, const FBiomeZone& Zone) const;
};

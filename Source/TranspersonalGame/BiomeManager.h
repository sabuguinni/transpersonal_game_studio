#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * FBiomeData — Runtime data for a single biome region.
 * Defined at global scope (RULE 1 — USTRUCT at global scope only).
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;   // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.6f;       // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.3f;    // 0-1 — affects predator density

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWeatherType CurrentWeather = EWeatherType::Clear;

    FBiomeData()
    {
        NativeDinosaurs.Empty();
    }
};

/**
 * FBiomeTransition — Blend data between two adjacent biomes.
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FBiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeA = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeB = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendWidth = 1000.0f;  // Transition zone width in cm

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BlendAlpha = 0.0f;     // 0=BiomeA, 1=BiomeB

    FBiomeTransition() {}
};

/**
 * ABiomeManager — World actor that manages all biome regions.
 *
 * Responsibilities:
 *   - Register and store biome data for the world
 *   - Query which biome a world position belongs to
 *   - Compute biome blending at transition zones
 *   - Provide dinosaur spawn lists per biome
 *   - Drive weather probability per biome
 *
 * Architecture: Singleton-style world actor. One per level.
 * Other systems (DinosaurAI, FoliageManager, WeatherSystem) query this actor.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // --- Biome Registration ---

    /** Register a new biome region at runtime or from editor. */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiome(const FBiomeData& BiomeData);

    /** Remove all registered biomes (editor utility). */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void ClearAllBiomes();

    // --- Biome Queries ---

    /** Returns the dominant biome type at a given world position. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    /** Returns full biome data for the dominant biome at a location. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    FBiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    /** Returns blend alpha between two biomes at a transition zone (0=BiomeA, 1=BiomeB). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    float GetBiomeBlendAlpha(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const;

    /** Returns the danger level (0-1) at a world position. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    /** Returns temperature (Celsius) at a world position. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    /** Returns list of dinosaur species native to the biome at a location. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    TArray<EDinosaurSpecies> GetNativeDinosaursAtLocation(const FVector& WorldLocation) const;

    // --- Weather ---

    /** Returns current weather for the biome at a location. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome|Weather")
    EWeatherType GetWeatherAtLocation(const FVector& WorldLocation) const;

    /** Set weather for a specific biome type (affects all regions of that type). */
    UFUNCTION(BlueprintCallable, Category = "Biome|Weather")
    void SetWeatherForBiome(EBiomeType BiomeType, EWeatherType NewWeather);

    // --- Default Biome Setup ---

    /** Populate default Cretaceous biomes for the MinPlayableMap. CallInEditor for editor use. */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome")
    void SetupDefaultCretaceousBiomes();

    // --- Accessors ---

    /** Returns total number of registered biomes. */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    int32 GetBiomeCount() const { return RegisteredBiomes.Num(); }

    /** Returns all registered biome data (read-only). */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Biome")
    TArray<FBiomeData> GetAllBiomes() const { return RegisteredBiomes; }

protected:
    virtual void BeginPlay() override;

    /** All registered biome regions. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FBiomeData> RegisteredBiomes;

    /** Default biome used when no region matches a query location. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType DefaultBiomeType = EBiomeType::Forest;

    /** If true, SetupDefaultCretaceousBiomes() is called automatically on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bAutoSetupOnBeginPlay = true;

private:
    /** Internal helper — find index of closest biome to a location. Returns -1 if none. */
    int32 FindClosestBiomeIndex(const FVector& WorldLocation) const;

    /** Internal helper — find index of biome by type. Returns -1 if not found. */
    int32 FindBiomeIndexByType(EBiomeType BiomeType) const;
};

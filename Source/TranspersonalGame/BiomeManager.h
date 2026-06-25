#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle        UMETA(DisplayName = "Jungle"),
    Savanna       UMETA(DisplayName = "Savanna"),
    Swamp         UMETA(DisplayName = "Swamp"),
    Volcanic      UMETA(DisplayName = "Volcanic"),
    Coastal       UMETA(DisplayName = "Coastal"),
    Forest        UMETA(DisplayName = "Forest"),
    Unknown       UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> DinosaurSpecies;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Returns the biome type at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

    /** Returns full biome data at a given world location */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataAtLocation(FVector WorldLocation) const;

    /** Returns the name of a biome type as string */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    FString GetBiomeName(EEng_BiomeType BiomeType) const;

    /** Returns danger level at location (0.0 = safe, 1.0 = lethal) */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDangerLevelAtLocation(FVector WorldLocation) const;

    /** Returns temperature at location in Celsius */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    /** Returns list of dinosaur species that spawn in this biome */
    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FString> GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const;

    /** World size for biome sampling (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSize = 200000.0f;

    /** Number of biome zones */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeZoneCount = 6;

    /** Noise scale for biome blending */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float NoiseScale = 0.00005f;

    /** Current active biome (updated each tick based on player position) */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Biome|Runtime")
    EEng_BiomeType CurrentPlayerBiome = EEng_BiomeType::Unknown;

    /** All biome definitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TArray<FEng_BiomeData> BiomeDefinitions;

protected:
    void InitializeBiomeDefinitions();
    float SampleNoise(float X, float Y, float Frequency) const;
    EEng_BiomeType ClassifyBiomeFromNoise(float NoiseValue, float AltitudeValue) const;
    FEng_BiomeData* FindBiomeData(EEng_BiomeType Type);
    const FEng_BiomeData* FindBiomeDataConst(EEng_BiomeType Type) const;

private:
    UPROPERTY()
    TObjectPtr<AActor> PlayerRef;

    float TimeSinceLastBiomeUpdate = 0.0f;
    static constexpr float BiomeUpdateInterval = 2.0f;
};

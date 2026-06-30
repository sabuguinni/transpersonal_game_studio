// BiomeSystem.h — Agent #05 Procedural World Generator
// Biome classification and environmental parameter system for prehistoric world
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    JungleForest    UMETA(DisplayName = "Jungle Forest"),
    VolcanicPlains  UMETA(DisplayName = "Volcanic Plains"),
    RiverWetlands   UMETA(DisplayName = "River Wetlands"),
    RockyHighlands  UMETA(DisplayName = "Rocky Highlands"),
    OpenSavanna     UMETA(DisplayName = "Open Savanna"),
    Unknown         UMETA(DisplayName = "Unknown")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;  // Celsius

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;      // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.3f;   // 0-1 predator activity

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString AmbientSoundCue = TEXT("");
};

UCLASS(ClassGroup = (WorldGen), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeDetectorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeDetectorComponent();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetCurrentBiome() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetCurrentBiomeParameters() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeDetection();

    UPROPERTY(BlueprintReadOnly, Category = "Biome")
    EWorld_BiomeType CurrentBiome = EWorld_BiomeType::Unknown;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Biome")
    float DetectionRadius = 500.0f;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastUpdate = 0.0f;
    static constexpr float UpdateInterval = 2.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystemManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystemManager();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType ClassifyLocationBiome(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeParameters GetBiomeParameters(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(FVector WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationNearWater(FVector WorldLocation, float Radius = 500.0f) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FWorld_BiomeParameters> BiomeTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldScale = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float NoiseFrequency = 0.0002f;

protected:
    virtual void BeginPlay() override;

private:
    void InitializeBiomeTable();
    float SampleNoise(float X, float Y, float Frequency) const;
};

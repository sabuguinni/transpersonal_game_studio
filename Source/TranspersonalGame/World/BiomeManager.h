#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Jungle         UMETA(DisplayName = "Jungle"),
    Savanna        UMETA(DisplayName = "Savanna"),
    Swamp          UMETA(DisplayName = "Swamp"),
    Volcanic       UMETA(DisplayName = "Volcanic"),
    Coastal        UMETA(DisplayName = "Coastal"),
    Forest         UMETA(DisplayName = "Forest"),
    Grassland      UMETA(DisplayName = "Grassland"),
    Desert         UMETA(DisplayName = "Desert"),
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.7f, 0.8f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeQueryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeQueryComponent();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Biome Query ---
    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    FEng_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome|Query")
    bool IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    // --- Biome Registration ---
    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void RegisterBiome(EEng_BiomeType BiomeType, const FEng_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome|Setup")
    void SetBiomeNoiseScale(float Scale);

    // --- World Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float WorldSizeKm = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float BiomeNoiseScale = 0.0001f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    int32 BiomeSeed = 42;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bDebugDrawBiomes = false;

    // --- Biome Table ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Data")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeTable;

    // --- Static accessor ---
    UFUNCTION(BlueprintCallable, Category = "Biome", meta = (WorldContext = "WorldContextObject"))
    static ABiomeManager* GetInstance(UObject* WorldContextObject);

private:
    EEng_BiomeType SampleBiomeFromNoise(float X, float Y) const;
    void InitializeDefaultBiomes();

    static ABiomeManager* CachedInstance;
};

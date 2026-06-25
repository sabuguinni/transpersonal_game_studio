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
    CoastalPlain   UMETA(DisplayName = "CoastalPlain"),
    RiverDelta     UMETA(DisplayName = "RiverDelta"),
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FName> DinosaurSpecies;
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FName> GetDinosaurSpeciesForBiome(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(CallInEditor, Category = "Biome|Debug")
    void DebugPrintAllBiomes();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    TArray<FEng_BiomeData> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    bool bEnableBiomeTransitions = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Config")
    float TransitionBlendRadius = 500.0f;

private:
    void InitializeDefaultBiomes();
};

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Eng_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BoundsMin = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D BoundsMax = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class AActor>> AllowedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class APawn>> AllowedDinosaurs;

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Savana;
        Center = FVector::ZeroVector;
        BoundsMin = FVector2D(-10000, -10000);
        BoundsMax = FVector2D(10000, 10000);
        Temperature = 25.0f;
        Humidity = 0.5f;
        WindStrength = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& WorldLocation, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<EBiomeType> GetAllBiomeTypes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome System")
    void ValidateBiomeSetup();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    float BiomeTransitionDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    bool bDebugBiomes = false;

private:
    void SetupDefaultBiomes();
    float CalculateDistanceToBiome(const FVector& WorldLocation, const FEng_BiomeData& BiomeData) const;
};

#include "Eng_BiomeManager.generated.h"
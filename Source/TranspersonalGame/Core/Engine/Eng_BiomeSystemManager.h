#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Eng_BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedVegetation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WaterProximity = 0.0f;

    FEng_BiomeData()
    {
        AllowedVegetation.Add(TEXT("Grass"));
        AllowedVegetation.Add(TEXT("Fern"));
        AllowedDinosaurs.Add(TEXT("Triceratops"));
        AllowedDinosaurs.Add(TEXT("Parasaurolophus"));
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Grassland;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeSystemManager();

    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core biome system methods
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanSpawnVegetationAtLocation(const FVector& WorldLocation, const FString& VegetationType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool CanSpawnDinosaurAtLocation(const FVector& WorldLocation, const FString& DinosaurType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetAllowedVegetationAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeTransition(const FEng_BiomeTransition& Transition);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float CalculateBiomeBlendFactor(const FVector& WorldLocation, EBiomeType BiomeA, EBiomeType BiomeB) const;

    // Debug and testing methods
    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void DebugPrintBiomeAtLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor = true)
    void GenerateBiomeMap(int32 MapSize = 100);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SetBiomeOverride(const FVector& WorldLocation, EBiomeType NewBiome, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ClearBiomeOverrides();

protected:
    // Core biome data storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data", meta = (AllowPrivateAccess = "true"))
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data", meta = (AllowPrivateAccess = "true"))
    TArray<FEng_BiomeTransition> BiomeTransitions;

    // Biome generation parameters
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float NoiseScale = 0.001f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float TemperatureNoiseScale = 0.0005f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float HumidityNoiseScale = 0.0008f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Generation", meta = (AllowPrivateAccess = "true"))
    float ElevationInfluence = 0.1f;

    // Runtime biome overrides for testing
    UPROPERTY()
    TMap<FVector, EBiomeType> BiomeOverrides;

    UPROPERTY()
    TMap<FVector, float> OverrideRadii;

private:
    // Internal helper methods
    float GeneratePerlinNoise(float X, float Y, float Scale) const;
    EBiomeType CalculateBiomeFromFactors(float Temperature, float Humidity, float Elevation) const;
    void InitializeBiomeData();
    bool IsLocationInBiomeOverride(const FVector& WorldLocation, EBiomeType& OutBiome) const;
};
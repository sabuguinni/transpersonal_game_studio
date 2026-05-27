#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "EngArch_BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Rainfall = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurPopulation = 20;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bIsActive = true;

    FEng_BiomeData()
    {
        AllowedDinosaurTypes.Add("TRex");
        AllowedDinosaurTypes.Add("Velociraptor");
        VegetationTypes.Add("Grass");
        VegetationTypes.Add("SmallTrees");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType FromBiome = EBiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EBiomeType ToBiome = EBiomeType::Floresta;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
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
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeData GetBiomeData(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FEng_BiomeData> GetAllBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetDistanceToBiomeCenter(const FVector& Location, EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeTransition GetBiomeTransition(const FVector& Location) const;

    // Environmental Factors
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    bool CanDinosaurSpawnAtLocation(const FString& DinosaurType, const FVector& Location) const;

    // Biome Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void RegisterBiome(const FEng_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void UpdateBiomeData(EBiomeType BiomeType, const FEng_BiomeData& NewData);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBiomeActive(EBiomeType BiomeType, bool bActive);

    // Debug and Validation
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DrawBiomeBounds(bool bDraw = true);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biomes")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    bool bDebugDrawEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float DefaultBiomeRadius = 15000.0f;

private:
    void SetupDefaultBiomes();
    void SetupBiomeTransitions();
    float CalculateBlendFactor(const FVector& Location, const FEng_BiomeData& BiomeA, const FEng_BiomeData& BiomeB) const;
    EBiomeType FindClosestBiome(const FVector& Location) const;
};
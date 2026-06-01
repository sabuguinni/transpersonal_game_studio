#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "Eng_BiomeArchitecture.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureRange = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxActors = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 MaxDinosaurs = 30;

    FEng_BiomeConfiguration()
    {
        BiomeType = EBiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        TemperatureRange = 25.0f;
        HumidityLevel = 0.5f;
        MaxActors = 1000;
        MaxDinosaurs = 30;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeSpawnRules
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    TArray<TSubclassOf<AActor>> AllowedActorTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    float VegetationDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    float RockDensity = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    float WaterPresence = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    bool bAllowCarnivores = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn Rules")
    bool bAllowHerbivores = true;

    FEng_BiomeSpawnRules()
    {
        VegetationDensity = 0.3f;
        RockDensity = 0.1f;
        WaterPresence = 0.0f;
        bAllowCarnivores = true;
        bAllowHerbivores = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeArchitecture : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeArchitecture();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core biome management
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void RegisterBiome(const FEng_BiomeConfiguration& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeConfiguration GetBiomeConfiguration(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool IsLocationValidForBiome(const FVector& Location, EBiomeType BiomeType) const;

    // Actor distribution and limits
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    bool CanSpawnActorInBiome(EBiomeType BiomeType, TSubclassOf<AActor> ActorClass) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    int32 GetActorCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    void EnforceActorLimits();

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType) const;

    // Environmental parameters
    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Architecture")
    FEng_BiomeSpawnRules GetSpawnRulesForBiome(EBiomeType BiomeType) const;

    // Debug and validation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateBiomeDistribution();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogBiomeStatistics();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    TMap<EBiomeType, FEng_BiomeSpawnRules> BiomeSpawnRules;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    int32 GlobalActorLimit = 8000;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome System")
    int32 GlobalDinosaurLimit = 150;

private:
    void SetupDefaultBiomes();
    void InitializeSavannaBiome();
    void InitializeSwampBiome();
    void InitializeForestBiome();
    void InitializeDesertBiome();
    void InitializeMountainBiome();
    
    float CalculateDistanceToNearestBiomeCenter(const FVector& Location) const;
    EBiomeType DetermineBiomeFromLocation(const FVector& Location) const;
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Math/Vector.h"
#include "Containers/Array.h"
#include "Containers/Map.h"
#include "../SharedTypes.h"
#include "Eng_BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
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
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<EDinosaurSpecies> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    FEng_BiomeDefinition()
    {
        BiomeType = EBiomeType::Savanna;
        CenterLocation = FVector::ZeroVector;
        Radius = 15000.0f;
        TemperatureRange = 25.0f;
        HumidityLevel = 0.5f;
        DangerLevel = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDistributionRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    EBiomeType TargetBiome = EBiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    float SpawnPercentage = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    int32 MaxActorsPerBiome = 1000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Distribution")
    bool bRequiresBiomeCompatibility = true;

    FEng_BiomeDistributionRule()
    {
        TargetBiome = EBiomeType::Savanna;
        SpawnPercentage = 0.2f;
        MaxActorsPerBiome = 1000;
        bRequiresBiomeCompatibility = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_BiomeSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UEng_BiomeSystemManager();

    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Biome Management
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeDefinition GetBiomeDefinition(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FVector> GetDistributedSpawnLocations(int32 TotalActors, float SpawnRadius = 100.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(EBiomeType BiomeType, float MinDistanceFromCenter = 0.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& Location, EBiomeType BiomeType) const;

    // Distribution System
    UFUNCTION(BlueprintCallable, Category = "Distribution")
    void SetDistributionRules(const TArray<FEng_BiomeDistributionRule>& Rules);

    UFUNCTION(BlueprintCallable, Category = "Distribution")
    TArray<FEng_BiomeDistributionRule> GetCurrentDistributionRules() const;

    // Actor Management
    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    void RegisterActorInBiome(AActor* Actor, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    int32 GetActorCountInBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Actor Management")
    TArray<AActor*> GetActorsInBiome(EBiomeType BiomeType) const;

    // Environmental Queries
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetDangerLevelAtLocation(const FVector& Location) const;

    // Validation and Debugging
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void ValidateBiomeConfiguration();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void LogBiomeStatistics();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TMap<EBiomeType, FEng_BiomeDefinition> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    TArray<FEng_BiomeDistributionRule> DistributionRules;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<EBiomeType, TArray<TWeakObjectPtr<AActor>>> BiomeActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoInitializeOnStartup = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float BiomeTransitionZone = 2000.0f;

private:
    void SetupDefaultBiomes();
    void SetupDefaultDistributionRules();
    float CalculateDistanceToBiomeCenter(const FVector& Location, EBiomeType BiomeType) const;
};
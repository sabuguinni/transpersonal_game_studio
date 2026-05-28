#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "EngArch_BiomeSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldBounds = FVector(10000.0f, 10000.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float WindStrength = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> AllowedDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float ResourceDensity = 1.0f;

    FEng_BiomeConfiguration()
    {
        AllowedDinosaurSpecies = {"TRex", "Velociraptor", "Triceratops"};
        VegetationTypes = {"Grass", "Trees", "Bushes"};
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType FromBiome = EEng_BiomeType::Savanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    EEng_BiomeType ToBiome = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlendFactor = 0.5f;
};

/**
 * Engine Architect's Biome System Manager
 * Defines the technical architecture for biome management across the prehistoric world
 * Manages biome configurations, transitions, and environmental parameters
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEngArch_BiomeSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEngArch_BiomeSystemManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeConfiguration GetBiomeConfiguration(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    TArray<FString> GetAllowedDinosaursAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void RegisterBiomeZone(EEng_BiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    bool IsLocationInBiome(const FVector& WorldLocation, EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeTransition CalculateBiomeTransition(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void ValidateBiomeConfiguration();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TMap<EEng_BiomeType, FEng_BiomeConfiguration> BiomeConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FEng_BiomeTransition> BiomeTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    float DefaultBiomeRadius = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    bool bDebugMode = false;

private:
    struct FEng_BiomeZone
    {
        EEng_BiomeType BiomeType;
        FVector Center;
        float Radius;
    };

    TArray<FEng_BiomeZone> RegisteredBiomeZones;

    void SetupDefaultBiomeConfigurations();
    void CreateBiomeZones();
    float CalculateDistanceToNearestBiome(const FVector& Location, EEng_BiomeType BiomeType) const;
};
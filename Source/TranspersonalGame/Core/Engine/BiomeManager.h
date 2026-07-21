#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "BiomeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FString BiomeName = TEXT("Unknown");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Fertility = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSubclassOf<class APawn>> NativeDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FString> VegetationTypes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor BiomeColor = FLinearColor::Green;

    FEng_BiomeData()
    {
        BiomeType = EBiomeType::Forest;
        BiomeName = TEXT("Forest");
        Temperature = 22.0f;
        Humidity = 70.0f;
        Fertility = 80.0f;
        BiomeColor = FLinearColor::Green;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FEng_BiomeData BiomeData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    bool bIsActive = true;

    FEng_BiomeZone()
    {
        Center = FVector::ZeroVector;
        Radius = 1000.0f;
        bIsActive = true;
    }
};

/**
 * BiomeManager - World Subsystem for managing biome zones and environmental systems
 * Handles biome transitions, environmental effects, and ecosystem simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UBiomeManager();

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
    void AddBiomeZone(const FEng_BiomeZone& NewZone);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RemoveBiomeZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FEng_BiomeZone> GetAllBiomeZones() const { return BiomeZones; }

    // Environmental Effects
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetFertilityAtLocation(const FVector& Location) const;

    // Biome Transitions
    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdateBiomeTransitions(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiomeTransition(const FVector& Location) const;

    // Debug and Visualization
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugDrawBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void CreateTestBiomes();

protected:
    // Biome Data Storage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data")
    TArray<FEng_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Biome Data")
    TMap<EBiomeType, FEng_BiomeData> BiomeDataMap;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float BiomeTransitionRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    float UpdateFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Configuration")
    bool bEnableDebugDraw = false;

private:
    // Internal Methods
    void SetupDefaultBiomes();
    FEng_BiomeZone* FindBiomeZoneAtLocation(const FVector& Location);
    float CalculateDistanceWeight(const FVector& Location, const FEng_BiomeZone& Zone) const;
    
    // Timer for periodic updates
    FTimerHandle UpdateTimer;
    float LastUpdateTime = 0.0f;
};
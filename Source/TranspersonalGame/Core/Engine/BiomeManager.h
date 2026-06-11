#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "../../SharedTypes.h"
#include "BiomeManager.generated.h"

/**
 * Biome Manager - Core engine system for managing biome data and transitions
 * Handles biome definitions, weather systems, and environmental parameters
 * Part of the Engine Architecture layer - provides data to World Generator
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UBiomeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UBiomeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === BIOME SYSTEM CORE ===
    
    /** Initialize biome system with default biome data */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomeSystem();
    
    /** Get current biome at world location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;
    
    /** Get biome data for specified biome type */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FEng_BiomeData GetBiomeData(EEng_BiomeType BiomeType) const;
    
    /** Update biome transition at location */
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void UpdateBiomeTransition(const FVector& Location, float TransitionRadius);

    // === WEATHER SYSTEM ===
    
    /** Get current weather state */
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    FEng_WeatherState GetCurrentWeather() const;
    
    /** Set weather state for biome */
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void SetWeatherState(EEng_BiomeType BiomeType, const FEng_WeatherState& NewWeather);
    
    /** Update weather progression */
    UFUNCTION(BlueprintCallable, Category = "Weather System")
    void UpdateWeatherProgression(float DeltaTime);

    // === ENVIRONMENTAL PARAMETERS ===
    
    /** Get temperature at location */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetTemperatureAtLocation(const FVector& Location) const;
    
    /** Get humidity at location */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetHumidityAtLocation(const FVector& Location) const;
    
    /** Get danger level for biome */
    UFUNCTION(BlueprintCallable, Category = "Environment")
    float GetBiomeDangerLevel(EEng_BiomeType BiomeType) const;

    // === BIOME VALIDATION ===
    
    /** Validate biome configuration */
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Validation")
    bool ValidateBiomeConfiguration();
    
    /** Get biome system status */
    UFUNCTION(BlueprintCallable, Category = "Debug")
    FString GetBiomeSystemStatus() const;

protected:
    // === BIOME DATA ===
    
    /** Map of biome type to biome data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Configuration")
    TMap<EEng_BiomeType, FEng_BiomeData> BiomeDataMap;
    
    /** Current active biomes in the world */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    TArray<EEng_BiomeType> ActiveBiomes;
    
    /** Current weather states per biome */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weather State")
    TMap<EEng_BiomeType, FEng_WeatherState> BiomeWeatherStates;

    // === SYSTEM PARAMETERS ===
    
    /** Biome transition smoothing factor */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Parameters", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float BiomeTransitionSmoothness = 2.0f;
    
    /** Weather update frequency in seconds */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Parameters", meta = (ClampMin = "1.0", ClampMax = "300.0"))
    float WeatherUpdateFrequency = 30.0f;
    
    /** Maximum biome influence radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Parameters", meta = (ClampMin = "1000.0", ClampMax = "50000.0"))
    float MaxBiomeInfluenceRadius = 10000.0f;

    // === RUNTIME STATE ===
    
    /** Time since last weather update */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    float TimeSinceWeatherUpdate = 0.0f;
    
    /** Is biome system initialized */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    bool bIsBiomeSystemInitialized = false;
    
    /** Number of active biome transitions */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Runtime State")
    int32 ActiveTransitionCount = 0;

private:
    // === INTERNAL METHODS ===
    
    /** Initialize default biome data */
    void InitializeDefaultBiomeData();
    
    /** Calculate biome influence at location */
    float CalculateBiomeInfluence(const FVector& Location, EEng_BiomeType BiomeType) const;
    
    /** Update biome weather internal */
    void UpdateBiomeWeatherInternal(EEng_BiomeType BiomeType, float DeltaTime);
    
    /** Validate single biome data */
    bool ValidateSingleBiomeData(const FEng_BiomeData& BiomeData) const;
};
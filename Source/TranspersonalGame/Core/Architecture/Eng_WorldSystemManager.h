#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Eng_WorldSystemManager.generated.h"

class UEng_BiomeSystemManager;
class UEng_WeatherSystemManager;
class UEng_TimeOfDayManager;

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_WorldSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // World Generation Control
    UFUNCTION(BlueprintCallable, Category = "World System")
    void InitializeWorldSystems();

    UFUNCTION(BlueprintCallable, Category = "World System")
    void RegenerateWorld(float WorldSize = 8000.0f);

    UFUNCTION(BlueprintCallable, Category = "World System")
    void SetWorldParameters(const FEng_WorldGenerationParams& Params);

    // System Access
    UFUNCTION(BlueprintPure, Category = "World System")
    UEng_BiomeSystemManager* GetBiomeManager() const { return BiomeManager; }

    UFUNCTION(BlueprintPure, Category = "World System")
    UEng_WeatherSystemManager* GetWeatherManager() const { return WeatherManager; }

    UFUNCTION(BlueprintPure, Category = "World System")
    UEng_TimeOfDayManager* GetTimeManager() const { return TimeManager; }

    // World State
    UFUNCTION(BlueprintPure, Category = "World System")
    bool IsWorldGenerated() const { return bWorldGenerated; }

    UFUNCTION(BlueprintPure, Category = "World System")
    float GetWorldSize() const { return CurrentWorldSize; }

    UFUNCTION(BlueprintPure, Category = "World System")
    FEng_WorldGenerationParams GetWorldParams() const { return WorldParams; }

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UEng_BiomeSystemManager* BiomeManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UEng_WeatherSystemManager* WeatherManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    UEng_TimeOfDayManager* TimeManager;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    bool bWorldGenerated;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    float CurrentWorldSize;

    UPROPERTY(BlueprintReadOnly, Category = "World State")
    FEng_WorldGenerationParams WorldParams;

private:
    void CreateSubsystems();
    void ValidateWorldIntegrity();
};
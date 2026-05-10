#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AudioTypes.h"
#include "Audio_DayNightCycleManager.generated.h"

class ADirectionalLight;
class AAudio_AdaptiveMusicManager;
class AAudio_BiomeAudioManager;

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DayNightCycleManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DayNightCycleManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Day/Night cycle parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CycleDurationMinutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    float CurrentTimeOfDay;  // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day/Night Cycle")
    bool bCycleActive;

    // Lighting parameters for Cretaceous period
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float DayLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    float NightLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor DayLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting")
    FLinearColor NightLightColor;

    // Sun movement parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Movement")
    float SunPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sun Movement")
    float SunYawRange;

    // Audio transition times
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DawnTime;  // Time when dawn audio starts

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float DuskTime;  // Time when dusk audio starts

private:
    // References to world objects
    UPROPERTY()
    ADirectionalLight* DirectionalLight;

    UPROPERTY()
    AAudio_AdaptiveMusicManager* MusicManager;

    UPROPERTY()
    AAudio_BiomeAudioManager* BiomeAudioManager;

    // Current time period for audio
    EAudio_TimeOfDay CurrentPeriod;

    // Internal methods
    void FindDirectionalLight();
    void FindAudioManagers();
    void UpdateLighting();
    void UpdateAudioForTimeOfDay();

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetTimeOfDay(float NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Day/Night Cycle")
    void SetCycleActive(bool bActive);

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintPure, Category = "Day/Night Cycle")
    EAudio_TimeOfDay GetCurrentPeriod() const;
};
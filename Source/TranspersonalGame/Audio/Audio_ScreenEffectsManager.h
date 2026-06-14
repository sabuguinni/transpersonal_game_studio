#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Materials/MaterialParameterCollection.h"
#include "SharedTypes.h"
#include "Audio_ScreenEffectsManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_ScreenEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenEffectsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Screen shake system
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerScreenShake(float Intensity, float Duration, float Frequency = 10.0f);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerDinosaurProximityShake(float Distance, float DinosaurSize = 1.0f);

    // Damage flash effect
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerDamageFlash(float Intensity = 1.0f, FLinearColor FlashColor = FLinearColor::Red);

    // Day/night cycle effects
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void UpdateTimeOfDayEffects(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void SetNightVisionMode(bool bEnabled);

    // Environmental screen effects
    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerWeatherEffect(EWeatherType WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void UpdateFearEffect(float FearLevel);

protected:
    // Screen shake properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float CurrentShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float MaxShakeDistance;

    // Damage flash properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float DamageFlashDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float CurrentFlashIntensity;

    // Post process volume reference
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    APostProcessVolume* PostProcessVolume;

    // Material parameter collection for global effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialParameterCollection* GlobalEffectsCollection;

    // Time of day effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Effects")
    float CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Effects")
    bool bNightVisionActive;

    // Environmental effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentWeatherIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EWeatherType CurrentWeatherType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CurrentFearLevel;

private:
    // Internal shake calculation
    void UpdateScreenShake(float DeltaTime);
    void UpdateDamageFlash(float DeltaTime);
    void ApplyPostProcessEffects();
    
    // Shake timing
    float ShakeTimer;
    float ShakeDuration;
    float ShakeFrequency;
    
    // Flash timing
    float FlashTimer;
    FLinearColor FlashColor;
};
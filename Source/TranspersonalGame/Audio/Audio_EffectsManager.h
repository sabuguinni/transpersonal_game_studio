#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Camera/CameraShakeBase.h"
#include "SharedTypes.h"
#include "Audio_EffectsManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenEffectData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float Intensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    FLinearColor Color = FLinearColor::Red;

    FAudio_ScreenEffectData()
    {
        Duration = 1.0f;
        Intensity = 0.5f;
        Color = FLinearColor::Red;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_FootstepData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    USoundCue* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    float ShakeRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    float ShakeIntensity = 1.0f;

    FAudio_FootstepData()
    {
        FootstepSound = nullptr;
        VolumeMultiplier = 1.0f;
        ShakeRadius = 500.0f;
        ShakeIntensity = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_EffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    FAudio_ScreenEffectData DamageFlashData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footsteps")
    TMap<ESurvivalCreatureType, FAudio_FootstepData> FootstepDataMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Shake")
    TSubclassOf<UCameraShakeBase> PlayerFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float DayNightCycleDuration = 1200.0f; // 20 minutes

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Day Night Cycle")
    float CurrentTimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Screen Effects")
    void TriggerDamageFlash(float Duration = 0.5f, float Intensity = 0.8f, FLinearColor Color = FLinearColor::Red);

    UFUNCTION(BlueprintCallable, Category = "Footsteps")
    void PlayFootstepEffect(ESurvivalCreatureType CreatureType, FVector Location, float VolumeOverride = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Camera Shake")
    void TriggerCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass, float Intensity = 1.0f, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    void UpdateDayNightCycle(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    float GetTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintCallable, Category = "Day Night Cycle")
    bool IsNightTime() const { return CurrentTimeOfDay < 0.25f || CurrentTimeOfDay > 0.75f; }

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySoundAtLocation(USoundCue* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

private:
    void InitializeFootstepData();
    void UpdateDirectionalLightRotation();
    
    UPROPERTY()
    class ADirectionalLight* SunLight;

    float ScreenFlashTimer = 0.0f;
    FAudio_ScreenEffectData ActiveFlashData;
    bool bIsFlashing = false;
};
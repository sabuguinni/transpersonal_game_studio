#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "TimerManager.h"
#include "Audio_DamageFlashController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DamageFlashSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    FLinearColor FlashColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    float FadeOutTime = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Flash")
    bool bUseHeartbeatEffect = true;

    FAudio_DamageFlashSettings()
    {
        FlashColor = FLinearColor(1.0f, 0.0f, 0.0f, 0.8f);
        FlashIntensity = 0.8f;
        FlashDuration = 0.3f;
        FadeOutTime = 0.2f;
        bUseHeartbeatEffect = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_DamageFlashController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DamageFlashController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Damage flash functions
    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void TriggerDamageFlash(float DamageAmount, FLinearColor CustomColor = FLinearColor::Red);

    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void TriggerCriticalDamageFlash(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void TriggerLowHealthFlash();

    UFUNCTION(BlueprintCallable, Category = "Audio Damage Flash")
    void StopAllFlashEffects();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Normal Damage")
    FAudio_DamageFlashSettings NormalDamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Critical Damage")
    FAudio_DamageFlashSettings CriticalDamageSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Low Health")
    FAudio_DamageFlashSettings LowHealthSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    float PostProcessBlendWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Thresholds")
    float CriticalDamageThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Health Thresholds")
    float LowHealthThreshold = 25.0f;

private:
    // Internal functions
    void ExecuteDamageFlash(const FAudio_DamageFlashSettings& Settings);
    void UpdateFlashIntensity(float DeltaTime);
    void CreatePostProcessVolume();
    void UpdatePostProcessSettings();
    void FadeOutFlash();

    // Post-process components
    UPROPERTY()
    APostProcessVolume* DamageFlashVolume;

    UPROPERTY()
    UMaterialInstanceDynamic* FlashMaterial;

    // State tracking
    bool bIsFlashing = false;
    float CurrentFlashIntensity = 0.0f;
    float FlashTimer = 0.0f;
    float FlashDuration = 0.0f;
    float FadeOutDuration = 0.0f;
    FLinearColor CurrentFlashColor = FLinearColor::Red;

    // Timers
    FTimerHandle FlashTimerHandle;
    FTimerHandle FadeOutTimerHandle;

    // Heartbeat effect
    bool bHeartbeatActive = false;
    float HeartbeatTimer = 0.0f;
    float HeartbeatFrequency = 1.2f; // beats per second
};
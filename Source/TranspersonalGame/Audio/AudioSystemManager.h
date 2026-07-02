
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DangerZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float HeartbeatBPM = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ScreenShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Aware;
};

/**
 * AAudio_SystemManager
 * Central audio orchestrator for the prehistoric survival game.
 * Manages adaptive ambient layers, danger proximity audio,
 * campfire sound zones, and screen shake triggers.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // --- Ambient Layer Control ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void TriggerDinosaurFootstepShake(float Intensity, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void TriggerCampfireAmbience(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void TriggerDamageAudioFeedback(float DamageAmount);

    // --- Proximity Detection ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void UpdateDangerProximity(float ClosestPredatorDistance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetNightMode(bool bIsNight);

    // --- Narration ---

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void PlayNarrationLine(const FString& NarrationID);

    // --- Configuration ---

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FAudio_DangerZoneConfig DangerConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> DayAmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> NightAmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DangerProximityUpdateInterval = 0.5f;

private:
    UPROPERTY()
    EAudio_DangerLevel CurrentDangerLevel;

    UPROPERTY()
    bool bIsNightMode;

    UPROPERTY()
    float TimeSinceLastProximityUpdate;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

    void UpdateAmbientLayersForDanger();
    void ApplyScreenShake(float Intensity);
    float ComputeVolumeForDanger(float BaseVolume, EAudio_DangerLevel Level) const;
};

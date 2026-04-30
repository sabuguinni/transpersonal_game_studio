#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "AudioEffectsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EffectType : uint8
{
    ScreenShake     UMETA(DisplayName = "Screen Shake"),
    DamageFlash     UMETA(DisplayName = "Damage Flash"),
    FootstepDust    UMETA(DisplayName = "Footstep Dust"),
    ProximityTension UMETA(DisplayName = "Proximity Tension")
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct FAudio_EffectSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float Intensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    bool bShouldLoop;

    FAudio_EffectSettings()
    {
        Duration = 1.0f;
        Intensity = 1.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.1f;
        bShouldLoop = false;
    }
};

USTRUCT(BlueprintType)
struct FAudio_ProximityEffect
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    UCurveFloat* IntensityCurve;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity")
    USoundBase* ProximitySound;

    FAudio_ProximityEffect()
    {
        MinDistance = 500.0f;
        MaxDistance = 2000.0f;
        IntensityCurve = nullptr;
        ProximitySound = nullptr;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioEffectsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioEffectsManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Effects")
    TMap<EAudio_EffectType, FAudio_EffectSettings> EffectSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Effects")
    TMap<FString, FAudio_ProximityEffect> ProximityEffects;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TArray<UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    USoundBase* ForestNarratorSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    USoundBase* WaterGuideSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    USoundBase* ForestAmbienceSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambient Audio")
    USoundBase* WaterFlowSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float ScreenShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    float DamageFlashDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Effects")
    FLinearColor DamageFlashColor;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core effect functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerScreenShake(EAudio_IntensityLevel Intensity, float Duration = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerDamageFlash(float Duration = 0.5f, FLinearColor FlashColor = FLinearColor::Red);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void TriggerFootstepDust(FVector Location, EAudio_IntensityLevel Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void UpdateProximityTension(AActor* SourceActor, float Distance);

    // Narrative audio functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrativeClip(const FString& ClipName, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayForestNarration(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayWaterGuideNarration(FVector Location);

    // Ambient audio functions
    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void StartForestAmbience(float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void StartWaterAmbience(FVector Location, float FadeInTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Ambient Audio")
    void StopAllAmbience(float FadeOutTime = 2.0f);

    // Proximity detection
    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void RegisterProximitySource(const FString& SourceName, AActor* SourceActor, FAudio_ProximityEffect Settings);

    UFUNCTION(BlueprintCallable, Category = "Proximity Detection")
    void UnregisterProximitySource(const FString& SourceName);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void SetGlobalEffectIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void StopAllEffects();

    UFUNCTION(BlueprintPure, Category = "Audio Effects")
    bool IsEffectActive(EAudio_EffectType EffectType) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Effects")
    void LoadNarrativeAudioAssets();

private:
    // Internal tracking
    TMap<EAudio_EffectType, float> ActiveEffectTimers;
    TMap<FString, AActor*> ProximitySourceActors;
    
    float GlobalEffectIntensity;
    bool bEffectsEnabled;

    // Helper functions
    UAudioComponent* CreateAudioComponent();
    void CleanupFinishedAudioComponents();
    float CalculateProximityIntensity(float Distance, const FAudio_ProximityEffect& Settings);
    void ApplyScreenShakeEffect(float Intensity, float Duration);
    void ApplyDamageFlashEffect(float Duration, FLinearColor Color);
};
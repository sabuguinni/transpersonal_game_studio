#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "Sound/AmbientSound.h"
#include "AdaptiveAudioManager.generated.h"

// ============================================================
// Enums — Audio_* prefix to avoid cross-agent collisions
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

// ============================================================
// Structs
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float CurrentVolume = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_ThreatLevel ActiveOnThreat = EAudio_ThreatLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay ActiveOnTime = EAudio_TimeOfDay::Day;
};

USTRUCT(BlueprintType)
struct FAudio_MusicCue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    FName CueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    TSoftObjectPtr<USoundBase> MusicAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel TriggerThreat = EAudio_ThreatLevel::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float TransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.8f;
};

USTRUCT(BlueprintType)
struct FAudio_SoundEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    FName EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    TSoftObjectPtr<USoundBase> SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    float MaxDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    bool bIs3D = true;
};

// ============================================================
// Delegates
// ============================================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnThreatLevelChanged, EAudio_ThreatLevel, NewThreatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAudio_OnTimeOfDayChanged, EAudio_TimeOfDay, NewTimeOfDay);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAudio_OnSoundEventFired, FName, EventID, FVector, Location);

// ============================================================
// UAudio_AdaptiveAudioManager
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent), DisplayName = "Adaptive Audio Manager")
class TRANSPERSONALGAME_API UAudio_AdaptiveAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveAudioManager();

    // --- State ---
    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|State", meta = (AllowPrivateAccess = "true"))
    EAudio_WeatherState CurrentWeather = EAudio_WeatherState::Clear;

    // --- Config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    TArray<FAudio_MusicCue> MusicCues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Events")
    TArray<FAudio_SoundEvent> SoundEventLibrary;

    // --- Master volumes ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume = 0.9f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SFXVolume = 1.0f;

    // --- Screen shake config ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexShakeRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TRexShakeIntensity = 1.0f;

    // --- Delegates ---
    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnThreatLevelChanged OnThreatLevelChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnTimeOfDayChanged OnTimeOfDayChanged;

    UPROPERTY(BlueprintAssignable, Category = "Audio|Events")
    FAudio_OnSoundEventFired OnSoundEventFired;

    // --- Blueprint-callable API ---
    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetWeather(EAudio_WeatherState NewWeather);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void FireSoundEvent(FName EventID, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void FireSoundEventAtActor(FName EventID, AActor* SourceActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void ForcePlayMusicCue(FName CueID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void StopMusic(float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerTRexScreenShake(FVector TRexLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    float GetThreatBlend() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    // Active audio component handles (raw pointers — no UPROPERTY to avoid GC complexity)
    UAudioComponent* ActiveMusicComponent = nullptr;

    float MusicFadeTimer = 0.0f;
    float MusicFadeTarget = 0.0f;
    float MusicFadeDuration = 3.0f;
    bool bFadingOut = false;

    float DamageFlashTimer = 0.0f;
    float DamageFlashDuration = 0.3f;
    bool bDamageFlashActive = false;

    void UpdateAmbientLayers(float DeltaTime);
    void UpdateMusicFade(float DeltaTime);
    void SelectMusicForState();
    void PopulateDefaultSoundEvents();
};

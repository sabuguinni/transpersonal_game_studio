#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Safe        UMETA(DisplayName = "Safe Zone"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    Combat      UMETA(DisplayName = "Combat Zone"),
    Exploration UMETA(DisplayName = "Exploration Zone"),
    Night       UMETA(DisplayName = "Night Zone")
};

UENUM(BlueprintType)
enum class EAudio_DinoThreatLevel : uint8
{
    None    UMETA(DisplayName = "No Threat"),
    Distant UMETA(DisplayName = "Distant"),
    Near    UMETA(DisplayName = "Near"),
    Imminent UMETA(DisplayName = "Imminent")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float CrossfadeDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bPlayDinoDistantCalls = false;
};

USTRUCT(BlueprintType)
struct FAudio_NarratorLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    float Duration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrator")
    EAudio_DinoThreatLevel TriggerThreatLevel = EAudio_DinoThreatLevel::None;
};

/**
 * AAudio_SystemManager
 * Central audio manager for the prehistoric survival game.
 * Handles adaptive music, ambient zones, dinosaur audio cues,
 * and narrator voice lines. Placed once in MinPlayableMap.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Zone Management ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetAudioZone(EAudio_ZoneType NewZone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetCurrentZone() const { return CurrentZone; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void CrossfadeToZone(EAudio_ZoneType TargetZone, float Duration = 3.0f);

    // --- Threat Level ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void UpdateDinoThreatLevel(EAudio_DinoThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    EAudio_DinoThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // --- Narrator ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Narrator")
    void PlayNarratorLine(const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narrator")
    void RegisterNarratorLine(const FAudio_NarratorLine& Line);

    // --- Screen Shake ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerTRexFootstepShake(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerDamageAudioFeedback(float DamageAmount);

    // --- Campfire Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetCampfireActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    bool IsCampfireActive() const { return bCampfireActive; }

    // --- Day/Night Cycle Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDayBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnNightBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDawnBegin();

    UFUNCTION(BlueprintCallable, Category = "Audio|DayNight")
    void OnDuskBegin();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> CampfireAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components")
    TObjectPtr<UAudioComponent> NarratorAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_ZoneConfig> ZoneConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_NarratorLine> RegisteredNarratorLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TRexFootstepShakeRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientFadeSpeed = 2.0f;

private:
    EAudio_ZoneType CurrentZone = EAudio_ZoneType::Safe;
    EAudio_DinoThreatLevel CurrentThreatLevel = EAudio_DinoThreatLevel::None;
    bool bCampfireActive = false;
    float CrossfadeTimer = 0.0f;
    float ThreatUpdateCooldown = 0.0f;

    void UpdateAmbientVolumes(float DeltaTime);
    const FAudio_ZoneConfig* FindZoneConfig(EAudio_ZoneType ZoneType) const;
};

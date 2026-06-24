#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// === Audio Zone Type ===
UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Safe        UMETA(DisplayName = "Safe"),
    Narrative   UMETA(DisplayName = "Narrative")
};

// === Audio Zone Config ===
USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ZoneLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString NarrativeAudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bOneShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bHasTriggered;

    FAudio_ZoneConfig()
        : ZoneLabel(TEXT(""))
        , Location(FVector::ZeroVector)
        , TriggerRadius(800.0f)
        , ZoneType(EAudio_ZoneType::Ambient)
        , NarrativeAudioURL(TEXT(""))
        , bOneShot(true)
        , bHasTriggered(false)
    {}
};

// === Screen Shake Config ===
USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float ShakeDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    float TriggerRadius;

    FAudio_ScreenShakeConfig()
        : ShakeIntensity(1.5f)
        , ShakeDuration(0.4f)
        , TriggerRadius(1200.0f)
    {}
};

/**
 * UAudio_SystemManager
 * Manages adaptive audio zones, narrative voice triggers, screen shake markers,
 * and ambient soundscape for the prehistoric survival world.
 * Integrates with NarrativeDialogueManager (Agent #15) zone positions.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SystemManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SystemManager();

    // === Audio Zone Registry ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zones")
    TArray<FAudio_ZoneConfig> AudioZones;

    // === Screen Shake ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig TRexShakeConfig;

    // === Adaptive Music State ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    bool bNightMode;

    // === TTS Audio URLs (from Agent #15 + this cycle) ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString TrackerElderWarningURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString TribeLeaderKaelURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString ElderNarratorURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString ScoutMiraURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString SurvivalNarratorURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narrative")
    FString TrackerElderRaptorURL;

    // === Freesound References ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString CampfireAmbientURL;

    // === Methods ===
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void InitializeDefaultZones();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TickAudioZones(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerZone(const FAudio_ZoneConfig& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdateDangerLevel(float NewDanger);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerScreenShake(const FVector& SourceLocation, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void TriggerDamageFlash();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    float GetCurrentDangerLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsPlayerInDangerZone(const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetNightMode(bool bIsNight);

    // UActorComponent overrides
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float TimeSinceLastTick;
    static constexpr float TickInterval = 0.5f;
};

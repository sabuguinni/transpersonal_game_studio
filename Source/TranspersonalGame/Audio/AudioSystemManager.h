#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

/**
 * Audio state enum for adaptive music system
 */
UENUM(BlueprintType)
enum class EAudio_GameState : uint8
{
    Exploration    UMETA(DisplayName = "Exploration"),
    Danger         UMETA(DisplayName = "Danger"),
    Combat         UMETA(DisplayName = "Combat"),
    Night          UMETA(DisplayName = "Night"),
    Safe           UMETA(DisplayName = "Safe")
};

/**
 * Struct for registered ambient sound zones
 */
USTRUCT(BlueprintType)
struct FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    FAudio_AmbientZone()
        : ZoneName(TEXT("DefaultZone"))
        , ZoneCenter(FVector::ZeroVector)
        , ZoneRadius(2000.0f)
        , AmbientSound(nullptr)
        , Volume(1.0f)
    {}
};

/**
 * AAudioSystemManager — Adaptive audio manager for prehistoric survival game.
 * Manages ambient soundscapes, adaptive music state transitions, and
 * dinosaur proximity audio triggers.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioSystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Adaptive Music State ──────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_GameState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float MusicTransitionBlendTime;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetMusicState(EAudio_GameState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_GameState GetCurrentMusicState() const;

    // ── Ambient Zone Management ───────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientZone> RegisteredAmbientZones;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void RegisterAmbientZone(const FAudio_AmbientZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientZones(const FVector& PlayerLocation);

    // ── Dinosaur Proximity Audio ──────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float DinosaurProximityThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaurs")
    float TRexRumbleRadius;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void TriggerDinosaurRoar(AActor* DinosaurActor, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaurs")
    void TriggerFootstepImpact(AActor* DinosaurActor, float Weight);

    // ── Campfire / Environmental Audio ───────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Environment")
    float CampfireAudioRadius;

    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SpawnCampfireAudio(const FVector& Location);

    // ── Screen Shake (camera feedback) ───────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerTRexScreenShake(float Magnitude, float Duration);

    // ── Narration / Dialogue ──────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Narration")
    TArray<FString> NarrationAudioURLs;

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void RegisterNarrationLine(const FString& AudioURL, const FString& LineID);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void PlayNarrationLine(const FString& LineID);

private:
    UPROPERTY()
    UAudioComponent* MusicAudioComponent;

    UPROPERTY()
    TMap<FString, FString> NarrationRegistry;

    float TimeSinceLastZoneUpdate;
    static constexpr float ZoneUpdateInterval = 0.5f;
};

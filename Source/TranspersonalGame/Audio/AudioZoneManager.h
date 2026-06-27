// AudioZoneManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260627_012
// Manages ambient audio zones, proximity-based sound triggering, and screen shake for dinosaur encounters.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "Camera/CameraShakeBase.h"
#include "AudioZoneManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums — prefixed Audio_ to avoid global collisions
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    River       UMETA(DisplayName = "River"),
    Danger      UMETA(DisplayName = "Danger Zone"),
    Camp        UMETA(DisplayName = "Camp"),
    Cave        UMETA(DisplayName = "Cave"),
    OpenPlain   UMETA(DisplayName = "Open Plain"),
};

UENUM(BlueprintType)
enum class EAudio_IntensityLevel : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tense       UMETA(DisplayName = "Tense"),
    Combat      UMETA(DisplayName = "Combat"),
    Flee        UMETA(DisplayName = "Flee"),
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> AmbientLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    TSoftObjectPtr<USoundBase> DangerStinger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TriggerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float Volume = 1.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float ShakeFrequency = 15.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<USoundBase> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString SubtitleText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration = 5.0f;
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudioZoneManager
// ─────────────────────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Zone Configuration ──────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_IntensityLevel CurrentIntensity = EAudio_IntensityLevel::Calm;

    // ── Screen Shake ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    FAudio_ScreenShakeConfig ScreenShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|ScreenShake")
    bool bEnableScreenShake = true;

    // ── Dialogue ────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    TArray<FAudio_DialogueLine> DialogueLines;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dialogue",
        meta = (AllowPrivateAccess = "true"))
    int32 CurrentDialogueIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Dialogue",
        meta = (AllowPrivateAccess = "true"))
    bool bDialogueActive = false;

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    // ── Public API ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetIntensity(EAudio_IntensityLevel NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_IntensityLevel GetCurrentIntensity() const { return CurrentIntensity; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopDialogue();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    bool IsDialogueActive() const { return bDialogueActive; }

    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerScreenShakeAtLocation(FVector DinosaurLocation, float DinosaurMass = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeInAmbient(float FadeTime = -1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void FadeOutAmbient(float FadeTime = -1.0f);

private:
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void AdvanceDialogueInternal();

    FTimerHandle DialogueTimerHandle;
    bool bPlayerInZone = false;
};

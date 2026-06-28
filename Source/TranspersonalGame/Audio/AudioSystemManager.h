// AudioSystemManager.h
// Agent #16 — Audio Agent | PROD_CYCLE_AUTO_20260628_003
// Adaptive audio system for prehistoric survival game
// Manages ambient layers, danger music, NPC dialogue, SFX triggers

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "SharedTypes.h"
#include "AudioSystemManager.generated.h"

// ── Enums ──────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe — Ambient Only"),
    Aware       UMETA(DisplayName = "Aware — Tension Layer"),
    Danger      UMETA(DisplayName = "Danger — Combat Layer"),
    Flee        UMETA(DisplayName = "Flee — Panic Layer"),
    Dead        UMETA(DisplayName = "Dead — Silence")
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
enum class EAudio_BiomeType : uint8
{
    OpenPlains  UMETA(DisplayName = "Open Plains"),
    DenseForest UMETA(DisplayName = "Dense Forest"),
    RiverValley UMETA(DisplayName = "River Valley"),
    RockyUpland UMETA(DisplayName = "Rocky Upland"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Cave        UMETA(DisplayName = "Cave")
};

// ── Structs ────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

USTRUCT(BlueprintType)
struct FAudio_DialogueLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString LineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    USoundBase* SoundAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    FString TriggerEvent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    bool bSpatialised = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dialogue")
    float MaxAudibleDistance = 1200.0f;
};

USTRUCT(BlueprintType)
struct FAudio_ThreatTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel FromLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    EAudio_ThreatLevel ToLevel = EAudio_ThreatLevel::Danger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    float CrossfadeDuration = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    USoundBase* MusicLayer = nullptr;
};

// ── Main Class ─────────────────────────────────────────────────────────────

UCLASS(ClassGroup = (Audio), meta = (DisplayName = "Audio System Manager"))
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Threat System ──────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void RegisterDinosaurNearby(float DistanceMetres, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void ClearDinosaurThreats();

    // ── Ambient System ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void CrossfadeAmbientLayer(FAudio_AmbientLayer NewLayer);

    // ── Dialogue System ────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void PlayDialogueLine(const FAudio_DialogueLine& Line, FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dialogue")
    void StopAllDialogue();

    // ── Screen Shake ───────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFootstepShake(float DinosaurMass, float DistanceMetres);

    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerImpactShake(float ImpactForce);

    // ── Properties ────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_ThreatTransition> MusicTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    TArray<FAudio_DialogueLine> DialogueLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float MusicVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float DialogueVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float SFXVolume = 0.9f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::OpenPlains;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|State",
        meta = (AllowPrivateAccess = "true"))
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

private:
    UPROPERTY()
    UAudioComponent* ActiveAmbientComponent = nullptr;

    UPROPERTY()
    UAudioComponent* ActiveMusicComponent = nullptr;

    float ThreatDecayTimer = 0.0f;
    float ClosestDinosaurDistance = 99999.0f;
    float HeaviestDinosaurMass = 0.0f;

    void UpdateThreatFromDinosaurData();
    void ApplyAmbientForCurrentState();
    float CalculateShakeIntensity(float Mass, float Distance) const;
};

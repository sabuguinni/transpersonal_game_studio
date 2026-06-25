// AudioSystem.h
// Agent #16 — Audio Agent
// Prehistoric survival game adaptive audio system
// PROD_CYCLE_AUTO_20260625_011

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn        UMETA(DisplayName = "Dawn"),
    Day         UMETA(DisplayName = "Day"),
    Dusk        UMETA(DisplayName = "Dusk"),
    Night       UMETA(DisplayName = "Night")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Volcanic    UMETA(DisplayName = "Volcanic"),
    Riverbank   UMETA(DisplayName = "Riverbank")
};

// ============================================================
// STRUCTS — must be at global scope (UHT rule)
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString LineText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    EAudio_ThreatLevel TriggerThreat;

    FAudio_VoiceLine()
        : CharacterName(TEXT(""))
        , LineText(TEXT(""))
        , AudioURL(TEXT(""))
        , Duration(5.0f)
        , TriggerThreat(EAudio_ThreatLevel::Safe)
    {}
};

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FString LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType Biome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay TimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float AttenuationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping;

    FAudio_AmbientLayer()
        : LayerName(TEXT(""))
        , Biome(EAudio_BiomeType::Jungle)
        , TimeOfDay(EAudio_TimeOfDay::Day)
        , BaseVolume(0.7f)
        , AttenuationRadius(3000.0f)
        , bLooping(true)
    {}
};

// ============================================================
// UCLASS — Adaptive Audio Manager
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_AdaptiveManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveManager();

    // Current state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    float DinosaurProximityMeters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|State")
    bool bPlayerInDanger;

    // Ambient layers config
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    // Voice lines registry
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> VoiceLines;

    // Master volume controls
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AmbientVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float VoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Volume", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MusicVolume;

    // Transition speed (seconds to crossfade)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Transitions")
    float CrossfadeDuration;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void UpdateDinosaurProximity(float DistanceMeters);

    UFUNCTION(BlueprintCallable, Category = "Audio|Volume")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    float GetAmbientVolumeForCurrentState() const;

    UFUNCTION(BlueprintPure, Category = "Audio|State")
    FString GetCurrentStateDescription() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void RegisterAmbientLayer(const FAudio_AmbientLayer& Layer);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    float CurrentCrossfadeTime;
    EAudio_ThreatLevel PreviousThreatLevel;
    EAudio_TimeOfDay PreviousTimeOfDay;

    void InitialiseDefaultLayers();
    void InitialiseDefaultVoiceLines();
    float ComputeThreatMultiplier() const;
};

// ============================================================
// UCLASS — Proximity Audio Trigger Actor
// ============================================================

UCLASS(ClassGroup = (TranspersonalGame), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ProximityTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ProximityTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float FadeInDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    float FadeOutDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    bool bPlayOnce;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    EAudio_ThreatLevel AssociatedThreat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Config")
    FString AssociatedVoiceURL;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ActivateTrigger();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DeactivateTrigger();

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsPlayerInRange() const;

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnPlayerEnterRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitRange(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    bool bPlayerPresent;
    bool bHasPlayed;
};

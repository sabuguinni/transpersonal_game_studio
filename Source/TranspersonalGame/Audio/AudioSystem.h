// AudioSystem.h
// Agent #16 — Audio Agent
// Prehistoric survival game audio system — ambient zones, fear-based music, dinosaur audio
// PROD_CYCLE_AUTO_20260622_006

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystem.generated.h"

// ============================================================
// ENUMS — must be at global scope (UE5 UHT rule)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    WindPost        UMETA(DisplayName = "Wind Post"),
    HunterCamp      UMETA(DisplayName = "Hunter Camp"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory"),
    RaptorPatrol    UMETA(DisplayName = "Raptor Patrol"),
    RiverAmbient    UMETA(DisplayName = "River Ambient"),
    OpenPlains      UMETA(DisplayName = "Open Plains"),
    DenseForest     UMETA(DisplayName = "Dense Forest"),
    Cave            UMETA(DisplayName = "Cave")
};

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Silence         UMETA(DisplayName = "Silence"),
    Calm            UMETA(DisplayName = "Calm Ambient"),
    Tension         UMETA(DisplayName = "Tension"),
    Danger          UMETA(DisplayName = "Danger"),
    Combat          UMETA(DisplayName = "Combat"),
    Exploration     UMETA(DisplayName = "Exploration")
};

UENUM(BlueprintType)
enum class EAudio_DinoSoundType : uint8
{
    Idle            UMETA(DisplayName = "Idle Call"),
    Alert           UMETA(DisplayName = "Alert"),
    Roar            UMETA(DisplayName = "Roar"),
    Footstep        UMETA(DisplayName = "Footstep"),
    Attack          UMETA(DisplayName = "Attack"),
    Death           UMETA(DisplayName = "Death"),
    Distant         UMETA(DisplayName = "Distant Call")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::OpenPlains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_MusicLayer MusicLayer = EAudio_MusicLayer::Calm;

    // Freesound IDs for reference (actual assets loaded separately)
    // Campfire: use crackling fire loop
    // RaptorPatrol: use creature distant calls
    // TRexTerritory: use heavy footstep + distant roar
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    int32 FreesoundReferenceId = 0;
};

USTRUCT(BlueprintType)
struct FAudio_FearMusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    float CurrentFear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    EAudio_MusicLayer ActiveLayer = EAudio_MusicLayer::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    float TransitionSpeed = 2.0f;

    // Fear thresholds for music layer transitions
    // 0-20   → Calm
    // 20-45  → Tension
    // 45-70  → Danger
    // 70+    → Combat
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    float TensionThreshold = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    float DangerThreshold = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Fear")
    float CombatThreshold = 70.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinoAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FName DinoSpecies = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float RoarRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FootstepRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float IdleCallInterval = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    float FearImpactOnPlayer = 12.0f;

    // Freesound reference IDs for this species
    // TRex: 837048 (berserker roar), 586545 (Dinosaur Roars Pack 2)
    // Raptor: 586547 (Dinosaur Growls Pack 2)
    // Croc/Brach: 811310 (crocodile bellowing)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    TArray<int32> FreesoundReferenceIds;
};

// ============================================================
// AUDIO ZONE ACTOR
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Audio Zone"))
class TRANSPERSONALGAME_API AAudio_AmbientZone : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZone();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Components",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bIsPlayerInside = false;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* OverlappedActor, AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void SetAmbientVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_MusicLayer GetMusicLayer() const;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    float CurrentVolume = 0.0f;
    float TargetVolume = 0.0f;
};

// ============================================================
// FEAR-BASED MUSIC MANAGER COMPONENT
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Fear Music Manager"))
class TRANSPERSONALGAME_API UAudio_FearMusicManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_FearMusicManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Music")
    FAudio_FearMusicState MusicState;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void UpdateFear(float NewFearValue);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_MusicLayer EvaluateMusicLayer(float FearValue) const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionToLayer(EAudio_MusicLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    EAudio_MusicLayer GetCurrentLayer() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    float GetCurrentFear() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    float FearBlendAlpha = 0.0f;
};

// ============================================================
// DINOSAUR AUDIO COMPONENT
// ============================================================

UCLASS(BlueprintType, Blueprintable, meta=(DisplayName="Dino Audio Component"))
class TRANSPERSONALGAME_API UAudio_DinoAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_DinoAudioComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    FAudio_DinoAudioProfile AudioProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dino")
    bool bIsAlerted = false;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void PlayDinoSound(EAudio_DinoSoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void OnDinoAlert();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void OnDinoRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    void OnDinoFootstep(float StepWeight);

    // Returns fear impact radius — used by FearMusicManager
    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    float GetRoarRadius() const;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dino")
    float GetFearImpact() const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

private:
    float IdleCallTimer = 0.0f;
};

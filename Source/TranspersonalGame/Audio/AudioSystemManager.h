// AudioSystemManager.h
// Audio Agent #16 — Transpersonal Game Studio
// Adaptive audio system: danger-level music, ambient layers, NPC voice proximity triggers

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/EngineTypes.h"
#include "AudioSystemManager.generated.h"

// ============================================================
// ENUMS — global scope (RULE 1)
// ============================================================

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Aware       UMETA(DisplayName = "Aware"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Critical    UMETA(DisplayName = "Critical")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn    UMETA(DisplayName = "Dawn"),
    Day     UMETA(DisplayName = "Day"),
    Dusk    UMETA(DisplayName = "Dusk"),
    Night   UMETA(DisplayName = "Night")
};

// ============================================================
// STRUCTS — global scope (RULE 1)
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    FName LayerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDawn  = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDay   = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtDusk  = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bActiveAtNight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeInTime  = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float FadeOutTime = 2.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DangerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    FName ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel DangerLevel = EAudio_DangerLevel::Aware;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float Radius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    FVector WorldLocation = FVector::ZeroVector;
};

// ============================================================
// UAudio_AmbientLayerComponent
// ============================================================

UCLASS(ClassGroup = "Audio", meta = (BlueprintSpawnableComponent), BlueprintType)
class TRANSPERSONALGAME_API UAudio_AmbientLayerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AmbientLayerComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    EAudio_DangerLevel GetCurrentDangerLevel() const { return CurrentDangerLevel; }

    UFUNCTION(BlueprintPure, Category = "Audio|Ambient")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float DangerUpdateInterval;

private:
    EAudio_DangerLevel CurrentDangerLevel;
    EAudio_TimeOfDay   CurrentTimeOfDay;
    bool bAudioInitialized;

    TArray<UAudioComponent*> ActiveAudioComponents;

    void OnDangerLevelChanged(EAudio_DangerLevel OldLevel, EAudio_DangerLevel NewLevel);
    void UpdateAmbientLayers();
};

// ============================================================
// AAudio_ProximityVoiceTrigger
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ProximityVoiceTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ProximityVoiceTrigger();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FName SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FName VoiceLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    FString VoiceLineURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    bool bRepeatTrigger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float RepeatCooldown;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* ProximitySphere;

    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* VoiceAudioComponent;

    bool  bHasTriggered;
    float LastTriggerTime;

    UFUNCTION()
    void OnPlayerEnterRadius(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);
};

// ============================================================
// AAudio_DangerZoneActor
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_DangerZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DangerZoneActor();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void NotifyAudioSystem(EAudio_DangerLevel NewLevel);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    EAudio_DangerLevel DangerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    float DangerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Danger")
    bool bDynamicRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Danger")
    bool bPlayerInside;

private:
    UPROPERTY(VisibleAnywhere, Category = "Components", meta = (AllowPrivateAccess = "true"))
    USphereComponent* DangerSphere;

    UFUNCTION()
    void OnPlayerEnter(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExit(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);
};

// ============================================================
// AAudio_SystemManager — world singleton
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void SetGlobalDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Manager")
    void RegisterVoiceLineURL(FName LineID, const FString& URL);

    UFUNCTION(BlueprintPure, Category = "Audio|Manager")
    FString GetVoiceLineURL(FName LineID) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Manager")
    float DayNightCycleDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Manager")
    bool bDayNightCycleActive;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    EAudio_DangerLevel CurrentDangerLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio|Manager")
    EAudio_TimeOfDay CurrentTimeOfDay;

private:
    TMap<FName, FString> VoiceLineURLRegistry;
    float ElapsedDayTime;

    void BroadcastTimeOfDayChange(EAudio_TimeOfDay NewTime);
};

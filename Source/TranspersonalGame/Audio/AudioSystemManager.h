#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "AudioSystemManager.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Audio Agent #16 — AudioSystemManager
// Adaptive audio system for prehistoric survival game.
// Manages ambient zones, danger music layers, voice line playback,
// and environmental audio state machine.
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EAudio_DangerLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Threatened  UMETA(DisplayName = "Threatened"),
    Critical    UMETA(DisplayName = "Critical")
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
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Riverbank   UMETA(DisplayName = "Riverbank"),
    Cave        UMETA(DisplayName = "Cave"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FAudio_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Voice")
    FString CharacterName;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Voice")
    FString LineText;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Voice")
    FString AudioURL;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Voice")
    float TriggerRadius = 400.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Voice")
    bool bHasPlayed = false;

    FAudio_VoiceLine() {}
    FAudio_VoiceLine(const FString& InName, const FString& InText, const FString& InURL, float InRadius = 400.0f)
        : CharacterName(InName), LineText(InText), AudioURL(InURL), TriggerRadius(InRadius), bHasPlayed(false) {}
};

USTRUCT(BlueprintType)
struct FAudio_AmbientZone
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    FString ZoneName;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    FVector WorldLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    float Radius = 1000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Jungle;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    float Volume = 1.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Ambient")
    bool bIsActive = true;

    FAudio_AmbientZone() {}
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString SpeciesName;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString RoarSoundID;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString FootstepSoundID;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 3000.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 800.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepInterval = 1.2f;

    FAudio_DinosaurSoundProfile() {}
};

USTRUCT(BlueprintType)
struct FAudio_MusicLayer
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    FString LayerName;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    EAudio_DangerLevel RequiredDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    float FadeInTime = 2.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    float FadeOutTime = 4.0f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    float Volume = 0.7f;

    UPROPERTY(BlueprintReadWrite, Category = "Audio|Music")
    bool bIsLooping = true;

    FAudio_MusicLayer() {}
};

// ─────────────────────────────────────────────────────────────────────────────
// UAudio_SystemManager — WorldSubsystem managing all audio state
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API UAudio_SystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // UWorldSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // ── Danger State ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    void SetDangerLevel(EAudio_DangerLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio|Danger")
    EAudio_DangerLevel GetDangerLevel() const { return CurrentDangerLevel; }

    // ── Time of Day ──
    UFUNCTION(BlueprintCallable, Category = "Audio|TimeOfDay")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintCallable, Category = "Audio|TimeOfDay")
    EAudio_TimeOfDay GetTimeOfDay() const { return CurrentTimeOfDay; }

    // ── Voice Lines ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    bool TryPlayVoiceLine(const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    int32 GetVoiceLineCount() const { return VoiceLineRegistry.Num(); }

    // ── Ambient Zones ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void RegisterAmbientZone(const FAudio_AmbientZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

    // ── Dinosaur Sounds ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void RegisterDinosaurSoundProfile(const FAudio_DinosaurSoundProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurRoar(const FString& SpeciesName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurFootstep(const FString& SpeciesName, const FVector& Location);

    // ── Screen Shake ──
    UFUNCTION(BlueprintCallable, Category = "Audio|ScreenShake")
    void TriggerScreenShake(float Intensity, float Duration, const FVector& SourceLocation);

    // ── Music ──
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void RegisterMusicLayer(const FAudio_MusicLayer& Layer);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void UpdateMusicState();

    // ── Debug ──
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Audio|Debug")
    void LogAudioSystemState() const;

private:
    void PopulateDefaultRegistry();
    void PopulateDinosaurProfiles();
    void PopulateAmbientZones();
    void PopulateMusicLayers();

    UPROPERTY()
    EAudio_DangerLevel CurrentDangerLevel = EAudio_DangerLevel::Safe;

    UPROPERTY()
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY()
    TArray<FAudio_VoiceLine> VoiceLineRegistry;

    UPROPERTY()
    TArray<FAudio_AmbientZone> AmbientZones;

    UPROPERTY()
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY()
    TArray<FAudio_MusicLayer> MusicLayers;

    UPROPERTY()
    FVector LastPlayerLocation = FVector::ZeroVector;

    // Freesound IDs for reference
    // T-Rex Calls: 529462 — https://cdn.freesound.org/previews/529/529462_4355850-hq.mp3
    // Campfire Crackle: 157187 — https://cdn.freesound.org/previews/157/157187_2840005-hq.mp3
    // Long Campfire Loop: 394952 — https://cdn.freesound.org/previews/394/394952_7037-hq.mp3
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_AmbientZoneActor — Placed in level to define ambient audio regions
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API AAudio_AmbientZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbientZoneActor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_AmbientZone ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone")
    TObjectPtr<class USphereComponent> ZoneSphere;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void DeactivateZone();

private:
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// ─────────────────────────────────────────────────────────────────────────────
// AAudio_DinosaurSoundEmitter — Attached to dinosaur actors for positional audio
// ─────────────────────────────────────────────────────────────────────────────
UCLASS()
class TRANSPERSONALGAME_API AAudio_DinosaurSoundEmitter : public AActor
{
    GENERATED_BODY()

public:
    AAudio_DinosaurSoundEmitter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FAudio_DinosaurSoundProfile SoundProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarCooldown = 15.0f;

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayRoar();

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayFootstep();

private:
    float TimeSinceLastRoar = 0.0f;
    float TimeSinceLastFootstep = 0.0f;

    UPROPERTY()
    TObjectPtr<UAudioComponent> RoarAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> FootstepAudioComponent;
};

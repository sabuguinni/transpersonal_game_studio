#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "TimerManager.h"
#include "PrehistoricAudioManager.generated.h"

// ============================================================
// ENUMS — Audio_* prefix to avoid cross-agent name collisions
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
    Aware       UMETA(DisplayName = "Aware"),
    Danger      UMETA(DisplayName = "Danger"),
    Critical    UMETA(DisplayName = "Critical")
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

UENUM(BlueprintType)
enum class EAudio_WeatherState : uint8
{
    Clear       UMETA(DisplayName = "Clear"),
    Overcast    UMETA(DisplayName = "Overcast"),
    Rain        UMETA(DisplayName = "Rain"),
    Storm       UMETA(DisplayName = "Storm")
};

// ============================================================
// STRUCTS
// ============================================================

USTRUCT(BlueprintType)
struct FAudio_AmbientLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    USoundBase* Sound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    float BaseVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_TimeOfDay ActiveTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    EAudio_BiomeType ActiveBiome = EAudio_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Ambient")
    bool bLooping = true;
};

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
    USoundBase* AudioAsset = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    float Duration = 5.0f;
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    FString SpeciesName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* IdleSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* AlertSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* AttackSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    USoundBase* FootstepSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float FootstepRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    float RoarRadius = 8000.0f;
};

// ============================================================
// MAIN CLASS
// ============================================================

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricAudioManager : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricAudioManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Time of Day ---
    UFUNCTION(BlueprintCallable, Category = "Audio|TimeOfDay")
    void SetTimeOfDay(EAudio_TimeOfDay NewTime);

    UFUNCTION(BlueprintPure, Category = "Audio|TimeOfDay")
    EAudio_TimeOfDay GetCurrentTimeOfDay() const { return CurrentTimeOfDay; }

    // --- Threat Level ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Threat")
    void SetThreatLevel(EAudio_ThreatLevel NewThreat);

    UFUNCTION(BlueprintPure, Category = "Audio|Threat")
    EAudio_ThreatLevel GetCurrentThreatLevel() const { return CurrentThreatLevel; }

    // --- Biome ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Biome")
    void SetActiveBiome(EAudio_BiomeType NewBiome);

    // --- Weather ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Weather")
    void SetWeatherState(EAudio_WeatherState NewWeather);

    // --- Voice Lines ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void PlayVoiceLine(const FAudio_VoiceLine& Line);

    UFUNCTION(BlueprintCallable, Category = "Audio|Voice")
    void RegisterVoiceLine(const FAudio_VoiceLine& Line);

    // --- Dinosaur Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurRoar(const FString& SpeciesName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void TriggerDinosaurFootstep(const FString& SpeciesName, FVector Location, float Mass);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void RegisterDinosaurProfile(const FAudio_DinosaurSoundProfile& Profile);

    // --- Screen Shake ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerFootstepShake(float Intensity, FVector SourceLocation);

    // --- Campfire Audio ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Environment")
    void SpawnCampfireAudio(FVector Location);

    // --- Ambient Layer Control ---
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateAmbientLayers();

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void FadeAmbientLayer(int32 LayerIndex, float TargetVolume, float FadeTime);

    // --- Freesound Asset References (set in Blueprint) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets|Campfire")
    USoundBase* CampfireLoopSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets|Campfire")
    USoundBase* CampfireNightAmbienceSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Assets|Ambient")
    TArray<FAudio_AmbientLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Dinosaur")
    TArray<FAudio_DinosaurSoundProfile> DinosaurProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Voice")
    TArray<FAudio_VoiceLine> RegisteredVoiceLines;

    // --- Freesound IDs for reference (populated at runtime) ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Freesound")
    TArray<int32> FreesoundCampfireIDs;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Freesound")
    TArray<FString> FreesoundCampfireURLs;

private:
    UPROPERTY()
    EAudio_TimeOfDay CurrentTimeOfDay = EAudio_TimeOfDay::Day;

    UPROPERTY()
    EAudio_ThreatLevel CurrentThreatLevel = EAudio_ThreatLevel::Safe;

    UPROPERTY()
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Jungle;

    UPROPERTY()
    EAudio_WeatherState CurrentWeather = EAudio_WeatherState::Clear;

    UPROPERTY()
    TArray<UAudioComponent*> ActiveAmbientComponents;

    FTimerHandle AmbientUpdateTimer;
    FTimerHandle ThreatDecayTimer;

    void InitializeDefaultProfiles();
    void DecayThreatLevel();
    float GetThreatMusicIntensity() const;
};

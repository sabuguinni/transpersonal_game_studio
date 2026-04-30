#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundBase.h"
#include "../SharedTypes.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_TensionLevel : uint8
{
    Calm = 0,
    Alert = 1,
    Danger = 2,
    Terror = 3
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest = 0,
    Swamp = 1,
    Savanna = 2,
    Desert = 3,
    SnowMountain = 4
};

USTRUCT(BlueprintType)
struct FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> Sound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLoop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 2.0f;

    FAudio_SoundLayer()
    {
        Sound = nullptr;
        Volume = 1.0f;
        Pitch = 1.0f;
        bLoop = true;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_BiomeAudioConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType = EAudio_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TArray<FAudio_SoundLayer> TensionLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> HeartbeatSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudibleDistance = 5000.0f;

    FAudio_BiomeAudioConfig()
    {
        BiomeType = EAudio_BiomeType::Forest;
        HeartbeatSound = nullptr;
        MaxAudibleDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_DinosaurAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundBase> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float ThreatRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    FAudio_DinosaurAudioData()
    {
        DinosaurType = TEXT("Unknown");
        RoarSound = nullptr;
        FootstepSound = nullptr;
        BreathingSound = nullptr;
        ThreatRadius = 1000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCurrentBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTensionLevel(EAudio_TensionLevel NewTension);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UpdatePlayerPosition(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterDinosaur(AActor* DinosaurActor, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void UnregisterDinosaur(AActor* DinosaurActor);

    // Tension and proximity system
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void CheckDinosaurProximity();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayHeartbeat(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopHeartbeat();

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayEnvironmentalSound(TSoftObjectPtr<USoundBase> Sound, const FVector& Location, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetTimeOfDay(float TimeNormalized); // 0.0 = midnight, 0.5 = noon

    // Voice and dialogue
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayVoiceLine(TSoftObjectPtr<USoundBase> VoiceSound, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAllVoiceLines();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_BiomeAudioConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_DinosaurAudioData> DinosaurAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float ProximityCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float TensionTransitionSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float MaxHeartbeatVolume = 0.8f;

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome = EAudio_BiomeType::Forest;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_TensionLevel CurrentTension = EAudio_TensionLevel::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_TensionLevel TargetTension = EAudio_TensionLevel::Calm;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FVector PlayerLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentHeartbeatIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float TimeOfDayNormalized = 0.5f; // Start at noon

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TArray<UActorComponent*> AmbientAudioComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UActorComponent* HeartbeatAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    UActorComponent* VoiceAudioComponent;

    // Registered dinosaurs
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    TMap<AActor*, FString> RegisteredDinosaurs;

    // Timers
    float ProximityCheckTimer = 0.0f;

private:
    // Internal methods
    void UpdateTensionTransition(float DeltaTime);
    void UpdateAmbientLayers();
    void UpdateTensionLayers();
    FAudio_BiomeAudioConfig* GetCurrentBiomeConfig();
    FAudio_DinosaurAudioData* GetDinosaurAudioData(const FString& DinosaurType);
    float CalculateDistanceToNearestDinosaur();
    EAudio_TensionLevel CalculateTensionFromProximity(float Distance);
    void CreateAudioComponents();
    void CleanupAudioComponents();
};
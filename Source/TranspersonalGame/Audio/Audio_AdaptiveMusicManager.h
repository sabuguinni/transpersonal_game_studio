#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../Narrative/NarrativeStoryManager.h"
#include "Audio_AdaptiveMusicManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient         UMETA(DisplayName = "Ambient"),
    Tension         UMETA(DisplayName = "Tension"),
    Combat          UMETA(DisplayName = "Combat"),
    Exploration     UMETA(DisplayName = "Exploration"),
    Discovery       UMETA(DisplayName = "Discovery")
};

UENUM(BlueprintType)
enum class EAudio_BiomeTheme : uint8
{
    Savana          UMETA(DisplayName = "Savana"),
    Forest          UMETA(DisplayName = "Forest"),
    Desert          UMETA(DisplayName = "Desert"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Mountain        UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EStoryPhase CurrentStoryPhase = EStoryPhase::Awakening;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_BiomeTheme CurrentBiome = EAudio_BiomeTheme::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    EAudio_MusicLayer ActiveLayer = EAudio_MusicLayer::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    float IntensityLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    bool bInCombat = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music State")
    bool bNearPredator = false;

    FAudio_MusicState()
    {
        CurrentStoryPhase = EStoryPhase::Awakening;
        CurrentBiome = EAudio_BiomeTheme::Savana;
        ActiveLayer = EAudio_MusicLayer::Ambient;
        FearLevel = 0.0f;
        IntensityLevel = 0.0f;
        bInCombat = false;
        bNearPredator = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Music control functions
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void UpdateMusicState(const FAudio_MusicState& NewState);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetStoryPhase(EStoryPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetBiome(EAudio_BiomeTheme NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetFearLevel(float NewFearLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetCombatState(bool bInCombat);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetPredatorProximity(bool bNearPredator);

    // Layer management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void FadeInLayer(EAudio_MusicLayer Layer, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void FadeOutLayer(EAudio_MusicLayer Layer, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void CrossfadeToLayer(EAudio_MusicLayer NewLayer, float FadeTime = 3.0f);

    // Music intensity control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void UpdateIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void PlayStinger(const FString& StingerName);

    // Audio component management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void InitializeAudioComponents();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void CleanupAudioComponents();

protected:
    // Current music state
    UPROPERTY(BlueprintReadOnly, Category = "Music State")
    FAudio_MusicState CurrentMusicState;

    // Audio components for layered music
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TMap<EAudio_MusicLayer, UAudioComponent*> MusicLayers;

    // Music assets organized by biome and layer
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TMap<EAudio_BiomeTheme, TMap<EAudio_MusicLayer, USoundCue*>> BiomeMusicAssets;

    // Stinger sounds for dramatic moments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music Assets")
    TMap<FString, USoundCue*> StingerSounds;

    // Crossfade parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crossfade")
    float DefaultFadeTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crossfade")
    float IntensityUpdateRate = 0.1f;

    // Internal update functions
    void UpdateLayerBasedOnState();
    void UpdateVolumeBasedOnIntensity();
    void HandleStoryPhaseTransition(EStoryPhase OldPhase, EStoryPhase NewPhase);
    void HandleBiomeTransition(EAudio_BiomeTheme OldBiome, EAudio_BiomeTheme NewBiome);

    // Timer handles for smooth transitions
    FTimerHandle IntensityUpdateTimer;
    FTimerHandle CrossfadeTimer;
};
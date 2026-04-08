#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "MetasoundSource.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Music,
    Ambience,
    SFX,
    UI,
    Voice,
    Dynamic
};

UENUM(BlueprintType)
enum class EMusicState : uint8
{
    Exploration_Calm,
    Exploration_Tense,
    Danger_Approaching,
    Danger_Imminent,
    Combat_Light,
    Combat_Intense,
    Discovery_Wonder,
    Discovery_Fear,
    Survival_Crafting,
    Survival_Shelter,
    Night_Peaceful,
    Night_Threatening
};

UENUM(BlueprintType)
enum class EAmbienceType : uint8
{
    Forest_Dense,
    Forest_Clearing,
    River_Gentle,
    River_Rapids,
    Cave_Shallow,
    Cave_Deep,
    Plains_Open,
    Swamp_Murky,
    Mountain_High,
    Valley_Protected
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    EMusicState CurrentMusicState = EMusicState::Exploration_Calm;

    UPROPERTY(BlueprintReadWrite)
    EAmbienceType CurrentAmbienceType = EAmbienceType::Forest_Dense;

    UPROPERTY(BlueprintReadWrite)
    float TensionLevel = 0.0f; // 0.0 = calm, 1.0 = maximum tension

    UPROPERTY(BlueprintReadWrite)
    float DangerProximity = 0.0f; // 0.0 = safe, 1.0 = immediate danger

    UPROPERTY(BlueprintReadWrite)
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(BlueprintReadWrite)
    bool bIsInShelter = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsNearWater = false;

    UPROPERTY(BlueprintReadWrite)
    int32 DinosaurCount = 0;

    UPROPERTY(BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0.0 = clear, 1.0 = storm
};

/**
 * Central audio system manager that controls adaptive music, ambient soundscapes,
 * and dynamic audio mixing based on game state
 */
UCLASS()
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Music System
    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void TransitionToMusicState(EMusicState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetTensionLevel(float NewTension, float TransitionTime = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Music")
    void SetDangerProximity(float NewProximity, float TransitionTime = 0.5f);

    // Ambience System
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void TransitionToAmbience(EAmbienceType NewAmbience, float TransitionTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambience")
    void UpdateEnvironmentalFactors(float TimeOfDay, float WeatherIntensity, bool bInShelter);

    // Dynamic Events
    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnDinosaurSpotted(class ADinosaurAI* Dinosaur, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnDinosaurLost(class ADinosaurAI* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnPlayerDiscovery(bool bPositiveDiscovery);

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnCraftingStart();

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnShelterEntered();

    UFUNCTION(BlueprintCallable, Category = "Audio|Events")
    void OnShelterExited();

    // Audio State
    UFUNCTION(BlueprintPure, Category = "Audio|State")
    FAudioStateData GetCurrentAudioState() const { return CurrentAudioState; }

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio|State")
    void SetLayerVolume(EAudioLayer Layer, float Volume);

protected:
    // Core audio state
    UPROPERTY()
    FAudioStateData CurrentAudioState;

    // MetaSound assets for adaptive music
    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveAmbienceMetaSound;

    // Audio components
    UPROPERTY()
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbienceAudioComponent;

    // Sound classes for mixing
    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundClass> MasterSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundClass> MusicSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundClass> AmbienceSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundClass> SFXSoundClass;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundClass> VoiceSoundClass;

    // Sound mixes for dynamic states
    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundMix> CalmStateMix;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundMix> TenseStateMix;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundMix> DangerStateMix;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Mixing")
    TObjectPtr<USoundMix> CombatStateMix;

private:
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    void ApplySoundMix(EMusicState State);
    
    // Transition management
    FTimerHandle MusicTransitionTimer;
    FTimerHandle AmbienceTransitionTimer;
    
    float TargetTensionLevel = 0.0f;
    float TargetDangerProximity = 0.0f;
    
    // Tracked dinosaurs for proximity calculations
    UPROPERTY()
    TArray<TWeakObjectPtr<class ADinosaurAI>> TrackedDinosaurs;
    
    void UpdateDangerProximity();
    void CalculateTensionFromEnvironment();
};
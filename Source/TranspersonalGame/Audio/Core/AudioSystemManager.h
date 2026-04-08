#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "MetasoundSource.h"
#include "AudioParameterControllerInterface.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm = 0,
    Tension,
    Fear,
    Panic,
    Wonder,
    Melancholy
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest = 0,
    OpenPlains,
    Riverside,
    Cave,
    DinosaurNest
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EEmotionalState CurrentEmotion = EEmotionalState::Calm;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    EEnvironmentType Environment = EEnvironmentType::DenseForest;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float ThreatLevel = 0.0f; // 0.0 = safe, 1.0 = maximum danger

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float TimeOfDay = 0.5f; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    int32 NearbyDinosaurs = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    bool bPlayerHidden = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere)
    float PlayerMovementSpeed = 0.0f;
};

/**
 * Central audio system that manages all adaptive music and ambient soundscapes
 * Uses MetaSounds for real-time audio synthesis and parameter control
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core state management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEmotionalState(EEmotionalState NewEmotion, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(float NewThreatLevel, float TransitionTime = 1.0f);

    // Environment audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EEnvironmentType NewEnvironment);

    // Dinosaur audio events
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnDinosaurSpotted(class ADinosaurCharacter* Dinosaur, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnDinosaurLost(class ADinosaurCharacter* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void OnPlayerHiding(bool bIsHiding);

    // Adaptive music control
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAdaptiveMusic();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAdaptiveMusic(float FadeTime = 3.0f);

protected:
    // MetaSound assets
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AmbientSoundscapeMetaSound;

    // Audio components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<class UAudioComponent> MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<class UAudioComponent> AmbientAudioComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    FAudioStateData CurrentAudioState;

    // Parameter controllers for real-time control
    TSharedPtr<Audio::FParameterControllerInterface> MusicParameterController;
    TSharedPtr<Audio::FParameterControllerInterface> AmbientParameterController;

private:
    void InitializeMetaSounds();
    void UpdateMusicParameters();
    void UpdateAmbientParameters();
    void CalculateEmotionalTransition(EEmotionalState TargetEmotion, float TransitionTime);

    // Threat calculation
    float CalculateThreatFromDinosaurs() const;
    
    // Tracked dinosaurs for threat calculation
    TArray<TWeakObjectPtr<class ADinosaurCharacter>> NearbyDinosaurs;

    // Transition timers
    FTimerHandle EmotionTransitionTimer;
    FTimerHandle ThreatTransitionTimer;
};
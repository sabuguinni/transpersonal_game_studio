#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "TranspersonalAudioManager.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessAudioState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Aware           UMETA(DisplayName = "Aware"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class EEnvironmentAudioType : uint8
{
    Forest          UMETA(DisplayName = "Forest"),
    Plains          UMETA(DisplayName = "Plains"),
    Mountains       UMETA(DisplayName = "Mountains"),
    Rivers          UMETA(DisplayName = "Rivers"),
    Sacred          UMETA(DisplayName = "Sacred Sites"),
    Caves           UMETA(DisplayName = "Caves")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudioStateTransition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition")
    EConsciousnessAudioState FromState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition")
    EConsciousnessAudioState ToState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Transition")
    class USoundCue* TransitionSound;

    FAudioStateTransition()
    {
        FromState = EConsciousnessAudioState::Dormant;
        ToState = EConsciousnessAudioState::Awakening;
        TransitionDuration = 3.0f;
        TransitionSound = nullptr;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvironmentAudioLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EEnvironmentAudioType EnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    class USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    class UMetaSoundSource* AdaptiveMusic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeOutTime = 2.0f;

    FEnvironmentAudioLayer()
    {
        EnvironmentType = EEnvironmentAudioType::Forest;
        AmbientSound = nullptr;
        AdaptiveMusic = nullptr;
        BaseVolume = 0.7f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
    }
};

/**
 * TranspersonalAudioManager - Core audio system for consciousness-driven adaptive music and spatial audio
 * Manages the emotional and spiritual audio landscape of the transpersonal journey
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ATranspersonalAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* TransitionAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SpatialAudioComponent;

    // Consciousness State Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    EConsciousnessAudioState CurrentConsciousnessState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    TArray<FAudioStateTransition> StateTransitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    TMap<EConsciousnessAudioState, class UMetaSoundSource*> ConsciousnessMusic;

    // Environment Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EEnvironmentAudioType CurrentEnvironmentType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TArray<FEnvironmentAudioLayer> EnvironmentLayers;

    // Audio Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MusicVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float SpatialAudioRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableAdaptiveMusic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableSpatialAudio = true;

    // Adaptive Music System
    UPROPERTY(BlueprintReadOnly, Category = "Adaptive Music")
    float CurrentMusicIntensity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Adaptive Music")
    float TargetMusicIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive Music")
    float MusicIntensityChangeRate = 0.5f;

public:
    // Consciousness State Management
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetConsciousnessState(EConsciousnessAudioState NewState, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    EConsciousnessAudioState GetConsciousnessState() const { return CurrentConsciousnessState; }

    // Environment Audio Management
    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void SetEnvironmentType(EEnvironmentAudioType NewEnvironment, bool bForceImmediate = false);

    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    EEnvironmentAudioType GetEnvironmentType() const { return CurrentEnvironmentType; }

    // Adaptive Music Control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    void SetMusicIntensity(float NewIntensity, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Music")
    float GetMusicIntensity() const { return CurrentMusicIntensity; }

    // Spatial Audio
    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlaySpatialSound(class USoundCue* Sound, FVector Location, float Volume = 1.0f, float Pitch = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Spatial Audio")
    void PlayConsciousnessTransition(EConsciousnessAudioState FromState, EConsciousnessAudioState ToState);

    // Audio Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMusicVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAmbientVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeOutAllAudio(float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeInAllAudio(float FadeTime = 3.0f);

    // Event Delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessStateChanged, EConsciousnessAudioState, OldState, EConsciousnessAudioState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnConsciousnessStateChanged OnConsciousnessStateChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnvironmentChanged, EEnvironmentAudioType, OldEnvironment, EEnvironmentAudioType, NewEnvironment);
    UPROPERTY(BlueprintAssignable, Category = "Audio Events")
    FOnEnvironmentChanged OnEnvironmentChanged;

private:
    // Internal state management
    bool bIsTransitioning = false;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 0.0f;
    
    EConsciousnessAudioState PendingConsciousnessState;
    EEnvironmentAudioType PendingEnvironmentType;

    // Internal audio management
    void UpdateAdaptiveMusic(float DeltaTime);
    void ProcessConsciousnessTransition(float DeltaTime);
    void ProcessEnvironmentTransition(float DeltaTime);
    void UpdateAudioParameters();

    // Helper functions
    FAudioStateTransition* FindStateTransition(EConsciousnessAudioState FromState, EConsciousnessAudioState ToState);
    FEnvironmentAudioLayer* FindEnvironmentLayer(EEnvironmentAudioType EnvironmentType);
    void ApplyConsciousnessAudioState(EConsciousnessAudioState State);
    void ApplyEnvironmentAudioLayer(EEnvironmentAudioType EnvironmentType);
};
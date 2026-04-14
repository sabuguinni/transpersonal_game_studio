#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Sound/SoundWave.h"
#include "AudioSystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ConsciousnessState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Aware           UMETA(DisplayName = "Aware"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    PrimordialForest    UMETA(DisplayName = "Primordial Forest"),
    SacredGrove         UMETA(DisplayName = "Sacred Grove"),
    AncientRiver        UMETA(DisplayName = "Ancient River"),
    MysticCave          UMETA(DisplayName = "Mystic Cave"),
    SpiritualPlain      UMETA(DisplayName = "Spiritual Plain")
};

USTRUCT(BlueprintType)
struct FAudio_ConsciousnessLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    EAudio_ConsciousnessState State = EAudio_ConsciousnessState::Dormant;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Audio")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    FAudio_ConsciousnessLayer()
    {
        State = EAudio_ConsciousnessState::Dormant;
        Intensity = 0.0f;
        TransitionSpeed = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FAudio_EnvironmentProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::PrimordialForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TSoftObjectPtr<UMetaSoundSource> AmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TArray<TSoftObjectPtr<USoundWave>> LayerSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeInTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float FadeOutTime = 2.0f;

    FAudio_EnvironmentProfile()
    {
        EnvironmentType = EAudio_EnvironmentType::PrimordialForest;
        BaseVolume = 0.7f;
        FadeInTime = 3.0f;
        FadeOutTime = 2.0f;
    }
};

/**
 * Core audio system manager for the Transpersonal Game
 * Handles adaptive music, consciousness-based audio layers, and environmental soundscapes
 * Uses MetaSounds for real-time audio synthesis and procedural composition
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioSystemManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioSystemManager();

    // Core System Methods
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioSystem();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void ShutdownAudioSystem();

    // Consciousness Audio Control
    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void SetConsciousnessState(EAudio_ConsciousnessState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Audio")
    void UpdateConsciousnessIntensity(float NewIntensity, float BlendTime = 1.0f);

    UFUNCTION(BlueprintPure, Category = "Consciousness Audio")
    EAudio_ConsciousnessState GetCurrentConsciousnessState() const { return CurrentConsciousnessState; }

    // Environment Audio Control
    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void TransitionToEnvironment(EAudio_EnvironmentType NewEnvironment, float TransitionTime = 4.0f);

    UFUNCTION(BlueprintCallable, Category = "Environment Audio")
    void SetEnvironmentIntensity(float Intensity, float BlendTime = 1.5f);

    // Narrative Audio Control
    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void PlayNarrationLine(const FString& NarrationKey, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Narrative Audio")
    void StopNarration(float FadeOutTime = 1.0f);

    // MetaSounds Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void UpdateMetaSoundParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void TriggerMetaSoundEvent(const FString& EventName);

protected:
    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_ConsciousnessState CurrentConsciousnessState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    EAudio_EnvironmentType CurrentEnvironment;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    float ConsciousnessIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State", meta = (AllowPrivateAccess = "true"))
    float EnvironmentIntensity;

    // Audio Components
    UPROPERTY(BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> ConsciousnessAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> EnvironmentAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Components", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_ConsciousnessState, FAudio_ConsciousnessLayer> ConsciousnessLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<EAudio_EnvironmentType, FAudio_EnvironmentProfile> EnvironmentProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TSoftObjectPtr<USoundWave>> NarrationLibrary;

private:
    // Internal Methods
    void InitializeConsciousnessLayers();
    void InitializeEnvironmentProfiles();
    void InitializeNarrationLibrary();
    
    void UpdateConsciousnessAudio();
    void UpdateEnvironmentAudio();
    
    bool bIsInitialized;
    float MasterVolume;
    
    // Transition Management
    FTimerHandle ConsciousnessTransitionTimer;
    FTimerHandle EnvironmentTransitionTimer;
    
    void OnConsciousnessTransitionComplete();
    void OnEnvironmentTransitionComplete();
};
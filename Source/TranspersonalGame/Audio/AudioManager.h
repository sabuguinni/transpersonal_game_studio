#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Engine/DataTable.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary     UMETA(DisplayName = "Ordinary Consciousness"),
    Meditative   UMETA(DisplayName = "Meditative State"),
    Transcendent UMETA(DisplayName = "Transcendent State"),
    Unity        UMETA(DisplayName = "Unity Consciousness")
};

UENUM(BlueprintType)
enum class EAudioLayer : uint8
{
    Ambient      UMETA(DisplayName = "Ambient"),
    Music        UMETA(DisplayName = "Music"),
    SFX          UMETA(DisplayName = "Sound Effects"),
    Voice        UMETA(DisplayName = "Voice/Dialogue"),
    Consciousness UMETA(DisplayName = "Consciousness Effects")
};

USTRUCT(BlueprintType)
struct FAudioStateData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessState ConsciousnessState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ReverbIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LowPassFilter = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpatialDistortion = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<USoundWave*> ConsciousnessEffects;
};

UCLASS()
class TRANSPERSONALGAME_API AAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ConsciousnessAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Data")
    class UDataTable* AudioStateDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float TransitionDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TMap<EAudioLayer, float> LayerVolumes;

private:
    EConsciousnessState CurrentConsciousnessState;
    bool bIsTransitioning;
    float TransitionTimer;
    FAudioStateData CurrentAudioState;
    FAudioStateData TargetAudioState;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlaySoundEffect(USoundCue* SoundCue, FVector Location = FVector::ZeroVector, bool bIs3D = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayVoiceLine(USoundWave* VoiceClip, float Delay = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetLayerVolume(EAudioLayer Layer, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void TriggerConsciousnessEffect(int32 EffectIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintPure, Category = "Audio Management")
    EConsciousnessState GetCurrentConsciousnessState() const { return CurrentConsciousnessState; }

    UFUNCTION(BlueprintPure, Category = "Audio Management")
    bool IsTransitioning() const { return bIsTransitioning; }

protected:
    virtual void Tick(float DeltaTime) override;

private:
    void UpdateAudioTransition(float DeltaTime);
    void ApplyAudioEffects(const FAudioStateData& AudioData, float Alpha);
    FAudioStateData* GetAudioStateData(EConsciousnessState State);
    void CrossfadeAudio(UAudioComponent* AudioComponent, USoundCue* NewSound, float FadeDuration);
};
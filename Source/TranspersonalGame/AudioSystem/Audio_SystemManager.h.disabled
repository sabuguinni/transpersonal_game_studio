#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"
#include "../Core/SharedTypes.h"
#include "Audio_SystemManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_LayerType : uint8
{
    Ambient         UMETA(DisplayName = "Ambient Environment"),
    Proximity       UMETA(DisplayName = "Proximity Warning"),
    Narration       UMETA(DisplayName = "Narrative Voice"),
    SeismicWarning  UMETA(DisplayName = "Seismic T-Rex Warning"),
    DinosaurSFX     UMETA(DisplayName = "Dinosaur Sound Effects"),
    PlayerSFX       UMETA(DisplayName = "Player Sound Effects")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_LayerConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    EAudio_LayerType LayerType = EAudio_LayerType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    bool bIs3D = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layer")
    float AttenuationDistance = 5000.0f;

    FAudio_LayerConfig()
    {
        LayerType = EAudio_LayerType::Ambient;
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bIs3D = true;
        AttenuationDistance = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float TRexWarningDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float RaptorWarningDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float SeismicIntensityMultiplier = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Proximity Audio")
    float ProximityUpdateRate = 0.5f;

    FAudio_ProximitySettings()
    {
        TRexWarningDistance = 3000.0f;
        RaptorWarningDistance = 1500.0f;
        SeismicIntensityMultiplier = 2.0f;
        ProximityUpdateRate = 0.5f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SystemManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SystemManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Audio Components for different layers
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* ProximityAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* NarrationAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SeismicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DinosaurSFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* PlayerSFXComponent;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_LayerConfig> AudioLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    FAudio_ProximitySettings ProximitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    bool bEnableProximityWarnings = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    bool bEnableEnvironmentalAudio = true;

    // Runtime state
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float ProximityCheckTimer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bTRexNearby = false;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bRaptorsNearby = false;

public:
    // Audio layer management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayAudioLayer(EAudio_LayerType LayerType, class USoundBase* Sound, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAudioLayer(EAudio_LayerType LayerType, float FadeOutTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetLayerVolume(EAudio_LayerType LayerType, float Volume, float FadeTime = 1.0f);

    // Proximity warning system
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerProximityWarning(EAudio_LayerType WarningType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateProximityWarnings();

    // Narration system integration
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarration(class USoundBase* NarrationSound, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopNarration(float FadeOutTime = 1.0f);

    // Environmental audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentalAudio(class USoundBase* AmbientSound, float Volume = 0.7f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionEnvironmentalAudio(class USoundBase* NewAmbientSound, float TransitionTime = 3.0f);

    // Utility functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    class UAudioComponent* GetAudioComponentForLayer(EAudio_LayerType LayerType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    float GetMasterVolume() const { return MasterVolume; }

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsLayerPlaying(EAudio_LayerType LayerType);

private:
    void InitializeAudioComponents();
    void SetupAudioLayerDefaults();
    float CalculateDistanceToPlayer();
    void CheckDinosaurProximity();
};
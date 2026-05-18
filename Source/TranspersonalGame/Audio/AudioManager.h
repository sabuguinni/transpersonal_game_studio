#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "No Threat"),
    Low         UMETA(DisplayName = "Low Threat"),
    Medium      UMETA(DisplayName = "Medium Threat"),
    High        UMETA(DisplayName = "High Threat"),
    Critical    UMETA(DisplayName = "Critical Threat")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> RoarSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> BreathingSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxAudibleDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier;

    FAudio_DinosaurSoundData()
    {
        DinosaurType = EAudio_DinosaurType::TRex;
        MaxAudibleDistance = 5000.0f;
        VolumeMultiplier = 1.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Dinosaur audio system
    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurFootstep(EAudio_DinosaurType DinosaurType, const FVector& Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayDinosaurRoar(EAudio_DinosaurType DinosaurType, const FVector& Location, float VolumeScale = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio|Dinosaur")
    void PlayProximityWarning(EAudio_DinosaurType DinosaurType, const FVector& PlayerLocation, const FVector& DinosaurLocation);

    // Ambient audio system
    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void SetBiomeAmbience(const FString& BiomeName, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|Ambient")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    // Screen shake audio feedback
    UFUNCTION(BlueprintCallable, Category = "Audio|Feedback")
    void TriggerScreenShakeAudio(float Intensity, const FVector& SourceLocation);

    // Narration system
    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void PlayNarrationClip(const FString& ClipName, bool bInterruptCurrent = false);

    UFUNCTION(BlueprintCallable, Category = "Audio|Narration")
    void StopNarration();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_DinosaurSoundData> DinosaurSoundDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TSoftObjectPtr<USoundCue> DefaultFootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TSoftObjectPtr<USoundCue> ScreenShakeRumbleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float GlobalVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxDinosaurAudioDistance;

private:
    UPROPERTY()
    TObjectPtr<UAudioComponent> CurrentNarrationComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbienceComponent;

    EAudio_ThreatLevel CurrentThreatLevel;

    void InitializeDinosaurSounds();
    void LoadAudioAssets();
    FAudio_DinosaurSoundData* GetDinosaurSoundData(EAudio_DinosaurType DinosaurType);
    float CalculateVolumeByDistance(const FVector& SourceLocation, const FVector& ListenerLocation, float MaxDistance);
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "SharedTypes.h"
#include "MetaSoundAudioManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float VolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping;

    FAudio_SoundZoneData()
    {
        ZoneName = TEXT("DefaultZone");
        VolumeMultiplier = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bLooping = true;
    }
};

UENUM(BlueprintType)
enum class EAudio_SurfaceType : uint8
{
    Dirt        UMETA(DisplayName = "Dirt"),
    Grass       UMETA(DisplayName = "Grass"),
    Rock        UMETA(DisplayName = "Rock"),
    Water       UMETA(DisplayName = "Water"),
    Sand        UMETA(DisplayName = "Sand"),
    Wood        UMETA(DisplayName = "Wood")
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AMetaSoundAudioManager : public AActor
{
    GENERATED_BODY()

public:
    AMetaSoundAudioManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class USphereComponent* ProximityDetectionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_SoundZoneData CurrentZoneData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_SoundZoneData> AudioZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float ProximityRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableProximityAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bEnableDynamicMixing;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayAmbientSound(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAmbientSound();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayFootstepSound(EAudio_SurfaceType SurfaceType, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayProximityWarning(const FString& ThreatType);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void FadeToZone(const FString& ZoneName, float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdateDynamicMix(float TensionLevel, float ExplorationLevel);

protected:
    UFUNCTION()
    void OnProximityEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                         UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                         bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnProximityExit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                        UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void InitializeAudioComponents();
    void SetupProximityDetection();
    FAudio_SoundZoneData* FindAudioZone(const FString& ZoneName);
    
    float CurrentTensionLevel;
    float CurrentExplorationLevel;
    bool bIsTransitioning;
    FTimerHandle TransitionTimerHandle;
};
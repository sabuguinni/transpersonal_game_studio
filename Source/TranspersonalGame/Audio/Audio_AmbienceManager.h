#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Audio_AmbienceManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Cave        UMETA(DisplayName = "Cave"),
    River       UMETA(DisplayName = "River"),
    Mountain    UMETA(DisplayName = "Mountain"),
    Swamp       UMETA(DisplayName = "Swamp")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_AmbienceLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    class USoundBase* SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MinDistance = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxDistance = 2000.0f;

    FAudio_AmbienceLayer()
    {
        SoundAsset = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_AmbienceManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_AmbienceManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* ForestAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WindAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UAudioComponent* WildlifeAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    EAudio_AmbienceType CurrentAmbienceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    TMap<EAudio_AmbienceType, FAudio_AmbienceLayer> AmbienceLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    float TransitionDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    float PlayerProximityRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    bool bEnableTimeOfDayVariation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    float DayVolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience Settings")
    float NightVolumeMultiplier = 0.7f;

    UFUNCTION(BlueprintCallable, Category = "Audio Ambience")
    void SetAmbienceType(EAudio_AmbienceType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio Ambience")
    void FadeToAmbience(EAudio_AmbienceType NewType, float FadeDuration = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Ambience")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio Ambience")
    void UpdateTimeOfDayVolume(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Audio Ambience")
    void PlayOneShot(class USoundBase* Sound, FVector Location, float Volume = 1.0f);

private:
    UPROPERTY()
    class APawn* CachedPlayerPawn;

    UPROPERTY()
    EAudio_AmbienceType TargetAmbienceType;

    UPROPERTY()
    bool bIsTransitioning;

    UPROPERTY()
    float TransitionTimer;

    UPROPERTY()
    float CurrentMasterVolume;

    void UpdatePlayerProximity();
    void ProcessAmbienceTransition(float DeltaTime);
    void InitializeAmbienceLayers();
    float CalculateDistanceAttenuation(float Distance) const;
};
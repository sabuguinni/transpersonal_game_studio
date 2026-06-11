#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Engine/TriggerVolume.h"
#include "Audio_MetaSoundsManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicState : uint8
{
    Peaceful,
    Exploration,
    Danger,
    Combat,
    Victory,
    Death
};

UENUM(BlueprintType)
enum class EAudio_AmbienceType : uint8
{
    Forest,
    River,
    Cave,
    Plains,
    Mountain,
    Swamp
};

USTRUCT(BlueprintType)
struct FAudio_SoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float FadeOutTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    bool bLoop = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Config")
    float AttenuationDistance = 1000.0f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundsManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundsManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbienceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* SFXAudioComponent;

    // Music System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    EAudio_MusicState CurrentMusicState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    TMap<EAudio_MusicState, class USoundBase*> MusicTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    FAudio_SoundConfig MusicConfig;

    // Ambience System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    EAudio_AmbienceType CurrentAmbienceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    TMap<EAudio_AmbienceType, class USoundBase*> AmbienceTracks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ambience")
    FAudio_SoundConfig AmbienceConfig;

    // Dynamic Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float MusicTransitionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    float AmbienceBlendRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Audio")
    bool bEnableAudioOcclusion;

public:
    // Music Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetMusicState(EAudio_MusicState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void FadeMusicTo(EAudio_MusicState NewState, float FadeTime = 3.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopMusic(float FadeTime = 2.0f);

    // Ambience Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAmbienceType(EAudio_AmbienceType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void BlendAmbienceTypes(EAudio_AmbienceType FromType, EAudio_AmbienceType ToType, float BlendFactor);

    // SFX Control
    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlaySFX(class USoundBase* Sound, FVector Location = FVector::ZeroVector, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlaySFXAtLocation(class USoundBase* Sound, FVector Location, FAudio_SoundConfig Config);

    // MetaSounds Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void SetMetaSoundParameter(FName ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void TriggerMetaSoundEvent(FName EventName);

    // Audio Zones
    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void RegisterAudioZone(class ATriggerVolume* Zone, EAudio_AmbienceType AmbienceType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void OnPlayerEnterAudioZone(EAudio_AmbienceType ZoneType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zones")
    void OnPlayerExitAudioZone(EAudio_AmbienceType ZoneType);

private:
    // Internal state
    bool bIsMusicFading;
    float MusicFadeTimer;
    EAudio_MusicState TargetMusicState;
    
    bool bIsAmbienceFading;
    float AmbienceFadeTimer;
    EAudio_AmbienceType TargetAmbienceType;

    // Registered audio zones
    TMap<class ATriggerVolume*, EAudio_AmbienceType> RegisteredAudioZones;

    // Internal methods
    void UpdateMusicFade(float DeltaTime);
    void UpdateAmbienceFade(float DeltaTime);
    void ApplyAudioOcclusion();
    float CalculateDistanceAttenuation(FVector SourceLocation, FVector ListenerLocation, float MaxDistance);
};
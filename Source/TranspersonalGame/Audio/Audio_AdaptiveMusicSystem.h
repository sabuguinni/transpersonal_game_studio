#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Subsystems/WorldSubsystem.h"
#include "../SharedTypes.h"
#include "Audio_AdaptiveMusicSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_MusicLayer : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Tension     UMETA(DisplayName = "Tension"),
    Danger      UMETA(DisplayName = "Danger"),
    Combat      UMETA(DisplayName = "Combat"),
    Exploration UMETA(DisplayName = "Exploration")
};

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MusicLayerData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    USoundBase* SoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeInTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    float FadeOutTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music")
    bool bIsLooping;

    FAudio_MusicLayerData()
    {
        SoundAsset = nullptr;
        Volume = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 2.0f;
        bIsLooping = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeMusicProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Music")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Music")
    TMap<EAudio_MusicLayer, FAudio_MusicLayerData> MusicLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Music")
    float BaseIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Music")
    float ThreatMultiplier;

    FAudio_BiomeMusicProfile()
    {
        BiomeType = EAudio_BiomeType::Forest;
        BaseIntensity = 0.3f;
        ThreatMultiplier = 2.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_AdaptiveMusicComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    TMap<EAudio_BiomeType, FAudio_BiomeMusicProfile> BiomeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    TMap<EAudio_MusicLayer, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    EAudio_MusicLayer CurrentMusicLayer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float CurrentThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float LayerTransitionSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Music System")
    float ThreatDecayRate;

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void SetThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void TransitionToLayer(EAudio_MusicLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void UpdateMusicIntensity(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Music System")
    void InitializeBiomeProfiles();

    UFUNCTION(BlueprintCallable, Category = "Music System")
    EAudio_MusicLayer CalculateTargetLayer();

private:
    void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime);
    void CreateAudioComponentForLayer(EAudio_MusicLayer Layer);
};

UCLASS()
class TRANSPERSONALGAME_API UAudio_AdaptiveMusicSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintReadOnly, Category = "Music Subsystem")
    UAudio_AdaptiveMusicComponent* GlobalMusicComponent;

    UFUNCTION(BlueprintCallable, Category = "Music Subsystem")
    void RegisterMusicComponent(UAudio_AdaptiveMusicComponent* MusicComponent);

    UFUNCTION(BlueprintCallable, Category = "Music Subsystem")
    void SetGlobalThreatLevel(float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Music Subsystem")
    void SetGlobalBiome(EAudio_BiomeType Biome);
};
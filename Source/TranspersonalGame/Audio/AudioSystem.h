#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioGameplayVolume.h"
#include "Subsystems/WorldSubsystem.h"
#include "AudioSystem.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm,
    Tense,
    Danger,
    Terror,
    Wonder,
    Melancholy
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    DenseForest,
    OpenPlains,
    RiverSide,
    CaveSystem,
    DinosaurNest,
    SafeZone
};

USTRUCT(BlueprintType)
struct FAudioStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEmotionalState EmotionalState = EEmotionalState::Calm;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EEnvironmentType EnvironmentType = EEnvironmentType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay = 0.5f; // 0-1 (0=midnight, 0.5=noon)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WeatherIntensity = 0.0f; // 0-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerHidden = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NearbyDinosaurCount = 0;
};

UCLASS()
class TRANSPERSONALGAME_API UAudioSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAudioState(const FAudioStateData& NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(class ADinosaur* Dinosaur, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayEnvironmentalSound(const FVector& Location, const FString& SoundType, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMusicIntensity(float Intensity);

private:
    UPROPERTY()
    class UMetaSoundSource* AdaptiveMusicSource;

    UPROPERTY()
    class UAudioComponent* MusicComponent;

    UPROPERTY()
    class UAudioComponent* AmbienceComponent;

    UPROPERTY()
    TArray<class UAudioComponent*> DynamicAudioComponents;

    FAudioStateData CurrentAudioState;

    void UpdateAdaptiveMusic();
    void UpdateAmbience();
    void ProcessAudioTransition(const FAudioStateData& OldState, const FAudioStateData& NewState);
    
    UPROPERTY()
    class USoundAttenuation* DinosaurAttenuation;

    UPROPERTY()
    class USoundAttenuation* EnvironmentAttenuation;
};
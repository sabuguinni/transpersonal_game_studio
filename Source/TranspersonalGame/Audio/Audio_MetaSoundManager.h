#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "Audio_MetaSoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dinosaur,
    Player,
    Weather,
    Combat,
    UI
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundLayer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category = EAudio_SoundCategory::Ambient;

    FAudio_SoundLayer()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        bLooping = false;
        Category = EAudio_SoundCategory::Ambient;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TArray<FAudio_SoundLayer> AmbientLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Layers")
    TArray<FAudio_SoundLayer> DinosaurLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float DinosaurVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float WeatherVolume = 0.8f;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void PlayMetaSoundLayer(const FAudio_SoundLayer& SoundLayer, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StopSoundCategory(EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetCategoryVolume(EAudio_SoundCategory Category, float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void UpdateAmbientLayers();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void TriggerDinosaurProximityAudio(float Distance, float DinosaurSize);

private:
    UPROPERTY()
    TMap<EAudio_SoundCategory, TArray<class UAudioComponent*>> ActiveAudioComponents;

    void CleanupFinishedComponents();
    UAudioComponent* CreateAudioComponentForMetaSound(UMetaSoundSource* MetaSound, FVector Location);
};
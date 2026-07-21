#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_EnvironmentalSoundscape.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Plains      UMETA(DisplayName = "Plains"),
    Mountains   UMETA(DisplayName = "Mountains"),
    River       UMETA(DisplayName = "River"),
    Cave        UMETA(DisplayName = "Cave")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundscapeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeTime = 2.0f;

    FAudio_SoundscapeData()
    {
        BiomeType = EAudio_BiomeType::Forest;
        Volume = 1.0f;
        FadeTime = 2.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_EnvironmentalSoundscape : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_EnvironmentalSoundscape();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_SoundscapeData> BiomeSoundscapes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float TransitionTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MaxAudibleDistance = 2000.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* AmbientAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MusicAudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsTransitioning;

public:
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetBiomeSoundscape(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void FadeToNewSoundscape(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintPure, Category = "Audio")
    EAudio_BiomeType GetCurrentBiome() const { return CurrentBiome; }

    UFUNCTION(BlueprintPure, Category = "Audio")
    bool IsTransitioning() const { return bIsTransitioning; }

private:
    void InitializeAudioComponents();
    void LoadSoundscapeForBiome(EAudio_BiomeType BiomeType);
    FAudio_SoundscapeData* GetSoundscapeData(EAudio_BiomeType BiomeType);

    FTimerHandle TransitionTimerHandle;
    void CompleteTransition();
};
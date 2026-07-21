#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Sound/SoundCue.h"
#include "Sound/SoundWave.h"
#include "Audio_PrehistoricSoundLibrary.generated.h"

UENUM(BlueprintType)
enum class EAudio_PrehistoricSoundType : uint8
{
    TribalDrums,
    DinosaurFootsteps,
    CanyonWind,
    PredatorGrowl,
    AmbientNature,
    WeatherStorm,
    FireCrackle,
    WaterFlow,
    BirdCalls,
    InsectChirps
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PrehistoricSound
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_PrehistoricSoundType SoundType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString FreesoundID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString PreviewURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsLooping = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DefaultVolume = 1.0f;

    FAudio_PrehistoricSound()
    {
        SoundType = EAudio_PrehistoricSoundType::AmbientNature;
        SoundName = TEXT("");
        FreesoundID = TEXT("");
        PreviewURL = TEXT("");
        Duration = 0.0f;
        bIsLooping = false;
        DefaultVolume = 1.0f;
    }
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudio_PrehistoricSoundLibrary : public UObject
{
    GENERATED_BODY()

public:
    UAudio_PrehistoricSoundLibrary();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static TArray<FAudio_PrehistoricSound> GetTribalDrumSounds();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static TArray<FAudio_PrehistoricSound> GetDinosaurFootstepSounds();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static TArray<FAudio_PrehistoricSound> GetAmbientNatureSounds();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static TArray<FAudio_PrehistoricSound> GetPredatorGrowlSounds();

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static FAudio_PrehistoricSound GetSoundByType(EAudio_PrehistoricSoundType SoundType);

    UFUNCTION(BlueprintCallable, Category = "Prehistoric Audio")
    static TArray<FAudio_PrehistoricSound> GetAllPrehistoricSounds();

protected:
    static void InitializeTribalDrumLibrary(TArray<FAudio_PrehistoricSound>& OutSounds);
    static void InitializeDinosaurFootstepLibrary(TArray<FAudio_PrehistoricSound>& OutSounds);
    static void InitializeAmbientNatureLibrary(TArray<FAudio_PrehistoricSound>& OutSounds);
    static void InitializePredatorGrowlLibrary(TArray<FAudio_PrehistoricSound>& OutSounds);
};
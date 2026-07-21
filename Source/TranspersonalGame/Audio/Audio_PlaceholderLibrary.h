#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundCue.h"
#include "SharedTypes.h"
#include "Audio_PlaceholderLibrary.generated.h"

// Audio placeholder specifications for manual generation
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_PlaceholderSpec
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    FString AudioID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    FString SuggestedFreesoundQuery;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    float TargetDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    float RecommendedVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    bool bShouldLoop;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Spec")
    FString AudioCategory;

    FAudio_PlaceholderSpec()
    {
        AudioID = TEXT("");
        Description = TEXT("");
        SuggestedFreesoundQuery = TEXT("");
        TargetDuration = 5.0f;
        RecommendedVolume = 1.0f;
        bShouldLoop = false;
        AudioCategory = TEXT("SFX");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_PlaceholderLibrary : public UDataAsset
{
    GENERATED_BODY()

public:
    UAudio_PlaceholderLibrary();

    // Prehistoric dinosaur audio specifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TArray<FAudio_PlaceholderSpec> DinosaurAudioSpecs;

    // Environment and ambient audio specifications  
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TArray<FAudio_PlaceholderSpec> EnvironmentAudioSpecs;

    // Survival gameplay audio specifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Audio")
    TArray<FAudio_PlaceholderSpec> SurvivalAudioSpecs;

    // Combat and action audio specifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Audio")
    TArray<FAudio_PlaceholderSpec> CombatAudioSpecs;

    UFUNCTION(BlueprintCallable, Category = "Audio Placeholder")
    TArray<FAudio_PlaceholderSpec> GetAllAudioSpecs() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Placeholder")
    FAudio_PlaceholderSpec GetAudioSpecByID(const FString& AudioID) const;

    UFUNCTION(BlueprintCallable, Category = "Audio Placeholder")
    TArray<FAudio_PlaceholderSpec> GetAudioSpecsByCategory(const FString& Category) const;

protected:
    void InitializePrehistoricAudioSpecs();
};
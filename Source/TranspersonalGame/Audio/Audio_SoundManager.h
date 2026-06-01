#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Music,
    SFX,
    Voice,
    UI
};

USTRUCT(BlueprintType)
struct FAudio_SoundEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<USoundCue> SoundCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EAudio_SoundCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bLoop = false;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::SFX;
        Volume = 1.0f;
        bLoop = false;
    }
};

UCLASS()
class TRANSPERSONALGAME_API UAudio_SoundManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlaySound(const FString& SoundName, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void RegisterSound(const FAudio_SoundEntry& SoundEntry);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayAmbientLoop(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopAmbientLoop();

private:
    UPROPERTY()
    TMap<FString, FAudio_SoundEntry> RegisteredSounds;

    UPROPERTY()
    TMap<FString, UAudioComponent*> ActiveAudioComponents;

    UPROPERTY()
    UAudioComponent* CurrentAmbientComponent;

    float MasterVolume = 1.0f;
    TMap<EAudio_SoundCategory, float> CategoryVolumes;

    void InitializeDefaultSounds();
    UAudioComponent* CreateAudioComponent(const FAudio_SoundEntry& SoundEntry, FVector Location);
};
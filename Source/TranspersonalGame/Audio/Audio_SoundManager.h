#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_SoundCategory : uint8
{
    Ambient,
    Dinosaur,
    Player,
    Environment,
    UI,
    Music
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
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIs3D = true;

    FAudio_SoundEntry()
    {
        SoundName = TEXT("");
        Category = EAudio_SoundCategory::Ambient;
        Volume = 1.0f;
        Pitch = 1.0f;
        bIs3D = true;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_SoundManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_SoundManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    TArray<FAudio_SoundEntry> SoundLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float DinosaurVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio System")
    float PlayerVolume = 0.8f;

    TMap<FString, UAudioComponent*> ActiveAudioComponents;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlaySound(const FString& SoundName, FVector Location = FVector::ZeroVector, AActor* AttachToActor = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopSound(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAllSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetCategoryVolume(EAudio_SoundCategory Category, float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterSound(const FString& SoundName, USoundCue* SoundCue, EAudio_SoundCategory Category);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    bool IsSoundPlaying(const FString& SoundName);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeDefaultSounds();

private:
    FAudio_SoundEntry* FindSoundEntry(const FString& SoundName);
    float GetCategoryVolume(EAudio_SoundCategory Category);
};
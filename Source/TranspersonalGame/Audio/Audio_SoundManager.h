#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Engine/World.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savanna     UMETA(DisplayName = "Savanna"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"),
    Triceratops UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct FAudio_SoundSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float Volume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float Pitch = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float AttenuationRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    bool bLooping = false;

    FAudio_SoundSettings()
    {
        Volume = 1.0f;
        Pitch = 1.0f;
        AttenuationRadius = 1000.0f;
        bLooping = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_SoundManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_SoundManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, TSoftObjectPtr<USoundCue>> BiomeAmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurType, TSoftObjectPtr<USoundCue>> DinosaurSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_SoundSettings DefaultSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayBiomeAmbient(EAudio_BiomeType BiomeType, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, const FAudio_SoundSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAllAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayFootstepSound(FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayEnvironmentalSound(const FString& SoundName, FVector Location, const FAudio_SoundSettings& Settings);

private:
    UPROPERTY()
    TArray<UAudioComponent*> ActiveAudioComponents;

    void CleanupFinishedAudioComponents();
    UAudioComponent* CreateAudioComponent(USoundCue* SoundCue, FVector Location, const FAudio_SoundSettings& Settings);
};
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"
#include "Audio_SoundManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana     UMETA(DisplayName = "Savana"),
    Pantano    UMETA(DisplayName = "Pantano"),
    Floresta   UMETA(DisplayName = "Floresta"),
    Deserto    UMETA(DisplayName = "Deserto"),
    Montanha   UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    TSoftObjectPtr<USoundCue> VocalizationSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float AttenuationRadius = 2000.0f;

    FAudio_SoundProfile()
    {
        VolumeMultiplier = 1.0f;
        AttenuationRadius = 2000.0f;
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

public:
    virtual void Tick(float DeltaTime) override;

    // Master audio control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Master")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Master")
    float EnvironmentVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Master")
    float DinosaurVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Master")
    float PlayerVolume = 0.7f;

    // Biome audio profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TMap<EAudio_BiomeType, FAudio_SoundProfile> BiomeAudioProfiles;

    // Dinosaur audio profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurType, FAudio_SoundProfile> DinosaurAudioProfiles;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<UAudioComponent*> BiomeAudioComponents;

    // Audio management functions
    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayBiomeAmbient(EAudio_BiomeType BiomeType, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void PlayDinosaurSound(EAudio_DinosaurType DinosaurType, FVector Location, bool bIsFootstep = false);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetMasterVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetEnvironmentVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void SetDinosaurVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void StopAllAmbientSounds();

    UFUNCTION(BlueprintCallable, Category = "Audio Management")
    void UpdatePlayerProximityAudio(FVector PlayerLocation);

private:
    // Internal audio management
    void InitializeBiomeAudio();
    void InitializeDinosaurAudio();
    void UpdateAudioLevels();

    // Current biome tracking
    EAudio_BiomeType CurrentBiome;
    float BiomeTransitionTimer;
    bool bIsTransitioningBiome;

    // Audio fade system
    void FadeAudioComponent(UAudioComponent* AudioComp, float TargetVolume, float FadeTime);
    
    // Proximity detection
    float LastProximityCheck;
    TArray<AActor*> NearbyDinosaurs;
};
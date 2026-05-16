#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "SharedTypes.h"
#include "Audio_MetaSoundSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_BiomeType : uint8
{
    Savana       UMETA(DisplayName = "Savana"),
    Pantano      UMETA(DisplayName = "Pantano"),
    Floresta     UMETA(DisplayName = "Floresta"),
    Deserto      UMETA(DisplayName = "Deserto"),
    Montanha     UMETA(DisplayName = "Montanha")
};

UENUM(BlueprintType)
enum class EAudio_DinosaurType : uint8
{
    TRex         UMETA(DisplayName = "T-Rex"),
    Raptor       UMETA(DisplayName = "Raptor"),
    Brachiosaurus UMETA(DisplayName = "Brachiosaurus"),
    Triceratops  UMETA(DisplayName = "Triceratops"),
    Pteranodon   UMETA(DisplayName = "Pteranodon")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_SoundData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString SoundName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float Volume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bIs3D;

    FAudio_SoundData()
    {
        SoundName = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        Volume = 1.0f;
        bIs3D = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_BiomeAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_SoundData> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FAudio_SoundData MusicTrack;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float FadeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    FVector BiomeCenter;

    FAudio_BiomeAmbience()
    {
        BiomeType = EAudio_BiomeType::Savana;
        FadeDistance = 5000.0f;
        BiomeCenter = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_MetaSoundSystem : public AActor
{
    GENERATED_BODY()

public:
    AAudio_MetaSoundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Core Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MasterAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* BiomeAmbienceComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DinosaurSFXComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* DialogueComponent;

    // Biome Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    TArray<FAudio_BiomeAmbience> BiomeAudioData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    EAudio_BiomeType CurrentBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Audio")
    float BiomeTransitionSpeed;

    // Dinosaur Audio System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurType, FAudio_SoundData> DinosaurRoars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TMap<EAudio_DinosaurType, FAudio_SoundData> DinosaurFootsteps;

    // Dialogue Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    TArray<FAudio_SoundData> TribalDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Audio")
    FAudio_SoundData CurrentDialogue;

    // Environmental Audio
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    FAudio_SoundData WeatherAmbience;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Audio")
    float EnvironmentalVolume;

public:
    // Biome Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateBiomeAudio(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TransitionToBiome(EAudio_BiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetBiomeAmbience(EAudio_BiomeType BiomeType, const FAudio_SoundData& AmbienceData);

    // Dinosaur Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurRoar(EAudio_DinosaurType DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurFootsteps(EAudio_DinosaurType DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDinosaurAudio(EAudio_DinosaurType DinosaurType, const FAudio_SoundData& RoarData, const FAudio_SoundData& FootstepData);

    // Dialogue Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayTribalDialogue(const FString& DialogueID, const FVector& SpeakerLocation);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void RegisterDialogueAudio(const FString& DialogueID, const FAudio_SoundData& DialogueData);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopCurrentDialogue();

    // Environmental Audio Functions
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateWeatherAudio(const FString& WeatherType, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void InitializeAudioDatabase();

private:
    // Internal Audio Management
    void UpdateAudioComponents();
    void CalculateBiomeFromLocation(const FVector& Location);
    void FadeAudioComponent(UAudioComponent* Component, float TargetVolume, float FadeTime);
    
    // Audio Database
    void LoadPreGeneratedAudio();
    void SetupBiomeAudioData();
    void SetupDinosaurAudioData();
    void SetupDialogueAudioData();

    // Current State
    float CurrentBiomeVolume;
    float TargetBiomeVolume;
    bool bIsTransitioning;
    float TransitionTimer;
};
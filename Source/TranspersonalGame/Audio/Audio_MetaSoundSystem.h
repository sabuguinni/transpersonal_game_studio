#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundWave.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "Audio_MetaSoundSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest      UMETA(DisplayName = "Forest"),
    Swamp       UMETA(DisplayName = "Swamp"),
    Plains      UMETA(DisplayName = "Plains"),
    Desert      UMETA(DisplayName = "Desert"),
    Mountain    UMETA(DisplayName = "Mountain")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Safe        UMETA(DisplayName = "Safe"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Lethal      UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float AmbientVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float WindIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float CreatureActivity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    EAudio_ThreatLevel ThreatLevel = EAudio_ThreatLevel::Safe;

    FAudio_EnvironmentSettings()
    {
        EnvironmentType = EAudio_EnvironmentType::Forest;
        AmbientVolume = 0.7f;
        WindIntensity = 0.5f;
        CreatureActivity = 0.3f;
        ThreatLevel = EAudio_ThreatLevel::Safe;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_DinosaurSoundProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TObjectPtr<USoundWave> IdleSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TObjectPtr<USoundWave> AlertSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TObjectPtr<USoundWave> AttackSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    TObjectPtr<USoundWave> FootstepSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Audio")
    float PitchVariation = 0.1f;

    FAudio_DinosaurSoundProfile()
    {
        DinosaurType = TEXT("Generic");
        IdleSound = nullptr;
        AlertSound = nullptr;
        AttackSound = nullptr;
        FootstepSound = nullptr;
        VolumeMultiplier = 1.0f;
        PitchVariation = 0.1f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Environment Audio Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurSound(const FString& DinosaurType, const FString& SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayNarrationAudio(const FString& AudioURL, float Volume = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetMasterVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StartAmbientLoop();

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void StopAmbientLoop();

    // MetaSound Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void UpdateMetaSoundParameters();

    UFUNCTION(BlueprintCallable, Category = "MetaSound")
    void SetMetaSoundParameter(const FString& ParameterName, float Value);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_EnvironmentSettings CurrentEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    TArray<FAudio_DinosaurSoundProfile> DinosaurSoundProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TObjectPtr<UMetaSoundSource> EnvironmentMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TObjectPtr<UMetaSoundSource> DinosaurMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Components")
    TObjectPtr<UAudioComponent> NarrationAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float MasterVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    float EnvironmentUpdateInterval = 2.0f;

private:
    float LastEnvironmentUpdate = 0.0f;
    bool bAmbientLoopActive = false;

    void InitializeAudioComponents();
    void LoadDinosaurSoundProfiles();
    void UpdateEnvironmentAudio();
    FAudio_DinosaurSoundProfile* GetDinosaurProfile(const FString& DinosaurType);
};
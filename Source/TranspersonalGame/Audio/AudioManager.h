#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/Engine.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "MetasoundSource.h"
#include "AudioManager.generated.h"

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Calm        UMETA(DisplayName = "Calm"),
    Tension     UMETA(DisplayName = "Tension"),
    Danger      UMETA(DisplayName = "Danger"),
    Terror      UMETA(DisplayName = "Terror"),
    Relief      UMETA(DisplayName = "Relief")
};

UENUM(BlueprintType)
enum class EEnvironmentType : uint8
{
    Forest      UMETA(DisplayName = "Dense Forest"),
    Clearing    UMETA(DisplayName = "Open Clearing"),
    Water       UMETA(DisplayName = "Near Water"),
    Cave        UMETA(DisplayName = "Cave System"),
    Cliff       UMETA(DisplayName = "Cliff Area")
};

USTRUCT(BlueprintType)
struct FTensionParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension")
    float ThreatProximity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension")
    float ThreatLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension")
    float PlayerHeartRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension")
    bool bIsHiding = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tension")
    bool bIsBeingHunted = false;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudioManager : public UObject
{
    GENERATED_BODY()

public:
    UAudioManager();

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void Initialize(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateEnvironmentType(EEnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void UpdateTensionParameters(const FTensionParameters& NewParameters);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void PlayDinosaurSound(FVector Location, FString DinosaurSpecies, FString SoundType);

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Audio Manager")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* MusicComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* AmbienceComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio Components")
    UAudioComponent* TensionComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* AdaptiveMusicMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* EnvironmentAmbienceMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MetaSounds")
    UMetaSoundSource* TensionSystemMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    EEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    EEnvironmentType CurrentEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "State")
    FTensionParameters CurrentTensionParams;

private:
    void UpdateMusicParameters();
    void UpdateAmbienceParameters();
    void UpdateTensionParameters();
    
    float CalculateTensionLevel();
    float CalculateMusicalIntensity();
    
    UWorld* GameWorld;
    float CurrentTimeOfDay;
    float CurrentWeatherIntensity;
};
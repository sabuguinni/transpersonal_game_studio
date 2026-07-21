#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Audio_EnvironmentalManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_EnvironmentType : uint8
{
    Forest UMETA(DisplayName = "Dense Forest"),
    River UMETA(DisplayName = "River Valley"),
    Plains UMETA(DisplayName = "Open Plains"),
    Volcanic UMETA(DisplayName = "Volcanic Region"),
    Cave UMETA(DisplayName = "Cave System"),
    Swamp UMETA(DisplayName = "Wetlands")
};

UENUM(BlueprintType)
enum class EAudio_TimeOfDay : uint8
{
    Dawn UMETA(DisplayName = "Dawn (5-7 AM)"),
    Morning UMETA(DisplayName = "Morning (7-11 AM)"),
    Midday UMETA(DisplayName = "Midday (11 AM-3 PM)"),
    Afternoon UMETA(DisplayName = "Afternoon (3-6 PM)"),
    Dusk UMETA(DisplayName = "Dusk (6-8 PM)"),
    Night UMETA(DisplayName = "Night (8 PM-5 AM)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_EnvironmentSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    EAudio_EnvironmentType EnvironmentType = EAudio_EnvironmentType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float BaseVolume = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float VariationIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float TransitionTime = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TArray<class USoundBase*> AmbientSounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    TArray<class USoundBase*> RandomEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Audio")
    float EventFrequency = 30.0f;

    FAudio_EnvironmentSettings()
    {
        EnvironmentType = EAudio_EnvironmentType::Forest;
        BaseVolume = 0.7f;
        VariationIntensity = 0.3f;
        TransitionTime = 5.0f;
        EventFrequency = 30.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_TimeBasedSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    EAudio_TimeOfDay TimeOfDay = EAudio_TimeOfDay::Morning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    float VolumeModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    float PitchModifier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Audio")
    TArray<class USoundBase*> SpecificSounds;

    FAudio_TimeBasedSettings()
    {
        TimeOfDay = EAudio_TimeOfDay::Morning;
        VolumeModifier = 1.0f;
        PitchModifier = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_EnvironmentalManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_EnvironmentalManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* PrimaryAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* SecondaryAmbientAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* EventAudio;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment Settings")
    FAudio_EnvironmentSettings CurrentEnvironment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time Settings")
    TArray<FAudio_TimeBasedSettings> TimeBasedSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
    float UpdateInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Manager Settings")
    float PlayerProximityRadius = 3000.0f;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_EnvironmentType ActiveEnvironment;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EAudio_TimeOfDay CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentIntensity = 1.0f;

    UFUNCTION(BlueprintCallable, Category = "Environment Control")
    void SetEnvironmentType(EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintCallable, Category = "Environment Control")
    void SetTimeOfDay(EAudio_TimeOfDay NewTimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Environment Control")
    void TriggerEnvironmentEvent();

    UFUNCTION(BlueprintCallable, Category = "Environment Control")
    void SetEnvironmentIntensity(float NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Environment Control")
    void StartEnvironmentTransition(EAudio_EnvironmentType TargetEnvironment);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environment Events")
    void OnEnvironmentChanged(EAudio_EnvironmentType OldEnvironment, EAudio_EnvironmentType NewEnvironment);

    UFUNCTION(BlueprintImplementableEvent, Category = "Environment Events")
    void OnTimeOfDayChanged(EAudio_TimeOfDay OldTime, EAudio_TimeOfDay NewTime);

    UFUNCTION()
    void UpdateEnvironmentalAudio();

    UFUNCTION()
    void TriggerRandomEvent();

    UFUNCTION()
    void CheckPlayerProximity();

public:
    virtual void Tick(float DeltaTime) override;

private:
    FTimerHandle UpdateTimerHandle;
    FTimerHandle EventTimerHandle;
    FTimerHandle ProximityTimerHandle;

    float TransitionTimer = 0.0f;
    EAudio_EnvironmentType TargetEnvironment;
    bool bInTransition = false;

    void InitializeTimeBasedSettings();
    void ApplyTimeBasedModifiers();
    void UpdateAmbientLayers();
    float CalculateDistanceToPlayer();
    void SetupRandomEventTimer();
};

#include "Audio_EnvironmentalManager.generated.h"
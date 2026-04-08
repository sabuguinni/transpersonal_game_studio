#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "MetasoundSource.h"
#include "AudioModulation.h"
#include "Components/AudioComponent.h"
#include "AudioSystemManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioStateChanged, FString, NewState);

UENUM(BlueprintType)
enum class EAudioEnvironmentState : uint8
{
    Forest_Day_Calm,
    Forest_Day_Tense,
    Forest_Night_Calm,
    Forest_Night_Danger,
    Cave_Exploration,
    Cave_Danger,
    Water_Shallow,
    Water_Deep,
    Open_Plains_Day,
    Open_Plains_Night,
    Storm_Approaching,
    Storm_Active
};

UENUM(BlueprintType)
enum class EThreatLevel : uint8
{
    Safe,
    Cautious,
    Alert,
    Danger,
    Combat
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UAudioSystemManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetEnvironmentState(EAudioEnvironmentState NewState);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetThreatLevel(EThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetTimeOfDay(float TimeOfDay); // 0.0 = midnight, 0.5 = noon

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void SetWeatherIntensity(float Intensity); // 0.0 = clear, 1.0 = storm

    // Adaptive Music System
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void TriggerMusicTransition(const FString& TargetState, float TransitionTime = 2.0f);

    // Dinosaur Audio Events
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayDinosaurCall(const FString& DinosaurSpecies, const FVector& Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void PlayFootstepSequence(const FString& SurfaceType, const FVector& Location, float Weight = 1.0f);

    // Environmental Audio
    UFUNCTION(BlueprintCallable, Category = "Audio System")
    void UpdateAmbientLayers();

    UPROPERTY(BlueprintAssignable)
    FOnAudioStateChanged OnAudioStateChanged;

protected:
    // Core MetaSound Assets
    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AdaptiveMusicMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> AmbientLayersMetaSound;

    UPROPERTY(EditDefaultsOnly, Category = "Audio Assets")
    TObjectPtr<UMetaSoundSource> DinosaurCallsMetaSound;

    // Audio Components
    UPROPERTY()
    TObjectPtr<UAudioComponent> MusicAudioComponent;

    UPROPERTY()
    TObjectPtr<UAudioComponent> AmbientAudioComponent;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudioEnvironmentState CurrentEnvironmentState;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentTimeOfDay;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    float CurrentWeatherIntensity;

private:
    void InitializeAudioComponents();
    void UpdateMusicParameters();
    void UpdateAmbientParameters();
    
    // Parameter names for MetaSounds
    static const FName PARAM_ThreatLevel;
    static const FName PARAM_TimeOfDay;
    static const FName PARAM_WeatherIntensity;
    static const FName PARAM_EnvironmentType;
};
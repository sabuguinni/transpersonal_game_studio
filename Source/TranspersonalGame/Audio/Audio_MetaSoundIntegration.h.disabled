#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundBase.h"
#include "MetasoundSource.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Actor.h"
#include "Audio_MetaSoundIntegration.generated.h"

UENUM(BlueprintType)
enum class EAudio_MetaSoundTrigger : uint8
{
    None UMETA(DisplayName = "None"),
    ProximityAlert UMETA(DisplayName = "Proximity Alert"),
    MedicalEmergency UMETA(DisplayName = "Medical Emergency"),
    EnvironmentalHazard UMETA(DisplayName = "Environmental Hazard"),
    ResourceDiscovery UMETA(DisplayName = "Resource Discovery"),
    WeatherWarning UMETA(DisplayName = "Weather Warning"),
    CombatAlert UMETA(DisplayName = "Combat Alert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_MetaSoundConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    FString ParameterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float DefaultValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float MinValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound")
    float MaxValue;

    FAudio_MetaSoundConfig()
    {
        ParameterName = TEXT("Intensity");
        DefaultValue = 0.5f;
        MinValue = 0.0f;
        MaxValue = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeAudioData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_MetaSoundTrigger TriggerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bIs3DPositional;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float MaxAudibleDistance;

    FAudio_NarrativeAudioData()
    {
        Duration = 0.0f;
        TriggerType = EAudio_MetaSoundTrigger::None;
        bIs3DPositional = true;
        MaxAudibleDistance = 2000.0f;
    }
};

/**
 * MetaSound Integration Component for Transpersonal Game
 * Bridges narrative dialogue system with UE5 MetaSounds for adaptive audio
 */
UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_MetaSoundIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_MetaSoundIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // MetaSound Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Integration")
    TArray<FAudio_MetaSoundConfig> MetaSoundConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Integration")
    TSoftObjectPtr<UMetaSoundSource> ProximityAlertMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Integration")
    TSoftObjectPtr<UMetaSoundSource> MedicalEmergencyMetaSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaSound Integration")
    TSoftObjectPtr<UMetaSoundSource> EnvironmentalHazardMetaSound;

    // Narrative Audio Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Integration")
    TArray<FAudio_NarrativeAudioData> NarrativeAudioLibrary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Integration")
    float NarrativeAudioCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Integration")
    bool bEnableNarrativeIntegration;

    // Audio Component Management
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> PrimaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> SecondaryAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    TObjectPtr<UAudioComponent> EnvironmentalAudioComponent;

    // MetaSound Control Functions
    UFUNCTION(BlueprintCallable, Category = "MetaSound Integration")
    void TriggerMetaSoundEvent(EAudio_MetaSoundTrigger TriggerType, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Integration")
    void SetMetaSoundParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Integration")
    void PlayNarrativeAudio(const FString& CharacterName, const FString& AudioURL, bool bInterrupt = false);

    UFUNCTION(BlueprintCallable, Category = "MetaSound Integration")
    void StopAllNarrativeAudio();

    UFUNCTION(BlueprintCallable, Category = "MetaSound Integration")
    bool IsNarrativeAudioPlaying() const;

    // Emergency Dialogue Integration
    UFUNCTION(BlueprintCallable, Category = "Emergency Integration")
    void OnEmergencyDialogueTriggered(EAudio_MetaSoundTrigger EmergencyType, float UrgencyLevel);

    UFUNCTION(BlueprintCallable, Category = "Emergency Integration")
    void RegisterNarrativeAudio(const FString& AudioURL, const FString& CharacterName, EAudio_MetaSoundTrigger TriggerType);

    // 3D Positional Audio
    UFUNCTION(BlueprintCallable, Category = "3D Audio")
    void Set3DAudioPosition(FVector WorldPosition);

    UFUNCTION(BlueprintCallable, Category = "3D Audio")
    void Enable3DAudio(bool bEnable);

    // Audio State Management
    UFUNCTION(BlueprintCallable, Category = "Audio State")
    void SetAudioIntensity(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Audio State")
    float GetCurrentAudioIntensity() const;

private:
    // Internal state
    float LastNarrativeAudioTime;
    bool bIsPlayingNarrativeAudio;
    FString CurrentNarrativeCharacter;
    EAudio_MetaSoundTrigger CurrentTriggerType;
    float CurrentAudioIntensity;

    // Internal functions
    void InitializeAudioComponents();
    void LoadMetaSoundAssets();
    UAudioComponent* GetAvailableAudioComponent();
    void UpdateAudioComponentSettings(UAudioComponent* AudioComp, const FAudio_NarrativeAudioData& AudioData);
};
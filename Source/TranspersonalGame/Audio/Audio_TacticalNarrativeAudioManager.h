#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "MetasoundSource.h"
#include "Kismet/GameplayStatics.h"
#include "Audio_TacticalNarrativeAudioManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_TacticalNarrativeContext : uint8
{
    MissionBriefing     UMETA(DisplayName = "Mission Briefing"),
    CombatInstruction   UMETA(DisplayName = "Combat Instruction"),
    HuntingGuidance     UMETA(DisplayName = "Hunting Guidance"),
    SurvivalTips        UMETA(DisplayName = "Survival Tips"),
    ThreatWarning       UMETA(DisplayName = "Threat Warning"),
    SquadCommunication  UMETA(DisplayName = "Squad Communication"),
    StorytellingMode    UMETA(DisplayName = "Storytelling Mode"),
    EnvironmentalCue    UMETA(DisplayName = "Environmental Cue")
};

UENUM(BlueprintType)
enum class EAudio_ThreatLevel : uint8
{
    Peaceful    UMETA(DisplayName = "Peaceful"),
    Cautious    UMETA(DisplayName = "Cautious"),
    Alert       UMETA(DisplayName = "Alert"),
    Dangerous   UMETA(DisplayName = "Dangerous"),
    Critical    UMETA(DisplayName = "Critical"),
    Lethal      UMETA(DisplayName = "Lethal")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_NarrativeAudioProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    EAudio_TacticalNarrativeContext PrimaryContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<USoundCue> VoiceCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    TSoftObjectPtr<UMetaSoundSource> MetaSoundAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float AudioVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    float AudioPitch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Audio")
    bool bSpatialAudio;

    FAudio_NarrativeAudioProfile()
    {
        CharacterName = TEXT("Unknown");
        PrimaryContext = EAudio_TacticalNarrativeContext::MissionBriefing;
        AudioVolume = 1.0f;
        AudioPitch = 1.0f;
        bSpatialAudio = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_TacticalAudioTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    EAudio_TacticalNarrativeContext TriggerContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    EAudio_ThreatLevel RequiredThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    TArray<FString> ContextualDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Trigger")
    bool bIsActive;

    FAudio_TacticalAudioTrigger()
    {
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        TriggerContext = EAudio_TacticalNarrativeContext::MissionBriefing;
        RequiredThreatLevel = EAudio_ThreatLevel::Peaceful;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_TacticalNarrativeAudioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_TacticalNarrativeAudioManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Tactical Narrative Audio Management
    UFUNCTION(BlueprintCallable, Category = "Tactical Audio")
    void PlayTacticalNarrative(EAudio_TacticalNarrativeContext Context, const FString& CharacterName);

    UFUNCTION(BlueprintCallable, Category = "Tactical Audio")
    void PlayMissionBriefing(const FString& MissionType, EAudio_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical Audio")
    void PlayCombatInstruction(const FString& TacticType, const FString& TargetSpecies);

    UFUNCTION(BlueprintCallable, Category = "Tactical Audio")
    void PlaySquadCommunication(const FString& Message, const FString& SenderName);

    UFUNCTION(BlueprintCallable, Category = "Tactical Audio")
    void PlayEnvironmentalCue(EAudio_ThreatLevel ThreatLevel, const FVector& ThreatLocation);

    // MetaSounds Integration
    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void PlayMetaSoundNarrative(UMetaSoundSource* MetaSoundAsset, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "MetaSounds")
    void UpdateMetaSoundParameters(const FString& ParameterName, float Value);

    // Audio Trigger System
    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void RegisterAudioTrigger(const FAudio_TacticalAudioTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void CheckAudioTriggers(const FVector& PlayerLocation, EAudio_ThreatLevel CurrentThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Audio Triggers")
    void ActivateNarrativeZone(const FString& ZoneName, bool bActivate);

    // Adaptive Audio System
    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void UpdateAudioContext(EAudio_TacticalNarrativeContext NewContext);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void SetThreatLevel(EAudio_ThreatLevel NewThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void FadeToNarrativeMode(float FadeTime = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Audio")
    void FadeToGameplayMode(float FadeTime = 2.0f);

protected:
    // Audio Profiles
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_NarrativeAudioProfile> NarrativeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Configuration")
    TArray<FAudio_TacticalAudioTrigger> AudioTriggers;

    // Current Audio State
    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_TacticalNarrativeContext CurrentContext;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    EAudio_ThreatLevel CurrentThreatLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Audio State")
    bool bIsNarrativeMode;

    // Audio Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* NarrativeAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Components")
    class UAudioComponent* MetaSoundComponent;

    // Audio Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TSoftObjectPtr<USoundCue> DefaultNarrativeCue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Assets")
    TSoftObjectPtr<UMetaSoundSource> DefaultMetaSound;

private:
    void InitializeAudioComponents();
    void LoadAudioAssets();
    FAudio_NarrativeAudioProfile* FindAudioProfile(const FString& CharacterName, EAudio_TacticalNarrativeContext Context);
    void ProcessAudioTrigger(const FAudio_TacticalAudioTrigger& Trigger);
    void UpdateSpatialAudio(const FVector& ListenerLocation);
};
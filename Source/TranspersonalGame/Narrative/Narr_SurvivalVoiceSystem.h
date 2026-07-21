#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "TranspersonalGame/SharedTypes.h"
#include "Narr_SurvivalVoiceSystem.generated.h"

USTRUCT(BlueprintType)
struct FNarr_SurvivalVoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString VoiceLineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    TSoftObjectPtr<USoundCue> AudioAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    ENarr_SurvivalContext TriggerContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    float Cooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice")
    bool bIsEmergency;

    FNarr_SurvivalVoiceLine()
    {
        VoiceLineID = TEXT("");
        DialogueText = TEXT("");
        TriggerContext = ENarr_SurvivalContext::General;
        Priority = 1.0f;
        Cooldown = 30.0f;
        bIsEmergency = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_VoiceContextData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    ENarr_SurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FNarr_SurvivalVoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    float LastTriggeredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 TimesTriggered;

    FNarr_VoiceContextData()
    {
        Context = ENarr_SurvivalContext::General;
        LastTriggeredTime = 0.0f;
        TimesTriggered = 0;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSurvivalVoiceTriggered, const FString&, VoiceLineID, const FString&, DialogueText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEmergencyVoiceTriggered, const FString&, VoiceLineID, const FString&, DialogueText, ENarr_SurvivalContext, Context);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_SurvivalVoiceSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_SurvivalVoiceSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Voice line management
    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void TriggerVoiceLineByContext(ENarr_SurvivalContext Context, float Priority = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void TriggerEmergencyVoiceLine(ENarr_SurvivalContext Context, const FString& CustomText = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void TriggerVoiceLineByID(const FString& VoiceLineID);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void StopCurrentVoiceLine();

    // Context monitoring
    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void UpdateSurvivalContext(float Health, float Hunger, float Thirst, float Stamina, float Fear);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void RegisterThreatDetection(ENarr_ThreatLevel ThreatLevel, const FString& ThreatDescription);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void RegisterResourceDiscovery(ENarr_ResourceType ResourceType, int32 Quantity);

    // Voice line database
    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void LoadVoiceLineDatabase();

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void AddVoiceLine(const FNarr_SurvivalVoiceLine& NewVoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    bool RemoveVoiceLine(const FString& VoiceLineID);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void SetVoiceVolume(float Volume);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    void SetVoicePitch(float Pitch);

    UFUNCTION(BlueprintCallable, Category = "Survival Voice")
    bool IsVoicePlaying() const;

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Survival Voice")
    FOnSurvivalVoiceTriggered OnSurvivalVoiceTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Survival Voice")
    FOnEmergencyVoiceTriggered OnEmergencyVoiceTriggered;

protected:
    // Voice line database
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Database")
    TMap<ENarr_SurvivalContext, FNarr_VoiceContextData> VoiceContextDatabase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Database")
    TMap<FString, FNarr_SurvivalVoiceLine> VoiceLineDatabase;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    UAudioComponent* EmergencyVoiceAudioComponent;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FString CurrentVoiceLineID;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_SurvivalContext CurrentContext;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsPlayingVoice;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastVoiceTime;

    // Survival state monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    float CurrentHealth;

    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    float CurrentHunger;

    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    float CurrentThirst;

    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    float CurrentStamina;

    UPROPERTY(BlueprintReadOnly, Category = "Survival State")
    float CurrentFear;

    // Thresholds for voice triggers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalHealthThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalHungerThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalThirstThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalStaminaThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float CriticalFearThreshold;

    // Voice timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float VoiceCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float EmergencyVoiceCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ContextUpdateInterval;

    // Timer handles
    FTimerHandle VoiceFinishedTimerHandle;
    FTimerHandle ContextUpdateTimerHandle;

private:
    // Internal methods
    void InitializeVoiceDatabase();
    void InitializeAudioComponents();
    void UpdateContextualVoices(float DeltaTime);
    void CheckSurvivalThresholds();
    void PlayVoiceLine(const FNarr_SurvivalVoiceLine& VoiceLine, bool bIsEmergency = false);
    FNarr_SurvivalVoiceLine* FindBestVoiceLineForContext(ENarr_SurvivalContext Context);
    bool CanTriggerVoiceForContext(ENarr_SurvivalContext Context) const;
    void OnVoiceFinished();
    ENarr_SurvivalContext DeterminePrimaryContext() const;
    void CreateDefaultVoiceLines();
};
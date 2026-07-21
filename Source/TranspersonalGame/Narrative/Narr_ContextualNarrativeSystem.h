#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "NarrativeTypes.h"
#include "Narr_VoicelineDatabase.h"
#include "Narr_ContextualNarrativeSystem.generated.h"

class ATranspersonalCharacter;
class AActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeTriggered, FString, VoicelineID, FString, CharacterName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNarrativeCompleted, FString, VoicelineID);

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ContextualTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString TriggerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float LastTriggeredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 MaxTriggerCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 CurrentTriggerCount;

    FNarr_ContextualTrigger()
    {
        TriggerID = "";
        TriggerCondition = "";
        TriggerRadius = 1000.0f;
        TriggerLocation = FVector::ZeroVector;
        bIsActive = true;
        CooldownTime = 30.0f;
        LastTriggeredTime = -1.0f;
        MaxTriggerCount = -1; // -1 = unlimited
        CurrentTriggerCount = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_ActiveNarrative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString VoicelineID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float StartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    bool bIsPlaying;

    FNarr_ActiveNarrative()
    {
        VoicelineID = "";
        AudioComponent = nullptr;
        StartTime = 0.0f;
        Duration = 0.0f;
        bIsPlaying = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_ContextualNarrativeSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_ContextualNarrativeSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Core narrative functions
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void InitializeNarrativeSystem();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool TriggerNarrative(const FString& TriggerCondition, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateNarrativeSystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StopCurrentNarrative();

    // Trigger management
    UFUNCTION(BlueprintCallable, Category = "Triggers")
    void RegisterContextualTrigger(const FNarr_ContextualTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Triggers")
    void UnregisterContextualTrigger(const FString& TriggerID);

    UFUNCTION(BlueprintCallable, Category = "Triggers")
    bool CheckTriggerConditions(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Triggers")
    void SetTriggerActive(const FString& TriggerID, bool bActive);

    // Dinosaur proximity detection
    UFUNCTION(BlueprintCallable, Category = "Detection")
    bool IsPlayerNearDinosaur(const FVector& PlayerLocation, const FString& DinosaurType, float MaxDistance = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Detection")
    TArray<AActor*> GetNearbyDinosaurs(const FVector& PlayerLocation, float SearchRadius = 3000.0f);

    // Audio management
    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool PlayVoiceline(const FString& VoicelineID, const FString& AudioURL, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopVoiceline(const FString& VoicelineID);

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNarrativeTriggered OnNarrativeTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNarrativeCompleted OnNarrativeCompleted;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float NarrativeVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    bool bEnableContextualNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
    float MinTimeBetweenNarratives;

protected:
    // Internal data
    UPROPERTY()
    UNarr_VoicelineDatabase* VoicelineDatabase;

    UPROPERTY()
    TMap<FString, FNarr_ContextualTrigger> ContextualTriggers;

    UPROPERTY()
    TArray<FNarr_ActiveNarrative> ActiveNarratives;

    UPROPERTY()
    float LastNarrativeTime;

    UPROPERTY()
    bool bIsInitialized;

    // Internal functions
    void SetupDefaultTriggers();
    void CleanupFinishedNarratives();
    FString GetDinosaurTypeFromActor(AActor* Actor);
    bool CanTriggerNarrative(const FNarr_ContextualTrigger& Trigger, float CurrentTime);
    void OnAudioFinished(UAudioComponent* AudioComponent);
};
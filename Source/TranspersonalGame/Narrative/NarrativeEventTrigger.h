#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "NarrativeEventTrigger.generated.h"

UENUM(BlueprintType)
enum class ENarr_TriggerType : uint8
{
    LocationBased UMETA(DisplayName = "Location Based"),
    SurvivalStatus UMETA(DisplayName = "Survival Status"),
    DinosaurEncounter UMETA(DisplayName = "Dinosaur Encounter"),
    ResourceDiscovery UMETA(DisplayName = "Resource Discovery"),
    TimeOfDay UMETA(DisplayName = "Time of Day"),
    WeatherChange UMETA(DisplayName = "Weather Change")
};

USTRUCT(BlueprintType)
struct FNarr_TriggerCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    ENarr_TriggerType TriggerType = ENarr_TriggerType::LocationBased;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float HealthThreshold = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float HungerThreshold = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float FearThreshold = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FString RequiredDinosaurType = TEXT("TRex");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bOnceOnly = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bRequiresLineOfSight = false;

    FNarr_TriggerCondition()
    {
        TriggerType = ENarr_TriggerType::LocationBased;
        HealthThreshold = 50.0f;
        HungerThreshold = 70.0f;
        FearThreshold = 60.0f;
        RequiredDinosaurType = TEXT("TRex");
        TriggerRadius = 2000.0f;
        bOnceOnly = true;
        bRequiresLineOfSight = false;
    }
};

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarrativeEventTrigger : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarrativeEventTrigger();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    FNarr_TriggerCondition TriggerCondition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    FString NarrativeEventID = TEXT("DefaultEvent");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    FString DialogueText = TEXT("Something stirs in the ancient lands...");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    FString SpeakerName = TEXT("AncientNarrator");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    float AudioDuration = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    bool bHasTriggered = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    float LastCheckTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative Trigger")
    float CheckInterval = 1.0f;

    // Event delegates
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNarrativeTriggered, FString, EventID, FString, DialogueText);
    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnNarrativeTriggered OnNarrativeTriggered;

    // Core functions
    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    bool CheckTriggerConditions();

    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    void TriggerNarrativeEvent();

    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    void ResetTrigger();

    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    bool IsPlayerInRange();

    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    bool CheckSurvivalStatus();

    UFUNCTION(BlueprintCallable, Category = "Narrative Trigger")
    bool CheckDinosaurPresence();

private:
    class ATranspersonalCharacter* GetPlayerCharacter();
    float GetDistanceToPlayer();
    bool HasLineOfSightToPlayer();
};
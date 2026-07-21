#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Narr_CrowdMissionDialogue.generated.h"

UENUM(BlueprintType)
enum class ENarr_CrowdMissionType : uint8
{
    Stealth         UMETA(DisplayName = "Stealth Mission"),
    Infiltration    UMETA(DisplayName = "Infiltration Mission"),
    Following       UMETA(DisplayName = "Following Mission"),
    Escape          UMETA(DisplayName = "Escape Mission"),
    Observation     UMETA(DisplayName = "Observation Mission")
};

UENUM(BlueprintType)
enum class ENarr_DetectionLevel : uint8
{
    Hidden          UMETA(DisplayName = "Completely Hidden"),
    Suspicious      UMETA(DisplayName = "Slightly Suspicious"),
    Noticed         UMETA(DisplayName = "Noticed by Guards"),
    Detected        UMETA(DisplayName = "Fully Detected"),
    Hunted          UMETA(DisplayName = "Actively Hunted")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CrowdMissionDialogue : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ENarr_CrowdMissionType MissionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    ENarr_DetectionLevel TriggerDetectionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString AudioPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float DialogueDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bIsUrgent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    bool bShowOnHUD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FString SpeakerName;

    FNarr_CrowdMissionDialogue()
    {
        MissionType = ENarr_CrowdMissionType::Stealth;
        TriggerDetectionLevel = ENarr_DetectionLevel::Hidden;
        DialogueText = TEXT("Default mission dialogue");
        AudioPath = TEXT("");
        DialogueDuration = 5.0f;
        bIsUrgent = false;
        bShowOnHUD = true;
        SpeakerName = TEXT("Narrator");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_MissionContext
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString SettlementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString ThreatDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    FString ObjectiveReason;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    TArray<FString> ConsequencesOfDetection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    int32 CrowdDensityRequired;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Context")
    bool bNightTimePreferred;

    FNarr_MissionContext()
    {
        SettlementName = TEXT("Ancient Settlement");
        ThreatDescription = TEXT("Hostile guards patrol the area");
        ObjectiveReason = TEXT("Gather information");
        CrowdDensityRequired = 5;
        bNightTimePreferred = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_CrowdMissionDialogue : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_CrowdMissionDialogue();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission dialogue management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerMissionDialogue(ENarr_CrowdMissionType MissionType, ENarr_DetectionLevel DetectionLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void PlayDetectionWarning(ENarr_DetectionLevel NewDetectionLevel);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SetMissionContext(const FNarr_MissionContext& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CrowdMissionDialogue GetDialogueForSituation(ENarr_CrowdMissionType MissionType, ENarr_DetectionLevel DetectionLevel);

    // Mission briefing system
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void StartMissionBriefing(ENarr_CrowdMissionType MissionType);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void UpdateMissionStatus(const FString& StatusMessage, bool bIsSuccess);

    // Audio integration
    UFUNCTION(BlueprintCallable, Category = "Audio")
    void PlayDialogueAudio(const FString& AudioPath);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopCurrentDialogue();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue Data")
    class UDataTable* MissionDialogueTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
    FNarr_MissionContext CurrentMissionContext;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_DetectionLevel CurrentDetectionLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    ENarr_CrowdMissionType ActiveMissionType;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bMissionActive;

    UPROPERTY(BlueprintReadOnly, Category = "Audio")
    bool bDialoguePlaying;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float DialogueCooldownTime;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float LastDialogueTime;

private:
    void InitializeDialogueTable();
    bool CanPlayDialogue() const;
    void LogMissionEvent(const FString& EventDescription);
};
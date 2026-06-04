#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../SharedTypes.h"
#include "Narr_TacticalNarrativeIntegration.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TacticalMissionBriefing
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString MissionName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString BriefingText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    FString ObjectiveDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    ENarr_ThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    TArray<FString> SquadRoleRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission Briefing")
    float EstimatedDuration;

    FNarr_TacticalMissionBriefing()
    {
        MissionName = TEXT("Unknown Mission");
        BriefingText = TEXT("Mission briefing not available");
        ObjectiveDescription = TEXT("No objectives defined");
        ThreatLevel = ENarr_ThreatLevel::Moderate;
        EstimatedDuration = 300.0f; // 5 minutes default
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TacticalNPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    ENarr_NPCRole TacticalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    TArray<FString> SpecializedDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    float ExperienceLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Profile")
    bool bIsAvailableForMissions;

    FNarr_TacticalNPCProfile()
    {
        NPCName = TEXT("Unnamed NPC");
        TacticalRole = ENarr_NPCRole::Support;
        BackgroundStory = TEXT("No background available");
        ExperienceLevel = 1.0f;
        bIsAvailableForMissions = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_TacticalNarrativeIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TacticalNarrativeIntegration();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Narrative")
    TArray<FNarr_TacticalMissionBriefing> MissionBriefings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Narrative")
    TArray<FNarr_TacticalNPCProfile> TacticalNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Narrative")
    FString CurrentMissionContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Narrative")
    bool bMissionBriefingActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactical Narrative")
    float MissionNarrativeTimer;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Mission Briefing Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void StartMissionBriefing(const FString& MissionName, ENarr_ThreatLevel ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void EndMissionBriefing();

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    FNarr_TacticalMissionBriefing GetMissionBriefing(const FString& MissionName);

    // NPC Integration Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void RegisterTacticalNPC(const FNarr_TacticalNPCProfile& NPCProfile);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    TArray<FNarr_TacticalNPCProfile> GetAvailableNPCs(ENarr_NPCRole RequiredRole);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    FString GetNPCDialogue(const FString& NPCName, const FString& Context);

    // Mission Context Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void UpdateMissionContext(const FString& NewContext);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void TriggerMissionNarrative(const FString& NarrativeEvent);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    bool IsMissionNarrativeActive() const;

    // Squad Communication Functions
    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void BroadcastSquadMessage(const FString& Message, ENarr_NPCRole SenderRole);

    UFUNCTION(BlueprintCallable, Category = "Tactical Narrative")
    void HandleSquadStatusUpdate(const FString& StatusMessage);

private:
    void InitializeMissionBriefings();
    void InitializeTacticalNPCs();
    void ProcessMissionNarrative(float DeltaTime);
    FString GenerateContextualDialogue(const FNarr_TacticalNPCProfile& NPC, const FString& Context);
};
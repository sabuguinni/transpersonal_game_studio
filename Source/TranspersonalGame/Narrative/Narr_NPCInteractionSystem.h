#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "NarrativeTypes.h"
#include "Narr_NPCInteractionSystem.generated.h"

class ATranspersonalCharacter;
class USoundBase;

UENUM(BlueprintType)
enum class ENarr_NPCInteractionType : uint8
{
    None,
    Trade,
    Information,
    Warning,
    Emergency,
    Casual,
    Hostile
};

UENUM(BlueprintType)
enum class ENarr_NPCMoodState : uint8
{
    Neutral,
    Friendly,
    Cautious,
    Fearful,
    Aggressive,
    Panicked,
    Curious
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCDialogueEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NPCInteractionType InteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NPCMoodState RequiredMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TriggerDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRequiresLineOfSight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VoicelineID;

    FNarr_NPCDialogueEntry()
    {
        DialogueText = TEXT("");
        InteractionType = ENarr_NPCInteractionType::None;
        RequiredMood = ENarr_NPCMoodState::Neutral;
        TriggerDistance = 500.0f;
        bRequiresLineOfSight = true;
        VoicelineID = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_NPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString NPCRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENarr_NPCMoodState DefaultMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FNarr_NPCDialogueEntry> DialogueOptions;

    FNarr_NPCPersonality()
    {
        NPCName = TEXT("Unknown");
        NPCRole = TEXT("Survivor");
        DefaultMood = ENarr_NPCMoodState::Neutral;
        AggressionLevel = 0.5f;
        FearThreshold = 0.7f;
        TrustLevel = 0.3f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_NPCInteractionSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_NPCInteractionSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // NPC Interaction Management
    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void InitializeNPCPersonality(const FNarr_NPCPersonality& Personality);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    bool CanInteractWithPlayer(ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    FNarr_NPCDialogueEntry GetBestDialogueOption(ATranspersonalCharacter* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void TriggerInteraction(ATranspersonalCharacter* Player, ENarr_NPCInteractionType InteractionType);

    // Mood and Relationship System
    UFUNCTION(BlueprintCallable, Category = "NPC Mood")
    void UpdateMoodBasedOnEvents(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Mood")
    void SetMoodState(ENarr_NPCMoodState NewMood);

    UFUNCTION(BlueprintCallable, Category = "NPC Mood")
    ENarr_NPCMoodState GetCurrentMood() const { return CurrentMood; }

    UFUNCTION(BlueprintCallable, Category = "NPC Mood")
    void ModifyTrustLevel(float TrustChange);

    // Environmental Response System
    UFUNCTION(BlueprintCallable, Category = "NPC Environment")
    void RespondToDinosaurThreat(AActor* DinosaurActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Environment")
    void RespondToWeatherChange(float Intensity);

    UFUNCTION(BlueprintCallable, Category = "NPC Environment")
    void RespondToPlayerActions(ATranspersonalCharacter* Player, const FString& ActionType);

    // Social Dynamics
    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ShareInformationWithNearbyNPCs(const FString& Information, float ShareRadius);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void ReceiveInformationFromNPC(UNarr_NPCInteractionSystem* SourceNPC, const FString& Information);

    UFUNCTION(BlueprintCallable, Category = "NPC Social")
    void FormGroupWithNearbyNPCs(float GroupRadius);

    // Audio Integration
    UFUNCTION(BlueprintCallable, Category = "NPC Audio")
    void PlayVoiceline(const FString& VoicelineID);

    UFUNCTION(BlueprintCallable, Category = "NPC Audio")
    void PlayEmergencyAlert(const FString& AlertMessage);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    FNarr_NPCPersonality NPCPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    ENarr_NPCMoodState CurrentMood;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float InteractionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float MoodUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Settings")
    float SocialShareRadius;

    // Internal State
    UPROPERTY()
    float LastMoodUpdate;

    UPROPERTY()
    TArray<AActor*> NearbyNPCs;

    UPROPERTY()
    TArray<FString> SharedInformation;

    UPROPERTY()
    bool bInGroup;

    UPROPERTY()
    float CurrentTrustLevel;

    // Helper Functions
    bool IsPlayerInRange(ATranspersonalCharacter* Player) const;
    bool HasLineOfSight(ATranspersonalCharacter* Player) const;
    float CalculatePlayerThreatLevel(ATranspersonalCharacter* Player) const;
    void UpdateNearbyNPCs();
    ENarr_NPCMoodState CalculateNewMood(float DeltaTime);
};
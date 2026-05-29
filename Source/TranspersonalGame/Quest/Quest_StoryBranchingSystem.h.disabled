#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/TriggerBox.h"
#include "Quest_StoryBranchingSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_StoryBranch : uint8
{
    None                    UMETA(DisplayName = "No Branch"),
    CaveExploration        UMETA(DisplayName = "Cave Exploration"),
    PredatorConfrontation  UMETA(DisplayName = "Predator Confrontation"),
    PeacefulGathering      UMETA(DisplayName = "Peaceful Gathering"),
    TribalEncounter        UMETA(DisplayName = "Tribal Encounter"),
    AncientDiscovery       UMETA(DisplayName = "Ancient Discovery"),
    SurvivalChallenge      UMETA(DisplayName = "Survival Challenge")
};

UENUM(BlueprintType)
enum class EQuest_StoryConsequence : uint8
{
    None                UMETA(DisplayName = "No Consequence"),
    GainAlly           UMETA(DisplayName = "Gain Ally"),
    LoseResource       UMETA(DisplayName = "Lose Resource"),
    UnlockArea         UMETA(DisplayName = "Unlock Area"),
    TriggerEvent       UMETA(DisplayName = "Trigger Event"),
    ChangeReputation   UMETA(DisplayName = "Change Reputation"),
    AlterEnvironment   UMETA(DisplayName = "Alter Environment")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_StoryChoice
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    FString ChoiceText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    FString ChoiceDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    EQuest_StoryBranch BranchType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    EQuest_StoryConsequence Consequence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    float DifficultyModifier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Choice")
    int32 RequiredSurvivalLevel;

    FQuest_StoryChoice()
    {
        ChoiceText = TEXT("");
        ChoiceDescription = TEXT("");
        BranchType = EQuest_StoryBranch::None;
        Consequence = EQuest_StoryConsequence::None;
        DifficultyModifier = 1.0f;
        RequiredSurvivalLevel = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_StoryBranchData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    FString BranchName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    FString BranchDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    EQuest_StoryBranch BranchType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    TArray<FQuest_StoryChoice> AvailableChoices;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branch")
    bool bIsCompleted;

    FQuest_StoryBranchData()
    {
        BranchName = TEXT("");
        BranchDescription = TEXT("");
        BranchType = EQuest_StoryBranch::None;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bIsActive = true;
        bIsCompleted = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_StoryBranchingSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_StoryBranchingSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branching")
    TArray<FQuest_StoryBranchData> StoryBranches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branching")
    EQuest_StoryBranch CurrentBranch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branching")
    TArray<EQuest_StoryBranch> CompletedBranches;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branching")
    int32 PlayerReputationScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Branching")
    TMap<EQuest_StoryBranch, float> BranchCooldowns;

public:
    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void InitializeStoryBranches();

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    bool CanActivateBranch(EQuest_StoryBranch BranchType) const;

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void ActivateStoryBranch(EQuest_StoryBranch BranchType);

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void CompleteStoryBranch(EQuest_StoryBranch BranchType, const FQuest_StoryChoice& ChosenOption);

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    TArray<FQuest_StoryChoice> GetAvailableChoices(EQuest_StoryBranch BranchType) const;

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void ProcessStoryChoice(const FQuest_StoryChoice& Choice);

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void UpdateBranchCooldowns(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    FString GetBranchNarrative(EQuest_StoryBranch BranchType) const;

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    void ModifyPlayerReputation(int32 ReputationChange);

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    bool HasCompletedBranch(EQuest_StoryBranch BranchType) const;

    UFUNCTION(BlueprintCallable, Category = "Story Branching")
    float GetBranchDifficulty(EQuest_StoryBranch BranchType) const;

private:
    void CreateDefaultStoryBranches();
    void ApplyStoryConsequence(EQuest_StoryConsequence Consequence, float Magnitude);
    bool ValidateStoryChoice(const FQuest_StoryChoice& Choice) const;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_StoryBranchTrigger : public ATriggerBox
{
    GENERATED_BODY()

public:
    AQuest_StoryBranchTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    EQuest_StoryBranch AssociatedBranch;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    FString TriggerNarrative;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bOneTimeUse;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    bool bHasBeenTriggered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story Trigger")
    float ActivationDelay;

public:
    UFUNCTION()
    void OnTriggerEnter(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION()
    void OnTriggerExit(class AActor* OverlappedActor, class AActor* OtherActor);

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    void ActivateStoryBranch();

    UFUNCTION(BlueprintCallable, Category = "Story Trigger")
    bool CanTrigger() const;

private:
    void SetupTriggerEvents();
    void DisplayStoryPrompt();
};
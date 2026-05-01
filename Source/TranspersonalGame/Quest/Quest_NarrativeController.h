#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Quest_NarrativeController.generated.h"

UENUM(BlueprintType)
enum class EQuest_NarrativeType : uint8
{
    MainStory       UMETA(DisplayName = "Main Story"),
    SideQuest       UMETA(DisplayName = "Side Quest"),
    RescueMission   UMETA(DisplayName = "Rescue Mission"),
    SurvivalTask    UMETA(DisplayName = "Survival Task"),
    ExplorationQuest UMETA(DisplayName = "Exploration Quest")
};

UENUM(BlueprintType)
enum class EQuest_EmotionalTone : uint8
{
    Desperate       UMETA(DisplayName = "Desperate"),
    Hopeful         UMETA(DisplayName = "Hopeful"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Determined      UMETA(DisplayName = "Determined"),
    Cautious        UMETA(DisplayName = "Cautious")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NarrativeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EQuest_NarrativeType NarrativeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    EQuest_EmotionalTone EmotionalTone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    TArray<FString> RequiredItems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Narrative")
    float RewardValue;

    FQuest_NarrativeData()
    {
        QuestTitle = TEXT("Untitled Quest");
        QuestDescription = TEXT("No description");
        DialogueText = TEXT("No dialogue");
        NarrativeType = EQuest_NarrativeType::SurvivalTask;
        EmotionalTone = EQuest_EmotionalTone::Cautious;
        UrgencyLevel = 0.5f;
        TargetLocation = FVector::ZeroVector;
        RewardValue = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivorProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    FString SurvivorName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    FString BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    float TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    float FearLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    TArray<FString> KnownInformation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    TArray<FQuest_NarrativeData> AvailableQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    bool bIsRescued;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survivor")
    FVector LastKnownLocation;

    FQuest_SurvivorProfile()
    {
        SurvivorName = TEXT("Unknown Survivor");
        BackgroundStory = TEXT("A fellow human trying to survive");
        TrustLevel = 0.5f;
        FearLevel = 0.7f;
        bIsRescued = false;
        LastKnownLocation = FVector::ZeroVector;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_NarrativeController : public AActor
{
    GENERATED_BODY()

public:
    AQuest_NarrativeController();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    TArray<FQuest_NarrativeData> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    TArray<FQuest_SurvivorProfile> SurvivorProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    int32 MaxActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    float QuestGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Narrative")
    float PlayerProximityThreshold;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void InitializeNarrativeSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    FQuest_NarrativeData CreateSurvivalQuest(EQuest_NarrativeType QuestType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void AddSurvivorProfile(const FQuest_SurvivorProfile& NewSurvivor);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    bool StartQuest(const FQuest_NarrativeData& QuestData);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void CompleteQuest(const FString& QuestTitle);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    TArray<FQuest_NarrativeData> GetAvailableQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    FQuest_SurvivorProfile GetSurvivorByName(const FString& SurvivorName) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void UpdateSurvivorTrust(const FString& SurvivorName, float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void GenerateEmergencyQuest(const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    FString GetContextualDialogue(const FString& SurvivorName, EQuest_EmotionalTone Tone) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    bool IsQuestAvailable(const FString& QuestTitle) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Narrative")
    void UpdateQuestProgress(const FString& QuestTitle, float ProgressAmount);

private:
    FTimerHandle QuestGenerationTimer;
    
    void GenerateRandomQuest();
    FQuest_NarrativeData CreateRescueQuest(const FVector& Location);
    FQuest_NarrativeData CreateExplorationQuest(const FVector& Location);
    FQuest_NarrativeData CreateResourceQuest(const FVector& Location);
    
    FString GenerateQuestDialogue(EQuest_NarrativeType QuestType, EQuest_EmotionalTone Tone) const;
    void CheckPlayerProximityToSurvivors();
};
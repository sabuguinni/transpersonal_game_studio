#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_DinosaurHuntManager.generated.h"

// Forward declarations
class ATranspersonalCharacter;

USTRUCT(BlueprintType)
struct FQuest_DinosaurHuntData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    EDinosaurSpecies TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt")
    FString Description;

    FQuest_DinosaurHuntData()
    {
        TargetSpecies = EDinosaurSpecies::Raptor;
        RequiredKills = 1;
        CurrentKills = 0;
        TimeLimit = 600.0f; // 10 minutes
        ElapsedTime = 0.0f;
        bIsActive = false;
        QuestName = TEXT("Hunt Quest");
        Description = TEXT("Hunt target dinosaur");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DinosaurHuntManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_DinosaurHuntManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quest management
    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void StartHuntQuest(EDinosaurSpecies Species, int32 KillCount, float TimeLimit, const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CompleteHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CancelHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void RegisterDinosaurKill(EDinosaurSpecies Species, ATranspersonalCharacter* Hunter);

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    bool IsHuntQuestActive() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    FQuest_DinosaurHuntData GetCurrentHuntQuest() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    float GetHuntQuestProgress() const;

    // Quest creation helpers
    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CreateRaptorHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CreateTRexHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CreateTriceratopsHuntQuest();

    UFUNCTION(BlueprintCallable, Category = "Quest Hunt")
    void CreateBrachiosaurusHuntQuest();

    // Events
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHuntQuestStarted, EDinosaurSpecies, Species, FString, QuestName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHuntQuestCompleted, EDinosaurSpecies, Species, int32, KillCount);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHuntQuestFailed, FString, Reason);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDinosaurKilled, EDinosaurSpecies, Species, int32, CurrentKills, int32, RequiredKills);

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnHuntQuestStarted OnHuntQuestStarted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnHuntQuestCompleted OnHuntQuestCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnHuntQuestFailed OnHuntQuestFailed;

    UPROPERTY(BlueprintAssignable, Category = "Quest Events")
    FOnDinosaurKilled OnDinosaurKilled;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest State")
    FQuest_DinosaurHuntData CurrentHuntQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    TMap<EDinosaurSpecies, int32> DinosaurDifficultyScores;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    float BaseTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Config")
    int32 MaxActiveHunts;

private:
    void UpdateQuestTimer();
    void CheckQuestCompletion();
    void InitializeDifficultyScores();

    FTimerHandle QuestTimerHandle;
};
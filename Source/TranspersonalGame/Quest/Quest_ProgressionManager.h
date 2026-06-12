#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Quest_ProgressionManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ProgressionStage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    FString StageName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    int32 RequiredLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FString> PrerequisiteQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FString> UnlockedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    FVector StageLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    bool bIsCompleted;

    FQuest_ProgressionStage()
    {
        StageName = TEXT("Unknown Stage");
        RequiredLevel = 1;
        StageLocation = FVector::ZeroVector;
        bIsCompleted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_PlayerProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    int32 CurrentLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    TArray<FString> CompletedQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    TArray<FString> ActiveQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    int32 CurrentStageIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Progress")
    float TotalPlayTime;

    FQuest_PlayerProgress()
    {
        CurrentLevel = 1;
        CurrentStageIndex = 0;
        TotalPlayTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ProgressionManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ProgressionManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<FQuest_ProgressionStage> ProgressionStages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    FQuest_PlayerProgress PlayerProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    float StageCompletionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    bool bAutoProgressEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Progression")
    TArray<AActor*> QuestMarkers;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void InitializeProgressionStages();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool CanProgressToStage(int32 StageIndex);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void CompleteCurrentStage();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void AdvanceToNextStage();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    FQuest_ProgressionStage GetCurrentStage();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    TArray<FString> GetAvailableQuests();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void UpdatePlayerProgress(const FString& CompletedQuest, int32 ExperienceGained);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    bool IsQuestUnlocked(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void CreateProgressionMarkers();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    void UpdateMarkerVisibility();

    UFUNCTION(BlueprintCallable, Category = "Quest Progression")
    float GetProgressionPercentage();

private:
    void CheckPlayerProximityToStages();
    void HandleStageTransition(int32 NewStageIndex);
    void SpawnQuestMarker(const FVector& Location, const FString& MarkerName);
};
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_DefenseQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DefenseObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    FString DefenseLocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    FVector DefensePosition;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    float DefenseRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    int32 WavesToSurvive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    int32 CurrentWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    FString DefenseReward;

    FQuest_DefenseObjective()
    {
        DefenseLocationName = TEXT("Base Camp");
        DefensePosition = FVector::ZeroVector;
        DefenseRadius = 1000.0f;
        WavesToSurvive = 3;
        CurrentWave = 0;
        bIsActive = false;
        bIsCompleted = false;
        DefenseReward = TEXT("Fortification Materials");
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DefenseQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DefenseQuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    TArray<FQuest_DefenseObjective> ActiveDefenseQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    float WaveSpawnTimer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    float WaveInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    int32 EnemiesPerWave;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Defense Quest")
    int32 CompletedDefenseCount;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    void StartDefenseQuest(const FString& LocationName, const FVector& Position, int32 Waves);

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    void ProcessWaveCompletion(int32 QuestIndex);

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    void CompleteDefenseQuest(int32 QuestIndex);

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    bool IsPlayerInDefenseZone(const FVector& PlayerLocation, int32 QuestIndex) const;

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    TArray<FQuest_DefenseObjective> GetActiveDefenseQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    FString GetDefenseProgressText() const;

    UFUNCTION(BlueprintCallable, Category = "Defense Quest")
    void GenerateRandomDefenseQuest();
};
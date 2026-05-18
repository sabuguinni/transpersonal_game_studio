#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Quest_SurvivalChallengeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_SurvivalChallenge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    FString ChallengeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    ESurvivalStat RequiredStat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    float TargetValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Challenge")
    FVector ChallengeLocation;

    FQuest_SurvivalChallenge()
    {
        ChallengeName = TEXT("Survive the Night");
        RequiredStat = ESurvivalStat::Health;
        TargetValue = 50.0f;
        TimeLimit = 300.0f;
        bIsCompleted = false;
        ChallengeLocation = FVector::ZeroVector;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_SurvivalChallengeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_SurvivalChallengeSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    TArray<FQuest_SurvivalChallenge> ActiveChallenges;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    float ChallengeCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest System")
    int32 MaxActiveChallenges;

    FTimerHandle ChallengeTimerHandle;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void StartSurvivalChallenge(const FString& ChallengeName, ESurvivalStat StatType, float Target, float Duration, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CheckChallengeProgress();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    bool CompleteSurvivalChallenge(const FString& ChallengeName);

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    TArray<FQuest_SurvivalChallenge> GetActiveChallenges() const;

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateNightSurvivalChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateHungerChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateThirstChallenge();

    UFUNCTION(BlueprintCallable, Category = "Quest System")
    void CreateTemperatureChallenge();

private:
    void OnChallengeTimer();
    void SpawnChallengeMarker(FVector Location, const FString& ChallengeName);
    void RemoveExpiredChallenges();
};
#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "../SharedTypes.h"
#include "Quest_PrehistoricHuntManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EDinosaurSpecies TargetSpecies = EDinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float RewardMeat = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float RewardHide = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FString HuntDescription = TEXT("Hunt the mighty predator");

    FQuest_HuntTarget()
    {
        TargetSpecies = EDinosaurSpecies::TRex;
        RequiredCount = 1;
        RewardMeat = 50.0f;
        RewardHide = 25.0f;
        HuntDescription = TEXT("Hunt the mighty predator");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ActiveHunt
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Hunt")
    FString QuestID = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Hunt")
    FQuest_HuntTarget HuntTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Hunt")
    int32 CurrentKills = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Hunt")
    bool bIsCompleted = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Hunt")
    float TimeRemaining = 600.0f;

    FQuest_ActiveHunt()
    {
        QuestID = TEXT("");
        CurrentKills = 0;
        bIsCompleted = false;
        TimeRemaining = 600.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_PrehistoricHuntManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UQuest_PrehistoricHuntManager();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    void StartHuntQuest(const FString& QuestID, const FQuest_HuntTarget& HuntTarget);

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    void RegisterDinosaurKill(EDinosaurSpecies Species, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    bool IsHuntActive(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    FQuest_ActiveHunt GetActiveHunt(const FString& QuestID) const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    TArray<FQuest_ActiveHunt> GetAllActiveHunts() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    void CompleteHunt(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    void UpdateHuntTimer(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    FQuest_HuntTarget CreateTRexHunt();

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    FQuest_HuntTarget CreateRaptorPackHunt();

    UFUNCTION(BlueprintCallable, Category = "Hunt Manager")
    FQuest_HuntTarget CreateTriceratopsHunt();

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Hunt Manager")
    TArray<FQuest_ActiveHunt> ActiveHunts;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Manager")
    int32 CompletedHunts = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Manager")
    float TotalHuntTime = 0.0f;

private:
    void ProcessHuntCompletion(FQuest_ActiveHunt& Hunt);
    void GiveHuntRewards(const FQuest_HuntTarget& Target);
    void BroadcastHuntEvent(const FString& EventType, const FString& QuestID);
};
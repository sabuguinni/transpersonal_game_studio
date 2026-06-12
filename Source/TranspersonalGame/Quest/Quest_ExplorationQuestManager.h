#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "Quest_ExplorationQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_ExplorationObjective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    FString LocationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    float ExplorationRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    FString DiscoveryReward;

    FQuest_ExplorationObjective()
    {
        LocationName = TEXT("Unknown Territory");
        TargetLocation = FVector::ZeroVector;
        ExplorationRadius = 500.0f;
        bIsCompleted = false;
        DiscoveryReward = TEXT("Map Fragment");
    }
};

UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_ExplorationQuestManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_ExplorationQuestManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    TArray<FQuest_ExplorationObjective> ActiveExplorationQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    float PlayerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    int32 CompletedExplorationCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Exploration Quest")
    bool bAutoGenerateQuests;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    void CreateExplorationQuest(const FString& LocationName, const FVector& TargetLocation, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    bool CheckExplorationProgress(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    void CompleteExplorationQuest(int32 QuestIndex);

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    TArray<FQuest_ExplorationObjective> GetActiveQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    void GenerateRandomExplorationQuest();

    UFUNCTION(BlueprintCallable, Category = "Exploration Quest")
    FString GetQuestProgressText() const;
};
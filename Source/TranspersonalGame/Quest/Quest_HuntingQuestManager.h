#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "../SharedTypes.h"
#include "Quest_HuntingQuestManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EDinosaurSpecies TargetSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 CurrentKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector HuntingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float HuntingRadius;

    FQuest_HuntTarget()
    {
        TargetSpecies = EDinosaurSpecies::TRex;
        RequiredKills = 1;
        CurrentKills = 0;
        HuntingArea = FVector::ZeroVector;
        HuntingRadius = 1000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_HuntingQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    FString QuestName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    FString QuestDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    TArray<FQuest_HuntTarget> HuntTargets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    EQuestStatus QuestStatus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    float TimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    float ElapsedTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quest")
    TArray<FString> RewardItems;

    FQuest_HuntingQuest()
    {
        QuestName = TEXT("Hunt the Beast");
        QuestDescription = TEXT("Hunt dangerous creatures to protect the tribe");
        QuestStatus = EQuestStatus::NotStarted;
        TimeLimit = 600.0f; // 10 minutes
        ElapsedTime = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_HuntingQuestManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_HuntingQuestManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* QuestMarkerMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    TArray<FQuest_HuntingQuest> ActiveHuntingQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunting Quests")
    TArray<FQuest_HuntingQuest> CompletedHuntingQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    float QuestUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    bool bAutoGenerateQuests;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Settings")
    int32 MaxActiveQuests;

public:
    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void StartHuntingQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void CompleteHuntingQuest(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void RegisterDinosaurKill(EDinosaurSpecies Species, const FVector& KillLocation);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void UpdateQuestProgress(const FString& QuestName);

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    FQuest_HuntingQuest CreateRandomHuntingQuest();

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    TArray<FQuest_HuntingQuest> GetActiveHuntingQuests() const;

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    bool IsQuestActive(const FString& QuestName) const;

    UFUNCTION(BlueprintCallable, Category = "Hunting Quests")
    void GenerateHuntingQuestsForArea(const FVector& AreaCenter, float AreaRadius);

protected:
    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    void UpdateActiveQuests(float DeltaTime);
    void CheckQuestCompletion();
    void GenerateNewQuests();
    FQuest_HuntTarget CreateHuntTarget(EDinosaurSpecies Species, int32 KillCount, const FVector& Area);

private:
    float LastQuestUpdateTime;
    int32 QuestIDCounter;
};
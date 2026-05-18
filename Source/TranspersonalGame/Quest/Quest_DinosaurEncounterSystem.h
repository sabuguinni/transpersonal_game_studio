#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "Quest_DinosaurEncounterSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_DinosaurEncounter
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FString DinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    EQuest_EncounterType EncounterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float RewardPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsActiveEncounter;

    FQuest_DinosaurEncounter()
    {
        DinosaurSpecies = TEXT("TRex");
        EncounterType = EQuest_EncounterType::Hunt;
        SpawnLocation = FVector::ZeroVector;
        DinosaurCount = 1;
        ThreatLevel = 5.0f;
        RewardPoints = 100.0f;
        bIsActiveEncounter = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EncounterReward
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    int32 ExperiencePoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    TArray<FString> ItemRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    float SurvivalPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
    bool bUnlocksNewArea;

    FQuest_EncounterReward()
    {
        ExperiencePoints = 50;
        SurvivalPoints = 25.0f;
        bUnlocksNewArea = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_DinosaurEncounterSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DinosaurEncounterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    TArray<FQuest_DinosaurEncounter> ActiveEncounters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    TMap<FString, FQuest_EncounterReward> EncounterRewards;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    float EncounterCheckRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    float EncounterSpawnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounters")
    int32 MaxActiveEncounters;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void SpawnDinosaurEncounter(const FString& Species, EQuest_EncounterType Type, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void CompleteEncounter(const FString& EncounterId);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    bool CheckPlayerNearEncounter(const FVector& PlayerLocation, float CheckDistance);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    TArray<FQuest_DinosaurEncounter> GetActiveEncounters() const;

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void TriggerRandomEncounter(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void SetEncounterDifficulty(float NewDifficulty);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    FQuest_EncounterReward GetEncounterReward(const FString& EncounterId) const;

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void InitializeEncounterSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounters")
    void CleanupCompletedEncounters();

private:
    void UpdateEncounterStates(float DeltaTime);
    void CheckEncounterTriggers();
    FString GenerateEncounterId() const;
    void LoadEncounterData();
    void SaveEncounterProgress();

    float EncounterTimer;
    float CurrentDifficulty;
    int32 EncounterCounter;
};
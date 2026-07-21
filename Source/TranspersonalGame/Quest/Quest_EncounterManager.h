#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/TriggerBox.h"
#include "../SharedTypes.h"
#include "Quest_EncounterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EncounterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FString EncounterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    EQuest_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    int32 DinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
    float DifficultyRating;

    FQuest_EncounterData()
    {
        EncounterName = TEXT("Default Encounter");
        DinosaurType = EQuest_DinosaurType::Velociraptor;
        SpawnLocation = FVector::ZeroVector;
        DinosaurCount = 1;
        bIsCompleted = false;
        DifficultyRating = 1.0f;
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
    float SurvivalBonus;

    FQuest_EncounterReward()
    {
        ExperiencePoints = 100;
        SurvivalBonus = 0.1f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_EncounterManager : public AActor
{
    GENERATED_BODY()

public:
    AQuest_EncounterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EncounterMarker;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    FQuest_EncounterData EncounterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    FQuest_EncounterReward EncounterReward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    TArray<AActor*> SpawnedDinosaurs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    bool bEncounterActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    float EncounterRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    FString QuestDialogue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Encounter")
    FString CompletionDialogue;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    void CompleteEncounter();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    void SpawnEncounterDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    bool CheckEncounterCompletion();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    void GiveRewards();

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    void SetEncounterData(const FQuest_EncounterData& NewEncounterData);

    UFUNCTION(BlueprintCallable, Category = "Quest Encounter")
    FQuest_EncounterData GetEncounterData() const;

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Encounter")
    void OnEncounterStarted();

    UFUNCTION(BlueprintImplementableEvent, Category = "Quest Encounter")
    void OnEncounterCompleted();

    UFUNCTION()
    void OnTriggerEnter(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
    void InitializeEncounter();
    void CleanupEncounter();
    bool ValidateEncounterSetup() const;
    void UpdateEncounterState();
};
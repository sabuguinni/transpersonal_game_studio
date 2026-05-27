#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Quest_DinosaurHuntSystem.generated.h"

UENUM(BlueprintType)
enum class EQuest_DinosaurSpecies : uint8
{
    TRex        UMETA(DisplayName = "T-Rex"),
    Raptor      UMETA(DisplayName = "Velociraptor"), 
    Triceratops UMETA(DisplayName = "Triceratops"),
    Ankylo      UMETA(DisplayName = "Ankylosaurus"),
    Brachio     UMETA(DisplayName = "Brachiosaurus")
};

UENUM(BlueprintType)
enum class EQuest_HuntDifficulty : uint8
{
    Easy        UMETA(DisplayName = "Easy Hunt"),
    Medium      UMETA(DisplayName = "Medium Hunt"),
    Hard        UMETA(DisplayName = "Hard Hunt"),
    Legendary   UMETA(DisplayName = "Legendary Hunt")
};

USTRUCT(BlueprintType)
struct FQuest_DinosaurTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_DinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredKills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_HuntDifficulty Difficulty;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector HuntLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float TimeLimit;

    FQuest_DinosaurTarget()
    {
        Species = EQuest_DinosaurSpecies::Raptor;
        RequiredKills = 1;
        Difficulty = EQuest_HuntDifficulty::Easy;
        HuntLocation = FVector::ZeroVector;
        TimeLimit = 300.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_HuntProgress
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Progress")
    int32 CurrentKills;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Progress")
    bool bHuntActive;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Progress")
    float TimeRemaining;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt Progress")
    TArray<AActor*> TrackedDinosaurs;

    FQuest_HuntProgress()
    {
        CurrentKills = 0;
        bHuntActive = false;
        TimeRemaining = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DinosaurHuntSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DinosaurHuntSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    TArray<FQuest_DinosaurTarget> AvailableHunts;

    UPROPERTY(BlueprintReadOnly, Category = "Hunt System")
    FQuest_HuntProgress CurrentHunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    float HuntDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt System")
    bool bAutoTrackNearbyDinosaurs;

    FTimerHandle HuntTimerHandle;

public:
    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void StartDinosaurHunt(const FQuest_DinosaurTarget& Target);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void EndCurrentHunt(bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void RegisterDinosaurKill(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    bool IsHuntActive() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    FQuest_HuntProgress GetCurrentHuntProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    TArray<FQuest_DinosaurTarget> GetAvailableHunts() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void CreateBasicHuntMissions();

    UFUNCTION(BlueprintCallable, Category = "Hunt System")
    void ScanForNearbyDinosaurs();

protected:
    UFUNCTION()
    void OnHuntTimerExpired();

    UFUNCTION(BlueprintImplementableEvent, Category = "Hunt System")
    void OnHuntStarted(const FQuest_DinosaurTarget& Target);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hunt System")
    void OnHuntCompleted(bool bSuccess, int32 KillCount);

    UFUNCTION(BlueprintImplementableEvent, Category = "Hunt System")
    void OnDinosaurKilled(AActor* DinosaurActor);

    void UpdateHuntProgress();
    void CheckHuntCompletion();
};
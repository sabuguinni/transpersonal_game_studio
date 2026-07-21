#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "Quest_DinosaurHuntMission.generated.h"

UENUM(BlueprintType)
enum class EQuest_HuntPhase : uint8
{
    Tracking,
    Stalking,
    Ambush,
    Combat,
    Harvest,
    Completed,
    Failed
};

UENUM(BlueprintType)
enum class EQuest_DinosaurType : uint8
{
    TRex,
    Velociraptor,
    Triceratops,
    Brachiosaurus,
    Ankylosaurus,
    Parasaurolophus
};

USTRUCT(BlueprintType)
struct FQuest_HuntTarget
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    EQuest_DinosaurType DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    int32 RequiredCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float MinimumSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    bool bRequireAlpha;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    FVector PreferredHuntingArea;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Target")
    float HuntingAreaRadius;

    FQuest_HuntTarget()
    {
        DinosaurType = EQuest_DinosaurType::Velociraptor;
        RequiredCount = 1;
        MinimumSize = 1.0f;
        bRequireAlpha = false;
        PreferredHuntingArea = FVector::ZeroVector;
        HuntingAreaRadius = 5000.0f;
    }
};

USTRUCT(BlueprintType)
struct FQuest_HuntProgress
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    EQuest_HuntPhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    int32 DinosaursSighted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    int32 DinosaursKilled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    float TrackingProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    float StealthLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    bool bTargetDetected;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Progress")
    AActor* CurrentTarget;

    FQuest_HuntProgress()
    {
        CurrentPhase = EQuest_HuntPhase::Tracking;
        DinosaursSighted = 0;
        DinosaursKilled = 0;
        TrackingProgress = 0.0f;
        StealthLevel = 100.0f;
        bTargetDetected = false;
        CurrentTarget = nullptr;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_DinosaurHuntMission : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DinosaurHuntMission();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Hunt Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FQuest_HuntTarget HuntTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    FQuest_HuntProgress HuntProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float TrackingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float StealthDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float AmbushRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    float MissionTimeLimit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    bool bAllowGroupHunt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hunt Mission")
    TArray<AActor*> NearbyDinosaurs;

    // Mission Events
    UPROPERTY(BlueprintAssignable, Category = "Hunt Events")
    FOnMissionPhaseChanged OnPhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Hunt Events")
    FOnMissionTargetSighted OnTargetSighted;

    UPROPERTY(BlueprintAssignable, Category = "Hunt Events")
    FOnMissionCompleted OnMissionCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Hunt Events")
    FOnMissionFailed OnMissionFailed;

public:
    // Hunt Mission Interface
    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void StartHuntMission(const FQuest_HuntTarget& Target);

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void UpdateTrackingProgress(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void CheckForNearbyDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void EnterStalkingPhase(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void AttemptAmbush();

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void OnDinosaurKilled(AActor* KilledDinosaur);

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void CompleteMission();

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void FailMission(const FString& Reason);

    // Utility Functions
    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    float CalculateStealthLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    bool IsValidHuntTarget(AActor* Dinosaur) const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    FVector GetOptimalAmbushPosition(AActor* Target) const;

    UFUNCTION(BlueprintCallable, Category = "Hunt Mission")
    void UpdateHuntPhase(EQuest_HuntPhase NewPhase);

    // Getters
    UFUNCTION(BlueprintPure, Category = "Hunt Mission")
    EQuest_HuntPhase GetCurrentPhase() const { return HuntProgress.CurrentPhase; }

    UFUNCTION(BlueprintPure, Category = "Hunt Mission")
    float GetMissionProgress() const;

    UFUNCTION(BlueprintPure, Category = "Hunt Mission")
    bool IsMissionActive() const;

private:
    FTimerHandle MissionTimerHandle;
    FTimerHandle TrackingTimerHandle;
    float MissionStartTime;
    bool bMissionActive;

    void OnMissionTimeout();
    void UpdateStealthDetection(float DeltaTime);
    void ProcessTrackingPhase(float DeltaTime);
    void ProcessStalkingPhase(float DeltaTime);
    void ProcessAmbushPhase(float DeltaTime);
};
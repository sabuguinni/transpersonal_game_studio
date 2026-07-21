#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "NPCBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_ActivityType : uint8
{
    Hunting     UMETA(DisplayName = "Hunting"),
    Gathering   UMETA(DisplayName = "Gathering"), 
    Guarding    UMETA(DisplayName = "Guarding"),
    WaterGathering UMETA(DisplayName = "Water Gathering"),
    Resting     UMETA(DisplayName = "Resting"),
    Socializing UMETA(DisplayName = "Socializing")
};

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Working     UMETA(DisplayName = "Working"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following"),
    Investigating UMETA(DisplayName = "Investigating")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_ActivityType PrimaryActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPC_BehaviorState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString PrimaryZone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float PatrolRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float AlertRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    float FleeDistance;

    FNPC_BehaviorData()
    {
        PrimaryActivity = ENPC_ActivityType::Idle;
        CurrentState = ENPC_BehaviorState::Idle;
        PrimaryZone = TEXT("Default_Zone");
        PatrolRadius = 500.0f;
        MovementSpeed = 150.0f;
        AlertRadius = 800.0f;
        FleeDistance = 1200.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
    float DawnActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
    float MorningActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
    float NoonActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
    float EveningActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Schedule")
    float NightActivity;

    FNPC_DailySchedule()
    {
        DawnActivity = 0.0f;
        MorningActivity = 6.0f;
        NoonActivity = 12.0f;
        EveningActivity = 18.0f;
        NightActivity = 22.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorManager : public AActor
{
    GENERATED_BODY()

public:
    ANPCBehaviorManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ManagerMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    TArray<AActor*> ManagedNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    TMap<FString, FNPC_BehaviorData> NPCBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    FNPC_DailySchedule GlobalSchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    float UpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Management")
    bool bEnableDynamicBehavior;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void RegisterNPC(AActor* NPCActor, const FNPC_BehaviorData& BehaviorData);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void UnregisterNPC(AActor* NPCActor);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void UpdateNPCBehavior(AActor* NPCActor, ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void SetNPCActivity(AActor* NPCActor, ENPC_ActivityType NewActivity);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    TArray<AActor*> GetNPCsInRadius(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void TriggerGroupBehavior(const FString& ZoneName, ENPC_BehaviorState GroupState);

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    ENPC_ActivityType GetScheduledActivity(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void InitializeBehaviorZones();

    UFUNCTION(BlueprintCallable, Category = "NPC Management")
    void ProcessDailySchedule();

private:
    FTimerHandle UpdateTimer;
    float GameTimeHours;

    void UpdateAllNPCs();
    void ProcessNPCInteractions();
    void HandlePlayerProximity();
    void UpdateNPCStates();
};
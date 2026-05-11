#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "../SharedTypes.h"
#include "NPC_TribalBehaviorManager.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Guard       UMETA(DisplayName = "Guard")
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Working     UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Eating      UMETA(DisplayName = "Eating"),
    Sleeping    UMETA(DisplayName = "Sleeping"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Crafting    UMETA(DisplayName = "Crafting"),
    Teaching    UMETA(DisplayName = "Teaching")
};

USTRUCT(BlueprintType)
struct FNPC_TribalSchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_TribalActivity Activity = ENPC_TribalActivity::Working;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float Priority = 1.0f;

    FNPC_TribalSchedule()
    {
        StartHour = 6.0f;
        EndHour = 18.0f;
        Activity = ENPC_TribalActivity::Working;
        TargetLocation = FVector::ZeroVector;
        Priority = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_SocialRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    AActor* TargetNPC = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float RelationshipValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    int32 InteractionCount = 0;

    FNPC_SocialRelationship()
    {
        TargetNPC = nullptr;
        RelationshipValue = 0.0f;
        LastInteractionTime = 0.0f;
        InteractionCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalActivity CurrentActivity = ENPC_TribalActivity::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float EnergyLevel = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float HungerLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    float SocialNeed = 50.0f;

    // Schedule System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_TribalSchedule> DailySchedule;

    UPROPERTY(BlueprintReadOnly, Category = "Schedule")
    int32 CurrentScheduleIndex = 0;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<FNPC_SocialRelationship> SocialRelationships;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float SocialInteractionRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    float MaxSocialDistance = 1000.0f;

    // Work System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Work")
    TArray<AActor*> WorkLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Work")
    AActor* CurrentWorkTarget = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Work")
    float WorkEfficiency = 1.0f;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void StartActivity(ENPC_TribalActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    void UpdateSchedule();

    UFUNCTION(BlueprintCallable, Category = "Schedule")
    FNPC_TribalSchedule GetCurrentScheduleItem();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateSocialRelationships();

    UFUNCTION(BlueprintCallable, Category = "Social")
    void InteractWithNPC(AActor* TargetNPC);

    UFUNCTION(BlueprintCallable, Category = "Social")
    TArray<AActor*> FindNearbyNPCs();

    UFUNCTION(BlueprintCallable, Category = "Work")
    void FindWorkLocation();

    UFUNCTION(BlueprintCallable, Category = "Work")
    void PerformWork(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool ShouldSeekSocialInteraction();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool IsActivityAllowedForRole(ENPC_TribalActivity Activity);

private:
    // Internal timers
    FTimerHandle ScheduleUpdateTimer;
    FTimerHandle SocialUpdateTimer;
    FTimerHandle NeedsUpdateTimer;

    // Internal state
    float LastActivityChange = 0.0f;
    float ActivityDuration = 0.0f;
    bool bIsPerformingScheduledActivity = false;

    // Helper functions
    void InitializeRoleBasedSchedule();
    void UpdateActivityBasedOnNeeds();
    float CalculateActivityPriority(ENPC_TribalActivity Activity);
    void HandleRoleSpecificBehavior(float DeltaTime);
};
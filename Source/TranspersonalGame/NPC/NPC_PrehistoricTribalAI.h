#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "SharedTypes.h"
#include "NPC_PrehistoricTribalAI.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Scout       UMETA(DisplayName = "Scout"),
    Elder       UMETA(DisplayName = "Elder"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Guardian    UMETA(DisplayName = "Guardian")
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    AlertWatch      UMETA(DisplayName = "Alert Watch")
};

USTRUCT(BlueprintType)
struct FNPC_TribalKnowledge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FVector> KnownDangerZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TArray<FVector> SafeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    TMap<FString, float> SpeciesKnowledge;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Knowledge")
    float LastDangerEncounter;

    FNPC_TribalKnowledge()
    {
        LastDangerEncounter = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Needs", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Rest;

    FNPC_TribalNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Safety = 80.0f;
        Social = 60.0f;
        Rest = 70.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_PrehistoricTribalAI : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_PrehistoricTribalAI();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core tribal identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString TribeName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 TribalHierarchy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    float Experience;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ENPC_TribalActivity CurrentActivity;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FNPC_TribalNeeds Needs;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FNPC_TribalKnowledge Knowledge;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AlertnessRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float WorkRadius;

    // Daily routine
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<ENPC_TribalActivity> DailySchedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float DayStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    float DayEndTime;

    // Communication
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    TArray<FString> VocalAlerts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication")
    float CommunicationRange;

    // AI behavior functions
    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void SetTribalRole(ENPC_TribalRole NewRole);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void ProcessDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void RespondToDanger(AActor* ThreatActor, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void ShareKnowledge(UNPC_PrehistoricTribalAI* OtherTribalMember);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void ExecuteTribalActivity(ENPC_TribalActivity Activity);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    bool IsInDanger() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    FVector FindNearestSafeZone() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void AddDangerZone(FVector Location, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Tribal AI")
    void BroadcastAlert(const FString& AlertMessage);

private:
    // Internal state tracking
    float LastActivityChange;
    float LastNeedsUpdate;
    float LastKnowledgeShare;
    
    // Helper functions
    void InitializeTribalBehavior();
    void UpdateActivityBasedOnNeeds();
    void ProcessSocialInteractions();
    void UpdateKnowledge();
    ENPC_TribalActivity DetermineOptimalActivity();
    void ExecuteRoleSpecificBehavior();
};
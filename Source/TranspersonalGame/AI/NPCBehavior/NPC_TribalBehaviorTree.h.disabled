#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "NPC_TribalBehaviorTree.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Idle,
    Gathering,
    Hunting,
    Patrolling,
    SocialInteraction,
    Sleeping,
    Crafting,
    GuardDuty,
    FleeingDanger
};

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Hunter,
    Gatherer,
    Crafter,
    Elder,
    Scout,
    Guard,
    Child
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownDangerLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> KnownResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownAllies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownThreats;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastDangerTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation;

    FNPC_TribalMemory()
    {
        LastDangerTime = 0.0f;
        LastKnownPlayerLocation = FVector::ZeroVector;
    }
};

USTRUCT(BlueprintType)
struct FNPC_TribalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social;

    FNPC_TribalNeeds()
    {
        Hunger = 50.0f;
        Thirst = 50.0f;
        Energy = 100.0f;
        Safety = 100.0f;
        Social = 50.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorTree : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorTree();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Behavior Tree Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void InitializeBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void UpdateBehaviorState(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void SetCurrentActivity(ENPC_TribalActivity NewActivity);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    ENPC_TribalActivity GetCurrentActivity() const { return CurrentActivity; }

    // Decision Making
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    ENPC_TribalActivity DetermineNextActivity();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool ShouldFleeFromDanger();

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool IsNightTime();

    // Memory System
    UFUNCTION(BlueprintCallable, Category = "Tribal Memory")
    void RememberDangerLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Memory")
    void RememberResourceLocation(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Memory")
    void RememberThreat(AActor* ThreatActor);

    UFUNCTION(BlueprintCallable, Category = "Tribal Memory")
    bool IsLocationDangerous(FVector Location);

    // Needs Management
    UFUNCTION(BlueprintCallable, Category = "Tribal Needs")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Tribal Needs")
    ENPC_TribalActivity GetMostUrgentNeed();

    UFUNCTION(BlueprintCallable, Category = "Tribal Needs")
    void SatisfyNeed(ENPC_TribalActivity NeedType, float Amount);

    // Social Behavior
    UFUNCTION(BlueprintCallable, Category = "Tribal Social")
    TArray<AActor*> FindNearbyTribalMembers(float SearchRadius = 2000.0f);

    UFUNCTION(BlueprintCallable, Category = "Tribal Social")
    bool ShouldInteractSocially();

    UFUNCTION(BlueprintCallable, Category = "Tribal Social")
    AActor* FindBestSocialTarget();

protected:
    // Core Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalRole TribalRole;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    ENPC_TribalActivity CurrentActivity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FNPC_TribalMemory TribalMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Behavior")
    FNPC_TribalNeeds TribalNeeds;

    // Behavior Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Config")
    float ActivityChangeInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Config")
    float DangerDetectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Config")
    float SocialInteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Config")
    float NeedDecayRate;

    // Behavior Tree References
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* TribalBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardComponent* BlackboardComponent;

    // Timers
    UPROPERTY(BlueprintReadOnly, Category = "Tribal State")
    float TimeSinceLastActivityChange;

    UPROPERTY(BlueprintReadOnly, Category = "Tribal State")
    float TimeSinceLastNeedUpdate;

private:
    // Internal state tracking
    bool bIsInitialized;
    float LastDangerCheckTime;
    AActor* CurrentSocialTarget;
};
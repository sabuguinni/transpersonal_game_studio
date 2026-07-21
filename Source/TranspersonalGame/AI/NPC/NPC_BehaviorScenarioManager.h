#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "SharedTypes.h"
#include "NPC_BehaviorScenarioManager.generated.h"

// Forward declarations
class ANPC_DinosaurAIController;
class UNPC_SocialDynamicsManager;
class UNPC_PackBehaviorComponent;

UENUM(BlueprintType)
enum class ENPC_BehaviorScenarioType : uint8
{
    TerritorialConflict UMETA(DisplayName = "Territorial Conflict"),
    PackHunting UMETA(DisplayName = "Pack Hunting"),
    SocialInteraction UMETA(DisplayName = "Social Interaction"),
    FeedingBehavior UMETA(DisplayName = "Feeding Behavior"),
    MigrationPattern UMETA(DisplayName = "Migration Pattern"),
    MatingRitual UMETA(DisplayName = "Mating Ritual"),
    ParentalCare UMETA(DisplayName = "Parental Care"),
    FleeResponse UMETA(DisplayName = "Flee Response")
};

USTRUCT(BlueprintType)
struct FNPC_BehaviorScenario
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FString ScenarioName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    ENPC_BehaviorScenarioType ScenarioType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    TArray<AActor*> ParticipatingActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    FVector TriggerLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    float ScenarioDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenario")
    int32 Priority;

    FNPC_BehaviorScenario()
    {
        ScenarioName = TEXT("Default Scenario");
        ScenarioType = ENPC_BehaviorScenarioType::SocialInteraction;
        TriggerLocation = FVector::ZeroVector;
        TriggerRadius = 1000.0f;
        ScenarioDuration = 60.0f;
        bIsActive = false;
        Priority = 1;
    }
};

/**
 * Manages complex behavioral scenarios involving multiple NPCs and environmental triggers
 * Coordinates pack hunting, territorial disputes, social interactions, and migration patterns
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_BehaviorScenarioManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_BehaviorScenarioManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Scenario Management
    UFUNCTION(BlueprintCallable, Category = "Behavior Scenarios")
    void InitializeScenarioManager();

    UFUNCTION(BlueprintCallable, Category = "Behavior Scenarios")
    void RegisterBehaviorScenario(const FNPC_BehaviorScenario& NewScenario);

    UFUNCTION(BlueprintCallable, Category = "Behavior Scenarios")
    void TriggerScenario(const FString& ScenarioName);

    UFUNCTION(BlueprintCallable, Category = "Behavior Scenarios")
    void EndScenario(const FString& ScenarioName);

    UFUNCTION(BlueprintCallable, Category = "Behavior Scenarios")
    bool IsScenarioActive(const FString& ScenarioName) const;

    // Pack Behavior Coordination
    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void CoordinatePackHunt(const TArray<AActor*>& PackMembers, AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void FormDefensiveCircle(const TArray<AActor*>& PackMembers, const FVector& ThreatLocation);

    UFUNCTION(BlueprintCallable, Category = "Pack Behavior")
    void ExecuteMigrationPattern(const TArray<AActor*>& HerdMembers, const FVector& Destination);

    // Territorial Behavior
    UFUNCTION(BlueprintCallable, Category = "Territorial")
    void EstablishTerritory(AActor* TerritorialActor, const FVector& CenterPoint, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territorial")
    void HandleTerritorialIntrusion(AActor* TerritorialOwner, AActor* Intruder);

    UFUNCTION(BlueprintCallable, Category = "Territorial")
    bool IsInTerritory(AActor* TerritorialOwner, const FVector& Location) const;

    // Social Interaction Management
    UFUNCTION(BlueprintCallable, Category = "Social")
    void InitiateSocialInteraction(AActor* Initiator, AActor* Target, const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ManageHerdCohesion(const TArray<AActor*>& HerdMembers);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void ProcessDominanceHierarchy(const TArray<AActor*>& GroupMembers);

    // Environmental Response
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void TriggerFleeResponse(const TArray<AActor*>& FleeingActors, const FVector& DangerSource);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void HandleWeatherResponse(const TArray<AActor*>& AffectedActors, const FString& WeatherType);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void ManageFeedingBehavior(const TArray<AActor*>& FeedingActors, const FVector& FoodSource);

    // Debugging and Monitoring
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawActiveScenarios() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    TArray<FString> GetActiveScenarioNames() const;

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogScenarioStatus() const;

protected:
    // Core scenario data
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenarios")
    TArray<FNPC_BehaviorScenario> RegisteredScenarios;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scenarios")
    TArray<FNPC_BehaviorScenario> ActiveScenarios;

    // Timing and coordination
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float ScenarioUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float MaxConcurrentScenarios;

    // Territory tracking
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    TMap<AActor*, FVector> TerritorialCenters;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Territory")
    TMap<AActor*, float> TerritorialRadii;

    // Timer handles
    FTimerHandle ScenarioUpdateTimer;
    TMap<FString, FTimerHandle> ScenarioTimers;

private:
    // Internal scenario processing
    void UpdateActiveScenarios();
    void ProcessScenarioTriggers();
    void CleanupExpiredScenarios();
    bool CanActivateScenario(const FNPC_BehaviorScenario& Scenario) const;
    void ExecuteScenarioBehavior(const FNPC_BehaviorScenario& Scenario);
};
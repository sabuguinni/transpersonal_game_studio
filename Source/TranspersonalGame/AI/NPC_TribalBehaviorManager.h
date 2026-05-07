#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "NPC_TribalBehaviorManager.generated.h"

// Forward declarations
class AActor;
class UWorld;

UENUM(BlueprintType)
enum class ENPC_TribalRole : uint8
{
    Chief           UMETA(DisplayName = "Tribal Chief"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Crafter         UMETA(DisplayName = "Crafter"),
    Scout           UMETA(DisplayName = "Scout"),
    Shaman          UMETA(DisplayName = "Medicine Keeper"),
    Warrior         UMETA(DisplayName = "Warrior"),
    Elder           UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class ENPC_TribalActivity : uint8
{
    Sleeping        UMETA(DisplayName = "Sleeping"),
    Eating          UMETA(DisplayName = "Eating"),
    Working         UMETA(DisplayName = "Working"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Crafting        UMETA(DisplayName = "Crafting"),
    Gathering       UMETA(DisplayName = "Gathering"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Resting         UMETA(DisplayName = "Resting"),
    Storytelling    UMETA(DisplayName = "Storytelling")
};

USTRUCT(BlueprintType)
struct FNPC_TribalNeeds
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Hunger = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Thirst = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Energy = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Social = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Safety = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Purpose = 60.0f;
};

USTRUCT(BlueprintType)
struct FNPC_TribalSchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<float, ENPC_TribalActivity> DailyActivities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorkStartTime = 7.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WorkEndTime = 17.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SleepTime = 21.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float WakeTime = 6.0f;
};

USTRUCT(BlueprintType)
struct FNPC_TribalMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> KnownPeople;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> ResourceLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> LearnedSkills;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PlayerInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerEncounter = 0.0f;
};

USTRUCT(BlueprintType)
struct FNPC_TribalPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Courage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Curiosity = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Sociability = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Aggression = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Intelligence = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float Loyalty = 70.0f;
};

// Delegate declarations
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPC_ActivityChanged, ENPC_TribalActivity, OldActivity, ENPC_TribalActivity, NewActivity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNPC_PlayerInteraction, float, TrustChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnNPC_NeedsCritical, FString, NeedType, float, Value);

/**
 * Comprehensive tribal NPC behavior system
 * Manages daily routines, social interactions, survival needs, and player relationships
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalBehaviorManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalBehaviorManager();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    ENPC_TribalRole TribalRole = ENPC_TribalRole::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FString NPCName = "Unnamed Tribal";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Identity")
    FNPC_TribalPersonality Personality;

    // Current State
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Current State")
    ENPC_TribalActivity CurrentActivity = ENPC_TribalActivity::Resting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Current State")
    FNPC_TribalNeeds Needs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FNPC_TribalSchedule Schedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    FNPC_TribalMemory Memory;

    // Tribal Camp Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Camp")
    FVector CampCenter = FVector(0, 0, 50);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Camp")
    float CampRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Camp")
    TArray<FVector> WorkStations;

    // Delegates
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNPC_ActivityChanged OnActivityChanged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNPC_PlayerInteraction OnPlayerInteraction;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNPC_NeedsCritical OnNeedsCritical;

    // Public Methods
    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void InteractWithPlayer(AActor* Player, const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void LearnNewSkill(const FString& SkillName);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void AddResourceLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    void AddDangerZone(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool CanPerformActivity(ENPC_TribalActivity Activity) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    FVector GetNearestResourceLocation() const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    bool IsInDangerZone(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    float GetPlayerTrustLevel() const { return Memory.PlayerTrustLevel; }

    UFUNCTION(BlueprintCallable, Category = "Tribal Behavior")
    TArray<FString> GetKnownSkills() const { return Memory.LearnedSkills; }

protected:
    // Internal state
    UPROPERTY()
    float LastScheduleCheck = 0.0f;

    UPROPERTY()
    float LastNeedsUpdate = 0.0f;

    UPROPERTY()
    float ActivityStartTime = 0.0f;

    // Core behavior methods
    void UpdateNeeds(float DeltaTime);
    void ProcessDailySchedule();
    ENPC_TribalActivity DetermineCurrentActivity();
    void ExecuteActivity(ENPC_TribalActivity Activity, float DeltaTime);
    
    // Role-specific behaviors
    void PerformChiefDuties(float DeltaTime);
    void PerformHunterDuties(float DeltaTime);
    void PerformGathererDuties(float DeltaTime);
    void PerformCrafterDuties(float DeltaTime);
    void PerformScoutDuties(float DeltaTime);
    
    // Utility methods
    void InitializeRoleDefaults();
    float GetCurrentTimeOfDay() const;
    bool IsTimeForActivity(float ScheduledTime, float Tolerance = 1.0f) const;
    void UpdateMemory(float DeltaTime);
    void SocialInteraction(float DeltaTime);
    
    // Movement and positioning
    FVector GetWorkStationLocation() const;
    FVector GetRandomPatrolPoint() const;
    bool IsNearCamp() const;
};
// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "MassEntitySubsystem.h"
#include "MassSpawnerSubsystem.h"
#include "MassSimulationSubsystem.h"
#include "MassNavigationSubsystem.h"
#include "MassActorSubsystem.h"
#include "MassCommonFragments.h"
#include "MassMovementFragments.h"
#include "MassNavigationFragments.h"
#include "MassAvoidanceFragments.h"
#include "MassZoneGraphNavigationFragments.h"
#include "ZoneGraphSubsystem.h"
#include "Engine/DataTable.h"
#include "GameplayTags/Classes/GameplayTagContainer.h"
#include "../NPCBehavior/NPCBehaviorSystem.h"
#include "../CombatAI/CombatAISystem.h"
#include "CrowdSimulationSystem.generated.h"

class UMassEntityConfigAsset;
class UZoneGraphData;

/**
 * Crowd Behavior Types - defines how groups of entities behave
 */
UENUM(BlueprintType)
enum class ECrowdBehaviorType : uint8
{
    // Herbivore Herd Behaviors
    GrazingHerd         UMETA(DisplayName = "Grazing Herd"),        // Peaceful feeding groups
    MigratingHerd       UMETA(DisplayName = "Migrating Herd"),      // Moving to new areas
    FleeingHerd         UMETA(DisplayName = "Fleeing Herd"),        // Escaping from threats
    DefensiveCircle     UMETA(DisplayName = "Defensive Circle"),    // Protecting young/weak
    
    // Predator Pack Behaviors
    HuntingPack         UMETA(DisplayName = "Hunting Pack"),        // Coordinated hunting
    PatrollingPack      UMETA(DisplayName = "Patrolling Pack"),     // Territory patrol
    ScavengingGroup     UMETA(DisplayName = "Scavenging Group"),    // Feeding on carrion
    RestingPride        UMETA(DisplayName = "Resting Pride"),       // Resting together
    
    // Mixed Ecosystem Behaviors
    WateringHole        UMETA(DisplayName = "Watering Hole"),       // Mixed species at water
    ForestGathering     UMETA(DisplayName = "Forest Gathering"),    // Multiple species foraging
    RiverCrossing       UMETA(DisplayName = "River Crossing"),      // Migration crossing
    NestingGround       UMETA(DisplayName = "Nesting Ground"),      // Breeding areas
    
    // Environmental Response Behaviors
    StormSheltering     UMETA(DisplayName = "Storm Sheltering"),    // Weather response
    FireEscape          UMETA(DisplayName = "Fire Escape"),         // Fleeing natural disasters
    TerritorialDispute  UMETA(DisplayName = "Territorial Dispute"), // Fighting over territory
    SeasonalMigration   UMETA(DisplayName = "Seasonal Migration"),  // Long-distance movement
    
    // Individual Behaviors (for lone entities)
    SolitaryHunter      UMETA(DisplayName = "Solitary Hunter"),     // Lone predator
    TerritorialGuard    UMETA(DisplayName = "Territorial Guard"),   // Defending territory
    Scavenger           UMETA(DisplayName = "Scavenger"),           // Individual scavenging
    Wanderer            UMETA(DisplayName = "Wanderer"),            // Random exploration
    
    MAX                 UMETA(Hidden)
};

/**
 * Crowd Density Levels - affects performance and behavior complexity
 */
UENUM(BlueprintType)
enum class ECrowdDensity : uint8
{
    VeryLow             UMETA(DisplayName = "Very Low (1-10)"),     // 1-10 entities
    Low                 UMETA(DisplayName = "Low (10-50)"),        // 10-50 entities
    Medium              UMETA(DisplayName = "Medium (50-200)"),    // 50-200 entities
    High                UMETA(DisplayName = "High (200-1000)"),    // 200-1000 entities
    VeryHigh            UMETA(DisplayName = "Very High (1000-5000)"), // 1000-5000 entities
    Massive             UMETA(DisplayName = "Massive (5000+)"),    // 5000+ entities
    
    MAX                 UMETA(Hidden)
};

/**
 * Crowd Formation Types - spatial arrangements of crowds
 */
UENUM(BlueprintType)
enum class ECrowdFormation : uint8
{
    // Natural Formations
    Scattered           UMETA(DisplayName = "Scattered"),          // Random distribution
    Cluster             UMETA(DisplayName = "Cluster"),            // Tight groups
    Line                UMETA(DisplayName = "Line"),               // Single file
    Column              UMETA(DisplayName = "Column"),             // Multiple lines
    
    // Defensive Formations
    Circle              UMETA(DisplayName = "Circle"),             // Defensive circle
    Wedge               UMETA(DisplayName = "Wedge"),              // V formation
    Wall                UMETA(DisplayName = "Wall"),               // Defensive line
    Huddle              UMETA(DisplayName = "Huddle"),             // Tight protection
    
    // Movement Formations
    Stream              UMETA(DisplayName = "Stream"),             // Flowing movement
    Wave                UMETA(DisplayName = "Wave"),               // Wave-like motion
    Spiral              UMETA(DisplayName = "Spiral"),             // Circular movement
    Funnel              UMETA(DisplayName = "Funnel"),             // Converging movement
    
    // Feeding Formations
    Spread              UMETA(DisplayName = "Spread"),             // Spread out feeding
    Hierarchy           UMETA(DisplayName = "Hierarchy"),          // Pecking order feeding
    Rotation            UMETA(DisplayName = "Rotation"),           // Taking turns
    Competition         UMETA(DisplayName = "Competition"),        // Competitive feeding
    
    MAX                 UMETA(Hidden)
};

/**
 * Crowd Response Types - how crowds react to stimuli
 */
UENUM(BlueprintType)
enum class ECrowdResponse : uint8
{
    // Threat Responses
    FleeInPanic         UMETA(DisplayName = "Flee in Panic"),      // Immediate scattered flight
    OrganizedRetreat    UMETA(DisplayName = "Organized Retreat"),  // Coordinated withdrawal
    StandAndFight       UMETA(DisplayName = "Stand and Fight"),    // Defensive stance
    ScatterAndHide      UMETA(DisplayName = "Scatter and Hide"),   // Disperse to cover
    
    // Curiosity Responses
    CautiousApproach    UMETA(DisplayName = "Cautious Approach"),  // Careful investigation
    CircleAndObserve    UMETA(DisplayName = "Circle and Observe"), // Surround at distance
    SendScouts          UMETA(DisplayName = "Send Scouts"),        // Few investigate first
    IgnoreStimulus      UMETA(DisplayName = "Ignore Stimulus"),    // No reaction
    
    // Social Responses
    GatherTogether      UMETA(DisplayName = "Gather Together"),    // Form tighter groups
    CallForHelp         UMETA(DisplayName = "Call for Help"),      // Vocalize for aid
    FollowLeader        UMETA(DisplayName = "Follow Leader"),      // Follow alpha
    FormAlliance        UMETA(DisplayName = "Form Alliance"),      // Join with other groups
    
    // Environmental Responses
    SeekShelter         UMETA(DisplayName = "Seek Shelter"),       // Find protection
    ChangeDirection     UMETA(DisplayName = "Change Direction"),   // Alter movement path
    SpeedUp             UMETA(DisplayName = "Speed Up"),           // Increase movement speed
    SlowDown            UMETA(DisplayName = "Slow Down"),          // Decrease movement speed
    
    MAX                 UMETA(Hidden)
};

/**
 * Mass Entity Configuration for different dinosaur types
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdEntityConfig
{
    GENERATED_BODY()

    // Basic Entity Info
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FString EntityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    UMassEntityConfigAsset* EntityConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    ECombatArchetype CombatArchetype;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entity")
    FGameplayTagContainer EntityTags;

    // Crowd Behavior
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ECrowdBehaviorType PreferredBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TArray<ECrowdBehaviorType> AlternateBehaviors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float BehaviorChangeChance; // Probability of switching behaviors

    // Group Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 MinGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 MaxGroupSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float GroupCohesion; // How tightly groups stay together

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    float LeadershipChance; // Chance this entity becomes group leader

    // Movement Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float WalkSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float RunSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float TurnRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float PersonalSpace; // Minimum distance from others

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightAngle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SmellRange;

    // Response Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    TMap<FGameplayTag, ECrowdResponse> ThreatResponses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    float PanicThreshold; // How easily this entity panics

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Response")
    float CuriosityLevel; // How likely to investigate new things

    FCrowdEntityConfig()
    {
        EntityName = TEXT("DefaultEntity");
        EntityConfig = nullptr;
        CombatArchetype = ECombatArchetype::FlightResponse;
        PreferredBehavior = ECrowdBehaviorType::GrazingHerd;
        BehaviorChangeChance = 0.1f;
        MinGroupSize = 3;
        MaxGroupSize = 12;
        GroupCohesion = 0.7f;
        LeadershipChance = 0.2f;
        WalkSpeed = 200.0f;
        RunSpeed = 600.0f;
        TurnRate = 90.0f;
        PersonalSpace = 150.0f;
        SightRange = 2000.0f;
        SightAngle = 120.0f;
        HearingRange = 1500.0f;
        SmellRange = 1000.0f;
        PanicThreshold = 0.6f;
        CuriosityLevel = 0.4f;
    }
};

/**
 * Crowd Group Data - represents a group of entities with shared behavior
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdGroupData
{
    GENERATED_BODY()

    // Group Identity
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    int32 GroupID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    FString GroupName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowdBehaviorType CurrentBehavior;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Group")
    ECrowdFormation CurrentFormation;

    // Group Composition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composition")
    TArray<FMassEntityHandle> GroupMembers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composition")
    FMassEntityHandle GroupLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composition")
    int32 CurrentSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Composition")
    FString DominantSpecies;

    // Group State
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FVector GroupCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    FVector GroupDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float GroupRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
    float GroupMorale; // Overall group confidence/fear level

    // Behavior Timers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BehaviorStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float BehaviorDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float NextBehaviorCheck;

    // Group Memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> RecentLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownDangerAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownSafeAreas;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FVector> KnownFoodSources;

    FCrowdGroupData()
    {
        GroupID = -1;
        GroupName = TEXT("UnnamedGroup");
        CurrentBehavior = ECrowdBehaviorType::GrazingHerd;
        CurrentFormation = ECrowdFormation::Scattered;
        CurrentSize = 0;
        DominantSpecies = TEXT("Unknown");
        GroupCenter = FVector::ZeroVector;
        GroupDestination = FVector::ZeroVector;
        GroupRadius = 500.0f;
        GroupMorale = 0.5f;
        BehaviorStartTime = 0.0f;
        BehaviorDuration = 300.0f; // 5 minutes default
        NextBehaviorCheck = 0.0f;
    }
};

/**
 * Crowd Simulation Zone - defines areas with specific crowd behaviors
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCrowdSimulationZone
{
    GENERATED_BODY()

    // Zone Definition
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector ZoneCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FGameplayTagContainer ZoneTags;

    // Crowd Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    ECrowdDensity TargetDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    int32 MaxEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crowd")
    TArray<FCrowdEntityConfig> AllowedEntityTypes;

    // Behavior Modifiers
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    TMap<ECrowdBehaviorType, float> BehaviorWeights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float AggressionModifier; // Affects how aggressive entities are in this zone

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FearModifier; // Affects how easily entities become afraid

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float ActivityLevel; // How active/energetic entities are

    // Environmental Factors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsSafeZone; // No predators allowed

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsWaterSource; // Attracts thirsty entities

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsFoodSource; // Attracts hungry entities

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    bool bIsNestingArea; // Breeding/nesting behavior

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float NoiseLevel; // Ambient noise affects perception

    FCrowdSimulationZone()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneCenter = FVector::ZeroVector;
        ZoneRadius = 5000.0f;
        TargetDensity = ECrowdDensity::Medium;
        MaxEntities = 200;
        AggressionModifier = 1.0f;
        FearModifier = 1.0f;
        ActivityLevel = 1.0f;
        bIsSafeZone = false;
        bIsWaterSource = false;
        bIsFoodSource = false;
        bIsNestingArea = false;
        NoiseLevel = 0.5f;
    }
};

/**
 * Main Crowd Simulation Subsystem
 * Manages up to 50,000 simultaneous entities using Mass AI
 */
UCLASS()
class TRANSPERSONALGAME_API UCrowdSimulationSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // Subsystem Interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Core Crowd Management
    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void InitializeCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void ShutdownCrowdSimulation();

    UFUNCTION(BlueprintCallable, Category = "Crowd Simulation")
    void UpdateCrowdSimulation(float DeltaTime);

    // Zone Management
    UFUNCTION(BlueprintCallable, Category = "Zones")
    int32 CreateSimulationZone(const FCrowdSimulationZone& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Zones")
    bool RemoveSimulationZone(int32 ZoneID);

    UFUNCTION(BlueprintCallable, Category = "Zones")
    void UpdateZoneDensity(int32 ZoneID, ECrowdDensity NewDensity);

    UFUNCTION(BlueprintCallable, Category = "Zones")
    TArray<FCrowdSimulationZone> GetAllZones() const;

    // Group Management
    UFUNCTION(BlueprintCallable, Category = "Groups")
    int32 CreateCrowdGroup(const FCrowdEntityConfig& EntityConfig, int32 GroupSize, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Groups")
    bool DisbandCrowdGroup(int32 GroupID);

    UFUNCTION(BlueprintCallable, Category = "Groups")
    void MergeGroups(int32 GroupID1, int32 GroupID2);

    UFUNCTION(BlueprintCallable, Category = "Groups")
    void SplitGroup(int32 GroupID, float SplitRatio = 0.5f);

    // Behavior Control
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetGroupBehavior(int32 GroupID, ECrowdBehaviorType NewBehavior);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetGroupDestination(int32 GroupID, FVector Destination);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void TriggerGroupResponse(int32 GroupID, ECrowdResponse Response, FVector StimulusLocation);

    // Player Interaction
    UFUNCTION(BlueprintCallable, Category = "Player")
    void OnPlayerEnterZone(int32 ZoneID, APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Player")
    void OnPlayerExitZone(int32 ZoneID, APawn* Player);

    UFUNCTION(BlueprintCallable, Category = "Player")
    void OnPlayerMakeNoise(FVector Location, float NoiseLevel);

    UFUNCTION(BlueprintCallable, Category = "Player")
    void OnPlayerUseFire(FVector Location, float FireIntensity);

    // Query Functions
    UFUNCTION(BlueprintCallable, Category = "Query")
    TArray<FCrowdGroupData> GetGroupsInRadius(FVector Center, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Query")
    int32 GetEntityCountInZone(int32 ZoneID) const;

    UFUNCTION(BlueprintCallable, Category = "Query")
    float GetZoneDensity(int32 ZoneID) const;

    UFUNCTION(BlueprintCallable, Category = "Query")
    ECrowdBehaviorType GetDominantBehaviorInArea(FVector Center, float Radius) const;

    // Performance Management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetMaxSimulatedEntities(int32 MaxEntities);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDynamicLOD(bool bEnable);

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawZones(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawGroups(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugDrawEntityPaths(bool bEnable);

protected:
    // Core Systems
    UPROPERTY()
    UMassEntitySubsystem* MassEntitySubsystem;

    UPROPERTY()
    UMassSpawnerSubsystem* MassSpawnerSubsystem;

    UPROPERTY()
    UMassSimulationSubsystem* MassSimulationSubsystem;

    UPROPERTY()
    UMassNavigationSubsystem* MassNavigationSubsystem;

    UPROPERTY()
    UZoneGraphSubsystem* ZoneGraphSubsystem;

    // Simulation Data
    UPROPERTY()
    TArray<FCrowdSimulationZone> SimulationZones;

    UPROPERTY()
    TArray<FCrowdGroupData> CrowdGroups;

    UPROPERTY()
    TMap<int32, FCrowdEntityConfig> EntityConfigurations;

    // Performance Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxSimulatedEntities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDynamicLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

    // Debug Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawGroups;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bDebugDrawPaths;

private:
    // Internal Management
    void UpdateZones(float DeltaTime);
    void UpdateGroups(float DeltaTime);
    void UpdateEntityBehaviors(float DeltaTime);
    void ManagePerformance(float DeltaTime);

    // Group Behavior Logic
    void ProcessGroupBehavior(FCrowdGroupData& Group, float DeltaTime);
    void UpdateGroupFormation(FCrowdGroupData& Group);
    void HandleGroupInteractions(FCrowdGroupData& Group);

    // Utility Functions
    int32 GetNextGroupID();
    int32 GetNextZoneID();
    FCrowdSimulationZone* FindZoneByID(int32 ZoneID);
    FCrowdGroupData* FindGroupByID(int32 GroupID);

    // Counters
    int32 NextGroupID;
    int32 NextZoneID;
    float LastUpdateTime;
};
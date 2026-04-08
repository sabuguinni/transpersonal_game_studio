#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorSystem.generated.h"

/**
 * Core NPC Behavior System for Transpersonal Game Studio
 * Manages autonomous dinosaur behaviors, routines, and social interactions
 * Based on the principle that NPCs live their own lives independent of the player
 */

UENUM(BlueprintType)
enum class EDinosaurArchetype : uint8
{
    // Herbivores - Potential domestication candidates
    SmallHerbivore      UMETA(DisplayName = "Small Herbivore"),     // Compsognathus, Microraptor
    MediumHerbivore     UMETA(DisplayName = "Medium Herbivore"),    // Parasaurolophus, Triceratops
    LargeHerbivore      UMETA(DisplayName = "Large Herbivore"),     // Brontosaurus, Diplodocus
    
    // Carnivores - Threat levels
    SmallCarnivore      UMETA(DisplayName = "Small Carnivore"),     // Velociraptor, Dilophosaurus
    MediumCarnivore     UMETA(DisplayName = "Medium Carnivore"),    // Allosaurus, Carnotaurus
    LargeCarnivore      UMETA(DisplayName = "Large Carnivore"),     // T-Rex, Spinosaurus
    
    // Special behaviors
    Scavenger          UMETA(DisplayName = "Scavenger"),           // Opportunistic feeders
    PackHunter         UMETA(DisplayName = "Pack Hunter"),         // Coordinated hunting
    TerritorialGuard   UMETA(DisplayName = "Territorial Guard"),   // Defends specific areas
    
    MAX                UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDinosaurMoodState : uint8
{
    Calm               UMETA(DisplayName = "Calm"),
    Alert              UMETA(DisplayName = "Alert"),
    Aggressive         UMETA(DisplayName = "Aggressive"),
    Fearful            UMETA(DisplayName = "Fearful"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Feeding            UMETA(DisplayName = "Feeding"),
    Resting            UMETA(DisplayName = "Resting"),
    Socializing        UMETA(DisplayName = "Socializing"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Curious            UMETA(DisplayName = "Curious"),
    
    MAX                UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild               UMETA(DisplayName = "Wild"),               // 0% - Completely wild
    Wary               UMETA(DisplayName = "Wary"),               // 25% - Notices player but flees
    Neutral            UMETA(DisplayName = "Neutral"),            // 50% - Tolerates player presence
    Curious            UMETA(DisplayName = "Curious"),            // 75% - Approaches player cautiously
    Bonded             UMETA(DisplayName = "Bonded"),             // 100% - Fully domesticated
    
    MAX                UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Core personality traits (0.0 to 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;

    // Unique identifier for this individual
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IndividualID;
    
    // Physical variation seed for procedural differences
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PhysicalVariationSeed = 0;
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Player interaction history
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<float> PlayerEncounterTimes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> PlayerEncounterLocations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions = 0;
    
    // Territory and routine memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector HomeTerritory = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> FavoriteLocations;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> DangerousLocations;
    
    // Social memory
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownAllies;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownThreats;
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    // Time-based activities (0.0 = midnight, 0.5 = noon, 1.0 = midnight)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<float, FString> ScheduledActivities;
    
    // Preferred locations for different activities
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector FeedingLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector RestingLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PatrolRoute_Start = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PatrolRoute_End = FVector::ZeroVector;
    
    // Routine flexibility (0.0 = rigid, 1.0 = completely flexible)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float RoutineFlexibility = 0.3f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core dinosaur properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    EDinosaurArchetype DinosaurType = EDinosaurArchetype::SmallHerbivore;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Identity")
    FDinosaurPersonality Personality;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur State")
    EDinosaurMoodState CurrentMood = EDinosaurMoodState::Calm;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DomesticationProgress = 0.0f;

    // Memory and learning systems
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory Memory;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FDailyRoutine DailyRoutine;

    // Behavior parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float SightRange = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "10000.0"))
    float HearingRange = 2000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float SightAngle = 120.0f;

    // Blackboard keys for behavior trees
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName PlayerActorKey = "PlayerActor";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName CurrentMoodKey = "CurrentMood";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName TargetLocationKey = "TargetLocation";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    FName ThreatLevelKey = "ThreatLevel";

    // Public interface functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateMood(EDinosaurMoodState NewMood);
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddPlayerInteraction(bool bWasPositive, FVector InteractionLocation);
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetTrustLevel() const;
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool CanBeDomesticated() const;
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetCurrentActivityLocation() const;
    
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void GenerateUniquePersonality(int32 Seed);

private:
    // Internal state tracking
    float LastMoodChangeTime = 0.0f;
    float LastPlayerInteractionTime = 0.0f;
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    
    // Helper functions
    void UpdateDailyRoutine();
    void ProcessMemories();
    void UpdateDomesticationProgress(float DeltaTime);
    EDinosaurMoodState CalculateNewMood();
    
    // Blackboard updates
    void UpdateBlackboardValues();
    
    // References
    UPROPERTY()
    UBlackboardComponent* BlackboardComponent = nullptr;
    
    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent = nullptr;
    
    UPROPERTY()
    UAIPerceptionComponent* PerceptionComponent = nullptr;
};
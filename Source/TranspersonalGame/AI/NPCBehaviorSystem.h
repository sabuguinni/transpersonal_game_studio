#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
// DISABLED: #include "Perception/PerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EAI_DinosaurSpecies_ABB : uint8
{
    Peaceful        UMETA(DisplayName = "Peaceful"),
    Aggressive      UMETA(DisplayName = "Aggressive"), 
    Curious         UMETA(DisplayName = "Curious"),
    Fearful         UMETA(DisplayName = "Fearful"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary")
};

UENUM(BlueprintType)
enum class EAI_DinosaurBehaviorState_ABB : uint8
{
    Idle              UMETA(DisplayName = "Idle"),
    Foraging          UMETA(DisplayName = "Foraging"),
    Hunting           UMETA(DisplayName = "Hunting"),
    Drinking          UMETA(DisplayName = "Drinking"),
    Resting           UMETA(DisplayName = "Resting"),
    Socializing       UMETA(DisplayName = "Socializing"),
    Territorial       UMETA(DisplayName = "Territorial"),
    Fleeing           UMETA(DisplayName = "Fleeing"),
    Investigating     UMETA(DisplayName = "Investigating"),
    Patrolling        UMETA(DisplayName = "Patrolling"),
    Mating            UMETA(DisplayName = "Mating"),
    Nesting           UMETA(DisplayName = "Nesting")
};

UENUM(BlueprintType)
enum class EAI_DomesticationLevel_ABB : uint8
{
    Wild              UMETA(DisplayName = "Wild"),
    Wary              UMETA(DisplayName = "Wary"),
    Curious           UMETA(DisplayName = "Curious"),
    Tolerant          UMETA(DisplayName = "Tolerant"),
    Friendly          UMETA(DisplayName = "Friendly"),
    Bonded            UMETA(DisplayName = "Bonded"),
    Domesticated      UMETA(DisplayName = "Domesticated")
};

USTRUCT(BlueprintType)
struct FAI_DinosaurPersonality_ABB
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> KnownActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FVector> InterestingLocations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> ActorThreatLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastPlayerInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bPlayerIsKnown;

    FNPCMemory()
    {
        LastPlayerInteractionTime = 0.0f;
        bPlayerIsKnown = false;
    }
};

USTRUCT(BlueprintType)
struct FAI_DinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hour of day (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duration in hours

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCState Activity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, float> KnownActorsThreatLevel; // -1.0 (amigo) a 1.0 (inimigo)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<AActor*, FDateTime> LastSeenActors;
    
    // Memória do jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownPlayerLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastPlayerEncounter;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f; // -1.0 (hostil) a 1.0 (confiança total)
    
    // Contador de interações positivas/negativas com o jogador
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositivePlayerInteractions = 0;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativePlayerInteractions = 0;
};

USTRUCT(BlueprintType)
struct FAI_DinosaurNeeds_ABB
{
    GENERATED_BODY()

    // Necessidades básicas (0.0 = satisfeito, 1.0 = crítico)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hunger = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thirst = 0.2f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fatigue = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SocialNeed = 0.4f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float TerritorialSecurity = 0.6f;
    
    // Necessidades reprodutivas (sazonais)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MatingUrge = 0.0f;
    
    // Taxa de degradação das necessidades (por hora)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float HungerDecayRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThirstDecayRate = 0.15f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FatigueDecayRate = 0.05f;
};

/**
 * Componente principal que gere o comportamento complexo dos dinossauros NPCs
 * Integra Behavior Trees, memória persistente, personalidade e necessidades
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO BÁSICA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    EAI_DinosaurSpecies_ABB Species = EAI_DinosaurSpecies_ABB::Compsognathus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FAI_DinosaurPersonality_ABB Personality;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Config")
    FString IndividualName; // Nome único gerado proceduralmente
    
    // === BEHAVIOR TREE INTEGRATION ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* BehaviorTreeAsset;
    
// [UHT-FIX]     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardData* BlackboardAsset;
    
    // === ESTADO COMPORTAMENTAL ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    EAI_DinosaurBehaviorState_ABB CurrentBehaviorState = EAI_DinosaurBehaviorState_ABB::Idle;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    FAI_DinosaurNeeds_ABB CurrentNeeds;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior State")
    FAI_DinosaurMemory Memory;
    
    // === DOMESTICAÇÃO ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Domestication")
    EAI_DomesticationLevel_ABB DomesticationLevel = EAI_DomesticationLevel_ABB::Wild;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    bool bCanBeDomesticated = false;
    
    // === ROTINAS DIÁRIAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    TArray<FVector> PatrolPoints;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    float RoutineVariation = 0.2f; // Quanto a rotina pode variar (0.0 = rígida, 1.0 = totalmente aleatória)
    
    // === FUNÇÕES PÚBLICAS ===
    
    // Gestão de estado comportamental
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EAI_DinosaurBehaviorState_ABB NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EAI_DinosaurBehaviorState_ABB GetBehaviorState() const { return CurrentBehaviorState; }
    
    // Gestão de necessidades
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    float GetMostUrgentNeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void SatisfyNeed(const FString& NeedType, float Amount);
    
    // Gestão de memória
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberLocation(FVector Location, const FString& LocationType);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float ThreatLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    FVector GetNearestKnownLocation(const FString& LocationType) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool HasSeenActorRecently(AActor* Actor, float TimeThreshold = 300.0f) const;
    
    // Domesticação
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(bool bPositive, float Intensity = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void UpdateDomesticationLevel();
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanAcceptFood() const;
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool WillFollowPlayer() const;
    
    // Utilidades para Behavior Trees
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    void UpdateBlackboardValues();
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    FVector GetBestLocationForCurrentNeed() const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    bool ShouldFleeFromActor(AActor* Actor) const;
    
    UFUNCTION(BlueprintCallable, Category = "AI Utils")
    bool ShouldInvestigateActor(AActor* Actor) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCState CurrentState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    int32 NPCAge;

    // Behavior Tree Integration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBlackboardAsset* BlackboardAsset;

    // Memory System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FNPCMemory NPCMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryRetentionTime;

    // Daily Routine System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPCRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bFollowDailyRoutine;

    // Social System
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TArray<AActor*> KnownNPCs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social")
    TMap<AActor*, float> RelationshipValues; // -1.0 (enemy) to 1.0 (friend)

    // Perception Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float SightRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float HearingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
    float PeripheralVisionAngle;

    // Behavior Functions
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetNPCState(ENPCState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCState GetCurrentState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void RememberActor(AActor* Actor, float ThreatLevel = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    bool IsActorKnown(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FNPCRoutine GetCurrentRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "Routine")
    void AddRoutine(const FNPCRoutine& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "Social")
    void UpdateRelationship(AActor* OtherActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Social")
    float GetRelationshipValue(AActor* OtherActor) const;

    // AI Integration
    UFUNCTION(BlueprintCallable, Category = "AI")
    void StartBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void StopBehaviorTree();

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardValues();

protected:
    // Internal Functions
    void UpdateDailyRoutine();
    void ProcessMemory(float DeltaTime);
    void HandlePerception();
    float GetCurrentTimeOfDay() const;
    
    // Timers
    float MemoryCleanupTimer;
    float RoutineUpdateTimer;
    
    // References
    UPROPERTY()
    class AAIController* NPCAIController;
    
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
};
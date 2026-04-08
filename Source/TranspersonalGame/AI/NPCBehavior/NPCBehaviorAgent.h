#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/WorldSubsystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Components/ActorComponent.h"
#include "Engine/DataAsset.h"
#include "../NPCBehaviorSystem.h"
#include "NPCBehaviorAgent.generated.h"

class UBehaviorTree;
class UBlackboardAsset;
class AAIController;
class APawn;
class UNPCBehaviorComponent;

UENUM(BlueprintType)
enum class ENPCArchetype : uint8
{
    // Dinossauros Herbívoros Pequenos (Domesticáveis)
    SmallHerbivore_Timid        UMETA(DisplayName = "Small Herbivore (Timid)"),
    SmallHerbivore_Curious      UMETA(DisplayName = "Small Herbivore (Curious)"),
    SmallHerbivore_Social       UMETA(DisplayName = "Small Herbivore (Social)"),
    
    // Dinossauros Herbívoros Grandes (Neutros)
    LargeHerbivore_Gentle       UMETA(DisplayName = "Large Herbivore (Gentle)"),
    LargeHerbivore_Protective   UMETA(DisplayName = "Large Herbivore (Protective)"),
    LargeHerbivore_Territorial  UMETA(DisplayName = "Large Herbivore (Territorial)"),
    
    // Dinossauros Carnívoros Pequenos (Agressivos)
    SmallCarnivore_Pack         UMETA(DisplayName = "Small Carnivore (Pack Hunter)"),
    SmallCarnivore_Opportunist  UMETA(DisplayName = "Small Carnivore (Opportunist)"),
    SmallCarnivore_Ambush       UMETA(DisplayName = "Small Carnivore (Ambush)"),
    
    // Dinossauros Carnívoros Grandes (Apex)
    ApexPredator_Solitary       UMETA(DisplayName = "Apex Predator (Solitary)"),
    ApexPredator_Territorial    UMETA(DisplayName = "Apex Predator (Territorial)"),
    ApexPredator_Nomadic        UMETA(DisplayName = "Apex Predator (Nomadic)")
};

UENUM(BlueprintType)
enum class ENPCEmotionalState : uint8
{
    Calm                UMETA(DisplayName = "Calm"),
    Alert               UMETA(DisplayName = "Alert"),
    Curious             UMETA(DisplayName = "Curious"),
    Fearful             UMETA(DisplayName = "Fearful"),
    Aggressive          UMETA(DisplayName = "Aggressive"),
    Playful             UMETA(DisplayName = "Playful"),
    Protective          UMETA(DisplayName = "Protective"),
    Territorial         UMETA(DisplayName = "Territorial"),
    Stressed            UMETA(DisplayName = "Stressed"),
    Content             UMETA(DisplayName = "Content"),
    Bonded              UMETA(DisplayName = "Bonded"),
    Mourning            UMETA(DisplayName = "Mourning")
};

UENUM(BlueprintType)
enum class ENPCRelationshipType : uint8
{
    Unknown             UMETA(DisplayName = "Unknown"),
    Neutral             UMETA(DisplayName = "Neutral"),
    Friendly            UMETA(DisplayName = "Friendly"),
    Hostile             UMETA(DisplayName = "Hostile"),
    Fearful             UMETA(DisplayName = "Fearful"),
    Protective          UMETA(DisplayName = "Protective"),
    Bonded              UMETA(DisplayName = "Bonded"),
    Rival               UMETA(DisplayName = "Rival"),
    PackMember          UMETA(DisplayName = "Pack Member"),
    Offspring           UMETA(DisplayName = "Offspring"),
    Parent              UMETA(DisplayName = "Parent"),
    Mate                UMETA(DisplayName = "Mate")
};

USTRUCT(BlueprintType)
struct FNPCRelationship
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCRelationshipType RelationshipType = ENPCRelationshipType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float RelationshipStrength = 0.0f; // -1.0 (máxima hostilidade) a 1.0 (máxima afinidade)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastInteraction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 PositiveInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 NegativeInteractions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> SharedExperiences;

    FNPCRelationship()
    {
        LastInteraction = FDateTime::Now();
    }
};

USTRUCT(BlueprintType)
struct FNPCDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString RoutineName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTimeOfDay = 0.0f; // 0.0 = meia-noite, 0.5 = meio-dia, 1.0 = meia-noite seguinte

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration = 0.1f; // Duração em fracção do dia (0.1 = 2.4 horas)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState BehaviorState = EDinosaurBehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationTolerance = 500.0f; // Raio em que a actividade pode ocorrer

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority = 1; // 1 = baixa, 5 = alta prioridade

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCanBeInterrupted = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredConditions; // Condições que devem ser satisfeitas
};

USTRUCT(BlueprintType)
struct FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MemoryType; // "Location", "Actor", "Event", "Threat", etc.

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TWeakObjectPtr<AActor> RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight = 0.0f; // -1.0 (muito negativo) a 1.0 (muito positivo)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryStrength = 1.0f; // 0.0 = esquecido, 1.0 = muito vívido

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ReinforcementCount = 1; // Quantas vezes foi relembrado/reforçado

    FNPCMemoryEntry()
    {
        Timestamp = FDateTime::Now();
    }
};

/**
 * NPC Behavior Data Asset - Configuração de comportamento por arquétipo
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UNPCBehaviorDataAsset : public UDataAsset
{
    GENERATED_BODY()

public:
    // Configuração básica do arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetype")
    ENPCArchetype ArchetypeType = ENPCArchetype::SmallHerbivore_Timid;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetype")
    FString ArchetypeName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Archetype")
    FString ArchetypeDescription;

    // Behavior Tree e Blackboard
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    TSoftObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "AI")
    TSoftObjectPtr<UBlackboardAsset> Blackboard;

    // Personalidade base do arquétipo
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality")
    FDinosaurPersonality BasePersonality;

    // Variação permitida na personalidade (0.0 = sem variação, 1.0 = máxima variação)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PersonalityVariation = 0.3f;

    // Rotinas diárias padrão
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Daily Routines")
    TArray<FNPCDailyRoutine> DefaultDailyRoutines;

    // Necessidades base
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Needs")
    FDinosaurNeeds BaseNeeds;

    // Configurações de domesticação
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domestication")
    bool bCanBeDomesticated = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domestication")
    float DomesticationDifficulty = 0.5f; // 0.0 = fácil, 1.0 = muito difícil

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Domestication")
    TArray<FString> DomesticationRequirements;

    // Configurações sociais
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social")
    bool bIsPackAnimal = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social")
    int32 PreferredPackSize = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Social")
    float SocialRadius = 1000.0f; // Distância para reconhecer outros da mesma espécie

    // Configurações de memória
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    int32 MaxMemoryEntries = 50;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    float MemoryDecayRate = 0.01f; // Taxa de esquecimento por dia

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Memory")
    float ImportantMemoryThreshold = 0.7f; // Memórias acima deste peso não decaem
};

/**
 * Advanced NPC Behavior Component - Sistema completo de comportamento NPC
 * Integra personalidade, memória, relações sociais e rotinas diárias
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAdvancedNPCBehaviorComponent : public UNPCBehaviorComponent
{
    GENERATED_BODY()

public:
    UAdvancedNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO AVANÇADA ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Config")
    ENPCArchetype NPCArchetype = ENPCArchetype::SmallHerbivore_Timid;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Config")
    UNPCBehaviorDataAsset* BehaviorDataAsset;

    // === ESTADO EMOCIONAL ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotional State")
    ENPCEmotionalState CurrentEmotionalState = ENPCEmotionalState::Calm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Emotional State")
    float EmotionalIntensity = 0.5f; // 0.0 = baixa intensidade, 1.0 = alta intensidade

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float EmotionalStability = 0.7f; // Quão rapidamente as emoções mudam

    // === RELAÇÕES SOCIAIS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social Relations")
    TMap<AActor*, FNPCRelationship> SocialRelationships;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social Relations")
    TArray<TWeakObjectPtr<AActor>> PackMembers;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Social Relations")
    TWeakObjectPtr<AActor> PackLeader;

    // === ROTINAS DIÁRIAS ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    TArray<FNPCDailyRoutine> DailyRoutines;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Daily Routines")
    FNPCDailyRoutine* CurrentRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    float RoutineFlexibility = 0.2f; // Quão flexível é com horários

    // === MEMÓRIA AVANÇADA ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Advanced Memory")
    TArray<FNPCMemoryEntry> MemoryBank;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Memory")
    int32 MaxMemoryCapacity = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Advanced Memory")
    float MemoryConsolidationRate = 0.1f; // Taxa de consolidação de memórias

    // === FUNÇÕES PÚBLICAS AVANÇADAS ===

    // Gestão de estado emocional
    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void SetEmotionalState(ENPCEmotionalState NewState, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    void ModifyEmotionalIntensity(float DeltaIntensity);

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    ENPCEmotionalState GetEmotionalState() const { return CurrentEmotionalState; }

    UFUNCTION(BlueprintCallable, Category = "Emotional State")
    float GetEmotionalIntensity() const { return EmotionalIntensity; }

    // Gestão de relações sociais
    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    void EstablishRelationship(AActor* OtherActor, ENPCRelationshipType RelationType, float Strength = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    void ModifyRelationship(AActor* OtherActor, float DeltaStrength);

    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    FNPCRelationship GetRelationshipWith(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    TArray<AActor*> GetActorsWithRelationType(ENPCRelationshipType RelationType) const;

    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    void JoinPack(const TArray<AActor*>& NewPackMembers, AActor* Leader = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Social Relations")
    void LeavePack();

    // Gestão de rotinas diárias
    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    void AddDailyRoutine(const FNPCDailyRoutine& NewRoutine);

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    void RemoveDailyRoutine(const FString& RoutineName);

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    FNPCDailyRoutine* GetCurrentRoutine() const { return CurrentRoutine; }

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    FNPCDailyRoutine* GetRoutineForTimeOfDay(float TimeOfDay) const;

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    void InterruptCurrentRoutine(const FString& Reason);

    // Gestão de memória avançada
    UFUNCTION(BlueprintCallable, Category = "Advanced Memory")
    void AddMemory(const FString& MemoryType, const FVector& Location, AActor* RelatedActor, 
                   const FString& Description, float EmotionalWeight = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Advanced Memory")
    TArray<FNPCMemoryEntry> GetMemoriesByType(const FString& MemoryType) const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Memory")
    TArray<FNPCMemoryEntry> GetMemoriesAboutActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Advanced Memory")
    void ReinforceMemory(int32 MemoryIndex);

    UFUNCTION(BlueprintCallable, Category = "Advanced Memory")
    void ForgetMemory(int32 MemoryIndex);

    // Comportamentos emergentes
    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    void ReactToEvent(const FString& EventType, AActor* EventSource, const FVector& EventLocation, float EventIntensity);

    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    bool ShouldReactToActor(AActor* OtherActor) const;

    UFUNCTION(BlueprintCallable, Category = "Emergent Behavior")
    void UpdateBehaviorBasedOnContext();

protected:
    // Funções internas
    void UpdateEmotionalState(float DeltaTime);
    void UpdateDailyRoutines(float DeltaTime);
    void UpdateMemorySystem(float DeltaTime);
    void UpdateSocialRelationships(float DeltaTime);
    
    void ProcessMemoryDecay(float DeltaTime);
    void ConsolidateImportantMemories();
    void ManageMemoryCapacity();
    
    void EvaluateCurrentSituation();
    void MakeContextualDecisions();
    
    // Configuração inicial
    void InitializeFromDataAsset();
    void GeneratePersonalityVariation();
    void SetupDefaultRoutines();

private:
    // Timers e cache
    float EmotionalUpdateTimer = 0.0f;
    float MemoryUpdateTimer = 0.0f;
    float SocialUpdateTimer = 0.0f;
    float RoutineUpdateTimer = 0.0f;
    
    // Estado interno
    bool bIsInitialized = false;
    float CurrentTimeOfDay = 0.0f;
    FNPCDailyRoutine* PreviousRoutine = nullptr;
};

/**
 * NPC Behavior Agent - Sistema principal que coordena todos os NPCs
 * Este é o "sociólogo" do mundo - define como os NPCs se relacionam entre si
 */
UCLASS()
class TRANSPERSONALGAME_API UNPCBehaviorAgent : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // === GESTÃO GLOBAL DE NPCs ===
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior Agent")
    void RegisterNPC(APawn* NPC, UAdvancedNPCBehaviorComponent* BehaviorComponent);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior Agent")
    void UnregisterNPC(APawn* NPC);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior Agent")
    TArray<APawn*> GetAllNPCs() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior Agent")
    TArray<APawn*> GetNPCsByArchetype(ENPCArchetype ArchetypeType) const;

    // === EVENTOS GLOBAIS ===
    UFUNCTION(BlueprintCallable, Category = "Global Events")
    void BroadcastGlobalEvent(const FString& EventType, const FVector& EventLocation, 
                             float EventRadius, float EventIntensity, AActor* EventSource = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Global Events")
    void NotifyNPCDeath(APawn* DeadNPC, AActor* Killer = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Global Events")
    void NotifyPlayerAction(const FString& ActionType, const FVector& ActionLocation, float ActionIntensity);

    // === DINÂMICAS SOCIAIS ===
    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void FormPack(const TArray<APawn*>& NPCs, APawn* Leader = nullptr);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void DisbandPack(const TArray<APawn*>& PackMembers);

    UFUNCTION(BlueprintCallable, Category = "Social Dynamics")
    void EstablishTerritorialBoundaries(APawn* TerritorialNPC, const FVector& CenterLocation, float Radius);

    // === ANÁLISE E ESTATÍSTICAS ===
    UFUNCTION(BlueprintCallable, Category = "Analytics")
    int32 GetTotalNPCCount() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    TMap<ENPCArchetype, int32> GetArchetypeDistribution() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    float GetAverageNPCHappiness() const;

    UFUNCTION(BlueprintCallable, Category = "Analytics")
    TArray<FString> GetMostCommonNPCActivities() const;

protected:
    // Registry de NPCs
    UPROPERTY()
    TMap<TWeakObjectPtr<APawn>, TWeakObjectPtr<UAdvancedNPCBehaviorComponent>> RegisteredNPCs;

    UPROPERTY()
    TMap<ENPCArchetype, TArray<TWeakObjectPtr<APawn>>> NPCsByArchetype;

    // Configurações globais
    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float GlobalEventUpdateInterval = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    float SocialDynamicsUpdateInterval = 5.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Configuration")
    int32 MaxSimultaneousNPCs = 200;

private:
    // Timers
    FTimerHandle GlobalEventTimerHandle;
    FTimerHandle SocialDynamicsTimerHandle;
    
    // Funções internas
    void UpdateGlobalEvents();
    void UpdateSocialDynamics();
    void ProcessNPCInteractions();
    void ManageNPCPopulation();
    
    // Cache e optimização
    mutable FCriticalSection NPCRegistryMutex;
    TArray<FString> CachedCommonActivities;
    float LastAnalyticsUpdate = 0.0f;
};
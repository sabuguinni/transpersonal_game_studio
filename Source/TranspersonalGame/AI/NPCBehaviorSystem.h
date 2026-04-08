#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorSystem.generated.h"

// Enums para estados comportamentais
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle UMETA(DisplayName = "Idle"),
    Foraging UMETA(DisplayName = "Foraging"),
    Drinking UMETA(DisplayName = "Drinking"),
    Socializing UMETA(DisplayName = "Socializing"),
    Resting UMETA(DisplayName = "Resting"),
    Hunting UMETA(DisplayName = "Hunting"),
    Fleeing UMETA(DisplayName = "Fleeing"),
    Territorial UMETA(DisplayName = "Territorial"),
    Mating UMETA(DisplayName = "Mating"),
    Parenting UMETA(DisplayName = "Parenting"),
    Migrating UMETA(DisplayName = "Migrating"),
    Investigating UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive UMETA(DisplayName = "Aggressive"),
    Cautious UMETA(DisplayName = "Cautious"),
    Curious UMETA(DisplayName = "Curious"),
    Social UMETA(DisplayName = "Social"),
    Solitary UMETA(DisplayName = "Solitary"),
    Territorial UMETA(DisplayName = "Territorial"),
    Docile UMETA(DisplayName = "Docile"),
    Skittish UMETA(DisplayName = "Skittish")
};

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    Triceratops UMETA(DisplayName = "Triceratops"),
    Parasaurolophus UMETA(DisplayName = "Parasaurolophus"),
    Stegosaurus UMETA(DisplayName = "Stegosaurus"),
    Ankylosaurus UMETA(DisplayName = "Ankylosaurus"),
    Compsognathus UMETA(DisplayName = "Compsognathus"),
    Gallimimus UMETA(DisplayName = "Gallimimus"),
    Velociraptor UMETA(DisplayName = "Velociraptor"),
    Dilophosaurus UMETA(DisplayName = "Dilophosaurus"),
    Allosaurus UMETA(DisplayName = "Allosaurus"),
    TyrannosaurusRex UMETA(DisplayName = "Tyrannosaurus Rex"),
    Spinosaurus UMETA(DisplayName = "Spinosaurus"),
    Carnotaurus UMETA(DisplayName = "Carnotaurus")
};

UENUM(BlueprintType)
enum class EDinosaurThreatLevel : uint8
{
    Harmless UMETA(DisplayName = "Harmless"),
    LowThreat UMETA(DisplayName = "Low Threat"),
    ModerateThreat UMETA(DisplayName = "Moderate Threat"),
    HighThreat UMETA(DisplayName = "High Threat"),
    ApexPredator UMETA(DisplayName = "Apex Predator")
};

UENUM(BlueprintType)
enum class EDomesticationStage : uint8
{
    Wild UMETA(DisplayName = "Wild"),
    Aware UMETA(DisplayName = "Aware"),
    Curious UMETA(DisplayName = "Curious"),
    Cautious UMETA(DisplayName = "Cautious"),
    Accepting UMETA(DisplayName = "Accepting"),
    Trusting UMETA(DisplayName = "Trusting"),
    Bonded UMETA(DisplayName = "Bonded"),
    Domesticated UMETA(DisplayName = "Domesticated")
};

// Estrutura para memória de NPCs
USTRUCT(BlueprintType)
struct FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FamiliarityLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPlayerRelated;

    FNPCMemoryEntry()
    {
        Actor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
        ThreatLevel = 0.0f;
        FamiliarityLevel = 0.0f;
        bIsPlayerRelated = false;
    }
};

// Estrutura para rotinas diárias
USTRUCT(BlueprintType)
struct FDailyRoutineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeOfDay; // 0.0 = midnight, 0.5 = noon, 1.0 = midnight

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority; // 0.0 - 1.0

    FDailyRoutineEntry()
    {
        TimeOfDay = 0.0f;
        BehaviorState = EDinosaurBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        Duration = 1.0f;
        Priority = 0.5f;
    }
};

// Estrutura para características individuais
USTRUCT(BlueprintType)
struct FDinosaurIndividualTraits
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString IndividualName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurPersonality PrimaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurPersonality SecondaryPersonality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AggressionLevel; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CuriosityLevel; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SocialLevel; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float IntelligenceLevel; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FearThreshold; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TerritorialRadius; // Em unidades UE

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> PreferredCompanions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EDinosaurSpecies> NaturalEnemies;

    FDinosaurIndividualTraits()
    {
        IndividualName = TEXT("Unnamed");
        PrimaryPersonality = EDinosaurPersonality::Cautious;
        SecondaryPersonality = EDinosaurPersonality::Curious;
        AggressionLevel = 0.5f;
        CuriosityLevel = 0.5f;
        SocialLevel = 0.5f;
        IntelligenceLevel = 0.5f;
        FearThreshold = 0.5f;
        TerritorialRadius = 1000.0f;
    }
};

/**
 * Sistema principal de comportamento para NPCs
 * Gerencia rotinas, memória, domesticação e personalidades individuais
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
    // Propriedades básicas do NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    EDinosaurSpecies Species;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    EDinosaurThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FDinosaurIndividualTraits IndividualTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    EDomesticationStage CurrentDomesticationStage;

    // Sistema de memória
    UPROPERTY(BlueprintReadOnly, Category = "Memory System")
    TArray<FNPCMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    float MemoryDuration; // Tempo em segundos que as memórias persistem

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory System")
    int32 MaxMemoryEntries;

    // Sistema de rotinas
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routines")
    TArray<FDailyRoutineEntry> DailyRoutine;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routines")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Daily Routines")
    FDailyRoutineEntry CurrentRoutineEntry;

    // Sistema de domesticação
    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float DomesticationProgress; // 0.0 - 1.0

    UPROPERTY(BlueprintReadOnly, Category = "Domestication")
    float PlayerFamiliarity; // 0.0 - 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Domestication")
    float DomesticationRate; // Velocidade de domesticação

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Memory System")
    void AddMemoryEntry(AActor* Actor, float ThreatLevel, bool bIsPlayerRelated = false);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    FNPCMemoryEntry GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Memory System")
    bool HasMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Daily Routines")
    FDailyRoutineEntry GetCurrentRoutineForTime(float TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessPlayerInteraction(float InteractionQuality, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanAdvanceDomestication();

    UFUNCTION(BlueprintPure, Category = "Behavior")
    float GetReactionToActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    bool ShouldFleeFromActor(AActor* Actor);

    UFUNCTION(BlueprintPure, Category = "Behavior")
    bool ShouldInvestigateActor(AActor* Actor);

private:
    // Funções internas
    void UpdateMemory(float DeltaTime);
    void UpdateDailyRoutine(float DeltaTime);
    void UpdateDomestication(float DeltaTime);
    float CalculateActorThreatLevel(AActor* Actor);
    void CleanupOldMemories();
    
    // Timers internos
    float MemoryUpdateTimer;
    float RoutineUpdateTimer;
    float DomesticationUpdateTimer;
    
    // Cache para performance
    float CachedTimeOfDay;
    bool bRoutineNeedsUpdate;
};
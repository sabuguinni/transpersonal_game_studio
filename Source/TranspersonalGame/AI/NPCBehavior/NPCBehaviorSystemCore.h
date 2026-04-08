#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/ActorComponent.h"
#include "NPCBehaviorSystemCore.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNPCBehavior, Log, All);

// Enums para classificação de comportamentos
UENUM(BlueprintType)
enum class ENPCSpeciesType : uint8
{
    SmallHerbivore      UMETA(DisplayName = "Small Herbivore"),
    LargeHerbivore      UMETA(DisplayName = "Large Herbivore"),
    SmallCarnivore      UMETA(DisplayName = "Small Carnivore"),
    LargeCarnivore      UMETA(DisplayName = "Large Carnivore"),
    Omnivore           UMETA(DisplayName = "Omnivore"),
    Scavenger          UMETA(DisplayName = "Scavenger")
};

UENUM(BlueprintType)
enum class ENPCBehaviorState : uint8
{
    Idle               UMETA(DisplayName = "Idle"),
    Foraging           UMETA(DisplayName = "Foraging"),
    Hunting            UMETA(DisplayName = "Hunting"),
    Fleeing            UMETA(DisplayName = "Fleeing"),
    Socializing        UMETA(DisplayName = "Socializing"),
    Resting            UMETA(DisplayName = "Resting"),
    Drinking           UMETA(DisplayName = "Drinking"),
    Territorial        UMETA(DisplayName = "Territorial"),
    Mating             UMETA(DisplayName = "Mating"),
    Nesting            UMETA(DisplayName = "Nesting"),
    Migrating          UMETA(DisplayName = "Migrating"),
    Investigating      UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class ENPCThreatLevel : uint8
{
    None               UMETA(DisplayName = "No Threat"),
    Low                UMETA(DisplayName = "Low Threat"),
    Medium             UMETA(DisplayName = "Medium Threat"),
    High               UMETA(DisplayName = "High Threat"),
    Extreme            UMETA(DisplayName = "Extreme Threat")
};

UENUM(BlueprintType)
enum class ENPCDomesticationLevel : uint8
{
    Wild               UMETA(DisplayName = "Wild"),
    Wary               UMETA(DisplayName = "Wary"),
    Curious            UMETA(DisplayName = "Curious"),
    Tolerant           UMETA(DisplayName = "Tolerant"),
    Friendly           UMETA(DisplayName = "Friendly"),
    Bonded             UMETA(DisplayName = "Bonded"),
    Domesticated       UMETA(DisplayName = "Domesticated")
};

// Estruturas de dados comportamentais
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCMemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LastKnownLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCThreatLevel ThreatLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MemoryStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LastSeenTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString InteractionHistory;

    FNPCMemoryEntry()
    {
        TargetActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        ThreatLevel = ENPCThreatLevel::None;
        MemoryStrength = 1.0f;
        LastSeenTime = 0.0f;
        InteractionHistory = TEXT("");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCPersonalityTraits
{
    GENERATED_BODY()

    // Traços de personalidade (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Patience;

    FNPCPersonalityTraits()
    {
        Aggressiveness = 0.5f;
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Fearfulness = 0.5f;
        Territoriality = 0.5f;
        Intelligence = 0.5f;
        Patience = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hora do dia (0.0 = meia-noite, 0.5 = meio-dia)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duração em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority; // 0.0 a 1.0

    FNPCDailyRoutine()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        BehaviorState = ENPCBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPCNeedsState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Hunger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Thirst;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Energy;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Social;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Safety;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Comfort;

    FNPCNeedsState()
    {
        Hunger = 0.5f;
        Thirst = 0.5f;
        Energy = 1.0f;
        Social = 0.5f;
        Safety = 1.0f;
        Comfort = 0.5f;
    }
};

/**
 * Componente central do sistema de comportamento de NPCs
 * Gerencia memória, personalidade, rotinas e estados comportamentais
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorSystemCore : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystemCore();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração base do NPC
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Configuration")
    ENPCSpeciesType SpeciesType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Configuration")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Configuration")
    int32 UniqueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Configuration")
    FNPCPersonalityTraits PersonalityTraits;

    // Sistema de necessidades
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Needs")
    FNPCNeedsState CurrentNeeds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Needs")
    float NeedsDecayRate;

    // Sistema de memória
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    TArray<FNPCMemoryEntry> MemoryEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemoryEntries;

    // Rotinas diárias
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    TArray<FNPCDailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Routine")
    float RoutineFlexibility; // Quão flexível é em seguir rotinas

    // Estado atual
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    ENPCDomesticationLevel DomesticationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC State")
    float PlayerRelationship; // -1.0 (hostil) a 1.0 (amigável)

    // Território
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Territory")
    bool bHasTerritory;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void AddMemoryEntry(AActor* Target, ENPCThreatLevel ThreatLevel, const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPCMemoryEntry* GetMemoryEntry(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCBehaviorState GetMostUrgentNeed();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFollowRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPCDailyRoutine GetCurrentRoutine();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void ModifyPlayerRelationship(float Delta);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool IsInTerritory(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    float GetDistanceFromTerritory(const FVector& Location);

private:
    void DecayMemories(float DeltaTime);
    void ProcessRoutines();
    float GetCurrentTimeOfDay();
    void UpdateBehaviorState();
};
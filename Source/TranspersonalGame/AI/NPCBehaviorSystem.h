#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorSystem.generated.h"

// Forward declarations
class UBehaviorTree;
class UBlackboardAsset;
class ADinosaurNPC;

/**
 * Enumeração dos tipos de personalidade dos dinossauros
 * Baseado no conceito de que cada dinossauro tem uma vida própria
 */
UENUM(BlueprintType)
enum class EDinosaurPersonality : uint8
{
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Cautious        UMETA(DisplayName = "Cautious"), 
    Curious         UMETA(DisplayName = "Curious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Social          UMETA(DisplayName = "Social"),
    Solitary        UMETA(DisplayName = "Solitary"),
    Protective      UMETA(DisplayName = "Protective"),
    Skittish        UMETA(DisplayName = "Skittish")
};

/**
 * Estados comportamentais básicos dos dinossauros
 */
UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle            UMETA(DisplayName = "Idle"),
    Foraging        UMETA(DisplayName = "Foraging"),
    Hunting         UMETA(DisplayName = "Hunting"),
    Drinking        UMETA(DisplayName = "Drinking"),
    Resting         UMETA(DisplayName = "Resting"),
    Socializing     UMETA(DisplayName = "Socializing"),
    Fleeing         UMETA(DisplayName = "Fleeing"),
    Investigating   UMETA(DisplayName = "Investigating"),
    Patrolling      UMETA(DisplayName = "Patrolling"),
    Nesting         UMETA(DisplayName = "Nesting")
};

/**
 * Níveis de domesticação possíveis
 */
UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild            UMETA(DisplayName = "Wild"),
    Aware           UMETA(DisplayName = "Aware of Player"),
    Tolerant        UMETA(DisplayName = "Tolerant"),
    Friendly        UMETA(DisplayName = "Friendly"),
    Bonded          UMETA(DisplayName = "Bonded"),
    Domesticated    UMETA(DisplayName = "Domesticated")
};

/**
 * Estrutura para armazenar memórias do dinossauro
 */
USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Timestamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight; // -1.0 (muito negativo) a 1.0 (muito positivo)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MemoryType; // "food", "threat", "player", "safe_spot", etc.

    FDinosaurMemory()
    {
        Location = FVector::ZeroVector;
        Actor = nullptr;
        Timestamp = 0.0f;
        EmotionalWeight = 0.0f;
        MemoryType = TEXT("generic");
    }
};

/**
 * Estrutura para rotinas diárias dos dinossauros
 */
USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hora do dia (0.0 = meia-noite, 12.0 = meio-dia)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duração em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority; // 0.0 a 1.0

    FDailyRoutine()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        BehaviorState = EDinosaurBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        Priority = 0.5f;
    }
};

/**
 * Componente principal do sistema de comportamento dos NPCs
 * Gerencia a IA individual de cada dinossauro
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === PROPRIEDADES BÁSICAS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    EDinosaurPersonality Personality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float AggressionLevel; // 0.0 a 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float CuriosityLevel; // 0.0 a 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float SocialLevel; // 0.0 a 1.0

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    float FearLevel; // 0.0 a 1.0

    // === ESTADO ATUAL ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDinosaurBehaviorState CurrentBehaviorState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    EDomesticationLevel DomesticationLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float PlayerTrustLevel; // -1.0 (hostil) a 1.0 (confiante)

    // === MEMÓRIA E ROTINAS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FDinosaurMemory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutine> DailyRoutines;

    // === BEHAVIOR TREE ASSETS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBehaviorTree* MainBehaviorTree;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    UBlackboardAsset* DinosaurBlackboard;

    // === FUNÇÕES PÚBLICAS ===

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemory(FVector Location, AActor* Actor, FString MemoryType, float EmotionalWeight);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    TArray<FDinosaurMemory> GetMemoriesOfType(FString MemoryType);

    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ModifyPlayerTrust(float TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutine GetCurrentRoutine();

    UFUNCTION(BlueprintPure, Category = "Time")
    float GetCurrentTimeOfDay();

protected:
    // === FUNÇÕES INTERNAS ===
    
    void UpdateDailyRoutine();
    void ProcessMemories(float DeltaTime);
    void UpdateBehaviorTree();
    void HandlePlayerInteraction();
    
    // === VARIÁVEIS INTERNAS ===
    
    float LastRoutineCheck;
    float MemoryDecayRate;
    AActor* PlayerReference;
};

/**
 * Controller específico para dinossauros
 * Gerencia a integração com o sistema de Behavior Trees
 */
UCLASS()
class TRANSPERSONALGAME_API ADinosaurAIController : public AAIController
{
    GENERATED_BODY()

public:
    ADinosaurAIController();

protected:
    virtual void BeginPlay() override;
    virtual void Possess(APawn* InPawn) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    UNPCBehaviorComponent* BehaviorComponent;

    UFUNCTION(BlueprintCallable, Category = "AI")
    void InitializeBehaviorSystem(UBehaviorTree* BehaviorTree, UBlackboardAsset* Blackboard);

    UFUNCTION(BlueprintCallable, Category = "AI")
    void UpdateBlackboardFromBehaviorComponent();
};
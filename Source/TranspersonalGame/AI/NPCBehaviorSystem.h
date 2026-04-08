#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorSystem.generated.h"

// Forward Declarations
class UBehaviorTreeComponent;
class UBlackboardComponent;
class ANPCCreature;
class UNPCMemoryComponent;
class UNPCPersonalityComponent;

/**
 * Enum para definir os estados comportamentais base de qualquer NPC
 */
UENUM(BlueprintType)
enum class ENPCBehaviorState : uint8
{
    Idle           UMETA(DisplayName = "Idle - Sem actividade específica"),
    Foraging       UMETA(DisplayName = "Foraging - Procura de comida"),
    Drinking       UMETA(DisplayName = "Drinking - Beber água"),
    Resting        UMETA(DisplayName = "Resting - Descanso/Sono"),
    Socializing    UMETA(DisplayName = "Socializing - Interação social"),
    Patrolling     UMETA(DisplayName = "Patrolling - Patrulhamento territorial"),
    Hunting        UMETA(DisplayName = "Hunting - Caça (predadores)"),
    Fleeing        UMETA(DisplayName = "Fleeing - Fuga de perigo"),
    Investigating  UMETA(DisplayName = "Investigating - Investigação de estímulo"),
    Mating         UMETA(DisplayName = "Mating - Comportamento reprodutivo"),
    Nesting        UMETA(DisplayName = "Nesting - Construção/manutenção de ninho"),
    Migrating      UMETA(DisplayName = "Migrating - Migração sazonal"),
    Domesticated   UMETA(DisplayName = "Domesticated - Comportamento domesticado")
};

/**
 * Enum para tipos de personalidade que afectam decisões comportamentais
 */
UENUM(BlueprintType)
enum class ENPCPersonalityType : uint8
{
    Aggressive     UMETA(DisplayName = "Aggressive - Mais propenso a confronto"),
    Cautious       UMETA(DisplayName = "Cautious - Evita riscos, foge facilmente"),
    Curious        UMETA(DisplayName = "Curious - Investiga estímulos novos"),
    Territorial    UMETA(DisplayName = "Territorial - Defende área específica"),
    Social         UMETA(DisplayName = "Social - Prefere grupos, evita solidão"),
    Solitary       UMETA(DisplayName = "Solitary - Prefere isolamento"),
    Adaptive       UMETA(DisplayName = "Adaptive - Muda comportamento facilmente"),
    Stubborn       UMETA(DisplayName = "Stubborn - Mantém padrões estabelecidos")
};

/**
 * Struct para armazenar memórias específicas de eventos
 */
USTRUCT(BlueprintType)
struct FNPCMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* RelatedActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmotionalWeight; // -1.0 (muito negativo) a 1.0 (muito positivo)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeStamp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsImportant; // Memórias importantes não se degradam

    FNPCMemory()
    {
        Location = FVector::ZeroVector;
        RelatedActor = nullptr;
        EmotionalWeight = 0.0f;
        TimeStamp = 0.0f;
        EventDescription = TEXT("");
        bIsImportant = false;
    }
};

/**
 * Struct para rotinas diárias
 */
USTRUCT(BlueprintType)
struct FNPCDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime; // Hora do dia (0.0 = meia-noite, 12.0 = meio-dia)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Duration; // Duração em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ENPCBehaviorState BehaviorState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation; // Localização preferida para esta actividade

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationTolerance; // Quão flexível é sobre a localização

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Priority; // 1-10, prioridade desta rotina

    FNPCDailyRoutine()
    {
        StartTime = 0.0f;
        Duration = 1.0f;
        BehaviorState = ENPCBehaviorState::Idle;
        PreferredLocation = FVector::ZeroVector;
        LocationTolerance = 500.0f;
        Priority = 5;
    }
};

/**
 * Componente principal que gere todo o comportamento de um NPC
 * Este componente é o cérebro que coordena memória, personalidade e rotinas
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
    // === CONFIGURAÇÃO BASE ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    ENPCPersonalityType PersonalityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    TArray<FNPCDailyRoutine> DailyRoutines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Behavior")
    UBehaviorTree* BehaviorTreeAsset;

    // === ESTADO ACTUAL ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    ENPCBehaviorState CurrentBehaviorState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    float CurrentStateTime; // Há quanto tempo está neste estado

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC State")
    AActor* CurrentTarget; // Actor que está a focar (comida, predador, etc.)

    // === MEMÓRIA ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPCMemory> Memories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemories; // Limite de memórias armazenadas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate; // Velocidade de degradação das memórias

    // === DOMESTICAÇÃO ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Domestication")
    float DomesticationLevel; // 0.0 = selvagem, 1.0 = completamente domesticado

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Domestication")
    bool bCanBeDomesticated;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Domestication")
    float DomesticationRate; // Velocidade de ganho de domesticação

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Domestication")
    float DomesticationDecayRate; // Velocidade de perda se não houver interação

    // === MÉTODOS PÚBLICOS ===

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPCBehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    ENPCBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(FVector Location, AActor* Actor, float EmotionalWeight, const FString& Description, bool bImportant = false);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPCMemory> GetMemoriesOfActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPCMemory GetStrongestMemoryOfActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Domestication")
    void InteractWithPlayer(float PositiveInteractionStrength);

    UFUNCTION(BlueprintCallable, Category = "NPC Domestication")
    bool IsDomesticated() const { return DomesticationLevel > 0.7f; }

    UFUNCTION(BlueprintCallable, Category = "NPC Domestication")
    bool IsFriendly() const { return DomesticationLevel > 0.3f; }

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FNPCDailyRoutine GetCurrentRoutine() const;

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldChangeRoutine() const;

private:
    // === MÉTODOS PRIVADOS ===
    
    void UpdateDailyRoutine();
    void UpdateMemories(float DeltaTime);
    void UpdateDomestication(float DeltaTime);
    void ProcessPersonalityInfluence();
    
    float GetCurrentTimeOfDay() const;
    void CleanOldMemories();
    
    // Referências para componentes relacionados
    UPROPERTY()
    UBehaviorTreeComponent* BehaviorTreeComponent;
    
    UPROPERTY()
    UBlackboardComponent* BlackboardComponent;
};
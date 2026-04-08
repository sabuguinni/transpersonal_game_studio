#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NPCBehaviorSystem.generated.h"

// Forward declarations
class UDinosaurPersonality;
class UDailyRoutineComponent;
class UMemorySystem;
class USocialRelationshipComponent;

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros pequenos (domesticáveis)
    Compsognathus,
    Parasaurolophus_Juvenile,
    Triceratops_Juvenile,
    
    // Herbívoros grandes (não domesticáveis)
    Triceratops_Adult,
    Brachiosaurus,
    Stegosaurus,
    
    // Carnívoros pequenos (perigosos mas evitáveis)
    Velociraptor,
    Dilophosaurus,
    
    // Carnívoros grandes (predadores apex)
    Tyrannosaurus,
    Allosaurus,
    Carnotaurus,
    
    // Aquáticos
    Plesiosaur,
    
    // Voadores
    Pteranodon
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle,
    Foraging,
    Drinking,
    Resting,
    Socializing,
    Hunting,
    Fleeing,
    Territorial,
    Mating,
    Nesting,
    Migrating,
    Domesticated_Following,
    Domesticated_Guarding,
    Domesticated_Resting
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild,
    Wary,           // Começou a notar o jogador
    Curious,        // Aproxima-se ocasionalmente
    Tolerant,       // Não foge imediatamente
    Friendly,       // Aceita comida
    Bonded,         // Segue o jogador
    Domesticated    // Completamente domesticado
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurPersonalityTraits
{
    GENERATED_BODY()

    // Traços base de personalidade (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggression = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
    
    // Variações físicas únicas
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SizeVariation = FVector(1.0f, 1.0f, 1.0f); // Multiplicador do tamanho base
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor ColorVariation = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> UniqueFeatures; // "Cicatriz no olho esquerdo", "Corno quebrado", etc.
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDailyRoutineSchedule
{
    GENERATED_BODY()

    // Horário do dia (0.0 = meia-noite, 0.5 = meio-dia, 1.0 = meia-noite seguinte)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StartTime = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EndTime = 0.25f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState ScheduledBehavior = EDinosaurBehaviorState::Idle;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationRadius = 500.0f; // Raio em torno da localização preferida
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Priority = 0.5f; // Quão importante é esta rotina
};

/**
 * Sistema central de comportamento dos NPCs dinossauros
 * Gerencia personalidades, rotinas diárias, memória e relações sociais
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === CONFIGURAÇÃO INICIAL ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FDinosaurPersonalityTraits PersonalityTraits;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FDailyRoutineSchedule> DailyRoutine;
    
    // === ESTADO ATUAL ===
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float DomesticationProgress = 0.0f; // 0.0 a 1.0 para cada nível
    
    // === MEMÓRIA E RELAÇÕES ===
    
    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    TMap<AActor*, float> ActorMemory; // Actor -> Familiaridade (-1.0 a 1.0)
    
    UPROPERTY(BlueprintReadOnly, Category = "Social")
    TArray<AActor*> PackMembers; // Outros dinossauros do mesmo grupo
    
    UPROPERTY(BlueprintReadOnly, Category = "Social")
    AActor* PackLeader = nullptr;
    
    // === FUNÇÕES PÚBLICAS ===
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void InteractWithPlayer(AActor* Player, float PositiveInteraction = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    float GetFamiliarityWithActor(AActor* Actor) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdateActorMemory(AActor* Actor, float FamiliarityChange);
    
    UFUNCTION(BlueprintCallable, Category = "Routine")
    EDinosaurBehaviorState GetCurrentScheduledBehavior() const;
    
    UFUNCTION(BlueprintCallable, Category = "Routine")
    FVector GetCurrentScheduledLocation() const;
    
    UFUNCTION(BlueprintCallable, Category = "Species")
    bool CanBeDomesticated() const;
    
    UFUNCTION(BlueprintCallable, Category = "Species")
    bool IsHerbivore() const;
    
    UFUNCTION(BlueprintCallable, Category = "Species")
    bool IsCarnivore() const;
    
    UFUNCTION(BlueprintCallable, Category = "Species")
    bool IsApexPredator() const;

protected:
    // === FUNÇÕES INTERNAS ===
    
    void UpdateDailyRoutine();
    void UpdateDomestication(float DeltaTime);
    void UpdateMemoryDecay(float DeltaTime);
    void ProcessSocialBehavior();
    
    float GetCurrentTimeOfDay() const;
    FDailyRoutineSchedule GetActiveRoutineSchedule() const;
    
    // === CONFIGURAÇÕES ===
    
    UPROPERTY(EditAnywhere, Category = "Settings")
    float MemoryDecayRate = 0.1f; // Por dia
    
    UPROPERTY(EditAnywhere, Category = "Settings")
    float DomesticationDecayRate = 0.05f; // Por dia sem interação
    
    UPROPERTY(EditAnywhere, Category = "Settings")
    float RoutineFlexibility = 0.2f; // Quão rígidas são as rotinas (0.0 = muito rígido, 1.0 = muito flexível)

private:
    float LastRoutineUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    float LastDomesticationUpdate = 0.0f;
    
    // Referências aos componentes relacionados
    UPROPERTY()
    class UBehaviorTreeComponent* BehaviorTreeComponent;
    
    UPROPERTY()
    class UBlackboardComponent* BlackboardComponent;
};
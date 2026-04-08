#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "NPCBehaviorSystem.generated.h"

UENUM(BlueprintType)
enum class EDinosaurSpecies : uint8
{
    // Herbívoros Pequenos (Domesticáveis)
    Compsognathus       UMETA(DisplayName = "Compsognathus"),
    Parasaurolophus     UMETA(DisplayName = "Parasaurolophus"),
    Pachycephalosaurus  UMETA(DisplayName = "Pachycephalosaurus"),
    
    // Herbívoros Grandes (Não-Agressivos)
    Triceratops         UMETA(DisplayName = "Triceratops"),
    Brachiosaurus       UMETA(DisplayName = "Brachiosaurus"),
    Ankylosaurus        UMETA(DisplayName = "Ankylosaurus"),
    
    // Carnívoros Pequenos (Oportunistas)
    Velociraptor        UMETA(DisplayName = "Velociraptor"),
    Dilophosaurus       UMETA(DisplayName = "Dilophosaurus"),
    
    // Carnívoros Grandes (Predadores Apex)
    TyrannosaurusRex    UMETA(DisplayName = "T-Rex"),
    Allosaurus          UMETA(DisplayName = "Allosaurus"),
    Spinosaurus         UMETA(DisplayName = "Spinosaurus"),
    
    // Voadores
    Pteranodon          UMETA(DisplayName = "Pteranodon"),
    
    // Aquáticos
    Mosasaurus          UMETA(DisplayName = "Mosasaurus")
};

UENUM(BlueprintType)
enum class EDinosaurBehaviorState : uint8
{
    Idle                UMETA(DisplayName = "Idle"),
    Foraging            UMETA(DisplayName = "Foraging"),
    Drinking            UMETA(DisplayName = "Drinking"),
    Resting             UMETA(DisplayName = "Resting"),
    Socializing         UMETA(DisplayName = "Socializing"),
    Hunting             UMETA(DisplayName = "Hunting"),
    Fleeing             UMETA(DisplayName = "Fleeing"),
    Territorial         UMETA(DisplayName = "Territorial"),
    Mating              UMETA(DisplayName = "Mating"),
    Nesting             UMETA(DisplayName = "Nesting"),
    Migrating           UMETA(DisplayName = "Migrating"),
    Investigating       UMETA(DisplayName = "Investigating"),
    Domesticated        UMETA(DisplayName = "Domesticated")
};

UENUM(BlueprintType)
enum class EDomesticationLevel : uint8
{
    Wild                UMETA(DisplayName = "Wild"),
    Wary                UMETA(DisplayName = "Wary"),
    Curious             UMETA(DisplayName = "Curious"),
    Tolerant            UMETA(DisplayName = "Tolerant"),
    Friendly            UMETA(DisplayName = "Friendly"),
    Bonded              UMETA(DisplayName = "Bonded"),
    Loyal               UMETA(DisplayName = "Loyal")
};

USTRUCT(BlueprintType)
struct FDinosaurPersonality
{
    GENERATED_BODY()

    // Traços de personalidade (0.0 a 1.0)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Intelligence = 0.5f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Loyalty = 0.0f; // Apenas para domesticados
};

USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    // Localizações conhecidas
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownFoodSources;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownWaterSources;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownSafePlaces;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownDangerZones;
    
    // Relacionamentos com outros NPCs
    UPROPERTY(BlueprintReadWrite)
    TMap<AActor*, float> NPCRelationships; // -1.0 (inimigo) a 1.0 (aliado)
    
    // Memória do jogador
    UPROPERTY(BlueprintReadWrite)
    FVector LastPlayerLocation;
    
    UPROPERTY(BlueprintReadWrite)
    float PlayerTrustLevel = 0.0f; // -1.0 (hostil) a 1.0 (confiança total)
    
    UPROPERTY(BlueprintReadWrite)
    float TimeSincePlayerSeen = 999.0f;
};

USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartTime = 6.0f; // Hora do dia (0-24)
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EndTime = 8.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EDinosaurBehaviorState Activity = EDinosaurBehaviorState::Foraging;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector PreferredLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Priority = 1.0f; // Prioridade da actividade
};

/**
 * Componente principal do sistema de comportamento de NPCs
 * Gere a IA, personalidade, memória e rotinas diárias dos dinossauros
 */
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCBehaviorComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração da espécie
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Species")
    EDinosaurSpecies Species = EDinosaurSpecies::Compsognathus;
    
    // Personalidade única
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality")
    FDinosaurPersonality Personality;
    
    // Sistema de memória
    UPROPERTY(BlueprintReadWrite, Category = "Memory")
    FDinosaurMemory Memory;
    
    // Estado comportamental atual
    UPROPERTY(BlueprintReadWrite, Category = "Behavior")
    EDinosaurBehaviorState CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    
    // Nível de domesticação (apenas para espécies domesticáveis)
    UPROPERTY(BlueprintReadWrite, Category = "Domestication")
    EDomesticationLevel DomesticationLevel = EDomesticationLevel::Wild;
    
    // Rotina diária
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    TArray<FDailyRoutine> DailyRoutines;
    
    // Necessidades básicas (0.0 a 1.0)
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Hunger = 0.5f;
    
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Thirst = 0.5f;
    
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Energy = 1.0f;
    
    UPROPERTY(BlueprintReadWrite, Category = "Needs")
    float Health = 1.0f;
    
    // Estado social
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    TArray<AActor*> PackMembers;
    
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    AActor* PackLeader = nullptr;
    
    UPROPERTY(BlueprintReadWrite, Category = "Social")
    AActor* Mate = nullptr;
    
    // Território
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius = 2000.0f;

    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetBehaviorState(EDinosaurBehaviorState NewState);
    
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    EDinosaurBehaviorState GetCurrentBehaviorState() const { return CurrentBehaviorState; }
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void AddMemoryLocation(FVector Location, const FString& LocationType);
    
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void UpdatePlayerRelationship(float DeltaRelationship);
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    bool CanBeDomesticated() const;
    
    UFUNCTION(BlueprintCallable, Category = "Domestication")
    void ProcessDomesticationInteraction(float PositiveInteraction);
    
    UFUNCTION(BlueprintCallable, Category = "Routine")
    FDailyRoutine GetCurrentRoutineActivity() const;
    
    UFUNCTION(BlueprintCallable, Category = "Needs")
    void UpdateNeeds(float DeltaTime);
    
    UFUNCTION(BlueprintCallable, Category = "Social")
    void JoinPack(AActor* Leader, const TArray<AActor*>& Members);
    
    UFUNCTION(BlueprintCallable, Category = "Social")
    void LeavePack();

private:
    // Funções internas
    void InitializePersonality();
    void ProcessDailyRoutine();
    void UpdateBehaviorBasedOnNeeds();
    void ProcessSocialInteractions();
    void UpdateMemory(float DeltaTime);
    
    // Timers
    float LastRoutineCheck = 0.0f;
    float LastNeedsUpdate = 0.0f;
    float LastMemoryUpdate = 0.0f;
    
    // Cache do sistema de tempo
    class ATimeOfDayManager* TimeManager = nullptr;
};
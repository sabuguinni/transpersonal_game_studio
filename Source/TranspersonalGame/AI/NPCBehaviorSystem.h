#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NPCBehaviorSystem.generated.h"

// Enum para tipos de comportamento base
UENUM(BlueprintType)
enum class EDinosaurBehaviorType : uint8
{
    Herbivore_Passive,      // Herbívoros pacíficos (domesticáveis)
    Herbivore_Defensive,    // Herbívoros defensivos (Triceratops)
    Carnivore_Ambush,       // Predadores de emboscada
    Carnivore_Pack,         // Predadores de matilha
    Carnivore_Apex,         // Predadores apex (T-Rex)
    Scavenger              // Necrófagos oportunistas
};

// Estados emocionais dos dinossauros
UENUM(BlueprintType)
enum class EDinosaurEmotionalState : uint8
{
    Calm,           // Calmo - comportamento normal
    Alert,          // Alerta - detectou algo
    Aggressive,     // Agressivo - pronto para atacar
    Fearful,        // Com medo - fuga ou defesa
    Curious,        // Curioso - investigação
    Territorial,    // Territorial - defende área
    Hungry,         // Com fome - procura comida
    Tired,          // Cansado - procura descanso
    Protective      // Protetor - defende crias/grupo
};

// Estrutura para memória individual do dinossauro
USTRUCT(BlueprintType)
struct FDinosaurMemory
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownFoodLocations;

    UPROPERTY(BlueprintReadWrite)
    TArray<FVector> KnownDangerZones;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownThreats;

    UPROPERTY(BlueprintReadWrite)
    TArray<AActor*> KnownAllies;

    UPROPERTY(BlueprintReadWrite)
    FVector LastSeenPlayerLocation;

    UPROPERTY(BlueprintReadWrite)
    float LastPlayerEncounterTime;

    UPROPERTY(BlueprintReadWrite)
    bool HasSeenPlayer;

    UPROPERTY(BlueprintReadWrite)
    float TrustLevelTowardsPlayer; // 0.0 = hostil, 1.0 = domesticado

    FDinosaurMemory()
    {
        LastPlayerEncounterTime = 0.0f;
        HasSeenPlayer = false;
        TrustLevelTowardsPlayer = 0.0f;
    }
};

// Rotina diária do dinossauro
USTRUCT(BlueprintType)
struct FDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float WakeUpTime; // Hora do dia (0.0 = meia-noite, 0.5 = meio-dia)

    UPROPERTY(BlueprintReadWrite)
    float FeedingTime1;

    UPROPERTY(BlueprintReadWrite)
    float FeedingTime2;

    UPROPERTY(BlueprintReadWrite)
    float RestTime;

    UPROPERTY(BlueprintReadWrite)
    float SleepTime;

    UPROPERTY(BlueprintReadWrite)
    FVector PreferredFeedingArea;

    UPROPERTY(BlueprintReadWrite)
    FVector PreferredRestingArea;

    UPROPERTY(BlueprintReadWrite)
    FVector TerritoryCenter;

    UPROPERTY(BlueprintReadWrite)
    float TerritoryRadius;

    FDailyRoutine()
    {
        WakeUpTime = 0.25f; // 6:00
        FeedingTime1 = 0.33f; // 8:00
        FeedingTime2 = 0.67f; // 16:00
        RestTime = 0.5f; // 12:00
        SleepTime = 0.83f; // 20:00
        TerritoryRadius = 1000.0f;
    }
};

// Componente principal de comportamento
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

    // Configuração do comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurBehaviorType BehaviorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    EDinosaurEmotionalState CurrentEmotionalState;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDinosaurMemory Memory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    FDailyRoutine DailyRoutine;

    // Características individuais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    float Aggressiveness; // 0.0 = pacífico, 1.0 = muito agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    float Curiosity; // 0.0 = evita novidades, 1.0 = muito curioso

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    float Sociability; // 0.0 = solitário, 1.0 = muito social

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    float Intelligence; // 0.0 = instintivo, 1.0 = muito inteligente

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Individual Traits")
    float Territoriality; // 0.0 = nómada, 1.0 = muito territorial

    // Funções de comportamento
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateEmotionalState();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessDailyRoutine();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(AActor* Player, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void UpdateMemory(AActor* Actor, bool IsThreat);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool ShouldEnterDomesticationProcess();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ProcessDomestication(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector GetCurrentGoalLocation();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    bool IsInTerritory(FVector Location);

private:
    float CurrentTimeOfDay;
    float DomesticationProgress;
    bool bIsBeingDomesticated;
    
    // Timers internos
    float LastRoutineUpdate;
    float LastEmotionalUpdate;
    float LastMemoryUpdate;
};
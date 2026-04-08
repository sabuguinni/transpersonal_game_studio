#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "NPCDailyRoutineSystem.generated.h"

// Tipos de atividades diárias que um NPC pode realizar
UENUM(BlueprintType)
enum class ENPCActivityType : uint8
{
    Idle           UMETA(DisplayName = "Idle"),
    Foraging       UMETA(DisplayName = "Foraging"),
    Drinking       UMETA(DisplayName = "Drinking"),
    Socializing    UMETA(DisplayName = "Socializing"),
    Resting        UMETA(DisplayName = "Resting"),
    Patrolling     UMETA(DisplayName = "Patrolling"),
    Grooming       UMETA(DisplayName = "Grooming"),
    Feeding        UMETA(DisplayName = "Feeding"),
    Nesting        UMETA(DisplayName = "Nesting"),
    Exploring      UMETA(DisplayName = "Exploring"),
    Fleeing        UMETA(DisplayName = "Fleeing"),
    Hunting        UMETA(DisplayName = "Hunting")
};

// Estados emocionais que afetam o comportamento
UENUM(BlueprintType)
enum class ENPCEmotionalState : uint8
{
    Calm           UMETA(DisplayName = "Calm"),
    Curious        UMETA(DisplayName = "Curious"),
    Fearful        UMETA(DisplayName = "Fearful"),
    Aggressive     UMETA(DisplayName = "Aggressive"),
    Hungry         UMETA(DisplayName = "Hungry"),
    Tired          UMETA(DisplayName = "Tired"),
    Alert          UMETA(DisplayName = "Alert"),
    Playful        UMETA(DisplayName = "Playful"),
    Protective     UMETA(DisplayName = "Protective"),
    Stressed       UMETA(DisplayName = "Stressed")
};

// Estrutura para definir uma atividade específica
USTRUCT(BlueprintType)
struct FNPCActivity
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    ENPCActivityType ActivityType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    FString ActivityName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float StartTime; // Hora do dia (0-24)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Duration; // Duração em horas

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float Priority; // Prioridade da atividade (0-1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    FVector PreferredLocation; // Local preferido para a atividade

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float LocationRadius; // Raio de busca para locais adequados

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    TArray<ENPCEmotionalState> RequiredEmotionalStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    bool bCanBeInterrupted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Activity")
    float InterruptionThreshold;

    FNPCActivity()
    {
        ActivityType = ENPCActivityType::Idle;
        ActivityName = TEXT("Default Activity");
        StartTime = 8.0f;
        Duration = 2.0f;
        Priority = 0.5f;
        PreferredLocation = FVector::ZeroVector;
        LocationRadius = 1000.0f;
        bCanBeInterrupted = true;
        InterruptionThreshold = 0.7f;
    }
};

// Estrutura para rotina diária completa
USTRUCT(BlueprintType)
struct FNPCDailyRoutine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    FString RoutineName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    TArray<FNPCActivity> Activities;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    float RoutineVariation; // Variação aleatória nos horários (0-1)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bAdaptToWeather;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routine")
    bool bAdaptToSeason;

    FNPCDailyRoutine()
    {
        RoutineName = TEXT("Default Routine");
        RoutineVariation = 0.2f;
        bAdaptToWeather = true;
        bAdaptToSeason = false;
    }
};

// Estrutura para dados de personalidade do NPC
USTRUCT(BlueprintType)
struct FNPCPersonality
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Curiosity; // Tendência para explorar

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Sociability; // Tendência para interagir com outros

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Aggressiveness; // Tendência para comportamento agressivo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Fearfulness; // Tendência para ter medo

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Territoriality; // Tendência para defender território

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Personality", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ActivityLevel; // Nível geral de atividade

    FNPCPersonality()
    {
        Curiosity = 0.5f;
        Sociability = 0.5f;
        Aggressiveness = 0.3f;
        Fearfulness = 0.4f;
        Territoriality = 0.5f;
        ActivityLevel = 0.6f;
    }
};

// Componente principal do sistema de rotinas diárias
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCDailyRoutineComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCDailyRoutineComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Configuração da rotina
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FNPCDailyRoutine DailyRoutine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Daily Routine")
    FNPCPersonality Personality;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    FNPCActivity CurrentActivity;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ENPCEmotionalState CurrentEmotionalState;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float CurrentTime; // Tempo atual do dia (0-24)

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    float ActivityStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    bool bIsActivityActive;

    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float TimeScale; // Escala de tempo (1.0 = tempo real)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bUseGameTimeOfDay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float EmotionalStateDecayRate;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void StartRoutine();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void StopRoutine();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void InterruptCurrentActivity(float InterruptionStrength);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void SetEmotionalState(ENPCEmotionalState NewState, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    FNPCActivity GetNextScheduledActivity();

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    bool IsActivityTime(const FNPCActivity& Activity);

    UFUNCTION(BlueprintCallable, Category = "Daily Routine")
    void AdaptRoutineToConditions();

    // Eventos
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActivityChanged, ENPCActivityType, OldActivity, ENPCActivityType, NewActivity);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnActivityChanged OnActivityChanged;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, ENPCEmotionalState, OldState, ENPCEmotionalState, NewState);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;

private:
    void UpdateCurrentTime(float DeltaTime);
    void CheckForActivityTransition();
    void ExecuteCurrentActivity();
    void UpdateEmotionalState(float DeltaTime);
    FNPCActivity SelectBestActivity();
    float CalculateActivityScore(const FNPCActivity& Activity);
    void ApplyPersonalityToActivity(FNPCActivity& Activity);

    float EmotionalStateIntensity;
    float LastActivityTransitionTime;
    bool bRoutineActive;
};

// Sistema global para gerenciar todas as rotinas NPCs
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCDailyRoutineManager : public AActor
{
    GENERATED_BODY()

public:
    ANPCDailyRoutineManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // Configurações globais
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float GlobalTimeScale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    float DayDuration; // Duração de um dia em segundos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Settings")
    bool bSynchronizeAllNPCs;

    // Estado global
    UPROPERTY(BlueprintReadOnly, Category = "Global State")
    float CurrentGameTime; // 0-24 horas

    UPROPERTY(BlueprintReadOnly, Category = "Global State")
    int32 CurrentDay;

    UPROPERTY(BlueprintReadOnly, Category = "Global State")
    TArray<UNPCDailyRoutineComponent*> RegisteredNPCs;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Routine Manager")
    void RegisterNPC(UNPCDailyRoutineComponent* NPCComponent);

    UFUNCTION(BlueprintCallable, Category = "Routine Manager")
    void UnregisterNPC(UNPCDailyRoutineComponent* NPCComponent);

    UFUNCTION(BlueprintCallable, Category = "Routine Manager")
    void SetGlobalTimeScale(float NewTimeScale);

    UFUNCTION(BlueprintCallable, Category = "Routine Manager")
    float GetCurrentTimeOfDay() const;

    UFUNCTION(BlueprintCallable, Category = "Routine Manager")
    void TriggerGlobalEvent(const FString& EventName, float Intensity);

    // Eventos globais
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewDay, int32, DayNumber);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnNewDay OnNewDay;

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeOfDayChanged, float, TimeOfDay);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeOfDayChanged OnTimeOfDayChanged;

private:
    void UpdateGlobalTime(float DeltaTime);
    void BroadcastTimeUpdate();
    void HandleNewDay();

    float LastBroadcastTime;
    float TimeUpdateInterval;
};
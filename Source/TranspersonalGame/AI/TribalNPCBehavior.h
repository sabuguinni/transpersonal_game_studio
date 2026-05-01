#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/Navigation/NavigationTypes.h"
#include "NavigationSystem.h"
#include "TribalNPCBehavior.generated.h"

// Enums para comportamento tribal
UENUM(BlueprintType)
enum class ENPC_TribalState : uint8
{
    Idle,
    Gathering,
    Patrolling,
    Fleeing,
    Socializing,
    Resting,
    Hunting,
    Crafting
};

UENUM(BlueprintType)
enum class ENPC_TribalPersonality : uint8
{
    Cautious,
    Aggressive,
    Curious,
    Fearful,
    Leader,
    Follower
};

UENUM(BlueprintType)
enum class ENPC_SurvivalNeed : uint8
{
    Food,
    Water,
    Shelter,
    Safety,
    Social,
    Rest
};

// Estrutura para necessidades de sobrevivência
USTRUCT(BlueprintType)
struct FNPC_SurvivalStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Energy = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Fear = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Comfort = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    float Social = 50.0f;
};

// Estrutura para memória de localizações
USTRUCT(BlueprintType)
struct FNPC_LocationMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FString LocationType = "";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float LastVisited = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    bool bIsSafe = true;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_TribalNPCBehavior : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_TribalNPCBehavior();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Estado e personalidade
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_TribalState CurrentState = ENPC_TribalState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    ENPC_TribalPersonality Personality = ENPC_TribalPersonality::Cautious;

    // Estatísticas de sobrevivência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
    FNPC_SurvivalStats SurvivalStats;

    // Memória de localizações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    TArray<FNPC_LocationMemory> KnownLocations;

    // Configurações de comportamento
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float FleeDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float SocialDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float MovementSpeed = 200.0f;

    // Timers para atividades
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float StateChangeInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float SurvivalDecayRate = 1.0f;

    // Referências
    UPROPERTY(BlueprintReadOnly, Category = "References")
    ACharacter* OwnerCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "References")
    UCharacterMovementComponent* MovementComponent;

    // Funções públicas
    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void SetTribalState(ENPC_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void AddLocationMemory(FVector Location, FString LocationType, float Importance = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    FVector FindNearestLocationOfType(FString LocationType);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    void UpdateSurvivalNeeds(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Survival")
    ENPC_SurvivalNeed GetHighestPriorityNeed();

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToPlayer(ACharacter* PlayerCharacter, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Behavior")
    void ReactToDinosaur(AActor* DinosaurActor, float Distance);

private:
    // Variáveis internas
    float StateTimer = 0.0f;
    float SurvivalTimer = 0.0f;
    FVector HomeLocation = FVector::ZeroVector;
    FVector CurrentTarget = FVector::ZeroVector;
    bool bHasTarget = false;

    // Funções internas
    void UpdateBehaviorState(float DeltaTime);
    void ExecuteCurrentState(float DeltaTime);
    void MoveToTarget(FVector TargetLocation);
    void IdleBehavior();
    void GatheringBehavior();
    void PatrollingBehavior();
    void FleeingBehavior();
    void SocializingBehavior();
    void RestingBehavior();
    void HuntingBehavior();
    void CraftingBehavior();
    
    void CheckForThreats();
    void CheckForOpportunities();
    void UpdatePersonalityInfluence();
    bool IsLocationSafe(FVector Location);
    float CalculateLocationSafety(FVector Location);
};
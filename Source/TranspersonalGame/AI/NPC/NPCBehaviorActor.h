#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "NPCBehaviorActor.generated.h"

UENUM(BlueprintType)
enum class ENPC_BehaviorState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrolling  UMETA(DisplayName = "Patrolling"),
    Working     UMETA(DisplayName = "Working"),
    Socializing UMETA(DisplayName = "Socializing"),
    Fleeing     UMETA(DisplayName = "Fleeing"),
    Following   UMETA(DisplayName = "Following"),
    Investigating UMETA(DisplayName = "Investigating")
};

UENUM(BlueprintType)
enum class ENPC_Profession : uint8
{
    Elder       UMETA(DisplayName = "Elder"),
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Crafter     UMETA(DisplayName = "Crafter"),
    Scout       UMETA(DisplayName = "Scout"),
    Healer      UMETA(DisplayName = "Healer"),
    Child       UMETA(DisplayName = "Child")
};

USTRUCT(BlueprintType)
struct FNPC_DailySchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float StartHour = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    float EndHour = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    FVector WorkLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    ENPC_BehaviorState ScheduledActivity = ENPC_BehaviorState::Working;

    FNPC_DailySchedule()
    {
        StartHour = 6.0f;
        EndHour = 18.0f;
        WorkLocation = FVector::ZeroVector;
        ScheduledActivity = ENPC_BehaviorState::Working;
    }
};

USTRUCT(BlueprintType)
struct FNPC_Memory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    AActor* RememberedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    FVector LastKnownLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float ThreatLevel = 0.0f;

    FNPC_Memory()
    {
        RememberedActor = nullptr;
        LastKnownLocation = FVector::ZeroVector;
        Timestamp = 0.0f;
        ThreatLevel = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANPCBehaviorActor : public ACharacter
{
    GENERATED_BODY()

public:
    ANPCBehaviorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // === CORE NPC PROPERTIES ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    FString NPCName = TEXT("Unnamed NPC");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    ENPC_Profession Profession = ENPC_Profession::Gatherer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Identity")
    float SocialNeed = 50.0f;

    // === BEHAVIOR STATE ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
    ENPC_BehaviorState CurrentState = ENPC_BehaviorState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float StateChangeTimer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
    float InteractionRange = 300.0f;

    // === DAILY SCHEDULE ===
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Schedule")
    TArray<FNPC_DailySchedule> DailySchedule;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Schedule")
    int32 CurrentScheduleIndex = 0;

    // === MEMORY SYSTEM ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
    TArray<FNPC_Memory> ShortTermMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    int32 MaxMemoryEntries = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory")
    float MemoryDecayRate = 0.1f;

    // === AI COMPONENTS ===
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class UPawnSensingComponent* PawnSensing;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    class USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
    class UBehaviorTree* BehaviorTreeAsset;

    // === BEHAVIOR FUNCTIONS ===
    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void SetBehaviorState(ENPC_BehaviorState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    void UpdateDailySchedule();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(AActor* Actor, FVector Location, float ThreatLevel);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_Memory GetMemoryOfActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void OnPlayerApproach(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "NPC Interaction")
    void OnDinosaurSighted(AActor* Dinosaur);

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    FVector GetRandomPatrolPoint();

    UFUNCTION(BlueprintCallable, Category = "NPC Behavior")
    bool ShouldFlee();

    // === SENSING CALLBACKS ===
    UFUNCTION()
    void OnSeePawn(APawn* Pawn);

    UFUNCTION()
    void OnHearNoise(APawn* NoiseInstigator, const FVector& Location, float Volume);

protected:
    // === INTERNAL STATE ===
    FVector HomeLocation;
    FVector CurrentTarget;
    float LastScheduleUpdate = 0.0f;
    bool bIsInConversation = false;
    
    // === HELPER FUNCTIONS ===
    void InitializeSchedule();
    void ProcessBehaviorLogic(float DeltaTime);
    void HandleMovement(float DeltaTime);
    void UpdateFearLevel(float DeltaTime);
};
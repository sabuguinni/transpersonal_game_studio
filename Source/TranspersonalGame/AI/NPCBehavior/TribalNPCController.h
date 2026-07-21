#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "TribalNPCController.generated.h"

UENUM(BlueprintType)
enum class ENPC_TribalState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Patrol      UMETA(DisplayName = "Patrol"),
    Gather      UMETA(DisplayName = "Gather"),
    Flee        UMETA(DisplayName = "Flee"),
    Hide        UMETA(DisplayName = "Hide"),
    Alert       UMETA(DisplayName = "Alert"),
    Combat      UMETA(DisplayName = "Combat"),
    Dead        UMETA(DisplayName = "Dead")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    None        UMETA(DisplayName = "None"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Critical    UMETA(DisplayName = "Critical")
};

USTRUCT(BlueprintType)
struct FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FVector ThreatLocation;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    float ThreatTime;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    ENPC_ThreatLevel ThreatLevel;

    UPROPERTY(BlueprintReadWrite, Category = "NPC|Memory")
    FString ThreatActorName;

    FNPC_MemoryEntry()
        : ThreatLocation(FVector::ZeroVector)
        , ThreatTime(0.0f)
        , ThreatLevel(ENPC_ThreatLevel::None)
        , ThreatActorName(TEXT(""))
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ATribalNPCController : public AAIController
{
    GENERATED_BODY()

public:
    ATribalNPCController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;

    // Perception callbacks
    UFUNCTION()
    void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

    // State management
    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void SetNPCState(ENPC_TribalState NewState);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    ENPC_TribalState GetNPCState() const { return CurrentState; }

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void RegisterThreat(AActor* ThreatActor, ENPC_ThreatLevel Level, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    void ClearMemory();

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    bool HasRecentThreat(float WithinSeconds = 10.0f) const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    FVector GetLastKnownThreatLocation() const;

    UFUNCTION(BlueprintCallable, Category = "NPC|Behavior")
    float GetFearLevel() const { return FearLevel; }

    // Blackboard keys
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_TargetLocation = FName("TargetLocation");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_ThreatActor = FName("ThreatActor");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_NPCState = FName("NPCState");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_FearLevel = FName("FearLevel");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Blackboard")
    FName BB_FleeTarget = FName("FleeTarget");

    // Behavior tree
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|AI")
    UBehaviorTree* BehaviorTree;

    // Perception radii
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float SightRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float HearingRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Perception")
    float LoseSightRadius = 2500.0f;

    // Memory decay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    float MemoryDecayTime = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Memory")
    int32 MaxMemoryEntries = 8;

    // Fear
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Fear")
    float FearDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Fear")
    float FearBuildRate = 0.3f;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Perception")
    UAIPerceptionComponent* AIPerceptionComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Perception")
    UAISenseConfig_Sight* SightConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Perception")
    UAISenseConfig_Hearing* HearingConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|State")
    ENPC_TribalState CurrentState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Memory")
    TArray<FNPC_MemoryEntry> MemoryEntries;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC|Fear")
    float FearLevel;

    void UpdateFear(float DeltaTime);
    void ProcessMemoryDecay(float DeltaTime);
    void UpdateBlackboard();
    void DetermineStateFromPerception();
    FVector FindFleeLocation(const FVector& ThreatLocation) const;
    FVector FindHideLocation(const FVector& ThreatLocation) const;
};

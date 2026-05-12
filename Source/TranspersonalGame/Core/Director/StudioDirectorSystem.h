#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "../../SharedTypes.h"
#include "StudioDirectorSystem.generated.h"

UENUM(BlueprintType)
enum class EDir_AgentPriority : uint8
{
    Critical    UMETA(DisplayName = "Critical"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Standby     UMETA(DisplayName = "Standby")
};

UENUM(BlueprintType)
enum class EDir_AgentStatus : uint8
{
    Active      UMETA(DisplayName = "Active"),
    Waiting     UMETA(DisplayName = "Waiting"),
    Blocked     UMETA(DisplayName = "Blocked"),
    Complete    UMETA(DisplayName = "Complete"),
    Failed      UMETA(DisplayName = "Failed")
};

UENUM(BlueprintType)
enum class EDir_CyclePhase : uint8
{
    Planning        UMETA(DisplayName = "Planning"),
    Architecture    UMETA(DisplayName = "Architecture"),
    CoreSystems     UMETA(DisplayName = "Core Systems"),
    WorldBuilding   UMETA(DisplayName = "World Building"),
    Characters      UMETA(DisplayName = "Characters"),
    Gameplay        UMETA(DisplayName = "Gameplay"),
    Polish          UMETA(DisplayName = "Polish"),
    Testing         UMETA(DisplayName = "Testing"),
    Integration     UMETA(DisplayName = "Integration")
};

USTRUCT(BlueprintType)
struct FDir_AgentInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString AgentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    int32 AgentNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentPriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    EDir_AgentStatus Status;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    FString CurrentTask;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    float CompletionPercentage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Dependencies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Agent")
    TArray<FString> Deliverables;

    FDir_AgentInfo()
    {
        AgentName = TEXT("");
        AgentNumber = 0;
        Priority = EDir_AgentPriority::Medium;
        Status = EDir_AgentStatus::Waiting;
        CurrentTask = TEXT("");
        CompletionPercentage = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FDir_CycleInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString CycleID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    EDir_CyclePhase CurrentPhase;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    float CycleProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FDateTime CycleStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    FString PrimaryObjective;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    TArray<FString> Milestones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cycle")
    TArray<FDir_AgentInfo> AgentQueue;

    FDir_CycleInfo()
    {
        CycleID = TEXT("");
        CurrentPhase = EDir_CyclePhase::Planning;
        CycleProgress = 0.0f;
        CycleStartTime = FDateTime::Now();
        PrimaryObjective = TEXT("");
    }
};

/**
 * Studio Director System - Centralized coordination and management for all development agents
 * Responsible for task distribution, priority management, and quality assurance
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UStudioDirectorSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UStudioDirectorSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CORE DIRECTOR FUNCTIONS ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void InitializeProductionCycle(const FString& CycleID, const FString& PrimaryObjective);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetCyclePhase(EDir_CyclePhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateAgentStatus(int32 AgentNumber, EDir_AgentStatus NewStatus, float CompletionPercentage = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AssignTaskToAgent(int32 AgentNumber, const FString& TaskDescription, EDir_AgentPriority Priority);

    // === AGENT MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void RegisterAgent(int32 AgentNumber, const FString& AgentName, const TArray<FString>& Capabilities);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FDir_AgentInfo GetAgentInfo(int32 AgentNumber);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    TArray<FDir_AgentInfo> GetActiveAgents();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void BlockAgent(int32 AgentNumber, const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UnblockAgent(int32 AgentNumber);

    // === MILESTONE TRACKING ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void AddMilestone(const FString& MilestoneDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CompleteMilestone(const FString& MilestoneDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    float GetOverallProgress();

    // === QUALITY ASSURANCE ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ReportQualityIssue(int32 AgentNumber, const FString& IssueDescription);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ValidateDeliverable(int32 AgentNumber, const FString& DeliverableName, bool bIsValid);

    // === COORDINATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void TriggerAgentSequence(const TArray<int32>& AgentNumbers);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void EmergencyStop(const FString& Reason);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void ResumeProduction();

    // === REPORTING ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    FString GenerateStatusReport();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void LogDirectorMessage(const FString& Message, bool bIsError = false);

    // === EVENTS ===
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAgentStatusChanged, int32, AgentNumber, EDir_AgentStatus, NewStatus);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCyclePhaseChanged, EDir_CyclePhase, NewPhase);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMilestoneCompleted, FString, MilestoneName);
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmergencyStop, FString, Reason);

    UPROPERTY(BlueprintAssignable, Category = "Studio Director Events")
    FOnAgentStatusChanged OnAgentStatusChanged;

    UPROPERTY(BlueprintAssignable, Category = "Studio Director Events")
    FOnCyclePhaseChanged OnCyclePhaseChanged;

    UPROPERTY(BlueprintAssignable, Category = "Studio Director Events")
    FOnMilestoneCompleted OnMilestoneCompleted;

    UPROPERTY(BlueprintAssignable, Category = "Studio Director Events")
    FOnEmergencyStop OnEmergencyStop;

protected:
    // === CORE DATA ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    FDir_CycleInfo CurrentCycle;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TMap<int32, FDir_AgentInfo> RegisteredAgents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    TArray<FString> QualityIssues;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    bool bProductionActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Studio Director", meta = (AllowPrivateAccess = "true"))
    FString EmergencyStopReason;

    // === INTERNAL FUNCTIONS ===
    
    void InitializeDefaultAgents();
    void ValidateAgentDependencies();
    void UpdateCycleProgress();
    bool CanAgentProceed(int32 AgentNumber);
    void NotifyAgentCompletion(int32 AgentNumber);
};

/**
 * Studio Director Actor - Physical representation in the world for coordination
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AStudioDirectorActor : public AActor
{
    GENERATED_BODY()

public:
    AStudioDirectorActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStudioDirectorSystem* DirectorSystem;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* HubMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UPointLightComponent* CoordinationLight;

    // === VISUAL COORDINATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void UpdateVisualStatus();

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SetCoordinationLightColor(FLinearColor NewColor);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void SpawnAgentMarker(int32 AgentNumber, FVector Location, FLinearColor Color);

    UFUNCTION(BlueprintCallable, Category = "Studio Director")
    void CreateTestingArea(FVector CenterLocation, float Radius);
};